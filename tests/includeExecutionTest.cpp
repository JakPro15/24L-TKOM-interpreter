#include "includeExecution.hpp"

#include "helpers.hpp"
#include "parserExceptions.hpp"

#include <catch2/catch_test_macros.hpp>

using enum Type::Builtin;

TEST_CASE("mergePrograms test", "[includeExecution]")
{
    Program p1({1, 1});
    Program p2({1, 1});

    p2.structs.emplace(
        L"S", StructDeclaration(
                  {1, 1}, L"<test>",
                  {
                      Field({1, 10}, {INT}, L"a"),
                      Field({1, 20}, {INT}, L"b"),
                  }
              )
    );
    p2.variants.emplace(
        L"V", VariantDeclaration(
                  {1, 1}, L"<test>",
                  {
                      Field({1, 10}, {INT}, L"a"),
                      Field({1, 20}, {INT}, L"b"),
                  }
              )
    );

    std::set<std::wstring> printedFunctions;
    printedFunctions.insert(addOverload(p1, FunctionIdentification(L"f", {{INT}, {STR}}), {{INT}}));
    printedFunctions.insert(addOverload(p1, FunctionIdentification(L"f", {{STR}, {INT}}), {}));
    printedFunctions.insert(addOverload(p1, FunctionIdentification(L"f", {{INT}, {FLOAT}, {INT}}), {}));
    printedFunctions.insert(addOverload(p2, FunctionIdentification(L"g", {{INT}, {STR}}), {{INT}}));
    printedFunctions.insert(addOverload(p2, FunctionIdentification(L"g", {{STR}, {INT}}), {}));
    printedFunctions.insert(addOverload(p2, FunctionIdentification(L"g", {{INT}, {FLOAT}, {INT}}), {}));
    mergePrograms(p1, p2);
    checkNodeContainer(p1.functions, printedFunctions);
    checkNodeContainer(
        p1.structs, {L"S: StructDeclaration <line: 1, col: 1> source=<test>\n"
                     L"|-Field <line: 1, col: 10> type=int name=a\n"
                     L"`-Field <line: 1, col: 20> type=int name=b\n"}
    );
    checkNodeContainer(
        p1.variants, {L"V: VariantDeclaration <line: 1, col: 1> source=<test>\n"
                      L"|-Field <line: 1, col: 10> type=int name=a\n"
                      L"`-Field <line: 1, col: 20> type=int name=b\n"}
    );
}

TEST_CASE("mergePrograms name collisions", "[includeExecution]")
{
    Program p1({1, 1});
    Program p2({1, 1});
    p1.structs.emplace(
        L"S", StructDeclaration(
                  {1, 1}, L"<test>",
                  {
                      Field({1, 10}, {INT}, L"a"),
                      Field({1, 20}, {INT}, L"b"),
                  }
              )
    );
    p2.structs.emplace(
        L"S", StructDeclaration(
                  {1, 1}, L"<test>",
                  {
                      Field({1, 10}, {INT}, L"a"),
                      Field({1, 20}, {INT}, L"b"),
                  }
              )
    );
    REQUIRE_THROWS_AS(mergePrograms(p1, p2), DuplicateStructError);

    p1 = Program({1, 1});
    p2 = Program({1, 1});
    p1.variants.emplace(
        L"V", VariantDeclaration(
                  {1, 1}, L"<test>",
                  {
                      Field({1, 10}, {INT}, L"a"),
                      Field({1, 20}, {INT}, L"b"),
                  }
              )
    );
    p2.variants.emplace(
        L"V", VariantDeclaration(
                  {1, 1}, L"<test>",
                  {
                      Field({1, 10}, {INT}, L"a"),
                      Field({1, 20}, {INT}, L"b"),
                  }
              )
    );
    REQUIRE_THROWS_AS(mergePrograms(p1, p2), DuplicateVariantError);

    p1 = Program({1, 1});
    p2 = Program({1, 1});
    addOverload(p1, FunctionIdentification(L"f", {{INT}, {STR}}), {{INT}});
    addOverload(p2, FunctionIdentification(L"f", {{INT}, {STR}}), {{INT}});
    REQUIRE_THROWS_AS(mergePrograms(p1, p2), DuplicateFunctionError);
}

TEST_CASE("includeExecution", "[includeExecution]")
{
    Program firstProgram({1, 1});
    firstProgram.includes.emplace_back(IncludeStatement({1, 1}, L"second.txt"));
    std::wstring printedFunction = addOverload(firstProgram, FunctionIdentification(L"f", {{INT}, {STR}}), {{INT}});
    std::wstringstream filesIncluded;
    auto parseFromFile = [&](const std::wstring &fileName) {
        filesIncluded << fileName << L'\n';
        if(fileName == L"first.txt")
            return std::move(firstProgram);
        else
            return Program({1, 1});
    };

    Program program({1, 1});
    program.includes.emplace_back(IncludeStatement({1, 1}, L"first.txt"));
    program.includes.emplace_back(IncludeStatement({2, 1}, L"third.txt"));
    program.includes.emplace_back(IncludeStatement({3, 1}, L"second.txt"));
    std::vector<std::wstring> sourceFiles = {L"zeroth.txt"};
    executeIncludes(program, sourceFiles, parseFromFile);
    REQUIRE(program.includes.size() == 0);
    REQUIRE(filesIncluded.str() == L"first.txt\nsecond.txt\nthird.txt\n");
    REQUIRE(sourceFiles == std::vector<std::wstring>{L"zeroth.txt", L"first.txt", L"second.txt", L"third.txt"});
    checkNodeContainer(program.functions, {printedFunction});
}
