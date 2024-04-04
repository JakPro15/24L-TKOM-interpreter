#ifndef IREADER_HPP
#define IREADER_HPP

#include "position.hpp"

class IReader
{
public:
    static const wchar_t EOT = L'\3';

    // Advances to the next wide character of input.
    virtual void next() = 0;

    // Returns the current character of input.
    // Returns EOT when end of input is reached.
    virtual wchar_t get() = 0;

    // Returns the position of the current character in input.
    // Line and column numbers start from 1.
    virtual Position getPosition() = 0;
};

#endif
