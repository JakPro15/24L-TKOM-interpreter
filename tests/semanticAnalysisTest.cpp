#include "semanticAnalysis.hpp"

#include "documentTree.hpp"
#include "helpers.hpp"
#include "semanticExceptions.hpp"

#include <catch2/catch_test_macros.hpp>

#include <iostream>

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
        FunctionIdentification(L"function", {}), std::make_unique<FunctionDeclaration>(
                                                     Position{1, 1}, L"<test>", std::vector<VariableDeclaration>{},
                                                     std::nullopt, std::vector<std::unique_ptr<Instruction>>{}
                                                 )
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

TEST_CASE("duplicate fields", "[doSemanticAnalysis]")
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
                            Field({3, 2}, {STR}, L"b"),
                        }
                    )
    );
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), FieldNameCollisionError);
    program.variants.clear();
    program.variants.emplace(
        L"wariant", VariantDeclaration(
                        {1, 1}, L"<test>",
                        {
                            Field({2, 2}, {INT}, L"a"),
                            Field({3, 2}, {INT}, L"b"),
                        }
                    )
    );
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), FieldTypeCollisionError);
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
        FunctionIdentification(L"structure", {}), std::make_unique<FunctionDeclaration>(
                                                      Position{1, 1}, L"<test>", std::vector<VariableDeclaration>{},
                                                      std::nullopt, std::vector<std::unique_ptr<Instruction>>{}
                                                  )
    );
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), NameCollisionError);

    program = Program({1, 1}); // variant-function collision
    program.variants.emplace(L"structure", VariantDeclaration({1, 1}, L"<test>", {Field({2, 2}, {INT}, L"b")}));
    program.functions.emplace(
        FunctionIdentification(L"structure", {}), std::make_unique<FunctionDeclaration>(
                                                      Position{1, 1}, L"<test>", std::vector<VariableDeclaration>{},
                                                      std::nullopt, std::vector<std::unique_ptr<Instruction>>{}
                                                  )
    );
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), NameCollisionError);
}

TEST_CASE("valid function parameters", "[doSemanticAnalysis]")
{
    Program program({1, 1});
    program.functions.emplace(
        FunctionIdentification(L"function", {}), std::make_unique<FunctionDeclaration>(
                                                     Position{1, 1}, L"<test>",
                                                     std::vector<VariableDeclaration>{
                                                         VariableDeclaration({2, 1}, {INT}, L"a", false),
                                                         VariableDeclaration({3, 1}, {STR}, L"b", true),
                                                         VariableDeclaration({4, 1}, {BOOL}, L"c", false),
                                                         VariableDeclaration({5, 1}, {INT}, L"d", false),
                                                     },
                                                     std::nullopt, std::vector<std::unique_ptr<Instruction>>{}
                                                 )
    );
    doSemanticAnalysis(program);
}

TEST_CASE("function parameters errors", "[doSemanticAnalysis]")
{
    Program program({1, 1}); // two parameters with same name
    program.functions.emplace(
        FunctionIdentification(L"function", {}), std::make_unique<FunctionDeclaration>(
                                                     Position{1, 1}, L"<test>",
                                                     std::vector<VariableDeclaration>{
                                                         VariableDeclaration({2, 1}, {INT}, L"a", false),
                                                         VariableDeclaration({3, 1}, {STR}, L"a", true),
                                                     },
                                                     std::nullopt, std::vector<std::unique_ptr<Instruction>>{}
                                                 )
    );
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), VariableNameCollisionError);

    program = Program({1, 1}); // parameter with nonexistent type
    program.functions.emplace(
        FunctionIdentification(L"function", {}),
        std::make_unique<FunctionDeclaration>(
            Position{1, 1}, L"<test>",
            std::vector<VariableDeclaration>{VariableDeclaration({2, 1}, {L"xd"}, L"a", false)}, std::nullopt,
            std::vector<std::unique_ptr<Instruction>>{}
        )
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
        FunctionIdentification(L"function", {{INT}, {L"strt1"}, {L"vart1"}, {L"strt2"}, {L"vart2"}}),
        std::make_unique<FunctionDeclaration>(
            Position{1, 1}, L"<test>",
            std::vector<VariableDeclaration>{
                VariableDeclaration({2, 1}, {INT}, L"arg", true),
                VariableDeclaration({2, 10}, {L"strt1"}, L"s1", true),
                VariableDeclaration({2, 20}, {L"vart1"}, L"v1", true),
                VariableDeclaration({2, 30}, {L"strt2"}, L"s2", true),
                VariableDeclaration({2, 40}, {L"vart2"}, L"v2", true),
            },
            std::nullopt, std::move(instructions)
        )
    );
    return program;
}

const std::wstring wrappedFunctionHeader =
    L"function(int, strt1, vart1, strt2, vart2): FunctionDeclaration <line: 1, col: 1> source=<test>\n"
    L"|-Parameters:\n"
    L"||-VariableDeclaration <line: 2, col: 1> type=int name=arg mutable=true\n"
    L"||-VariableDeclaration <line: 2, col: 10> type=strt1 name=s1 mutable=true\n"
    L"||-VariableDeclaration <line: 2, col: 20> type=vart1 name=v1 mutable=true\n"
    L"||-VariableDeclaration <line: 2, col: 30> type=strt2 name=s2 mutable=true\n"
    L"|`-VariableDeclaration <line: 2, col: 40> type=vart2 name=v2 mutable=true\n";

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
        program.functions,
        {wrappedFunctionHeader + L"`-Body:\n"
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

TEST_CASE("AssignmentStatement cast insertion", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    functionBody.push_back(
        std::make_unique<AssignmentStatement>(Position{3, 1}, Assignable({3, 1}, L"arg"), makeLiteral({3, 10}, 2))
    );
    functionBody.push_back(
        std::make_unique<AssignmentStatement>(Position{4, 1}, Assignable({4, 1}, L"arg"), makeLiteral({4, 10}, L"2"))
    );
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{5, 1},
        Assignable(
            {5, 1},
            std::make_unique<Assignable>(Position{5, 1}, std::make_unique<Assignable>(Position{5, 1}, L"s2"), L"b"),
            L"c"
        ),
        makeLiteral({5, 10}, L"2")
    ));
    Program program = wrapInFunction(std::move(functionBody));
    doSemanticAnalysis(program);
    checkNodeContainer(
        program.functions, {wrappedFunctionHeader + L"`-Body:\n"
                                                    L" |-AssignmentStatement <line: 3, col: 1>\n"
                                                    L" ||-Assignable <line: 3, col: 1> right=arg\n"
                                                    L" |`-Literal <line: 3, col: 10> type=int value=2\n"
                                                    L" |-AssignmentStatement <line: 4, col: 1>\n"
                                                    L" ||-Assignable <line: 4, col: 1> right=arg\n"
                                                    L" |`-CastExpression <line: 4, col: 10> targetType=int\n"
                                                    L" | `-Literal <line: 4, col: 10> type=str value=2\n"
                                                    L" `-AssignmentStatement <line: 5, col: 1>\n"
                                                    L"  |-Assignable <line: 5, col: 1> right=c\n"
                                                    L"  |`-Assignable <line: 5, col: 1> right=b\n"
                                                    L"  | `-Assignable <line: 5, col: 1> right=s2\n"
                                                    L"  `-CastExpression <line: 5, col: 10> targetType=float\n"
                                                    L"   `-Literal <line: 5, col: 10> type=str value=2\n"}
    );
}

TEST_CASE("AssignmentStatement errors", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    functionBody.push_back(
        std::make_unique<AssignmentStatement>(Position{3, 1}, Assignable({3, 1}, L"bad"), makeLiteral({3, 10}, 2))
    );
    checkSemanticError<UnknownVariableError>(functionBody); // nonexistent variable to assign to

    functionBody.clear();
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{3, 1}, Assignable({3, 1}, L"arg"), std::make_unique<Variable>(Position{3, 10}, L"xd")
    ));
    checkSemanticError<UnknownVariableError>(functionBody); // nonexistent variable to read from

    functionBody.clear();
    functionBody.push_back(
        std::make_unique<AssignmentStatement>(Position{3, 1}, Assignable({3, 1}, L"s2"), makeLiteral({3, 10}, 2))
    );
    checkSemanticError<InvalidCastError>(functionBody); // cannot cast to struct

    functionBody.clear();
    functionBody.push_back(
        std::make_unique<AssignmentStatement>(Position{3, 1}, Assignable({3, 1}, L"v2"), makeLiteral({3, 10}, true))
    );
    checkSemanticError<InvalidCastError>(functionBody); // cannot cast to variant as bool is not a field

    functionBody.clear();
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{3, 1}, Assignable({3, 1}, std::make_unique<Assignable>(Position{3, 1}, L"arg"), L"a"),
        makeLiteral({3, 10}, true)
    ));
    checkSemanticError<FieldAccessError>(functionBody); // access to field of builtin type

    functionBody.clear();
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{3, 1}, Assignable({3, 1}, std::make_unique<Assignable>(Position{3, 1}, L"s2"), L"xd"),
        makeLiteral({3, 10}, true)
    ));
    checkSemanticError<FieldAccessError>(functionBody); // access to nonexistent struct field

    functionBody.clear();
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{3, 1}, Assignable({3, 1}, std::make_unique<Assignable>(Position{3, 1}, L"v2"), L"xd"),
        makeLiteral({3, 10}, true)
    ));
    checkSemanticError<FieldAccessError>(functionBody); // access to nonexistent variant field

    functionBody.clear();
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{5, 1},
        Assignable(
            {5, 1},
            std::make_unique<Assignable>(Position{5, 1}, std::make_unique<Assignable>(Position{5, 1}, L"v2"), L"b"),
            L"b"
        ),
        makeLiteral({5, 10}, L"2")
    ));
    checkSemanticError<FieldAccessError>(functionBody); // access to field of variant field

    functionBody.clear();
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{3, 1}, VariableDeclaration({3, 1}, {STR}, L"a", false), makeLiteral(Position{3, 10}, L"2")
    ));
    functionBody.push_back(
        std::make_unique<AssignmentStatement>(Position{4, 1}, Assignable({4, 1}, L"a"), makeLiteral({4, 10}, L"3"))
    );
    checkSemanticError<ImmutableError>(functionBody); // modification of immutable variable
}

