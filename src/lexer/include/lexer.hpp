#ifndef LEXER_HPP
#define LEXER_HPP

#include "iLexer.hpp"
#include "iReader.hpp"

#include <functional>
#include <optional>
#include <unordered_map>
#include <utility>

class Lexer: public ILexer
{
public:
    Lexer(IReader &reader);
    Token getNextToken() override;

    static const int MAX_IDENTIFIER_SIZE = 40;
    static const int MAX_COMMENT_SIZE = 100;
    static const int MAX_STRING_SIZE = 100;
private:
    IReader &reader;
    void skipWhitespace();
    // Will also build keywords and bool literals
    std::optional<Token> tryBuildIdentifier();
    std::optional<Token> tryBuildComment();
    std::optional<Token> tryBuildString();
    // Will build int or float literals
    std::optional<Token> tryBuildNumber();
    std::optional<Token> tryBuildFraction(int32_t integralPart);
    // Returns the built integer and the number of leading zeros
    std::pair<int32_t, int> buildIntegerWithLeadingZeros();
    std::optional<std::pair<int32_t, int>> tryBuildFractionalPart();
    std::optional<int32_t> tryBuildExponent();
    std::optional<Token> tryBuildOperator();
    TokenType build2CharOp(wchar_t second, TokenType oneCharType, TokenType twoCharType);
    TokenType build3CharOp(
        wchar_t second, wchar_t third, TokenType oneCharType, TokenType twoCharType, TokenType threeCharType
    );
    unsigned hexToNumber(wchar_t character);
    wchar_t buildHexChar();
    void buildEscapeSequence(std::wstringstream &tokenValue);
    std::unordered_map<wchar_t, std::function<Token(void)>> firstCharToFunction;
    void prepareOperatorMap();
    Position tokenStart;
};

#endif
