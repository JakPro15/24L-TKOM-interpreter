#include "readerExceptions.hpp"
#include "streamReader.hpp"

#include <catch2/catch_test_macros.hpp>

#include <sstream>

void checkChar(StreamReader &reader, wchar_t character, Position position)
{
    REQUIRE(reader.get() == std::pair<wchar_t, Position>{character, position});
}

TEST_CASE("no newlines ASCII case", "[StreamReader]")
{
    std::wstringstream stream;
    stream.str(L"abc");
    StreamReader reader(stream);
    checkChar(reader, L'a', {1, 1});
    reader.next();
    checkChar(reader, L'b', {1, 2});
    checkChar(reader, L'b', {1, 2});
    reader.next();
    checkChar(reader, L'c', {1, 3});
    reader.next();
    for(int i = 0; i < 10; i++)
    {
        checkChar(reader, IReader::EOT, {1, 4});
        reader.next();
    }
}

TEST_CASE("empty string", "[StreamReader]")
{
    std::wstringstream stream;
    stream.str(L"");
    StreamReader reader(stream);
    checkChar(reader, IReader::EOT, {1, 1});
    checkChar(reader, IReader::EOT, {1, 1});
    reader.next();
    checkChar(reader, IReader::EOT, {1, 1});
}

void nextAndCheck(StreamReader &reader, wchar_t character, Position position)
{
    REQUIRE(reader.next() == std::pair<wchar_t, Position>{character, position});
}

TEST_CASE("no newlines Unicode case", "[StreamReader]")
{
    std::wstringstream stream;
    stream.str(L"ść ඞ读");
    StreamReader reader(stream);
    checkChar(reader, L'ś', {1, 1});
    nextAndCheck(reader, L'ć', {1, 2});
    nextAndCheck(reader, L' ', {1, 3});
    nextAndCheck(reader, L'ඞ', {1, 4});
    nextAndCheck(reader, L'读', {1, 5});
    nextAndCheck(reader, IReader::EOT, {1, 6});
}

TEST_CASE("newlines conversion", "[StreamReader]")
{
    std::vector<std::wstring> stringsToCheck = {L"a\nb\n", L"a\r\nb\r\n", L"a\rb\r"};
    for(const auto &toCheck: stringsToCheck)
    {
        std::wstringstream stream;
        stream.str(toCheck);
        StreamReader reader(stream);
        checkChar(reader, L'a', {1, 1});
        nextAndCheck(reader, L'\n', {1, 2});
        nextAndCheck(reader, L'b', {2, 1});
        nextAndCheck(reader, L'\n', {2, 2});
        nextAndCheck(reader, IReader::EOT, {3, 1});
    }
}

TEST_CASE("control character in input", "[StreamReader]")
{
    std::wstringstream stream;
    stream.str(L"ab\3");
    StreamReader reader(stream);
    reader.next();
    try
    {
        reader.next();
    }
    catch(const ControlCharError &e)
    {
        REQUIRE(e.getPosition() == Position{1, 3});
    }
}

TEST_CASE("error in input stream", "[StreamReader]")
{
    std::wstringstream stream;
    stream.str(L"ab\3");
    StreamReader reader(stream);
    reader.next();
    stream.setstate(std::ios::badbit);
    try
    {
        reader.next();
    }
    catch(const ReaderInputError &e)
    {
        REQUIRE(e.getPosition() == Position{1, 3});
    }
}
