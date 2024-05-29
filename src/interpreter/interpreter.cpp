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

void Interpreter::addVariable(const std::wstring &name, Object &&object)
{
    variables.top()[variables.top().size() - 1].insert({name, std::move(object)});
}

void Interpreter::addVariable(const std::wstring &name, std::reference_wrapper<Object> object)
{
    variables.top()[variables.top().size() - 1].insert({name, object});
}

Object Interpreter::getLastResultValue()
{
    if(std::holds_alternative<std::reference_wrapper<Object>>(lastResult))
        return Object(std::get<std::reference_wrapper<Object>>(lastResult).get());
    else
        return std::move(std::get<Object>(lastResult));
}

Object &Interpreter::getLastResultReference()
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
    LeftType left = std::get<LeftType>(getLastResultReference().value);
    visited.right->accept(*this);
    RightType right = std::get<RightType>(getLastResultReference().value);
    return {left, right};
}

template <typename BinaryOperation>
std::pair<Object, Object> Interpreter::getBinaryOpArgsObjects(BinaryOperation &visited)
{
    visited.left->accept(*this);
    Object left = getLastResultValue();
    visited.right->accept(*this);
    Object right = getLastResultValue();
    return {std::move(left), std::move(right)};
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

void Interpreter::visitInstructionScope(std::vector<std::unique_ptr<Instruction>> &block)
{
    variables.top().emplace_back();
    visitInstructionBlock(block);
    variables.top().pop_back();
}

void Interpreter::visit(Literal &visited)
{
    lastResult = Object(
        visited.getType(),
        std::visit(
            [](auto &value
            ) -> std::variant<std::wstring, int32_t, double, bool, std::vector<Object>, std::unique_ptr<Object>> {
                return value;
            },
            visited.value
        )
    );
}

void Interpreter::visit(Variable &visited)
{
    lastResult = getVariable(visited.name);
}

void Interpreter::visit(IsExpression &visited)
{
    visited.left->accept(*this);
    Object &value = getLastResultReference();
    if(value.type.isBuiltin() || program->structs.count(std::get<std::wstring>(value.type.value)) == 1)
    {
        lastResult = Object{{BOOL}, value.type == visited.right};
        return;
    }
    if(value.type == visited.right)
        lastResult = Object{{BOOL}, true};
    else
    {
        Object &containedInVariant = *std::get<std::unique_ptr<Object>>(value.value).get();
        lastResult = Object{{BOOL}, containedInVariant.type == visited.right};
    }
}

void Interpreter::visit(OrExpression &visited)
{
    auto [left, right] = getBinaryOpArgs<bool, bool>(visited);
    lastResult = Object{{BOOL}, left || right};
}

void Interpreter::visit(XorExpression &visited)
{
    auto [left, right] = getBinaryOpArgs<bool, bool>(visited);
    lastResult = Object{{BOOL}, left != right};
}

void Interpreter::visit(AndExpression &visited)
{
    auto [left, right] = getBinaryOpArgs<bool, bool>(visited);
    lastResult = Object{{BOOL}, left && right};
}

bool Interpreter::isVariantType(const Type &type)
{
    return !type.isBuiltin() && program->variants.count(std::get<std::wstring>(type.value)) == 1;
}

Object &Interpreter::getNonvariantValue(const Object &variant)
{
    Object *value = std::get<std::unique_ptr<Object>>(variant.value).get();
    while(isVariantType(value->type))
        value = std::get<std::unique_ptr<Object>>(value->value).get();
    return *value;
}

std::pair<Object, Object> Interpreter::castEqualityArguments(Object &left, Object &right, Position position)
{
    Type::Builtin targetType = getTargetTypeForEquality(
        std::get<Type::Builtin>(left.type.value), std::get<Type::Builtin>(right.type.value)
    );
    auto leftCasted = doCast(targetType, left, position);
    auto rightCasted = doCast(targetType, right, position);
    return {std::move(leftCasted), std::move(rightCasted)};
}

template <typename EqualityExpression>
bool Interpreter::compareArgumentsEqual(EqualityExpression &visited)
{
    auto [left, right] = getBinaryOpArgsObjects(visited);
    if(isVariantType(left.type))
    {
        Object &leftValue = getNonvariantValue(left);
        Object &rightValue = getNonvariantValue(right);

        if(leftValue.type != rightValue.type)
        {
            if(!leftValue.type.isBuiltin() || !rightValue.type.isBuiltin())
                return false;
            auto [leftCasted, rightCasted] = castEqualityArguments(leftValue, rightValue, visited.getPosition());
            return leftCasted == rightCasted;
        }
        return leftValue.value == rightValue.value;
    }
    return left.value == right.value;
}

void Interpreter::visit(EqualExpression &visited)
{
    lastResult = Object{{BOOL}, compareArgumentsEqual(visited)};
}

void Interpreter::visit(NotEqualExpression &visited)
{
    lastResult = Object{{BOOL}, !compareArgumentsEqual(visited)};
}

void Interpreter::visit(IdenticalExpression &visited)
{
    auto [left, right] = getBinaryOpArgsObjects(visited);
    lastResult = Object{{BOOL}, left == right};
}

void Interpreter::visit(NotIdenticalExpression &visited)
{
    auto [left, right] = getBinaryOpArgsObjects(visited);
    lastResult = Object{{BOOL}, left != right};
}

void Interpreter::visit(ConcatExpression &visited)
{
    auto [left, right] = getBinaryOpArgs<std::wstring, std::wstring>(visited);
    lastResult = Object{{STR}, left + right};
}

void Interpreter::visit(StringMultiplyExpression &visited)
{
    auto [left, right] = getBinaryOpArgs<std::wstring, int32_t>(visited);
    if(right < 0)
        throw OperatorArgumentError(
            L"'@' operator's right argument must be positive", currentSource, visited.getPosition()
        );
    std::wstring result;
    for(int32_t i = 0; i < right; i++)
        result += left;
    lastResult = Object{{STR}, result};
}

template <typename BinaryOperation>
void Interpreter::doComparison(BinaryOperation &visited, auto compare)
{
    visited.left->accept(*this);
    if(getLastResultReference().type == Type{INT})
    {
        auto [left, right] = getBinaryOpArgsLeftAccepted<int32_t, int32_t>(visited);
        lastResult = Object{{BOOL}, compare(left, right)};
    }
    else
    {
        auto [left, right] = getBinaryOpArgsLeftAccepted<double, double>(visited);
        lastResult = Object{{BOOL}, compare(left, right)};
    }
}

void Interpreter::visit(GreaterExpression &visited)
{
    doComparison(visited, [&](auto left, auto right) { return left > right; });
}

void Interpreter::visit(LesserExpression &visited)
{
    doComparison(visited, [&](auto left, auto right) { return left < right; });
}

void Interpreter::visit(GreaterEqualExpression &visited)
{
    doComparison(visited, [&](auto left, auto right) { return left >= right; });
}

void Interpreter::visit(LesserEqualExpression &visited)
{
    doComparison(visited, [&](auto left, auto right) { return left <= right; });
}

namespace {
bool wouldAdditionOverflow(int32_t left, int32_t right)
{
    if(left >= 0)
        return std::numeric_limits<int32_t>::max() - left < right;
    else
        return right < std::numeric_limits<int32_t>::min() - left;
}
}

int32_t Interpreter::addIntegers(int32_t left, int32_t right, Position position)
{
    if(wouldAdditionOverflow(left, right))
        throw IntegerRangeError(L"Addition or subtraction of integers would overflow", currentSource, position);
    return left + right;
}

void Interpreter::visit(PlusExpression &visited)
{
    visited.left->accept(*this);
    if(getLastResultReference().type == Type{INT})
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
    if(getLastResultReference().type == Type{INT})
    {
        auto [left, right] = getBinaryOpArgsLeftAccepted<int32_t, int32_t>(visited);
        if(right > std::numeric_limits<int32_t>::min())
            lastResult = Object{{INT}, addIntegers(left, -right, visited.getPosition())};
        else if(left > std::numeric_limits<int32_t>::min())
        {
            int32_t result = addIntegers(-left, right, visited.getPosition());
            if(result == std::numeric_limits<int32_t>::min())
                throw IntegerRangeError(
                    L"Subtraction of integers would overflow", currentSource, visited.getPosition()
                );
            lastResult = Object{{INT}, -result};
        }
        else
            throw IntegerRangeError(L"Subtraction of integers would overflow", currentSource, visited.getPosition());
    }
    else
    {
        auto [left, right] = getBinaryOpArgsLeftAccepted<double, double>(visited);
        lastResult = Object{{FLOAT}, left - right};
    }
}

namespace {
bool wouldMultiplicationOverflow(int32_t left, int32_t right)
{
    if(right > 0)
    {
        if(left > std::numeric_limits<int32_t>::max() / right || left < std::numeric_limits<int32_t>::min() / right)
            return true;
    }
    else if(right < 0)
    {
        if(right == -1)
            return left == std::numeric_limits<int32_t>::min();

        if(left < std::numeric_limits<int32_t>::max() / right || left > std::numeric_limits<int32_t>::min() / right)
            return true;
    }
    return false;
}
}

void Interpreter::visit(MultiplyExpression &visited)
{
    visited.left->accept(*this);
    if(getLastResultReference().type == Type{INT})
    {
        auto [left, right] = getBinaryOpArgsLeftAccepted<int32_t, int32_t>(visited);
        if(wouldMultiplicationOverflow(left, right))
            throw IntegerRangeError(L"Multiplication of integers would overflow", currentSource, visited.getPosition());
        lastResult = Object{{INT}, left * right};
    }
    else
    {
        auto [left, right] = getBinaryOpArgsLeftAccepted<double, double>(visited);
        lastResult = Object{{FLOAT}, left * right};
    }
}

void Interpreter::visit(DivideExpression &visited)
{
    auto [left, right] = getBinaryOpArgs<double, double>(visited);
    if(right == 0.0)
        throw ZeroDivisionError(L"Floating-point division by zero detected", currentSource, visited.getPosition());
    lastResult = Object{{FLOAT}, left / right};
}

void Interpreter::visit(FloorDivideExpression &visited)
{
    auto [left, right] = getBinaryOpArgs<int32_t, int32_t>(visited);
    if(left == std::numeric_limits<int32_t>::min() && right == -1)
        throw IntegerRangeError(L"Division of integers would overflow", currentSource, visited.getPosition());
    if(right == 0)
        throw ZeroDivisionError(L"Floor division by zero detected", currentSource, visited.getPosition());
    lastResult = Object{{INT}, static_cast<int32_t>(std::floor(static_cast<double>(left) / right))};
}

void Interpreter::visit(ModuloExpression &visited)
{
    auto [left, right] = getBinaryOpArgs<int32_t, int32_t>(visited);
    if(right == 0)
        throw ZeroDivisionError(L"Modulo operation by zero detected", currentSource, visited.getPosition());
    if(left == std::numeric_limits<int32_t>::min() && right == -1)
    {
        lastResult = Object{{INT}, 0};
        return;
    }
    int32_t result = left % right;
    if(result < 0)
        result += right;
    lastResult = Object{{INT}, result};
}

void Interpreter::visit(ExponentExpression &visited)
{
    auto [left, right] = getBinaryOpArgs<double, double>(visited);
    if(left < 0)
        throw OperatorArgumentError(L"Exponent base must not be negative", currentSource, visited.getPosition());
    lastResult = Object{{FLOAT}, std::pow(left, right)};
}

void Interpreter::visit(UnaryMinusExpression &visited)
{
    visited.value->accept(*this);
    if(getLastResultReference().type == Type{INT})
    {
        int32_t value = std::get<int32_t>(getLastResultReference().value);
        if(value == std::numeric_limits<int32_t>::min())
            throw IntegerRangeError(L"Negation of integer would overflow", currentSource, visited.getPosition());
        lastResult = Object{{INT}, -value};
    }
    else
    {
        double value = std::get<double>(getLastResultReference().value);
        lastResult = Object{{FLOAT}, -value};
    }
}

void Interpreter::visit(NotExpression &visited)
{
    visited.value->accept(*this);
    bool value = std::get<bool>(getLastResultReference().value);
    lastResult = Object{{BOOL}, !value};
}

void Interpreter::visit(SubscriptExpression &visited)
{
    auto [left, right] = getBinaryOpArgs<std::wstring, int32_t>(visited);
    if(right < 0 || static_cast<size_t>(right) >= left.size())
        throw OperatorArgumentError(L"Invalid index for subscript operator", currentSource, visited.getPosition());
    lastResult = Object{{STR}, std::wstring(1, left.at(static_cast<size_t>(right)))};
}

void Interpreter::visit(DotExpression &visited)
{
    visited.value->accept(*this);
    Object &left = getLastResultReference();
    lastResult = getField(left, program->structs.find(std::get<std::wstring>(left.type.value)), visited.field);
}

void Interpreter::visit(StructExpression &visited)
{
    std::vector<Object> fields;
    for(auto &argument: visited.arguments)
    {
        argument->accept(*this);
        fields.push_back(getLastResultValue());
    }
    lastResult = Object{{*visited.structType}, std::move(fields)};
}

template <typename TargetType, typename SourceType>
TargetType Interpreter::cast(const SourceType &, Position)
{
    throw RuntimeSemanticException("Invalid cast detected");
}

template <>
int32_t Interpreter::cast(const double &value, Position position)
{
    double rounded = std::round(value);
    if(rounded < std::numeric_limits<int32_t>::min() || rounded > std::numeric_limits<int32_t>::max() ||
       std::isnan(rounded))
        throw CastImpossibleError(
            std::format(L"Conversion of float {} to integer would result in a value out of integer range", value),
            currentSource, position
        );
    return static_cast<int32_t>(rounded);
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
int32_t Interpreter::cast(const std::wstring &value, Position position)
{
    return fromString<int32_t>(value, position, L"integer");
}

template <>
int32_t Interpreter::cast(const bool &value, Position)
{
    return static_cast<int32_t>(value);
}

template <>
int32_t Interpreter::cast(const int32_t &value, Position)
{
    return value;
}

template <>
double Interpreter::cast(const int32_t &value, Position)
{
    return static_cast<double>(value);
}

template <>
double Interpreter::cast(const std::wstring &value, Position position)
{
    return fromString<double>(value, position, L"float");
}

template <>
double Interpreter::cast(const bool &value, Position)
{
    return static_cast<double>(value);
}

template <>
double Interpreter::cast(const double &value, Position)
{
    return value;
}

template <>
std::wstring Interpreter::cast(const int32_t &value, Position)
{
    return std::format(L"{}", value);
}

template <>
std::wstring Interpreter::cast(const double &value, Position)
{
    return std::format(L"{}", value);
}

template <>
std::wstring Interpreter::cast(const bool &value, Position)
{
    if(value)
        return L"true";
    else
        return L"false";
}

template <>
std::wstring Interpreter::cast(const std::wstring &value, Position)
{
    return value;
}

template <>
bool Interpreter::cast(const int32_t &value, Position)
{
    return value != 0;
}

template <>
bool Interpreter::cast(const double &value, Position)
{
    return value != 0;
}

template <>
bool Interpreter::cast(const std::wstring &value, Position)
{
    return value.size() > 0;
}

template <>
bool Interpreter::cast(const bool &value, Position)
{
    return value;
}

template <typename TargetType>
Object Interpreter::getCastedObject(Type::Builtin targetType, Object &toCast, Position position)
{
    return Object(
        {targetType}, std::visit([&](const auto &value) { return cast<TargetType>(value, position); }, toCast.value)
    );
}

Object Interpreter::doCast(Type::Builtin targetType, Object &toCast, Position position)
{
    switch(targetType)
    {
    case INT:
        return getCastedObject<int32_t>(targetType, toCast, position);
    case STR:
        return getCastedObject<std::wstring>(targetType, toCast, position);
    case FLOAT:
        return getCastedObject<double>(targetType, toCast, position);
    case BOOL:
        return getCastedObject<bool>(targetType, toCast, position);
    default:
        throw RuntimeSemanticException("Invalid builtin type detected");
    }
}

void Interpreter::visit(CastExpression &visited)
{
    visited.value->accept(*this);
    if(visited.targetType.isBuiltin())
    {
        Type::Builtin type = std::get<Type::Builtin>(visited.targetType.value);
        lastResult = doCast(type, getLastResultReference(), visited.getPosition());
    }
    else // cast to variant type case
        lastResult = Object{{visited.targetType}, std::make_unique<Object>(getLastResultValue())};
}

void Interpreter::visit(VariableDeclStatement &visited)
{
    visited.value->accept(*this);
    Object &value = getLastResultReference();
    if(value.type == visited.declaration.type)
        return addVariable(visited.declaration.name, getLastResultValue());

    Object &containedInVariant = *std::get<std::unique_ptr<Object>>(value.value).get();
    if(containedInVariant.type == visited.declaration.type)
    {
        addVariable(visited.declaration.name, containedInVariant);
        lastResult = Object{{BOOL}, true};
    }
    else
        lastResult = Object{{BOOL}, false};
}

void Interpreter::visit(Assignable &visited)
{
    if(!visited.left)
    {
        lastResult = getVariable(visited.right);
        return;
    }
    visit(*visited.left);
    Object &left = getLastResultReference();
    std::wstring typeName = std::get<std::wstring>(left.type.value);

    auto structFound = program->structs.find(typeName);
    if(structFound != program->structs.end())
        lastResult = getField(left, structFound, visited.right);
    else // variant access case
        lastResult = *std::get<std::unique_ptr<Object>>(left.value).get();
}

void Interpreter::visit(AssignmentStatement &visited)
{
    visit(visited.left);
    Object &assignmentTarget = getLastResultReference();
    visited.right->accept(*this);
    assignmentTarget = std::move(getLastResultValue());
}

std::vector<Type> Interpreter::prepareArguments(FunctionCall &visited)
{
    functionArguments.clear();
    for(auto &argument: visited.arguments)
    {
        argument->accept(*this);
        std::visit(
            [&](auto &value) {
                if constexpr(std::is_same_v<Object &, decltype(value)>)
                    functionArguments.push_back(std::move(value));
                else
                    functionArguments.push_back(value);
            },
            lastResult
        );
    }
    std::vector<Type> argumentTypes;
    for(auto &argument: functionArguments)
        argumentTypes.push_back(getObject(argument).type);
    return argumentTypes;
}

void Interpreter::visit(FunctionCall &visited)
{
    std::vector<Type> argumentTypes = prepareArguments(visited);
    auto functionFound = program->functions.find(FunctionIdentification(visited.functionName, argumentTypes));
    if(functionFound != program->functions.end())
        return functionFound->second->accept(*this);

    for(unsigned index: visited.runtimeResolved)
    {
        std::visit(
            [&](auto &argument) {
                if constexpr(std::is_same_v<Object &, decltype(argument)>)
                    functionArguments[index] = std::move(*std::get<std::unique_ptr<Object>>(argument.value).get());
                else
                    functionArguments[index] = *std::get<std::unique_ptr<Object>>(argument.get().value).get();
            },
            functionArguments[index]
        );
        argumentTypes[index] = getObject(functionArguments[index]).type;
    }
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
    // case body is visited in visit(IfStatement &)
}

void Interpreter::visit(IfStatement &visited)
{
    bool executeElse = true;
    for(SingleIfCase &singleCase: visited.cases)
    {
        variables.top().emplace_back();
        visit(singleCase);
        if(std::get<bool>(getLastResultReference().value))
        {
            executeElse = false;
            visitInstructionBlock(singleCase.body);
            variables.top().pop_back();
            break;
        }
        variables.top().pop_back();
    }
    if(executeElse)
        visitInstructionScope(visited.elseCaseBody);
}

#define HANDLE_LOOP_FLAGS           \
    if(shouldBreak || shouldReturn) \
    {                               \
        shouldBreak = false;        \
        break;                      \
    }                               \
    if(shouldContinue)              \
        shouldContinue = false;

void Interpreter::visit(WhileStatement &visited)
{
    while(visited.condition->accept(*this), std::get<bool>(getLastResultReference().value))
    {
        visitInstructionScope(visited.body);
        HANDLE_LOOP_FLAGS;
    }
}

void Interpreter::visit(DoWhileStatement &visited)
{
    do
    {
        visitInstructionScope(visited.body);
        HANDLE_LOOP_FLAGS;
    }
    while(visited.condition->accept(*this), std::get<bool>(getLastResultReference().value));
}

void Interpreter::visit(FunctionDeclaration &visited)
{
    callPosition = visited.getPosition();
    currentSource = visited.getSource();
    variables.emplace();
    variables.top().emplace_back();
    for(unsigned i = 0; i < functionArguments.size(); i++)
    {
        std::visit(
            [&](auto &value) {
                if constexpr(std::is_same_v<Object &, decltype(value)>)
                    addVariable(visited.parameters.at(i).name, std::move(value));
                else
                    addVariable(visited.parameters.at(i).name, value);
            },
            functionArguments[i]
        );
    }
    visitInstructionBlock(visited.body);
    shouldReturn = false;
    variables.pop();
}

void Interpreter::visit(BuiltinFunctionDeclaration &visited)
{
    auto result = visited.body(callPosition, currentSource, functionArguments);
    if(result)
        lastResult = std::move(*result);
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
