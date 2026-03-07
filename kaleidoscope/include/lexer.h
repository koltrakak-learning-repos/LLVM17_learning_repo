#pragma once

#include <cstdio>

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

    // control
    tok_if = -6,
    tok_then = -7,
    tok_else = -8,
    tok_for = -9,
    tok_in = -10,
};

/// CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
/// token the parser is looking at.  getNextToken reads another token from the
/// lexer and updates CurTok with its results.
extern int CurTok;
int getNextToken(FILE* InputFile);


// If the current token is an identifier, the IdentifierStr global variable holds the name of the identifier.
// (IdentifierStr also saves keyword names)
// If the current token is a numeric literal (like 1.0), NumVal holds its value.
extern std::string IdentifierStr;
extern double NumVal;


// gettok - Return the next token from already opened InputFile.
// Each token returned by our lexer will either be one of the Token enum values or
// it will be an ‘unknown’ character like ‘+’, which is returned as its ASCII value
int gettok(FILE* InputFile);
