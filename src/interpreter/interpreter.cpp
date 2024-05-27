#include "interpreter.hpp"

#include "builtinFunctions.hpp"
#include "includeExecution.hpp"
#include "runtimeExceptions.hpp"
#include "semanticAnalysis.hpp"

Interpreter::Interpreter(
    std::wstring programSource, std::vector<std::wstring> arguments, std::wistream &input, std::wostream &output,
    std::function<Program(std::wifstream &, std::wstring)> parseFromFile
): currentSource(programSource), arguments(arguments), input(input), output(output), parseFromFile(parseFromFile)
{}

#define EMPTY_VISIT(type) \
    void Interpreter::visit(type &) {}

Object &Interpreter::getVariable(const std::wstring &name)
{
    for(auto scope: variables.top())
    {
        auto found = scope.find(name);
        if(found != scope.end())
            return found->second;
    }
    throw RuntimeSemanticException("Variable not found");
}

void Interpreter::addVariable(const std::wstring &name, const Object &object)
{
    variables.top()[variables.top().size() - 1].insert({name, object});
}

Object &Interpreter::getLastResult()
{
    if(std::holds_alternative<std::reference_wrapper<Object>>(lastResult))
        return std::get<std::reference_wrapper<Object>>(lastResult).get();
    else
        return std::get<Object>(lastResult);
}

void Interpreter::visit(Literal &visited)
{
    lastResult = Object(
        visited.getType(),
        std::visit(
            [](auto value) -> std::variant<std::wstring, int32_t, double, bool, std::vector<Object>> { return value; },
            visited.value
        )
    );
}

void Interpreter::visit(Variable &visited)
{
    lastResult = getVariable(visited.name);
}

void Interpreter::visit(IsExpression &) {}

void Interpreter::visit(OrExpression &) {}

void Interpreter::visit(XorExpression &) {}

void Interpreter::visit(AndExpression &) {}

void Interpreter::visit(EqualExpression &) {}

void Interpreter::visit(NotEqualExpression &) {}

void Interpreter::visit(IdenticalExpression &) {}

void Interpreter::visit(NotIdenticalExpression &) {}

void Interpreter::visit(ConcatExpression &) {}

void Interpreter::visit(StringMultiplyExpression &) {}

void Interpreter::visit(GreaterExpression &) {}

void Interpreter::visit(LesserExpression &) {}

void Interpreter::visit(GreaterEqualExpression &) {}

void Interpreter::visit(LesserEqualExpression &) {}

void Interpreter::visit(PlusExpression &) {}

void Interpreter::visit(MinusExpression &) {}

void Interpreter::visit(MultiplyExpression &) {}

void Interpreter::visit(DivideExpression &) {}

void Interpreter::visit(FloorDivideExpression &) {}

void Interpreter::visit(ModuloExpression &) {}

void Interpreter::visit(ExponentExpression &) {}

void Interpreter::visit(UnaryMinusExpression &) {}

void Interpreter::visit(NotExpression &) {}

void Interpreter::visit(SubscriptExpression &) {}

void Interpreter::visit(DotExpression &) {}

void Interpreter::visit(StructExpression &) {}

void Interpreter::visit(CastExpression &) {}

void Interpreter::visit(VariableDeclaration &) {}

void Interpreter::visit(VariableDeclStatement &visited)
{
    visited.value->accept(*this);
    addVariable(visited.declaration.name, getLastResult());
}

void Interpreter::visit(Assignable &) {}

void Interpreter::visit(AssignmentStatement &) {}

void Interpreter::visit(FunctionCall &visited)
{
    functionArguments.clear();
    for(auto &argument: visited.arguments)
    {
        argument->accept(*this);
        functionArguments.push_back(getLastResult());
    }
    std::vector<Type> argumentTypes;
    for(auto &argument: functionArguments)
        argumentTypes.push_back(argument.get().type);
    functions->at(FunctionIdentification(visited.functionName, argumentTypes))->accept(*this);
}

void Interpreter::visit(FunctionCallInstruction &visited)
{
    visit(visited.functionCall);
}

void Interpreter::visit(ReturnStatement &) {}

void Interpreter::visit(ContinueStatement &) {}

void Interpreter::visit(BreakStatement &) {}

void Interpreter::visit(SingleIfCase &) {}

void Interpreter::visit(IfStatement &) {}

void Interpreter::visit(WhileStatement &) {}

void Interpreter::visit(DoWhileStatement &) {}

void Interpreter::visit(Field &) {}

void Interpreter::visit(StructDeclaration &) {}

void Interpreter::visit(VariantDeclaration &) {}

void Interpreter::visit(FunctionDeclaration &visited)
{
    callPosition = visited.getPosition();
    currentSource = visited.getSource();
    variables.push({{}});
    for(unsigned i = 0; i < functionArguments.size(); i++)
        addVariable(visited.parameters.at(i).name, functionArguments.at(i));
    for(auto &instruction: visited.body)
        instruction->accept(*this);
    variables.pop();
}

void Interpreter::visit(BuiltinFunctionDeclaration &visited)
{
    auto result = visited.body(callPosition, currentSource, functionArguments);
    if(result)
        lastResult = *result;
}

void Interpreter::visit(IncludeStatement &) {}

void Interpreter::visit(Program &visited)
{
    Program program = prepareBuiltinFunctions(visited.getPosition(), arguments, input, output);
    mergePrograms(program, visited);
    executeIncludes(program, currentSource, parseFromFile);
    doSemanticAnalysis(program);
    if(program.functions.count({L"main", {}}) == 0)
        throw MainNotFoundError(
            L"main function has not been found in the program", currentSource, program.getPosition()
        );
    functions = &program.functions;
    program.functions.at({L"main", {}})->accept(*this);
}
