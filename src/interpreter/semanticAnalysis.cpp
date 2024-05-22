#include "semanticAnalysis.hpp"

#include "interpreterExceptions.hpp"

#include <iostream>
#include <unordered_map>
#include <unordered_set>

using enum Type::Builtin;

#define EMPTY_VISIT(type)              \
    void visit(type &visited) override \
    {                                  \
        static_cast<void>(visited);    \
    }

namespace {
class SemanticAnalyzer: public DocumentTreeVisitor
{
public:
    explicit SemanticAnalyzer(Program &program):
        program(program), noReturnFunctionPermitted(false), variantReadAccessPermitted(false),
        blockFurtherDotAccess(false)
    {}

    void visit(Literal &visited) override
    {
        lastExpressionType = visited.getType();
    }

    void visit(Variable &visited) override
    {
        auto found = variableTypes.find(visited.name);
        if(found == variableTypes.end())
            throw UnknownVariableError(
                std::format(L"Unknown variable: {}", visited.name), currentSource, visited.getPosition()
            );
        lastExpressionType = found->second.first;
    }

    void visit(OrExpression &visited) override
    {
        ensureExpressionHasType(visited.left, Type{BOOL});
        ensureExpressionHasType(visited.right, Type{BOOL});
        lastExpressionType = Type{BOOL};
    }

    void visit(XorExpression &visited) override
    {
        ensureExpressionHasType(visited.left, Type{BOOL});
        ensureExpressionHasType(visited.right, Type{BOOL});
        lastExpressionType = Type{BOOL};
    }

    void visit(AndExpression &visited) override
    {
        ensureExpressionHasType(visited.left, Type{BOOL});
        ensureExpressionHasType(visited.right, Type{BOOL});
        lastExpressionType = Type{BOOL};
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
        lastExpressionType = Type{STR};
    }

    void visit(StringMultiplyExpression &visited) override
    {
        ensureExpressionHasType(visited.left, Type{STR});
        ensureExpressionHasType(visited.right, Type{INT});
        lastExpressionType = Type{STR};
    }

    void visit(GreaterExpression &visited) override
    {
        visitIntOrFloatExpression(visited);
        lastExpressionType = Type{BOOL};
    }

    void visit(LesserExpression &visited) override
    {
        visitIntOrFloatExpression(visited);
        lastExpressionType = Type{BOOL};
    }

    void visit(GreaterEqualExpression &visited) override
    {
        visitIntOrFloatExpression(visited);
        lastExpressionType = Type{BOOL};
    }

    void visit(LesserEqualExpression &visited) override
    {
        visitIntOrFloatExpression(visited);
        lastExpressionType = Type{BOOL};
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
        lastExpressionType = Type{FLOAT};
    }

    void visit(FloorDivideExpression &visited) override
    {
        ensureExpressionHasType(visited.left, Type{INT});
        ensureExpressionHasType(visited.right, Type{INT});
        lastExpressionType = Type{INT};
    }

    void visit(ModuloExpression &visited) override
    {
        ensureExpressionHasType(visited.left, Type{INT});
        ensureExpressionHasType(visited.right, Type{INT});
        lastExpressionType = Type{INT};
    }

    void visit(ExponentExpression &visited) override
    {
        lastExpressionType = visitIntOrFloatExpression(visited);
    }

    void visit(UnaryMinusExpression &visited) override
    {
        visitExpression(visited.value);
        if(lastExpressionType != Type{INT} && lastExpressionType != Type{FLOAT})
            insertCast(visited.value, lastExpressionType, Type{FLOAT});
        lastExpressionType = (lastExpressionType == Type{INT}) ? Type{INT} : Type{FLOAT};
    }

    void visit(NotExpression &visited) override
    {
        ensureExpressionHasType(visited.value, Type{BOOL});
        lastExpressionType = Type{BOOL};
    }

    void visit(IsExpression &visited) override
    {
        visitExpression(visited.left);
        if(lastExpressionType.isInitList())
            throw InvalidInitListError(
                L"Structure initialization list is not allowed with 'is' operator", currentSource,
                visited.left->getPosition()
            );
        lastExpressionType = Type{BOOL};
    }

