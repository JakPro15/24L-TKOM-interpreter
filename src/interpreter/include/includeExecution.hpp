#include "documentTree.hpp"

void executeIncludes(
    Program &program, const std::wstring &programSource, std::function<Program(std::wstring)> parseFromFile
);
void mergePrograms(Program &program, Program &toAdd);
