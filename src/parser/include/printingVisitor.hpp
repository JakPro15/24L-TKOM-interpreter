#ifndef PRINTINGVISITOR_HPP
#define PRINTINGVISITOR_HPP

#include "documentTree.hpp"
#include "position.hpp"

#include <memory>
#include <ostream>
#include <unordered_map>
#include <vector>

class PrintingVisitor: public DocumentTreeVisitor
{
public:
    explicit PrintingVisitor(std::wostream &out);
    void visit(Literal &visited) override;
    void visit(Variable &visited) override;
    void visit(OrExpression &visited) override;
    void visit(XorExpression &visited) override;
    void visit(AndExpression &visited) override;
    void visit(EqualExpression &visited) override;
    void visit(NotEqualExpression &visited) override;
    void visit(IdenticalExpression &visited) override;
    void visit(NotIdenticalExpression &visited) override;
    void visit(ConcatExpression &visited) override;
    void visit(StringMultiplyExpression &visited) override;
    void visit(GreaterExpression &visited) override;
    void visit(LesserExpression &visited) override;
    void visit(GreaterEqualExpression &visited) override;
    void visit(LesserEqualExpression &visited) override;
    void visit(PlusExpression &visited) override;
    void visit(MinusExpression &visited) override;
    void visit(MultiplyExpression &visited) override;
    void visit(DivideExpression &visited) override;
    void visit(FloorDivideExpression &visited) override;
    void visit(ModuloExpression &visited) override;
    void visit(ExponentExpression &visited) override;
    void visit(UnaryMinusExpression &visited) override;
    void visit(NotExpression &visited) override;
    void visit(IsExpression &visited) override;
    void visit(SubscriptExpression &visited) override;
    void visit(DotExpression &visited) override;
    void visit(StructExpression &visited) override;
    void visit(CastExpression &visited) override;
    void visit(VariableDeclaration &visited) override;
    void visit(VariableDeclStatement &visited) override;
    void visit(Assignable &visited) override;
    void visit(AssignmentStatement &visited) override;
    void visit(FunctionCall &visited) override;
    void visit(FunctionCallInstruction &visited) override;
    void visit(ReturnStatement &visited) override;
    void visit(ContinueStatement &visited) override;
    void visit(BreakStatement &visited) override;
    void visit(SingleIfCase &visited) override;
    void visit(IfStatement &visited) override;
    void visit(WhileStatement &visited) override;
    void visit(DoWhileStatement &visited) override;
    void visit(Field &visited) override;
    void visit(StructDeclaration &visited) override;
    void visit(VariantDeclaration &visited) override;
    void visit(FunctionDeclaration &visited) override;
    void visit(BuiltinFunctionDeclaration &visited) override;
    void visit(IncludeStatement &visited) override;
    void visit(Program &visited) override;
    void visit(std::pair<const FunctionIdentification, std::unique_ptr<BaseFunctionDeclaration>> &visited);

    template <typename NodeContainer>
    void visitContainer(NodeContainer &visited)
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
    void visitCondition(ConditionalStatement &visited)
    {
        if(!visited.body.empty())
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

    template <typename Node>
    void visit(std::pair<const std::wstring, Node> &visited)
    {
        out << visited.first << L": ";
        visited.second.accept(*this);
    }

    template <typename Node>
    void visit(std::unique_ptr<Node> &visited)
    {
        visited->accept(*this);
    }

    template <typename... Types>
    void visit(std::variant<Types...> &visited)
    {
        std::visit([&](auto &value) { visit(value); }, visited);
    }
private:
    std::wostream &out;
    std::wstring indent;
    void popIndent();

    void visitUnaryOperation(std::wstring name, Position position, DocumentTreeNode &child);
    void visitBinaryOperation(std::wstring name, BinaryOperation &visited);
};

#endif