Program wrapExpression(std::unique_ptr<Expression> &expression)
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{3, 1}, VariableDeclaration({3, 1}, {STR}, L"a", true), std::move(expression)
    ));
    return wrapInFunction(std::move(functionBody));
}

template <typename BinaryExpression>
void pushBackBinaryExpr(
    std::vector<std::unique_ptr<Instruction>> &instructions, unsigned int line, const std::wstring &assignmentTarget,
    std::variant<std::wstring, int32_t, double, bool> leftLiteral,
    std::variant<std::wstring, int32_t, double, bool> rightLiteral
)
{
    instructions.push_back(std::make_unique<AssignmentStatement>(
        Position{line, 1}, Assignable({line, 1}, assignmentTarget),
        std::make_unique<BinaryExpression>(
            Position{line, 10}, makeLiteral({line, 10}, leftLiteral), makeLiteral({line, 20}, rightLiteral)
        )
    ));
}

std::wstring binaryExprNoCasts(
    const std::wstring &assignmentTarget, const std::wstring &type, unsigned int line, Type leftSourceType,
    Type rightSourceType
)
{
    return std::format(
        L" |-AssignmentStatement <line: {}, col: 1>\n"
        L" ||-Assignable <line: {}, col: 1> right={}\n"
        L" |`-{} <line: {}, col: 10>\n"
        L" | |-Literal <line: {}, col: 10> type={} value=2\n"
        L" | `-Literal <line: {}, col: 20> type={} value=2\n",
        line, line, assignmentTarget, type, line, line, leftSourceType, line, rightSourceType
    );
}

std::wstring binaryExprCasts(
    const std::wstring &assignmentTarget, const std::wstring &type, unsigned int line, Type leftTargetType,
    Type rightTargetType, Type leftSourceType, Type rightSourceType
)
{
    return std::format(
        L" |-AssignmentStatement <line: {}, col: 1>\n"
        L" ||-Assignable <line: {}, col: 1> right={}\n"
        L" |`-{} <line: {}, col: 10>\n"
        L" | |-CastExpression <line: {}, col: 10> targetType={}\n"
        L" | |`-Literal <line: {}, col: 10> type={} value=2\n"
        L" | `-CastExpression <line: {}, col: 20> targetType={}\n"
        L" |  `-Literal <line: {}, col: 20> type={} value=2\n",
        line, line, assignmentTarget, type, line, line, leftTargetType, line, leftSourceType, line, rightTargetType,
        line, rightSourceType
    );
}

TEST_CASE("boolean Expressions cast insertions", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{3, 1}, VariableDeclaration({3, 1}, {BOOL}, L"a", true),
        std::make_unique<Variable>(Position{3, 10}, L"arg")
    ));
    pushBackBinaryExpr<OrExpression>(functionBody, 4, L"a", 2, L"2");
    pushBackBinaryExpr<XorExpression>(functionBody, 5, L"a", 2, L"2");
    pushBackBinaryExpr<AndExpression>(functionBody, 6, L"a", 2, L"2");
    functionBody.push_back(std::make_unique<ReturnStatement>(Position{7, 1}, nullptr));

    Program program = wrapInFunction(std::move(functionBody));
    doSemanticAnalysis(program);
    checkNodeContainer(
        program.functions, {wrappedFunctionHeader +
                            L"`-Body:\n"
                            L" |-VariableDeclStatement <line: 3, col: 1>\n"
                            L" ||-VariableDeclaration <line: 3, col: 1> type=bool name=a mutable=true\n"
                            L" |`-CastExpression <line: 3, col: 10> targetType=bool\n"
                            L" | `-Variable <line: 3, col: 10> name=arg\n" +
                            binaryExprCasts(L"a", L"OrExpression", 4, Type{BOOL}, Type{BOOL}, Type{INT}, Type{STR}) +
                            binaryExprCasts(L"a", L"XorExpression", 5, Type{BOOL}, Type{BOOL}, Type{INT}, Type{STR}) +
                            binaryExprCasts(L"a", L"AndExpression", 6, Type{BOOL}, Type{BOOL}, Type{INT}, Type{STR}) +
                            L" `-ReturnStatement <line: 7, col: 1>\n"}
    );
}

TEST_CASE("equality Expressions - no cast for same types", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{3, 1}, VariableDeclaration({3, 1}, {BOOL}, L"a", true),
        std::make_unique<Variable>(Position{3, 10}, L"arg")
    ));
    pushBackBinaryExpr<EqualExpression>(functionBody, 4, L"a", 2, 2);
    pushBackBinaryExpr<NotEqualExpression>(functionBody, 5, L"a", 2, 2);
    pushBackBinaryExpr<IdenticalExpression>(functionBody, 6, L"a", 2, 2);
    pushBackBinaryExpr<NotIdenticalExpression>(functionBody, 7, L"a", 2, 2);
    pushBackBinaryExpr<EqualExpression>(functionBody, 8, L"a", L"2", L"2");
    pushBackBinaryExpr<NotEqualExpression>(functionBody, 9, L"a", L"2", L"2");
    pushBackBinaryExpr<IdenticalExpression>(functionBody, 10, L"a", L"2", L"2");
    pushBackBinaryExpr<NotIdenticalExpression>(functionBody, 11, L"a", L"2", L"2");
    functionBody.push_back(std::make_unique<ReturnStatement>(Position{12, 1}, nullptr));

    Program program = wrapInFunction(std::move(functionBody));
    doSemanticAnalysis(program);
    checkNodeContainer(
        program.functions, {wrappedFunctionHeader +
                            L"`-Body:\n"
                            L" |-VariableDeclStatement <line: 3, col: 1>\n"
                            L" ||-VariableDeclaration <line: 3, col: 1> type=bool name=a mutable=true\n"
                            L" |`-CastExpression <line: 3, col: 10> targetType=bool\n"
                            L" | `-Variable <line: 3, col: 10> name=arg\n" +
                            binaryExprNoCasts(L"a", L"EqualExpression", 4, Type{INT}, Type{INT}) +
                            binaryExprNoCasts(L"a", L"NotEqualExpression", 5, Type{INT}, Type{INT}) +
                            binaryExprNoCasts(L"a", L"IdenticalExpression", 6, Type{INT}, Type{INT}) +
                            binaryExprNoCasts(L"a", L"NotIdenticalExpression", 7, Type{INT}, Type{INT}) +
                            binaryExprNoCasts(L"a", L"EqualExpression", 8, Type{STR}, Type{STR}) +
                            binaryExprNoCasts(L"a", L"NotEqualExpression", 9, Type{STR}, Type{STR}) +
                            binaryExprNoCasts(L"a", L"IdenticalExpression", 10, Type{STR}, Type{STR}) +
                            binaryExprNoCasts(L"a", L"NotIdenticalExpression", 11, Type{STR}, Type{STR}) +
                            L" `-ReturnStatement <line: 12, col: 1>\n"}
    );
}

TEST_CASE("equality Expressions - builtin types casts", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{3, 1}, VariableDeclaration({3, 1}, {BOOL}, L"a", true),
        std::make_unique<Variable>(Position{3, 10}, L"arg")
    ));
    pushBackBinaryExpr<EqualExpression>(functionBody, 4, L"a", 2, L"2");
    pushBackBinaryExpr<NotEqualExpression>(functionBody, 5, L"a", 2.0, 2);
    pushBackBinaryExpr<IdenticalExpression>(functionBody, 6, L"a", L"2", 2);
    pushBackBinaryExpr<NotIdenticalExpression>(functionBody, 7, L"a", 2, 2.0);
    pushBackBinaryExpr<EqualExpression>(functionBody, 8, L"a", true, L"2");
    pushBackBinaryExpr<NotEqualExpression>(functionBody, 9, L"a", 2, false);
    pushBackBinaryExpr<IdenticalExpression>(functionBody, 10, L"a", 2.0, L"2");
    pushBackBinaryExpr<NotIdenticalExpression>(functionBody, 11, L"a", 2, 2.0);
    functionBody.push_back(std::make_unique<ReturnStatement>(Position{12, 1}, nullptr));

    Program program = wrapInFunction(std::move(functionBody));
    doSemanticAnalysis(program);
    checkNodeContainer(
        program.functions, {wrappedFunctionHeader +
                            L"`-Body:\n"
                            L" |-VariableDeclStatement <line: 3, col: 1>\n"
                            L" ||-VariableDeclaration <line: 3, col: 1> type=bool name=a mutable=true\n"
                            L" |`-CastExpression <line: 3, col: 10> targetType=bool\n"
                            L" | `-Variable <line: 3, col: 10> name=arg\n"
                            L" |-AssignmentStatement <line: 4, col: 1>\n"
                            L" ||-Assignable <line: 4, col: 1> right=a\n"
                            L" |`-EqualExpression <line: 4, col: 10>\n"
                            L" | |-CastExpression <line: 4, col: 10> targetType=str\n"
                            L" | |`-Literal <line: 4, col: 10> type=int value=2\n"
                            L" | `-Literal <line: 4, col: 20> type=str value=2\n"
                            L" |-AssignmentStatement <line: 5, col: 1>\n"
                            L" ||-Assignable <line: 5, col: 1> right=a\n"
                            L" |`-NotEqualExpression <line: 5, col: 10>\n"
                            L" | |-Literal <line: 5, col: 10> type=float value=2\n"
                            L" | `-CastExpression <line: 5, col: 20> targetType=float\n"
                            L" |  `-Literal <line: 5, col: 20> type=int value=2\n" +
                            binaryExprNoCasts(L"a", L"IdenticalExpression", 6, Type{STR}, Type{INT}) +
                            binaryExprNoCasts(L"a", L"NotIdenticalExpression", 7, Type{INT}, Type{FLOAT}) +
                            L" |-AssignmentStatement <line: 8, col: 1>\n"
                            L" ||-Assignable <line: 8, col: 1> right=a\n"
                            L" |`-EqualExpression <line: 8, col: 10>\n"
                            L" | |-CastExpression <line: 8, col: 10> targetType=str\n"
                            L" | |`-Literal <line: 8, col: 10> type=bool value=true\n"
                            L" | `-Literal <line: 8, col: 20> type=str value=2\n"
                            L" |-AssignmentStatement <line: 9, col: 1>\n"
                            L" ||-Assignable <line: 9, col: 1> right=a\n"
                            L" |`-NotEqualExpression <line: 9, col: 10>\n"
                            L" | |-Literal <line: 9, col: 10> type=int value=2\n"
                            L" | `-CastExpression <line: 9, col: 20> targetType=int\n"
                            L" |  `-Literal <line: 9, col: 20> type=bool value=false\n" +
                            binaryExprNoCasts(L"a", L"IdenticalExpression", 10, Type{FLOAT}, Type{STR}) +
                            binaryExprNoCasts(L"a", L"NotIdenticalExpression", 11, Type{INT}, Type{FLOAT}) +
                            L" `-ReturnStatement <line: 12, col: 1>\n"}
    );
}

