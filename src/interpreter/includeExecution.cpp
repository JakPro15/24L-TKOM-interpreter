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

namespace {
void addProgram(Program &program, Program &toAdd)
{
    for(auto &variant: toAdd.variants)
        program.add(std::move(variant));

    for(auto &structure: toAdd.structs)
        program.add(std::move(structure));

    for(auto &function: toAdd.functions)
    {
        if(program.functions.find(function.first) != program.functions.end())
        {
            throw DuplicateFunctionError(
                std::format(L"Duplicate function with signature {}", function.first), function.second->getSource(),
                function.second->getPosition()
            );
        }
        program.functions.insert(std::move(function));
    }
}

void executeAllIncludes(Program &program, std::wstring programSource, std::vector<std::wstring> &pastIncludes)
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
        std::wifstream inputFile(convertToString(include.filePath));
        if(!inputFile.is_open())
            throw FileError(
                std::format(L"Error opening file {} required from include statement", programSource), include.filePath,
                include.getPosition()
            );
        StreamReader reader(inputFile, include.filePath);
        Lexer lexer(reader);
        CommentDiscarder commentDiscarder(lexer);
        Parser parser(lexer);
        Program newProgram = parser.parseProgram();
        executeAllIncludes(newProgram, include.filePath, pastIncludes);
        addProgram(program, newProgram);
    }
    pastIncludes.pop_back();
}
}

void executeIncludes(Program &program, std::wstring programSource)
{
    std::vector<std::wstring> pastIncludes;
    executeAllIncludes(program, programSource, pastIncludes);
}
