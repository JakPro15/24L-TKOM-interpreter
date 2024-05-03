#ifndef DOCUMENTTREE_HPP
#define DOCUMENTTREE_HPP

#include "documentTreeVisitor.hpp"
#include "position.hpp"

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

class DocumentTreeNode
{
public:
    DocumentTreeNode(Position position);
    Position getPosition() const;
    virtual void accept(DocumentTreeVisitor &visitor) = 0;
    virtual ~DocumentTreeNode() = default;
private:
    Position position;
};

struct Expression: public virtual DocumentTreeNode
{
    Expression();
};

struct Literal: public Expression
{
    Literal(Position position, std::variant<std::wstring, int32_t, double, bool> value);
    std::variant<std::wstring, int32_t, double, bool> value;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct Variable: public Expression
{
    Variable(Position position, std::wstring name);
    std::wstring name;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct BinaryOperation: public Expression
{
    BinaryOperation(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    std::unique_ptr<Expression> left, right;
};

struct OrExpression: public BinaryOperation
{
    OrExpression(Position begin, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct XorExpression: public BinaryOperation
{
    XorExpression(Position begin, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct AndExpression: public BinaryOperation
{
    AndExpression(Position begin, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct EqualExpression: public BinaryOperation
{
    EqualExpression(Position begin, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct NotEqualExpression: public BinaryOperation
{
    NotEqualExpression(Position begin, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct IdenticalExpression: public BinaryOperation
{
    IdenticalExpression(Position begin, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct NotIdenticalExpression: public BinaryOperation
{
    NotIdenticalExpression(Position begin, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct ConcatExpression: public BinaryOperation
{
    ConcatExpression(Position begin, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct StringMultiplyExpression: public BinaryOperation
{
    StringMultiplyExpression(Position begin, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct GreaterExpression: public BinaryOperation
{
    GreaterExpression(Position begin, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct LesserExpression: public BinaryOperation
{
    LesserExpression(Position begin, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct GreaterEqualExpression: public BinaryOperation
{
    GreaterEqualExpression(Position begin, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct LesserEqualExpression: public BinaryOperation
{
    LesserEqualExpression(Position begin, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct PlusExpression: public BinaryOperation
{
    PlusExpression(Position begin, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct MinusExpression: public BinaryOperation
{
    MinusExpression(Position begin, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct MultiplyExpression: public BinaryOperation
{
    MultiplyExpression(Position begin, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct DivideExpression: public BinaryOperation
{
    DivideExpression(Position begin, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct FloorDivideExpression: public BinaryOperation
{
    FloorDivideExpression(Position begin, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct ModuloExpression: public BinaryOperation
{
    ModuloExpression(Position begin, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct ExponentExpression: public BinaryOperation
{
    ExponentExpression(Position begin, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct UnaryMinusExpression: public Expression
{
    UnaryMinusExpression(Position position, std::unique_ptr<Expression> value);
    std::unique_ptr<Expression> value;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct NotExpression: public Expression
{
    NotExpression(Position position, std::unique_ptr<Expression> value);
    std::unique_ptr<Expression> value;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct IsExpression: public Expression
{
    IsExpression(Position begin, std::unique_ptr<Expression> left, std::wstring right);
    std::unique_ptr<Expression> left;
    std::wstring right;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct SubscriptExpression: public BinaryOperation
{
    SubscriptExpression(Position begin, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct DotExpression: public Expression
{
    DotExpression(Position position, std::unique_ptr<Expression> value, std::wstring field);
    std::unique_ptr<Expression> value;
    std::wstring field;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct StructExpression: public Expression
{
    StructExpression(Position position, std::vector<std::unique_ptr<Expression>> arguments);
    std::vector<std::unique_ptr<Expression>> arguments;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct Instruction: public virtual DocumentTreeNode
{
    Instruction();
};

struct VariableDeclaration: public DocumentTreeNode
{
    VariableDeclaration(Position position, std::wstring type, std::wstring name, bool isMutable);
    std::wstring type, name;
    bool isMutable;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct VariableDeclStatement: public Instruction
{
    VariableDeclStatement(Position position, VariableDeclaration declaration, std::unique_ptr<Expression> value);
    VariableDeclaration declaration;
    std::unique_ptr<Expression> value;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct Assignable: public DocumentTreeNode
{
    Assignable(Position position, std::unique_ptr<Assignable> left, std::wstring right);
    Assignable(Position position, std::wstring value);
    std::unique_ptr<Assignable> left;
    std::wstring right;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct AssignmentStatement: public Instruction
{
    AssignmentStatement(Position position, Assignable left, std::unique_ptr<Expression> right);
    Assignable left;
    std::unique_ptr<Expression> right;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct FunctionCall: public Instruction,
                     public Expression
{
    FunctionCall(Position position, std::wstring functionName, std::vector<std::unique_ptr<Expression>> parameters);
    std::wstring functionName;
    std::vector<std::unique_ptr<Expression>> arguments;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct ReturnStatement: public Instruction
{
    ReturnStatement(Position position, std::unique_ptr<Expression> returnValue);
    std::unique_ptr<Expression> returnValue;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct ContinueStatement: public Instruction
{
    ContinueStatement(Position position);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct BreakStatement: public Instruction
{
    BreakStatement(Position position);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct SingleIfCase: public DocumentTreeNode
{
    SingleIfCase(
        Position position, std::variant<VariableDeclStatement, std::unique_ptr<Expression>> condition,
        std::vector<std::unique_ptr<Instruction>> body
    );
    std::variant<VariableDeclStatement, std::unique_ptr<Expression>> condition;
    std::vector<std::unique_ptr<Instruction>> body;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct IfStatement: public Instruction
{
    IfStatement(
        Position position, std::vector<SingleIfCase> cases, std::vector<std::unique_ptr<Instruction>> elseCaseBody
    );
    std::vector<SingleIfCase> cases;
    std::vector<std::unique_ptr<Instruction>> elseCaseBody;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct WhileStatement: public Instruction
{
    WhileStatement(
        Position position, std::unique_ptr<Expression> condition, std::vector<std::unique_ptr<Instruction>> body
    );
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Instruction>> body;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct DoWhileStatement: public Instruction
{
    DoWhileStatement(
        Position position, std::unique_ptr<Expression> condition, std::vector<std::unique_ptr<Instruction>> body
    );
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Instruction>> body;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct FunctionIdentification
{
    FunctionIdentification(std::wstring name, std::vector<std::wstring> parameterTypes);
    std::wstring name;
    std::vector<std::wstring> parameterTypes;
    bool operator==(const FunctionIdentification &other) const = default;
};

template <>
struct std::hash<FunctionIdentification>
{
    std::size_t operator()(const FunctionIdentification &id) const
    {
        std::size_t value = std::hash<std::wstring>()(id.name);
        for(const std::wstring &type: id.parameterTypes)
        {
            value <<= 1;
            value ^= std::hash<std::wstring>()(type);
        }
        return value;
    }
};

struct Field: public DocumentTreeNode
{
    Field(Position position, std::wstring type, std::wstring name);
    std::wstring type, name;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct StructDeclaration: public DocumentTreeNode
{
    StructDeclaration(Position position, std::vector<Field> fields);
    std::vector<Field> fields;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct VariantDeclaration: public DocumentTreeNode
{
    VariantDeclaration(Position position, std::vector<Field> fields);
    std::vector<Field> fields;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct FunctionDeclaration: public DocumentTreeNode
{
    FunctionDeclaration(
        Position position, std::vector<VariableDeclaration> parameters, std::optional<std::wstring> returnType,
        std::vector<std::unique_ptr<Instruction>> body
    );
    std::vector<VariableDeclaration> parameters;
    std::optional<std::wstring> returnType;
    std::vector<std::unique_ptr<Instruction>> body;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct IncludeStatement: public DocumentTreeNode
{
    IncludeStatement(Position position, std::wstring filePath);
    std::wstring filePath;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct Program: public DocumentTreeNode
{
    Program(Position position);
    std::unordered_map<FunctionIdentification, FunctionDeclaration> functions;
    std::vector<IncludeStatement> includes;
    std::unordered_map<std::wstring, StructDeclaration> structs;
    std::unordered_map<std::wstring, VariantDeclaration> variants;
    void accept(DocumentTreeVisitor &visitor) override;
};

#endif
