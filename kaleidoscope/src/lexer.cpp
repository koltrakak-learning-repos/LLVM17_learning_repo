#include <string>
#include <iostream>

// The lexer returns tokens [0-255] (ascii value) if it is an unknown character (e.g. operators),
// otherwise one of these for known things.
enum Token {
    tok_eof = -1,
    // FIXME: i should probably have something like token_error to deal with nonsensical stuff
    // (9.23.45 non deve essere tokenizzato come 9.24 e 0.45)
    // The tutorial just doesn't care.

    // commands
    tok_def = -2,
    tok_extern = -3,

    // primary
    tok_identifier = -4,
    tok_number = -5,
};

// If the current token is an identifier, the IdentifierStr global variable holds the name of the identifier.
// (IdentifierStr also saves keyword names)
// If the current token is a numeric literal (like 1.0), NumVal holds its value.
static std::string IdentifierStr;
static double NumVal;

// gettok - Return the next token from standard input.
// Each token returned by our lexer will either be one of the Token enum values or
// it will be an ‘unknown’ character like ‘+’, which is returned as its ASCII value
static int gettok() {
    static int LastChar = ' ';  // last character read, but not processed

    // Skip any whitespace.
    while (isspace(LastChar))
        LastChar = getchar();

    // identifier: [a-zA-Z][a-zA-Z0-9]*
    if (isalpha(LastChar)) {
        IdentifierStr = LastChar;

        while (isalnum((LastChar = getchar())))
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
            LastChar = getchar();

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
            LastChar = getchar();
        while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

        if (LastChar != EOF)
            return gettok(); // return token after comment
    }

    // Check for end of file.  Don't eat the EOF.
    if (LastChar == EOF)
        return tok_eof;

    // Otherwise, just return the character as its ascii value.
    // (probably an operator character like ‘+’)
    // TODO: i should probably add specific token types for these
    int ThisChar = LastChar;
    LastChar = getchar(); // spostiamo il cursore per la prossima chiamata (LastChar è static)
    return ThisChar;
}

int main() {
    int cur_tok;

    while((cur_tok=gettok()) != tok_eof) {
        switch (cur_tok) {
            case tok_def:
                std::cout << "keyword def\n";
                break;

            case tok_extern:
                std::cout << "keyword extern\n";
                break;

            case tok_identifier:
                std::cout << "Identifier: " << IdentifierStr << "\n";
                break;

            case tok_number:
                std::cout << "Number: " << NumVal << "\n";
                break;
            // operator case
            default:
                std::cout << static_cast<char>(cur_tok) << " (probably an operator)" << "\n";
        }
    }
}
