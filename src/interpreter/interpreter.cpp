#include "interpreter.hpp"

Interpreter::Interpreter(std::vector<std::wstring> arguments, std::istream &input, std::ostream &output):
    arguments(arguments), input(input), output(output)
{}
