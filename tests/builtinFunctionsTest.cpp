#include "builtinFunctions.hpp"

#include "runtimeExceptions.hpp"

#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <sstream>

using enum Type::Builtin;

TEST_CASE("noArguments", "[builtinFunctions]")
{
    std::vector<std::wstring> arguments = {};
    BuiltinFunction builtin = builtinNoArguments(arguments);
    std::vector<std::variant<Object, std::reference_wrapper<Object>>> args = {};
    REQUIRE(builtin.second.body(Position{1, 1}, L"<test>", args) == Object{{INT}, 0});

    arguments = {L"arg1", L"arg2"};
    builtin = builtinNoArguments(arguments);
    REQUIRE(builtin.second.body(Position{1, 1}, L"<test>", args) == Object{{INT}, 2});
}

TEST_CASE("argument", "[builtinFunctions]")
{
    std::vector<std::wstring> arguments = {L"arg1", L"arg2"};
    BuiltinFunction builtin = builtinArgument(arguments);
    Object arg{{INT}, 0};
    std::vector<std::variant<Object, std::reference_wrapper<Object>>> args = {arg};
    REQUIRE(builtin.second.body(Position{1, 1}, L"<test>", args) == Object{{STR}, L"arg1"});

    arg.value = 1;
    REQUIRE(builtin.second.body(Position{1, 1}, L"<test>", args) == Object{{STR}, L"arg2"});

    arg.value = -1;
    REQUIRE_THROWS_AS(builtin.second.body(Position{1, 1}, L"<test>", args), BuiltinFunctionArgumentError);

    arg.value = 2;
    REQUIRE_THROWS_AS(builtin.second.body(Position{1, 1}, L"<test>", args), BuiltinFunctionArgumentError);
}

TEST_CASE("print", "[builtinFunctions]")
{
    std::wstringstream output;
    BuiltinFunction builtin = builtinPrint(output);
    Object arg{{STR}, L"message"};
    std::vector<std::variant<Object, std::reference_wrapper<Object>>> args = {arg};
    REQUIRE(builtin.second.body(Position{1, 1}, L"<test>", args) == std::nullopt);
    REQUIRE(output.str() == L"message");

    output.setstate(std::ios::badbit);
    REQUIRE_THROWS_AS(builtin.second.body(Position{1, 1}, L"<test>", args), StandardOutputError);
}

TEST_CASE("println", "[builtinFunctions]")
{
    std::wstringstream output;
    BuiltinFunction builtin = builtinPrintln(output);
    Object arg{{STR}, L"message"};
    std::vector<std::variant<Object, std::reference_wrapper<Object>>> args = {arg};
    REQUIRE(builtin.second.body(Position{1, 1}, L"<test>", args) == std::nullopt);
    REQUIRE(output.str() == L"message\n");

    output.setstate(std::ios::badbit);
    REQUIRE_THROWS_AS(builtin.second.body(Position{1, 1}, L"<test>", args), StandardOutputError);
}

TEST_CASE("input()", "[builtinFunctions]")
{
    std::wstringstream input(L"a line of data\nanother line\nend");
    BuiltinFunction builtin = builtinInputLine(input);
    std::vector<std::variant<Object, std::reference_wrapper<Object>>> args = {};
    REQUIRE(builtin.second.body(Position{1, 1}, L"<test>", args) == Object{{STR}, L"a line of data"});
    REQUIRE(builtin.second.body(Position{1, 1}, L"<test>", args) == Object{{STR}, L"another line"});
    REQUIRE(builtin.second.body(Position{1, 1}, L"<test>", args) == Object{{STR}, L"end"});
    REQUIRE(builtin.second.body(Position{1, 1}, L"<test>", args) == Object{{STR}, L""});

    input.setstate(std::ios::badbit);
    REQUIRE_THROWS_AS(builtin.second.body(Position{1, 1}, L"<test>", args), StandardInputError);
}

TEST_CASE("input(int)", "[builtinFunctions]")
{
    std::wstringstream input(L"a line of data\nanother line\nend");
    BuiltinFunction builtin = builtinInput(input);
    Object arg{{INT}, 0};
    std::vector<std::variant<Object, std::reference_wrapper<Object>>> args = {arg};
    REQUIRE(builtin.second.body(Position{1, 1}, L"<test>", args) == Object{{STR}, L""});

    arg.value = 5;
    REQUIRE(builtin.second.body(Position{1, 1}, L"<test>", args) == Object{{STR}, L"a lin"});

    arg.value = 15;
    REQUIRE(builtin.second.body(Position{1, 1}, L"<test>", args) == Object{{STR}, L"e of data\nanoth"});
    REQUIRE(builtin.second.body(Position{1, 1}, L"<test>", args) == Object{{STR}, L"er line\nend"});
    REQUIRE(builtin.second.body(Position{1, 1}, L"<test>", args) == Object{{STR}, L""});

    arg.value = -1;
    REQUIRE_THROWS_AS(builtin.second.body(Position{1, 1}, L"<test>", args), BuiltinFunctionArgumentError);

    arg.value = 5;
    input.setstate(std::ios::badbit);
    REQUIRE_THROWS_AS(builtin.second.body(Position{1, 1}, L"<test>", args), StandardInputError);
}

TEST_CASE("len", "[builtinFunctions]")
{
    Object arg{{STR}, L""};
    std::vector<std::variant<Object, std::reference_wrapper<Object>>> args = {arg};
    REQUIRE(builtinLen.second.body(Position{1, 1}, L"<test>", args) == Object{{INT}, 0});

    arg.value = L"value";
    REQUIRE(builtinLen.second.body(Position{1, 1}, L"<test>", args) == Object{{INT}, 5});
}

