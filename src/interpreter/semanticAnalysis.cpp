#include "semanticAnalysis.hpp"

#include "semanticExceptions.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <unordered_set>

using enum Type::Builtin;

#define EMPTY_VISIT(type) \
    void visit(type &) override {}

namespace {
class SemanticAnalyzer: public DocumentTreeVisitor
{
public:
    explicit SemanticAnalyzer(Program &program):
        program(program), noReturnFunctionPermitted(false), variantReadAccessPermitted(false), accessedVariant(false),
        blockFurtherDotAccess(false), hasReturned(false), loopCounter(0)
    {}

    void visit(Literal &visited) override
    {
        lastExpressionType = {visited.getType(), false};
    }

    void visit(Variable &visited) override
    {
        auto found = getVariableType(visited.name);
        if(!found)
            throw UnknownVariableError(
                std::format(L"Unknown variable: {}", visited.name), currentSource, visited.getPosition()
            );
        lastExpressionType = {found->first, true};
    }

    void visit(OrExpression &visited) override
    {
        ensureExpressionHasType(visited.left, Type{BOOL});
        ensureExpressionHasType(visited.right, Type{BOOL});
        lastExpressionType = {{BOOL}, false};
    }

    void visit(XorExpression &visited) override
    {
        ensureExpressionHasType(visited.left, Type{BOOL});
        ensureExpressionHasType(visited.right, Type{BOOL});
        lastExpressionType = {{BOOL}, false};
    }

    void visit(AndExpression &visited) override
    {
        ensureExpressionHasType(visited.left, Type{BOOL});
        ensureExpressionHasType(visited.right, Type{BOOL});
        lastExpressionType = {{BOOL}, false};
    }

    void visit(EqualExpression &visited) override
    {
        visitEqualityExpression(visited);
    }

    void visit(NotEqualExpression &visited) override
    {
        visitEqualityExpression(visited);
    }

    void visit(IdenticalExpression &visited) override
    {
        visitTypedEqualityExpression(visited);
    }

    void visit(NotIdenticalExpression &visited) override
    {
        visitTypedEqualityExpression(visited);
    }

    void visit(ConcatExpression &visited) override
    {
        ensureExpressionHasType(visited.left, Type{STR});
        ensureExpressionHasType(visited.right, Type{STR});
        lastExpressionType = {{STR}, false};
    }

    void visit(StringMultiplyExpression &visited) override
    {
        ensureExpressionHasType(visited.left, Type{STR});
        ensureExpressionHasType(visited.right, Type{INT});
        lastExpressionType = {{STR}, false};
    }

    void visit(GreaterExpression &visited) override
    {
        visitIntOrFloatExpression(visited);
        lastExpressionType = {{BOOL}, false};
    }

    void visit(LesserExpression &visited) override
    {
        visitIntOrFloatExpression(visited);
        lastExpressionType = {{BOOL}, false};
    }

    void visit(GreaterEqualExpression &visited) override
    {
        visitIntOrFloatExpression(visited);
        lastExpressionType = {{BOOL}, false};
    }

    void visit(LesserEqualExpression &visited) override
    {
        visitIntOrFloatExpression(visited);
        lastExpressionType = {{BOOL}, false};
    }

    void visit(PlusExpression &visited) override
    {
        lastExpressionType = visitIntOrFloatExpression(visited);
    }

    void visit(MinusExpression &visited) override
    {
        lastExpressionType = visitIntOrFloatExpression(visited);
    }

    void visit(MultiplyExpression &visited) override
    {
        lastExpressionType = visitIntOrFloatExpression(visited);
    }

    void visit(DivideExpression &visited) override
    {
        ensureExpressionHasType(visited.left, Type{FLOAT});
        ensureExpressionHasType(visited.right, Type{FLOAT});
        lastExpressionType = {{FLOAT}, false};
    }

    void visit(FloorDivideExpression &visited) override
    {
        ensureExpressionHasType(visited.left, Type{INT});
        ensureExpressionHasType(visited.right, Type{INT});
        lastExpressionType = {{INT}, false};
    }

    void visit(ModuloExpression &visited) override
    {
        ensureExpressionHasType(visited.left, Type{INT});
        ensureExpressionHasType(visited.right, Type{INT});
        lastExpressionType = {{INT}, false};
    }

    void visit(ExponentExpression &visited) override
    {
        lastExpressionType = visitIntOrFloatExpression(visited);
    }

