#ifndef IREADER_HPP
#define IREADER_HPP

#include "position.hpp"

#define ETX wchar_t(3)

class IReader
{
    // Advances to the next wide character of input.
    virtual void next() = 0;

    // Returns the current character of input.
    // Returns ETX when end of input is reached.
    virtual wchar_t get() = 0;

    // Returns the position of the current character in input.
    // Line and column numbers start from 1.
    virtual Position getPosition() = 0;
};

#endif
