#include <cstdio>
#include <string>
#include <iostream>

#include "lexer.h"

// If the current token is an identifier, the IdentifierStr global variable holds the name of the identifier.
// (IdentifierStr also saves keyword names)
// If the current token is a numeric literal (like 1.0), NumVal holds its value.
static std::string IdentifierStr;
static double NumVal;

// gettok - Return the next token from standard input.
// Each token returned by our lexer will either be one of the Token enum values or
// it will be an ‘unknown’ character like ‘+’, which is returned as its ASCII value
int gettok(FILE* InputFile) {
    static int LastChar = ' ';  // last character read, but not processed

    // Skip any whitespace.
    while (isspace(LastChar))
        LastChar = fgetc(InputFile);

    // identifier: [a-zA-Z][a-zA-Z0-9]*
    if (isalpha(LastChar)) {
        IdentifierStr = LastChar;

        while ( isalnum(LastChar=fgetc(InputFile)) )
            IdentifierStr += LastChar;

        // keyword
        if (IdentifierStr == "def")
            return tok_def;
        if (IdentifierStr == "extern")
            return tok_extern;

        return tok_identifier;
    }

    // Number: [0-9.]+
    if (isdigit(LastChar) || LastChar == '.') {
        std::string NumStr {};
        // FIXME:
        // bool DecimalPointFound {false};

        do {
            NumStr += LastChar;
            LastChar = fgetc(InputFile);

            // FIXME: se faccio così 8.24.56 mi restituisce due token.
            // Piuttosto dovrei gestire l'errore consumando fino alla fine della parola
            // e restituendo un token_error
            //
            // check for multiple decimal points in a number literal (es: 8.23.154)
            // if (LastChar == '.' && !DecimalPointFound) {
            //     DecimalPointFound = true;
            // } else if (LastChar == '.' && DecimalPointFound) {
            //     break;
            // }
        } while (isdigit(LastChar) || LastChar == '.');

        NumVal = strtod(NumStr.c_str(), 0);
        return tok_number;
    }

    // Comments
    if (LastChar == '#') {
        // skip until end of line.
        do
            LastChar = fgetc(InputFile);
        while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

        if (LastChar != EOF)
            return gettok(InputFile); // return token after comment
    }

    // Check for end of file.  Don't eat the EOF.
    if (LastChar == EOF)
        return tok_eof;

    // Otherwise, just return the character as its ascii value.
    // (probably an operator character like ‘+’)
    // TODO: i should probably add specific token types for these
    int ThisChar = LastChar;
    LastChar = fgetc(InputFile); // spostiamo il cursore per la prossima chiamata (LastChar è static)
    return ThisChar;
}

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
