#include <string>

#include "lexer.h"

/// CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
/// token the parser is looking at.  getNextToken reads another token from the
/// lexer and updates CurTok with its results.
int CurTok;
int getNextToken(FILE* InputFile) {
  return CurTok = gettok(InputFile);
}

// If the current token is an identifier, the IdentifierStr global variable holds the name of the identifier.
// (IdentifierStr also saves keyword names)
// If the current token is a numeric literal (like 1.0), NumVal holds its value.
std::string IdentifierStr;
double NumVal;

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
        if (IdentifierStr == "if")
            return tok_if;
        if (IdentifierStr == "then")
            return tok_then;
        if (IdentifierStr == "else")
            return tok_else;
        if (IdentifierStr == "for")
            return tok_for;
        if (IdentifierStr == "in")
            return tok_in;
        if (IdentifierStr == "binary")
            return tok_binary;
        if (IdentifierStr == "unary")
            return tok_unary;

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
