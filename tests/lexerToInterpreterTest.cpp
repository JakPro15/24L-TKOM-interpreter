#include "commentDiscarder.hpp"
#include "helpers.hpp"
#include "interpreter.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "streamReader.hpp"

namespace {
std::wstring interpret(
    const std::wstring &sourceCode, const std::vector<std::wstring> &arguments = {}, const std::wstring &input = L""
)
{
    std::wstringstream sourceStream(sourceCode);
    StreamReader reader(sourceStream, L"<test>");
    Lexer lexer(reader);
    CommentDiscarder commentDiscarder(lexer);
    Parser parser(commentDiscarder);
    Program program = parser.parseProgram();
    std::wstringstream inputStream(input), outputStream;
    Interpreter interpreter(L"<test>", arguments, inputStream, outputStream, [](const std::wstring &) -> Program {
        throw std::runtime_error("No files should be included in these tests");
    });
    interpreter.visit(program);
    return outputStream.str();
}
}

TEST_CASE("variable declarations", "[Lexer+Parser+Interpreter]")
{
    REQUIRE(interpret(wrapInMain(L"int a = 20; print(a);")) == L"20");
}
