#ifndef HELPERS_HPP
#define HELPERS_HPP

#include "documentTree.hpp"
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

std::unique_ptr<Literal> makeLiteral(Position position, std::variant<std::wstring, int32_t, double, bool> value);
std::wstring addOverload(
    Program &program, const FunctionIdentification &id, std::optional<Type> returnType = std::nullopt
);
std::wstring wrapInMain(const std::wstring &source);

#endif
