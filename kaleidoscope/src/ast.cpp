#include <cstdio>
#include <string>

#include "ast.h"

// Inserisci qua i metodi non banali delle classi

std::string NumberExprAST::ToString() const {
    return std::to_string(Val);
}

std::string VariableExprAST::ToString() const {
    return Name;
}

std::string BinaryExprAST::ToString() const {
    std::string LeftString = LHS->ToString();
    std::string RightString = RHS->ToString();

    return "(" + LeftString + Op + RightString + ")";
}

std::string CallExprAST::ToString() const {
    std::string res {};

    res += Callee;
    res += "(";
    for(const auto& arg : Args) {
        res += arg->ToString();
        res += ",";
    }
    // se ci sono argomenti sostituisco l'ultimo ',' spurio con ')'
    if(Args.size() > 0)
        res.at(res.size()-1) = ')';
    else
        res += ')';

    return res;
}

std::string PrototypeAST::ToString() const {
    std::string res {};

    res += Name;
    res += "(";
    for(const auto& arg : Args) {
        res += arg;
        res += " "; // i parametri formali non sono comma separated in kaleidoscope
    }
     // se ci sono argomenti sostituisco l'ultimo ',' spurio con ')'
    if(Args.size() > 0)
        res.at(res.size()-1) = ')';
    else
        res += ')';

    return res;
}

std::string FunctionAST::ToString() const {
    return Proto->ToString() + "\n\t" + Body->ToString();
}

/// LogError* - These are little helper functions for error handling.
std::unique_ptr<ExprAST> LogError(const char *Str) {
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
    LogError(Str);
    return nullptr;
}
