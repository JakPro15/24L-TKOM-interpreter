#ifndef STREAMREADER_HPP
#define STREAMREADER_HPP

#include "iReader.hpp"

#include <istream>

class StreamReader final: public IReader
{
public:
    StreamReader(std::wistream &source, std::wstring sourceName);
    std::pair<wchar_t, Position> next() override;
    std::pair<wchar_t, Position> get() override;
private:
    std::wistream &source;
    std::wstring sourceName;
    wchar_t current;
    Position currentPosition;
};

#endif
