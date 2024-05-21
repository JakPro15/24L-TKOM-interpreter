#include "semanticAnalysis.hpp"

#include "checkNodeContainer.hpp"
#include "documentTree.hpp"
#include "interpreterExceptions.hpp"

#include <catch2/catch_test_macros.hpp>

using enum Type::Builtin;

TEST_CASE("valid single top level statement Programs", "[doSemanticAnalysis]")
{
    Program program({1, 1});
    doSemanticAnalysis(program);
    program.structs.emplace(L"structure", StructDeclaration({1, 1}, L"<test>", {Field({2, 2}, {INT}, L"b")}));
    doSemanticAnalysis(program);
    program.structs.clear();
    program.variants.emplace(L"wariant", VariantDeclaration({1, 1}, L"<test>", {Field({2, 2}, {INT}, L"b")}));
    doSemanticAnalysis(program);
    program.variants.clear();
    program.functions.emplace(
        FunctionIdentification(L"function", {}), FunctionDeclaration({1, 1}, L"<test>", {}, {}, {})
    );
    doSemanticAnalysis(program);
}

TEST_CASE("include in semantic analysis", "[doSemanticAnalysis]")
{
    Program program({1, 1});
    program.includes.emplace_back(Position{1, 1}, L"file");
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), IncludeInSemanticAnalysisError);
}

TEST_CASE("nonexistent field types", "[doSemanticAnalysis]")
{
    Program program({1, 1});
    program.structs.emplace(L"structure", StructDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"some_type"}, L"b")}));
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), UnknownFieldTypeError);
    program.structs.clear();
    program.variants.emplace(L"wariant", VariantDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"some_type"}, L"b")}));
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), UnknownFieldTypeError);
}

TEST_CASE("duplicate field names", "[doSemanticAnalysis]")
{
    Program program({1, 1});
    program.structs.emplace(
        L"structure", StructDeclaration(
                          {1, 1}, L"<test>",
                          {
                              Field({2, 2}, {INT}, L"b"),
                              Field({3, 2}, {INT}, L"b"),
                          }
                      )
    );
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), FieldNameCollisionError);
    program.structs.clear();
    program.variants.emplace(
        L"wariant", VariantDeclaration(
                        {1, 1}, L"<test>",
                        {
                            Field({2, 2}, {INT}, L"b"),
                            Field({3, 2}, {INT}, L"b"),
                        }
                    )
    );
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), FieldNameCollisionError);
}

TEST_CASE("multiple structures and variants - valid Program", "[doSemanticAnalysis]")
{
    Program program({1, 1});
    program.structs.emplace(
        L"structure", StructDeclaration(
                          {1, 1}, L"<test>",
                          {
                              Field({2, 2}, {INT}, L"b"),
                              Field({3, 2}, {L"wariat"}, L"c"),
                          }
                      )
    );
    program.structs.emplace(L"struktura", StructDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"structure"}, L"b")}));
    program.structs.emplace(L"sint", StructDeclaration({1, 1}, L"<test>", {Field({2, 2}, {INT}, L"b")}));
    program.variants.emplace(
        L"wariant", VariantDeclaration(
                        {1, 1}, L"<test>",
                        {
                            Field({2, 2}, {INT}, L"b"),
                            Field({3, 2}, {STR}, L"c"),
                        }
                    )
    );
    program.variants.emplace(
        L"wariat", VariantDeclaration(
                       {1, 1}, L"<test>",
                       {
                           Field({2, 2}, {L"wariant"}, L"b"),
                           Field({3, 2}, {L"sint"}, L"c"),
                           Field({3, 2}, {FLOAT}, L"wariat"),
                       }
                   )
    );
    doSemanticAnalysis(program);
}

TEST_CASE("recurrent fields", "[doSemanticAnalysis]")
{
    Program program({1, 1}); // direct recursion in structure
    program.structs.emplace(L"struktura", StructDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"struktura"}, L"b")}));
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), FieldTypeRecursionError);

    program.structs.clear(); // indirect recursion in structure
    program.structs.emplace(L"st1", StructDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"st2"}, L"b")}));
    program.structs.emplace(L"st2", StructDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"st1"}, L"b")}));
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), FieldTypeRecursionError);

    program.structs.clear(); // direct recursion in variant
    program.variants.emplace(L"wariant", VariantDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"wariant"}, L"b")}));
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), FieldTypeRecursionError);
    program.variants.clear(); // indirect recursion in variant
    program.variants.emplace(L"w1", VariantDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"w2"}, L"b")}));
    program.variants.emplace(L"w2", VariantDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"w1"}, L"b")}));
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), FieldTypeRecursionError);

    program.variants.clear(); // recursion in both struct and variant, 4 times removed
    program.structs.emplace(L"s1", StructDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"s2"}, L"b")}));
    program.structs.emplace(L"s2", StructDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"w1"}, L"b")}));
    program.variants.emplace(L"w1", VariantDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"w2"}, L"b")}));
    program.variants.emplace(L"w2", VariantDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"s1"}, L"b")}));
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), FieldTypeRecursionError);
}