    void visit(UnaryMinusExpression &visited) override
    {
        visitExpression(visited.value);
        if(lastExpressionType.first != Type{INT} && lastExpressionType.first != Type{FLOAT})
            insertCast(visited.value, lastExpressionType.first, Type{FLOAT});
        lastExpressionType = {(lastExpressionType.first == Type{INT}) ? Type{INT} : Type{FLOAT}, false};
    }

    void visit(NotExpression &visited) override
    {
        ensureExpressionHasType(visited.value, Type{BOOL});
        lastExpressionType = {{BOOL}, false};
    }

    void visit(IsExpression &visited) override
    {
        visitExpression(visited.left);
        if(lastExpressionType.first.isInitList())
            throw InvalidInitListError(
                L"Structure initialization list is not allowed with 'is' operator", currentSource,
                visited.left->getPosition()
            );
        lastExpressionType = {{BOOL}, false};
    }

    void visit(SubscriptExpression &visited) override
    {
        ensureExpressionHasType(visited.left, Type{STR});
        ensureExpressionHasType(visited.right, Type{INT});
        lastExpressionType = {{STR}, false};
    }

    void visit(DotExpression &visited) override
    {
        bool canAccessVariant = variantReadAccessPermitted;
        visitExpression(visited.value);
        bool isMutable = lastExpressionType.second;
        std::wstring typeName = getTypeNameForDotExpression(lastExpressionType.first, visited.getPosition());
        if(auto structFound = findIn(program.structs, typeName))
        {
            lastExpressionType = {
                getTypeOfField((*structFound)->second.fields, visited.field, visited.getPosition()), isMutable
            };
            return;
        }
        std::optional<std::unordered_map<std::wstring, VariantDeclaration>::iterator> variantFound;
        if(canAccessVariant && (variantFound = findIn(program.variants, typeName)))
        {
            accessedVariant = true;
            lastExpressionType = {
                getTypeOfField((*variantFound)->second.fields, visited.field, visited.getPosition()), isMutable
            };
            toReplace = std::move(visited.value);
            return;
        }
        throw FieldAccessError(
            std::format(L"Cannot access {} field of type {}", visited.field, typeName), currentSource,
            visited.getPosition()
        );
    }

    void visit(StructExpression &visited) override
    {
        std::vector<Type> types;
        for(auto &argument: visited.arguments)
        {
            visitExpression(argument);
            types.push_back(lastExpressionType.first);
        }
        lastExpressionType = {{types}, false};
    }

    void visit(CastExpression &visited) override
    {
        visitExpression(visited.value);
        if(!areTypesConvertible(lastExpressionType.first, visited.targetType))
            throw InvalidCastError(
                std::format(
                    L"Explicit conversion between types {} and {} is impossible", lastExpressionType.first,
                    visited.targetType
                ),
                currentSource, visited.getPosition()
            );
        lastExpressionType = {visited.targetType, false};
    }

    void visit(VariableDeclaration &visited) override
    {
        if(getVariableType(visited.name))
            throw VariableNameCollisionError(
                std::format(L"There already exists a variable with this name: {}", visited.name), currentSource,
                visited.getPosition()
            );
        if(!isValidType(visited.type))
            throw UnknownVariableTypeError(
                std::format(L"{} is not a type", visited.type), currentSource, visited.getPosition()
            );
        addVariableType(visited.name, {visited.type, visited.isMutable});
    }

    void visit(VariableDeclStatement &visited) override
    {
        visit(visited.declaration);
        bool shouldAccessVariant = variantReadAccessPermitted;
        visited.value->accept(*this);
        doReplacement(visited.value);
        bool isValueVariant = validateConditionVariantAccess(visited, lastExpressionType.first, shouldAccessVariant);
        if(lastExpressionType.first != visited.declaration.type && !isValueVariant)
            insertCast(visited.value, lastExpressionType.first, visited.declaration.type);
    }

    void visit(Assignable &visited) override
    {
        if(visited.left == nullptr)
            return visitLeafAssignable(visited);
        blockFurtherDotAccess = false;
        visit(*visited.left);
        if(blockFurtherDotAccess)
            throw FieldAccessError(
                std::format(L"Attempted access to field of field of variant type"), currentSource,
                visited.left->getPosition()
            );
        if(lastExpressionType.first.isBuiltin())
            throw FieldAccessError(
                std::format(L"Attempted access to field of simple type {}", lastExpressionType.first), currentSource,
                visited.left->getPosition()
            );
        ensureFieldAssignable(
            std::get<std::wstring>(lastExpressionType.first.value), visited.right, visited.left->getPosition()
        );
    }

