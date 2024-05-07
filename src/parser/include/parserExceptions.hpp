#ifndef PARSEREXCEPTIONS_HPP
#define PARSEREXCEPTIONS_HPP

#include "readerExceptions.hpp"

class ParserError: public ErrorAtPosition
{
    using ErrorAtPosition::ErrorAtPosition;
};

class SyntaxError: public ParserError
{
    using ParserError::ParserError;
};

class DuplicateFunctionError: public ParserError
{
    using ParserError::ParserError;
};

class DuplicateStructError: public ParserError
{
    using ParserError::ParserError;
};

class DuplicateVariantError: public ParserError
{
    using ParserError::ParserError;
};

#endif
