#include "streamReader.hpp"

#include <format>

StreamReader::StreamReader(std::wistream &source, IErrorHandler &errorHandler):
    source(source), errorHandler(errorHandler), current(0), currentPosition(Position{1, 0})
{
    next(); // set to the first character of input
}

void StreamReader::next()
{
    if(current == EOT)
        return;

    if(current == L'\n')
    {
        currentPosition.line += 1;
        currentPosition.column = 1;
    }
    else
        currentPosition.column += 1;

    current = source.get();
    if(current != L'\r' && current != L'\n' && std::iswcntrl(current))
        errorHandler.handleError(
            Error::READER_CONTROL_CHAR, std::format(L"Control character encountered in input: \\x{:x}", current),
            currentPosition
        );
    if(source.eof())
    {
        current = EOT;
        return;
    }
    else if(source.bad() || source.fail())
        errorHandler.handleError(Error::READER_INPUT_ERROR, L"Input stream returned error", currentPosition);

    if(current == L'\r')
    {
        if(source.peek() == L'\n')
            source.get();
        current = L'\n';
    }
}

wchar_t StreamReader::get()
{
    return current;
}

Position StreamReader::getPosition()
{
    return currentPosition;
}
