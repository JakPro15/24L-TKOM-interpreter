#ifndef LEXER_HPP
#define LEXER_HPP

#include "iErrorHandler.hpp"
#include "ilexer.hpp"
#include "ireader.hpp"

#include <functional>
#include <unordered_map>

class Lexer: public ILexer
{
public:
    Lexer(IReader &reader, IErrorHandler &errorHandler);
    Token getNextToken() override;
private:
    static const int MAX_IDENTIFIER_SIZE = 40;
    static const int MAX_COMMENT_SIZE = 100;
    static const int MAX_STRING_SIZE = 100;

    IReader &reader;
    IErrorHandler &errorHandler;
    void skipWhitespace();
    // Will also build keywords and bool literals
    bool buildIdentifier();
    bool buildComment();
    bool buildStringLiteral();
    // Will build int or float literals
    bool buildNumberLiteral();
    int32_t buildIntLiteral();
    bool buildOperator();
    void build2CharOp(wchar_t second, TokenType oneCharType, TokenType twoCharType);
    void build3CharOp(
        wchar_t second, wchar_t third, TokenType oneCharType, TokenType twoCharType, TokenType threeCharType
    );
    unsigned hexToNumber(wchar_t character);
    wchar_t buildHexChar();
    void buildEscapeSequence(std::wstringstream &tokenValue);
    std::unordered_map<wchar_t, std::function<void(void)>> firstCharToFunction;
    void prepareOperatorMap();
    Token tokenBuilt;
};

#endif