    void visit(AssignmentStatement &visited) override
    {
        visited.left.accept(*this);
        Type leftType = lastExpressionType.first;
        visitExpression(visited.right);
        Type rightType = lastExpressionType.first;
        if(leftType != rightType)
            insertCast(visited.right, rightType, leftType);
    }

    void visit(FunctionCall &visited) override
    {
        bool noReturnPermitted = noReturnFunctionPermitted;
        noReturnFunctionPermitted = false;
        auto [argumentTypes, argumentsMutable] = visitArguments(visited.arguments);
        if(auto structFound = findIn(program.structs, visited.functionName))
            return visitStructFunctionCall(visited, argumentTypes);
        if(auto variantFound = findIn(program.variants, visited.functionName))
            return visitVariantFunctionCall(visited, argumentTypes, (*variantFound)->second.fields);

        const FunctionIdentification &bestId = getBestOverload(visited, argumentTypes);
        const std::unique_ptr<BaseFunctionDeclaration> &function = program.functions.at(bestId);

        validateArgumentMutability(bestId, function, argumentsMutable, visited.getPosition());
        insertArgumentConversions(bestId, visited.arguments, argumentTypes);

        auto returnType = function->returnType;
        if(returnType)
            lastExpressionType = {*returnType, false};
        else if(!noReturnPermitted)
            throw InvalidFunctionCallError(
                L"Cannot call a function with no return type where an expression is expected", currentSource,
                visited.getPosition()
            );
    }

    void visit(FunctionCallInstruction &visited) override
    {
        noReturnFunctionPermitted = true;
        variantReadAccessPermitted = false;
        visit(visited.functionCall);
    }

    void visit(ReturnStatement &visited) override
    {
        if(!expectedReturnType || !visited.returnValue)
        {
            if(expectedReturnType || visited.returnValue)
                throw InvalidReturnError(
                    L"Return with value is required if and only if function returns a type", currentSource,
                    visited.getPosition()
                );
            return;
        }
        visitExpression(visited.returnValue);
        if(lastExpressionType.first != expectedReturnType)
            insertCast(visited.returnValue, lastExpressionType.first, *expectedReturnType);
        hasReturned = true;
    }

    void visit(ContinueStatement &visited) override
    {
        if(loopCounter < 1)
            throw InvalidContinueError(
                L"Continue statement is permitted only within a loop", currentSource, visited.getPosition()
            );
    }

    void visit(BreakStatement &visited) override
    {
        if(loopCounter < 1)
            throw InvalidBreakError(
                L"Break statement is permitted only within a loop", currentSource, visited.getPosition()
            );
    }

    void visit(SingleIfCase &visited) override
    {
        variableTypeScopes.push_back({});
        std::visit([&](auto &condition) { visitCondition(condition); }, visited.condition);
        visitInstructions(visited.body);
        variableTypeScopes.pop_back();
    }

    void visit(IfStatement &visited) override
    {
        bool previousInstructionReturned = hasReturned;
        bool allPathsReturned = false;
        for(SingleIfCase &ifCase: visited.cases)
        {
            hasReturned = false;
            ifCase.accept(*this);
            if(!hasReturned)
                allPathsReturned = false;
        }
        hasReturned = false;
        visitNewScope(visited.elseCaseBody);
        if(!hasReturned)
            allPathsReturned = false;
        hasReturned = previousInstructionReturned || allPathsReturned;
    }

    void visit(WhileStatement &visited) override
    {
        visitCondition(visited.condition);
        loopCounter += 1;
        visitNewScope(visited.body);
        loopCounter -= 1;
    }

    void visit(DoWhileStatement &visited) override
    {
        visitCondition(visited.condition);
        loopCounter += 1;
        visitNewScope(visited.body);
        loopCounter -= 1;
    }

    void visit(FunctionDeclaration &visited) override
    {
        currentSource = visited.getSource();
        parametersToVariables(visited.parameters);
        expectedReturnType = visited.returnType;
        hasReturned = false;
        visitInstructions(visited.body);
        if(expectedReturnType && !hasReturned)
            throw InvalidReturnError(
                L"Return with value is required in a function that returns a value", currentSource,
                visited.getPosition()
            );
    }

    // these are verified via regular functions, not visitation
    EMPTY_VISIT(Field);
    EMPTY_VISIT(StructDeclaration);
    EMPTY_VISIT(VariantDeclaration);
    EMPTY_VISIT(IncludeStatement);
    EMPTY_VISIT(BuiltinFunctionDeclaration);