std::vector<std::unique_ptr<Expression>> prepareStrt1Arguments(unsigned line, unsigned column)
{
    std::vector<std::unique_ptr<Expression>> structArguments;
    structArguments.push_back(makeLiteral(Position{line, column + 1}, 2));
    structArguments.push_back(makeLiteral(Position{line, column + 3}, L"2"));
    structArguments.push_back(makeLiteral(Position{line, column + 7}, 2.0));
    return structArguments;
}

TEST_CASE("equality Expressions - struct init list casts", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{3, 1}, VariableDeclaration({3, 1}, {BOOL}, L"a", true),
        std::make_unique<Variable>(Position{3, 10}, L"arg")
    ));
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{4, 1}, Assignable({4, 1}, L"a"),
        std::make_unique<EqualExpression>(
            Position{4, 10}, std::make_unique<Variable>(Position{4, 10}, L"s1"),
            std::make_unique<StructExpression>(Position{4, 20}, prepareStrt1Arguments(4, 20))
        )
    ));
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{5, 1}, Assignable({5, 1}, L"a"),
        std::make_unique<NotEqualExpression>(
            Position{5, 10}, std::make_unique<StructExpression>(Position{5, 10}, prepareStrt1Arguments(5, 10)),
            std::make_unique<Variable>(Position{5, 20}, L"s1")
        )
    ));
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{6, 1}, Assignable({6, 1}, L"a"),
        std::make_unique<IdenticalExpression>(
            Position{6, 10}, std::make_unique<Variable>(Position{6, 10}, L"s1"),
            std::make_unique<StructExpression>(Position{6, 20}, prepareStrt1Arguments(6, 20))
        )
    ));
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{7, 1}, Assignable({7, 1}, L"a"),
        std::make_unique<NotIdenticalExpression>(
            Position{7, 10}, std::make_unique<StructExpression>(Position{7, 10}, prepareStrt1Arguments(7, 10)),
            std::make_unique<Variable>(Position{7, 20}, L"s1")
        )
    ));
    functionBody.push_back(std::make_unique<ReturnStatement>(Position{8, 1}, nullptr));

    Program program = wrapInFunction(std::move(functionBody));
    doSemanticAnalysis(program);
    checkNodeContainer(
        program.functions,
        {wrappedFunctionHeader + L"`-Body:\n"
                                 L" |-VariableDeclStatement <line: 3, col: 1>\n"
                                 L" ||-VariableDeclaration <line: 3, col: 1> type=bool name=a mutable=true\n"
                                 L" |`-CastExpression <line: 3, col: 10> targetType=bool\n"
                                 L" | `-Variable <line: 3, col: 10> name=arg\n"
                                 L" |-AssignmentStatement <line: 4, col: 1>\n"
                                 L" ||-Assignable <line: 4, col: 1> right=a\n"
                                 L" |`-EqualExpression <line: 4, col: 10>\n"
                                 L" | |-Variable <line: 4, col: 10> name=s1\n"
                                 L" | `-StructExpression <line: 4, col: 20> structType=strt1\n"
                                 L" |  |-Literal <line: 4, col: 21> type=int value=2\n"
                                 L" |  |-Literal <line: 4, col: 23> type=str value=2\n"
                                 L" |  `-Literal <line: 4, col: 27> type=float value=2\n"
                                 L" |-AssignmentStatement <line: 5, col: 1>\n"
                                 L" ||-Assignable <line: 5, col: 1> right=a\n"
                                 L" |`-NotEqualExpression <line: 5, col: 10>\n"
                                 L" | |-StructExpression <line: 5, col: 10> structType=strt1\n"
                                 L" | ||-Literal <line: 5, col: 11> type=int value=2\n"
                                 L" | ||-Literal <line: 5, col: 13> type=str value=2\n"
                                 L" | |`-Literal <line: 5, col: 17> type=float value=2\n"
                                 L" | `-Variable <line: 5, col: 20> name=s1\n"
                                 L" |-AssignmentStatement <line: 6, col: 1>\n"
                                 L" ||-Assignable <line: 6, col: 1> right=a\n"
                                 L" |`-IdenticalExpression <line: 6, col: 10>\n"
                                 L" | |-Variable <line: 6, col: 10> name=s1\n"
                                 L" | `-StructExpression <line: 6, col: 20> structType=strt1\n"
                                 L" |  |-Literal <line: 6, col: 21> type=int value=2\n"
                                 L" |  |-Literal <line: 6, col: 23> type=str value=2\n"
                                 L" |  `-Literal <line: 6, col: 27> type=float value=2\n"
                                 L" |-AssignmentStatement <line: 7, col: 1>\n"
                                 L" ||-Assignable <line: 7, col: 1> right=a\n"
                                 L" |`-NotIdenticalExpression <line: 7, col: 10>\n"
                                 L" | |-StructExpression <line: 7, col: 10> structType=strt1\n"
                                 L" | ||-Literal <line: 7, col: 11> type=int value=2\n"
                                 L" | ||-Literal <line: 7, col: 13> type=str value=2\n"
                                 L" | |`-Literal <line: 7, col: 17> type=float value=2\n"
                                 L" | `-Variable <line: 7, col: 20> name=s1\n"
                                 L" `-ReturnStatement <line: 8, col: 1>\n"}
    );
}

template <typename EqualityExpression>
void checkTypeEqualityExpressionErrors()
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{4, 1}, Assignable({4, 1}, L"arg"),
        std::make_unique<EqualExpression>(
            Position{4, 10}, std::make_unique<Variable>(Position{4, 10}, L"s2"),
            std::make_unique<StructExpression>(Position{4, 20}, prepareStrt1Arguments(4, 20))
        )
    ));
    checkSemanticError<InvalidCastError>(functionBody); // init list to struct conversion not possible

    functionBody.clear();
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{4, 1}, Assignable({4, 1}, L"arg"),
        std::make_unique<EqualExpression>(
            Position{4, 10}, std::make_unique<Variable>(Position{4, 10}, L"v1"),
            std::make_unique<StructExpression>(Position{4, 20}, prepareStrt1Arguments(4, 20))
        )
    ));
    checkSemanticError<InvalidCastError>(functionBody); // init list to variant conversion not possible
}

template <typename EqualityExpression>
void checkEqualityExpressionErrors()
{
    checkTypeEqualityExpressionErrors<EqualityExpression>();
    std::vector<std::unique_ptr<Instruction>> functionBody;
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{4, 1}, Assignable({4, 1}, L"arg"),
        std::make_unique<EqualityExpression>(
            Position{4, 10}, std::make_unique<Variable>(Position{4, 10}, L"s2"),
            std::make_unique<Variable>(Position{4, 10}, L"s1")
        )
    ));
    checkSemanticError<InvalidOperatorArgsError>(functionBody); // cannot compare two different struct types
}

TEST_CASE("equality Expressions errors", "[doSemanticAnalysis]")
{
    checkEqualityExpressionErrors<EqualExpression>();
    checkEqualityExpressionErrors<NotEqualExpression>();
    checkTypeEqualityExpressionErrors<IdenticalExpression>();
    checkTypeEqualityExpressionErrors<NotIdenticalExpression>();
}

