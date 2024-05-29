#ifndef ARGUMENTPARSING_HPP
#define ARGUMENTPARSING_HPP

#include <string>
#include <vector>

struct Arguments
{
    std::vector<std::wstring> files;
    bool dumpDocumentTree;
    std::vector<std::wstring> programArguments;
};

Arguments parseArguments(int argc, const char * const argv[]);

#endif
