#include "interpreter.hpp"

#include "commentDiscarder.hpp"
#include "helpers.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "runtimeExceptions.hpp"
#include "streamReader.hpp"

#include <catch2/catch_test_macros.hpp>

#include <fstream>

using enum Type::Builtin;

Program parseFromStream(std::wistream &input, const std::wstring &fileName)
{
    StreamReader reader(input, fileName);
    Lexer lexer(reader);
    CommentDiscarder commentDiscarder(lexer);
    Parser parser(commentDiscarder);
    return parser.parseProgram();
}

TEST_CASE("hello world", "[Interpreter]")
{
    Program program({1, 1});
    std::vector<std::unique_ptr<Expression>> arguments;
    arguments.push_back(makeLiteral(Position{2, 10}, L"hello, world!"));
    std::vector<std::unique_ptr<Instruction>> instructions;
    instructions.push_back(std::make_unique<FunctionCallInstruction>(
        Position{2, 1}, FunctionCall({2, 1}, L"println", std::move(arguments))
    ));
    program.functions.emplace(
        FunctionIdentification(L"main", {}),
        std::make_unique<FunctionDeclaration>(
            Position{1, 1}, L"<test>", std::vector<VariableDeclaration>{}, std::nullopt, std::move(instructions)
        )
    );
    std::wstringstream input, output;
    Interpreter interpreter(L"<test>", {}, input, output, parseFromStream);
    interpreter.visit(program);
    REQUIRE(output.str() == L"hello, world!\n");
}

TEST_CASE("main function errors", "[Interpreter]")
{
    Program program({1, 1});
    program.functions.emplace(
        FunctionIdentification(L"maint", {}), std::make_unique<FunctionDeclaration>(
                                                  Position{1, 1}, L"<test>", std::vector<VariableDeclaration>{},
                                                  std::nullopt, std::vector<std::unique_ptr<Instruction>>{}
                                              )
    );
    program.functions.emplace(
        FunctionIdentification(L"main", {{INT}}), std::make_unique<FunctionDeclaration>(
                                                      Position{1, 1}, L"<test>", std::vector<VariableDeclaration>{},
                                                      std::nullopt, std::vector<std::unique_ptr<Instruction>>{}
                                                  )
    );
    std::wstringstream input, output;
    Interpreter interpreter(L"<test>", {}, input, output, parseFromStream);
    REQUIRE_THROWS_AS(interpreter.visit(program), MainNotFoundError); // no function with signature main()

    program = Program({1, 1});
    std::vector<std::unique_ptr<Instruction>> instructions;
    instructions.push_back(std::make_unique<ReturnStatement>(Position{2, 1}, makeLiteral({2, 10}, 2)));
    program.functions.emplace(
        FunctionIdentification(L"main", {}),
        std::make_unique<FunctionDeclaration>(
            Position{1, 1}, L"<test>", std::vector<VariableDeclaration>{}, Type{INT}, std::move(instructions)
        )
    );
    REQUIRE_THROWS_AS(interpreter.visit(program), MainReturnTypeError); // main() has a return type
}

TEST_CASE("VariableDeclStatement and Variable", "[Interpreter]")
{
    Program program({1, 1});
    std::vector<std::unique_ptr<Instruction>> instructions;
    instructions.push_back(std::make_unique<VariableDeclStatement>(
        Position{2, 1}, VariableDeclaration({2, 1}, {STR}, L"a", false), makeLiteral({2, 10}, L"msg")
    ));
    std::vector<std::unique_ptr<Expression>> arguments;
    arguments.push_back(std::make_unique<Variable>(Position{3, 10}, L"a"));
    instructions.push_back(std::make_unique<FunctionCallInstruction>(
        Position{3, 1}, FunctionCall({3, 1}, L"println", std::move(arguments))
    ));
    program.functions.emplace(
        FunctionIdentification(L"main", {}),
        std::make_unique<FunctionDeclaration>(
            Position{1, 1}, L"<test>", std::vector<VariableDeclaration>{}, std::nullopt, std::move(instructions)
        )
    );
    std::wstringstream input, output;
    Interpreter interpreter(L"<test>", {}, input, output, parseFromStream);
    interpreter.visit(program);
    REQUIRE(output.str() == L"msg\n");
}

