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
#define DECLARE_LITERAL_TYPE(type, string)       \
    std::wstring literalTypeToString(type value) \
    {                                            \
        (void) value;                            \
        return string;                           \
    }
DECLARE_LITERAL_TYPE(std::wstring, L"string")
DECLARE_LITERAL_TYPE(double, L"float")
DECLARE_LITERAL_TYPE(int32_t, L"int")
DECLARE_LITERAL_TYPE(bool, L"bool")
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

void PrintingVisitor::visitInstructionBlock(std::vector<std::unique_ptr<Instruction>> &block)
{
    for(auto &instruction: block)
    {
        out << indent;
        if(&instruction != &block.back())
        {
            out << L"|-";
            indent += L"|";
        }
        else
        {
            out << L"`-";
            indent += L" ";
        }
        instruction->accept(*this);
        popIndent();
    }
}

void PrintingVisitor::visitFields(std::vector<Field> &fields)
{
    for(auto &field: fields)
    {
        out << indent;
        if(&field != &fields.back())
        {
            out << L"|-";
            indent += L"|";
        }
        else
        {
            out << L"`-";
            indent += L" ";
        }
        field.accept(*this);
        popIndent();
    }
}

void PrintingVisitor::visit(OrExpression &visited)
{
    visitBinaryOperation(L"OrExpression", visited);
}

void PrintingVisitor::visit(XorExpression &visited)
{
    visitBinaryOperation(L"XorExpression", visited);
}

void PrintingVisitor::visit(AndExpression &visited)
{
    visitBinaryOperation(L"AndExpression", visited);
}

void PrintingVisitor::visit(EqualExpression &visited)
{
    visitBinaryOperation(L"EqualExpression", visited);
}

void PrintingVisitor::visit(NotEqualExpression &visited)
{
    visitBinaryOperation(L"NotEqualExpression", visited);
}

void PrintingVisitor::visit(IdenticalExpression &visited)
{
    visitBinaryOperation(L"IdenticalExpression", visited);
}

void PrintingVisitor::visit(NotIdenticalExpression &visited)
{
    visitBinaryOperation(L"NotIdenticalExpression", visited);
}

void PrintingVisitor::visit(ConcatExpression &visited)
{
    visitBinaryOperation(L"ConcatExpression", visited);
}

void PrintingVisitor::visit(StringMultiplyExpression &visited)
{
    visitBinaryOperation(L"StringMultiplyExpression", visited);
}

void PrintingVisitor::visit(GreaterExpression &visited)
{
    visitBinaryOperation(L"GreaterExpression", visited);
}

void PrintingVisitor::visit(LesserExpression &visited)
{
    visitBinaryOperation(L"LesserExpression", visited);
}

void PrintingVisitor::visit(GreaterEqualExpression &visited)
{
    visitBinaryOperation(L"GreaterEqualExpression", visited);
}

void PrintingVisitor::visit(LesserEqualExpression &visited)
{
    visitBinaryOperation(L"LesserEqualExpression", visited);
}

void PrintingVisitor::visit(PlusExpression &visited)
{
    visitBinaryOperation(L"PlusExpression", visited);
}

void PrintingVisitor::visit(MinusExpression &visited)
{
    visitBinaryOperation(L"MinusExpression", visited);
}

void PrintingVisitor::visit(MultiplyExpression &visited)
{
    visitBinaryOperation(L"MultiplyExpression", visited);
}

void PrintingVisitor::visit(DivideExpression &visited)
{
    visitBinaryOperation(L"DivideExpression", visited);
}

void PrintingVisitor::visit(FloorDivideExpression &visited)
{
    visitBinaryOperation(L"FloorDivideExpression", visited);
}

void PrintingVisitor::visit(ModuloExpression &visited)
{
    visitBinaryOperation(L"ModuloExpression", visited);
}

void PrintingVisitor::visit(ExponentExpression &visited)
{
    visitBinaryOperation(L"ExponentExpression", visited);
}

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

