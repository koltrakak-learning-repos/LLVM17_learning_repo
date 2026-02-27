#pragma once

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

// gettok - Return the next token from already opened InputFile.
// Each token returned by our lexer will either be one of the Token enum values or
// it will be an ‘unknown’ character like ‘+’, which is returned as its ASCII value
int gettok(FILE* InputFile);
