#include <cstdio>
#include <string>
#include <map>

#include "ast.h"


/// ---------------------------------------------
/// Inserisci qua i metodi non banali delle classi
/// ----------------------------------------------

// NumberExpreAST
std::string NumberExprAST::ToString() const {
    return std::to_string(Val);
}

Value *NumberExprAST::codegen() const {
    // In the LLVM IR, numeric constants are represented with the ConstantFP class,
    // which holds the numeric value in an APFloat (ArbitraryPrecision) internally.
    // This code basically just creates and returns a ConstantFP.
    return ConstantFP::get(*TheContext, APFloat(Val));
}

// VariableExprAST
std::string VariableExprAST::ToString() const {
    return Name;
}

Value *VariableExprAST::codegen() const {
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

Value *BinaryExprAST::codegen() const {
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

Value *CallExprAST::codegen() const {
    // Look up the name in the global module table.
    Function *CalleeF = TheModule->getFunction(Callee);
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

std::string FunctionAST::ToString() const {
    return Proto->ToString() + "\n\t" + Body->ToString();
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

Value *LogErrorV(const char *Str) {
    LogError(Str);
    return nullptr;
}
