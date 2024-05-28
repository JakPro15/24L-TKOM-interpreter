#ifndef DOCUMENTTREE_HPP
#define DOCUMENTTREE_HPP

#include "documentTreeVisitor.hpp"
#include "object.hpp"
#include "position.hpp"
#include "type.hpp"

#include <algorithm>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>

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
    Type getType() const;
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
    explicit IsExpression(Position begin, std::unique_ptr<Expression> left, Type right);
    std::unique_ptr<Expression> left;
    Type right;
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

struct CastExpression: public Expression
{
    explicit CastExpression(Position position, std::unique_ptr<Expression> value, Type targetType);
    std::unique_ptr<Expression> value;
    Type targetType;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct StructExpression: public Expression
{
    explicit StructExpression(
        Position position, std::vector<std::unique_ptr<Expression>> arguments,
        std::optional<std::wstring> structType = std::nullopt
    );
    std::vector<std::unique_ptr<Expression>> arguments;
    std::optional<std::wstring> structType;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct Instruction: public DocumentTreeNode
{
    using DocumentTreeNode::DocumentTreeNode;
};

struct VariableDeclaration: public DocumentTreeNode
{
    explicit VariableDeclaration(Position position, Type type, std::wstring name, bool isMutable);
    Type type;
    std::wstring name;
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
    FunctionIdentification(std::wstring name, std::vector<Type> parameterTypes);
    std::wstring name;
    std::vector<Type> parameterTypes;
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
        if(!id.parameterTypes.empty())
        {
            std::format_to(context.out(), L"({}", id.parameterTypes[0]);
            std::for_each(id.parameterTypes.begin() + 1, id.parameterTypes.end(), [&](const Type &parameter) {
                std::format_to(context.out(), L", {}", parameter);
            });
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
        for(const Type &type: id.parameterTypes)
        {
            value <<= 1;
            value ^= std::hash<Type>()(type);
        }
        return value;
    }
};

struct Field: public DocumentTreeNode
{
    explicit Field(Position position, Type type, std::wstring name);
    Type type;
    std::wstring name;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct StructDeclaration: public DocumentTreeNode
{
    explicit StructDeclaration(Position position, std::wstring source, std::vector<Field> fields);
    std::wstring getSource() const;
    std::vector<Field> fields;
    void accept(DocumentTreeVisitor &visitor) override;
private:
    std::wstring source;
};

struct VariantDeclaration: public DocumentTreeNode
{
    explicit VariantDeclaration(Position position, std::wstring source, std::vector<Field> fields);
    std::wstring getSource() const;
    std::vector<Field> fields;
    void accept(DocumentTreeVisitor &visitor) override;
private:
    std::wstring source;
};

struct BaseFunctionDeclaration: public DocumentTreeNode
{
    explicit BaseFunctionDeclaration(
        Position position, std::wstring source, std::vector<VariableDeclaration> parameters,
        std::optional<Type> returnType
    );
    std::wstring getSource() const;
    std::vector<VariableDeclaration> parameters;
    std::optional<Type> returnType;
private:
    std::wstring source;
};

struct FunctionDeclaration: public BaseFunctionDeclaration
{
    explicit FunctionDeclaration(
        Position position, std::wstring source, std::vector<VariableDeclaration> parameters,
        std::optional<Type> returnType, std::vector<std::unique_ptr<Instruction>> body
    );
    std::vector<std::unique_ptr<Instruction>> body;
    void accept(DocumentTreeVisitor &visitor) override;
};

struct BuiltinFunctionDeclaration: public BaseFunctionDeclaration
{
    using Body = std::function<std::optional<
        Object>(Position, const std::wstring &, std::vector<std::variant<Object, std::reference_wrapper<Object>>> &)>;
    explicit BuiltinFunctionDeclaration(
        Position position, std::wstring source, std::vector<VariableDeclaration> parameters,
        std::optional<Type> returnType, Body body
    );
    Body body;
    void accept(DocumentTreeVisitor &visitor) override;
private:
    std::wstring source;
};

template <typename... Types>
Object &getObject(std::variant<Types...> &variant)
{
    if(std::holds_alternative<std::reference_wrapper<Object>>(variant))
        return std::get<std::reference_wrapper<Object>>(variant).get();
    else
        return std::get<Object>(variant);
}

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
    std::unordered_map<FunctionIdentification, std::unique_ptr<BaseFunctionDeclaration>> functions;
    void accept(DocumentTreeVisitor &visitor) override;
    void add(std::pair<std::wstring, StructDeclaration> structBuilt);
    void add(std::pair<std::wstring, VariantDeclaration> variantBuilt);
    void add(std::pair<FunctionIdentification, FunctionDeclaration> functionBuilt);
    void add(std::pair<FunctionIdentification, BuiltinFunctionDeclaration> builtinFunction);
};

#endif
