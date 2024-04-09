#include "readerExceptions.hpp"

#include <codecvt>
#include <locale>

ErrorAtPosition::ErrorAtPosition(std::wstring message, Position position): message(message), position(position) {}

const char *ErrorAtPosition::what() const noexcept
{
    // conversion as std::exception::what must return a char string, not wchar_t string
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.to_bytes(message).c_str();
}

std::wstring ErrorAtPosition::getMessage() const
{
    return message;
}

Position ErrorAtPosition::getPosition() const
{
    return position;
}
