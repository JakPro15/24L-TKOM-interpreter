#include "readerExceptions.hpp"

#include "convertToString.hpp"

#include <format>

namespace {
std::string formatErrorMessage(std::wstring sourceName, std::wstring message, Position position)
{
    return std::format(
        "Error: {}\nin file {}\nat line {}, column {}.", convertToString(sourceName), convertToString(message),
        position.line, position.column
    );
}
}

ErrorAtPosition::ErrorAtPosition(std::wstring sourceName, std::wstring message, Position position):
    std::runtime_error(formatErrorMessage(sourceName, message, position))
{}
