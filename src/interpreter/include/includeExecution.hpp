#include "documentTree.hpp"

void executeIncludes(
    Program &program, const std::wstring &programSource, std::function<Program(const std::wstring &)> parseFromFile
);
void mergePrograms(Program &program, Program &toAdd);
