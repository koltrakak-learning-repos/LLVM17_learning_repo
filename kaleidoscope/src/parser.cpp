#include <cstdio>
#include <iostream>

#include "lexer.h"
#include "parser.h"


/// This holds the precedence for each binary operator that is defined.
// 1 is lowest precedence.
std::map<char, int> BinopPrecedence = {
    {'<', 10},
    {'+', 20},
    {'-', 20},
    {'*', 40},
};

/// Get the precedence of the pending binary operator token.
int GetTokPrecedence() {
    if (!isascii(CurTok))
        return -1;

    // Make sure it's a declared binop.
    int TokPrec = BinopPrecedence[CurTok];
    if (TokPrec <= 0)
        return -1;

    return TokPrec;
}


/// numberexpr ::= number
std::unique_ptr<ExprAST> ParseNumberExpr(FILE* InputFile) {
    auto Result = std::make_unique<NumberExprAST>(NumVal);
    getNextToken(InputFile); // consume the number

    return std::move(Result);
}

/// parenexpr ::= '(' expression ')'
std::unique_ptr<ExprAST> ParseParenExpr(FILE* InputFile) {
    getNextToken(InputFile); // eat (.
    auto V = ParseExpression(InputFile);
    if (!V)
        return LogError("ParseParenExpr | couldn't parse the expression inside the parenthesis");

    if (CurTok != ')')
        return LogError("ParseParenExpr | expected ')'");
    getNextToken(InputFile); // eat ).

    return V;
}

/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'
std::unique_ptr<ExprAST> ParseIdentifierExpr(FILE* InputFile) {
    std::string IdName = IdentifierStr;
    getNextToken(InputFile);  // eat identifier.

    if (CurTok != '(') // Simple variable ref.
        return std::make_unique<VariableExprAST>(IdName);

    // Call.
    getNextToken(InputFile);  // eat (
    std::vector<std::unique_ptr<ExprAST>> Args;
    if (CurTok != ')') {
        while (true) {
            if (auto Arg = ParseExpression(InputFile))
                Args.push_back(std::move(Arg));
            else
                return LogError("ParseIdentifierExpr | couldn't parse an argument of the call expression");

            if (CurTok == ')')
                break;

            if (CurTok != ',')
                return LogError("ParseIdentifierExpr | Expected ')' or ',' in argument list");

            getNextToken(InputFile); // eat ,
        }
    }
    // Eat the ')'.
    getNextToken(InputFile);

    return std::make_unique<CallExprAST>(IdName, std::move(Args));
}

/// ifexpr
///   ::= 'if' expression 'then' expression 'else' expression
std::unique_ptr<ExprAST> ParseIfExpr(FILE* InputFile) {
    getNextToken(InputFile);  // eat the if.

    // condition.
    auto Cond = ParseExpression(InputFile);
    if (!Cond)
        return LogError("ParseIfExpr | couldn't parse the if condition expression");

    if (CurTok != tok_then)
        return LogError("ParseIfExpr | expected 'then'");
    getNextToken(InputFile);  // eat the then

    auto Then = ParseExpression(InputFile);
    if (!Then)
        return LogError("ParseIfExpr | couldn't parse the then expression");

    if (CurTok != tok_else)
        return LogError("ParseIfExpr | expected 'else'");

    getNextToken(InputFile);

    auto Else = ParseExpression(InputFile);
    if (!Else)
        return LogError("ParseIfExpr | couldn't parse the else expression");

    return std::make_unique<IfExprAST>(std::move(Cond), std::move(Then), std::move(Else));
}


/// forexpr
///   ::= 'for' identifier '=' expr ',' expr (',' expr)? 'in' expr
std::unique_ptr<ExprAST> ParseForExpr(FILE* InputFile) {
    // TODO: i should fix the error messages, adesso non ho voglia

    getNextToken(InputFile);  // eat the for.

    if (CurTok != tok_identifier)
        return LogError("expected identifier after for");

    std::string IdName = IdentifierStr;
    getNextToken(InputFile);  // eat identifier.

    if (CurTok != '=')
        return LogError("expected '=' after for");
    getNextToken(InputFile);  // eat '='.

    auto Start = ParseExpression(InputFile);
    if (!Start)
        return nullptr;
    if (CurTok != ',')
        return LogError("expected ',' after for start value");
    getNextToken(InputFile); // eat first ','

    auto End = ParseExpression(InputFile);
    if (!End)
        return nullptr;

    // The step value is optional.
    std::unique_ptr<ExprAST> Step;
    if (CurTok == ',') {
        getNextToken(InputFile); // eat second ','
        Step = ParseExpression(InputFile);
        if (!Step)
            return nullptr;
    }

    if (CurTok != tok_in)
        return LogError("expected 'in' after for");
    getNextToken(InputFile);  // eat 'in'.

    auto Body = ParseExpression(InputFile);
    if (!Body)
        return nullptr;

    return std::make_unique<ForExprAST>(IdName, std::move(Start), std::move(End), std::move(Step), std::move(Body));
}

