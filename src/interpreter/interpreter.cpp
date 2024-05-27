#include "interpreter.hpp"

#include "builtinFunctions.hpp"
#include "includeExecution.hpp"
#include "runtimeExceptions.hpp"
#include "semanticAnalysis.hpp"

#include <cmath>

using enum Type::Builtin;

Interpreter::Interpreter(
    std::wstring programSource, std::vector<std::wstring> arguments, std::wistream &input, std::wostream &output,
    std::function<Program(std::wifstream &, std::wstring)> parseFromFile
): currentSource(programSource), arguments(arguments), input(input), output(output), parseFromFile(parseFromFile)
{}

#define EMPTY_VISIT(type) \
    void Interpreter::visit(type &) {}

Object &Interpreter::getVariable(const std::wstring &name)
{
    for(auto &scope: variables.top())
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

const std::vector<Field> &Interpreter::getFields(const std::wstring &typeName)
{
    auto structFound = program->structs.find(typeName);
    if(structFound != program->structs.end())
        return structFound->second.fields;
    return program->variants.at(typeName).fields;
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
    lastResult = std::reference_wrapper(getVariable(visited.name));
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

template <typename TargetType, typename SourceType>
TargetType Interpreter::cast(SourceType, Position)
{
    throw RuntimeSemanticException("Invalid cast detected");
}

template <>
int32_t Interpreter::cast(double value, Position position)
{
    value = std::round(value);
    if(value < std::numeric_limits<int32_t>::min() || value > std::numeric_limits<int32_t>::max() || std::isnan(value))
        throw IntegerRangeError(
            std::format(L"Conversion of float {} to integer would result in a value out of integer range", value),
            currentSource, position
        );
    return static_cast<int32_t>(value);
}

template <>
int32_t Interpreter::cast(const std::wstring &value, Position position)
{
    long converted;
    try
    {
        converted = std::stol(value);
    }
    catch(const std::invalid_argument &e)
    {
        throw CastImpossibleError(
            std::format(L"Conversion of string {} to integer failed", value), currentSource, position
        );
    }
    catch(const std::out_of_range &e)
    {
        throw IntegerRangeError(
            std::format(L"Conversion of string {} to integer would result in a value out of integer range", value),
            currentSource, position
        );
    }
    if(converted < std::numeric_limits<int32_t>::min() || converted > std::numeric_limits<int32_t>::max())
        throw IntegerRangeError(
            std::format(L"Conversion of string {} to integer would result in a value out of integer range", value),
            currentSource, position
        );
    return static_cast<int32_t>(converted);
}

template <>
int32_t Interpreter::cast(bool value, Position)
{
    return static_cast<int32_t>(value);
}

template <>
double Interpreter::cast(int32_t value, Position)
{
    return static_cast<double>(value);
}

template <>
double Interpreter::cast(const std::wstring &value, Position position)
{
    try
    {
        return std::stod(value);
    }
    catch(const std::invalid_argument &e)
    {
        throw CastImpossibleError(
            std::format(L"Conversion of string {} to integer failed", value), currentSource, position
        );
    }
    catch(const std::out_of_range &e)
    {
        throw IntegerRangeError(
            std::format(L"Conversion of string {} to integer would result in a value out of integer range", value),
            currentSource, position
        );
    }
}

template <>
double Interpreter::cast(bool value, Position)
{
    return static_cast<double>(value);
}

template <>
std::wstring Interpreter::cast(int32_t value, Position)
{
    return std::to_wstring(value);
}

template <>
std::wstring Interpreter::cast(double value, Position)
{
    return std::to_wstring(value);
}

template <>
std::wstring Interpreter::cast(bool value, Position)
{
    if(value)
        return L"true";
    else
        return L"false";
}

template <>
bool Interpreter::cast(int32_t value, Position)
{
    return value != 0;
}

template <>
bool Interpreter::cast(double value, Position)
{
    return value != 0;
}

template <>
bool Interpreter::cast(const std::wstring &value, Position)
{
    return value.size() > 0;
}

template <typename TargetType>
Object Interpreter::getCastedObject(Type::Builtin targetType, Position position)
{
    return Object(
        {targetType}, std::visit([&](auto value) { return cast<TargetType>(value, position); }, getLastResult().value)
    );
}

void Interpreter::visit(CastExpression &visited)
{
    visited.value->accept(*this);
    if(visited.targetType.isBuiltin())
    {
        Type::Builtin type = std::get<Type::Builtin>(visited.targetType.value);
        switch(type)
        {
        case INT:
            lastResult = getCastedObject<int32_t>(type, visited.getPosition());
            break;
        case STR:
            lastResult = getCastedObject<std::wstring>(type, visited.getPosition());
            break;
        case FLOAT:
            lastResult = getCastedObject<double>(type, visited.getPosition());
            break;
        case BOOL:
            lastResult = getCastedObject<bool>(type, visited.getPosition());
        }
    }
    else // cast to variant type case
        getLastResult().type = visited.targetType;
}

void Interpreter::visit(VariableDeclStatement &visited)
{
    visited.value->accept(*this);
    addVariable(visited.declaration.name, getLastResult());
}

void Interpreter::visit(Assignable &visited)
{
    if(!visited.left)
    {
        lastResult = std::reference_wrapper(getVariable(visited.right));
        return;
    }
    visit(*visited.left);
    Object &left = getLastResult();
    std::wstring typeName = std::get<std::wstring>(left.type.value);

    auto structFound = program->structs.find(typeName);
    if(structFound != program->structs.end())
        return; // variant access case - leave lastResult as is

    const std::vector<Field> &fields = structFound->second.fields;
    unsigned fieldIndex = std::find_if(
                              fields.begin(), fields.end(),
                              [&](const Field &field) { return field.name == visited.right; }
                          ) -
                          fields.begin();
    lastResult = std::reference_wrapper(std::get<std::vector<Object>>(left.value)[fieldIndex]);
}

void Interpreter::visit(AssignmentStatement &visited)
{
    visit(visited.left);
    Object &assignmentTarget = getLastResult();
    visited.right->accept(*this);
    Object &value = getLastResult();
    assignmentTarget.value = value.value;
}

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
    program->functions.at(FunctionIdentification(visited.functionName, argumentTypes))->accept(*this);
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

void Interpreter::visit(Program &visited)
{
    Program fullProgram = prepareBuiltinFunctions(visited.getPosition(), arguments, input, output);
    mergePrograms(fullProgram, visited);
    executeIncludes(fullProgram, currentSource, parseFromFile);
    doSemanticAnalysis(fullProgram);
    if(fullProgram.functions.count({L"main", {}}) == 0)
        throw MainNotFoundError(
            L"main function has not been found in the program", currentSource, fullProgram.getPosition()
        );
    program = &fullProgram;
    auto &main = fullProgram.functions.at({L"main", {}});
    if(main->returnType)
        throw MainReturnTypeError(
            std::format(L"main function should not return a type, returns {}", *main->returnType), currentSource,
            main->getPosition()
        );
    fullProgram.functions.at({L"main", {}})->accept(*this);
}

EMPTY_VISIT(VariableDeclaration);
EMPTY_VISIT(IncludeStatement);
EMPTY_VISIT(Field);
EMPTY_VISIT(StructDeclaration);
EMPTY_VISIT(VariantDeclaration);
