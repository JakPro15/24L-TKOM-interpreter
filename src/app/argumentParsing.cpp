#include "argumentParsing.hpp"

#include "appExceptions.hpp"
#include "convertToString.hpp"

#include <algorithm>
#include <format>
#include <iostream>

std::vector<std::string> getArguments(int argc, const char * const argv[])
{
    std::vector<std::string> arguments;
    for(int i = 1; i < argc; i++)
        arguments.emplace_back(argv[i]);
    return arguments;
}

Arguments parseArguments(int argc, const char * const argv[])
{
    std::vector<std::string> args = getArguments(argc, argv);
    Arguments arguments = {{}, false, {}};
    bool files = true;
    for(const std::string &arg: args)
    {
        std::wstring argument = convertToWstring(arg);
        if(argument == L"--dump-dt")
            arguments.dumpDocumentTree = true;
        else if(argument == L"--args")
            files = false;
        else if(files)
        {
            if(std::find(arguments.files.begin(), arguments.files.end(), argument) != arguments.files.end())
                throw DuplicateFileError(std::format("File name given to interpreter more than once: {}", arg));
            arguments.files.push_back(argument);
        }
        else
            arguments.programArguments.push_back(argument);
    }
    if(arguments.files.empty())
        throw NoFilesError("No source code files given to interpreter");
    return arguments;
}
