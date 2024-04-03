#include "streamReader.hpp"
#include "testErrorHandler.hpp"

#include <catch2/catch_test_macros.hpp>

#include <sstream>

TEST_CASE("no newlines ASCII case", "[StreamReader]")
{
    std::wstringstream stream;
    TestErrorHandler errorHandler;
    stream.str(L"abc");
    StreamReader reader(stream, errorHandler);
    REQUIRE(reader.get() == L'a');
    REQUIRE(reader.getPosition() == Position{1, 1});
    REQUIRE(reader.get() == L'a');
    REQUIRE(reader.getPosition() == Position{1, 1});
    reader.next();
    REQUIRE(reader.getPosition() == Position{1, 2});
    REQUIRE(reader.get() == L'b');
    REQUIRE(reader.get() == L'b');
    REQUIRE(reader.get() == L'b');
    reader.next();
    REQUIRE(reader.getPosition() == Position{1, 3});
    reader.next();
    for(int i = 0; i < 10; i++)
    {
        REQUIRE(reader.getPosition() == Position{1, 4});
        REQUIRE(reader.get() == IReader::EOT);
        reader.next();
    }
}

TEST_CASE("empty string", "[StreamReader]")
{
    std::wstringstream stream;
    TestErrorHandler errorHandler;
    stream.str(L"");
    StreamReader reader(stream, errorHandler);
    REQUIRE(reader.getPosition() == Position{1, 1});
    REQUIRE(reader.get() == IReader::EOT);
    REQUIRE(reader.get() == IReader::EOT);
    reader.next();
    REQUIRE(reader.get() == IReader::EOT);
    REQUIRE(reader.getPosition() == Position{1, 1});
}

wchar_t getAndNext(StreamReader &reader)
{
    wchar_t result = reader.get();
    reader.next();
    return result;
}

TEST_CASE("no newlines Unicode case", "[StreamReader]")
{
    std::wstringstream stream;
    TestErrorHandler errorHandler;
    stream.str(L"ść ඞ读");
    StreamReader reader(stream, errorHandler);
    REQUIRE(reader.getPosition() == Position{1, 1});
    REQUIRE(getAndNext(reader) == L'ś');
    REQUIRE(reader.getPosition() == Position{1, 2});
    REQUIRE(getAndNext(reader) == L'ć');
    REQUIRE(reader.getPosition() == Position{1, 3});
    REQUIRE(getAndNext(reader) == L' ');
    REQUIRE(reader.getPosition() == Position{1, 4});
    REQUIRE(getAndNext(reader) == L'ඞ');
    REQUIRE(reader.getPosition() == Position{1, 5});
    REQUIRE(getAndNext(reader) == L'读');
    REQUIRE(reader.getPosition() == Position{1, 6});
    REQUIRE(getAndNext(reader) == IReader::EOT);
}

#include <iostream>

TEST_CASE("LF newlines", "[StreamReader]")
{
    std::wstringstream stream;
    TestErrorHandler errorHandler;
    stream.str(L"a\nb\n");
    StreamReader reader(stream, errorHandler);
    REQUIRE(reader.getPosition() == Position{1, 1});
    REQUIRE(getAndNext(reader) == L'a');
    REQUIRE(reader.getPosition() == Position{1, 2});
    REQUIRE(getAndNext(reader) == L'\n');
    REQUIRE(reader.getPosition() == Position{2, 1});
    REQUIRE(getAndNext(reader) == L'b');
    REQUIRE(reader.getPosition() == Position{2, 2});
    REQUIRE(getAndNext(reader) == L'\n');
    REQUIRE(reader.getPosition() == Position{3, 1});
    REQUIRE(getAndNext(reader) == IReader::EOT);
}

TEST_CASE("CRLF newlines", "[StreamReader]")
{
    std::wstringstream stream;
    TestErrorHandler errorHandler;
    stream.str(L"a\r\nb\r\n");
    StreamReader reader(stream, errorHandler);
    REQUIRE(reader.getPosition() == Position{1, 1});
    REQUIRE(getAndNext(reader) == L'a');
    REQUIRE(reader.getPosition() == Position{1, 2});
    REQUIRE(getAndNext(reader) == L'\n');
    REQUIRE(reader.getPosition() == Position{2, 1});
    REQUIRE(getAndNext(reader) == L'b');
    REQUIRE(reader.getPosition() == Position{2, 2});
    REQUIRE(getAndNext(reader) == L'\n');
    REQUIRE(reader.getPosition() == Position{3, 1});
    REQUIRE(getAndNext(reader) == IReader::EOT);
}

TEST_CASE("CR newlines", "[StreamReader]")
{
    std::wstringstream stream;
    TestErrorHandler errorHandler;
    stream.str(L"a\rb\r");
    StreamReader reader(stream, errorHandler);
    REQUIRE(reader.getPosition() == Position{1, 1});
    REQUIRE(getAndNext(reader) == L'a');
    REQUIRE(reader.getPosition() == Position{1, 2});
    REQUIRE(getAndNext(reader) == L'\n');
    REQUIRE(reader.getPosition() == Position{2, 1});
    REQUIRE(getAndNext(reader) == L'b');
    REQUIRE(reader.getPosition() == Position{2, 2});
    REQUIRE(getAndNext(reader) == L'\n');
    REQUIRE(reader.getPosition() == Position{3, 1});
    REQUIRE(getAndNext(reader) == IReader::EOT);
}

TEST_CASE("control character in input", "[StreamReader]")
{
    std::wstringstream stream;
    TestErrorHandler errorHandler;
    stream.str(L"ab\3");
    StreamReader reader(stream, errorHandler);
    reader.next();
    REQUIRE_THROWS(reader.next());
    REQUIRE(errorHandler.error == Error::READER_CONTROL_CHAR);
    REQUIRE(errorHandler.position == Position{1, 3});
}

TEST_CASE("error in input stream", "[StreamReader]")
{
    std::wstringstream stream;
    TestErrorHandler errorHandler;
    stream.str(L"ab\3");
    StreamReader reader(stream, errorHandler);
    reader.next();
    stream.setstate(std::ios::badbit);
    REQUIRE_THROWS(reader.next());
    REQUIRE(errorHandler.error == Error::READER_INPUT_ERROR);
    REQUIRE(errorHandler.position == Position{1, 3});
}
