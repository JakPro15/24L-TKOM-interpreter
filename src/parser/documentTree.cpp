#include "documentTree.hpp"

#include "parserExceptions.hpp"

#include <iterator>

bool Type::isBuiltin() const
{
    return std::holds_alternative<Type::Builtin>(value);
}

std::wstring toString(Type::Builtin type)
{
    using enum Type::Builtin;
    switch(type)
    {
    case INT:
        return L"int";
    case FLOAT:
        return L"float";
    case STR:
        return L"str";
    case BOOL:
        return L"bool";
    default:
        return L"unknown";
    }
}

std::wstring toString(const std::wstring &type)
{
    return type;
}

std::wostream &operator<<(std::wostream &out, Type type)
{
    std::ostream_iterator<wchar_t, wchar_t> outIterator(out);
    std::format_to(outIterator, L"{}", type);
    return out;
}

DocumentTreeNode::DocumentTreeNode(Position position): position(position) {}

Position DocumentTreeNode::getPosition() const
{
    return position;
}

Literal::Literal(Position position, std::variant<std::wstring, int32_t, double, bool> value):
    Expression(position), value(value)
{}

Type Literal::getType() const
{
    using enum Type::Builtin;
    if(std::holds_alternative<int32_t>(value))
        return {INT};
    else if(std::holds_alternative<double>(value))
        return {FLOAT};
    else if(std::holds_alternative<std::wstring>(value))
        return {STR};
    else if(std::holds_alternative<bool>(value))
        return {BOOL};
    else
        return {L"unknown"};
}

Variable::Variable(Position position, std::wstring name): Expression(position), name(name) {}

BinaryOperation::BinaryOperation(
    Position position, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right
): Expression(position), left(std::move(left)), right(std::move(right))
{}

UnaryMinusExpression::UnaryMinusExpression(Position position, std::unique_ptr<Expression> value):
    Expression(position), value(std::move(value))
{}

NotExpression::NotExpression(Position position, std::unique_ptr<Expression> value):
    Expression(position), value(std::move(value))
{}

IsExpression::IsExpression(Position begin, std::unique_ptr<Expression> left, Type right):
    Expression(begin), left(std::move(left)), right(right)
{}

DotExpression::DotExpression(Position position, std::unique_ptr<Expression> value, std::wstring field):
    Expression(position), value(std::move(value)), field(field)
{}

StructExpression::StructExpression(Position position, std::vector<std::unique_ptr<Expression>> arguments):
    Expression(position), arguments(std::move(arguments))
{}

CastExpression::CastExpression(Position position, std::unique_ptr<Expression> value, Type targetType):
    Expression(position), value(std::move(value)), targetType(targetType)
{}

VariableDeclaration::VariableDeclaration(Position position, Type type, std::wstring name, bool isMutable):
    DocumentTreeNode(position), type(type), name(name), isMutable(isMutable)
{}

VariableDeclStatement::VariableDeclStatement(
    Position position, VariableDeclaration declaration, std::unique_ptr<Expression> value
): Instruction(position), declaration(declaration), value(std::move(value))
{}

Assignable::Assignable(Position position, std::unique_ptr<Assignable> left, std::wstring right):
    DocumentTreeNode(position), left(std::move(left)), right(right)
{}

Assignable::Assignable(Position position, std::wstring value): DocumentTreeNode(position), left(nullptr), right(value)
{}

AssignmentStatement::AssignmentStatement(Position position, Assignable left, std::unique_ptr<Expression> right):
    Instruction(position), left(std::move(left)), right(std::move(right))
{}

FunctionCall::FunctionCall(
    Position position, std::wstring functionName, std::vector<std::unique_ptr<Expression>> arguments
): Expression(position), functionName(functionName), arguments(std::move(arguments))
{}

FunctionCallInstruction::FunctionCallInstruction(Position position, FunctionCall functionCall):
    Instruction(position), functionCall(std::move(functionCall))
{}

ContinueStatement::ContinueStatement(Position position): Instruction(position) {}

BreakStatement::BreakStatement(Position position): Instruction(position) {}

ReturnStatement::ReturnStatement(Position position, std::unique_ptr<Expression> returnValue):
    Instruction(position), returnValue(std::move(returnValue))
{}

SingleIfCase::SingleIfCase(
    Position position, std::variant<VariableDeclStatement, std::unique_ptr<Expression>> condition,
    std::vector<std::unique_ptr<Instruction>> body
): DocumentTreeNode(position), condition(std::move(condition)), body(std::move(body))
{}

IfStatement::IfStatement(
    Position position, std::vector<SingleIfCase> cases, std::vector<std::unique_ptr<Instruction>> elseCaseBody
): Instruction(position), cases(std::move(cases)), elseCaseBody(std::move(elseCaseBody))
{}

WhileStatement::WhileStatement(
    Position position, std::unique_ptr<Expression> condition, std::vector<std::unique_ptr<Instruction>> body
): Instruction(position), condition(std::move(condition)), body(std::move(body))
{}

DoWhileStatement::DoWhileStatement(
    Position position, std::unique_ptr<Expression> condition, std::vector<std::unique_ptr<Instruction>> body
): Instruction(position), condition(std::move(condition)), body(std::move(body))
{}

FunctionIdentification::FunctionIdentification(std::wstring name, std::vector<Type> parameterTypes):
    name(name), parameterTypes(parameterTypes)
{}

