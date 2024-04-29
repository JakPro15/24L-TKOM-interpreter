#ifndef PARSEREXCEPTIONS_HPP
#define PARSEREXCEPTIONS_HPP

#include "readerExceptions.hpp"

class ParserException: public ErrorAtPosition
{
    using ErrorAtPosition::ErrorAtPosition;
};

class SyntaxError: public ParserException
{
    using ParserException::ParserException;
};

#endif
