#include "documentTree.hpp"

DocumentTreeNode::DocumentTreeNode(Position position): position(position) {}

Position DocumentTreeNode::getPosition() const
{
    return position;
}

Expression::Expression(): DocumentTreeNode({0, 0}) {}

Literal::Literal(Position position, std::variant<std::wstring, int32_t, double, bool> value):
    DocumentTreeNode(position), value(value)
{}

Variable::Variable(Position position, std::wstring name): DocumentTreeNode(position), name(name) {}

BinaryOperation::BinaryOperation(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right):
    left(std::move(left)), right(std::move(right))
{}

#define BINARY_OP_CONSTRUCTOR(type)                                                                  \
    type::type(Position begin, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right): \
        DocumentTreeNode(begin), BinaryOperation(std::move(left), std::move(right))                  \
    {}

BINARY_OP_CONSTRUCTOR(OrExpression)
BINARY_OP_CONSTRUCTOR(XorExpression)
BINARY_OP_CONSTRUCTOR(AndExpression)
BINARY_OP_CONSTRUCTOR(EqualExpression)
BINARY_OP_CONSTRUCTOR(NotEqualExpression)
BINARY_OP_CONSTRUCTOR(IdenticalExpression)
BINARY_OP_CONSTRUCTOR(NotIdenticalExpression)
BINARY_OP_CONSTRUCTOR(ConcatExpression)
BINARY_OP_CONSTRUCTOR(StringMultiplyExpression)
BINARY_OP_CONSTRUCTOR(GreaterExpression)
BINARY_OP_CONSTRUCTOR(LesserExpression)
BINARY_OP_CONSTRUCTOR(GreaterEqualExpression)
BINARY_OP_CONSTRUCTOR(LesserEqualExpression)
BINARY_OP_CONSTRUCTOR(PlusExpression)
BINARY_OP_CONSTRUCTOR(MinusExpression)
BINARY_OP_CONSTRUCTOR(MultiplyExpression)
BINARY_OP_CONSTRUCTOR(DivideExpression)
BINARY_OP_CONSTRUCTOR(FloorDivideExpression)
BINARY_OP_CONSTRUCTOR(ModuloExpression)
BINARY_OP_CONSTRUCTOR(ExponentExpression)
BINARY_OP_CONSTRUCTOR(SubscriptExpression)

UnaryMinusExpression::UnaryMinusExpression(Position position, std::unique_ptr<Expression> value):
    DocumentTreeNode(position), value(std::move(value))
{}

NotExpression::NotExpression(Position position, std::unique_ptr<Expression> value):
    DocumentTreeNode(position), value(std::move(value))
{}

IsExpression::IsExpression(Position begin, std::unique_ptr<Expression> left, std::wstring right):
    DocumentTreeNode(begin), left(std::move(left)), right(right)
{}

DotExpression::DotExpression(Position position, std::unique_ptr<Expression> value, std::wstring field):
    DocumentTreeNode(position), value(std::move(value)), field(field)
{}

StructExpression::StructExpression(Position position, std::vector<std::unique_ptr<Expression>> arguments):
    DocumentTreeNode(position), arguments(std::move(arguments))
{}

Instruction::Instruction(): DocumentTreeNode({0, 0}) {}

VariableDeclaration::VariableDeclaration(Position position, std::wstring type, std::wstring name, bool isMutable):
    DocumentTreeNode(position), type(type), name(name), isMutable(isMutable)
{}

VariableDeclStatement::VariableDeclStatement(
    Position position, VariableDeclaration declaration, std::unique_ptr<Expression> value
): DocumentTreeNode(position), declaration(declaration), value(std::move(value))
{}

Assignable::Assignable(Position position, std::unique_ptr<Assignable> left, std::wstring right):
    DocumentTreeNode(position), left(std::move(left)), right(right)
{}

Assignable::Assignable(Position position, std::wstring value): DocumentTreeNode(position), left(nullptr), right(value)
{}

AssignmentStatement::AssignmentStatement(Position position, Assignable left, std::unique_ptr<Expression> right):
    DocumentTreeNode(position), left(std::move(left)), right(std::move(right))
{}

FunctionCall::FunctionCall(
    Position position, std::wstring functionName, std::vector<std::unique_ptr<Expression>> arguments
): DocumentTreeNode(position), functionName(functionName), arguments(std::move(arguments))
{}

ContinueStatement::ContinueStatement(Position position): DocumentTreeNode(position) {}

BreakStatement::BreakStatement(Position position): DocumentTreeNode(position) {}

ReturnStatement::ReturnStatement(Position position, std::unique_ptr<Expression> returnValue):
    DocumentTreeNode(position), returnValue(std::move(returnValue))
{}

SingleIfCase::SingleIfCase(
    Position position, std::variant<VariableDeclStatement, std::unique_ptr<Expression>> condition,
    std::vector<std::unique_ptr<Instruction>> body
): DocumentTreeNode(position), condition(std::move(condition)), body(std::move(body))
{}

IfStatement::IfStatement(
    Position position, std::vector<SingleIfCase> cases, std::vector<std::unique_ptr<Instruction>> elseCaseBody
): DocumentTreeNode(position), cases(std::move(cases)), elseCaseBody(std::move(elseCaseBody))
{}

WhileStatement::WhileStatement(
    Position position, std::unique_ptr<Expression> condition, std::vector<std::unique_ptr<Instruction>> body
): DocumentTreeNode(position), condition(std::move(condition)), body(std::move(body))
{}

DoWhileStatement::DoWhileStatement(
    Position position, std::unique_ptr<Expression> condition, std::vector<std::unique_ptr<Instruction>> body
): DocumentTreeNode(position), condition(std::move(condition)), body(std::move(body))
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
    Position position, std::vector<VariableDeclaration> parameters, std::optional<std::wstring> returnType,
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
DEFINE_ACCEPT(VariableDeclaration);
DEFINE_ACCEPT(VariableDeclStatement);
DEFINE_ACCEPT(Assignable);
DEFINE_ACCEPT(AssignmentStatement);
DEFINE_ACCEPT(FunctionCall);
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
