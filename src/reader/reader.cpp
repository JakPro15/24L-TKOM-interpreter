#include "reader.hpp"

Position::Position(unsigned line, unsigned column): line(line), column(column) {}

bool Position::operator==(const Position &other) const
{
    return this->line == other.line && this->column == other.column;
}

Reader::Reader(std::wistream &source): source(source), current(0), currentPosition(Position{1, 0})
{
    this->next(); // set to the first character of input
}

void Reader::next()
{
    if(this->current == ETX)
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
        this->current = ETX;
        return;
    }
    if(this->current == L'\r')
    {
        if(source.peek() == L'\n')
            source.get();
        this->current = L'\n';
    }
}

wchar_t Reader::get()
{
    return this->current;
}

Position Reader::getPosition()
{
    return this->currentPosition;
}