    void visit(SubscriptExpression &visited) override
    {
        ensureExpressionHasType(visited.left, Type{STR});
        ensureExpressionHasType(visited.right, Type{INT});
        lastExpressionType = Type{STR};
    }

    void visit(DotExpression &visited) override
    {
        bool canAccessVariant = variantReadAccessPermitted;
        visitExpression(visited.value);
        std::wstring typeName = getTypeNameForDotExpression(lastExpressionType, visited.getPosition());
        if(auto structFound = findIn(program.structs, typeName))
        {
            lastExpressionType = getTypeOfField((*structFound)->second.fields, visited.field, visited.getPosition());
            return;
        }
        std::optional<std::unordered_map<std::wstring, VariantDeclaration>::iterator> variantFound;
        if(canAccessVariant && (variantFound = findIn(program.variants, typeName)))
        {
            lastExpressionType = getTypeOfField((*variantFound)->second.fields, visited.field, visited.getPosition());
            return;
        }
        throw FieldAccessError(
            std::format(L"Cannot access {} field of type {}", visited.field, typeName), currentSource,
            visited.getPosition()
        );
    }

    void visit(StructExpression &visited) override
    {
        if(structType)
            return setStructExpressionType(visited);
        std::vector<Type> types;
        for(auto &argument: visited.arguments)
        {
            visitExpression(argument);
            types.push_back(lastExpressionType);
        }
        lastExpressionType = {types};
    }

    void visit(CastExpression &visited) override
    {
        visited.value->accept(*this);
        if(!areTypesConvertible(lastExpressionType, visited.targetType))
            throw InvalidCastError(
                std::format(
                    L"Explicit conversion between types {} and {} is impossible", lastExpressionType, visited.targetType
                ),
                currentSource, visited.getPosition()
            );
        lastExpressionType = visited.targetType;
    }

    void visit(VariableDeclaration &visited) override
    {
        if(variableTypes.find(visited.name) != variableTypes.end())
            throw VariableNameCollisionError(
                std::format(L"There already exists a variable with this name: {}", visited.name), currentSource,
                visited.getPosition()
            );
        if(!isValidType(visited.type))
            throw UnknownVariableTypeError(
                std::format(L"{} is not a type", visited.type), currentSource, visited.getPosition()
            );
        variableTypes.insert({visited.name, {visited.type, visited.isMutable}});
    }

    void visit(VariableDeclStatement &visited) override
    {
        visited.declaration.accept(*this);
        visited.value->accept(*this);
        if(lastExpressionType != visited.declaration.type)
            insertCast(visited.value, lastExpressionType, visited.declaration.type);
    }

    void visit(Assignable &visited) override
    {
        if(visited.left == nullptr)
            return visitLeafAssignable(visited);
        blockFurtherDotAccess = false;
        visited.left->accept(*this);
        if(blockFurtherDotAccess)
            throw FieldAccessError(
                std::format(L"Attempted access to field of field of variant type"), currentSource,
                visited.left->getPosition()
            );
        if(lastExpressionType.isBuiltin())
            throw FieldAccessError(
                std::format(L"Attempted access to field of simple type {}", lastExpressionType), currentSource,
                visited.left->getPosition()
            );
        ensureFieldAssignable(
            std::get<std::wstring>(lastExpressionType.value), visited.right, visited.left->getPosition()
        );
    }

