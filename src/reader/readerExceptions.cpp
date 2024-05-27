#include "readerExceptions.hpp"

#include "convertToString.hpp"

#include <format>

namespace {
std::string formatErrorMessage(std::wstring message, std::wstring sourceName, Position position)
{
    return std::format(
        "Error: {}\nin file {}\nat line {}, column {}.", convertToString(message), convertToString(sourceName),
        position.line, position.column
    );
}
}

ErrorAtPosition::ErrorAtPosition(std::wstring message, std::wstring sourceName, Position position):
    std::runtime_error(formatErrorMessage(message, sourceName, position))
{}
