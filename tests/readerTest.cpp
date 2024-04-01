#include "reader.hpp"

#include <catch2/catch_test_macros.hpp>

#include <sstream>

TEST_CASE("no newlines ASCII case", "[Reader]")
{
    std::wstringstream stream;
    stream.str(L"abc");
    Reader reader(stream);
    REQUIRE(reader.get() == L'a');
    REQUIRE(reader.get() == L'a');
    reader.next();
    REQUIRE(reader.get() == L'b');
    REQUIRE(reader.get() == L'b');
    REQUIRE(reader.get() == L'b');
    reader.next();
    reader.next();
    for(int i = 0; i < 10; i++)
    {
        REQUIRE(reader.get() == ETX);
        reader.next();
    }
}

TEST_CASE("empty string", "[Reader]")
{
    std::wstringstream stream;
    stream.str(L"");
    Reader reader(stream);
    REQUIRE(reader.get() == ETX);
    REQUIRE(reader.get() == ETX);
    reader.next();
    REQUIRE(reader.get() == ETX);
}

wchar_t getAndNext(Reader &reader)
{
    wchar_t result = reader.get();
    reader.next();
    return result;
}

TEST_CASE("no newlines Unicode case", "[Reader]")
{
    std::wstringstream stream;
    stream.str(L"ść ඞ读");
    Reader reader(stream);
    REQUIRE(getAndNext(reader) == L'ś');
    REQUIRE(getAndNext(reader) == L'ć');
    REQUIRE(getAndNext(reader) == L' ');
    REQUIRE(getAndNext(reader) == L'ඞ');
    REQUIRE(getAndNext(reader) == L'读');
    REQUIRE(getAndNext(reader) == ETX);
}

TEST_CASE("LF newlines", "[Reader]")
{
    std::wstringstream stream;
    stream.str(L"a\nb\n");
    Reader reader(stream);
    REQUIRE(getAndNext(reader) == L'a');
    REQUIRE(getAndNext(reader) == L'\n');
    REQUIRE(getAndNext(reader) == L'b');
    REQUIRE(getAndNext(reader) == L'\n');
    REQUIRE(getAndNext(reader) == ETX);
}

TEST_CASE("CRLF newlines", "[Reader]")
{
    std::wstringstream stream;
    stream.str(L"a\r\nb\r\n");
    Reader reader(stream);
    REQUIRE(getAndNext(reader) == L'a');
    REQUIRE(getAndNext(reader) == L'\n');
    REQUIRE(getAndNext(reader) == L'b');
    REQUIRE(getAndNext(reader) == L'\n');
    REQUIRE(getAndNext(reader) == ETX);
}

TEST_CASE("CR newlines", "[Reader]")
{
    std::wstringstream stream;
    stream.str(L"a\rb\r");
    Reader reader(stream);
    REQUIRE(getAndNext(reader) == L'a');
    REQUIRE(getAndNext(reader) == L'\n');
    REQUIRE(getAndNext(reader) == L'b');
    REQUIRE(getAndNext(reader) == L'\n');
    REQUIRE(getAndNext(reader) == ETX);
}