TEST_CASE("comparison Expressions cast insertions", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{3, 1}, VariableDeclaration({3, 1}, {BOOL}, L"a", true),
        std::make_unique<Variable>(Position{3, 10}, L"arg")
    ));
    pushBackBinaryExpr<GreaterExpression>(functionBody, 4, L"a", 2, 2);
    pushBackBinaryExpr<LesserExpression>(functionBody, 5, L"a", 2, 2);
    pushBackBinaryExpr<GreaterEqualExpression>(functionBody, 6, L"a", 2, 2);
    pushBackBinaryExpr<LesserEqualExpression>(functionBody, 7, L"a", 2, 2);
    pushBackBinaryExpr<GreaterExpression>(functionBody, 8, L"a", 2, L"2");
    pushBackBinaryExpr<LesserExpression>(functionBody, 9, L"a", 2, L"2");
    pushBackBinaryExpr<GreaterEqualExpression>(functionBody, 10, L"a", 2, L"2");
    pushBackBinaryExpr<LesserEqualExpression>(functionBody, 11, L"a", 2, L"2");
    functionBody.push_back(std::make_unique<ReturnStatement>(Position{12, 1}, nullptr));

    Program program = wrapInFunction(std::move(functionBody));
    doSemanticAnalysis(program);
    checkNodeContainer(
        program.functions,
        {wrappedFunctionHeader +
         L"`-Body:\n"
         L" |-VariableDeclStatement <line: 3, col: 1>\n"
         L" ||-VariableDeclaration <line: 3, col: 1> type=bool name=a mutable=true\n"
         L" |`-CastExpression <line: 3, col: 10> targetType=bool\n"
         L" | `-Variable <line: 3, col: 10> name=arg\n" +
         binaryExprNoCasts(L"a", L"GreaterExpression", 4, Type{INT}, Type{INT}) +
         binaryExprNoCasts(L"a", L"LesserExpression", 5, Type{INT}, Type{INT}) +
         binaryExprNoCasts(L"a", L"GreaterEqualExpression", 6, Type{INT}, Type{INT}) +
         binaryExprNoCasts(L"a", L"LesserEqualExpression", 7, Type{INT}, Type{INT}) +
         binaryExprCasts(L"a", L"GreaterExpression", 8, Type{FLOAT}, Type{FLOAT}, Type{INT}, Type{STR}) +
         binaryExprCasts(L"a", L"LesserExpression", 9, Type{FLOAT}, Type{FLOAT}, Type{INT}, Type{STR}) +
         binaryExprCasts(L"a", L"GreaterEqualExpression", 10, Type{FLOAT}, Type{FLOAT}, Type{INT}, Type{STR}) +
         binaryExprCasts(L"a", L"LesserEqualExpression", 11, Type{FLOAT}, Type{FLOAT}, Type{INT}, Type{STR}) +
         L" `-ReturnStatement <line: 12, col: 1>\n"}
    );
}

TEST_CASE("binary arithmetic Expressions cast insertions", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{3, 1}, VariableDeclaration({3, 1}, {FLOAT}, L"a", true),
        std::make_unique<Variable>(Position{3, 10}, L"arg")
    ));
    pushBackBinaryExpr<PlusExpression>(functionBody, 4, L"arg", 2, 2);
    pushBackBinaryExpr<MinusExpression>(functionBody, 5, L"a", 2, L"2");
    pushBackBinaryExpr<MultiplyExpression>(functionBody, 6, L"a", 2.0, true);
    pushBackBinaryExpr<DivideExpression>(functionBody, 7, L"a", 2, 2);
    pushBackBinaryExpr<FloorDivideExpression>(functionBody, 8, L"arg", 2.0, L"2");
    pushBackBinaryExpr<ModuloExpression>(functionBody, 9, L"arg", L"2", 2.0);
    pushBackBinaryExpr<ExponentExpression>(functionBody, 10, L"a", 2.0, 2.0);
    functionBody.push_back(std::make_unique<ReturnStatement>(Position{11, 1}, nullptr));

    Program program = wrapInFunction(std::move(functionBody));
    doSemanticAnalysis(program);
    checkNodeContainer(
        program.functions,
        {wrappedFunctionHeader +
         L"`-Body:\n"
         L" |-VariableDeclStatement <line: 3, col: 1>\n"
         L" ||-VariableDeclaration <line: 3, col: 1> type=float name=a mutable=true\n"
         L" |`-CastExpression <line: 3, col: 10> targetType=float\n"
         L" | `-Variable <line: 3, col: 10> name=arg\n" +
         binaryExprNoCasts(L"arg", L"PlusExpression", 4, Type{INT}, Type{INT}) +
         binaryExprCasts(L"a", L"MinusExpression", 5, Type{FLOAT}, Type{FLOAT}, Type{INT}, Type{STR}) +
         L" |-AssignmentStatement <line: 6, col: 1>\n"
         L" ||-Assignable <line: 6, col: 1> right=a\n"
         L" |`-MultiplyExpression <line: 6, col: 10>\n"
         L" | |-Literal <line: 6, col: 10> type=float value=2\n"
         L" | `-CastExpression <line: 6, col: 20> targetType=float\n"
         L" |  `-Literal <line: 6, col: 20> type=bool value=true\n" +
         binaryExprCasts(L"a", L"DivideExpression", 7, Type{FLOAT}, Type{FLOAT}, Type{INT}, Type{INT}) +
         binaryExprCasts(L"arg", L"FloorDivideExpression", 8, Type{INT}, Type{INT}, Type{FLOAT}, Type{STR}) +
         binaryExprCasts(L"arg", L"ModuloExpression", 9, Type{INT}, Type{INT}, Type{STR}, Type{FLOAT}) +
         binaryExprNoCasts(L"a", L"ExponentExpression", 10, Type{FLOAT}, Type{FLOAT}) +
         L" `-ReturnStatement <line: 11, col: 1>\n"}
    );
}

TEST_CASE("binary string Expressions cast insertions", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{3, 1}, VariableDeclaration({3, 1}, {STR}, L"a", true),
        std::make_unique<Variable>(Position{3, 10}, L"arg")
    ));
    pushBackBinaryExpr<ConcatExpression>(functionBody, 4, L"a", 2, 2);
    pushBackBinaryExpr<StringMultiplyExpression>(functionBody, 5, L"a", 2.0, 2.0);
    pushBackBinaryExpr<SubscriptExpression>(functionBody, 6, L"a", 2.0, L"2");
    functionBody.push_back(std::make_unique<ReturnStatement>(Position{7, 1}, nullptr));

    Program program = wrapInFunction(std::move(functionBody));
    doSemanticAnalysis(program);
    checkNodeContainer(
        program.functions,
        {wrappedFunctionHeader +
         L"`-Body:\n"
         L" |-VariableDeclStatement <line: 3, col: 1>\n"
         L" ||-VariableDeclaration <line: 3, col: 1> type=str name=a mutable=true\n"
         L" |`-CastExpression <line: 3, col: 10> targetType=str\n"
         L" | `-Variable <line: 3, col: 10> name=arg\n" +
         binaryExprCasts(L"a", L"ConcatExpression", 4, Type{STR}, Type{STR}, Type{INT}, Type{INT}) +
         binaryExprCasts(L"a", L"StringMultiplyExpression", 5, Type{STR}, Type{INT}, Type{FLOAT}, Type{FLOAT}) +
         binaryExprCasts(L"a", L"SubscriptExpression", 6, Type{STR}, Type{INT}, Type{FLOAT}, Type{STR}) +
         L" `-ReturnStatement <line: 7, col: 1>\n"}
    );
}

TEST_CASE("unary Expressions cast insertions", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{3, 1}, VariableDeclaration({3, 1}, {FLOAT}, L"a", true), makeLiteral(Position{3, 10}, 2.0)
    ));
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{4, 1}, VariableDeclaration({4, 1}, {BOOL}, L"b", true), makeLiteral(Position{4, 10}, false)
    ));
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{5, 1}, Assignable({5, 1}, L"arg"),
        std::make_unique<UnaryMinusExpression>(Position{5, 10}, makeLiteral(Position{5, 11}, 2))
    ));
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{6, 1}, Assignable({6, 1}, L"a"),
        std::make_unique<UnaryMinusExpression>(Position{6, 10}, makeLiteral(Position{6, 11}, L"2"))
    ));
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{7, 1}, Assignable({7, 1}, L"b"),
        std::make_unique<NotExpression>(Position{7, 10}, makeLiteral(Position{7, 11}, L"2"))
    ));

    Program program = wrapInFunction(std::move(functionBody));
    doSemanticAnalysis(program);
    checkNodeContainer(
        program.functions,
        {wrappedFunctionHeader + L"`-Body:\n"
                                 L" |-VariableDeclStatement <line: 3, col: 1>\n"
                                 L" ||-VariableDeclaration <line: 3, col: 1> type=float name=a mutable=true\n"
                                 L" |`-Literal <line: 3, col: 10> type=float value=2\n"
                                 L" |-VariableDeclStatement <line: 4, col: 1>\n"
                                 L" ||-VariableDeclaration <line: 4, col: 1> type=bool name=b mutable=true\n"
                                 L" |`-Literal <line: 4, col: 10> type=bool value=false\n"
                                 L" |-AssignmentStatement <line: 5, col: 1>\n"
                                 L" ||-Assignable <line: 5, col: 1> right=arg\n"
                                 L" |`-UnaryMinusExpression <line: 5, col: 10>\n"
                                 L" | `-Literal <line: 5, col: 11> type=int value=2\n"
                                 L" |-AssignmentStatement <line: 6, col: 1>\n"
                                 L" ||-Assignable <line: 6, col: 1> right=a\n"
                                 L" |`-UnaryMinusExpression <line: 6, col: 10>\n"
                                 L" | `-CastExpression <line: 6, col: 11> targetType=float\n"
                                 L" |  `-Literal <line: 6, col: 11> type=str value=2\n"
                                 L" `-AssignmentStatement <line: 7, col: 1>\n"
                                 L"  |-Assignable <line: 7, col: 1> right=b\n"
                                 L"  `-NotExpression <line: 7, col: 10>\n"
                                 L"   `-CastExpression <line: 7, col: 11> targetType=bool\n"
                                 L"    `-Literal <line: 7, col: 11> type=str value=2\n"}
    );
}

