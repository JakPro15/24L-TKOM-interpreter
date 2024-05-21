#include "semanticAnalysis.hpp"

#include "interpreterExceptions.hpp"

#include <iostream>
#include <unordered_map>
#include <unordered_set>

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
        program(program), noReturnFunctionPermitted(false), castFromVariantPermitted(false)
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
        lastExpressionType = found->second;
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
    }

    void visit(VariableDeclaration &visited) override
    {
        if(variableTypes.find(visited.name) != variableTypes.end())
            throw VariableNameCollisionError(
                std::format(L"There already exists a variable with this name: {}", visited.name), currentSource,
                visited.getPosition()
            );
        if(!visited.type.isBuiltin() && !getStructOrVariantFields(std::get<std::wstring>(visited.type.value)))
            throw UnknownVariableTypeError(
                std::format(L"{} is not a type", visited.type), currentSource, visited.getPosition()
            );
        variableTypes.insert({visited.name, visited.type});
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

    bool areTypesConvertible(Type typeFrom, Type typeTo)
    {
        if(!typeFrom.isBuiltin())
        {
            std::wstring typeName = std::get<std::wstring>(typeFrom.value);
            if(castFromVariantPermitted)
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
        expression = std::make_unique<CastExpression>(expression->getPosition(), std::move(expression), typeTo);
    }

    void visit(VariableDeclStatement &visited) override
    {
        visited.declaration.accept(*this);
        visited.value->accept(*this);
        if(lastExpressionType != visited.declaration.type)
            insertCast(visited.value, lastExpressionType, visited.declaration.type);
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

    void ensureFieldAccessible(std::wstring complexTypeName, std::wstring fieldName, Position position)
    {
        auto structFound = program.structs.find(complexTypeName);
        if(structFound != program.structs.end())
        {
            lastExpressionType = getTypeOfField(structFound->second.fields, fieldName, position);
            return;
        }
        auto variantFound = program.variants.find(complexTypeName);
        if(variantFound != program.variants.end())
        {
            lastExpressionType = getTypeOfField(variantFound->second.fields, fieldName, position);
            blockFurtherDotAccess = true;
            return;
        }
        throw UnknownVariableTypeError(
            std::format(L"Internal error - {} is not a valid type", complexTypeName), currentSource, position
        );
    }

    void visit(Assignable &visited) override
    {
        if(visited.left == nullptr)
        {
            auto variableFound = variableTypes.find(visited.right);
            if(variableFound == variableTypes.end())
                throw UnknownVariableError(
                    std::format(L"{} is not a variable", visited.right), currentSource, visited.getPosition()
                );
            lastExpressionType = variableFound->second;
            return;
        }
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
        ensureFieldAccessible(
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

    void parametersToVariables(std::vector<VariableDeclaration> &parameters)
    {
        variableTypes.clear();
        for(VariableDeclaration &parameter: parameters)
            parameter.accept(*this);
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

    // Returns whether type1 (struct or variant type) is among the subtypes of type2.
    bool isInSubtypes(const std::wstring &type1, const Type &type2)
    {
        if(type2.isBuiltin())
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

    void ensureFieldTypesExist(const std::vector<Field> &fields)
    {
        for(const Field &field: fields)
        {
            if(field.type.isBuiltin())
                continue;
            std::wstring typeName = std::get<std::wstring>(field.type.value);
            if(!getStructOrVariantFields(typeName).has_value())
                throw UnknownFieldTypeError(
                    std::format(L"{} is not a type", typeName), currentSource, field.getPosition()
                );
        }
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

    template <typename StructOrVariantDeclaration>
    void checkStructOrVariant(const std::wstring &name, const StructOrVariantDeclaration &toCheck)
    {
        currentSource = toCheck.getSource();
        checkFieldNameDuplicates(toCheck.fields);
        ensureFieldTypesExist(toCheck.fields);
        checkTypeRecursion(name, toCheck.fields);
    }

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
    Type lastExpressionType;
    std::unordered_map<std::wstring, Type> variableTypes;
    bool noReturnFunctionPermitted, castFromVariantPermitted, blockFurtherDotAccess;
};

}

void doSemanticAnalysis(Program &program)
{
    SemanticAnalyzer(program).visit(program);
}
