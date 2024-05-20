#include "semanticAnalysis.hpp"

#include "interpreterExceptions.hpp"

#define EMPTY_VISIT(type)              \
    void visit(type &visited) override \
    {                                  \
        static_cast<void>(visited);    \
    }

namespace {
class SemanticAnalyzer: public DocumentTreeVisitor
{
public:
    explicit SemanticAnalyzer(Program &visitedProgram): program(program) {}

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

    void visit(FunctionDeclaration &visited) override
    {
        static_cast<void>(visited);
    }

    EMPTY_VISIT(StructDeclaration);
    EMPTY_VISIT(VariantDeclaration);
    EMPTY_VISIT(IncludeStatement);

    // Returns std::nullopt if no type of the given name exists.
    std::optional<std::reference_wrapper<std::vector<Field>>> getStructOrVariantFields(const std::wstring &name)
    {
        auto foundStruct = std::find(program.structs.begin(), program.structs.end(), name);
        if(foundStruct != program.structs.end())
            return foundStruct->second.fields;
        auto foundVariant = std::find(program.variants.begin(), program.variants.end(), name);
        if(foundVariant != program.variants.end())
            return foundVariant->second.fields;
        return std::nullopt;
    }

    // Returns whether type1 (struct or variant type) is among the subtypes of type2.
    bool isInSubtypes(const std::wstring &type1, const Type &type2)
    {
        if(std::holds_alternative<Type::Builtin>(type2.value))
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
            if(std::holds_alternative<Type::Builtin>(field.type.value))
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

    void visit(Program &visited) override
    {
        if(!visited.includes.empty())
            throw std::logic_error("Program's include statements should be executed before calling doSemanticAnalysis");

        checkNameDuplicates(visited);
        for(auto &[name, variant]: visited.variants)
        {
            currentSource = variant.getSource();
            ensureFieldTypesExist(variant.fields);
            checkTypeRecursion(name, variant.fields);
        }
        for(auto &[name, structure]: visited.structs)
        {
            currentSource = structure.getSource();
            ensureFieldTypesExist(structure.fields);
            checkTypeRecursion(name, structure.fields);
        }
        for(auto &function: visited.functions)
        {
            currentSource = function.second.getSource();
            function.second.accept(*this);
        }
    }
private:
    Program &program;
    std::wstring currentSource;
};
}

void doSemanticAnalysis(Program &program)
{
    SemanticAnalyzer(program).visit(program);
}
