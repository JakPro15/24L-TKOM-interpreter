#include "commentDiscarder.hpp"
#include "convertToString.hpp"
#include "documentTree.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "semanticExceptions.hpp"
#include "streamReader.hpp"

#include <format>
#include <fstream>
#include <vector>

void mergePrograms(Program &program, Program &toAdd)
{
    for(auto &variant: toAdd.variants)
        program.add(std::move(variant));

    for(auto &structure: toAdd.structs)
        program.add(std::move(structure));

    for(auto &function: toAdd.functions)
    {
        if(program.functions.count(function.first) == 1)
            throw DuplicateFunctionError(
                std::format(L"Duplicate function with signature {}", function.first), function.second->getSource(),
                function.second->getPosition()
            );
        program.functions.insert(std::move(function));
    }
}

namespace {
void executeAllIncludes(
    Program &program, const std::wstring &programSource, std::vector<std::wstring> &pastIncludes,
    std::function<Program(std::wstring)> parseFromFile
)
{
    pastIncludes.push_back(programSource);
    std::vector<IncludeStatement> includes = program.includes;
    program.includes.clear();
    for(IncludeStatement &include: includes)
    {
        if(std::find(pastIncludes.begin(), pastIncludes.end(), include.filePath) != pastIncludes.end())
            throw CircularIncludeError(
                std::format(L"Circular include of file {} detected", programSource), include.filePath,
                include.getPosition()
            );
        Program newProgram = parseFromFile(include.filePath);
        executeAllIncludes(newProgram, include.filePath, pastIncludes, parseFromFile);
        mergePrograms(program, newProgram);
    }
    pastIncludes.pop_back();
}
}

void executeIncludes(
    Program &program, const std::wstring &programSource, std::function<Program(std::wstring)> parseFromFile
)
{
    std::vector<std::wstring> pastIncludes;
    executeAllIncludes(program, programSource, pastIncludes, parseFromFile);
}
