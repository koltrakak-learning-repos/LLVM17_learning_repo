#include <iostream>

#include "lexer.h"

// Uncomment se vuoi testare il lexer
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

    int cur_tok;
    while((cur_tok=gettok(InputFile)) != tok_eof) {
        switch (cur_tok) {
            case tok_def:
                std::cout << "def\t - keyword\n";
                break;

            case tok_extern:
                std::cout << "extern\t - keyword\n";
                break;

            case tok_if:
                std::cout << "if\t - keyword\n";
                break;

            case tok_then:
                std::cout << "then\t - keyword\n";
                break;

            case tok_else:
                std::cout << "else\t - keyword\n";
                break;

            case tok_identifier:
                std::cout << IdentifierStr << "\t - identifier\n";
                break;

            case tok_number:
                std::cout << NumVal << "\t - number\n";
                break;

            // operator/parenthesis case (most of the time)
            default:
                std::cout << static_cast<char>(cur_tok) << "\t - (probably) an operator or parenthesis\n";
        }
    }

    fclose(InputFile);
}