    void visit(AssignmentStatement &visited) override
    {
        visited.left.accept(*this);
        Type leftType = lastExpressionType;
        visited.right->accept(*this);
        Type rightType = lastExpressionType;
        if(leftType != rightType)
            insertCast(visited.right, rightType, leftType);
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

    void visit(FunctionDeclaration &visited) override
    {
        currentSource = visited.getSource();
        parametersToVariables(visited.parameters);
        expectedReturnType = visited.returnType;
        for(auto &instruction: visited.body)
            instruction->accept(*this);
    }

    // these are verified via regular functions, not visitation
    EMPTY_VISIT(Field);
    EMPTY_VISIT(StructDeclaration);
    EMPTY_VISIT(VariantDeclaration);
    EMPTY_VISIT(IncludeStatement);

    void visit(Program &visited) override
    {
        if(!visited.includes.empty())
            throw IncludeInSemanticAnalysisError(
                "Internal error - program's include statements should be executed before calling doSemanticAnalysis"
            );

        checkNameDuplicates(visited);
        for(const auto &[name, variant]: visited.variants)
            checkStructOrVariant(name, variant);
        for(const auto &[name, structure]: visited.structs)
            checkStructOrVariant(name, structure);
        for(auto &function: visited.functions)
            function.second.accept(*this);
    }
private:
    Program &program;
    std::wstring currentSource;
    std::optional<Type> expectedReturnType;
    std::optional<std::wstring> structType;
    Type::InitializationList structInitListType;
    Type lastExpressionType;
    std::unordered_map<std::wstring, std::pair<Type, bool>> variableTypes;
    bool noReturnFunctionPermitted, variantReadAccessPermitted, blockFurtherDotAccess;
    std::unique_ptr<Expression> *replaceableExpression;

    void visitExpression(std::unique_ptr<Expression> &expression)
    {
        variantReadAccessPermitted = false;
        replaceableExpression = &expression;
        expression->accept(*this);
    }

    void ensureExpressionHasType(std::unique_ptr<Expression> &expression, Type desiredType)
    {
        visitExpression(expression);
        if(lastExpressionType != desiredType)
            insertCast(expression, lastExpressionType, desiredType);
    }

    Type getTargetTypeForEquality(Type leftType, Type rightType)
    {
        if(leftType == Type{STR} || rightType == Type{STR})
            return Type{STR};
        if(leftType == Type{FLOAT} || rightType == Type{FLOAT})
            return Type{FLOAT};
        if(leftType == Type{INT} || rightType == Type{INT})
            return Type{INT};
        return Type{BOOL};
    }

    template <typename EqualityExpression>
    void visitEqualityExpression(EqualityExpression &visited)
    {
        visitExpression(visited.left);
        Type leftType = lastExpressionType;
        visitExpression(visited.right);
        Type rightType = lastExpressionType;
        lastExpressionType = Type{BOOL};

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
        Type targetType = getTargetTypeForEquality(leftType, rightType);
        if(leftType != targetType)
            return insertCast(visited.left, leftType, targetType);
        if(rightType != targetType)
            return insertCast(visited.right, rightType, targetType);
    }

    template <typename TypedEqualityExpression>
    void visitTypedEqualityExpression(TypedEqualityExpression &visited)
    {
        visitExpression(visited.left);
        Type leftType = lastExpressionType;
        visitExpression(visited.right);
        Type rightType = lastExpressionType;
        lastExpressionType = Type{BOOL};

        if(leftType.isInitList())
            return insertCast(visited.left, leftType, rightType);
        if(rightType.isInitList())
            return insertCast(visited.right, rightType, leftType);
    }

    template <typename IntOrFloatExpression>
    Type visitIntOrFloatExpression(IntOrFloatExpression &expression)
    {
        visitExpression(expression.left);
        Type leftType = lastExpressionType;
        visitExpression(expression.right);
        Type rightType = lastExpressionType;

        if(leftType == Type{INT} && rightType == Type{INT})
            return Type{INT};
        if(leftType != Type{FLOAT})
            insertCast(expression.left, leftType, Type{FLOAT});
        if(rightType != Type{FLOAT})
            insertCast(expression.right, rightType, Type{FLOAT});
        return Type{FLOAT};
    }

    std::wstring getTypeNameForDotExpression(Type leftType, Position position)
    {
        if(lastExpressionType.isInitList())
            throw InvalidInitListError(
                L"Structure initialization list is not allowed with '.' operator", currentSource, position
            );
        if(lastExpressionType.isBuiltin())
            throw FieldAccessError(L"Attempted access to field of simple type {}", currentSource, position);
        return std::get<std::wstring>(leftType.value);
    }

    // Expects the struct expression's initialization list type in lastExpressionType
    void setStructExpressionType(StructExpression &visited)
    {
        visited.structType = structType;
        for(auto &argument: visited.arguments)
        {
            std::vector<Field> &structFields = *getStructOrVariantFields(*structType);
            for(unsigned i = 0; i < structFields.size(); i++)
            {
                if(structInitListType[i] != structFields[i].type)
                    insertCast(argument, structInitListType[i], structFields[i].type);
            }
        }
        structType = std::nullopt;
    }

    void visitLeafAssignable(Assignable &visited)
    {
        auto variableFound = variableTypes.find(visited.right);
        if(variableFound == variableTypes.end())
            throw UnknownVariableError(
                std::format(L"{} is not a variable", visited.right), currentSource, visited.getPosition()
            );
        if(!variableFound->second.second)
            throw ImmutableError(
                std::format(L"Attempted to modify immutable variable {}", visited.right), currentSource,
                visited.getPosition()
            );
        lastExpressionType = variableFound->second.first;
    }

    Type getTypeOfField(const std::vector<Field> &fields, std::wstring fieldName, Position position)
    {
        auto fieldFound = std::find_if(fields.begin(), fields.end(), [&](const Field &field) {
            return field.name == fieldName;
        });
        if(fieldFound == fields.end())
            throw FieldAccessError(
                std::format(L"Attempted access to nonexistent field {} of type {}", fieldName, lastExpressionType),
                currentSource, position
            );
        return fieldFound->type;
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
            lastExpressionType = getTypeOfField((*structFound)->second.fields, fieldName, position);
            return;
        }
        if(auto variantFound = findIn(program.variants, complexTypeName))
        {
            lastExpressionType = getTypeOfField((*variantFound)->second.fields, fieldName, position);
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
        return std::find_if(variantFields.begin(), variantFields.end(), [&](const Field &field) {
                   return field.type == fieldType;
               }) != variantFields.end();
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
        {
            std::wstring typeName = std::get<std::wstring>(typeFrom.value);
            if(variantReadAccessPermitted)
                return isFieldOfVariant(typeName, typeTo);
            else
                return false;
        }
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
        {
            structType = std::get<std::wstring>(typeTo.value);
            structInitListType = std::get<Type::InitializationList>(typeFrom.value);
            expression->accept(*this);
        }
        else
            expression = std::make_unique<CastExpression>(expression->getPosition(), std::move(expression), typeTo);
    }

    bool isValidType(Type type)
    {
        if(type.isInitList())
            return false;
        if(type.isBuiltin())
            return true;
        std::wstring typeName = std::get<std::wstring>(type.value);
        return program.structs.find(typeName) != program.structs.end() ||
               program.variants.find(typeName) != program.variants.end();
    }

    // Returns std::nullopt if no type of the given name exists.
    std::optional<std::reference_wrapper<std::vector<Field>>> getStructOrVariantFields(const std::wstring &name)
    {
        auto foundStruct = program.structs.find(name);
        if(foundStruct != program.structs.end())
            return foundStruct->second.fields;
        auto foundVariant = program.variants.find(name);
        if(foundVariant != program.variants.end())
            return foundVariant->second.fields;
        return std::nullopt;
    }

    void checkNameDuplicates(Program &visited)
    {
        for(auto &[id, function]: visited.functions)
        {
            auto variantFound = visited.variants.find(id.name);
            if(variantFound != visited.variants.end())
                throw NameCollisionError(
                    L"function " + id.name, function, L"variant " + variantFound->first, variantFound->second
                );
            auto structFound = visited.structs.find(id.name);
            if(structFound != visited.structs.end())
                throw NameCollisionError(
                    L"function " + id.name, function, L"struct " + structFound->first, structFound->second
                );
        }
        for(auto &[name, variant]: visited.variants)
        {
            auto structFound = visited.structs.find(name);
            if(structFound != visited.structs.end())
                throw NameCollisionError(
                    L"variant " + name, variant, L"struct " + structFound->first, structFound->second
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

        for(const Field &field: getStructOrVariantFields(type2Name)->get())
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
        variableTypes.clear();
        for(VariableDeclaration &parameter: parameters)
            parameter.accept(*this);
    }
};

}

void doSemanticAnalysis(Program &program)
{
    SemanticAnalyzer(program).visit(program);
}
