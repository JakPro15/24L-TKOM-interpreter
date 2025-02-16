#ifndef SEMANTICEXCEPTIONS_HPP
#define SEMANTICEXCEPTIONS_HPP

#include "documentTree.hpp"
#include "readerExceptions.hpp"

#include <string>
#include <vector>

class IncludeInSemanticAnalysisError: public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

class InvalidTypeError: public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

class SemanticError: public ErrorAtPosition
{
    using ErrorAtPosition::ErrorAtPosition;
};

#define DECLARE_SEMANTIC_ERROR(class_name)  \
    class class_name: public SemanticError  \
    {                                       \
        using SemanticError::SemanticError; \
    }

DECLARE_SEMANTIC_ERROR(UnknownFieldTypeError);
DECLARE_SEMANTIC_ERROR(UnknownVariableTypeError);
DECLARE_SEMANTIC_ERROR(FieldTypeRecursionError);

class NameCollisionError: public ErrorAtPosition
{
public:
    template <typename TopLevelStmt1, typename TopLevelStmt2>
    NameCollisionError(std::wstring firstName, TopLevelStmt1 &first, std::wstring secondName, TopLevelStmt2 &second):
        ErrorAtPosition(
            std::format(
                L"Name collision of {}\n"
                L"in file {}\n"
                L"at line {}, column {}\n"
                L"and {}",
                firstName, first.getSource(), first.getPosition().line, first.getPosition().column, secondName
            ),
            second.getSource(), second.getPosition()
        )
    {}
};

DECLARE_SEMANTIC_ERROR(FieldNameCollisionError);
DECLARE_SEMANTIC_ERROR(FieldTypeCollisionError);
DECLARE_SEMANTIC_ERROR(VariableNameCollisionError);
DECLARE_SEMANTIC_ERROR(UnknownVariableError);
DECLARE_SEMANTIC_ERROR(InvalidCastError);
DECLARE_SEMANTIC_ERROR(FieldAccessError);
DECLARE_SEMANTIC_ERROR(InvalidOperatorArgsError);
DECLARE_SEMANTIC_ERROR(InvalidInitListError);
DECLARE_SEMANTIC_ERROR(ImmutableError);
DECLARE_SEMANTIC_ERROR(InvalidFunctionCallError);
DECLARE_SEMANTIC_ERROR(AmbiguousFunctionCallError);
DECLARE_SEMANTIC_ERROR(InvalidReturnError);
DECLARE_SEMANTIC_ERROR(InvalidBreakError);
DECLARE_SEMANTIC_ERROR(InvalidContinueError);
DECLARE_SEMANTIC_ERROR(InvalidIfConditionError);
DECLARE_SEMANTIC_ERROR(InvalidOverloadError);

#endif
