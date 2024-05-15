#include "documentTree.hpp"

#include "parserExceptions.hpp"

#include <iterator>

DocumentTreeNode::DocumentTreeNode(Position position): position(position) {}

Position DocumentTreeNode::getPosition() const
{
    return position;
}

Literal::Literal(Position position, std::variant<std::wstring, int32_t, double, bool> value):
    Expression(position), value(value)
{}

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

IsExpression::IsExpression(Position begin, std::unique_ptr<Expression> left, std::wstring right):
    Expression(begin), left(std::move(left)), right(right)
{}

DotExpression::DotExpression(Position position, std::unique_ptr<Expression> value, std::wstring field):
    Expression(position), value(std::move(value)), field(field)
{}

StructExpression::StructExpression(Position position, std::vector<std::unique_ptr<Expression>> arguments):
    Expression(position), arguments(std::move(arguments))
{}

VariableDeclaration::VariableDeclaration(Position position, std::wstring type, std::wstring name, bool isMutable):
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

FunctionIdentification::FunctionIdentification(std::wstring name, std::vector<std::wstring> parameterTypes):
    name(name), parameterTypes(parameterTypes)
{}

std::wostream &operator<<(std::wostream &out, const FunctionIdentification &id)
{
    std::ostream_iterator<wchar_t, wchar_t> outIterator(out);
    std::format_to(outIterator, L"{}", id);
    return out;
}

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

void Program::add(std::pair<std::wstring, StructDeclaration> structBuilt)
{
    if(structs.find(structBuilt.first) != structs.end())
    {
        throw DuplicateStructError(
            std::format(L"Duplicate structure with name {}", structBuilt.first), structBuilt.second.getPosition()
        );
    }
    structs.insert(std::move(structBuilt));
}

void Program::add(std::pair<std::wstring, VariantDeclaration> variantBuilt)
{
    if(variants.find(variantBuilt.first) != variants.end())
    {
        throw DuplicateVariantError(
            std::format(L"Duplicate variant with name {}", variantBuilt.first), variantBuilt.second.getPosition()
        );
    }
    variants.insert(std::move(variantBuilt));
}

void Program::add(std::pair<FunctionIdentification, FunctionDeclaration> functionBuilt)
{
    if(functions.find(functionBuilt.first) != functions.end())
    {
        throw DuplicateFunctionError(
            std::format(L"Duplicate function with signature {}", functionBuilt.first),
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
