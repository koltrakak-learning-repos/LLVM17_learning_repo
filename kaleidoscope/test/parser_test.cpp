#include <cstdio>
#include <iostream>

#include "lexer.h"
#include "ast.h"
#include "parser.h"

// libraries i have to include because of the state sotto
// altrimenti linker error
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/PassManager.h>

// global state used for codegen (i don't use in this test, but
// i have to include it anyway, otherwise i get a linker error)
std::unique_ptr<LLVMContext> TheContext;
std::unique_ptr<Module> TheModule;
std::unique_ptr<IRBuilder<>> Builder;
std::map<std::string, Value*> NamedValues;
std::unique_ptr<FunctionPassManager> TheFPM;
std::unique_ptr<FunctionAnalysisManager> TheFAM;
std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;

void HandleDefinition(FILE* InputFile) {
    if ( const auto node=ParseDefinition(InputFile) ) {
        fprintf(stderr, "Parsed a function definition.\n");
        std::cout << node->ToString() << "\n";
    } else {
        // Skip token for error recovery.
        fprintf(stderr, "skipping problematic token: %c\n", CurTok);
        getNextToken(InputFile);
    }
}

void HandleExtern(FILE* InputFile) {
    if ( const auto node=ParseExtern(InputFile) ) {
        fprintf(stderr, "Parsed an extern\n");
        std::cout << node->ToString() << "\n";
    } else {
        // Skip token for error recovery.
        fprintf(stderr, "skipping problematic token: %c\n", CurTok);
        getNextToken(InputFile);
    }
}

void HandleTopLevelExpression(FILE* InputFile) {
    // Evaluate a top-level expression into an anonymous function.
    if ( const auto node=ParseTopLevelExpr(InputFile) ) {
        fprintf(stderr, "Parsed a top-level expr\n");
        std::cout << node->ToString() << "\n";
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
void MainLoop(FILE* InputFile) {
    while (true) {
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
    if (argc > 1) {
        std::cout << "reading program from: " << argv[1] << "\n";
        InputFile = fopen(argv[1], "r");
        if (InputFile == NULL) {
            printf("Errore apertura %s\n", argv[1]);
            std::exit(127);
        }
    } else {
        std::cout << "specify a kaleidoscope file to parse\n";
        std::exit(127);
    }

    // Prime the first token.
    getNextToken(InputFile);

    // Run the main "interpreter loop" now.
    MainLoop(InputFile);

    fclose(InputFile);
    return 0;
}
