#include "printingVisitor.hpp"

#include "documentTree.hpp"

#include <format>
#include <iomanip>

PrintingVisitor::PrintingVisitor(std::wostream &out): out(out) {}

void PrintingVisitor::popIndent()
{
    indent.erase(indent.size() - 1, 1);
}

namespace {
#define LITERAL_TYPE(type, string)               \
    std::wstring literalTypeToString(type value) \
    {                                            \
        (void) value;                            \
        return string;                           \
    }
LITERAL_TYPE(std::wstring, L"string")
LITERAL_TYPE(double, L"float")
LITERAL_TYPE(int32_t, L"int")
LITERAL_TYPE(bool, L"bool")
}

void PrintingVisitor::visit(Literal &visited)
{
    auto outFlags = out.flags();
    out << L"Literal " << visited.getPosition() << L" type=" << std::boolalpha;
    std::visit([&](auto &value) { out << literalTypeToString(value) << L" value=" << value; }, visited.value);
    out << L"\n";
    out.flags(outFlags);
}

void PrintingVisitor::visit(Variable &visited)
{
    out << L"Variable " << visited.getPosition() << L" name=" << visited.name << L"\n";
}

void PrintingVisitor::visitUnaryOperation(std::wstring name, Position position, DocumentTreeNode &child)
{
    out << name << L" " << position << L"\n";
    out << indent << L"`-";
    indent += L" ";
    child.accept(*this);
    popIndent();
}

void PrintingVisitor::visitBinaryOperation(std::wstring name, BinaryOperation &visited)
{
    out << name << L" " << visited.getPosition() << L"\n";
    out << indent << L"|-";
    indent += L"|";
    visited.left->accept(*this);
    popIndent();
    out << indent << L"`-";
    indent += L" ";
    visited.right->accept(*this);
    popIndent();
}

template <typename Node>
void PrintingVisitor::visit(std::pair<const std::wstring, Node> &visited)
{
    out << visited.first << L": ";
    visited.second.accept(*this);
}

void PrintingVisitor::visit(std::pair<const FunctionIdentification, FunctionDeclaration> &visited)
{
    out << visited.first.name;
    if(visited.first.parameterTypes.size() > 0)
    {
        out << L"(";
        for(const std::wstring &parameter: visited.first.parameterTypes)
        {
            out << parameter;
            if(&parameter != &visited.first.parameterTypes.back())
                out << L", ";
        }
        out << L")";
    }
    out << L": ";
    visited.second.accept(*this);
}

template <typename NodeContainer>
void PrintingVisitor::visitContainer(NodeContainer &visited)
{
    for(auto it = visited.begin(); it != visited.end(); it++)
    {
        out << indent;
        if(std::next(it) != visited.end())
        {
            out << L"|-";
            indent += L"|";
        }
        else
        {
            out << L"`-";
            indent += L" ";
        }
        visit(*it);
        popIndent();
    }
}

template <typename ConditionalStatement>
void PrintingVisitor::visitCondition(ConditionalStatement &visited)
{
    if(visited.body.size() > 0)
    {
        out << L"|-";
        indent += L"|";
    }
    else
    {
        out << L"`-";
        indent += L" ";
    }
    visit(visited.condition);
    popIndent();
}

void PrintingVisitor::visit(DocumentTreeNode &visited)
{
    visited.accept(*this);
}

template <typename Node>
void PrintingVisitor::visit(std::unique_ptr<Node> &visited)
{
    visited->accept(*this);
}

