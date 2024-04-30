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
    PrintingVisitor(std::wostream &out);
    void visit(Literal &visited) override;
    void visit(IsExpression &visited) override;
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
    void visit(SubscriptExpression &visited) override;
    void visit(DotExpression &visited) override;
    void visit(StructExpression &visited) override;
    void visit(VariableDeclaration &visited) override;
    void visit(VariableDeclStatement &visited) override;
    void visit(Assignable &visited) override;
    void visit(AssignmentStatement &visited) override;
    void visit(FunctionCall &visited) override;
    void visit(ReturnStatement &visited) override;
    void visit(ContinueStatement &visited) override;
    void visit(BreakStatement &visited) override;
    void visit(IfStatement &visited) override;
    void visit(WhileStatement &visited) override;
    void visit(DoWhileStatement &visited) override;
    void visit(Field &visited) override;
    void visit(StructDeclaration &visited) override;
    void visit(VariantDeclaration &visited) override;
    void visit(FunctionDeclaration &visited) override;
    void visit(IncludeStatement &visited) override;
    void visit(Program &visited) override;
private:
    std::wostream &out;
    int indent;
    void visitUnaryOperation(std::wstring name, Position position, DocumentTreeNode &child);
    void visitBinaryOperation(std::wstring name, BinaryOperation &visited);
    void visitInstructionBlock(std::vector<std::unique_ptr<Instruction>> &block);
    void visitFields(std::vector<Field> &fields);
};

#endif
