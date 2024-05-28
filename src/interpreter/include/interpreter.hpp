#include "documentTree.hpp"
#include "documentTreeVisitor.hpp"

#include <functional>
#include <iostream>
#include <stack>
#include <string>
#include <vector>

class Interpreter: public DocumentTreeVisitor
{
public:
    Interpreter(
        std::wstring programSource, std::vector<std::wstring> arguments, std::wistream &input, std::wostream &output,
        std::function<Program(std::wifstream &, std::wstring)> parseFromFile
    );
    void visit(Program &visited) override;
private:
    Position callPosition;
    std::wstring currentSource;
    std::vector<std::wstring> arguments;
    std::wistream &input;
    std::wostream &output;
    std::function<Program(std::wifstream &, std::wstring)> parseFromFile;
    std::variant<Object, std::reference_wrapper<Object>> lastResult;
    std::stack<std::vector<std::unordered_map<std::wstring, std::variant<Object, std::reference_wrapper<Object>>>>>
        variables;
    Program *program;
    std::vector<std::variant<Object, std::reference_wrapper<Object>>> functionArguments;
    bool shouldReturn, shouldContinue, shouldBreak;

    Object &getVariable(const std::wstring &name);
    void addVariable(const std::wstring &name, Object &&object);
    void addVariable(const std::wstring &name, std::reference_wrapper<Object> object);
    Object getLastResultValue();
    Object &getLastResultReference();

    template <typename NumberType>
    NumberType fromString(const std::wstring &value, Position position, const std::wstring &typeName);
    template <typename TargetType, typename SourceType>
    TargetType cast(const SourceType &value, Position position);
    template <typename TargetType>
    Object getCastedObject(Type::Builtin targetType, Position position);

    template <typename LeftType, typename RightType, typename BinaryOperation>
    std::pair<LeftType, RightType> getBinaryOpArgs(BinaryOperation &visited);
    template <typename LeftType, typename RightType, typename BinaryOperation>
    std::pair<LeftType, RightType> getBinaryOpArgsLeftAccepted(BinaryOperation &visited);
    template <typename BinaryOperation>
    std::pair<Object, Object> getBinaryOpArgsObjects(BinaryOperation &visited);

    template <typename BinaryOperation>
    void doComparison(BinaryOperation &visited, auto compare);

    int32_t addIntegers(int32_t left, int32_t right, Position position);

    void visitInstructionBlock(std::vector<std::unique_ptr<Instruction>> &block);
    void visitInstructionScope(std::vector<std::unique_ptr<Instruction>> &block);

    void visit(Literal &visited) override;
    void visit(Variable &visited) override;
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
};