    void visit(Program &visited) override
    {
        if(!visited.includes.empty())
            throw IncludeInSemanticAnalysisError(
                "Internal error - program's include statements should be executed before calling doSemanticAnalysis"
            );

        checkNameDuplicates(visited);
        for(const auto &[name, variant]: visited.variants)
        {
            checkStructOrVariant(name, variant);
            checkFieldTypeDuplicates(variant.fields);
        }
        for(const auto &[name, structure]: visited.structs)
            checkStructOrVariant(name, structure);
        for(auto &function: visited.functions)
            function.second->accept(*this);
    }
private:
    Program &program;
    std::wstring currentSource;
    std::optional<Type> expectedReturnType;
    std::pair<Type, bool> lastExpressionType;
    std::vector<std::unordered_map<std::wstring, std::pair<Type, bool>>> variableTypeScopes;
    bool noReturnFunctionPermitted, variantReadAccessPermitted, accessedVariant, blockFurtherDotAccess, hasReturned;
    std::unique_ptr<Expression> toReplace;
    unsigned loopCounter;

    std::optional<std::pair<Type, bool>> getVariableType(const std::wstring &name)
    {
        for(auto scope: variableTypeScopes)
        {
            if(auto found = findIn(scope, name))
                return (*found)->second;
        }
        return std::nullopt;
    }

    void addVariableType(const std::wstring &name, const std::pair<Type, bool> &type)
    {
        variableTypeScopes[variableTypeScopes.size() - 1].insert({name, type});
    }

    void doReplacement(std::unique_ptr<Expression> &expression)
    {
        if(toReplace)
        {
            expression = std::move(toReplace);
            toReplace = nullptr;
        }
    }

    void visitExpression(std::unique_ptr<Expression> &expression)
    {
        variantReadAccessPermitted = false;
        expression->accept(*this);
        doReplacement(expression);
    }

    void ensureExpressionHasType(std::unique_ptr<Expression> &expression, Type desiredType)
    {
        visitExpression(expression);
        if(lastExpressionType.first != desiredType)
            insertCast(expression, lastExpressionType.first, desiredType);
    }

    Type getTargetTypeForEquality(Type::Builtin leftType, Type::Builtin rightType)
    {
        if(leftType == STR || rightType == STR)
            return {STR};
        if(leftType == FLOAT || rightType == FLOAT)
            return {FLOAT};
        if(leftType == INT || rightType == INT)
            return {INT};
        return {BOOL};
    }

    template <typename EqualityExpression>
    void visitEqualityExpression(EqualityExpression &visited)
    {
        visitExpression(visited.left);
        Type leftType = lastExpressionType.first;
        visitExpression(visited.right);
        Type rightType = lastExpressionType.first;
        lastExpressionType = {{BOOL}, false};

        if(leftType.isInitList())
            return insertCast(visited.left, leftType, rightType);
        if(rightType.isInitList())
            return insertCast(visited.right, rightType, leftType);
        if(leftType == rightType)
            return;
        if(!leftType.isBuiltin() || !rightType.isBuiltin())
            throw InvalidOperatorArgsError(
                std::format(L"{} and {} are not valid types for equality operator", leftType, rightType), currentSource,
                visited.getPosition()
            );
        Type targetType = getTargetTypeForEquality(
            std::get<Type::Builtin>(leftType.value), std::get<Type::Builtin>(rightType.value)
        );
        if(leftType != targetType)
            return insertCast(visited.left, leftType, targetType);
        if(rightType != targetType)
            return insertCast(visited.right, rightType, targetType);
    }

    template <typename TypedEqualityExpression>
    void visitTypedEqualityExpression(TypedEqualityExpression &visited)
    {
        visitExpression(visited.left);
        Type leftType = lastExpressionType.first;
        visitExpression(visited.right);
        Type rightType = lastExpressionType.first;
        lastExpressionType = {{BOOL}, false};

        if(leftType.isInitList())
            return insertCast(visited.left, leftType, rightType);
        if(rightType.isInitList())
            return insertCast(visited.right, rightType, leftType);
    }

    template <typename IntOrFloatExpression>
    std::pair<Type, bool> visitIntOrFloatExpression(IntOrFloatExpression &expression)
    {
        visitExpression(expression.left);
        Type leftType = lastExpressionType.first;
        visitExpression(expression.right);
        Type rightType = lastExpressionType.first;

        if(leftType == Type{INT} && rightType == Type{INT})
            return {{INT}, false};
        if(leftType != Type{FLOAT})
            insertCast(expression.left, leftType, {FLOAT});
        if(rightType != Type{FLOAT})
            insertCast(expression.right, rightType, {FLOAT});
        return {{FLOAT}, false};
    }

