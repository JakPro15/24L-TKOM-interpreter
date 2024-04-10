#include "readerExceptions.hpp"

#include <codecvt>
#include <format>
#include <iostream>
#include <locale>
#include <sstream>

ErrorAtPosition::ErrorAtPosition(std::wstring message, Position position): message(message), position(position)
{
    // prepare char string for what()
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::string convertedMessage = converter.to_bytes(message).c_str();
    whatMessage = std::format("Error: {} at line {}, column {}", convertedMessage, position.line, position.column);
}

const char *ErrorAtPosition::what() const noexcept
{
    return whatMessage.c_str();
}

std::wstring ErrorAtPosition::getMessage() const
{
    return message;
}

Position ErrorAtPosition::getPosition() const
{
    return position;
}
