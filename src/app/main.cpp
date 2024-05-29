#include "appExceptions.hpp"
#include "argumentParsing.hpp"
#include "commentDiscarder.hpp"
#include "convertToString.hpp"
#include "includeExecution.hpp"
#include "interpreter.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "printingVisitor.hpp"
#include "streamReader.hpp"

#include <fstream>
#include <iostream>

Program parseFromStream(std::wistream &input, const std::wstring &inputName)
{
    StreamReader reader(input, inputName);
    Lexer lexer(reader);
    CommentDiscarder commentDiscarder(lexer);
    Parser parser(commentDiscarder);
    return parser.parseProgram();
}

Program parseFromFile(const std::wstring &fileName)
{
    std::string fileNameString = convertToString(fileName);
    std::wifstream fileStream(fileNameString);
    if(!fileStream.is_open())
        throw FileError(std::format("Failed to open file {}", fileNameString));
    return parseFromStream(fileStream, fileName);
}

Program loadProgram(const std::vector<std::wstring> &files)
{
    Program program = parseFromFile(files[0]);
    std::for_each(files.begin() + 1, files.end(), [&](const std::wstring &fileName) {
        Program next = parseFromFile(fileName);
        mergePrograms(program, next);
    });
    return program;
}

void doMain(int argc, const char * const argv[])
{
    Arguments arguments = parseArguments(argc, argv);
    Program program = loadProgram(arguments.files);
    if(arguments.dumpDocumentTree)
    {
        PrintingVisitor printer(std::wcout);
        printer.visit(program);
        return;
    }
    Interpreter interpreter(arguments.files, arguments.programArguments, std::wcin, std::wcout, parseFromFile);
    interpreter.visit(program);
}

int main(int argc, char *argv[])
{
    try
    {
        doMain(argc, argv);
        return 0;
    }
    catch(const AppError &e)
    {
        std::cerr << "The interpreter's command line interface encountered an error:\n" << e.what() << "\n";
        return 1;
    }
    catch(const InterpreterPipelineError &e)
    {
        std::cerr << e.what() << "\n";
        return 1;
    }
    catch(const std::exception &e)
    {
        std::cerr << "An unexpected error occured:\n" << e.what() << "\n";
        return 1;
    }
}
