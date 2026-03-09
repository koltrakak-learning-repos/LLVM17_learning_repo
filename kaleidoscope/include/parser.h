#pragma once

#include <memory>
#include <map>
#include <cstdio>

#include "ast.h"


/// This holds the precedence for each binary operator that is defined.
extern std::map<char, int> BinopPrecedence;

/// Get the precedence of the pending binary operator token.
/// Returns -1 if the token is not a binary operator
int GetTokPrecedence();

/// All the routines of the parser expect to be called when the current token is the
/// first one of one of their productions.
/// All the routines of the parser loads the lexer buffer with the next token before
/// they return.

/// numberexpr ::= number
std::unique_ptr<ExprAST> ParseNumberExpr(FILE* InputFile);

/// parenexpr ::= '(' expression ')'
std::unique_ptr<ExprAST> ParseParenExpr(FILE* InputFile);

/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'
std::unique_ptr<ExprAST> ParseIdentifierExpr(FILE* InputFile);

/// ifexpr
///   ::= 'if' expression 'then' expression 'else' expression
std::unique_ptr<ExprAST> ParseIfExpr(FILE* InputFile);


/// forexpr
///   ::= 'for' identifier '=' expr ',' expr (',' expr)? 'in' expr
std::unique_ptr<ExprAST> ParseForExpr(FILE* InputFile);

/// varexpr
///   ::= 'var' identifier ('=' expression)?(',' identifier ('=' expression)?)* 'in' expression
std::unique_ptr<ExprAST> ParseVarExpr(FILE* InputFile);

/// Helper function that parses a "primary expression", that is an
/// expression that can be one of the operands of a binary operator
///
/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
///   ::= ifexpr
///   ::= forexpr
///   ::= varexpr
std::unique_ptr<ExprAST> ParsePrimary(FILE* InputFile);


/// an expression is a primary expression potentially followed by a
/// sequence of [binop, primaryexpr] pairs
///
/// expression
///   ::= unary binoprhs
std::unique_ptr<ExprAST> ParseExpression(FILE* InputFile);

/// unary
///   ::= primary
///   ::= '!' unary
std::unique_ptr<ExprAST> ParseUnary(FILE* InputFile);

/// ParseBinOpRHS is the function that parses the sequence of pairs.
/// It takes a precedence and a pointer to an expression for the part
/// that has been parsed so far (Note that an expression may note even
/// have a sequence of pairs (x is a valid expression), as such “binoprhs”
///  is allowed to be empty, in which case it returns the expression that
/// is passed into it).
///
/// The precedence value passed into ParseBinOpRHS indicates the minimal
/// operator precedence that the function is allowed to eat. In other
/// words, this function parses expressions with higher precedence than the
/// specified power (makes sense, the returned expression is supposed to be
/// the rhs of the current lower-precedence operator).
///
/// binoprhs
///   ::= ('+' unary)*
std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS, FILE* InputFile);

/// used both for ‘extern’ function declarations as well as function
/// body definitions (def)
///
/// prototype
///   ::= id '(' id* ')'
///   ::= binary LETTER number? (id, id)
///   ::= unary LETTER (id)
std::unique_ptr<PrototypeAST> ParsePrototype(FILE* InputFile);

/// definition
///   ::= 'def' prototype expression
std::unique_ptr<FunctionAST> ParseDefinition(FILE* InputFile);

/// external
///   ::= 'extern' prototype
std::unique_ptr<PrototypeAST> ParseExtern(FILE* InputFile);

/// Handle top level expressions by defining anonymous zero argument functions
///
/// toplevelexpr
///   ::= expression
std::unique_ptr<FunctionAST> ParseTopLevelExpr(FILE* InputFile);
