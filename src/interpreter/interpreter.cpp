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
):
    currentSource(programSource), arguments(arguments), input(input), output(output), parseFromFile(parseFromFile),
    shouldReturn(false), shouldContinue(false), shouldBreak(false)
{}

#define EMPTY_VISIT(type) \
    void Interpreter::visit(type &) {}

namespace {
template <typename... Types>
Object &getObject(std::variant<Types...> &variant)
{
    if(std::holds_alternative<std::reference_wrapper<Object>>(variant))
        return std::get<std::reference_wrapper<Object>>(variant).get();
    else
        return std::get<Object>(variant);
}

Object &getField(
    Object &structure, std::unordered_map<std::wstring, StructDeclaration>::iterator structFound,
    const std::wstring &fieldName
)
{
    const std::vector<Field> &fields = structFound->second.fields;
    unsigned fieldIndex = std::find_if(
                              fields.begin(), fields.end(), [&](const Field &field) { return field.name == fieldName; }
                          ) -
                          fields.begin();
    return std::get<std::vector<Object>>(structure.value)[fieldIndex];
}
}

Object &Interpreter::getVariable(const std::wstring &name)
{
    for(auto &scope: variables.top())
    {
        auto found = scope.find(name);
        if(found != scope.end())
            return getObject(found->second);
    }
    throw RuntimeSemanticException("Variable not found");
}

void Interpreter::addVariable(const std::wstring &name, const Object &object)
{
    variables.top()[variables.top().size() - 1].insert({name, object});
}

void Interpreter::addVariable(const std::wstring &name, std::reference_wrapper<Object> object)
{
    variables.top()[variables.top().size() - 1].insert({name, object});
}

Object &Interpreter::getLastResult()
{
    return getObject(lastResult);
}

template <typename LeftType, typename RightType, typename BinaryOperation>
std::pair<LeftType, RightType> Interpreter::getBinaryOpArgs(BinaryOperation &visited)
{
    visited.left->accept(*this);
    return getBinaryOpArgsLeftAccepted<LeftType, RightType>(visited);
}

template <typename LeftType, typename RightType, typename BinaryOperation>
std::pair<LeftType, RightType> Interpreter::getBinaryOpArgsLeftAccepted(BinaryOperation &visited)
{
    LeftType left = std::get<LeftType>(getLastResult().value);
    visited.right->accept(*this);
    RightType right = std::get<RightType>(getLastResult().value);
    return {left, right};
}

int32_t Interpreter::addIntegers(int32_t left, int32_t right, Position position)
{
    if(left >= 0)
    {
        if(std::numeric_limits<int32_t>::max() - left < right)
            throw IntegerRangeError(L"Addition or subtraction of integers would overflow", currentSource, position);
    }
    else
    {
        if(right < std::numeric_limits<int32_t>::min() - left)
            throw IntegerRangeError(L"Addition or subtraction of integers would overflow", currentSource, position);
    }
    return left + right;
}

void Interpreter::visitInstructionBlock(std::vector<std::unique_ptr<Instruction>> &block)
{
    for(auto &instruction: block)
    {
        instruction->accept(*this);
        if(shouldReturn || shouldBreak || shouldContinue)
            return;
    }
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

void Interpreter::visit(PlusExpression &visited)
{
    visited.left->accept(*this);
    if(getLastResult().type == Type{INT})
    {
        auto [left, right] = getBinaryOpArgsLeftAccepted<int32_t, int32_t>(visited);
        lastResult = Object{{INT}, addIntegers(left, right, visited.getPosition())};
    }
    else
    {
        auto [left, right] = getBinaryOpArgsLeftAccepted<double, double>(visited);
        lastResult = Object{{FLOAT}, left + right};
    }
}

void Interpreter::visit(MinusExpression &visited)
{
    visited.left->accept(*this);
    if(getLastResult().type == Type{INT})
    {
        auto [left, right] = getBinaryOpArgsLeftAccepted<int32_t, int32_t>(visited);
        if(right > std::numeric_limits<int32_t>::min())
            lastResult = Object{{INT}, addIntegers(left, -right, visited.getPosition())};
        else if(left > std::numeric_limits<int32_t>::min())
            lastResult = Object{{INT}, -addIntegers(-left, right, visited.getPosition())};
        else
            throw IntegerRangeError(L"Subtraction of integers would overflow", currentSource, visited.getPosition());
    }
    else
    {
        auto [left, right] = getBinaryOpArgsLeftAccepted<double, double>(visited);
        lastResult = Object{{FLOAT}, left - right};
    }
}

void Interpreter::visit(MultiplyExpression &) {}

void Interpreter::visit(DivideExpression &) {}

void Interpreter::visit(FloorDivideExpression &) {}

void Interpreter::visit(ModuloExpression &) {}

void Interpreter::visit(ExponentExpression &) {}

void Interpreter::visit(UnaryMinusExpression &) {}

void Interpreter::visit(NotExpression &) {}

void Interpreter::visit(SubscriptExpression &) {}

void Interpreter::visit(DotExpression &visited)
{
    visited.value->accept(*this);
    Object &left = getLastResult();
    lastResult = getField(left, program->structs.find(std::get<std::wstring>(left.type.value)), visited.field);
}

void Interpreter::visit(StructExpression &visited)
{
    std::vector<Object> fields;
    for(auto &argument: visited.arguments)
    {
        argument->accept(*this);
        fields.push_back(getLastResult());
    }
    lastResult = Object{{*visited.structType}, fields};
}

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
        throw CastImpossibleError(
            std::format(L"Conversion of float {} to integer would result in a value out of integer range", value),
            currentSource, position
        );
    return static_cast<int32_t>(value);
}

