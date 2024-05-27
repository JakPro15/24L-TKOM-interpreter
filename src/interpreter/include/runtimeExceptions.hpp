#ifndef RUNTIMEEXCEPTIONS_HPP
#define RUNTIMEEXCEPTIONS_HPP

#include "documentTree.hpp"
#include "readerExceptions.hpp"

#include <string>
#include <vector>

class RuntimeError: public ErrorAtPosition
{
    using ErrorAtPosition::ErrorAtPosition;
};

#define DECLARE_RUNTIME_ERROR(class_name) \
    class class_name: public RuntimeError \
    {                                     \
        using RuntimeError::RuntimeError; \
    }

DECLARE_RUNTIME_ERROR(BuiltinFunctionArgumentError);
DECLARE_RUNTIME_ERROR(IntegerRangeError);
DECLARE_RUNTIME_ERROR(StandardInputError);
DECLARE_RUNTIME_ERROR(StandardOutputError);
DECLARE_RUNTIME_ERROR(MainNotFoundError);

class RuntimeSemanticException: public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

#endif
