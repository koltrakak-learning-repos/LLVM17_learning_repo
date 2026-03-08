#include <cstdio>
#include <iostream>
#include <map>
#include <utility>

#include "lexer.h"
#include "ast.h"
#include "parser.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>

#include "KaleidoscopeJIT.h"
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar/Reassociate.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>

using namespace llvm;
using namespace llvm::orc;

enum class Mode {
    CLI,
    FILE
};

// global state used for codegen
std::unique_ptr<LLVMContext> TheContext;
std::unique_ptr<Module> TheModule;
std::unique_ptr<IRBuilder<>> Builder;
std::map<std::string, Value*> NamedValues;

// This JIT's API are very simple:
// - addModule() adds an LLVM IR module to the JIT, making its functions
//   available for execution (with its memory managed by a ResourceTracker)
// - lookup() allows us to look up pointers to the compiled code.
std::unique_ptr<KaleidoscopeJIT> TheJIT;

std::unique_ptr<FunctionPassManager> TheFPM;
std::unique_ptr<LoopAnalysisManager> TheLAM;
std::unique_ptr<FunctionAnalysisManager> TheFAM;
std::unique_ptr<CGSCCAnalysisManager> TheCGAM;
std::unique_ptr<ModuleAnalysisManager> TheMAM;
std::unique_ptr<PassInstrumentationCallbacks> ThePIC;
std::unique_ptr<StandardInstrumentations> TheSI;
// Map that holds the most recent prototype for each function.
std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;

ExitOnError ExitOnErr;

// ----- builtin -----
// siccome il jit risolve i simboli utilizzando anche il dynamic linker
// (vedi commento sotto). Possiamo estendere kaleidoscope aggiungendo
// delle funzioni "builtin" al binario stesso, che verranno trovate dal
// jit quando vedrà che non sono visibili in alcun Module aggiunto a
// quest'ultimo.
// (extern "C" serve ad evitare name mangling che avverrebbe con la
//  definizione di funzioni c++ dato che possono essere overloaded)
//
// ATTENZIONE: siccome KaleidocopeJIT utilizza dlsym() (linker dinamico)
// per risolvere i simboli, se si compila normalmente il progetto, i builtin
// non verrebbero trovati in quanto sono simboli interni del binario e non
// appartenenti a shared library. Per rendere visibili i builtin bisogna
// compilare con il flag '-rdynamic' anche i simboli interni vengono aggiunti
// nella tabella dei simboli dinamici diventando visibili

/// putchard - putchar that takes a double and returns 0.
extern "C" double putchard(double X) {
    fputc((char)X, stderr);
    return 0;
}

/// printd - printf that takes a double prints it as "%f\n", returning 0.
extern "C" double printd(double X) {
    fprintf(stderr, "%f\n", X);
    return 0;
}
// -------------------



