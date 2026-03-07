#include <cstdio>
#include <string>
#include <map>
#include <algorithm>
#include <utility> // for std::move()

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/PassManager.h>
#include "KaleidoscopeJIT.h"

#include "ast.h"

using namespace llvm;
using namespace llvm::orc;

// global state defined in main used for codegen
extern std::unique_ptr<LLVMContext> TheContext;
extern std::unique_ptr<Module> TheModule;
extern std::unique_ptr<IRBuilder<>> Builder;
// Map the holds what is in the current scope
extern std::map<std::string, Value*> NamedValues;
extern std::unique_ptr<KaleidoscopeJIT> TheJIT;

// Map that holds the most recent prototype for each function.
extern std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;

// global state defined in main used for optimization
extern std::unique_ptr<FunctionPassManager> TheFPM;
extern std::unique_ptr<FunctionAnalysisManager> TheFAM;


// Helper that replaces calls to TheModule->getFunction(). Searches TheModule for an
// existing function declaration, falling back to generating a new declaration from
// FunctionProtos (previous Modules) if it doesn’t find one. This allows each function
// to live in its own module, because we re-generate previous function declarations into
// each new module we open; WE CAN ALWAYS OBTAIN A FUNCTION DECLARATION IN THE CURRENT
// MODULE FOR ANY PREVIOUSLY DECLARED FUNCTION. By regenerating declarations we enable
// each function to live in its Module and we deal with the current module being deleted
//  when evaluating a top-level expression.
static Function* getFunction(std::string Name) {
    // First, see if the function has already been added to the current module.
    if (auto *F = TheModule->getFunction(Name))
        return F;

    // If not, check whether we can codegen the declaration from some existing
    // prototype that was added in another module.
    auto FI = FunctionProtos.find(Name);
    if (FI != FunctionProtos.end())
        return FI->second->codegen();

    // If no existing prototype exists, return null.
    return nullptr;
}

/// ---------------------------------------------
/// Inserisci qua i metodi non banali delle classi
/// ----------------------------------------------

// NumberExpreAST
std::string NumberExprAST::ToString() const {
    return std::to_string(Val);
}

Value* NumberExprAST::codegen() const {
    // In the LLVM IR, numeric constants are represented with the ConstantFP class,
    // which holds the numeric value in an APFloat (ArbitraryPrecision) internally.
    // This code basically just creates and returns a ConstantFP.
    return ConstantFP::get(*TheContext, APFloat(Val));
}

// VariableExprAST
std::string VariableExprAST::ToString() const {
    return Name;
}

Value* VariableExprAST::codegen() const {
    // Look this variable up in the function.
    Value *V = NamedValues[Name];
    if (!V)
        LogErrorV("VariableExprAST::codegen() | Unknown variable name");
    return V;
}

// BinaryExprAST
std::string BinaryExprAST::ToString() const {
    std::string LeftString = LHS->ToString();
    std::string RightString = RHS->ToString();

    return "(" + LeftString + Op + RightString + ")";
}

Value* BinaryExprAST::codegen() const {
    Value *L = LHS->codegen();
    Value *R = RHS->codegen();
    if (!L || !R)
        return LogErrorV("BinaryExprAST::codegen() | couldn't codegen of of my operands");

    // Here we specify what instruction to create (e.g. with CreateFAdd),
    // which operands to use (L and R here) and optionally provide a name
    // for the generated instruction. The name is just an hint if the code
    //  above emits multiple “addtmp” variables, LLVM will automatically
    // provide each one with an increasing, unique numeric suffix.
    // The IRBuilder knows where to insert the newly created instruction.
    switch (Op) {
    case '+':
        return Builder->CreateFAdd(L, R, "addtmp");
    case '-':
        return Builder->CreateFSub(L, R, "subtmp");
    case '*':
        return Builder->CreateFMul(L, R, "multmp");
    case '<':
        // fcmp instruction always returns a 1-bit integer (i1), but
        // kaleidoscope wants the valued to be a dobule (0.0/1.0).
        // To get the desired semantics, first we create the fcmp
        // instruction, then we combine it with the uitofp (UnsignedInt2FP)
        // instruction.
        L = Builder->CreateFCmpULT(L, R, "cmptmp");
        // Convert bool 0/1 to double 0.0 or 1.0
        return Builder->CreateUIToFP(L, Type::getDoubleTy(*TheContext), "booltmp");
    default:
        return LogErrorV("BinaryExprAST::codegen() | invalid binary operator");
    }
}

