#ifndef DOCUMENTTREE_HPP
#define DOCUMENTTREE_HPP

#include "documentTreeVisitor.hpp"
#include "position.hpp"

#include <algorithm>
#include <format>
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

class DocumentTreeNode
{
public:
    explicit DocumentTreeNode(Position position);
    Position getPosition() const;
    virtual void accept(DocumentTreeVisitor &visitor) = 0;
    virtual ~DocumentTreeNode() = default;
private:
    Position position;
};

struct Expression: public DocumentTreeNode
{
    using DocumentTreeNode::DocumentTreeNode;
};

struct Literal: public Expression
{
    explicit Literal(Position position, std::variant<std::wstring, int32_t, double, bool> value);
    std::variant<std::wstring, int32_t, double, bool> value;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct Variable: public Expression
{
    explicit Variable(Position position, std::wstring name);
    std::wstring name;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct BinaryOperation: public Expression
{
    explicit BinaryOperation(Position position, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
    std::unique_ptr<Expression> left, right;
};

struct OrExpression: public BinaryOperation
{
    using BinaryOperation::BinaryOperation;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct XorExpression: public BinaryOperation
{
    using BinaryOperation::BinaryOperation;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct AndExpression: public BinaryOperation
{
    using BinaryOperation::BinaryOperation;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct EqualExpression: public BinaryOperation
{
    using BinaryOperation::BinaryOperation;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct NotEqualExpression: public BinaryOperation
{
    using BinaryOperation::BinaryOperation;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct IdenticalExpression: public BinaryOperation
{
    using BinaryOperation::BinaryOperation;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct NotIdenticalExpression: public BinaryOperation
{
    using BinaryOperation::BinaryOperation;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct ConcatExpression: public BinaryOperation
{
    using BinaryOperation::BinaryOperation;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct StringMultiplyExpression: public BinaryOperation
{
    using BinaryOperation::BinaryOperation;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct GreaterExpression: public BinaryOperation
{
    using BinaryOperation::BinaryOperation;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct LesserExpression: public BinaryOperation
{
    using BinaryOperation::BinaryOperation;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct GreaterEqualExpression: public BinaryOperation
{
    using BinaryOperation::BinaryOperation;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct LesserEqualExpression: public BinaryOperation
{
    using BinaryOperation::BinaryOperation;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct PlusExpression: public BinaryOperation
{
    using BinaryOperation::BinaryOperation;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct MinusExpression: public BinaryOperation
{
    using BinaryOperation::BinaryOperation;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct MultiplyExpression: public BinaryOperation
{
    using BinaryOperation::BinaryOperation;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct DivideExpression: public BinaryOperation
{
    using BinaryOperation::BinaryOperation;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct FloorDivideExpression: public BinaryOperation
{
    using BinaryOperation::BinaryOperation;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct ModuloExpression: public BinaryOperation
{
    using BinaryOperation::BinaryOperation;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct ExponentExpression: public BinaryOperation
{
    using BinaryOperation::BinaryOperation;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct UnaryMinusExpression: public Expression
{
    explicit UnaryMinusExpression(Position position, std::unique_ptr<Expression> value);
    std::unique_ptr<Expression> value;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct NotExpression: public Expression
{
    explicit NotExpression(Position position, std::unique_ptr<Expression> value);
    std::unique_ptr<Expression> value;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct IsExpression: public Expression
{
    explicit IsExpression(Position begin, std::unique_ptr<Expression> left, std::wstring right);
    std::unique_ptr<Expression> left;
    std::wstring right;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct SubscriptExpression: public BinaryOperation
{
    using BinaryOperation::BinaryOperation;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct DotExpression: public Expression
{
    explicit DotExpression(Position position, std::unique_ptr<Expression> value, std::wstring field);
    std::unique_ptr<Expression> value;
    std::wstring field;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct StructExpression: public Expression
{
    explicit StructExpression(Position position, std::vector<std::unique_ptr<Expression>> arguments);
    std::vector<std::unique_ptr<Expression>> arguments;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct Instruction: public DocumentTreeNode
{
    using DocumentTreeNode::DocumentTreeNode;
};

struct VariableDeclaration: public DocumentTreeNode
{
    explicit VariableDeclaration(Position position, std::wstring type, std::wstring name, bool isMutable);
    std::wstring type, name;
    bool isMutable;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct VariableDeclStatement: public Instruction
{
    explicit VariableDeclStatement(
        Position position, VariableDeclaration declaration, std::unique_ptr<Expression> value
    );
    VariableDeclaration declaration;
    std::unique_ptr<Expression> value;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct Assignable: public DocumentTreeNode
{
    explicit Assignable(Position position, std::unique_ptr<Assignable> left, std::wstring right);
    explicit Assignable(Position position, std::wstring value);
    std::unique_ptr<Assignable> left;
    std::wstring right;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct AssignmentStatement: public Instruction
{
    explicit AssignmentStatement(Position position, Assignable left, std::unique_ptr<Expression> right);
    Assignable left;
    std::unique_ptr<Expression> right;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct FunctionCall: public Expression
{
    explicit FunctionCall(
        Position position, std::wstring functionName, std::vector<std::unique_ptr<Expression>> parameters
    );
    std::wstring functionName;
    std::vector<std::unique_ptr<Expression>> arguments;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct FunctionCallInstruction: public Instruction
{
    FunctionCallInstruction(Position position, FunctionCall functionCall);
    FunctionCall functionCall;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct ReturnStatement: public Instruction
{
    explicit ReturnStatement(Position position, std::unique_ptr<Expression> returnValue);
    std::unique_ptr<Expression> returnValue;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct ContinueStatement: public Instruction
{
    explicit ContinueStatement(Position position);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct BreakStatement: public Instruction
{
    explicit BreakStatement(Position position);
    void accept(DocumentTreeVisitor &visitor) override;
};

struct SingleIfCase: public DocumentTreeNode
{
    explicit SingleIfCase(
        Position position, std::variant<VariableDeclStatement, std::unique_ptr<Expression>> condition,
        std::vector<std::unique_ptr<Instruction>> body
    );
    std::variant<VariableDeclStatement, std::unique_ptr<Expression>> condition;
    std::vector<std::unique_ptr<Instruction>> body;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct IfStatement: public Instruction
{
    explicit IfStatement(
        Position position, std::vector<SingleIfCase> cases, std::vector<std::unique_ptr<Instruction>> elseCaseBody
    );
    std::vector<SingleIfCase> cases;
    std::vector<std::unique_ptr<Instruction>> elseCaseBody;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct WhileStatement: public Instruction
{
    explicit WhileStatement(
        Position position, std::unique_ptr<Expression> condition, std::vector<std::unique_ptr<Instruction>> body
    );
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Instruction>> body;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct DoWhileStatement: public Instruction
{
    explicit DoWhileStatement(
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
    friend std::wostream &operator<<(std::wostream &out, const FunctionIdentification &id);
};

template <>
struct std::formatter<FunctionIdentification, wchar_t>
{
    template <class ParseContext>
    constexpr auto parse(ParseContext &context)
    {
        if(context.begin() != context.end() && *context.begin() != L'}')
            throw std::format_error("FunctionIdentification does not take any format args.");
        return context.begin();
    }

    template <class FormatContext>
    auto format(const FunctionIdentification &id, FormatContext &context) const
    {
        std::format_to(context.out(), L"{}", id.name);
        if(id.parameterTypes.size() > 0)
        {
            std::format_to(context.out(), L"({}", id.parameterTypes[0]);
            std::transform(
                id.parameterTypes.begin() + 1, id.parameterTypes.end(), context.out(),
                [](const std::wstring &parameter) { return L", " + parameter; }
            );
            std::format_to(context.out(), L")");
        }
        return context.out();
    }
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
    explicit Field(Position position, std::wstring type, std::wstring name);
    std::wstring type, name;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct StructDeclaration: public DocumentTreeNode
{
    explicit StructDeclaration(Position position, std::vector<Field> fields);
    std::vector<Field> fields;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct VariantDeclaration: public DocumentTreeNode
{
    explicit VariantDeclaration(Position position, std::vector<Field> fields);
    std::vector<Field> fields;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct FunctionDeclaration: public DocumentTreeNode
{
    explicit FunctionDeclaration(
        Position position, std::wstring source, std::vector<VariableDeclaration> parameters,
        std::optional<std::wstring> returnType, std::vector<std::unique_ptr<Instruction>> body
    );
    std::wstring source;
    std::vector<VariableDeclaration> parameters;
    std::optional<std::wstring> returnType;
    std::vector<std::unique_ptr<Instruction>> body;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct IncludeStatement: public DocumentTreeNode
{
    explicit IncludeStatement(Position position, std::wstring filePath);
    std::wstring filePath;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct Program: public DocumentTreeNode
{
    explicit Program(Position position);
    std::vector<IncludeStatement> includes;
    std::unordered_map<std::wstring, StructDeclaration> structs;
    std::unordered_map<std::wstring, VariantDeclaration> variants;
    std::unordered_map<FunctionIdentification, FunctionDeclaration> functions;
    void accept(DocumentTreeVisitor &visitor) override;
    void add(std::pair<std::wstring, StructDeclaration> structBuilt);
    void add(std::pair<std::wstring, VariantDeclaration> variantBuilt);
    void add(std::pair<FunctionIdentification, FunctionDeclaration> functionBuilt);
};

#endif
