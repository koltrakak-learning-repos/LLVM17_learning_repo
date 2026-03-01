#include <cstdio>
#include <iostream>

#include "lexer.h"
#include "parser.h"

void HandleDefinition(FILE* InputFile) {
    if ( const auto node=ParseDefinition(InputFile) ) {
        fprintf(stderr, "Parsed a function definition.\n");
        std::cout << node->ToString() << "\n";
    } else {
        // Skip token for error recovery.
        getNextToken(InputFile);
    }
}

void HandleExtern(FILE* InputFile) {
    if ( const auto node=ParseExtern(InputFile) ) {
        fprintf(stderr, "Parsed an extern\n");
        std::cout << node->ToString()<< "\n";
    } else {
        // Skip token for error recovery.
        getNextToken(InputFile);
    }
}

void HandleTopLevelExpression(FILE* InputFile) {
    // Evaluate a top-level expression into an anonymous function.
    if ( const auto node=ParseTopLevelExpr(InputFile) ) {
        fprintf(stderr, "Parsed a top-level expr\n");
        std::cout << node->ToString()<< "\n";
    } else {
        // Skip token for error recovery.
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
        // FIXME: aggiusta per quando stai parsando un file
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
            break;

        case tok_extern:
            HandleExtern(InputFile);
            break;

        default:
            HandleTopLevelExpression(InputFile);
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
        std::cout << "reading program from CLI\n";
        InputFile = stdin;
    }

    // Prime the first token.
    fprintf(stderr, "ready> ");
    getNextToken(InputFile);

    // Run the main "interpreter loop" now.
    MainLoop(InputFile);

    fclose(InputFile);
    return 0;
}