    std::wstring getTypeNameForDotExpression(Type leftType, Position position)
    {
        if(lastExpressionType.first.isInitList())
            throw InvalidInitListError(
                L"Structure initialization list is not allowed with '.' operator", currentSource, position
            );
        if(lastExpressionType.first.isBuiltin())
            throw FieldAccessError(L"Attempted access to field of simple type {}", currentSource, position);
        return std::get<std::wstring>(leftType.value);
    }

    void setStructExpressionType(
        StructExpression &expression, const std::wstring &structType, const Type::InitializationList &structInitListType
    )
    {
        expression.structType = structType;
        const std::vector<Field> &structFields = getStructOrVariantFields(structType);
        for(unsigned i = 0; i < structFields.size(); i++)
        {
            if(structInitListType[i] != structFields[i].type)
                insertCast(expression.arguments[i], structInitListType[i], structFields[i].type);
        }
    }

    std::vector<FunctionIdentification> getFunctionIdsWithName(const std::wstring &functionName)
    {
        std::vector<FunctionIdentification> result;
        for(auto &[id, declaration]: program.functions)
        {
            if(id.name == functionName)
                result.push_back(id);
        }
        return result;
    }

    std::pair<std::vector<Type>, std::vector<bool>> visitArguments(std::vector<std::unique_ptr<Expression>> &arguments)
    {
        std::vector<Type> argumentTypes;
        std::vector<bool> argumentsMutable;
        for(auto &argument: arguments)
        {
            visitExpression(argument);
            argumentTypes.push_back(lastExpressionType.first);
            argumentsMutable.push_back(lastExpressionType.second);
        }
        return {argumentTypes, argumentsMutable};
    }

    Type::InitializationList getInitListType(FunctionCall &visited, const std::vector<Type> &argumentTypes)
    {
        if(argumentTypes.size() != 1)
            throw InvalidCastError(
                L"Explicit cast to struct type must have only one argument", currentSource, visited.getPosition()
            );
        if(!argumentTypes[0].isInitList())
            throw InvalidCastError(
                L"Explicit cast to struct type can only be done on initalization lists", currentSource,
                visited.getPosition()
            );
        return std::get<Type::InitializationList>(argumentTypes[0].value);
    }

    // case when the function call is an explicit cast of initialization list to a struct type, parsed as FunctionCall
    void visitStructFunctionCall(FunctionCall &visited, const std::vector<Type> &argumentTypes)
    {
        Type::InitializationList initListType = getInitListType(visited, argumentTypes);
        if(!isStructInitListValid(initListType, {visited.functionName}))
            throw InvalidInitListError(
                std::format(
                    L"Structure initialization list type {} cannot be converted to type {}", initListType,
                    visited.functionName
                ),
                currentSource, visited.getPosition()
            );
        auto structExpression = std::make_unique<StructExpression>(
            visited.arguments[0]->getPosition(),
            std::move(static_cast<StructExpression *>(visited.arguments[0].get())->arguments)
        );
        setStructExpressionType(*structExpression, visited.functionName, initListType);
        toReplace = std::move(structExpression);
        lastExpressionType = {{visited.functionName}, false};
    }

    // case when the function call is an explicit cast to a variant type, parsed as FunctionCall
    void visitVariantFunctionCall(
        FunctionCall &visited, const std::vector<Type> &argumentTypes, const std::vector<Field> &variantFields
    )
    {
        if(argumentTypes.size() != 1)
            throw InvalidCastError(
                L"Explicit cast to variant type must have only one argument", currentSource, visited.getPosition()
            );
        if(!getField(variantFields, argumentTypes[0]))
            throw InvalidCastError(
                std::format(L"Cannot cast value of type {} to variant type {}", argumentTypes[0], visited.functionName),
                currentSource, visited.getPosition()
            );
        toReplace = std::make_unique<CastExpression>(
            visited.getPosition(), std::move(visited.arguments[0]), Type{visited.functionName}
        );
        lastExpressionType = {{visited.functionName}, false};
    }

    unsigned countNeededConversions(const std::vector<Type> &parameterTypes, const std::vector<Type> &argumentTypes)
    {
        if(parameterTypes.size() != argumentTypes.size())
            return std::numeric_limits<unsigned>::max();

        unsigned conversions = 0;
        for(unsigned i = 0; i < parameterTypes.size(); i++)
        {
            if(parameterTypes[i] != argumentTypes[i])
            {
                if(!areTypesConvertible(argumentTypes[i], parameterTypes[i]))
                    return std::numeric_limits<unsigned>::max();
                conversions += 1;
            }
        }
        return conversions;
    }

