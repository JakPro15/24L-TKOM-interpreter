#include "printingVisitor.hpp"

#include "documentTree.hpp"

PrintingVisitor::PrintingVisitor(std::wostream &out): out(out), indent(0) {}

void PrintingVisitor::visit(Literal &visited)
{
    out << L"Literal " << visited.getPosition() << L" value=";
    std::visit([&](auto &value) { out << value; }, visited.value);
    out << L"\n";
}

void PrintingVisitor::visitUnaryOperation(std::wstring name, Position position, DocumentTreeNode &child)
{
    out << name << L" " << position << L"\n";
    indent += 1;
    out << std::wstring(indent, L' ') << L"`-";
    child.accept(*this);
    indent -= 1;
}

void PrintingVisitor::visitBinaryOperation(std::wstring name, BinaryOperation &visited)
{
    out << name << L" " << visited.getPosition() << L"\n";
    indent += 1;
    out << std::wstring(indent, L' ') << L"|-";
    visited.left->accept(*this);
    out << std::wstring(indent, L' ') << L"`-";
    visited.right->accept(*this);
    indent -= 1;
}

void PrintingVisitor::visitInstructionBlock(std::vector<std::unique_ptr<Instruction>> &block)
{
    for(auto &parameter: block)
    {
        out << std::wstring(indent, L' ');
        if(&parameter != &block.back())
            out << L"|-";
        else
            out << L"`-";
        parameter->accept(*this);
    }
}

void PrintingVisitor::visitFields(std::vector<Field> &fields)
{
    for(auto &field: fields)
    {
        out << std::wstring(indent, L' ');
        if(&field != &fields.back())
            out << L"|-";
        else
            out << L"`-";
        field.accept(*this);
    }
}