/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
///   ::= ifexpr
std::unique_ptr<ExprAST> ParsePrimary(FILE* InputFile) {
    switch (CurTok) {
    default:
        return LogError("ParsePrimary | unknown token when expecting a primary expression");
    case tok_identifier:
        return ParseIdentifierExpr(InputFile);
    case tok_number:
        return ParseNumberExpr(InputFile);
    case '(':
        return ParseParenExpr(InputFile);
    case tok_if:
        return ParseIfExpr(InputFile);
    case tok_for:
        return ParseForExpr(InputFile);
    }
}

/// expression
///   ::= primary binoprhs
std::unique_ptr<ExprAST> ParseExpression(FILE* InputFile) {
    auto LHS = ParsePrimary(InputFile);
    if (!LHS)
        return LogError("ParseExpression | couldn't parse a primary expression");

    return ParseBinOpRHS(0, std::move(LHS), InputFile);
}

/// binoprhs
///   ::= ('+' primary)*
std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS, FILE* InputFile) {
    while (true) {
        // If this is a binop, find its precedence.
        // ritorna -1 per token che non sono binop
        int TokPrec = GetTokPrecedence();
        // If this is a binop that binds at least as tightly as the current binop,
        // consume it, otherwise we are done parsing the rhs and we can return the
        // expression accumulated in LHS.
        if (TokPrec < ExprPrec)
            return LHS;

        // Okay, we know this is a binop. We can build the [binOp, rhs] pair
        int BinOp = CurTok;
        getNextToken(InputFile);  // eat binop
        // Parse the primary expression after the binary operator.
        auto RHS = ParsePrimary(InputFile);
        if (!RHS)
            return LogError("ParseBinOpRHS | couldn't parse primary expression");

        // We parsed the left-hand side of an expression and one pair of the RHS sequence,
        // we have to decide which way the expression associates. We could have
        // (a cur_binop b) next_binop unparsed” or “a cur_binop (b next_binop unparsed).
        // -> We just lookup the precedences!
        // If the current BinOp binds less tightly with RHS than the operator after
        // RHS, let the pending operator take RHS as its LHS. That is, the next operator
        // has higher precedence than the current operator.
        int NextPrec = GetTokPrecedence();
        if (TokPrec < NextPrec) {
            // the next operator has higher precedence, so the current expression associates
            // to the right. The rhs of the current operator is the entire expression composed
            // of binops with higher precedence, thus, TokPrec+1 is the new minimum precedence
            // allowed for ParseBinOpRHS to continue parsing.
            RHS = ParseBinOpRHS(TokPrec+1, std::move(RHS), InputFile);
            if (!RHS)
                return LogError("ParseBinOpRHS | couldn't parse rhs");
        }

        // Merge LHS/RHS.
        LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
    }
}

/// prototype
///   ::= id '(' id* ')'
std::unique_ptr<PrototypeAST> ParsePrototype(FILE* InputFile) {
    if (CurTok != tok_identifier)
        return LogErrorP("ParsePrototype | Expected function name in prototype");
    std::string FnName = IdentifierStr;
    getNextToken(InputFile); // eat identifier

    if (CurTok != '(')
        return LogErrorP("ParseProtytype | Expected '(' in prototype");

    // Read the list of argument names (whitespace separated).
    std::vector<std::string> ArgNames;
    while (getNextToken(InputFile) == tok_identifier)
        ArgNames.push_back(IdentifierStr);

    // TODO: vedi se vuoi aggiustare qua per avere le virgole anche nei prototipi
    // delle funzioni e non solo nelle chiamate
    // while (true) {
    //     if (CurTok == ')')
    //         break;

    //     if (CurTok != ',')
    //         return LogError("ParseIdentifierExpr | Expected ')' or ',' in argument list");

    //     getNextToken(InputFile); // eat ,
    // }

    if (CurTok != ')')
        return LogErrorP("ParsePrototype | Expected ')' in prototype");

    // success.
    getNextToken(InputFile);  // eat ')'.

    return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}

/// definition
///   ::= 'def' prototype expression
std::unique_ptr<FunctionAST> ParseDefinition(FILE* InputFile) {
    getNextToken(InputFile);  // eat def.

    auto Proto = ParsePrototype(InputFile);
    if (!Proto) {
        std::cout << "ParseDefinition | couldn't parse the function prototype\n";
        return nullptr;
    }

    auto Expr = ParseExpression(InputFile);
    if (!Expr) {
        std::cout << "ParseDefinition | couldn't parse the function body\n";
        return nullptr;
    }

    return std::make_unique<FunctionAST>(std::move(Proto), std::move(Expr));
}

/// external
///   ::= 'extern' prototype
std::unique_ptr<PrototypeAST> ParseExtern(FILE* InputFile) {
    getNextToken(InputFile);  // eat extern.
    return ParsePrototype(InputFile);
}

/// toplevelexpr
///   ::= expression
std::unique_ptr<FunctionAST> ParseTopLevelExpr(FILE* InputFile) {
    if (auto E = ParseExpression(InputFile)) {
        // Make an anonymous proto.
        auto Proto = std::make_unique<PrototypeAST>("__anon_expr", std::vector<std::string>());
        return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    }

    std::cout << "ParseTopLevelExpr | couldn't parse the expression\n";
    return nullptr;
}