    std::pair<unsigned, bool> getMinimumIndex(const std::vector<unsigned> &conversionsNeeded)
    {
        unsigned bestIndex = 0;
        unsigned leastConversions = conversionsNeeded[0];
        bool multipleBests = false;
        for(unsigned i = 1; i < conversionsNeeded.size(); i++)
        {
            if(conversionsNeeded[i] < leastConversions)
            {
                leastConversions = conversionsNeeded[i];
                bestIndex = i;
                multipleBests = false;
            }
            else if(conversionsNeeded[i] == leastConversions)
                multipleBests = true;
        }
        return {bestIndex, multipleBests};
    }

    FunctionIdentification getBestOverload(FunctionCall &visited, const std::vector<Type> &argumentTypes)
    {
        std::vector<FunctionIdentification> overloads = getFunctionIdsWithName(visited.functionName);
        std::vector<unsigned> conversionsNeeded;
        for(const FunctionIdentification &id: overloads)
            conversionsNeeded.push_back(countNeededConversions(id.parameterTypes, argumentTypes));
        auto [bestIndex, multipleBests] = getMinimumIndex(conversionsNeeded);
        if(conversionsNeeded[bestIndex] == std::numeric_limits<unsigned>::max())
            throw InvalidFunctionCallError(
                std::format(
                    L"No matching function to call with name {} for argument types {}", visited.functionName,
                    argumentTypes
                ),
                currentSource, visited.getPosition()
            );
        if(multipleBests)
            throw AmbiguousFunctionCallError(
                std::format(
                    L"Call of overloaded function {} with arguments {} is ambiguous", visited.functionName,
                    argumentTypes
                ),
                currentSource, visited.getPosition()
            );
        return overloads[bestIndex];
    }

    void validateArgumentMutability(
        const FunctionIdentification &id, const std::unique_ptr<BaseFunctionDeclaration> &function,
        std::vector<bool> argumentsMutable, Position position
    )
    {
        for(unsigned i = 0; i < argumentsMutable.size(); i++)
        {
            if(!argumentsMutable[i] && function->parameters[i].isMutable)
                throw ImmutableError(
                    std::format(
                        L"{}th argument to function {} must be mutable and must not be a temporary value", i + 1, id
                    ),
                    currentSource, position
                );
        }
    }

    void insertArgumentConversions(
        const FunctionIdentification &id, std::vector<std::unique_ptr<Expression>> &arguments,
        std::vector<Type> argumentTypes
    )
    {
        for(unsigned i = 0; i < id.parameterTypes.size(); i++)
        {
            if(argumentTypes[i] != id.parameterTypes[i])
                insertCast(arguments[i], argumentTypes[i], id.parameterTypes[i]);
        }
    }

    void visitLeafAssignable(Assignable &visited)
    {
        auto variableFound = getVariableType(visited.right);
        if(!variableFound)
            throw UnknownVariableError(
                std::format(L"{} is not a variable", visited.right), currentSource, visited.getPosition()
            );
        if(!variableFound->second)
            throw ImmutableError(
                std::format(L"Attempted to modify immutable variable {}", visited.right), currentSource,
                visited.getPosition()
            );
        lastExpressionType = {variableFound->first, true};
    }

    void visitInstructions(std::vector<std::unique_ptr<Instruction>> &instructions)
    {
        for(auto &instruction: instructions)
            instruction->accept(*this);
    }

    void visitNewScope(std::vector<std::unique_ptr<Instruction>> &instructions)
    {
        variableTypeScopes.push_back({});
        visitInstructions(instructions);
        variableTypeScopes.pop_back();
    }

    void visitCondition(VariableDeclStatement &condition)
    {
        variantReadAccessPermitted = true;
        visit(condition);
        variantReadAccessPermitted = false;
    }

    void visitCondition(std::unique_ptr<Expression> &condition)
    {
        visitExpression(condition);
        if(lastExpressionType.first != Type{BOOL})
            insertCast(condition, lastExpressionType.first, Type{BOOL});
    }