std::wostream &operator<<(std::wostream &out, const FunctionIdentification &id)
{
    std::ostream_iterator<wchar_t, wchar_t> outIterator(out);
    std::format_to(outIterator, L"{}", id);
    return out;
}

Field::Field(Position position, Type type, std::wstring name): DocumentTreeNode(position), type(type), name(name) {}

StructDeclaration::StructDeclaration(Position position, std::wstring source, std::vector<Field> fields):
    DocumentTreeNode(position), fields(fields), source(source)
{}

std::wstring StructDeclaration::getSource() const
{
    return source;
}

VariantDeclaration::VariantDeclaration(Position position, std::wstring source, std::vector<Field> fields):
    DocumentTreeNode(position), fields(fields), source(source)
{}

std::wstring VariantDeclaration::getSource() const
{
    return source;
}

FunctionDeclaration::FunctionDeclaration(
    Position position, std::wstring source, std::vector<VariableDeclaration> parameters, std::optional<Type> returnType,
    std::vector<std::unique_ptr<Instruction>> body
): DocumentTreeNode(position), parameters(parameters), returnType(returnType), body(std::move(body)), source(source)
{}

std::wstring FunctionDeclaration::getSource() const
{
    return source;
}

IncludeStatement::IncludeStatement(Position position, std::wstring filePath):
    DocumentTreeNode(position), filePath(filePath)
{}

Program::Program(Position position): DocumentTreeNode(position) {}

void Program::add(std::pair<std::wstring, StructDeclaration> structBuilt)
{
    if(structs.find(structBuilt.first) != structs.end())
    {
        throw DuplicateStructError(
            std::format(L"Duplicate structure with name {}", structBuilt.first), structBuilt.second.getSource(),
            structBuilt.second.getPosition()
        );
    }
    structs.insert(std::move(structBuilt));
}

void Program::add(std::pair<std::wstring, VariantDeclaration> variantBuilt)
{
    if(variants.find(variantBuilt.first) != variants.end())
    {
        throw DuplicateVariantError(
            std::format(L"Duplicate variant with name {}", variantBuilt.first), variantBuilt.second.getSource(),
            variantBuilt.second.getPosition()
        );
    }
    variants.insert(std::move(variantBuilt));
}

void Program::add(std::pair<FunctionIdentification, FunctionDeclaration> functionBuilt)
{
    if(functions.find(functionBuilt.first) != functions.end())
    {
        throw DuplicateFunctionError(
            std::format(L"Duplicate function with signature {}", functionBuilt.first), functionBuilt.second.getSource(),
            functionBuilt.second.getPosition()
        );
    }
    functions.insert(std::move(functionBuilt));
}

#define DEFINE_ACCEPT(type)                         \
    void type::accept(DocumentTreeVisitor &visitor) \
    {                                               \
        visitor.visit(*this);                       \
    }

DEFINE_ACCEPT(Literal);
DEFINE_ACCEPT(Variable);
DEFINE_ACCEPT(IsExpression);
DEFINE_ACCEPT(OrExpression);
DEFINE_ACCEPT(XorExpression);
DEFINE_ACCEPT(AndExpression);
DEFINE_ACCEPT(EqualExpression);
DEFINE_ACCEPT(NotEqualExpression);
DEFINE_ACCEPT(IdenticalExpression);
DEFINE_ACCEPT(NotIdenticalExpression);
DEFINE_ACCEPT(ConcatExpression);
DEFINE_ACCEPT(StringMultiplyExpression);
DEFINE_ACCEPT(GreaterExpression);
DEFINE_ACCEPT(LesserExpression);
DEFINE_ACCEPT(GreaterEqualExpression);
DEFINE_ACCEPT(LesserEqualExpression);
DEFINE_ACCEPT(PlusExpression);
DEFINE_ACCEPT(MinusExpression);
DEFINE_ACCEPT(MultiplyExpression);
DEFINE_ACCEPT(DivideExpression);
DEFINE_ACCEPT(FloorDivideExpression);
DEFINE_ACCEPT(ModuloExpression);
DEFINE_ACCEPT(ExponentExpression);
DEFINE_ACCEPT(UnaryMinusExpression);
DEFINE_ACCEPT(NotExpression);
DEFINE_ACCEPT(SubscriptExpression);
DEFINE_ACCEPT(DotExpression);
DEFINE_ACCEPT(StructExpression);
DEFINE_ACCEPT(CastExpression);
DEFINE_ACCEPT(VariableDeclaration);
DEFINE_ACCEPT(VariableDeclStatement);
DEFINE_ACCEPT(Assignable);
DEFINE_ACCEPT(AssignmentStatement);
DEFINE_ACCEPT(FunctionCall);
DEFINE_ACCEPT(FunctionCallInstruction);
DEFINE_ACCEPT(ReturnStatement);
DEFINE_ACCEPT(ContinueStatement);
DEFINE_ACCEPT(BreakStatement);
DEFINE_ACCEPT(SingleIfCase);
DEFINE_ACCEPT(IfStatement);
DEFINE_ACCEPT(WhileStatement);
DEFINE_ACCEPT(DoWhileStatement);
DEFINE_ACCEPT(Field);
DEFINE_ACCEPT(StructDeclaration);
DEFINE_ACCEPT(VariantDeclaration);
DEFINE_ACCEPT(FunctionDeclaration);
DEFINE_ACCEPT(IncludeStatement);
DEFINE_ACCEPT(Program);
