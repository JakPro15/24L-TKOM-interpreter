#include "iVisitable.hpp"

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#define DECLARE_ACCEPT                      \
    void accept(IVisitor &visitor) override \
    {                                       \
        visitor.visit(*this);               \
    }

struct Expression: public IVisitable
{};

struct Literal: public Expression
{
    std::variant<std::wstring, int32_t, double, bool> value;
    DECLARE_ACCEPT
};

struct IsExpression: public Expression
{
    std::unique_ptr<Expression> left, right;
    DECLARE_ACCEPT
};

struct OrExpression: public Expression
{
    std::unique_ptr<Expression> left, right;
    DECLARE_ACCEPT
};

struct XorExpression: public Expression
{
    std::unique_ptr<Expression> left, right;
    DECLARE_ACCEPT
};

struct AndExpression: public Expression
{
    std::unique_ptr<Expression> left, right;
    DECLARE_ACCEPT
};

struct EqualExpression: public Expression
{
    std::unique_ptr<Expression> left, right;
    DECLARE_ACCEPT
};

struct NotEqualExpression: public Expression
{
    std::unique_ptr<Expression> left, right;
    DECLARE_ACCEPT
};

struct IdenticalExpression: public Expression
{
    std::unique_ptr<Expression> left, right;
    DECLARE_ACCEPT
};

struct NotIdenticalExpression: public Expression
{
    std::unique_ptr<Expression> left, right;
    DECLARE_ACCEPT
};

struct ConcatExpression: public Expression
{
    std::unique_ptr<Expression> left, right;
    DECLARE_ACCEPT
};

struct StringMultiplyExpression: public Expression
{
    std::unique_ptr<Expression> left, right;
    DECLARE_ACCEPT
};

struct GreaterExpression: public Expression
{
    std::unique_ptr<Expression> left, right;
    DECLARE_ACCEPT
};

struct LesserExpression: public Expression
{
    std::unique_ptr<Expression> left, right;
    DECLARE_ACCEPT
};

struct GreaterEqualExpression: public Expression
{
    std::unique_ptr<Expression> left, right;
    DECLARE_ACCEPT
};

struct LesserEqualExpression: public Expression
{
    std::unique_ptr<Expression> left, right;
    DECLARE_ACCEPT
};

struct PlusExpression: public Expression
{
    std::unique_ptr<Expression> left, right;
    DECLARE_ACCEPT
};

struct MinusExpression: public Expression
{
    std::unique_ptr<Expression> left, right;
    DECLARE_ACCEPT
};

struct MultiplyExpression: public Expression
{
    std::unique_ptr<Expression> left, right;
    DECLARE_ACCEPT
};

struct DivideExpression: public Expression
{
    std::unique_ptr<Expression> left, right;
    DECLARE_ACCEPT
};

struct FloorDivideExpression: public Expression
{
    std::unique_ptr<Expression> left, right;
    DECLARE_ACCEPT
};

struct ModuloExpression: public Expression
{
    std::unique_ptr<Expression> left, right;
    DECLARE_ACCEPT
};

struct ExponentExpression: public Expression
{
    std::unique_ptr<Expression> left, right;
    DECLARE_ACCEPT
};

struct UnaryMinusExpression: public Expression
{
    std::unique_ptr<Expression> value;
    DECLARE_ACCEPT
};

struct NotExpression: public Expression
{
    std::unique_ptr<Expression> value;
    DECLARE_ACCEPT
};

struct SubscriptExpression: public Expression
{
    std::unique_ptr<Expression> subscripted, index;
    DECLARE_ACCEPT
};

struct DotExpression: public Expression
{
    std::unique_ptr<Expression> value;
    std::wstring field;
    DECLARE_ACCEPT
};

struct StructExpression: public Expression
{
    std::vector<std::unique_ptr<Expression>> arguments;
    DECLARE_ACCEPT
};

struct Instruction: public IVisitable
{};

struct VariableDeclaration: public Instruction
{
    std::wstring type, name;
    std::unique_ptr<Expression> value;
    DECLARE_ACCEPT
};

struct Assignable: public IVisitable
{
    std::unique_ptr<Assignable> left;
    std::wstring right;
    DECLARE_ACCEPT
};

struct AssignmentStatement: public Instruction
{
    Assignable left;
    std::unique_ptr<Expression> right;
    DECLARE_ACCEPT
};

struct FunctionCall: public IVisitable
{
    std::wstring functionName;
    std::vector<std::unique_ptr<Expression>> parameters;
    DECLARE_ACCEPT
};

struct ReturnStatement: public IVisitable
{
    std::unique_ptr<Expression> returnValue;
    DECLARE_ACCEPT
};

struct ContinueStatement: public IVisitable
{};

struct BreakStatement: public IVisitable
{};

struct IfStatement: public IVisitable
{
    std::variant<VariableDeclaration, std::unique_ptr<Expression>> condition;
    std::vector<Instruction> body;
    DECLARE_ACCEPT
};

struct WhileStatement: public IVisitable
{
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Instruction>> body;
    DECLARE_ACCEPT
};

struct DoWhileStatement: public IVisitable
{
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Instruction>> body;
    DECLARE_ACCEPT
};

struct FunctionIdentification: public IVisitable
{
    std::wstring name;
    std::vector<std::wstring> parameterTypes;
    DECLARE_ACCEPT
};

struct Field: public IVisitable
{
    std::wstring type, name;
    DECLARE_ACCEPT
};

struct FunctionDeclaration: public IVisitable
{
    std::wstring name;
    std::vector<Field> parameters;
    std::wstring returnType;
    std::vector<std::unique_ptr<Instruction>> body;
    DECLARE_ACCEPT
};

struct IncludeStatement: public IVisitable
{
    std::wstring filePath;
    DECLARE_ACCEPT
};

struct Program: public IVisitable
{
    std::unordered_map<FunctionIdentification, FunctionDeclaration> functions;
    std::vector<IncludeStatement> includes;
    std::unordered_map<std::wstring, std::vector<Field>> structs;
    std::unordered_map<std::wstring, std::vector<Field>> variants;
    DECLARE_ACCEPT
};