TEST_CASE("abs(float)", "[builtinFunctions]")
{
    Object arg{{FLOAT}, 3.4};
    std::vector<std::variant<Object, std::reference_wrapper<Object>>> args = {arg};
    REQUIRE(builtinAbsFloat.second.body(Position{1, 1}, L"<test>", args) == Object{{FLOAT}, 3.4});

    arg.value = -3.4;
    REQUIRE(builtinAbsFloat.second.body(Position{1, 1}, L"<test>", args) == Object{{FLOAT}, 3.4});

    arg.value = 0.0;
    REQUIRE(builtinAbsFloat.second.body(Position{1, 1}, L"<test>", args) == Object{{FLOAT}, 0.0});
}

TEST_CASE("abs(int)", "[builtinFunctions]")
{
    Object arg{{INT}, 5};
    std::vector<std::variant<Object, std::reference_wrapper<Object>>> args = {arg};
    REQUIRE(builtinAbsInt.second.body(Position{1, 1}, L"<test>", args) == Object{{INT}, 5});

    arg.value = -5;
    REQUIRE(builtinAbsInt.second.body(Position{1, 1}, L"<test>", args) == Object{{INT}, 5});

    arg.value = 0;
    REQUIRE(builtinAbsInt.second.body(Position{1, 1}, L"<test>", args) == Object{{INT}, 0});

    arg.value = std::numeric_limits<int32_t>::max();
    REQUIRE(
        builtinAbsInt.second.body(Position{1, 1}, L"<test>", args) == Object{{INT}, std::numeric_limits<int32_t>::max()}
    );

    arg.value = std::numeric_limits<int32_t>::min() + 1;
    REQUIRE(
        builtinAbsInt.second.body(Position{1, 1}, L"<test>", args) == Object{{INT}, std::numeric_limits<int32_t>::max()}
    );

    arg.value = std::numeric_limits<int32_t>::min();
    REQUIRE_THROWS_AS(builtinAbsInt.second.body(Position{1, 1}, L"<test>", args), IntegerRangeError);
}

TEST_CASE("max(float, float), min(float, float)", "[builtinFunctions]")
{
    Object arg1{{FLOAT}, 3.4};
    Object arg2{{FLOAT}, 7.8};
    std::vector<std::variant<Object, std::reference_wrapper<Object>>> args = {arg1, arg2};
    REQUIRE(builtinMaxFloat.second.body(Position{1, 1}, L"<test>", args) == Object{{FLOAT}, 7.8});
    REQUIRE(builtinMinFloat.second.body(Position{1, 1}, L"<test>", args) == Object{{FLOAT}, 3.4});

    arg1.value = -3.4;
    REQUIRE(builtinMaxFloat.second.body(Position{1, 1}, L"<test>", args) == Object{{FLOAT}, 7.8});
    REQUIRE(builtinMinFloat.second.body(Position{1, 1}, L"<test>", args) == Object{{FLOAT}, -3.4});

    arg2.value = -20.0;
    REQUIRE(builtinMaxFloat.second.body(Position{1, 1}, L"<test>", args) == Object{{FLOAT}, -3.4});
    REQUIRE(builtinMinFloat.second.body(Position{1, 1}, L"<test>", args) == Object{{FLOAT}, -20.0});
}

TEST_CASE("max(int, int), min(int, int)", "[builtinFunctions]")
{
    Object arg1{{INT}, 3};
    Object arg2{{INT}, 7};
    std::vector<std::variant<Object, std::reference_wrapper<Object>>> args = {arg1, arg2};
    REQUIRE(builtinMaxInt.second.body(Position{1, 1}, L"<test>", args) == Object{{INT}, 7});
    REQUIRE(builtinMinInt.second.body(Position{1, 1}, L"<test>", args) == Object{{INT}, 3});

    arg1.value = -3;
    REQUIRE(builtinMaxInt.second.body(Position{1, 1}, L"<test>", args) == Object{{INT}, 7});
    REQUIRE(builtinMinInt.second.body(Position{1, 1}, L"<test>", args) == Object{{INT}, -3});

    arg2.value = -20;
    REQUIRE(builtinMaxInt.second.body(Position{1, 1}, L"<test>", args) == Object{{INT}, -3});
    REQUIRE(builtinMinInt.second.body(Position{1, 1}, L"<test>", args) == Object{{INT}, -20});
}

TEST_CASE("prepareBuiltinFunctions", "[builtinFunctions]")
{
    std::wstringstream iostream;
    Program builtins = prepareBuiltinFunctions({2, 1}, {}, iostream, iostream);
    REQUIRE(builtins.getPosition() == Position{2, 1});
    REQUIRE(builtins.includes.size() == 0);
    REQUIRE(builtins.variants.size() == 0);
    REQUIRE(builtins.structs.size() == 0);
    std::vector<FunctionIdentification> builtinIds = {
        {L"no_arguments", {}},    {L"argument", {{INT}}},       {L"print", {{STR}}},      {L"println", {{STR}}},
        {L"input", {}},           {L"input", {{INT}}},          {L"len", {{STR}}},        {L"abs", {{INT}}},
        {L"abs", {{FLOAT}}},      {L"max", {{FLOAT}, {FLOAT}}}, {L"max", {{INT}, {INT}}}, {L"min", {{FLOAT}, {FLOAT}}},
        {L"min", {{INT}, {INT}}},
    };
    REQUIRE(builtins.functions.size() == builtinIds.size());
    for(const FunctionIdentification &id: builtinIds)
        REQUIRE(builtins.functions.count(id) == 1);
}