// CallExprAST
std::string CallExprAST::ToString() const {
    std::string res {};

    res += Callee;
    res += "(";
    for(const auto& arg : Args) {
        res += arg->ToString();
        res += ",";
    }
    // se ci sono argomenti sostituisco l'ultimo ',' spurio con ')'
    if(Args.size() > 0)
        res.at(res.size()-1) = ')';
    else
        res += ')';

    return res;
}

Value* CallExprAST::codegen() const {
    // Look up the name in one of the active modules
    Function *CalleeF = getFunction(Callee);
    if (!CalleeF)
        return LogErrorV("CallExprAST::codegen() | Unknown function referenced");

    // If argument mismatch error.
    if (CalleeF->arg_size() != Args.size())
        return LogErrorV("CallExprAST::codegen() | Incorrect # arguments passed");

    // codegen all the args
    std::vector<Value *> ArgsV;
    for (unsigned i=0, e=Args.size(); i != e; ++i) {
        ArgsV.push_back(Args[i]->codegen());
        // check if the current argument has been codegened
        if (!ArgsV.back())
            return nullptr;
    }

    // codegen the call
    return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

// IfExprAST
std::string IfExprAST::ToString() const {
    std::string res {};

    res += "if " + Cond->ToString() + " then\n\t";
    res += Then->ToString() + "\nelse\n\t";
    res += Else->ToString() + "\n";

    return res;
}

Value* IfExprAST::codegen() const {
    Value *CondV = Cond->codegen();
    if (!CondV)
        return LogErrorV("IfExprAST::codegen() | couldn't codegen the condition");

    // Convert condition to a bool by comparing non-equal (ONE == Ordered and Not Equal) to 0.0.
    CondV = Builder->CreateFCmpONE(CondV, ConstantFP::get(*TheContext, APFloat(0.0)), "ifcond");

    // This gets the current Function object that is being built.
    // - asks the builder for the current BasicBlock
    // - and asks that block for its “parent”
    //    - (the function it is currently embedded into).
    //
    // Remember that an if/else is always codegened inside a function
    // body (maybe __anon_expr).This means that we're already inside
    // the BB of that function (created in FunctionAST::codegen).
    // TheBuilder saves this state and also records where the current
    // insertion point for the IR is.
    // NB: actually, we might also be inside a then/else BB if we have
    // nested ifs.
    Function *TheFunction = Builder->GetInsertBlock()->getParent();
    // Create blocks for the then and else cases. Insert the 'then' block
    // at the (current) end of the function. The other two blocks are
    // created, but aren’t yet inserted into the function.
    BasicBlock *ThenBB = BasicBlock::Create(*TheContext, "then", TheFunction);
    BasicBlock *ElseBB = BasicBlock::Create(*TheContext, "else");
    BasicBlock *MergeBB = BasicBlock::Create(*TheContext, "ifcont");
    // Emit the conditional branch
    Builder->CreateCondBr(CondV, ThenBB, ElseBB);

    // Emit then block.
    Builder->SetInsertPoint(ThenBB);
    Value *ThenV = Then->codegen();
    if (!ThenV)
        return LogErrorV("IfExprAST::codegen() | couldn't codegen the then block");
    // Emit the jump to the merge block
    Builder->CreateBr(MergeBB);
    // Codegen of 'Then' can change the current block, update ThenBB to be the
    // current block we're inserting into, this is the right source for the PHI.
    ThenBB = Builder->GetInsertBlock(); // gets the inserting-into current block

    // Emit else block.
    // il then block era già in fondo; con l'else devo aggiungerlo io
    TheFunction->insert(TheFunction->end(), ElseBB);
    Builder->SetInsertPoint(ElseBB);
    Value *ElseV = Else->codegen();
    if (!ElseV)
        return LogErrorV("IfExprAST::codegen() | couldn't codegen the else block");
    // Emite the jump to the merge block
    Builder->CreateBr(MergeBB);
    // codegen of 'Else' can change the current block, update ElseBB for the PHI.
    ElseBB = Builder->GetInsertBlock();

    // Emit merge block.
    TheFunction->insert(TheFunction->end(), MergeBB);
    Builder->SetInsertPoint(MergeBB);
    PHINode *PN = Builder->CreatePHI(Type::getDoubleTy(*TheContext), 2, "iftmp");
    // setup the source block/value pairs of the phi operation
    PN->addIncoming(ThenV, ThenBB);
    PN->addIncoming(ElseV, ElseBB);

    // Return the phi node as the value computed by the if/then/else expression.
    // This means that if the condition was true we return the then value, otherwise
    // the we return the else value.
    return PN;
}


// ForExprAST
std::string ForExprAST::ToString() const {
    std::string res {};

    // TODO: non sto gestendo il fatto che lo Step è opzionale
    res += "for " + Start->ToString() + ", " + End->ToString() + ", " + Step->ToString() + " in\n\t";
    res += Body->ToString() + "\n";

    return res;
}

Value* ForExprAST::codegen() const {
    // Emit the Start exp first, without the loop variable in scope.
    // (Start è solo l'espressione a destra dell'uguale)
    Value *StartVal = Start->codegen();
    if (!StartVal)
        return LogErrorV("ForExprAST::codegen() | couldn't codegen the start expression");

    // Because we will need it to create the Phi node, we remember the block that falls through into the loop.
    BasicBlock *PreheaderBB = Builder->GetInsertBlock();
    // Make the new basic block for the loop header, inserting after current block.
    Function *TheFunction = Builder->GetInsertBlock()->getParent();
    BasicBlock *LoopBB = BasicBlock::Create(*TheContext, "loop", TheFunction); //append a basic block to TheFunction
    // Insert an explicit fall through from the current block to the LoopBB.
    Builder->CreateBr(LoopBB);

    // Start insertion in LoopBB.
    Builder->SetInsertPoint(LoopBB);

    // Start the PHI node for the loop induction variable with an entry for the Start expression.
    PHINode *Variable = Builder->CreatePHI(Type::getDoubleTy(*TheContext), 2, VarName);
    Variable->addIncoming(StartVal, PreheaderBB);

    // Within the loop, the variable is defined equal to the PHI node.  If it
    // shadows an existing variable, we have to restore it, so save it now.
    Value *OldVal = NamedValues[VarName]; // can be null if there is no outer shadowed variable
    NamedValues[VarName] = Variable; // registro la loop-induction variable nello scope

    // Emit the body of the loop. This, like any other expr, can change the
    // current BB (nested fors/ifs).
    // Note that we ignore the value computed by the body (the loop returns
    // zero), but don't allow an error.
    // Also, note that, the body can use the loop induction variable because
    // we've installed it above.
    if (!Body->codegen())
        return LogErrorV("ForExprAST::codegen() | couldn't codegen the body of the loop");

    // Emit the step value.
    Value *StepVal = nullptr;
    if (Step) {
        StepVal = Step->codegen();
        if (!StepVal)
            return LogErrorV("ForExprAST::codegen() | couldn't codegen the step expression");
    } else {
        // If not specified, use 1.0.
        StepVal = ConstantFP::get(*TheContext, APFloat(1.0));
    }

    // Emit the increment of the loop induction variable
    Value *NextVar = Builder->CreateFAdd(Variable, StepVal, "nextvar");

    // FIXME: this is wrong, the loop condition gets evaluated after the body
    // this means that the loop executes always at least one time and always
    // una volta di troppo
    // Emit the end condition.
    Value *EndCond = End->codegen();
    if (!EndCond)
        return LogErrorV("ForExprAST::codegen() | couldn't codegen the end condition");
    // Convert condition to a bool by comparing non-equal to 0.0.
    EndCond = Builder->CreateFCmpONE( EndCond, ConstantFP::get(*TheContext, APFloat(0.0)), "loopcond");

    // Create the "after loop" block and insert it.
    BasicBlock *LoopEndBB = Builder->GetInsertBlock(); // remember this for the phi node
    BasicBlock *AfterBB = BasicBlock::Create(*TheContext, "afterloop", TheFunction);
    // Insert the conditional branch into the end of LoopEndBB.
    Builder->CreateCondBr(EndCond, LoopBB, AfterBB);
    // Any new code will be inserted in AfterBB.
    Builder->SetInsertPoint(AfterBB);

    // Add a new entry to the PHI node for the backedge.
    Variable->addIncoming(NextVar, LoopEndBB);

    // Restore the unshadowed variable.
    if (OldVal)
        NamedValues[VarName] = OldVal;
    else
        NamedValues.erase(VarName);

    // for expr always returns 0.0.
    return Constant::getNullValue(Type::getDoubleTy(*TheContext));
}

// PrototypeAST
std::string PrototypeAST::ToString() const {
    std::string res {};

    res += Name;
    res += "(";
    for(const auto& arg : Args) {
        res += arg;
        res += " "; // i parametri formali non sono comma separated in kaleidoscope
    }
     // se ci sono argomenti sostituisco l'ultimo ',' spurio con ')'
    if(Args.size() > 0)
        res.at(res.size()-1) = ')';
    else
        res += ')';

    return res;
}

Function* PrototypeAST::codegen() const {
    std::vector<Type*> Doubles(Args.size(), Type::getDoubleTy(*TheContext));
    // create a function type that returns a double, accepts Args.size() doubles and that is not variadic
    FunctionType *FT = FunctionType::get(Type::getDoubleTy(*TheContext), Doubles, false);
    // create the function and insert it into TheModule
    Function *F = Function::Create(FT, Function::ExternalLinkage, Name, TheModule.get());

    // Set names for all arguments to make the IR more readable.
    unsigned Idx = 0;
    for (auto &Arg : F->args())
        Arg.setName(Args[Idx++]);

    return F;
}

// FunctionAST
std::string FunctionAST::ToString() const {
    return Proto->ToString() + "\n\t" + Body->ToString();
}

// FIXME: This code does have a bug: If the FunctionAST::codegen() method finds an existing IR Function,
// IT DOES NOT VALIDATE ITS SIGNATURE AGAINST THE DEFINITION’S OWN PROTOTYPE. This means that an earlier
// ‘extern’ declaration will take precedence over the function definition’s signature, which can cause
//  codegen to fail, for instance if the function arguments are named differently.
Function* FunctionAST::codegen() {
    // First, check for an existing function from a previous 'extern' declaration
    // inside any module symbol table. Se non c'è, generiamo il prototipo nel current
    // module (grazie a getFunction()).
    auto FuncName = Proto->getName();
    FunctionProtos[Proto->getName()] = std::move(Proto);
    Function *TheFunction = getFunction(FuncName);
    if (!TheFunction)
        return (Function*)LogErrorV("FunctionAST::codegen() | Function prototype could not be codegened.");

    // Create a new basic block to start insertion into.
    BasicBlock *BB = BasicBlock::Create(*TheContext, "entry", TheFunction);
    Builder->SetInsertPoint(BB);

    // Record the function arguments in the NamedValues map, so we can define
    // what's inside the function's scope. (the recorded arguments are accessed
    // by VariableExprAST::codegen())
    NamedValues.clear();
    for (auto &Arg : TheFunction->args())
        NamedValues[std::string(Arg.getName())] = &Arg;

    if (Value *RetVal = Body->codegen()) {
        // Finish off the function by inserting the return value.
        Builder->CreateRet(RetVal);
        // Validate the generated code, checking for consistency.
        verifyFunction(*TheFunction);
        // Optimize the newly generated function in-place.
        TheFPM->run(*TheFunction, *TheFAM);

        return TheFunction;
    }

    // Error reading body, remove function. This allows the user to redefine a function that
    // they incorrectly typed in before; if we didn’t delete it, it would live in the symbol
    // table of TheModule, with a body, preventing future redefinition.
    TheFunction->eraseFromParent();
    return (Function*)LogErrorV("FunctionAST::codegen() | Function body could not be codegened.");
}


/// LogError* - These are little helper functions for error handling.
std::unique_ptr<ExprAST> LogError(const char *Str) {
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
    LogError(Str);
    return nullptr;
}

Value* LogErrorV(const char *Str) {
    LogError(Str);
    return nullptr;
}
