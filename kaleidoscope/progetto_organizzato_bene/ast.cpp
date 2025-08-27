#include <memory>
#include <string>
#include <vector>

/// ExprAST - Base class for all expression nodes.
class ExprAST {
public:
    virtual ~ExprAST() = default;
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
    double Val;

public:
    NumberExprAST(double Val) : Val(Val) {} // dopo i due punti si ha una lista di inizializzazione
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
    std::string Name;

public:
    VariableExprAST(const std::string &Name) : Name(Name) {}
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
    char Op;
    // Un nodo BinaryExprAst è responsabile della vita dei suoi figli.
    // Quando viene distrutto, anche i figli devono essere distrutti.
    // Usano unique_ptr i sottoalberi vengono distrutti quando il padre 
    // viene distrutto (gli smart pointer fanno ref counting)
    std::unique_ptr<ExprAST> LHS, RHS;

    // unique_ptr possono avere un solo proprietario,
    // move() permette di trasferire la ownership al 
    // nodo BinaryExprAST (padre).
    // Gli unique_ptr passati al costruttore vengono
    // annullati
    // 
    // move annulla il puntatore alla memoria dell'oggetto,
    // e restituisce un puntatore alla stessa memoria in maniera
    // da "spostarla". 
public:
    BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,
                    std::unique_ptr<ExprAST> RHS)
        : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {} 
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
    std::string Callee; // function name
    std::vector<std::unique_ptr<ExprAST>> Args;

public:
    CallExprAST(const std::string &Callee,
                std::vector<std::unique_ptr<ExprAST>> Args)
        : Callee(Callee), Args(std::move(Args)) {}
};


/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class PrototypeAST {
    std::string Name;
    std::vector<std::string> Args;

public:
    PrototypeAST(const std::string &Name, std::vector<std::string> Args)
        : Name(Name), Args(std::move(Args)) {}

    const std::string &getName() const { return Name; }
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST {
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<ExprAST> Body; // FIXME: il body è un unica espressione?

public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto,
                std::unique_ptr<ExprAST> Body) 
        : Proto(std::move(Proto)), Body(std::move(Body)) {}
};


int main() {
    auto LHS = std::make_unique<VariableExprAST>("x");
    auto RHS = std::make_unique<VariableExprAST>("y");
    auto Result = std::make_unique<BinaryExprAST>('+', std::move(LHS), std::move(RHS));
}