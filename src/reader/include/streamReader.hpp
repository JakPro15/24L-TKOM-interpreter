#ifndef STREAMREADER_HPP
#define STREAMREADER_HPP

#include "iReader.hpp"

#include <istream>

class StreamReader final: public IReader
{
public:
    StreamReader(std::wistream &source);

    // Advances to the next wide character of input.
    void next() override;

    // Returns the current character of input.
    wchar_t get() override;

    // Returns the position of the current character in input.
    // Line and column numbers start from 1.
    Position getPosition() override;
private:
    std::wistream &source;
    wchar_t current;
    Position currentPosition;
};

#endif