TEST_CASE("DotExpression valid access", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{3, 1}, Assignable({3, 1}, L"arg"),
        std::make_unique<DotExpression>(
            Position{3, 10},
            std::make_unique<DotExpression>(Position{3, 10}, std::make_unique<Variable>(Position{3, 10}, L"s2"), L"a"),
            L"a"
        )
    ));
    std::vector<SingleIfCase> cases;
    cases.push_back(SingleIfCase(
        {4, 1},
        VariableDeclStatement(
            {4, 10}, VariableDeclaration({4, 10}, {INT}, L"a", false),
            std::make_unique<DotExpression>(
                Position{4, 10},
                std::make_unique<DotExpression>(
                    Position{4, 10}, std::make_unique<Variable>(Position{4, 10}, L"s2"), L"b"
                ),
                L"a"
            )
        ),
        {}
    ));
    functionBody.push_back(
        std::make_unique<IfStatement>(Position{4, 1}, std::move(cases), std::vector<std::unique_ptr<Instruction>>{})
    );

    Program program = wrapInFunction(std::move(functionBody));
    doSemanticAnalysis(program);
    checkNodeContainer(
        program.functions,
        {wrappedFunctionHeader + L"`-Body:\n"
                                 L" |-AssignmentStatement <line: 3, col: 1>\n"
                                 L" ||-Assignable <line: 3, col: 1> right=arg\n"
                                 L" |`-DotExpression <line: 3, col: 10> field=a\n"
                                 L" | `-DotExpression <line: 3, col: 10> field=a\n"
                                 L" |  `-Variable <line: 3, col: 10> name=s2\n"
                                 L" `-IfStatement <line: 4, col: 1>\n"
                                 L"  `-SingleIfCase <line: 4, col: 1>\n"
                                 L"   `-VariableDeclStatement <line: 4, col: 10>\n"
                                 L"    |-VariableDeclaration <line: 4, col: 10> type=int name=a mutable=false\n"
                                 L"    `-DotExpression <line: 4, col: 10> field=b\n"
                                 L"     `-Variable <line: 4, col: 10> name=s2\n"}
    );
}

TEST_CASE("DotExpression errors", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{3, 1}, Assignable({3, 1}, L"arg"),
        std::make_unique<DotExpression>(Position{3, 10}, std::make_unique<Variable>(Position{3, 10}, L"s1"), L"d")
    ));
    checkSemanticError<FieldAccessError>(functionBody); // access to nonexistent field

    functionBody.clear();
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{3, 1}, Assignable({3, 1}, L"arg"),
        std::make_unique<DotExpression>(Position{3, 10}, std::make_unique<Variable>(Position{3, 10}, L"v1"), L"a")
    ));
    checkSemanticError<FieldAccessError>(functionBody); // variant field access not permitted here

    functionBody.clear();
    std::vector<std::unique_ptr<Expression>> structArguments;
    structArguments.push_back(makeLiteral({3, 11}, 2));
    structArguments.push_back(makeLiteral({3, 13}, L"2"));
    structArguments.push_back(makeLiteral({3, 17}, 2.0));
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{3, 1}, Assignable({3, 1}, L"arg"),
        std::make_unique<DotExpression>(
            Position{3, 10}, std::make_unique<StructExpression>(Position{3, 10}, std::move(structArguments)), L"a"
        )
    ));
    checkSemanticError<InvalidInitListError>(functionBody); // access to field of raw initialization list

    functionBody.clear();
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{3, 1}, Assignable({3, 1}, L"arg"),
        std::make_unique<DotExpression>(Position{3, 10}, std::make_unique<Variable>(Position{3, 10}, L"arg"), L"a")
    ));
    checkSemanticError<FieldAccessError>(functionBody); // access to field of simple type

    functionBody.clear();
    std::vector<SingleIfCase> cases;
    cases.push_back(SingleIfCase(
        {3, 1},
        VariableDeclStatement(
            {3, 10}, VariableDeclaration({3, 10}, {INT}, L"a", false),
            std::make_unique<DotExpression>(
                Position{3, 10},
                std::make_unique<DotExpression>(
                    Position{3, 10}, std::make_unique<Variable>(Position{3, 10}, L"v2"), L"b"
                ),
                L"a"
            )
        ),
        {}
    ));
    functionBody.push_back(
        std::make_unique<IfStatement>(Position{4, 1}, std::move(cases), std::vector<std::unique_ptr<Instruction>>{})
    );
    checkSemanticError<FieldAccessError>(functionBody); // access to field of accessed field of variant type
}

TEST_CASE("IsExpression", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{3, 1}, VariableDeclaration({3, 1}, {BOOL}, L"a", true),
        std::make_unique<IsExpression>(Position{3, 10}, std::make_unique<Variable>(Position{3, 10}, L"s2"), Type{INT})
    ));

    Program program = wrapInFunction(std::move(functionBody));
    doSemanticAnalysis(program);
    checkNodeContainer(
        program.functions,
        {wrappedFunctionHeader + L"`-Body:\n"
                                 L" `-VariableDeclStatement <line: 3, col: 1>\n"
                                 L"  |-VariableDeclaration <line: 3, col: 1> type=bool name=a mutable=true\n"
                                 L"  `-IsExpression <line: 3, col: 10> right=int\n"
                                 L"   `-Variable <line: 3, col: 10> name=s2\n"}
    );
}

TEST_CASE("IsExpression error", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    std::vector<std::unique_ptr<Expression>> structArguments;
    structArguments.push_back(makeLiteral({3, 11}, 2));
    structArguments.push_back(makeLiteral({3, 13}, L"2"));
    structArguments.push_back(makeLiteral({3, 17}, 2.0));
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{3, 1}, VariableDeclaration({3, 1}, {BOOL}, L"a", true),
        std::make_unique<IsExpression>(
            Position{3, 10}, std::make_unique<StructExpression>(Position{3, 10}, std::move(structArguments)), Type{INT}
        )
    ));
    checkSemanticError<InvalidInitListError>(functionBody); // cannot use is operator on initialization list
}

TEST_CASE("explicit CastExpression", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{3, 1}, VariableDeclaration({3, 1}, {BOOL}, L"a", true),
        std::make_unique<CastExpression>(
            Position{3, 10}, std::make_unique<Variable>(Position{3, 10}, L"arg"), Type{BOOL}
        )
    ));

    Program program = wrapInFunction(std::move(functionBody));
    doSemanticAnalysis(program);
    checkNodeContainer(
        program.functions,
        {wrappedFunctionHeader + L"`-Body:\n"
                                 L" `-VariableDeclStatement <line: 3, col: 1>\n"
                                 L"  |-VariableDeclaration <line: 3, col: 1> type=bool name=a mutable=true\n"
                                 L"  `-CastExpression <line: 3, col: 10> targetType=bool\n"
                                 L"   `-Variable <line: 3, col: 10> name=arg\n"}
    );
}

TEST_CASE("CastExpression error", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{3, 1}, VariableDeclaration({3, 1}, {BOOL}, L"a", true),
        std::make_unique<CastExpression>(
            Position{3, 10}, std::make_unique<Variable>(Position{3, 10}, L"s1"), Type{BOOL}
        )
    ));
    checkSemanticError<InvalidCastError>(functionBody); // cannot do such cast
}

TEST_CASE("FunctionCall - explicit casts", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    std::vector<std::unique_ptr<Expression>> struct1Arguments;
    struct1Arguments.push_back(makeLiteral({3, 22}, 2));
    struct1Arguments.push_back(makeLiteral({3, 24}, L"2"));
    struct1Arguments.push_back(makeLiteral({3, 28}, 2.0));
    std::vector<std::unique_ptr<Expression>> struct2Arguments;
    struct2Arguments.push_back(std::make_unique<StructExpression>(Position{3, 21}, std::move(struct1Arguments)));
    struct2Arguments.push_back(makeLiteral({3, 31}, 2));
    std::vector<std::unique_ptr<Expression>> functionArguments;
    functionArguments.push_back(std::make_unique<StructExpression>(Position{3, 20}, std::move(struct2Arguments)));
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{3, 1}, Assignable({3, 1}, L"arg"),
        std::make_unique<DotExpression>(
            Position{3, 10},
            std::make_unique<DotExpression>(
                Position{3, 10},
                std::make_unique<FunctionCall>(Position{3, 10}, L"strt2", std::move(functionArguments)), L"a"
            ),
            L"a"
        )
    ));
    functionArguments.clear();
    functionArguments.push_back(makeLiteral(Position{4, 20}, 2));
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{4, 1}, Assignable({4, 1}, L"v1"),
        std::make_unique<FunctionCall>(Position{4, 10}, L"vart1", std::move(functionArguments))
    ));

    Program program = wrapInFunction(std::move(functionBody));
    doSemanticAnalysis(program);
    checkNodeContainer(
        program.functions, {wrappedFunctionHeader + L"`-Body:\n"
                                                    L" |-AssignmentStatement <line: 3, col: 1>\n"
                                                    L" ||-Assignable <line: 3, col: 1> right=arg\n"
                                                    L" |`-DotExpression <line: 3, col: 10> field=a\n"
                                                    L" | `-DotExpression <line: 3, col: 10> field=a\n"
                                                    L" |  `-StructExpression <line: 3, col: 20> structType=strt2\n"
                                                    L" |   |-StructExpression <line: 3, col: 21> structType=strt1\n"
                                                    L" |   ||-Literal <line: 3, col: 22> type=int value=2\n"
                                                    L" |   ||-Literal <line: 3, col: 24> type=str value=2\n"
                                                    L" |   |`-Literal <line: 3, col: 28> type=float value=2\n"
                                                    L" |   `-CastExpression <line: 3, col: 31> targetType=vart1\n"
                                                    L" |    `-Literal <line: 3, col: 31> type=int value=2\n"
                                                    L" `-AssignmentStatement <line: 4, col: 1>\n"
                                                    L"  |-Assignable <line: 4, col: 1> right=v1\n"
                                                    L"  `-CastExpression <line: 4, col: 10> targetType=vart1\n"
                                                    L"   `-Literal <line: 4, col: 20> type=int value=2\n"}
    );
}