void InitializeModuleAndManagers() {
    // Open a new context and module.
    TheContext = std::make_unique<LLVMContext>();
    TheModule = std::make_unique<Module>("KaleidoscopeJIT", *TheContext);
    TheModule->setDataLayout(TheJIT->getDataLayout());

    // Create a new builder for the module.
    Builder = std::make_unique<IRBuilder<>>(*TheContext);


    // Create new pass and analysis managers.
    TheFPM = std::make_unique<FunctionPassManager>();
    TheLAM = std::make_unique<LoopAnalysisManager>();
    TheFAM = std::make_unique<FunctionAnalysisManager>();
    TheCGAM = std::make_unique<CGSCCAnalysisManager>();
    TheMAM = std::make_unique<ModuleAnalysisManager>();
    ThePIC = std::make_unique<PassInstrumentationCallbacks>();
    TheSI = std::make_unique<StandardInstrumentations>(*TheContext, /*DebugLogging*/ true);
    TheSI->registerCallbacks(*ThePIC, TheMAM.get());

    // Add transform passes.
    // Do simple "peephole" optimizations and bit-twiddling optzns.
    TheFPM->addPass(InstCombinePass());
    // Reassociate expressions.
    TheFPM->addPass(ReassociatePass());
    // Eliminate Common SubExpressions.
    TheFPM->addPass(GVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    TheFPM->addPass(SimplifyCFGPass());

    // Register analysis passes used in these transform passes.
    PassBuilder PB;
    PB.registerModuleAnalyses(*TheMAM);
    PB.registerFunctionAnalyses(*TheFAM);
    PB.crossRegisterProxies(*TheLAM, *TheFAM, *TheCGAM, *TheMAM);
}


void HandleDefinition(FILE* InputFile) {
    if ( const auto FnAST = ParseDefinition(InputFile) ) {
        if ( const auto* FnIR = FnAST->codegen() ) { // using auto* instead of auto specifies that we must return a raw pointer
            fprintf(stderr, "Read function definition\n");
            FnIR->print(errs());

            // each function lives in its own module so we can
            // delete top-level expressions safely
            auto TSM = ThreadSafeModule(std::move(TheModule), std::move(TheContext));
            ExitOnErr( TheJIT->addModule(std::move(TSM)) );
            InitializeModuleAndManagers();
        }
    } else {
        // Skip token for error recovery.
        fprintf(stderr, "skipping problematic token: %c\n", CurTok);
        getNextToken(InputFile);
    }
}

void HandleExtern(FILE* InputFile) {
    if ( auto ProtoAST = ParseExtern(InputFile) ) {
        if ( auto *FnIR = ProtoAST->codegen() ) {
            fprintf(stderr, "Read extern\n");
            FnIR->print(errs());

            // salvo il nodo del prototipo in FunctionProtos in
            // maniera tale che anche moduli successivi abbiano
            // visibilità di dichiarazioni passate
            FunctionProtos[ProtoAST->getName()] = std::move(ProtoAST);
        }
    } else {
        // Skip token for error recovery.
        fprintf(stderr, "skipping problematic token: %c\n", CurTok);
        getNextToken(InputFile);
    }
}

void HandleTopLevelExpression(FILE* InputFile) {
    // Evaluate a top-level expression into an anonymous function.
    if ( const auto FnAST = ParseTopLevelExpr(InputFile) ) {
        // No need to get explicitely the codegened IR; the functions (even __anon_expr())
        // are saved inside TheModule
        if (auto *FnIR = FnAST->codegen()) {
            fprintf(stderr, "Read top-level expr\n");
            FnIR->print(errs());

            // Create a ResourceTracker to track JIT'd memory allocated to our
            // anonymous expression -- that way we can free it after executing.
            auto RT = TheJIT->getMainJITDylib().createResourceTracker();

            auto TSM = ThreadSafeModule(std::move(TheModule), std::move(TheContext));
            // here we add to the JIT the current module, triggering compilation of
            // its functions to native code that we can execute.
            ExitOnErr(TheJIT->addModule(std::move(TSM), RT));

            // let's get a fresh new module for later functions
            InitializeModuleAndManagers();

            // Search the JIT for the __anon_expr symbol.
            auto ExprSymbol = ExitOnErr(TheJIT->lookup("__anon_expr"));
            // Get the symbol's address and cast it to the right type (takes no
            // arguments, returns a double) so we can call it as a native function.
            // NB: The JIT will resolve function calls across module boundaries, as
            // long as each of the functions called has a prototype (so the call can
            // be codegened), and is added to the JIT before it is called. This even
            // works for stdlib functions (like cos()) that are linked with this
            // frontend.
            // The KaleidoscopJIT work like this when resolving symbols:
            // - First it searches all the modules that have already been added to the
            //   JIT, from the most recent to the oldest, to find the newest definition
            // - If no definition is found inside the JIT, it falls back to calling
            //   “dlsym("sin")” on the Kaleidoscope process itself
            double (*FP)() = ExprSymbol.toPtr<double (*)()>();
            fprintf(stderr, "Evaluated to %f\n", FP());

            // Delete the anonymous expression module from the JIT.
            // This frees the associated memory.
            ExitOnErr(RT->remove());
        }
    } else {
        // Skip token for error recovery.
        fprintf(stderr, "skipping problematic token: %c\n", CurTok);
        getNextToken(InputFile);
    }
}

/// top
///   ::= definition
///   ::= external
///   ::= expression
///   ::= ';'
void MainLoop(FILE* InputFile, Mode mode) {
    while (true) {
        if (mode == Mode::CLI)
            fprintf(stderr, "ready> ");

        switch (CurTok) {
        case tok_eof:
            return;

        // NB: top-level semicolons are ignored, they just
        // serve as expression separators. Without a separator
        // the parser would keep parsing until the EOF or until
        // it encounters an unknowns token for the current rule.
        // ';' is an unkown token that is used to stop parsing,
        // this way, an expression on the CLI can be evaluated
        // immediately, instead of requiring to insert something
        // unrelated in the token stream.
        case ';':
            getNextToken(InputFile); // eat ';'
            break;

         case tok_def:
            HandleDefinition(InputFile);
            std::cout << "\n";
            break;

        case tok_extern:
            HandleExtern(InputFile);
            std::cout << "\n";
            break;

        default:
            HandleTopLevelExpression(InputFile);
            std::cout << "\n";
            break;
        }
    }
}

int main(int argc, char** argv) {

    FILE* InputFile;
    Mode mode {};
    if (argc > 1) {
        mode = Mode::FILE;
        std::cout << "reading program from: " << argv[1] << "\n";
        InputFile = fopen(argv[1], "r");
        if (InputFile == NULL) {
            printf("Errore apertura %s\n", argv[1]);
            std::exit(127);
        }
    } else {
        mode = Mode::CLI;
        std::cout << "reading program from CLI\n";
        InputFile = stdin;
    }

    // Prepare the environment to create code for the current
    // native target and declare and initialize the JIT.
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();
    TheJIT = ExitOnErr(KaleidoscopeJIT::Create());

    // Make the module, which holds all the code.
    InitializeModuleAndManagers();

    // Prime the first token.
    if (mode == Mode::CLI)
        fprintf(stderr, "ready> ");
    getNextToken(InputFile);

    // Run the main "interpreter loop" now.
    MainLoop(InputFile, mode);

    // Print out all of the generated code.
    TheModule->print(errs(), nullptr);

    fclose(InputFile);
    return 0;
}