    Type getTypeOfField(const std::vector<Field> &fields, std::wstring fieldName, Position position)
    {
        auto fieldFound = std::find_if(fields.begin(), fields.end(), [&](const Field &field) {
            return field.name == fieldName;
        });
        if(fieldFound == fields.end())
            throw FieldAccessError(
                std::format(
                    L"Attempted access to nonexistent field {} of type {}", fieldName, lastExpressionType.first
                ),
                currentSource, position
            );
        return fieldFound->type;
    }

    const Field *getField(const std::vector<Field> &fields, Type type)
    {
        auto fieldFound = std::find_if(fields.begin(), fields.end(), [&](const Field &field) {
            return field.type == type;
        });
        if(fieldFound != fields.end())
            return &*fieldFound;
        else
            return nullptr;
    }

    template <typename Container, typename Content>
    std::optional<decltype(Container().begin())> findIn(Container &container, const Content &value)
    {
        auto found = container.find(value);
        if(found != container.end())
            return found;
        else
            return std::nullopt;
    }

    void ensureFieldAssignable(std::wstring complexTypeName, std::wstring fieldName, Position position)
    {
        if(auto structFound = findIn(program.structs, complexTypeName))
        {
            lastExpressionType = {getTypeOfField((*structFound)->second.fields, fieldName, position), true};
            return;
        }
        if(auto variantFound = findIn(program.variants, complexTypeName))
        {
            lastExpressionType = {getTypeOfField((*variantFound)->second.fields, fieldName, position), true};
            blockFurtherDotAccess = true;
            return;
        }
        throw UnknownVariableTypeError(
            std::format(L"Internal error - {} is not a valid type", complexTypeName), currentSource, position
        );
    }

    bool isFieldOfVariant(const std::wstring &complexType, Type fieldType)
    {
        if(program.structs.find(complexType) != program.structs.end())
            return false;
        const std::vector<Field> &variantFields = program.variants.find(complexType)->second.fields;
        return getField(variantFields, fieldType);
    }

    bool validateConditionVariantAccess(VariableDeclStatement &visited, Type valueType, bool shouldAccessVariant)
    {
        std::vector<Field> *variantFields = getVariantFields(valueType);
        if(shouldAccessVariant && !accessedVariant && !variantFields)
            throw InvalidIfConditionError(
                L"If with declaration must access a variant type", currentSource, visited.getPosition()
            );
        if(valueType != visited.declaration.type)
        {
            if(accessedVariant)
                throw InvalidCastError(
                    L"The type in if condition declaration must match the type of the variant's accessed field",
                    currentSource, visited.getPosition()
                );
            if(variantFields && !getField(*variantFields, visited.declaration.type))
                throw InvalidCastError(
                    L"The type in if condition declaration must match one of the variant's types", currentSource,
                    visited.getPosition()
                );
        }
        return variantFields;
    }

    bool isStructInitListValid(Type::InitializationList typeFrom, Type typeTo)
    {
        if(typeTo.isBuiltin())
            return false;
        auto structFound = findIn(program.structs, std::get<std::wstring>(typeTo.value));
        if(!structFound)
            return false;
        std::vector<Field> &structFields = (*structFound)->second.fields;
        if(typeFrom.size() != structFields.size())
            return false;
        for(unsigned i = 0; i < typeFrom.size(); i++)
            if(!areTypesConvertible(typeFrom[i], structFields[i].type))
                return false;
        return true;
    }

    bool areTypesConvertible(Type typeFrom, Type typeTo)
    {
        if(typeTo.isInitList())
            return false;
        if(typeFrom.isInitList())
            return isStructInitListValid(std::get<Type::InitializationList>(typeFrom.value), typeTo);
        if(!typeFrom.isBuiltin())
            return false;
        if(!typeTo.isBuiltin())
        {
            std::wstring typeName = std::get<std::wstring>(typeTo.value);
            return isFieldOfVariant(typeName, typeFrom);
        }
        return true;
    }

    void insertCast(std::unique_ptr<Expression> &expression, Type typeFrom, Type typeTo)
    {
        if(!areTypesConvertible(typeFrom, typeTo))
            throw InvalidCastError(
                std::format(L"Implicit conversion between types {} and {} is impossible", typeFrom, typeTo),
                currentSource, expression->getPosition()
            );
        if(typeFrom.isInitList())
            setStructExpressionType(
                *static_cast<StructExpression *>(expression.get()), std::get<std::wstring>(typeTo.value),
                std::get<Type::InitializationList>(typeFrom.value)
            );
        else
            expression = std::make_unique<CastExpression>(expression->getPosition(), std::move(expression), typeTo);
    }

