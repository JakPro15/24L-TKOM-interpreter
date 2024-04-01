#include "reader.hpp"

Reader::Reader(std::wistream &source): source(source)
{
    this->next(); // set to the first character of input
}

void Reader::next()
{
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
