#include "semanticAnalysis.hpp"

namespace {
class SemanticAnalyzer: public DocumentTreeVisitor
{
public:
    explicit SemanticAnalyzer(Program &visitedProgram): visitedProgram(visitedProgram) {}

    void visit(Literal &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(Variable &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(OrExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(XorExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(AndExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(EqualExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(NotEqualExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(IdenticalExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(NotIdenticalExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(ConcatExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(StringMultiplyExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(GreaterExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(LesserExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(GreaterEqualExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(LesserEqualExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(PlusExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(MinusExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(MultiplyExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(DivideExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(FloorDivideExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(ModuloExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(ExponentExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(UnaryMinusExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(NotExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(IsExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(SubscriptExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(DotExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(StructExpression &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(VariableDeclaration &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(VariableDeclStatement &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(Assignable &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(AssignmentStatement &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(FunctionCall &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(FunctionCallInstruction &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(ReturnStatement &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(ContinueStatement &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(BreakStatement &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(SingleIfCase &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(IfStatement &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(WhileStatement &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(DoWhileStatement &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(Field &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(StructDeclaration &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(VariantDeclaration &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(FunctionDeclaration &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(IncludeStatement &visited) override
    {
        static_cast<void>(visited);
    }

    void visit(Program &visited) override
    {
        static_cast<void>(visited);
    }
private:
    Program &visitedProgram;
};
}

void doSemanticAnalysis(Program &program)
{
    SemanticAnalyzer(program).visit(program);
}
