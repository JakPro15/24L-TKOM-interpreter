#include "commentDiscarder.hpp"
#include "convertToString.hpp"
#include "documentTree.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "semanticExceptions.hpp"
#include "streamReader.hpp"

#include <format>
#include <vector>

void mergePrograms(Program &program, Program &toAdd)
{
    for(auto &include: toAdd.includes)
        program.includes.push_back(std::move(include));

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

#include <iostream>

void executeIncludes(
    Program &program, std::vector<std::wstring> &sourceFiles, std::function<Program(const std::wstring &)> parseFromFile
)
{
    for(IncludeStatement &include: program.includes)
    {
        if(std::find(sourceFiles.begin(), sourceFiles.end(), include.filePath) != sourceFiles.end())
            continue;
        sourceFiles.push_back(include.filePath);
        Program newProgram = parseFromFile(include.filePath);
        executeIncludes(newProgram, sourceFiles, parseFromFile);
        mergePrograms(program, newProgram);
    }
    program.includes.clear();
}