TEST_CASE("struct, variant, function name collisions", "[doSemanticAnalysis]")
{
    Program program({1, 1}); // struct-variant collision
    program.structs.emplace(L"structure", StructDeclaration({1, 1}, L"<test>", {Field({2, 2}, {INT}, L"b")}));
    program.variants.emplace(L"structure", VariantDeclaration({1, 1}, L"<test>", {Field({2, 2}, {INT}, L"b")}));
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), NameCollisionError);

    program = Program({1, 1}); // struct-function collision
    program.structs.emplace(L"structure", StructDeclaration({1, 1}, L"<test>", {Field({2, 2}, {INT}, L"b")}));
    program.functions.emplace(
        FunctionIdentification(L"structure", {}), FunctionDeclaration({1, 1}, L"<test>", {}, {}, {})
    );
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), NameCollisionError);

    program = Program({1, 1}); // struct-function collision
    program.variants.emplace(L"structure", VariantDeclaration({1, 1}, L"<test>", {Field({2, 2}, {INT}, L"b")}));
    program.functions.emplace(
        FunctionIdentification(L"structure", {}), FunctionDeclaration({1, 1}, L"<test>", {}, {}, {})
    );
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), NameCollisionError);
}

TEST_CASE("valid function parameters", "[doSemanticAnalysis]")
{
    Program program({1, 1});
    program.functions.emplace(
        FunctionIdentification(L"function", {}), FunctionDeclaration(
                                                     {1, 1}, L"<test>",
                                                     {
                                                         VariableDeclaration({2, 1}, {INT}, L"a", false),
                                                         VariableDeclaration({3, 1}, {STR}, L"b", true),
                                                         VariableDeclaration({4, 1}, {BOOL}, L"c", false),
                                                         VariableDeclaration({5, 1}, {INT}, L"d", false),
                                                     },
                                                     {}, {}
                                                 )
    );
    doSemanticAnalysis(program);
}

TEST_CASE("function parameters errors", "[doSemanticAnalysis]")
{
    Program program({1, 1}); // two parameters with same name
    program.functions.emplace(
        FunctionIdentification(L"function", {}), FunctionDeclaration(
                                                     {1, 1}, L"<test>",
                                                     {
                                                         VariableDeclaration({2, 1}, {INT}, L"a", false),
                                                         VariableDeclaration({3, 1}, {STR}, L"a", true),
                                                     },
                                                     {}, {}
                                                 )
    );
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), VariableNameCollisionError);

    program = Program({1, 1}); // parameter with nonexistent type
    program.functions.emplace(
        FunctionIdentification(L"function", {}),
        FunctionDeclaration({1, 1}, L"<test>", {VariableDeclaration({2, 1}, {L"xd"}, L"a", false)}, {}, {})
    );
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), UnknownVariableTypeError);
}

Program wrapInFunction(std::vector<std::unique_ptr<Instruction>> instructions)
{
    Program program({1, 1});
    // Prepare several user-defined types that will be used in wrapped tests
    program.structs.emplace(
        L"strt1", StructDeclaration(
                      {1, 1}, L"<test>",
                      {
                          Field({2, 2}, {INT}, L"a"),
                          Field({3, 2}, {STR}, L"b"),
                          Field({4, 2}, {FLOAT}, L"c"),
                      }
                  )
    );
    program.structs.emplace(
        L"strt2", StructDeclaration(
                      {1, 1}, L"<test>",
                      {
                          Field({2, 2}, {L"strt1"}, L"a"),
                          Field({3, 2}, {L"vart1"}, L"b"),
                      }
                  )
    );
    program.variants.emplace(
        L"vart1", VariantDeclaration(
                      {1, 1}, L"<test>",
                      {
                          Field({2, 2}, {INT}, L"a"),
                          Field({3, 2}, {STR}, L"b"),
                          Field({4, 2}, {FLOAT}, L"c"),
                      }
                  )
    );
    program.variants.emplace(
        L"vart2", VariantDeclaration(
                      {1, 1}, L"<test>",
                      {
                          Field({2, 2}, {L"vart1"}, L"a"),
                          Field({3, 2}, {L"strt1"}, L"b"),
                      }
                  )
    );
    program.functions.emplace(
        FunctionIdentification(L"function", {{INT}}),
        FunctionDeclaration(
            {1, 1}, L"<test>", {VariableDeclaration({2, 1}, {INT}, L"arg", true)}, {}, std::move(instructions)
        )
    );
    return program;
}