template <typename NumberType>
NumberType Interpreter::fromString(const std::wstring &value, Position position, const std::wstring &typeName)
{
    std::wstringstream stream(value);
    NumberType converted;
    stream >> converted;
    if(stream.fail())
        throw CastImpossibleError(
            std::format(L"Conversion of string {} to {} failed", value, typeName), currentSource, position
        );
    std::wstring rest;
    rest.resize(value.size());
    stream.read(&rest[0], value.size());
    rest.resize(stream.gcount());
    if(!std::all_of(rest.begin(), rest.end(), isspace))
        throw CastImpossibleError(
            std::format(L"Conversion of string {} to {} failed", value, typeName), currentSource, position
        );
    return converted;
}

template <>
int32_t Interpreter::cast(std::wstring value, Position position)
{
    return fromString<int32_t>(value, position, L"integer");
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
double Interpreter::cast(std::wstring value, Position position)
{
    return fromString<double>(value, position, L"float");
}

template <>
double Interpreter::cast(bool value, Position)
{
    return static_cast<double>(value);
}

template <>
std::wstring Interpreter::cast(int32_t value, Position)
{
    return std::format(L"{}", value);
}

template <>
std::wstring Interpreter::cast(double value, Position)
{
    return std::format(L"{}", value);
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
bool Interpreter::cast(std::wstring value, Position)
{
    return value.size() > 0;
}

template <typename TargetType>
Object Interpreter::getCastedObject(Type::Builtin targetType, Position position)
{
    return Object(
        {targetType},
        std::visit([&](const auto &value) { return cast<TargetType>(value, position); }, getLastResult().value)
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
    if(structFound == program->structs.end())
        return; // variant access case - leave lastResult as is

    lastResult = std::reference_wrapper(getField(left, structFound, visited.right));
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

void Interpreter::visit(ReturnStatement &visited)
{
    if(visited.returnValue)
        visited.returnValue->accept(*this);
    shouldReturn = true;
}

void Interpreter::visit(ContinueStatement &)
{
    shouldContinue = true;
}

void Interpreter::visit(BreakStatement &)
{
    shouldBreak = true;
}

void Interpreter::visit(SingleIfCase &visited)
{
    if(std::holds_alternative<VariableDeclStatement>(visited.condition))
        visit(std::get<VariableDeclStatement>(visited.condition));
    else
        std::get<std::unique_ptr<Expression>>(visited.condition)->accept(*this);
}

void Interpreter::visit(IfStatement &visited)
{
    bool executeElse = true;
    for(SingleIfCase &singleCase: visited.cases)
    {
        visit(singleCase);
        if(std::get<bool>(getLastResult().value))
        {
            executeElse = false;
            visitInstructionBlock(singleCase.body);
            break;
        }
    }
    if(executeElse)
        visitInstructionBlock(visited.elseCaseBody);
}

#define HANDLE_LOOP_FLAGS           \
    if(shouldBreak || shouldReturn) \
    {                               \
        shouldBreak = false;        \
        return;                     \
    }                               \
    if(shouldContinue)              \
    {                               \
        shouldContinue = false;     \
        break;                      \
    }

void Interpreter::visit(WhileStatement &visited)
{
    while(visited.condition->accept(*this), std::get<bool>(getLastResult().value))
    {
        visitInstructionBlock(visited.body);
        HANDLE_LOOP_FLAGS;
    }
}

void Interpreter::visit(DoWhileStatement &visited)
{
    do
    {
        visitInstructionBlock(visited.body);
        HANDLE_LOOP_FLAGS;
    }
    while(visited.condition->accept(*this), std::get<bool>(getLastResult().value));
}

void Interpreter::visit(FunctionDeclaration &visited)
{
    callPosition = visited.getPosition();
    currentSource = visited.getSource();
    variables.push({{}});
    for(unsigned i = 0; i < functionArguments.size(); i++)
        addVariable(visited.parameters.at(i).name, functionArguments.at(i));
    visitInstructionBlock(visited.body);
    shouldReturn = false;
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