void PrintingVisitor::visit(IsExpression &visited)
{
    visitBinaryOperation(L"IsExpression", visited);
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

void PrintingVisitor::visit(SubscriptExpression &visited)
{
    visitBinaryOperation(L"SubscriptExpression", visited);
}

void PrintingVisitor::visit(DotExpression &visited)
{
    out << L"DotExpression " << visited.getPosition() << L" field=" << visited.field << L"\n";
    indent += 1;
    out << std::wstring(indent, L' ') << L"`-";
    visited.value->accept(*this);
    indent -= 1;
}

void PrintingVisitor::visit(StructExpression &visited)
{
    out << L"StructExpression " << visited.getPosition() << L"\n";
    indent += 1;
    for(auto &parameter: visited.arguments)
    {
        out << std::wstring(indent, L' ');
        if(&parameter != &visited.arguments.back())
            out << L"|-";
        else
            out << L"`-";
        parameter->accept(*this);
    }
    indent -= 1;
}

void PrintingVisitor::visit(VariableDeclaration &visited)
{
    out << L"VariableDeclaration " << visited.getPosition() << L" type=" << visited.type << L" name=" << visited.name
        << L" mutable=" << visited.isMutable << L"\n";
}

void PrintingVisitor::visit(VariableDeclStatement &visited)
{
    out << L"VariableDeclStatement " << visited.getPosition() << L"\n";
    indent += 1;
    out << std::wstring(indent, L' ') << L"|-";
    visited.declaration.accept(*this);
    out << std::wstring(indent, L' ') << L"`-";
    visited.value->accept(*this);
    indent -= 1;
}

void PrintingVisitor::visit(Assignable &visited)
{
    out << L"VariableDeclStatement " << visited.getPosition() << L" right=" << visited.right << L"\n";
    indent += 1;
    out << std::wstring(indent, L' ') << L"`-";
    visited.left->accept(*this);
    indent -= 1;
}

void PrintingVisitor::visit(AssignmentStatement &visited)
{
    out << L"AssignmentStatement " << visited.getPosition() << L"\n";
    indent += 1;
    out << std::wstring(indent, L' ') << L"|-";
    visited.left.accept(*this);
    out << std::wstring(indent, L' ') << L"`-";
    visited.right->accept(*this);
    indent -= 1;
}

void PrintingVisitor::visit(FunctionCall &visited)
{
    out << L"FunctionCall " << visited.getPosition() << L" functionName=" << visited.functionName << L"\n";
    indent += 1;
    for(auto &parameter: visited.parameters)
    {
        out << std::wstring(indent, L' ');
        if(&parameter != &visited.parameters.back())
            out << L"|-";
        else
            out << L"`-";
        parameter->accept(*this);
    }
    indent -= 1;
}

void PrintingVisitor::visit(ReturnStatement &visited)
{
    out << L"ReturnStatement " << visited.getPosition() << L"\n";
    if(visited.returnValue)
    {
        indent += 1;
        out << L"`-";
        visited.returnValue->accept(*this);
        indent -= 1;
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

void PrintingVisitor::visit(IfStatement &visited)
{
    out << L"IfStatement " << visited.getPosition() << L"\n" << std::wstring(indent, L' ');
    indent += 1;
    if(visited.body.size() > 0)
        out << L"|-";
    else
        out << L"`-";
    if(std::holds_alternative<VariableDeclStatement>(visited.condition))
        std::get<VariableDeclStatement>(visited.condition).accept(*this);
    else
        std::get<std::unique_ptr<Expression>>(visited.condition)->accept(*this);
    visitInstructionBlock(visited.body);
    indent -= 1;
}

void PrintingVisitor::visit(WhileStatement &visited)
{
    out << L"WhileStatement " << visited.getPosition() << L"\n" << std::wstring(indent, L' ');
    indent += 1;
    if(visited.body.size() > 0)
        out << L"|-";
    else
        out << L"`-";
    visited.condition->accept(*this);
    visitInstructionBlock(visited.body);
    indent -= 1;
}

void PrintingVisitor::visit(DoWhileStatement &visited)
{
    out << L"DoWhileStatement " << visited.getPosition() << L"\n" << std::wstring(indent, L' ');
    indent += 1;
    if(visited.body.size() > 0)
        out << L"|-";
    else
        out << L"`-";
    visited.condition->accept(*this);
    visitInstructionBlock(visited.body);
    indent -= 1;
}

void PrintingVisitor::visit(Field &visited)
{
    out << L"Field " << visited.getPosition() << L" type=" << visited.type << L" name=" << visited.name << L"\n";
}

void PrintingVisitor::visit(StructDeclaration &visited)
{
    out << L"StructDeclaration " << visited.getPosition() << L"\n";
    indent += 1;
    visitFields(visited.fields);
    indent -= 1;
}

void PrintingVisitor::visit(VariantDeclaration &visited)
{
    out << L"VariantDeclaration " << visited.getPosition() << L"\n";
    indent += 1;
    visitFields(visited.fields);
    indent -= 1;
}

void PrintingVisitor::visit(FunctionDeclaration &visited)
{
    out << L"FunctionDeclaration " << visited.getPosition();
    if(visited.returnType.has_value())
        out << L" returnType=" << *visited.returnType;
    out << L"\n";
    indent += 1;
    if(visited.parameters.size() > 0)
    {
        out << std::wstring(indent, L' ') << L"Parameters:\n";
        for(auto &parameter: visited.parameters)
        {
            out << std::wstring(indent, L' ');
            if(&parameter != &visited.parameters.back())
                out << L"|-";
            else
                out << L"`-";
            parameter.accept(*this);
        }
    }
    if(visited.body.size() > 0)
    {
        out << std::wstring(indent, L' ') << L"Body:\n";
        visitInstructionBlock(visited.body);
    }
    indent -= 1;
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
        out << std::wstring(indent, L' ') << L"Includes:\n";
        for(auto &includeStatement: visited.includes)
        {
            out << std::wstring(indent, L' ');
            if(&includeStatement != &visited.includes.back())
                out << L"|-";
            else
                out << L"`-";
            includeStatement.accept(*this);
        }
    }
    if(visited.structs.size() > 0)
    {
        out << std::wstring(indent, L' ') << L"Structs:\n";
        for(auto it = visited.structs.begin(); it != visited.structs.end(); it++)
        {
            out << std::wstring(indent, L' ');
            if(std::next(it) != visited.structs.end())
                out << L"|-";
            else
                out << L"`-";
            out << it->first << L": ";
            it->second.accept(*this);
        }
    }
    if(visited.variants.size() > 0)
    {
        out << std::wstring(indent, L' ') << L"Variants:\n";
        for(auto it = visited.variants.begin(); it != visited.variants.end(); it++)
        {
            out << std::wstring(indent, L' ');
            if(std::next(it) != visited.variants.end())
                out << L"|-";
            else
                out << L"`-";
            out << it->first << L": ";
            it->second.accept(*this);
        }
    }
    if(visited.functions.size() > 0)
    {
        out << std::wstring(indent, L' ') << L"Functions:\n";
        for(auto it = visited.functions.begin(); it != visited.functions.end(); it++)
        {
            out << std::wstring(indent, L' ');
            if(std::next(it) != visited.functions.end())
                out << L"|-";
            else
                out << L"`-";
            out << it->first.name << L": ";
            it->second.accept(*this);
        }
    }
}