std::unique_ptr<Literal> makeLiteral(Position position, std::variant<std::wstring, int32_t, double, bool> value)
{
    return std::make_unique<Literal>(position, value);
}

TEST_CASE("VariableDeclStatement cast insertion", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{3, 1}, VariableDeclaration({3, 1}, {INT}, L"a", false), makeLiteral({3, 10}, 2)
    ));
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{4, 1}, VariableDeclaration({4, 1}, {STR}, L"b", false), makeLiteral({4, 10}, 2)
    ));
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{5, 1}, VariableDeclaration({5, 1}, {L"vart1"}, L"c", false), makeLiteral({5, 10}, 2)
    ));
    Program program = wrapInFunction(std::move(functionBody));
    doSemanticAnalysis(program);
    checkNodeContainer(
        program.functions, {L"function(int): FunctionDeclaration <line: 1, col: 1> source=<test>\n"
                            L"|-Parameters:\n"
                            L"|`-VariableDeclaration <line: 2, col: 1> type=int name=arg mutable=true\n"
                            L"`-Body:\n"
                            L" |-VariableDeclStatement <line: 3, col: 1>\n"
                            L" ||-VariableDeclaration <line: 3, col: 1> type=int name=a mutable=false\n"
                            L" |`-Literal <line: 3, col: 10> type=int value=2\n"
                            L" |-VariableDeclStatement <line: 4, col: 1>\n"
                            L" ||-VariableDeclaration <line: 4, col: 1> type=str name=b mutable=false\n"
                            L" |`-CastExpression <line: 4, col: 10> targetType=str\n"
                            L" | `-Literal <line: 4, col: 10> type=int value=2\n"
                            L" `-VariableDeclStatement <line: 5, col: 1>\n"
                            L"  |-VariableDeclaration <line: 5, col: 1> type=vart1 name=c mutable=false\n"
                            L"  `-CastExpression <line: 5, col: 10> targetType=vart1\n"
                            L"   `-Literal <line: 5, col: 10> type=int value=2\n"}
    );
}

template <typename ErrorType>
void checkSemanticError(std::vector<std::unique_ptr<Instruction>> &functionBody)
{
    Program program = wrapInFunction(std::move(functionBody));
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), ErrorType);
}

TEST_CASE("VariableDeclStatement errors", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{3, 1}, VariableDeclaration({3, 1}, {L"bad"}, L"a", false), makeLiteral({3, 10}, 2)
    ));
    checkSemanticError<UnknownVariableTypeError>(functionBody); // invalid variable type

    functionBody.clear();
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{3, 1}, VariableDeclaration({3, 1}, {INT}, L"a", true), makeLiteral({3, 10}, 2)
    ));
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{4, 1}, VariableDeclaration({4, 1}, {STR}, L"a", false), makeLiteral({4, 10}, 2)
    ));
    checkSemanticError<VariableNameCollisionError>(functionBody); // two variables with same name

    functionBody.clear();
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{4, 1}, VariableDeclaration({4, 1}, {STR}, L"arg", true), makeLiteral({4, 10}, 2)
    ));
    checkSemanticError<VariableNameCollisionError>(functionBody); // variable with same name as parameter

    functionBody.clear();
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{4, 1}, VariableDeclaration({4, 1}, {L"strt1"}, L"a", true), makeLiteral({4, 10}, 2)
    ));
    checkSemanticError<InvalidCastError>(functionBody); // cast from int to struct impossible

    functionBody.clear();
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{4, 1}, VariableDeclaration({4, 1}, {L"vart1"}, L"a", true), makeLiteral({4, 10}, false)
    ));
    checkSemanticError<InvalidCastError>(functionBody); // vart1 does not contain bool field, so cast invalid
}
