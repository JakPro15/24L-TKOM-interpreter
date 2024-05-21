#include "printingVisitor.hpp"

#include <catch2/catch_test_macros.hpp>

#include <set>
#include <sstream>

template <typename NodeContainer>
void checkNodeContainer(NodeContainer &container, const std::set<std::wstring> &expected)
{
    std::set<std::wstring> results;
    for(auto &contained: container)
    {
        std::wstringstream result;
        PrintingVisitor printer(result);
        printer.visit(contained);
        results.insert(result.str());
    }
    REQUIRE(results == expected);
}
