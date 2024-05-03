#ifndef PARSEREXCEPTIONS_HPP
#define PARSEREXCEPTIONS_HPP

#include "readerExceptions.hpp"

class SyntaxError: public ErrorAtPosition
{
    using ErrorAtPosition::ErrorAtPosition;
};

#endif