TEST_CASE("AssignmentStatement", "[Interpreter]")
{
    Program program({1, 1});
    std::vector<std::unique_ptr<Instruction>> instructions;
    instructions.push_back(std::make_unique<VariableDeclStatement>(
        Position{2, 1}, VariableDeclaration({2, 1}, {STR}, L"a", true), makeLiteral({2, 10}, L"msg")
    ));
    instructions.push_back(std::make_unique<AssignmentStatement>(
        Position{3, 1}, Assignable({3, 1}, L"a"), makeLiteral({2, 10}, L"not msg")
    ));
    std::vector<std::unique_ptr<Expression>> arguments;
    arguments.push_back(std::make_unique<Variable>(Position{3, 10}, L"a"));
    instructions.push_back(std::make_unique<FunctionCallInstruction>(
        Position{4, 1}, FunctionCall({4, 1}, L"println", std::move(arguments))
    ));
    program.functions.emplace(
        FunctionIdentification(L"main", {}),
        std::make_unique<FunctionDeclaration>(
            Position{1, 1}, L"<test>", std::vector<VariableDeclaration>{}, std::nullopt, std::move(instructions)
        )
    );
    std::wstringstream input, output;
    Interpreter interpreter(L"<test>", {}, input, output, parseFromStream);
    interpreter.visit(program);
    REQUIRE(output.str() == L"not msg\n");
}

void doCastTest(
    Type target, std::variant<std::wstring, int32_t, double, bool> source, const std::wstring &expectedOutput
)
{
    Program program({1, 1});
    std::vector<std::unique_ptr<Instruction>> instructions;
    std::vector<std::unique_ptr<Expression>> arguments;
    arguments.push_back(std::make_unique<CastExpression>(Position{3, 10}, makeLiteral({3, 20}, source), target));
    instructions.push_back(
        std::make_unique<FunctionCallInstruction>(Position{3, 1}, FunctionCall({3, 1}, L"print", std::move(arguments)))
    );
    program.functions.emplace(
        FunctionIdentification(L"main", {}),
        std::make_unique<FunctionDeclaration>(
            Position{1, 1}, L"<test>", std::vector<VariableDeclaration>{}, std::nullopt, std::move(instructions)
        )
    );
    std::wstringstream input, output;
    Interpreter interpreter(L"<test>", {}, input, output, parseFromStream);
    interpreter.visit(program);
    REQUIRE(output.str() == expectedOutput);
}

TEST_CASE("CastExpression", "[Interpreter]")
{
    // float -> int
    doCastTest({INT}, 0.2, L"0");
    doCastTest({INT}, 10.5, L"11");
    doCastTest({INT}, -10.5, L"-11");
    REQUIRE_THROWS_AS(doCastTest({INT}, 1e100, L""), CastImpossibleError);
    REQUIRE_THROWS_AS(doCastTest({INT}, -1e100, L""), CastImpossibleError);
    // str -> int
    doCastTest({INT}, L"123", L"123");
    doCastTest({INT}, L"   123\r\n", L"123");
    doCastTest({INT}, L"-200", L"-200");
    REQUIRE_THROWS_AS(doCastTest({INT}, L"2.4", L""), CastImpossibleError);
    REQUIRE_THROWS_AS(
        doCastTest({INT}, std::to_wstring(1ULL + std::numeric_limits<int32_t>::max()), L""), CastImpossibleError
    );
    REQUIRE_THROWS_AS(
        doCastTest({INT}, std::to_wstring(-1ULL + std::numeric_limits<int32_t>::min()), L""), CastImpossibleError
    );
    REQUIRE_THROWS_AS(doCastTest({INT}, L"fdhbt", L""), CastImpossibleError);
    REQUIRE_THROWS_AS(doCastTest({INT}, L"-", L""), CastImpossibleError);
    // bool -> int
    doCastTest({INT}, true, L"1");
    doCastTest({INT}, false, L"0");
    // int -> float
    doCastTest({FLOAT}, 2, L"2");
    doCastTest({FLOAT}, 10, L"10");
    doCastTest({FLOAT}, -10, L"-10");
    // str -> float
    doCastTest({FLOAT}, L"1.23", L"1.23");
    doCastTest({FLOAT}, L"   -12.3\r\n", L"-12.3");
    doCastTest({FLOAT}, L"-20", L"-20");
    REQUIRE_THROWS_AS(doCastTest({FLOAT}, L"1.3 fdhbt", L""), CastImpossibleError);
    REQUIRE_THROWS_AS(doCastTest({FLOAT}, L"fdhbt", L""), CastImpossibleError);
    REQUIRE_THROWS_AS(doCastTest({FLOAT}, L"-", L""), CastImpossibleError);
    // bool -> float
    doCastTest({FLOAT}, true, L"1");
    doCastTest({FLOAT}, false, L"0");
    // int -> bool
    doCastTest({BOOL}, 2, L"true");
    doCastTest({BOOL}, 0, L"false");
    doCastTest({BOOL}, -10, L"true");
    // float -> bool
    doCastTest({BOOL}, 1.23, L"true");
    doCastTest({BOOL}, 0.0, L"false");
    doCastTest({BOOL}, -20.0, L"true");
    // str -> bool
    doCastTest({BOOL}, L"1.23", L"true");
    doCastTest({BOOL}, L"", L"false");
    doCastTest({BOOL}, L"abc", L"true");
}
