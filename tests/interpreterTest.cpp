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

TEST_CASE("FunctionDeclaration and passing values by reference", "[Interpreter]")
{
    Program program({1, 1});
    std::vector<std::unique_ptr<Instruction>> instructions;
    std::vector<std::unique_ptr<Expression>> arguments;
    arguments.push_back(makeLiteral({7, 20}, L"inside function"));
    instructions.push_back(std::make_unique<FunctionCallInstruction>(
        Position{1, 100}, FunctionCall({1, 110}, L"println", std::move(arguments))
    ));
    instructions.push_back(std::make_unique<AssignmentStatement>(
        Position{2, 1}, Assignable({2, 1}, L"a"), std::make_unique<Variable>(Position{2, 10}, L"a0")
    ));
    instructions.push_back(std::make_unique<ReturnStatement>(Position{3, 1}, makeLiteral({3, 10}, 4)));
    instructions.push_back(
        std::make_unique<AssignmentStatement>(Position{4, 1}, Assignable({4, 1}, L"a"), makeLiteral({4, 10}, 5))
    );
    program.functions.emplace(
        FunctionIdentification(L"f", {{INT}, {INT}}), std::make_unique<FunctionDeclaration>(
                                                          Position{1, 1}, L"<test>",
                                                          std::vector<VariableDeclaration>{
                                                              VariableDeclaration({1, 10}, {INT}, L"a0", false),
                                                              VariableDeclaration({1, 20}, {INT}, L"a", true),
                                                          },
                                                          Type{INT}, std::move(instructions)
                                                      )
    );

    instructions.clear();
    instructions.push_back(std::make_unique<VariableDeclStatement>(
        Position{6, 1}, VariableDeclaration({6, 1}, {INT}, L"a", true), makeLiteral({6, 10}, 2)
    ));
    arguments.clear();
    arguments.push_back(makeLiteral({7, 20}, 3));
    arguments.push_back(std::make_unique<Variable>(Position{7, 30}, L"a"));
    instructions.push_back(std::make_unique<VariableDeclStatement>(
        Position{7, 1}, VariableDeclaration({7, 1}, {INT}, L"b", false),
        std::make_unique<FunctionCall>(Position{7, 10}, L"f", std::move(arguments))
    ));
    arguments.clear();
    arguments.push_back(std::make_unique<Variable>(Position{8, 10}, L"a"));
    instructions.push_back(std::make_unique<FunctionCallInstruction>(
        Position{8, 1}, FunctionCall({8, 1}, L"println", std::move(arguments))
    ));
    arguments.clear();
    arguments.push_back(std::make_unique<Variable>(Position{9, 10}, L"b"));
    instructions.push_back(std::make_unique<FunctionCallInstruction>(
        Position{9, 1}, FunctionCall({9, 1}, L"println", std::move(arguments))
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
    REQUIRE(output.str() == L"inside function\n3\n4\n");
}

TEST_CASE("FunctionCall runtime variant overload resolution", "[Interpreter]")
{
    Program program({1, 1});
    program.variants.emplace(
        L"V", VariantDeclaration(
                  {1, 1}, L"<test>",
                  {
                      Field({1, 10}, {INT}, L"a"),
                      Field({1, 10}, {STR}, L"b"),
                  }
              )
    );

    std::vector<std::unique_ptr<Instruction>> instructions;
    std::vector<std::unique_ptr<Expression>> arguments;
    arguments.push_back(makeLiteral({7, 20}, L"f(int)"));
    instructions.push_back(std::make_unique<FunctionCallInstruction>(
        Position{1, 100}, FunctionCall({1, 110}, L"println", std::move(arguments))
    ));
    program.functions.emplace(
        FunctionIdentification(L"f", {{L"V"}, {INT}}), std::make_unique<FunctionDeclaration>(
                                                           Position{1, 1}, L"<test>",
                                                           std::vector<VariableDeclaration>{
                                                               VariableDeclaration({1, 10}, {L"V"}, L"a", false),
                                                               VariableDeclaration({1, 20}, {INT}, L"b", false),
                                                           },
                                                           std::nullopt, std::move(instructions)
                                                       )
    );
    instructions.clear();
    arguments.clear();
    arguments.push_back(makeLiteral({7, 20}, L"f(str)"));
    instructions.push_back(std::make_unique<FunctionCallInstruction>(
        Position{1, 100}, FunctionCall({1, 110}, L"println", std::move(arguments))
    ));
    program.functions.emplace(
        FunctionIdentification(L"f", {{L"V"}, {STR}}), std::make_unique<FunctionDeclaration>(
                                                           Position{1, 1}, L"<test>",
                                                           std::vector<VariableDeclaration>{
                                                               VariableDeclaration({1, 10}, {L"V"}, L"a", false),
                                                               VariableDeclaration({1, 20}, {STR}, L"b", false),
                                                           },
                                                           std::nullopt, std::move(instructions)
                                                       )
    );

    instructions.clear();
    instructions.push_back(std::make_unique<VariableDeclStatement>(
        Position{6, 1}, VariableDeclaration({6, 1}, {L"V"}, L"a", false), makeLiteral({6, 10}, L"abc")
    ));
    arguments.clear();
    arguments.push_back(std::make_unique<Variable>(Position{7, 10}, L"a"));
    arguments.push_back(std::make_unique<Variable>(Position{7, 20}, L"a"));
    instructions.push_back(
        std::make_unique<FunctionCallInstruction>(Position{7, 1}, FunctionCall({7, 1}, L"f", std::move(arguments)))
    );
    instructions.push_back(std::make_unique<VariableDeclStatement>(
        Position{8, 1}, VariableDeclaration({8, 1}, {L"V"}, L"b", false), makeLiteral({8, 10}, 22)
    ));
    arguments.clear();
    arguments.push_back(std::make_unique<Variable>(Position{9, 10}, L"a"));
    arguments.push_back(std::make_unique<Variable>(Position{9, 20}, L"b"));
    instructions.push_back(
        std::make_unique<FunctionCallInstruction>(Position{9, 1}, FunctionCall({9, 1}, L"f", std::move(arguments)))
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
    REQUIRE(output.str() == L"f(str)\nf(int)\n");
}

TEST_CASE("BuiltinFunctionDeclaration value returned", "[Interpreter]")
{
    Program program({1, 1});
    std::vector<std::unique_ptr<Instruction>> instructions;
    std::vector<std::unique_ptr<Expression>> arguments;
    arguments.push_back(makeLiteral({2, 20}, L"abc"));
    instructions.push_back(std::make_unique<VariableDeclStatement>(
        Position{2, 1}, VariableDeclaration({2, 1}, {INT}, L"a", false),
        std::make_unique<FunctionCall>(Position{2, 10}, L"len", std::move(arguments))
    ));
    arguments.clear();
    arguments.push_back(std::make_unique<Variable>(Position{3, 10}, L"a"));
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
    REQUIRE(output.str() == L"3");
}

TEST_CASE("StructExpression and struct field assignment", "[Interpreter]")
{
    Program program({1, 1});
    program.structs.emplace(
        L"S", StructDeclaration(
                  Position{1, 1}, L"<test>",
                  {
                      Field({1, 10}, {INT}, L"a"),
                      Field({1, 20}, {FLOAT}, L"b"),
                  }
              )
    );

    std::vector<std::unique_ptr<Instruction>> instructions;
    std::vector<std::unique_ptr<Expression>> arguments;
    arguments.push_back(makeLiteral(Position{2, 20}, 2));
    arguments.push_back(makeLiteral(Position{2, 30}, 3.5));
    instructions.push_back(std::make_unique<VariableDeclStatement>(
        Position{2, 1}, VariableDeclaration({2, 1}, {L"S"}, L"s", true),
        std::make_unique<StructExpression>(Position{2, 10}, std::move(arguments))
    ));
    instructions.push_back(std::make_unique<AssignmentStatement>(
        Position{3, 1}, Assignable({3, 1}, std::make_unique<Assignable>(Position{3, 1}, L"s"), L"a"),
        makeLiteral({3, 10}, 3)
    ));
    arguments.clear();
    arguments.push_back(
        std::make_unique<DotExpression>(Position{4, 10}, std::make_unique<Variable>(Position{4, 10}, L"s"), L"a")
    );
    instructions.push_back(std::make_unique<FunctionCallInstruction>(
        Position{4, 1}, FunctionCall({4, 1}, L"println", std::move(arguments))
    ));
    arguments.clear();
    arguments.push_back(
        std::make_unique<DotExpression>(Position{5, 10}, std::make_unique<Variable>(Position{5, 10}, L"s"), L"b")
    );
    instructions.push_back(std::make_unique<FunctionCallInstruction>(
        Position{5, 1}, FunctionCall({5, 1}, L"println", std::move(arguments))
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
    REQUIRE(output.str() == L"3\n3.5\n");
}

void addPrintlnOfVariant(
    std::vector<std::unique_ptr<Instruction>> &instructions, unsigned line, const std::wstring &variantName,
    const Type &type
)
{
    std::vector<SingleIfCase> cases;
    std::vector<std::unique_ptr<Instruction>> caseBody;
    std::vector<std::unique_ptr<Expression>> arguments;
    arguments.push_back(std::make_unique<Variable>(Position{line + 1, 10}, L"accessed"));
    caseBody.push_back(std::make_unique<FunctionCallInstruction>(
        Position{line + 1, 1}, FunctionCall({line + 1, 1}, L"println", std::move(arguments))
    ));
    cases.push_back(SingleIfCase(
        {line, 1},
        VariableDeclStatement(
            {line, 10}, VariableDeclaration({line, 10}, type, L"accessed", true),
            std::make_unique<Variable>(Position{line, 20}, variantName)
        ),
        std::move(caseBody)
    ));
    instructions.push_back(
        std::make_unique<IfStatement>(Position{line, 1}, std::move(cases), std::vector<std::unique_ptr<Instruction>>{})
    );
}

TEST_CASE("variant access and assignment", "[Interpreter]")
{
    Program program({1, 1});
    program.variants.emplace(
        L"V", VariantDeclaration(
                  Position{1, 1}, L"<test>",
                  {
                      Field({1, 10}, {INT}, L"a"),
                      Field({1, 20}, {STR}, L"b"),
                  }
              )
    );

    std::vector<std::unique_ptr<Instruction>> instructions;
    instructions.push_back(std::make_unique<VariableDeclStatement>(
        Position{3, 1}, VariableDeclaration({3, 1}, {L"V"}, L"v", true), makeLiteral({3, 10}, 2)
    ));
    std::vector<SingleIfCase> cases;
    std::vector<std::unique_ptr<Instruction>> caseBody;
    std::vector<std::unique_ptr<Expression>> arguments;
    arguments.push_back(std::make_unique<Variable>(Position{5, 10}, L"a"));
    caseBody.push_back(std::make_unique<FunctionCallInstruction>(
        Position{5, 1}, FunctionCall({5, 1}, L"println", std::move(arguments))
    ));
    caseBody.push_back(
        std::make_unique<AssignmentStatement>(Position{6, 1}, Assignable({6, 1}, L"a"), makeLiteral({6, 10}, 3))
    );
    cases.push_back(SingleIfCase(
        {4, 1},
        VariableDeclStatement(
            {4, 10}, VariableDeclaration({4, 10}, {INT}, L"a", true), std::make_unique<Variable>(Position{4, 20}, L"v")
        ),
        std::move(caseBody)
    ));
    instructions.push_back(
        std::make_unique<IfStatement>(Position{4, 1}, std::move(cases), std::vector<std::unique_ptr<Instruction>>{})
    );
    addPrintlnOfVariant(instructions, 7, L"v", {INT});
    addPrintlnOfVariant(instructions, 9, L"v", {STR});
    instructions.push_back(
        std::make_unique<AssignmentStatement>(Position{11, 1}, Assignable({11, 1}, L"v"), makeLiteral({11, 10}, L"abc"))
    );
    addPrintlnOfVariant(instructions, 12, L"v", {INT});
    addPrintlnOfVariant(instructions, 14, L"v", {STR});
    instructions.push_back(std::make_unique<AssignmentStatement>(
        Position{16, 1}, Assignable({16, 1}, std::make_unique<Assignable>(Position{16, 1}, L"v"), L"a"),
        makeLiteral({16, 10}, 100)
    ));
    addPrintlnOfVariant(instructions, 17, L"v", {INT});
    addPrintlnOfVariant(instructions, 19, L"v", {STR});

    program.functions.emplace(
        FunctionIdentification(L"main", {}),
        std::make_unique<FunctionDeclaration>(
            Position{1, 1}, L"<test>", std::vector<VariableDeclaration>{}, std::nullopt, std::move(instructions)
        )
    );
    std::wstringstream input, output;
    Interpreter interpreter(L"<test>", {}, input, output, parseFromStream);
    interpreter.visit(program);
    REQUIRE(output.str() == L"2\n3\nabc\n100\n");
}

template <typename OperatorType>
void checkBinaryOperator(
    std::variant<std::wstring, int32_t, double, bool> left, std::variant<std::wstring, int32_t, double, bool> right,
    const std::wstring &expectedOutput
)
{
    Program program({1, 1});
    std::vector<std::unique_ptr<Instruction>> instructions;
    std::vector<std::unique_ptr<Expression>> arguments;
    arguments.push_back(
        std::make_unique<OperatorType>(Position{2, 10}, makeLiteral({2, 10}, left), makeLiteral({2, 20}, right))
    );
    instructions.push_back(
        std::make_unique<FunctionCallInstruction>(Position{2, 1}, FunctionCall({2, 1}, L"print", std::move(arguments)))
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

TEST_CASE("boolean binary operators", "[Interpreter]")
{
    checkBinaryOperator<OrExpression>(true, true, L"true");
    checkBinaryOperator<OrExpression>(true, false, L"true");
    checkBinaryOperator<OrExpression>(false, true, L"true");
    checkBinaryOperator<OrExpression>(false, false, L"false");

    checkBinaryOperator<XorExpression>(true, true, L"false");
    checkBinaryOperator<XorExpression>(true, false, L"true");
    checkBinaryOperator<XorExpression>(false, true, L"true");
    checkBinaryOperator<XorExpression>(false, false, L"false");

    checkBinaryOperator<AndExpression>(true, true, L"true");
    checkBinaryOperator<AndExpression>(true, false, L"false");
    checkBinaryOperator<AndExpression>(false, true, L"false");
    checkBinaryOperator<AndExpression>(false, false, L"false");
}

TEST_CASE("equality binary operators - builtin arguments", "[Interpreter]")
{
    checkBinaryOperator<EqualExpression>(1, 2, L"false");
    checkBinaryOperator<EqualExpression>(2, L"2", L"true");
    checkBinaryOperator<EqualExpression>(2, 2.5, L"false");
    checkBinaryOperator<EqualExpression>(L"false", 0, L"false");
    checkBinaryOperator<EqualExpression>(false, false, L"true");

    checkBinaryOperator<NotEqualExpression>(1, 2, L"true");
    checkBinaryOperator<NotEqualExpression>(2, L"2", L"false");
    checkBinaryOperator<NotEqualExpression>(2, 2.5, L"true");
    checkBinaryOperator<NotEqualExpression>(L"false", 0, L"true");
    checkBinaryOperator<NotEqualExpression>(false, false, L"false");

    checkBinaryOperator<IdenticalExpression>(1, 2, L"false");
    checkBinaryOperator<IdenticalExpression>(2, L"2", L"false");
    checkBinaryOperator<IdenticalExpression>(2, 2.5, L"false");
    checkBinaryOperator<IdenticalExpression>(L"false", 0, L"false");
    checkBinaryOperator<IdenticalExpression>(false, false, L"true");
    checkBinaryOperator<IdenticalExpression>(2.3, 2.3, L"true");

    checkBinaryOperator<NotIdenticalExpression>(1, 2, L"true");
    checkBinaryOperator<NotIdenticalExpression>(2, L"2", L"true");
    checkBinaryOperator<NotIdenticalExpression>(2, 2.5, L"true");
    checkBinaryOperator<NotIdenticalExpression>(L"false", 0, L"true");
    checkBinaryOperator<NotIdenticalExpression>(false, false, L"false");
    checkBinaryOperator<NotIdenticalExpression>(2.3, 2.3, L"false");
}

template <typename EqualityOperator>
void checkEqualityOperator(
    std::unique_ptr<Expression> left, std::unique_ptr<Expression> right, const std::wstring &expectedOutput
)
{
    Program program({1, 1});
    program.structs.emplace(
        L"S", StructDeclaration(
                  Position{1, 1}, L"<test>",
                  {
                      Field({1, 10}, {INT}, L"a"),
                      Field({1, 20}, {FLOAT}, L"b"),
                  }
              )
    );
    program.variants.emplace(
        L"V", VariantDeclaration(
                  Position{1, 1}, L"<test>",
                  {
                      Field({1, 10}, {INT}, L"a"),
                      Field({1, 20}, {STR}, L"b"),
                      Field({1, 30}, {L"S"}, L"c"),
                  }
              )
    );
    program.variants.emplace(
        L"V2", VariantDeclaration(
                   Position{1, 1}, L"<test>",
                   {
                       Field({1, 10}, {L"V"}, L"a"),
                       Field({1, 20}, {STR}, L"b"),
                       Field({1, 30}, {L"S"}, L"c"),
                   }
               )
    );
    std::vector<std::unique_ptr<Instruction>> instructions;
    std::vector<std::unique_ptr<Expression>> arguments;
    arguments.push_back(std::make_unique<EqualityOperator>(Position{2, 10}, std::move(left), std::move(right)));
    instructions.push_back(
        std::make_unique<FunctionCallInstruction>(Position{2, 1}, FunctionCall({2, 1}, L"print", std::move(arguments)))
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

std::unique_ptr<FunctionCall> prepareStruct(
    const std::wstring &name, const std::vector<std::variant<std::wstring, int32_t, double, bool>> &values
)
{
    std::vector<std::unique_ptr<Expression>> arguments, structArguments;
    for(const auto &value: values)
        structArguments.push_back(makeLiteral(Position{3, 10}, value));
    arguments.push_back(std::make_unique<StructExpression>(Position{3, 10}, std::move(structArguments)));
    return std::make_unique<FunctionCall>(Position{3, 1}, name, std::move(arguments));
}

std::unique_ptr<FunctionCall> prepareVariant(
    const std::wstring &name, std::variant<std::wstring, int32_t, double, bool> value
)
{
    std::vector<std::unique_ptr<Expression>> arguments;
    arguments.push_back(makeLiteral({3, 10}, value));
    return std::make_unique<FunctionCall>(Position{3, 1}, name, std::move(arguments));
}

std::unique_ptr<FunctionCall> prepareVariant(const std::wstring &name, std::unique_ptr<Expression> value)
{
    std::vector<std::unique_ptr<Expression>> arguments;
    arguments.push_back(std::move(value));
    return std::make_unique<FunctionCall>(Position{3, 1}, name, std::move(arguments));
}

TEST_CASE("equality binary operators - complex types", "[Interpreter]")
{
    checkEqualityOperator<EqualExpression>(prepareStruct(L"S", {2, 2.4}), prepareStruct(L"S", {2, 2.4}), L"true");
    checkEqualityOperator<EqualExpression>(prepareStruct(L"S", {1, 2.4}), prepareStruct(L"S", {2, 2.4}), L"false");
    checkEqualityOperator<EqualExpression>(prepareVariant(L"V", 2), prepareVariant(L"V", L"2"), L"true");
    checkEqualityOperator<EqualExpression>(prepareVariant(L"V", L"2.6"), prepareVariant(L"V", 3), L"false");
    checkEqualityOperator<EqualExpression>(
        prepareVariant(L"V2", prepareVariant(L"V", 2)), prepareVariant(L"V2", L"2"), L"true"
    );
    checkEqualityOperator<EqualExpression>(
        prepareVariant(L"V2", prepareStruct(L"S", {2, 2.4})), prepareVariant(L"V2", prepareStruct(L"S", {2, 2.4})),
        L"true"
    );

    checkEqualityOperator<NotEqualExpression>(prepareStruct(L"S", {2, 2.4}), prepareStruct(L"S", {2, 1.5}), L"true");
    checkEqualityOperator<NotEqualExpression>(prepareVariant(L"V", 2), prepareVariant(L"V", 2), L"false");
    checkEqualityOperator<NotEqualExpression>(prepareVariant(L"V", 2), prepareVariant(L"V", 3), L"true");
    checkEqualityOperator<NotEqualExpression>(
        prepareVariant(L"V2", prepareStruct(L"S", {2, 2.4})), prepareVariant(L"V2", L"2"), L"true"
    );

    checkEqualityOperator<IdenticalExpression>(prepareStruct(L"S", {2, 2.4}), prepareStruct(L"S", {2, 2.4}), L"true");
    checkEqualityOperator<IdenticalExpression>(prepareStruct(L"S", {1, 2.4}), prepareStruct(L"S", {2, 2.4}), L"false");
    checkEqualityOperator<IdenticalExpression>(prepareVariant(L"V", 2), prepareVariant(L"V", L"2"), L"false");
    checkEqualityOperator<IdenticalExpression>(prepareVariant(L"V", L"2.6"), prepareVariant(L"V", 3), L"false");
    checkEqualityOperator<IdenticalExpression>(
        prepareVariant(L"V2", prepareVariant(L"V", 2)), prepareVariant(L"V2", L"2"), L"false"
    );
    checkEqualityOperator<IdenticalExpression>(
        prepareVariant(L"V2", prepareStruct(L"S", {2, 2.4})), prepareVariant(L"V2", prepareStruct(L"S", {2, 2.4})),
        L"true"
    );

    checkEqualityOperator<NotIdenticalExpression>(
        prepareStruct(L"S", {2, 2.4}), prepareStruct(L"S", {2, 1.5}), L"true"
    );
    checkEqualityOperator<NotIdenticalExpression>(prepareVariant(L"V", 2), prepareVariant(L"V", 2), L"false");
    checkEqualityOperator<NotIdenticalExpression>(prepareVariant(L"V", 2), prepareVariant(L"V", 3), L"true");
    checkEqualityOperator<NotIdenticalExpression>(
        prepareVariant(L"V2", prepareStruct(L"S", {2, 2.4})), prepareVariant(L"V2", L"2"), L"true"
    );
}

TEST_CASE("comparison binary operators", "[Interpreter]")
{
    checkBinaryOperator<GreaterExpression>(1, 1.4, L"false");
    checkBinaryOperator<GreaterExpression>(2, 2, L"false");
    checkBinaryOperator<GreaterExpression>(2.1, 2, L"true");
    checkBinaryOperator<GreaterExpression>(0, -2, L"true");

    checkBinaryOperator<GreaterEqualExpression>(1, 1.4, L"false");
    checkBinaryOperator<GreaterEqualExpression>(2, 2, L"true");
    checkBinaryOperator<GreaterEqualExpression>(2.1, 2, L"true");
    checkBinaryOperator<GreaterEqualExpression>(0, -2, L"true");

    checkBinaryOperator<LesserExpression>(1, 1.4, L"true");
    checkBinaryOperator<LesserExpression>(2, 2, L"false");
    checkBinaryOperator<LesserExpression>(2.1, 2, L"false");
    checkBinaryOperator<LesserExpression>(0, -2, L"false");

    checkBinaryOperator<LesserEqualExpression>(1, 1.4, L"true");
    checkBinaryOperator<LesserEqualExpression>(2, 2, L"true");
    checkBinaryOperator<LesserEqualExpression>(2.1, 2, L"false");
    checkBinaryOperator<LesserEqualExpression>(0, -2, L"false");
}

TEST_CASE("string binary operators", "[Interpreter]")
{
    checkBinaryOperator<ConcatExpression>(1, 1.4, L"11.4");
    checkBinaryOperator<ConcatExpression>(L"abc", L"", L"abc");
    checkBinaryOperator<ConcatExpression>(true, false, L"truefalse");

    checkBinaryOperator<StringMultiplyExpression>(L"abc", 1.4, L"abc");
    checkBinaryOperator<StringMultiplyExpression>(L"abc", L"3", L"abcabcabc");
    checkBinaryOperator<StringMultiplyExpression>(L"abc", 0, L"");
    checkBinaryOperator<StringMultiplyExpression>(L"", 20, L"");
    REQUIRE_THROWS_AS(checkBinaryOperator<StringMultiplyExpression>(L"abc", -1, L""), OperatorArgumentError);

    checkBinaryOperator<SubscriptExpression>(L"abc", 1.4, L"b");
    checkBinaryOperator<SubscriptExpression>(L"abc", L"0", L"a");
    checkBinaryOperator<SubscriptExpression>(true, 3, L"e");
    REQUIRE_THROWS_AS(checkBinaryOperator<SubscriptExpression>(L"abc", -1, L""), OperatorArgumentError);
    REQUIRE_THROWS_AS(checkBinaryOperator<SubscriptExpression>(L"abc", 3, L""), OperatorArgumentError);
}

namespace {
const int32_t minInt = std::numeric_limits<int32_t>::min(), maxInt = std::numeric_limits<int32_t>::max();
}

TEST_CASE("additive binary operators", "[Interpreter]")
{
    checkBinaryOperator<PlusExpression>(1, 1.4, L"2.4");
    checkBinaryOperator<PlusExpression>(1, -1, L"0");
    checkBinaryOperator<PlusExpression>(0, -1.4, L"-1.4");
    checkBinaryOperator<PlusExpression>(0, maxInt, std::format(L"{}", maxInt));
    checkBinaryOperator<PlusExpression>(-3, minInt + 3, std::format(L"{}", minInt));
    checkBinaryOperator<PlusExpression>(1e100, 1e100, std::format(L"{}", 2e100));
    REQUIRE_THROWS_AS(checkBinaryOperator<PlusExpression>(1, maxInt, L""), IntegerRangeError);
    REQUIRE_THROWS_AS(checkBinaryOperator<PlusExpression>(minInt, -1, L""), IntegerRangeError);

    checkBinaryOperator<MinusExpression>(1, 1.4, std::format(L"{}", 1 - 1.4));
    checkBinaryOperator<MinusExpression>(1, -1, L"2");
    checkBinaryOperator<MinusExpression>(0, -1.4, L"1.4");
    checkBinaryOperator<MinusExpression>(0, maxInt, std::format(L"-{}", maxInt));
    checkBinaryOperator<MinusExpression>(1, minInt + 2, std::format(L"{}", maxInt));
    checkBinaryOperator<MinusExpression>(-1, minInt, std::format(L"{}", maxInt));
    checkBinaryOperator<MinusExpression>(1e100, 1e101, std::format(L"{}", 1e100 - 1e101));
    REQUIRE_THROWS_AS(checkBinaryOperator<MinusExpression>(1, -maxInt, L""), IntegerRangeError);
    REQUIRE_THROWS_AS(checkBinaryOperator<MinusExpression>(minInt, 1, L""), IntegerRangeError);
    REQUIRE_THROWS_AS(checkBinaryOperator<MinusExpression>(0, minInt, L""), IntegerRangeError);
    REQUIRE_THROWS_AS(checkBinaryOperator<MinusExpression>(minInt, minInt, L""), IntegerRangeError);
}

TEST_CASE("multiplicative binary operators", "[Interpreter]")
{
    checkBinaryOperator<MultiplyExpression>(1, 1.4, L"1.4");
    checkBinaryOperator<MultiplyExpression>(-2, 5, L"-10");
    checkBinaryOperator<MultiplyExpression>(L"1e50", L"-10", std::format(L"{}", 1e50 * -10));
    checkBinaryOperator<MultiplyExpression>(1e150, 0, L"0");
    checkBinaryOperator<MultiplyExpression>(maxInt / 2, 2, std::format(L"{}", maxInt / 2 * 2));
    checkBinaryOperator<MultiplyExpression>(maxInt / 2, -2, std::format(L"{}", maxInt / 2 * -2));
    REQUIRE_THROWS_AS(checkBinaryOperator<MultiplyExpression>(maxInt, 2, L""), IntegerRangeError);
    REQUIRE_THROWS_AS(checkBinaryOperator<MultiplyExpression>(maxInt, -2, L""), IntegerRangeError);
    REQUIRE_THROWS_AS(checkBinaryOperator<MultiplyExpression>(2, minInt, L""), IntegerRangeError);
    REQUIRE_THROWS_AS(checkBinaryOperator<MultiplyExpression>(minInt, -1, L""), IntegerRangeError);

    checkBinaryOperator<DivideExpression>(1, 1.4, std::format(L"{}", 1.0 / 1.4));
    checkBinaryOperator<DivideExpression>(-2, 5, std::format(L"{}", -2.0 / 5.0));
    checkBinaryOperator<DivideExpression>(L"1e50", L"-10", std::format(L"{}", 1e50 / -10));
    checkBinaryOperator<DivideExpression>(0, 1e150, L"0");
    REQUIRE_THROWS_AS(checkBinaryOperator<DivideExpression>(231, L"0", L""), ZeroDivisionError);
    REQUIRE_THROWS_AS(checkBinaryOperator<DivideExpression>(0, 0, L""), ZeroDivisionError);

    checkBinaryOperator<FloorDivideExpression>(1, 1.4, L"1");
    checkBinaryOperator<FloorDivideExpression>(-22, 5, L"-5");
    checkBinaryOperator<FloorDivideExpression>(0, -3.5, L"0");
    checkBinaryOperator<FloorDivideExpression>(-22, -5, L"4");
    REQUIRE_THROWS_AS(checkBinaryOperator<FloorDivideExpression>(231, L"0", L""), ZeroDivisionError);
    REQUIRE_THROWS_AS(checkBinaryOperator<FloorDivideExpression>(0, 0.4, L""), ZeroDivisionError);
    REQUIRE_THROWS_AS(checkBinaryOperator<FloorDivideExpression>(minInt, -1, L""), IntegerRangeError);

    checkBinaryOperator<ModuloExpression>(1, 1.4, L"0");
    checkBinaryOperator<ModuloExpression>(-22, 5, L"3");
    checkBinaryOperator<ModuloExpression>(0, -3.5, L"0");
    checkBinaryOperator<ModuloExpression>(minInt, -1, L"0");
    REQUIRE_THROWS_AS(checkBinaryOperator<ModuloExpression>(231, L"0", L""), ZeroDivisionError);
    REQUIRE_THROWS_AS(checkBinaryOperator<ModuloExpression>(0, 0.4, L""), ZeroDivisionError);
}

TEST_CASE("exponent operator", "[Interpreter]")
{
    checkBinaryOperator<ExponentExpression>(1, 20, L"1");
    checkBinaryOperator<ExponentExpression>(2, 5, L"32");
    checkBinaryOperator<ExponentExpression>(L"1e5", L"-10", std::format(L"{}", 1e-50));
    checkBinaryOperator<ExponentExpression>(L"1e5", L"10", std::format(L"{}", 1e50));
    checkBinaryOperator<ExponentExpression>(1e150, 0, L"1");
    checkBinaryOperator<ExponentExpression>(0, 1e150, L"0");
    REQUIRE_THROWS_AS(checkBinaryOperator<ExponentExpression>(-100, 2, L""), OperatorArgumentError);
    REQUIRE_THROWS_AS(checkBinaryOperator<ExponentExpression>(-0.01, 20, L""), OperatorArgumentError);
}

template <typename OperatorType>
void checkUnaryOperator(std::variant<std::wstring, int32_t, double, bool> value, const std::wstring &expectedOutput)
{
    Program program({1, 1});
    std::vector<std::unique_ptr<Instruction>> instructions;
    std::vector<std::unique_ptr<Expression>> arguments;
    arguments.push_back(std::make_unique<OperatorType>(Position{2, 10}, makeLiteral({2, 10}, value)));
    instructions.push_back(
        std::make_unique<FunctionCallInstruction>(Position{2, 1}, FunctionCall({2, 1}, L"print", std::move(arguments)))
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

TEST_CASE("unary operators", "[Interpreter]")
{
    checkUnaryOperator<UnaryMinusExpression>(1, L"-1");
    checkUnaryOperator<UnaryMinusExpression>(10.54, L"-10.54");
    checkUnaryOperator<UnaryMinusExpression>(-21, L"21");
    checkUnaryOperator<UnaryMinusExpression>(maxInt, std::format(L"{}", -maxInt));
    checkUnaryOperator<UnaryMinusExpression>(
        static_cast<double>(minInt), std::format(L"{}", static_cast<double>(maxInt) + 1)
    );
    REQUIRE_THROWS_AS(checkUnaryOperator<UnaryMinusExpression>(minInt, L""), IntegerRangeError);

    checkUnaryOperator<NotExpression>(true, L"false");
    checkUnaryOperator<NotExpression>(false, L"true");
    checkUnaryOperator<NotExpression>(2, L"false");
    checkUnaryOperator<NotExpression>(0.0, L"true");
}

void checkIsOperator(std::unique_ptr<Expression> left, Type right, const std::wstring &expectedOutput)
{
    Program program({1, 1});
    program.structs.emplace(
        L"S", StructDeclaration(
                  Position{1, 1}, L"<test>",
                  {
                      Field({1, 10}, {INT}, L"a"),
                      Field({1, 20}, {FLOAT}, L"b"),
                  }
              )
    );
    program.variants.emplace(
        L"V", VariantDeclaration(
                  Position{1, 1}, L"<test>",
                  {
                      Field({1, 10}, {INT}, L"a"),
                      Field({1, 20}, {STR}, L"b"),
                  }
              )
    );
    std::vector<std::unique_ptr<Instruction>> instructions;
    std::vector<std::unique_ptr<Expression>> arguments;
    arguments.push_back(std::make_unique<IsExpression>(Position{2, 10}, std::move(left), right));
    instructions.push_back(
        std::make_unique<FunctionCallInstruction>(Position{2, 1}, FunctionCall({2, 1}, L"print", std::move(arguments)))
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

TEST_CASE("IsExpression", "[Interpreter]")
{
    checkIsOperator(makeLiteral({3, 1}, 2), {INT}, L"true");
    checkIsOperator(makeLiteral({3, 1}, 2), {FLOAT}, L"false");
    checkIsOperator(makeLiteral({3, 1}, L"true"), {BOOL}, L"false");
    checkIsOperator(makeLiteral({3, 1}, false), {BOOL}, L"true");
    for(auto [type, output]:
        std::vector<std::pair<Type, std::wstring>>{{{L"S"}, L"true"}, {{INT}, L"false"}, {{FLOAT}, L"false"}})
    {
        std::vector<std::unique_ptr<Expression>> arguments, structArguments;
        structArguments.push_back(makeLiteral(Position{3, 10}, 2));
        structArguments.push_back(makeLiteral(Position{3, 10}, 3.5));
        arguments.push_back(std::make_unique<StructExpression>(Position{3, 10}, std::move(structArguments)));
        checkIsOperator(std::make_unique<FunctionCall>(Position{3, 1}, L"S", std::move(arguments)), type, output);
    }
    for(auto [type, output]: std::vector<std::pair<Type, std::wstring>>{
            {{L"V"}, L"true"}, {{INT}, L"true"}, {{STR}, L"false"}, {{FLOAT}, L"false"}
        })
    {
        std::vector<std::unique_ptr<Expression>> arguments;
        arguments.push_back(makeLiteral(Position{3, 10}, 2));
        checkIsOperator(std::make_unique<FunctionCall>(Position{3, 1}, L"V", std::move(arguments)), type, output);
    }
}

TEST_CASE("WhileStatement, IfStatement, ContinueStatement", "[Interpreter]")
{
    Program program({1, 1});
    std::vector<std::unique_ptr<Instruction>> instructions;
    instructions.push_back(std::make_unique<VariableDeclStatement>(
        Position{2, 1}, VariableDeclaration({2, 1}, {INT}, L"a", true), makeLiteral({2, 10}, 0)
    ));
    instructions.push_back(std::make_unique<VariableDeclStatement>(
        Position{2, 20}, VariableDeclaration({2, 20}, {INT}, L"i", true), makeLiteral({2, 30}, 0)
    ));

    std::vector<std::unique_ptr<Instruction>> loopBody;
    loopBody.push_back(std::make_unique<AssignmentStatement>(
        Position{2, 100}, Assignable({2, 100}, L"a"),
        std::make_unique<PlusExpression>(
            Position{2, 110}, std::make_unique<Variable>(Position{2, 110}, L"a"), makeLiteral({2, 120}, 1)
        )
    ));
    std::vector<SingleIfCase> cases;
    std::vector<std::unique_ptr<Instruction>> caseBody;
    caseBody.push_back(std::make_unique<ContinueStatement>(Position{5, 1}));
    cases.push_back(SingleIfCase(
        {4, 1},
        std::make_unique<EqualExpression>(
            Position{4, 10}, std::make_unique<Variable>(Position{4, 10}, L"a"), makeLiteral({4, 20}, 2)
        ),
        std::move(caseBody)
    ));
    loopBody.push_back(
        std::make_unique<IfStatement>(Position{4, 1}, std::move(cases), std::vector<std::unique_ptr<Instruction>>{})
    );
    loopBody.push_back(std::make_unique<AssignmentStatement>(
        Position{6, 1}, Assignable({6, 1}, L"i"),
        std::make_unique<PlusExpression>(
            Position{6, 10}, std::make_unique<Variable>(Position{6, 10}, L"i"), makeLiteral({6, 20}, 1)
        )
    ));
    instructions.push_back(std::make_unique<WhileStatement>(
        Position{3, 1},
        std::make_unique<LesserExpression>(
            Position{3, 10}, std::make_unique<Variable>(Position{3, 10}, L"a"), makeLiteral({3, 20}, 5)
        ),
        std::move(loopBody)
    ));
    std::vector<std::unique_ptr<Expression>> arguments;
    arguments.push_back(std::make_unique<Variable>(Position{8, 10}, L"a"));
    instructions.push_back(std::make_unique<FunctionCallInstruction>(
        Position{8, 1}, FunctionCall({8, 1}, L"println", std::move(arguments))
    ));
    arguments.clear();
    arguments.push_back(std::make_unique<Variable>(Position{9, 10}, L"i"));
    instructions.push_back(std::make_unique<FunctionCallInstruction>(
        Position{9, 1}, FunctionCall({9, 1}, L"println", std::move(arguments))
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
    REQUIRE(output.str() == L"5\n4\n");
}

TEST_CASE("DoWhileStatement, IfStatement, BreakStatement", "[Interpreter]")
{
    Program program({1, 1});
    std::vector<std::unique_ptr<Instruction>> instructions;
    instructions.push_back(std::make_unique<VariableDeclStatement>(
        Position{2, 1}, VariableDeclaration({2, 1}, {INT}, L"a", true), makeLiteral({2, 10}, 0)
    ));
    instructions.push_back(std::make_unique<VariableDeclStatement>(
        Position{2, 20}, VariableDeclaration({2, 20}, {INT}, L"i", true), makeLiteral({2, 30}, 0)
    ));

    std::vector<std::unique_ptr<Instruction>> loopBody;
    loopBody.push_back(std::make_unique<AssignmentStatement>(
        Position{2, 100}, Assignable({2, 100}, L"a"),
        std::make_unique<PlusExpression>(
            Position{2, 110}, std::make_unique<Variable>(Position{2, 110}, L"a"), makeLiteral({2, 120}, 1)
        )
    ));
    std::vector<SingleIfCase> cases;
    std::vector<std::unique_ptr<Instruction>> caseBody;
    caseBody.push_back(std::make_unique<BreakStatement>(Position{5, 1}));
    cases.push_back(SingleIfCase(
        {4, 1},
        std::make_unique<EqualExpression>(
            Position{4, 10}, std::make_unique<Variable>(Position{4, 10}, L"a"), makeLiteral({4, 20}, 2)
        ),
        std::move(caseBody)
    ));
    loopBody.push_back(
        std::make_unique<IfStatement>(Position{4, 1}, std::move(cases), std::vector<std::unique_ptr<Instruction>>{})
    );
    loopBody.push_back(std::make_unique<AssignmentStatement>(
        Position{6, 1}, Assignable({6, 1}, L"i"),
        std::make_unique<PlusExpression>(
            Position{6, 10}, std::make_unique<Variable>(Position{6, 10}, L"i"), makeLiteral({6, 20}, 1)
        )
    ));
    instructions.push_back(std::make_unique<DoWhileStatement>(
        Position{3, 1},
        std::make_unique<LesserExpression>(
            Position{3, 10}, std::make_unique<Variable>(Position{3, 10}, L"a"), makeLiteral({3, 20}, 5)
        ),
        std::move(loopBody)
    ));
    std::vector<std::unique_ptr<Expression>> arguments;
    arguments.push_back(std::make_unique<Variable>(Position{8, 10}, L"a"));
    instructions.push_back(std::make_unique<FunctionCallInstruction>(
        Position{8, 1}, FunctionCall({8, 1}, L"println", std::move(arguments))
    ));
    arguments.clear();
    arguments.push_back(std::make_unique<Variable>(Position{9, 10}, L"i"));
    instructions.push_back(std::make_unique<FunctionCallInstruction>(
        Position{9, 1}, FunctionCall({9, 1}, L"println", std::move(arguments))
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
    REQUIRE(output.str() == L"2\n1\n");
}