TEST_CASE("FunctionCall - explicit cast to struct errors", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    std::vector<std::unique_ptr<Expression>> functionArguments;
    functionArguments.push_back(makeLiteral(Position{3, 20}, 2));
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{3, 1}, Assignable({3, 1}, L"s1"),
        std::make_unique<FunctionCall>(Position{3, 10}, L"strt1", std::move(functionArguments))
    ));
    checkSemanticError<InvalidCastError>(functionBody); // attempt to cast non-init-list into a struct

    functionBody.clear();
    std::vector<std::unique_ptr<Expression>> structArguments;
    structArguments.push_back(makeLiteral({3, 21}, 2));
    structArguments.push_back(makeLiteral({3, 23}, L"2"));
    structArguments.push_back(makeLiteral({3, 27}, 2.0));
    functionArguments.clear();
    functionArguments.push_back(std::make_unique<StructExpression>(Position{3, 20}, std::move(structArguments)));
    functionArguments.push_back(makeLiteral(Position{3, 30}, 2));
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{3, 1}, Assignable({3, 1}, L"s1"),
        std::make_unique<FunctionCall>(Position{3, 10}, L"strt1", std::move(functionArguments))
    ));
    checkSemanticError<InvalidCastError>(functionBody); // attempt to cast multiple arguments into a struct

    functionBody.clear();
    structArguments.clear();
    structArguments.push_back(std::make_unique<Variable>(Position{3, 20}, L"s1"));
    structArguments.push_back(makeLiteral({3, 23}, L"2"));
    structArguments.push_back(makeLiteral({3, 27}, 2.0));
    functionArguments.clear();
    functionArguments.push_back(std::make_unique<StructExpression>(Position{3, 20}, std::move(structArguments)));
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{3, 1}, Assignable({3, 1}, L"s1"),
        std::make_unique<FunctionCall>(Position{3, 10}, L"strt1", std::move(functionArguments))
    ));
    checkSemanticError<InvalidInitListError>(functionBody); // intialization list argument not castable

    functionBody.clear();
    structArguments.clear();
    structArguments.push_back(makeLiteral({3, 21}, 2));
    structArguments.push_back(makeLiteral({3, 23}, L"2"));
    structArguments.push_back(makeLiteral({3, 27}, 2.0));
    structArguments.push_back(makeLiteral({3, 31}, 2));
    functionArguments.clear();
    functionArguments.push_back(std::make_unique<StructExpression>(Position{3, 20}, std::move(structArguments)));
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{3, 1}, Assignable({3, 1}, L"s1"),
        std::make_unique<FunctionCall>(Position{3, 10}, L"strt1", std::move(functionArguments))
    ));
    checkSemanticError<InvalidInitListError>(functionBody); // intialization list has wrong number of arguments
}

TEST_CASE("FunctionCall - explicit cast to variant errors", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    std::vector<std::unique_ptr<Expression>> functionArguments;
    functionArguments.push_back(makeLiteral(Position{3, 20}, 2));
    functionArguments.push_back(makeLiteral(Position{3, 30}, 2));
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{3, 1}, Assignable({3, 1}, L"v1"),
        std::make_unique<FunctionCall>(Position{3, 10}, L"vart1", std::move(functionArguments))
    ));
    checkSemanticError<InvalidCastError>(functionBody); // attempt to cast multiple arguments into a variant

    functionBody.clear();
    functionArguments.clear();
    functionArguments.push_back(std::make_unique<Variable>(Position{3, 20}, L"s1"));
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{3, 1}, Assignable({3, 1}, L"v1"),
        std::make_unique<FunctionCall>(Position{3, 10}, L"vart1", std::move(functionArguments))
    ));
    checkSemanticError<InvalidCastError>(functionBody); // variant cast argument not castable
}

std::vector<VariableDeclaration> createParameters(const FunctionIdentification &id)
{
    std::vector<VariableDeclaration> parameters;
    for(unsigned i = 0; const Type &type: id.parameterTypes)
        parameters.emplace_back(Position{1, 1}, type, std::format(L"a{}", i++), false);
    return parameters;
}

std::wstring addOverload(
    Program &program, const FunctionIdentification &id, std::optional<Type> returnType = std::nullopt
)
{
    auto function = std::make_unique<FunctionDeclaration>(
        Position{1, 1}, L"<test>", createParameters(id), returnType, std::vector<std::unique_ptr<Instruction>>{}
    );
    std::wstringstream printed;
    std::pair<const FunctionIdentification, std::unique_ptr<BaseFunctionDeclaration>> toPrint{id, std::move(function)};
    PrintingVisitor(printed).visit(toPrint);
    program.functions.insert({id, std::move(toPrint.second)});
    return printed.str();
}

TEST_CASE("FunctionCall - overload resolution", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    std::vector<std::unique_ptr<Expression>> functionArguments;
    functionArguments.push_back(makeLiteral(Position{4, 20}, 2));
    functionArguments.push_back(makeLiteral(Position{4, 30}, 2.0));
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{4, 1}, Assignable(Position{4, 1}, L"arg"),
        std::make_unique<FunctionCall>(Position{4, 10}, L"f", std::move(functionArguments))
    ));
    functionArguments.clear();
    functionArguments.push_back(makeLiteral(Position{5, 20}, 2.0));
    functionArguments.push_back(makeLiteral(Position{5, 30}, 2));
    functionBody.push_back(std::make_unique<FunctionCallInstruction>(
        Position{5, 1}, FunctionCall(Position{5, 1}, L"f", std::move(functionArguments))
    ));
    Program program = wrapInFunction(std::move(functionBody));
    std::set<std::wstring> printedFunctions;
    printedFunctions.insert(addOverload(program, FunctionIdentification(L"f", {{INT}, {STR}}), {{INT}}));
    printedFunctions.insert(addOverload(program, FunctionIdentification(L"f", {{STR}, {INT}}), {}));
    printedFunctions.insert(addOverload(program, FunctionIdentification(L"f", {{INT}, {FLOAT}, {INT}}), {}));
    printedFunctions.insert(
        wrappedFunctionHeader + L"`-Body:\n"
                                L" |-AssignmentStatement <line: 4, col: 1>\n"
                                L" ||-Assignable <line: 4, col: 1> right=arg\n"
                                L" |`-FunctionCall <line: 4, col: 10> functionName=f\n"
                                L" | |-Literal <line: 4, col: 20> type=int value=2\n"
                                L" | `-CastExpression <line: 4, col: 30> targetType=str\n"
                                L" |  `-Literal <line: 4, col: 30> type=float value=2\n"
                                L" `-FunctionCallInstruction <line: 5, col: 1>\n"
                                L"  `-FunctionCall <line: 5, col: 1> functionName=f\n"
                                L"   |-CastExpression <line: 5, col: 20> targetType=str\n"
                                L"   |`-Literal <line: 5, col: 20> type=float value=2\n"
                                L"   `-Literal <line: 5, col: 30> type=int value=2\n"
    );
    doSemanticAnalysis(program);
    checkNodeContainer(program.functions, printedFunctions);
}

TEST_CASE("FunctionCall - overload resolution with initialization list", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    std::vector<std::unique_ptr<Expression>> structArguments;
    structArguments.push_back(makeLiteral({3, 21}, 2));
    structArguments.push_back(makeLiteral({3, 23}, 2));
    structArguments.push_back(makeLiteral({3, 27}, 2));
    std::vector<std::unique_ptr<Expression>> functionArguments;
    functionArguments.push_back(std::make_unique<StructExpression>(Position{3, 20}, std::move(structArguments)));
    functionBody.push_back(std::make_unique<FunctionCallInstruction>(
        Position{3, 1}, FunctionCall(Position{3, 1}, L"f", std::move(functionArguments))
    ));
    Program program = wrapInFunction(std::move(functionBody));
    std::set<std::wstring> printedFunctions;
    printedFunctions.insert(addOverload(program, FunctionIdentification(L"f", {{L"strt1"}})));
    printedFunctions.insert(addOverload(program, FunctionIdentification(L"f", {{L"strt2"}})));
    printedFunctions.insert(
        wrappedFunctionHeader + L"`-Body:\n"
                                L" `-FunctionCallInstruction <line: 3, col: 1>\n"
                                L"  `-FunctionCall <line: 3, col: 1> functionName=f\n"
                                L"   `-StructExpression <line: 3, col: 20> structType=strt1\n"
                                L"    |-Literal <line: 3, col: 21> type=int value=2\n"
                                L"    |-CastExpression <line: 3, col: 23> targetType=str\n"
                                L"    |`-Literal <line: 3, col: 23> type=int value=2\n"
                                L"    `-CastExpression <line: 3, col: 27> targetType=float\n"
                                L"     `-Literal <line: 3, col: 27> type=int value=2\n"
    );
    doSemanticAnalysis(program);
    checkNodeContainer(program.functions, printedFunctions);
}

