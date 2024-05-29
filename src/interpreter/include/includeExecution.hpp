#include "documentTree.hpp"

void executeIncludes(
    Program &program, std::vector<std::wstring> &sourceFiles, std::function<Program(const std::wstring &)> parseFromFile
);
void mergePrograms(Program &program, Program &toAdd);
