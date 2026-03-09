#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cassert>
#include <map>

#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>

using namespace llvm;


/// This holds the precedence for each binary operator that is defined.
extern std::map<char, int> BinopPrecedence;

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

    const std::string& getName() const { return Name; }
    std::string ToString() const override;
    Value* codegen() const override;
};

/// UnaryExprAST - Expression class for a unary operator.
class UnaryExprAST : public ExprAST {
private:
    char Opcode;
    std::unique_ptr<ExprAST> Operand;

public:
    UnaryExprAST(char Opcode, std::unique_ptr<ExprAST> Operand)
        : Opcode(Opcode), Operand(std::move(Operand)) {}

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

/// IfExprAST - Expression class for if/then/else.
class IfExprAST : public ExprAST {
private:
    std::unique_ptr<ExprAST> Cond;
    std::unique_ptr<ExprAST> Then;
    std::unique_ptr<ExprAST> Else;

public:
    IfExprAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<ExprAST> Then, std::unique_ptr<ExprAST> Else)
        : Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {}

    std::string ToString() const override;
    Value* codegen() const override;
};

/// ForExprAST - Expression class for for/in.
class ForExprAST : public ExprAST {
private:
    std::string VarName;
    std::unique_ptr<ExprAST> Start;
    std::unique_ptr<ExprAST> End;
    std::unique_ptr<ExprAST> Step;
    std::unique_ptr<ExprAST> Body;

public:
    ForExprAST(const std::string &VarName, std::unique_ptr<ExprAST> Start,
                std::unique_ptr<ExprAST> End, std::unique_ptr<ExprAST> Step,
                std::unique_ptr<ExprAST> Body)
        : VarName(VarName), Start(std::move(Start)), End(std::move(End)),
        Step(std::move(Step)), Body(std::move(Body)) {}

    std::string ToString() const override;
    Value *codegen() const override;
};

/// VarExprAST - Expression class for var/in
/// var/in allows a list of names to be defined all at once, and each
/// name can optionally have an initializer value. Also, var/in has a
/// body, this body is allowed to access the variables defined by the
/// var/in.
///
/// var a = 1, b = 1, c in
/// (for i = 3, i < x in
///     c = a + b :
///     a = b :
///     b = c) :
/// b;
class VarExprAST : public ExprAST {
private:
    std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> VarNames;
    std::unique_ptr<ExprAST> Body;

public:
    VarExprAST(std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> VarNames, std::unique_ptr<ExprAST> Body)
        : VarNames(std::move(VarNames)), Body(std::move(Body)) {}

    std::string ToString() const override;
    Value *codegen() const override;
};

/// PrototypeAST - This class represents the "prototype" for a function, which captures
/// its name, and its argument names (thus the number of arguments the function takes),
/// as well as if its an operator.
/// In Kaleidoscope, functions are typed with just a count of their arguments since
/// the only supported data type is double.
class PrototypeAST
{
private:
    std::string Name;
    std::vector<std::string> Args;
    bool IsOperator;
    unsigned Precedence;  // Precedence is used only if a binary op.

public:
    PrototypeAST(const std::string &Name, std::vector<std::string> Args, bool IsOperator = false, unsigned Prec = 0)
        : Name(Name), Args(std::move(Args)), IsOperator(IsOperator), Precedence(Prec) {}

    const std::string &getName() const { return Name; }

    bool isUnaryOp() const { return IsOperator && Args.size() == 1; }
    bool isBinaryOp() const { return IsOperator && Args.size() == 2; }

    char getOperatorName() const {
        assert(isUnaryOp() || isBinaryOp());
        return Name[Name.size() - 1];
    }
    unsigned getBinaryPrecedence() const { return Precedence; }

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
    Function* codegen(); // non const dato che il nodo proto subisce una move
};

/// LogError* - These are little helper functions for error handling.
/// they make it easier to handle errors in routines that have various
/// return types. They always return a nullptr
std::unique_ptr<ExprAST> LogError(const char *Str);
std::unique_ptr<PrototypeAST> LogErrorP(const char *Str);
Value* LogErrorV(const char *Str);
