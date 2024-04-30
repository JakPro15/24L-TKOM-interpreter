#include "documentTree.hpp"

DocumentTreeNode::DocumentTreeNode(Position position): position(position) {}

Position DocumentTreeNode::getPosition() const
{
    return position;
}

Literal::Literal(Position position, std::variant<std::wstring, int32_t, double, bool> value):
    Expression(position), value(value)
{}

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

DotExpression::DotExpression(Position position, std::unique_ptr<Expression> value, std::wstring field):
    Expression(position), value(std::move(value)), field(field)
{}

StructExpression::StructExpression(Position position, std::vector<std::unique_ptr<Expression>> arguments):
    Expression(position), arguments(std::move(arguments))
{}

VariableDeclaration::VariableDeclaration(
    Position position, std::wstring type, std::wstring name, std::unique_ptr<Expression> value
): Instruction(position), type(type), name(name), value(std::move(value))
{}

Assignable::Assignable(Position position, std::unique_ptr<Assignable> left, std::wstring right):
    DocumentTreeNode(position), left(std::move(left)), right(right)
{}

AssignmentStatement::AssignmentStatement(Position position, Assignable left, std::unique_ptr<Expression> right):
    Instruction(position), left(std::move(left)), right(std::move(right))
{}

FunctionCall::FunctionCall(
    Position position, std::wstring functionName, std::vector<std::unique_ptr<Expression>> parameters
): Instruction(position), functionName(functionName), parameters(std::move(parameters))
{}

ReturnStatement::ReturnStatement(Position position, std::unique_ptr<Expression> returnValue):
    Instruction(position), returnValue(std::move(returnValue))
{}

IfStatement::IfStatement(
    Position position, VariableDeclaration condition, std::vector<std::unique_ptr<Instruction>> body
): Instruction(position), condition(std::move(condition)), body(std::move(body))
{}

IfStatement::IfStatement(
    Position position, std::unique_ptr<Expression> condition, std::vector<std::unique_ptr<Instruction>> body
): Instruction(position), condition(std::move(condition)), body(std::move(body))
{}

WhileStatement::WhileStatement(
    Position position, std::unique_ptr<Expression> condition, std::vector<std::unique_ptr<Instruction>> body
): Instruction(position), condition(std::move(condition)), body(std::move(body))
{}

DoWhileStatement::DoWhileStatement(
    Position position, std::unique_ptr<Expression> condition, std::vector<std::unique_ptr<Instruction>> body
): Instruction(position), condition(std::move(condition)), body(std::move(body))
{}

FunctionIdentification::FunctionIdentification(std::wstring name, std::vector<std::wstring> parameterTypes):
    name(name), parameterTypes(parameterTypes)
{}

Field::Field(Position position, std::wstring type, std::wstring name):
    DocumentTreeNode(position), type(type), name(name)
{}

StructDeclaration::StructDeclaration(Position position, std::vector<Field> fields):
    DocumentTreeNode(position), fields(fields)
{}

VariantDeclaration::VariantDeclaration(Position position, std::vector<Field> fields):
    DocumentTreeNode(position), fields(fields)
{}

FunctionDeclaration::FunctionDeclaration(
    Position position, std::vector<Field> parameters, std::wstring returnType,
    std::vector<std::unique_ptr<Instruction>> body
): DocumentTreeNode(position), parameters(parameters), returnType(returnType), body(std::move(body))
{}

IncludeStatement::IncludeStatement(Position position, std::wstring filePath):
    DocumentTreeNode(position), filePath(filePath)
{}

Program::Program(Position position): DocumentTreeNode(position) {}

#define DEFINE_ACCEPT(type)                         \
    void type::accept(DocumentTreeVisitor &visitor) \
    {                                               \
        visitor.visit(*this);                       \
    }

DEFINE_ACCEPT(Literal);
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
DEFINE_ACCEPT(VariableDeclaration);
DEFINE_ACCEPT(Assignable);
DEFINE_ACCEPT(AssignmentStatement);
DEFINE_ACCEPT(FunctionCall);
DEFINE_ACCEPT(ReturnStatement);
DEFINE_ACCEPT(ContinueStatement);
DEFINE_ACCEPT(BreakStatement);
DEFINE_ACCEPT(IfStatement);
DEFINE_ACCEPT(WhileStatement);
DEFINE_ACCEPT(DoWhileStatement);
DEFINE_ACCEPT(Field);
DEFINE_ACCEPT(StructDeclaration);
DEFINE_ACCEPT(VariantDeclaration);
DEFINE_ACCEPT(FunctionDeclaration);
DEFINE_ACCEPT(IncludeStatement);
DEFINE_ACCEPT(Program);
