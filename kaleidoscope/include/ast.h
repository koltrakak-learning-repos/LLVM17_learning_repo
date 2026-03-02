#pragma once

#include <string>
#include <vector>
#include <memory>

#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>

using namespace llvm;

/// ExprAST - Base class for all expression nodes.
class ExprAST
{
public:
    virtual ~ExprAST() = default;

    virtual std::string ToString() const = 0;
    // codegen() emits LLVM IR for the AST node recursively.
    // “Value” is the class used to represent an SSA register in LLVM
    virtual Value* codegen() const = 0;
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST
{
private:
    double Val;

public:
    NumberExprAST(double Val) : Val(Val) {}

    std::string ToString() const override;
    Value* codegen() const override;
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST
{
private:
    std::string Name;

public:
    VariableExprAST(const std::string &Name) : Name(Name) {}

    std::string ToString() const override;
    Value* codegen() const override;
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST
{
private:
    char Op;
    std::unique_ptr<ExprAST> LHS, RHS;

public:
    BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS)
        : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

    std::string ToString() const override;
    Value* codegen() const override;
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST
{
private:
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;

public:
    CallExprAST(const std::string &Callee, std::vector<std::unique_ptr<ExprAST>> Args)
        : Callee(Callee), Args(std::move(Args)) {}

    std::string ToString() const override;
    Value* codegen() const override;
};

/// PrototypeAST - This class represents the "prototype" for a function, which captures
/// its name, and its argument names (thus the number of arguments the function takes).
/// In Kaleidoscope, functions are typed with just a count of their arguments since all
/// values are doubles.
class PrototypeAST
{
private:
    std::string Name;
    std::vector<std::string> Args;

public:
    PrototypeAST(const std::string &Name, std::vector<std::string> Args)
        : Name(Name), Args(std::move(Args)) {}

    const std::string &getName() const { return Name; }
    std::string ToString() const;
    Function* codegen() const;
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST
{
private:
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<ExprAST> Body;

public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<ExprAST> Body)
        : Proto(std::move(Proto)), Body(std::move(Body)) {}

    std::string ToString() const;
    Function* codegen() const;
};

/// LogError* - These are little helper functions for error handling.
/// they make it easier to handle errors in routines that have various
/// return types. They always return a nullptr
std::unique_ptr<ExprAST> LogError(const char *Str);
std::unique_ptr<PrototypeAST> LogErrorP(const char *Str);
Value* LogErrorV(const char *Str);
