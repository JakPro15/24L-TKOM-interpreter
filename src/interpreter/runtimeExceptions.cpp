#include "runtimeExceptions.hpp"

#include "convertToString.hpp"

#include <format>

namespace {
std::string formatErrorMessage(std::wstring message, std::wstring sourceName, Position position)
{
    return std::format(
        "The program was terminated following a runtime error:\n"
        "{}\nwhile executing file {}\nat line {}, column {}.",
        convertToString(message), convertToString(sourceName), position.line, position.column
    );
}
}

RuntimeError::RuntimeError(std::wstring message, std::wstring sourceName, Position position):
    InterpreterPipelineError(formatErrorMessage(message, sourceName, position))
{}
