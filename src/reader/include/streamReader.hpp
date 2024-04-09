#ifndef STREAMREADER_HPP
#define STREAMREADER_HPP

#include "iReader.hpp"

#include <istream>

class StreamReader final: public IReader
{
public:
    StreamReader(std::wistream &source);
    void next() override;
    std::pair<wchar_t, Position> get() override;
private:
    std::wistream &source;
    wchar_t current;
    Position currentPosition;
};

#endif
