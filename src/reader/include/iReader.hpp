#ifndef IREADER_HPP
#define IREADER_HPP

#include "position.hpp"

#include <utility>

class IReader
{
public:
    static const wchar_t EOT = L'\3';

    // Advances to the next wide character of input.
    virtual void next() = 0;

    // Returns the current character of input with its position.
    // Line and column numbers start from 1.
    // Returns EOT when end of input is reached.
    virtual std::pair<wchar_t, Position> get() = 0;

    virtual ~IReader() {}
};

#endif
