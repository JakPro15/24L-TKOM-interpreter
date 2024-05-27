#include "interpreter.hpp"

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
    Parser parser(lexer);
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

// TEST_CASE("VariableDeclStatement and Variable", "[Interpreter]")
// {
//     Program program({1, 1});
//     std::vector<std::unique_ptr<Expression>> arguments;
//     arguments.push_back(makeLiteral(Position{2, 10}, L"hello, world!"));
//     std::vector<std::unique_ptr<Instruction>> instructions;
//     instructions.push_back(std::make_unique<FunctionCallInstruction>(
//         Position{2, 1}, FunctionCall({2, 1}, L"println", std::move(arguments))
//     ));
//     program.functions.emplace(
//         FunctionIdentification(L"main", {}),
//         std::make_unique<FunctionDeclaration>(
//             Position{1, 1}, L"<test>", std::vector<VariableDeclaration>{}, std::nullopt, std::move(instructions)
//         )
//     );
//     std::wstringstream input, output;
//     Interpreter interpreter(L"<test>", {}, input, output, parseFromStream);
//     interpreter.visit(program);
//     REQUIRE(output.str() == L"hello, world!\n");
// }
