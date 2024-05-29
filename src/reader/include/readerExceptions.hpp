#ifndef READEREXCEPTIONS_HPP
#define READEREXCEPTIONS_HPP

#include "position.hpp"

#include <stdexcept>

class InterpreterPipelineError: public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

class ErrorAtPosition: public InterpreterPipelineError
{
public:
    ErrorAtPosition(std::wstring message, std::wstring sourceName, Position position);
};

class ReaderException: public ErrorAtPosition
{
    using ErrorAtPosition::ErrorAtPosition;
};

#define DECLARE_READER_ERROR(class_name)        \
    class class_name: public ReaderException    \
    {                                           \
        using ReaderException::ReaderException; \
    };

DECLARE_READER_ERROR(ControlCharError);
DECLARE_READER_ERROR(ReaderInputError);

#endif
