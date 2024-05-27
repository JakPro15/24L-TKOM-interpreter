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

class BuiltinFunctionArgumentError: public RuntimeError
{
    using RuntimeError::RuntimeError;
};

class IntegerRangeError: public RuntimeError
{
    using RuntimeError::RuntimeError;
};

#endif
