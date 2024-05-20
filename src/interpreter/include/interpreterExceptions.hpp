#ifndef INTERPRETEREXCEPTIONS_HPP
#define INTERPRETEREXCEPTIONS_HPP

#include "documentTree.hpp"
#include "readerExceptions.hpp"

#include <string>
#include <vector>

class FileError: public ErrorAtPosition
{
    using ErrorAtPosition::ErrorAtPosition;
};

class CircularIncludeError: public ErrorAtPosition
{
    using ErrorAtPosition::ErrorAtPosition;
};

class SemanticError: public ErrorAtPosition
{
    using ErrorAtPosition::ErrorAtPosition;
};

#define DECLARE_SEMANTIC_ERROR(class_name)  \
    class class_name: public SemanticError  \
    {                                       \
        using SemanticError::SemanticError; \
    };

DECLARE_SEMANTIC_ERROR(UnknownFieldTypeError);
DECLARE_SEMANTIC_ERROR(FieldTypeRecursionError);

class NameCollisionError: public ErrorAtPosition
{
public:
    template <typename T1, typename T2>
    NameCollisionError(std::wstring firstName, T1 &first, std::wstring secondName, T2 &second):
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

#endif
