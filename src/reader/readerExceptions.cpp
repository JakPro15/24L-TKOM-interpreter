#include "readerExceptions.hpp"

#include <codecvt>
#include <format>
#include <locale>

namespace {
std::string formatErrorMessage(std::wstring message, Position position)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::string convertedMessage = converter.to_bytes(message);
    return std::format("Error: {}\nat line {}, column {} of input.", convertedMessage, position.line, position.column);
}
}

ErrorAtPosition::ErrorAtPosition(std::wstring message, Position position):
    std::runtime_error(formatErrorMessage(message, position))
{}