    std::vector<Field> *getVariantFields(Type type)
    {
        if(type.isInitList() || type.isBuiltin())
            return nullptr;
        std::wstring typeName = std::get<std::wstring>(type.value);
        auto fields = findIn(program.variants, typeName);
        if(!fields)
            return nullptr;
        return &(*fields)->second.fields;
    }

    bool isValidType(Type type)
    {
        if(type.isInitList())
            return false;
        if(type.isBuiltin())
            return true;
        std::wstring typeName = std::get<std::wstring>(type.value);
        return findIn(program.structs, typeName) || findIn(program.variants, typeName);
    }

    const std::vector<Field> &getStructOrVariantFields(const std::wstring &name)
    {
        if(auto foundStruct = findIn(program.structs, name))
            return (*foundStruct)->second.fields;
        if(auto foundVariant = findIn(program.variants, name))
            return (*foundVariant)->second.fields;
        throw InvalidTypeError(std::format("Internal error - invalid struct or variant type name given"));
    }

    void checkNameDuplicates(Program &visited)
    {
        for(auto &[id, function]: visited.functions)
        {
            if(auto variantFound = findIn(visited.variants, id.name))
                throw NameCollisionError(
                    L"function " + id.name, *function, L"variant " + (*variantFound)->first, (*variantFound)->second
                );
            if(auto structFound = findIn(visited.structs, id.name))
                throw NameCollisionError(
                    L"function " + id.name, *function, L"struct " + (*structFound)->first, (*structFound)->second
                );
        }
        for(auto &[name, variant]: visited.variants)
        {
            if(auto structFound = findIn(visited.structs, name))
                throw NameCollisionError(
                    L"variant " + name, variant, L"struct " + (*structFound)->first, (*structFound)->second
                );
        }
    }

    void checkFieldNameDuplicates(const std::vector<Field> &fields)
    {
        std::unordered_set<std::wstring> fieldNames;
        for(const Field &field: fields)
        {
            if(fieldNames.find(field.name) != fieldNames.end())
                throw FieldNameCollisionError(
                    std::format(L"More than one structure or variant fields have the same name: {}", field.name),
                    currentSource, field.getPosition()
                );
            fieldNames.insert(field.name);
        }
    }

    void checkFieldTypeDuplicates(const std::vector<Field> &fields)
    {
        std::unordered_set<Type> fieldTypes;
        for(const Field &field: fields)
        {
            if(fieldTypes.find(field.type) != fieldTypes.end())
                throw FieldTypeCollisionError(
                    std::format(L"More than one variant fields have the same type: {}", field.type), currentSource,
                    field.getPosition()
                );
            fieldTypes.insert(field.type);
        }
    }

    void ensureFieldTypesExist(const std::vector<Field> &fields)
    {
        for(const Field &field: fields)
        {
            if(!isValidType(field.type))
                throw UnknownFieldTypeError(
                    std::format(L"{} is not a type", field.type), currentSource, field.getPosition()
                );
        }
    }

    // Returns whether type1 (struct or variant type) is among the subtypes of type2.
    bool isInSubtypes(const std::wstring &type1, const Type &type2)
    {
        if(type2.isBuiltin() || type2.isInitList())
            return false;
        std::wstring type2Name = std::get<std::wstring>(type2.value);
        if(type2Name == type1)
            return true;

        for(const Field &field: getStructOrVariantFields(type2Name))
        {
            if(isInSubtypes(type1, field.type))
                return true;
        }
        return false;
    }

    void checkTypeRecursion(const std::wstring &typeName, const std::vector<Field> &fields)
    {
        for(const Field &field: fields)
        {
            if(isInSubtypes(typeName, field.type))
                throw FieldTypeRecursionError(
                    std::format(
                        L"Field {} of struct or variant type {} contains the type itself", field.name, typeName
                    ),
                    currentSource, field.getPosition()
                );
        }
    }

    template <typename StructOrVariantDeclaration>
    void checkStructOrVariant(const std::wstring &name, const StructOrVariantDeclaration &toCheck)
    {
        currentSource = toCheck.getSource();
        checkFieldNameDuplicates(toCheck.fields);
        ensureFieldTypesExist(toCheck.fields);
        checkTypeRecursion(name, toCheck.fields);
    }

    void parametersToVariables(std::vector<VariableDeclaration> &parameters)
    {
        variableTypeScopes = {{}};
        for(VariableDeclaration &parameter: parameters)
            parameter.accept(*this);
    }
};
}

void doSemanticAnalysis(Program &program)
{
    SemanticAnalyzer(program).visit(program);
}
