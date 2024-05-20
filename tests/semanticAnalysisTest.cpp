#include "semanticAnalysis.hpp"

#include "documentTree.hpp"
#include "interpreterExceptions.hpp"

#include <catch2/catch_test_macros.hpp>

using enum Type::Builtin;

TEST_CASE("valid single top level statement Programs", "[doSemanticAnalysis]")
{
    Program program({1, 1});
    doSemanticAnalysis(program);
    program.structs.insert({L"structure", StructDeclaration({1, 1}, L"<test>", {Field({2, 2}, {INT}, L"b")})});
    doSemanticAnalysis(program);
    program.structs.clear();
    program.variants.insert({L"wariant", VariantDeclaration({1, 1}, L"<test>", {Field({2, 2}, {INT}, L"b")})});
    doSemanticAnalysis(program);
    program.variants.clear();
    program.functions.insert(
        {FunctionIdentification(L"function", {}), FunctionDeclaration({1, 1}, L"<test>", {}, {}, {})}
    );
    doSemanticAnalysis(program);
}

TEST_CASE("include in semantic analysis", "[doSemanticAnalysis]")
{
    Program program({1, 1});
    program.includes.push_back(IncludeStatement({1, 1}, L"file"));
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), IncludeInSemanticAnalysisError);
}

TEST_CASE("nonexistent field types", "[doSemanticAnalysis]")
{
    Program program({1, 1});
    program.structs.insert({L"structure", StructDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"some_type"}, L"b")})});
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), UnknownFieldTypeError);
    program.structs.clear();
    program.variants.insert({L"wariant", VariantDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"some_type"}, L"b")})});
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), UnknownFieldTypeError);
}

TEST_CASE("duplicate field names", "[doSemanticAnalysis]")
{
    Program program({1, 1});
    program.structs.insert(
        {L"structure", StructDeclaration(
                           {1, 1}, L"<test>",
                           {
                               Field({2, 2}, {INT}, L"b"),
                               Field({3, 2}, {INT}, L"b"),
                           }
                       )}
    );
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), FieldNameCollisionError);
    program.structs.clear();
    program.variants.insert(
        {L"wariant", VariantDeclaration(
                         {1, 1}, L"<test>",
                         {
                             Field({2, 2}, {INT}, L"b"),
                             Field({3, 2}, {INT}, L"b"),
                         }
                     )}
    );
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), FieldNameCollisionError);
}

TEST_CASE("multiple structures and variants - valid Program", "[doSemanticAnalysis]")
{
    Program program({1, 1});
    program.structs.insert(
        {L"structure", StructDeclaration(
                           {1, 1}, L"<test>",
                           {
                               Field({2, 2}, {INT}, L"b"),
                               Field({3, 2}, {L"wariat"}, L"c"),
                           }
                       )}
    );
    program.structs.insert({L"struktura", StructDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"structure"}, L"b")})});
    program.structs.insert({L"sint", StructDeclaration({1, 1}, L"<test>", {Field({2, 2}, {INT}, L"b")})});
    program.variants.insert(
        {L"wariant", VariantDeclaration(
                         {1, 1}, L"<test>",
                         {
                             Field({2, 2}, {INT}, L"b"),
                             Field({3, 2}, {STR}, L"c"),
                         }
                     )}
    );
    program.variants.insert(
        {L"wariat", VariantDeclaration(
                        {1, 1}, L"<test>",
                        {
                            Field({2, 2}, {L"wariant"}, L"b"),
                            Field({3, 2}, {L"sint"}, L"c"),
                            Field({3, 2}, {FLOAT}, L"wariat"),
                        }
                    )}
    );
    doSemanticAnalysis(program);
}

TEST_CASE("recurrent fields", "[doSemanticAnalysis]")
{
    Program program({1, 1}); // direct recursion in structure
    program.structs.insert({L"struktura", StructDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"struktura"}, L"b")})});
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), FieldTypeRecursionError);

    program.structs.clear(); // indirect recursion in structure
    program.structs.insert({L"st1", StructDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"st2"}, L"b")})});
    program.structs.insert({L"st2", StructDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"st1"}, L"b")})});
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), FieldTypeRecursionError);

    program.structs.clear(); // direct recursion in variant
    program.variants.insert({L"wariant", VariantDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"wariant"}, L"b")})});
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), FieldTypeRecursionError);
    program.variants.clear(); // indirect recursion in variant
    program.variants.insert({L"w1", VariantDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"w2"}, L"b")})});
    program.variants.insert({L"w2", VariantDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"w1"}, L"b")})});
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), FieldTypeRecursionError);

    program.variants.clear(); // recursion in both struct and variant, 4 times removed
    program.structs.insert({L"s1", StructDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"s2"}, L"b")})});
    program.structs.insert({L"s2", StructDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"w1"}, L"b")})});
    program.variants.insert({L"w1", VariantDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"w2"}, L"b")})});
    program.variants.insert({L"w2", VariantDeclaration({1, 1}, L"<test>", {Field({2, 2}, {L"s1"}, L"b")})});
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), FieldTypeRecursionError);
}

TEST_CASE("struct, variant, function name collisions", "[doSemanticAnalysis]")
{
    Program program({1, 1}); // struct-variant collision
    program.structs.insert({L"structure", StructDeclaration({1, 1}, L"<test>", {Field({2, 2}, {INT}, L"b")})});
    program.variants.insert({L"structure", VariantDeclaration({1, 1}, L"<test>", {Field({2, 2}, {INT}, L"b")})});
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), NameCollisionError);

    program = Program({1, 1}); // struct-function collision
    program.structs.insert({L"structure", StructDeclaration({1, 1}, L"<test>", {Field({2, 2}, {INT}, L"b")})});
    program.functions.insert(
        {FunctionIdentification(L"structure", {}), FunctionDeclaration({1, 1}, L"<test>", {}, {}, {})}
    );
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), NameCollisionError);

    program = Program({1, 1}); // struct-function collision
    program.variants.insert({L"structure", VariantDeclaration({1, 1}, L"<test>", {Field({2, 2}, {INT}, L"b")})});
    program.functions.insert(
        {FunctionIdentification(L"structure", {}), FunctionDeclaration({1, 1}, L"<test>", {}, {}, {})}
    );
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), NameCollisionError);
}

TEST_CASE("valid function parameters", "[doSemanticAnalysis]")
{
    Program program({1, 1});
    program.functions.insert(
        {FunctionIdentification(L"function", {}), FunctionDeclaration(
                                                      {1, 1}, L"<test>",
                                                      {
                                                          VariableDeclaration({2, 1}, {INT}, L"a", false),
                                                          VariableDeclaration({3, 1}, {STR}, L"b", true),
                                                          VariableDeclaration({4, 1}, {BOOL}, L"c", false),
                                                          VariableDeclaration({5, 1}, {INT}, L"d", false),
                                                      },
                                                      {}, {}
                                                  )}
    );
    doSemanticAnalysis(program);
}

TEST_CASE("function parameter name collision", "[doSemanticAnalysis]")
{
    Program program({1, 1}); // struct-variant collision
    program.functions.insert(
        {FunctionIdentification(L"function", {}), FunctionDeclaration(
                                                      {1, 1}, L"<test>",
                                                      {
                                                          VariableDeclaration({2, 1}, {INT}, L"a", false),
                                                          VariableDeclaration({3, 1}, {STR}, L"a", true),
                                                      },
                                                      {}, {}
                                                  )}
    );
    REQUIRE_THROWS_AS(doSemanticAnalysis(program), ParameterNameCollisionError);
}
