#include "streamReader.hpp"

StreamReader::StreamReader(std::wistream &source): source(source), current(0), currentPosition(Position{1, 0})
{
    this->next(); // set to the first character of input
}

void StreamReader::next()
{
    if(this->current == EOT)
        return;

    if(this->current == L'\n')
    {
        this->currentPosition.line += 1;
        this->currentPosition.column = 1;
    }
    else
        this->currentPosition.column += 1;

    this->current = source.get();
    if(source.eof())
    {
        this->current = EOT;
        return;
    }
    if(this->current == L'\r')
    {
        if(source.peek() == L'\n')
            source.get();
        this->current = L'\n';
    }
}

wchar_t StreamReader::get()
{
    return this->current;
}

Position StreamReader::getPosition()
{
    return this->currentPosition;
}