#define BINARY_OP_VISIT(type)                     \
    void PrintingVisitor::visit(type &visited)    \
    {                                             \
        visitBinaryOperation(L## #type, visited); \
    }

BINARY_OP_VISIT(OrExpression)
BINARY_OP_VISIT(XorExpression)
BINARY_OP_VISIT(AndExpression)
BINARY_OP_VISIT(EqualExpression)
BINARY_OP_VISIT(NotEqualExpression)
BINARY_OP_VISIT(IdenticalExpression)
BINARY_OP_VISIT(NotIdenticalExpression)
BINARY_OP_VISIT(ConcatExpression)
BINARY_OP_VISIT(StringMultiplyExpression)
BINARY_OP_VISIT(GreaterExpression)
BINARY_OP_VISIT(LesserExpression)
BINARY_OP_VISIT(GreaterEqualExpression)
BINARY_OP_VISIT(LesserEqualExpression)
BINARY_OP_VISIT(PlusExpression)
BINARY_OP_VISIT(MinusExpression)
BINARY_OP_VISIT(MultiplyExpression)
BINARY_OP_VISIT(DivideExpression)
BINARY_OP_VISIT(FloorDivideExpression)
BINARY_OP_VISIT(ModuloExpression)
BINARY_OP_VISIT(ExponentExpression)
BINARY_OP_VISIT(SubscriptExpression)

void PrintingVisitor::visit(UnaryMinusExpression &visited)
{
    visitUnaryOperation(L"UnaryMinusExpression", visited.getPosition(), *visited.value);
}

void PrintingVisitor::visit(NotExpression &visited)
{
    visitUnaryOperation(L"NotExpression", visited.getPosition(), *visited.value);
}

void PrintingVisitor::visit(IsExpression &visited)
{
    out << L"IsExpression " << visited.getPosition() << L" right=" << visited.right << L"\n";
    out << indent << L"`-";
    indent += L" ";
    visited.left->accept(*this);
    popIndent();
}

void PrintingVisitor::visit(DotExpression &visited)
{
    out << L"DotExpression " << visited.getPosition() << L" field=" << visited.field << L"\n";
    out << indent << L"`-";
    indent += L" ";
    visited.value->accept(*this);
    popIndent();
}

void PrintingVisitor::visit(StructExpression &visited)
{
    out << L"StructExpression " << visited.getPosition() << L"\n";
    visitContainer(visited.arguments);
}

void PrintingVisitor::visit(VariableDeclaration &visited)
{
    auto outFlags = out.flags();
    out << L"VariableDeclaration " << visited.getPosition() << L" type=" << visited.type << L" name=" << visited.name
        << L" mutable=" << std::boolalpha << visited.isMutable << L"\n";
    out.flags(outFlags);
}

void PrintingVisitor::visit(VariableDeclStatement &visited)
{
    out << L"VariableDeclStatement " << visited.getPosition() << L"\n";
    out << indent << L"|-";
    indent += L"|";
    visited.declaration.accept(*this);
    popIndent();
    out << indent << L"`-";
    indent += L" ";
    visited.value->accept(*this);
    popIndent();
}

void PrintingVisitor::visit(Assignable &visited)
{
    out << L"Assignable " << visited.getPosition() << L" right=" << visited.right << L"\n";
    if(visited.left)
    {
        out << indent << L"`-";
        indent += L" ";
        visited.left->accept(*this);
        popIndent();
    }
}

void PrintingVisitor::visit(AssignmentStatement &visited)
{
    out << L"AssignmentStatement " << visited.getPosition() << L"\n";
    out << indent << L"|-";
    indent += L"|";
    visited.left.accept(*this);
    popIndent();
    out << indent << L"`-";
    indent += L" ";
    visited.right->accept(*this);
    popIndent();
}

void PrintingVisitor::visit(FunctionCall &visited)
{
    out << L"FunctionCall " << visited.getPosition() << L" functionName=" << visited.functionName << L"\n";
    visitContainer(visited.parameters);
}

void PrintingVisitor::visit(ReturnStatement &visited)
{
    out << L"ReturnStatement " << visited.getPosition() << L"\n";
    if(visited.returnValue)
    {
        out << indent << L"`-";
        indent += L" ";
        visited.returnValue->accept(*this);
        popIndent();
    }
}

void PrintingVisitor::visit(ContinueStatement &visited)
{
    out << L"ContinueStatement " << visited.getPosition() << L"\n";
}

void PrintingVisitor::visit(BreakStatement &visited)
{
    out << L"BreakStatement " << visited.getPosition() << L"\n";
}

void PrintingVisitor::visit(SingleIfCase &visited)
{
    out << L"SingleIfCase " << visited.getPosition() << L"\n" << indent;
    if(visited.body.size() > 0)
    {
        out << L"|-";
        indent += L"|";
    }
    else
    {
        out << L"`-";
        indent += L" ";
    }
    if(std::holds_alternative<VariableDeclStatement>(visited.condition))
        std::get<VariableDeclStatement>(visited.condition).accept(*this);
    else
        std::get<std::unique_ptr<Expression>>(visited.condition)->accept(*this);
    popIndent();
    visitContainer(visited.body);
}

void PrintingVisitor::visit(IfStatement &visited)
{
    out << L"IfStatement " << visited.getPosition() << L"\n";
    for(auto &ifCase: visited.cases)
    {
        out << indent;
        if(&ifCase != &visited.cases.back() || visited.elseCaseBody.size() > 0)
        {
            out << L"|-";
            indent += L"|";
        }
        else
        {
            out << L"`-";
            indent += L" ";
        }
        ifCase.accept(*this);
        popIndent();
    }
    if(visited.elseCaseBody.size() > 0)
    {
        out << indent << L"`-ElseCase:\n";
        indent += L" ";
        visitContainer(visited.elseCaseBody);
        popIndent();
    }
}

void PrintingVisitor::visit(WhileStatement &visited)
{
    out << L"WhileStatement " << visited.getPosition() << L"\n" << indent;
    visitCondition(visited);
    visitContainer(visited.body);
}

void PrintingVisitor::visit(DoWhileStatement &visited)
{
    out << L"DoWhileStatement " << visited.getPosition() << L"\n" << indent;
    visitCondition(visited);
    visitContainer(visited.body);
}

void PrintingVisitor::visit(Field &visited)
{
    out << L"Field " << visited.getPosition() << L" type=" << visited.type << L" name=" << visited.name << L"\n";
}

void PrintingVisitor::visit(StructDeclaration &visited)
{
    out << L"StructDeclaration " << visited.getPosition() << L"\n";
    visitContainer(visited.fields);
}

void PrintingVisitor::visit(VariantDeclaration &visited)
{
    out << L"VariantDeclaration " << visited.getPosition() << L"\n";
    visitContainer(visited.fields);
}

void PrintingVisitor::visit(FunctionDeclaration &visited)
{
    out << L"FunctionDeclaration " << visited.getPosition();
    if(visited.returnType.has_value())
        out << L" returnType=" << *visited.returnType;
    out << L"\n";
    if(visited.parameters.size() > 0)
    {
        out << indent << L"Parameters:\n";
        if(visited.body.size() > 0)
        {
            out << L"|-";
            indent += L"|";
        }
        else
        {
            out << L"`-";
            indent += L" ";
        }
        for(auto &parameter: visited.parameters)
        {
            out << indent;
            if(&parameter != &visited.parameters.back())
                out << L"|-";
            else
                out << L"`-";
            parameter.accept(*this);
        }
        popIndent();
    }
    if(visited.body.size() > 0)
    {
        out << indent << L"`-Body:\n";
        indent += L" ";
        visitContainer(visited.body);
        popIndent();
    }
}

void PrintingVisitor::visit(IncludeStatement &visited)
{
    out << L"IncludeStatement " << visited.getPosition() << L" filePath=" << visited.filePath << L"\n";
}

void PrintingVisitor::visit(Program &visited)
{
    out << L"Program containing:\n";
    if(visited.includes.size() > 0)
    {
        out << indent << L"Includes:\n";
        visitContainer(visited.includes);
    }
    if(visited.structs.size() > 0)
    {
        out << indent << L"Structs:\n";
        visitContainer(visited.structs);
    }
    if(visited.variants.size() > 0)
    {
        out << indent << L"Variants:\n";
        visitContainer(visited.variants);
    }
    if(visited.functions.size() > 0)
    {
        out << indent << L"Functions:\n";
        visitContainer(visited.functions);
    }
}
