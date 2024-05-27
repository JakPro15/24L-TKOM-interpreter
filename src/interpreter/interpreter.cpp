#include "interpreter.hpp"

Interpreter::Interpreter(
    std::vector<std::wstring> arguments, std::istream &input, std::ostream &output,
    std::function<Program(std::wifstream &, std::wstring)> parseFromFile
): arguments(arguments), input(input), output(output), parseFromFile(parseFromFile)
{}

#define EMPTY_VISIT(type) \
    void Interpreter::visit(type &visited) {}
