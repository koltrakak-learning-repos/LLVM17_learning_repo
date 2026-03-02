#include <cstdio>
#include <iostream>
#include <map>

#include "lexer.h"
#include "ast.h"
#include "parser.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>

using namespace llvm;

enum class Mode {
    CLI,
    FILE
};

// global state used for codegen
std::unique_ptr<LLVMContext> TheContext;
std::unique_ptr<Module> TheModule;
std::unique_ptr<IRBuilder<>> Builder;
std::map<std::string, Value*> NamedValues;

static void InitializeModule() {
    // Open a new context and module.
    TheContext = std::make_unique<LLVMContext>();
    TheModule = std::make_unique<Module>("my cool jit", *TheContext);
    // Create a new builder for the module.
    Builder = std::make_unique<IRBuilder<>>(*TheContext);
}


void HandleDefinition(FILE* InputFile) {
    if ( const auto FnAST = ParseDefinition(InputFile) ) {
        if ( const auto* FnIR = FnAST->codegen() ) { // using auto* instead of auto specifies that we must return a raw pointer
            fprintf(stderr, "Read function definition\n");
            FnIR->print(errs());
        }
    } else {
        // Skip token for error recovery.
        fprintf(stderr, "skipping problematic token: %c\n", CurTok);
        getNextToken(InputFile);
    }
}

void HandleExtern(FILE* InputFile) {
    if ( const auto ProtoAST = ParseExtern(InputFile) ) {
        if ( auto *FnIR = ProtoAST->codegen() ) {
            fprintf(stderr, "Read extern\n");
            FnIR->print(errs());
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
        if (auto *FnIR = FnAST->codegen()) {
            fprintf(stderr, "Read top-level expr\n");
            FnIR->print(errs());
            // Remove the anonymous function so we can redefine it.
            FnIR->eraseFromParent();
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

    // Prime the first token.
    if (mode == Mode::CLI)
        fprintf(stderr, "ready> ");
    getNextToken(InputFile);

    // Make the module, which holds all the code.
    InitializeModule();

    // Run the main "interpreter loop" now.
    MainLoop(InputFile, mode);

    // Print out all of the generated code.
    TheModule->print(errs(), nullptr);

    fclose(InputFile);
    return 0;
}
