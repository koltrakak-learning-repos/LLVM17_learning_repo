#include <string>
#include <iostream>

// The lexer returns tokens [0-255] (ascii value) if it is an unknown character (e.g. operators),
// otherwise one of these for known things.
enum Token {
    tok_eof = -1,

    // commands
    tok_def = -2,
    tok_extern = -3,

    // primary
    tok_identifier = -4,
    tok_number = -5,
};

static std::string IdentifierStr; // Filled in if tok_identifier
static double NumVal;             // Filled in if tok_number

/// gettok - Return the next token from standard input.
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
        std::string NumStr;
        do {
            NumStr += LastChar;
            LastChar = getchar();
        } while (isdigit(LastChar) || LastChar == '.'); // FIXME: it will incorrectly read “1.23.45.67” and handle it as if you typed in “1.23”. 

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
    int ThisChar = LastChar;
    LastChar = getchar(); // spostiamo il cursore per la prossima chiamata
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
                std::cout << IdentifierStr << "\n";
                break;
            
            case tok_number:
                std::cout << NumVal << "\n";
                break;

            default:
                std::cout << "come sei finito qua?\n";

        }
    }
}