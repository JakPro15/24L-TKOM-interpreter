#include "streamReader.hpp"

#include "readerExceptions.hpp"

#include <format>

StreamReader::StreamReader(std::wistream &source): source(source), current(0), currentPosition(Position(1, 0))
{
    next(); // set to the first character of input
}

std::pair<wchar_t, Position> StreamReader::next()
{
    if(current == EOT)
        return get();

    if(current == L'\n')
    {
        currentPosition.line += 1;
        currentPosition.column = 1;
    }
    else
        currentPosition.column += 1;

    current = source.get();
    if(!std::iswspace(current) && std::iswcntrl(current))
        throw ControlCharError(
            std::format(L"Control character encountered in input: \\x{:x}", current), currentPosition
        );
    if(source.eof())
    {
        current = EOT;
        return get();
    }
    else if(source.bad() || source.fail())
        throw ReaderInputError(L"Input stream returned error", currentPosition);

    if(current == L'\r')
    {
        if(source.peek() == L'\n')
            source.get();
        current = L'\n';
    }
    return get();
}

std::pair<wchar_t, Position> StreamReader::get()
{
    return {current, currentPosition};
}