void PrintingVisitor::visit(SubscriptExpression &visited)
{
    visitBinaryOperation(L"SubscriptExpression", visited);
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
    for(auto &parameter: visited.arguments)
    {
        out << indent;
        if(&parameter != &visited.arguments.back())
        {
            out << L"|-";
            indent += L"|";
        }
        else
        {
            out << L"`-";
            indent += L" ";
        }
        parameter->accept(*this);
        popIndent();
    }
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
    for(auto &parameter: visited.parameters)
    {
        out << indent;
        if(&parameter != &visited.parameters.back())
        {
            out << L"|-";
            indent += L"|";
        }
        else
        {
            out << L"`-";
            indent += L" ";
        }
        parameter->accept(*this);
        popIndent();
    }
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
    visitInstructionBlock(visited.body);
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
        visitInstructionBlock(visited.elseCaseBody);
        popIndent();
    }
}

void PrintingVisitor::visit(WhileStatement &visited)
{
    out << L"WhileStatement " << visited.getPosition() << L"\n" << indent;
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
    visited.condition->accept(*this);
    popIndent();
    visitInstructionBlock(visited.body);
}

void PrintingVisitor::visit(DoWhileStatement &visited)
{
    out << L"DoWhileStatement " << visited.getPosition() << L"\n" << indent;
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
    visited.condition->accept(*this);
    popIndent();
    visitInstructionBlock(visited.body);
}

void PrintingVisitor::visit(Field &visited)
{
    out << L"Field " << visited.getPosition() << L" type=" << visited.type << L" name=" << visited.name << L"\n";
}

void PrintingVisitor::visit(StructDeclaration &visited)
{
    out << L"StructDeclaration " << visited.getPosition() << L"\n";
    visitFields(visited.fields);
}

void PrintingVisitor::visit(VariantDeclaration &visited)
{
    out << L"VariantDeclaration " << visited.getPosition() << L"\n";
    visitFields(visited.fields);
}

void PrintingVisitor::visit(FunctionDeclaration &visited)
{
    out << L"FunctionDeclaration " << visited.getPosition();
    if(visited.returnType.has_value())
        out << L" returnType=" << *visited.returnType;
    out << L"\n";
    if(visited.parameters.size() > 0)
    {
        out << indent;
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
        out << L"Parameters:\n";
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
        visitInstructionBlock(visited.body);
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
        for(auto &includeStatement: visited.includes)
        {
            out << indent;
            if(&includeStatement != &visited.includes.back())
                out << L"|-";
            else
                out << L"`-";
            includeStatement.accept(*this);
        }
    }
    if(visited.structs.size() > 0)
    {
        out << indent << L"Structs:\n";
        for(auto it = visited.structs.begin(); it != visited.structs.end(); it++)
        {
            out << indent;
            if(std::next(it) != visited.structs.end())
            {
                out << L"|-";
                indent += L"|";
            }
            else
            {
                out << L"`-";
                indent += L" ";
            }
            out << it->first << L": ";
            it->second.accept(*this);
            popIndent();
        }
    }
    if(visited.variants.size() > 0)
    {
        out << indent << L"Variants:\n";
        for(auto it = visited.variants.begin(); it != visited.variants.end(); it++)
        {
            out << indent;
            if(std::next(it) != visited.variants.end())
            {
                out << L"|-";
                indent += L"|";
            }
            else
            {
                out << L"`-";
                indent += L" ";
            }
            out << it->first << L": ";
            it->second.accept(*this);
            popIndent();
        }
    }
    if(visited.functions.size() > 0)
    {
        out << indent << L"Functions:\n";
        for(auto it = visited.functions.begin(); it != visited.functions.end(); it++)
        {
            out << indent;
            if(std::next(it) != visited.functions.end())
            {
                out << L"|-";
                indent += L"|";
            }
            else
            {
                out << L"`-";
                indent += L" ";
            }
            out << it->first.name << L": ";
            it->second.accept(*this);
            popIndent();
        }
    }
}
