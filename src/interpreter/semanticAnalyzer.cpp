#include "semanticAnalyzer.hpp"

void SemanticAnalyzer::doSemanticAnalysis(Program &program)
{
    SemanticAnalyzer(program).visit(program);
}

SemanticAnalyzer::SemanticAnalyzer(Program &visitedProgram): visitedProgram(visitedProgram) {}

void SemanticAnalyzer::visit(Literal &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(Variable &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(OrExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(XorExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(AndExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(EqualExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(NotEqualExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(IdenticalExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(NotIdenticalExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(ConcatExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(StringMultiplyExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(GreaterExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(LesserExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(GreaterEqualExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(LesserEqualExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(PlusExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(MinusExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(MultiplyExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(DivideExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(FloorDivideExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(ModuloExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(ExponentExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(UnaryMinusExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(NotExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(IsExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(SubscriptExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(DotExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(StructExpression &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(VariableDeclaration &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(VariableDeclStatement &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(Assignable &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(AssignmentStatement &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(FunctionCall &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(FunctionCallInstruction &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(ReturnStatement &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(ContinueStatement &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(BreakStatement &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(SingleIfCase &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(IfStatement &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(WhileStatement &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(DoWhileStatement &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(Field &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(StructDeclaration &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(VariantDeclaration &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(FunctionDeclaration &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(IncludeStatement &visited)
{
    static_cast<void>(visited);
}

void SemanticAnalyzer::visit(Program &visited)
{
    static_cast<void>(visited);
}