TEST_CASE("FunctionCall - errors", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    std::vector<std::unique_ptr<Expression>> functionArguments;
    functionArguments.push_back(makeLiteral(Position{4, 20}, 2));
    functionBody.push_back(std::make_unique<FunctionCallInstruction>(
        Position{4, 1}, FunctionCall(Position{4, 1}, L"f", std::move(functionArguments))
    ));
    Program program = wrapInFunction(std::move(functionBody));
    addOverload(program, FunctionIdentification(L"f", {{INT}, {STR}}));
    addOverload(program, FunctionIdentification(L"f", {{STR}, {INT}}));
    REQUIRE_THROWS_AS(
        doSemanticAnalysis(program), InvalidFunctionCallError
    ); // no function with matching number of arguments

    functionBody.clear();
    functionArguments.clear();
    functionArguments.push_back(makeLiteral(Position{4, 20}, 2));
    functionArguments.push_back(makeLiteral(Position{4, 30}, 2));
    functionBody.push_back(std::make_unique<FunctionCallInstruction>(
        Position{4, 1}, FunctionCall(Position{4, 1}, L"f", std::move(functionArguments))
    ));
    program = wrapInFunction(std::move(functionBody));
    addOverload(program, FunctionIdentification(L"f", {{INT}, {STR}}));
    addOverload(program, FunctionIdentification(L"f", {{STR}, {INT}}));
    REQUIRE_THROWS_AS(
        doSemanticAnalysis(program), AmbiguousFunctionCallError
    ); // two functions with same number of casts required

    functionBody.clear();
    functionBody.push_back(std::make_unique<AssignmentStatement>(
        Position{4, 1}, Assignable(Position{4, 1}, L"arg"),
        std::make_unique<FunctionCall>(Position{4, 10}, L"f", std::vector<std::unique_ptr<Expression>>{})
    ));
    program = wrapInFunction(std::move(functionBody));
    addOverload(program, FunctionIdentification(L"f", {}));
    REQUIRE_THROWS_AS(
        doSemanticAnalysis(program), InvalidFunctionCallError
    ); // attempt to use function call with no return type in an expression

    functionBody.clear();
    functionArguments.clear();
    functionArguments.push_back(std::make_unique<Variable>(Position{4, 20}, L"arg"));
    functionArguments.push_back(makeLiteral({4, 30}, 2));
    functionBody.push_back(std::make_unique<FunctionCallInstruction>(
        Position{4, 1}, FunctionCall(Position{4, 10}, L"f", std::move(functionArguments))
    ));
    program = wrapInFunction(std::move(functionBody));
    using enum Type::Builtin;
    program.functions.insert(
        {FunctionIdentification(L"f", {{INT}, {STR}}),
         std::make_unique<FunctionDeclaration>(
             Position{1, 1}, L"<test>",
             std::vector<VariableDeclaration>{
                 VariableDeclaration({1, 10}, {INT}, L"a", false), VariableDeclaration({1, 20}, {STR}, L"b", true)
             },
             std::nullopt, std::vector<std::unique_ptr<Instruction>>{}
         )}
    );
    REQUIRE_THROWS_AS(
        doSemanticAnalysis(program), ImmutableError
    ); // cannot pass immutable or temporary value as mutable reference
}

TEST_CASE("conditional statements", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody, instructions;
    instructions.push_back(std::make_unique<VariableDeclStatement>(
        Position{3, 50}, VariableDeclaration({3, 50}, {INT}, L"a", true), makeLiteral(Position{3, 60}, 0)
    ));
    std::vector<SingleIfCase> ifCases;
    ifCases.push_back(SingleIfCase(Position{3, 1}, makeLiteral(Position{3, 10}, 2), {}));
    functionBody.push_back(std::make_unique<IfStatement>(Position{3, 1}, std::move(ifCases), std::move(instructions)));

    instructions.clear();
    instructions.push_back(std::make_unique<VariableDeclStatement>(
        Position{4, 50}, VariableDeclaration({4, 50}, {INT}, L"a", true), makeLiteral(Position{4, 60}, 0)
    ));
    functionBody.push_back(
        std::make_unique<WhileStatement>(Position{4, 1}, makeLiteral(Position{4, 10}, 2), std::move(instructions))
    );

    instructions.clear();
    instructions.push_back(std::make_unique<VariableDeclStatement>(
        Position{5, 50}, VariableDeclaration({5, 50}, {INT}, L"a", true), makeLiteral(Position{5, 60}, 0)
    ));
    functionBody.push_back(
        std::make_unique<DoWhileStatement>(Position{5, 1}, makeLiteral(Position{5, 10}, true), std::move(instructions))
    );
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{6, 1}, VariableDeclaration({6, 1}, {INT}, L"a", true), makeLiteral(Position{6, 10}, 0)
    ));
    Program program = wrapInFunction(std::move(functionBody));

    doSemanticAnalysis(program);
    checkNodeContainer(
        program.functions,
        {wrappedFunctionHeader + L"`-Body:\n"
                                 L" |-IfStatement <line: 3, col: 1>\n"
                                 L" ||-SingleIfCase <line: 3, col: 1>\n"
                                 L" ||`-CastExpression <line: 3, col: 10> targetType=bool\n"
                                 L" || `-Literal <line: 3, col: 10> type=int value=2\n"
                                 L" |`-ElseCase:\n"
                                 L" | `-VariableDeclStatement <line: 3, col: 50>\n"
                                 L" |  |-VariableDeclaration <line: 3, col: 50> type=int name=a mutable=true\n"
                                 L" |  `-Literal <line: 3, col: 60> type=int value=0\n"
                                 L" |-WhileStatement <line: 4, col: 1>\n"
                                 L" ||-CastExpression <line: 4, col: 10> targetType=bool\n"
                                 L" ||`-Literal <line: 4, col: 10> type=int value=2\n"
                                 L" |`-VariableDeclStatement <line: 4, col: 50>\n"
                                 L" | |-VariableDeclaration <line: 4, col: 50> type=int name=a mutable=true\n"
                                 L" | `-Literal <line: 4, col: 60> type=int value=0\n"
                                 L" |-DoWhileStatement <line: 5, col: 1>\n"
                                 L" ||-Literal <line: 5, col: 10> type=bool value=true\n"
                                 L" |`-VariableDeclStatement <line: 5, col: 50>\n"
                                 L" | |-VariableDeclaration <line: 5, col: 50> type=int name=a mutable=true\n"
                                 L" | `-Literal <line: 5, col: 60> type=int value=0\n"
                                 L" `-VariableDeclStatement <line: 6, col: 1>\n"
                                 L"  |-VariableDeclaration <line: 6, col: 1> type=int name=a mutable=true\n"
                                 L"  `-Literal <line: 6, col: 10> type=int value=0\n"}
    );
}

TEST_CASE("variant access in condition", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    std::vector<SingleIfCase> ifCases;
    ifCases.push_back(SingleIfCase(
        Position{3, 1},
        VariableDeclStatement(
            {3, 10}, VariableDeclaration({3, 10}, {INT}, L"a", false),
            std::make_unique<Variable>(Position{3, 20}, L"v1")
        ),
        {}
    ));
    ifCases.push_back(SingleIfCase(
        Position{4, 1},
        VariableDeclStatement(
            {4, 10}, VariableDeclaration({4, 10}, {STR}, L"a", false),
            std::make_unique<DotExpression>(Position{4, 20}, std::make_unique<Variable>(Position{4, 20}, L"v1"), L"b")
        ),
        {}
    ));
    functionBody.push_back(
        std::make_unique<IfStatement>(Position{3, 1}, std::move(ifCases), std::vector<std::unique_ptr<Instruction>>{})
    );
    Program program = wrapInFunction(std::move(functionBody));
    doSemanticAnalysis(program);
    checkNodeContainer(
        program.functions,
        {wrappedFunctionHeader + L"`-Body:\n"
                                 L" `-IfStatement <line: 3, col: 1>\n"
                                 L"  |-SingleIfCase <line: 3, col: 1>\n"
                                 L"  |`-VariableDeclStatement <line: 3, col: 10>\n"
                                 L"  | |-VariableDeclaration <line: 3, col: 10> type=int name=a mutable=false\n"
                                 L"  | `-Variable <line: 3, col: 20> name=v1\n"
                                 L"  `-SingleIfCase <line: 4, col: 1>\n"
                                 L"   `-VariableDeclStatement <line: 4, col: 10>\n"
                                 L"    |-VariableDeclaration <line: 4, col: 10> type=str name=a mutable=false\n"
                                 L"    `-Variable <line: 4, col: 20> name=v1\n"}
    );
}

TEST_CASE("if condition declaration errors", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    std::vector<SingleIfCase> ifCases;
    ifCases.push_back(SingleIfCase(
        Position{3, 1},
        VariableDeclStatement(
            {3, 10}, VariableDeclaration({3, 10}, {INT}, L"a", false),
            std::make_unique<DotExpression>(
                Position{3, 20},
                std::make_unique<DotExpression>(
                    Position{3, 20}, std::make_unique<Variable>(Position{3, 20}, L"v2"), L"b"
                ),
                L"a"
            )
        ),
        {}
    ));
    functionBody.push_back(
        std::make_unique<IfStatement>(Position{3, 1}, std::move(ifCases), std::vector<std::unique_ptr<Instruction>>{})
    );
    checkSemanticError<FieldAccessError>(functionBody); // cannot access subfield of variant in if condition

    functionBody.clear();
    ifCases.clear();
    ifCases.push_back(SingleIfCase(
        Position{3, 1},
        VariableDeclStatement({3, 10}, VariableDeclaration({3, 10}, {INT}, L"a", false), makeLiteral({3, 20}, 2)), {}
    ));
    functionBody.push_back(
        std::make_unique<IfStatement>(Position{3, 1}, std::move(ifCases), std::vector<std::unique_ptr<Instruction>>{})
    );
    checkSemanticError<InvalidIfConditionError>(functionBody); // must access variant in if condition declaration

    functionBody.clear();
    ifCases.clear();
    ifCases.push_back(SingleIfCase(
        Position{3, 1},
        VariableDeclStatement(
            {3, 10}, VariableDeclaration({3, 10}, {BOOL}, L"a", false),
            std::make_unique<Variable>(Position{3, 20}, L"v1")
        ),
        {}
    ));
    functionBody.push_back(
        std::make_unique<IfStatement>(Position{3, 1}, std::move(ifCases), std::vector<std::unique_ptr<Instruction>>{})
    );
    checkSemanticError<InvalidCastError>(functionBody); // if condition declared type must match a field's type

    functionBody.clear();
    ifCases.clear();
    ifCases.push_back(SingleIfCase(
        Position{3, 1},
        VariableDeclStatement(
            {3, 10}, VariableDeclaration({3, 10}, {STR}, L"a", false),
            std::make_unique<DotExpression>(Position{3, 20}, std::make_unique<Variable>(Position{3, 20}, L"v1"), L"a")
        ),
        {}
    ));
    functionBody.push_back(
        std::make_unique<IfStatement>(Position{3, 1}, std::move(ifCases), std::vector<std::unique_ptr<Instruction>>{})
    );
    checkSemanticError<InvalidCastError>(functionBody); // if condition declared type must match the field
}

