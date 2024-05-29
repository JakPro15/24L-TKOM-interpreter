#ifndef RUNTIMEEXCEPTIONS_HPP
#define RUNTIMEEXCEPTIONS_HPP

#include "documentTree.hpp"
#include "readerExceptions.hpp"

#include <string>
#include <vector>

class RuntimeError: public InterpreterPipelineError
{
public:
    RuntimeError(std::wstring message, std::wstring sourceName, Position position);
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
DECLARE_RUNTIME_ERROR(MainReturnTypeError);
DECLARE_RUNTIME_ERROR(CastImpossibleError);
DECLARE_RUNTIME_ERROR(OperatorArgumentError);
DECLARE_RUNTIME_ERROR(ZeroDivisionError);
DECLARE_RUNTIME_ERROR(StackOverflowError);

class RuntimeSemanticException: public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

#endif