TEST_CASE("other conditional statements errors", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    functionBody.push_back(std::make_unique<WhileStatement>(
        Position{3, 1}, std::make_unique<Variable>(Position{3, 10}, L"s1"), std::vector<std::unique_ptr<Instruction>>{}
    ));
    checkSemanticError<InvalidCastError>(functionBody); // condition uncastable to bool

    functionBody.clear();
    std::vector<std::unique_ptr<Instruction>> instructions;
    instructions.push_back(std::make_unique<VariableDeclStatement>(
        Position{4, 1}, VariableDeclaration({4, 1}, {INT}, L"arg", false), makeLiteral({4, 10}, 2)
    ));
    functionBody.push_back(
        std::make_unique<WhileStatement>(Position{3, 1}, makeLiteral(Position{3, 10}, true), std::move(instructions))
    );
    checkSemanticError<VariableNameCollisionError>(functionBody); // redeclaration of variable from outside scope

    functionBody.clear();
    instructions.clear();
    instructions.push_back(std::make_unique<VariableDeclStatement>(
        Position{3, 1}, VariableDeclaration({3, 1}, {INT}, L"a", false), makeLiteral({3, 10}, 2)
    ));
    functionBody.push_back(
        std::make_unique<DoWhileStatement>(Position{4, 1}, makeLiteral(Position{4, 10}, true), std::move(instructions))
    );
    functionBody.push_back(std::make_unique<VariableDeclStatement>(
        Position{5, 1}, VariableDeclaration({5, 1}, {INT}, L"abc", false),
        std::make_unique<Variable>(Position{5, 10}, L"a")
    ));
    checkSemanticError<UnknownVariableError>(functionBody); // deletion of variable upon leaving scope

    functionBody.clear();
    instructions.clear();
    instructions.push_back(std::make_unique<VariableDeclStatement>(
        Position{4, 1}, VariableDeclaration({4, 1}, {INT}, L"a", false), makeLiteral({4, 10}, 2)
    ));
    std::vector<SingleIfCase> ifCases;
    ifCases.push_back(SingleIfCase(
        Position{3, 1},
        VariableDeclStatement(
            {3, 10}, VariableDeclaration({3, 10}, {INT}, L"a", false),
            std::make_unique<Variable>(Position{3, 20}, L"v1")
        ),
        std::move(instructions)
    ));
    functionBody.push_back(
        std::make_unique<IfStatement>(Position{3, 1}, std::move(ifCases), std::vector<std::unique_ptr<Instruction>>{})
    );
    checkSemanticError<VariableNameCollisionError>(functionBody); // redeclaration of variable from if condition
}

TEST_CASE("non-structural statements: break, continue, return", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody, instructions;
    instructions.push_back(std::make_unique<ReturnStatement>(Position{3, 50}, nullptr));
    std::vector<SingleIfCase> ifCases;
    ifCases.push_back(SingleIfCase(Position{3, 1}, makeLiteral(Position{3, 10}, true), {}));
    functionBody.push_back(std::make_unique<IfStatement>(Position{3, 1}, std::move(ifCases), std::move(instructions)));

    instructions.clear();
    instructions.push_back(std::make_unique<BreakStatement>(Position{4, 50}));
    functionBody.push_back(
        std::make_unique<WhileStatement>(Position{4, 1}, makeLiteral(Position{4, 10}, true), std::move(instructions))
    );

    instructions.clear();
    instructions.push_back(std::make_unique<ContinueStatement>(Position{5, 50}));
    functionBody.push_back(
        std::make_unique<DoWhileStatement>(Position{5, 1}, makeLiteral(Position{5, 10}, true), std::move(instructions))
    );
    functionBody.push_back(std::make_unique<ReturnStatement>(Position{6, 1}, nullptr));
    Program program = wrapInFunction(std::move(functionBody));

    doSemanticAnalysis(program);
    checkNodeContainer(
        program.functions, {wrappedFunctionHeader + L"`-Body:\n"
                                                    L" |-IfStatement <line: 3, col: 1>\n"
                                                    L" ||-SingleIfCase <line: 3, col: 1>\n"
                                                    L" ||`-Literal <line: 3, col: 10> type=bool value=true\n"
                                                    L" |`-ElseCase:\n"
                                                    L" | `-ReturnStatement <line: 3, col: 50>\n"
                                                    L" |-WhileStatement <line: 4, col: 1>\n"
                                                    L" ||-Literal <line: 4, col: 10> type=bool value=true\n"
                                                    L" |`-BreakStatement <line: 4, col: 50>\n"
                                                    L" |-DoWhileStatement <line: 5, col: 1>\n"
                                                    L" ||-Literal <line: 5, col: 10> type=bool value=true\n"
                                                    L" |`-ContinueStatement <line: 5, col: 50>\n"
                                                    L" `-ReturnStatement <line: 6, col: 1>\n"}
    );
}

TEST_CASE("ReturnStatement with value cast", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody, instructions;
    instructions.push_back(std::make_unique<ReturnStatement>(Position{4, 1}, makeLiteral(Position{4, 10}, L"2")));
    functionBody.push_back(
        std::make_unique<WhileStatement>(Position{3, 1}, makeLiteral(Position{3, 10}, true), std::move(instructions))
    );
    functionBody.push_back(std::make_unique<ReturnStatement>(Position{5, 1}, makeLiteral(Position{5, 10}, 2)));

    Program program({1, 1});
    program.functions.emplace(
        FunctionIdentification(L"function", {}), std::make_unique<FunctionDeclaration>(
                                                     Position{1, 1}, L"<test>", std::vector<VariableDeclaration>{},
                                                     std::optional<Type>{{INT}}, std::move(functionBody)
                                                 )
    );

    doSemanticAnalysis(program);
    checkNodeContainer(
        program.functions, {L"function: FunctionDeclaration <line: 1, col: 1> source=<test> returnType=int\n"
                            L"`-Body:\n"
                            L" |-WhileStatement <line: 3, col: 1>\n"
                            L" ||-Literal <line: 3, col: 10> type=bool value=true\n"
                            L" |`-ReturnStatement <line: 4, col: 1>\n"
                            L" | `-CastExpression <line: 4, col: 10> targetType=int\n"
                            L" |  `-Literal <line: 4, col: 10> type=str value=2\n"
                            L" `-ReturnStatement <line: 5, col: 1>\n"
                            L"  `-Literal <line: 5, col: 10> type=int value=2\n"}
    );
}

TEST_CASE("BreakStatement and ContinueStatement errors", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody, instructions;
    instructions.push_back(std::make_unique<BreakStatement>(Position{3, 50}));
    std::vector<SingleIfCase> ifCases;
    ifCases.push_back(SingleIfCase(Position{3, 1}, makeLiteral(Position{3, 10}, true), {}));
    functionBody.push_back(std::make_unique<IfStatement>(Position{3, 1}, std::move(ifCases), std::move(instructions)));
    checkSemanticError<InvalidBreakError>(functionBody); // break in if (not any loop) not allowed

    instructions.clear();
    instructions.push_back(std::make_unique<DoWhileStatement>(
        Position{3, 1}, makeLiteral(Position{3, 10}, true), std::vector<std::unique_ptr<Instruction>>{}
    ));
    functionBody.push_back(
        std::make_unique<WhileStatement>(Position{4, 1}, makeLiteral(Position{4, 10}, true), std::move(instructions))
    );
    functionBody.push_back(std::make_unique<ContinueStatement>(Position{4, 50}));
    checkSemanticError<InvalidContinueError>(functionBody); // continue outside of loop not allowed
}

TEST_CASE("ReturnStatement errors", "[doSemanticAnalysis]")
{
    std::vector<std::unique_ptr<Instruction>> functionBody;
    functionBody.push_back(std::make_unique<ReturnStatement>(Position{3, 1}, makeLiteral(Position{3, 10}, 2)));

    Program program({1, 1});
    program.functions.emplace(
        FunctionIdentification(L"function", {}),
        std::make_unique<FunctionDeclaration>(
            Position{1, 1}, L"<test>", std::vector<VariableDeclaration>{}, std::nullopt, std::move(functionBody)
        )
    );
    REQUIRE_THROWS_AS(
        doSemanticAnalysis(program), InvalidReturnError
    ); // return with value disallowed in function with no return type

    functionBody.clear();
    functionBody.push_back(std::make_unique<ReturnStatement>(Position{3, 1}, nullptr));

    program = Program({1, 1});
    program.functions.emplace(
        FunctionIdentification(L"function", {}), std::make_unique<FunctionDeclaration>(
                                                     Position{1, 1}, L"<test>", std::vector<VariableDeclaration>{},
                                                     std::optional<Type>{{INT}}, std::move(functionBody)
                                                 )
    );
    REQUIRE_THROWS_AS(
        doSemanticAnalysis(program), InvalidReturnError
    ); // return without value disallowed in function with return type

    functionBody.clear();
    std::vector<std::unique_ptr<Expression>> structArguments;
    structArguments.push_back(makeLiteral(Position{3, 20}, L"2"));
    structArguments.push_back(makeLiteral(Position{3, 30}, L"3"));
    functionBody.push_back(std::make_unique<ReturnStatement>(
        Position{3, 1}, std::make_unique<StructExpression>(Position{3, 10}, std::move(structArguments))
    ));

    program = Program({1, 1});
    program.functions.emplace(
        FunctionIdentification(L"function", {}), std::make_unique<FunctionDeclaration>(
                                                     Position{1, 1}, L"<test>", std::vector<VariableDeclaration>{},
                                                     std::optional<Type>{{INT}}, std::move(functionBody)
                                                 )
    );
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), InvalidCastError); // return value not castable to return type
}
