#include "lexer.hpp"

#include "token.hpp"

#include <cmath>
#include <format>
#include <sstream>
#include <string>
#include <unordered_map>

const std::unordered_map<std::wstring, TokenType> keywordToTokenType = {
    {L"include", TokenType::KW_INCLUDE},
    {L"struct", TokenType::KW_STRUCT},
    {L"variant", TokenType::KW_VARIANT},
    {L"func", TokenType::KW_FUNC},
    {L"continue", TokenType::KW_CONTINUE},
    {L"break", TokenType::KW_BREAK},
    {L"return", TokenType::KW_RETURN},
    {L"if", TokenType::KW_IF},
    {L"elif", TokenType::KW_ELIF},
    {L"else", TokenType::KW_ELSE},
    {L"while", TokenType::KW_WHILE},
    {L"do", TokenType::KW_DO},
    {L"is", TokenType::KW_IS},
    {L"or", TokenType::KW_OR},
    {L"xor", TokenType::KW_XOR},
    {L"and", TokenType::KW_AND},
    {L"not", TokenType::KW_NOT},
    {L"int", TokenType::KW_INT},
    {L"float", TokenType::KW_FLOAT},
    {L"bool", TokenType::KW_BOOL},
    {L"str", TokenType::KW_STR},
    {L"true", TokenType::TRUE_LITERAL},
    {L"false", TokenType::FALSE_LITERAL},
};

Lexer::Lexer(IReader &reader, IErrorHandler &errorHandler): reader(reader), errorHandler(errorHandler)
{
    prepareOperatorMap();
}

void Lexer::skipWhitespace()
{
    while(std::iswspace(reader.get()))
        reader.next();
}

bool Lexer::buildIdentifier()
{
    if(!(std::iswalpha(reader.get()) || reader.get() == L'_'))
        return false;

    std::wstringstream tokenValue;
    tokenValue.put(reader.get());
    reader.next();

    while(std::isalnum(reader.get()) || reader.get() == L'_' || reader.get() == L'\'')
    {
        tokenValue.put(reader.get());
        if(tokenValue.tellg() > MAX_IDENTIFIER_SIZE)
            errorHandler.handleError(
                Error::LEXER_IDENTIFIER_TOO_LONG, L"Maximum identifier size exceeded", tokenBuilt.position
            );
        reader.next();
    }
    std::wstring result = tokenValue.str();
    if(keywordToTokenType.find(result) != keywordToTokenType.end())
        tokenBuilt.type = keywordToTokenType.at(result);
    else
    {
        tokenBuilt.type = TokenType::IDENTIFIER;
        tokenBuilt.value = result;
    }
    return true;
}

bool Lexer::buildComment()
{
    if(reader.get() != L'#')
        return false;

    reader.next();

    std::wstringstream tokenValue;
    while(reader.get() != L'\n')
    {
        tokenValue.put(reader.get());
        if(tokenValue.tellg() > MAX_COMMENT_SIZE)
            errorHandler.handleError(
                Error::LEXER_COMMENT_TOO_LONG, L"Maximum comment size exceeded", tokenBuilt.position
            );
        reader.next();
    }
    tokenBuilt.type = TokenType::COMMENT;
    tokenBuilt.value = tokenValue.str();
    return true;
}

unsigned Lexer::hexToNumber(wchar_t character)
{
    if(character >= L'0' && character <= L'9')
        return character - L'0';
    if(character >= L'a' && character <= L'f')
        return character - L'a' + 10;
    if(character >= L'A' && character <= L'F')
        return character - L'A' + 10;
    errorHandler.handleError(
        Error::LEXER_INVALID_HEX_CHAR, std::format(L"The character {} is not a valid hex digit", character),
        tokenBuilt.position
    );
    return 0;
}

wchar_t Lexer::buildHexChar()
{
    unsigned first = hexToNumber(reader.get());
    reader.next();
    unsigned second = hexToNumber(reader.get());
    return static_cast<wchar_t>(first * 16 + second);
}

void Lexer::buildEscapeSequence(std::wstringstream &tokenValue)
{
    switch(reader.get())
    {
    case L't':
        tokenValue.put(L'\t');
        break;
    case L'r':
        tokenValue.put(L'\r');
        break;
    case L'n':
        tokenValue.put(L'\n');
        break;
    case L'"':
        tokenValue.put(L'"');
        break;
    case L'\\':
        tokenValue.put(L'\\');
        break;
    case L'x':
        reader.next();
        tokenValue.put(buildHexChar());
        break;
    default:
        errorHandler.handleError(
            Error::LEXER_UNKNOWN_ESCAPE, std::format(L"\\{} is not a valid escape sequence", reader.get()),
            tokenBuilt.position
        );
    }
}

bool Lexer::buildStringLiteral()
{
    if(reader.get() != L'"')
        return false;
    reader.next();

    std::wstringstream tokenValue;
    while(reader.get() != L'"')
    {
        if(reader.get() == L'\n')
            errorHandler.handleError(
                Error::LEXER_NEWLINE_IN_STRING, L"Newline character in string literal encountered", tokenBuilt.position
            );
        else if(reader.get() == L'\\')
        {
            reader.next();
            buildEscapeSequence(tokenValue);
        }
        else
        {
            tokenValue.put(reader.get());
            if(tokenValue.tellg() > MAX_STRING_SIZE)
                errorHandler.handleError(
                    Error::LEXER_STRING_TOO_LONG, L"Maximum string literal size exceeded", tokenBuilt.position
                );
        }
        reader.next();
    }
    reader.next();

    tokenBuilt.type = TokenType::STR_LITERAL;
    tokenBuilt.value = tokenValue.str();
    return true;
}

int32_t Lexer::buildIntLiteral()
{
    if(reader.get() == L'0')
    {
        reader.next();
        return 0;
    }
    reader.next();
    int32_t value = 0;
    while(std::iswdigit(reader.get()))
    {
        int nextDigit = reader.get() - L'0';
        if(value > (INT32_MAX - nextDigit) / 10)
        {
            errorHandler.handleError(
                Error::LEXER_INT_TOO_LARGE, L"Maximum integer literal size exceeded", tokenBuilt.position
            );
        }
        value *= 10;
        value += nextDigit;
        reader.next();
    }
    return value;
}

bool Lexer::buildNumberLiteral()
{
    if(!std::iswdigit(reader.get()))
        return false;
    int32_t integralPart = buildIntLiteral();

    if(reader.get() != L'.' && reader.get() != L'e' && reader.get() != L'E')
    {
        tokenBuilt.type = TokenType::INT_LITERAL;
        tokenBuilt.value = integralPart;
        return true;
    }

    int32_t fractionalPart = 0;
    int fractionalPartDigits = 0;
    if(reader.get() == L'.')
    {
        reader.next();
        while(std::iswdigit(reader.get()))
        {
            int nextDigit = reader.get() - L'0';
            if(fractionalPart > (INT32_MAX - nextDigit) / 10)
            {
                errorHandler.handleError(
                    Error::LEXER_INT_TOO_LARGE,
                    L"Maximum integer literal size exceeded in float literal fractional part", tokenBuilt.position
                );
            }
            fractionalPart *= 10;
            fractionalPart += nextDigit;
            fractionalPartDigits += 1;
            reader.next();
        }
    }
    int32_t exponent = 0;
    if(reader.get() == L'e' || reader.get() == L'E')
    {
        reader.next();
        exponent = buildIntLiteral();
    }
    double value = (integralPart + static_cast<double>(fractionalPart) / std::pow(10, fractionalPartDigits)) *
                   std::pow(10, exponent);
    tokenBuilt.type = TokenType::FLOAT_LITERAL;
    tokenBuilt.value = value;
    return true;
}

void Lexer::prepareOperatorMap()
{
    using enum TokenType;
    firstCharToFunction.emplace(L'{', [&]() { tokenBuilt.type = LBRACE; });
    firstCharToFunction.emplace(L'}', [&]() { tokenBuilt.type = RBRACE; });
    firstCharToFunction.emplace(L';', [&]() { tokenBuilt.type = SEMICOLON; });
    firstCharToFunction.emplace(L'(', [&]() { tokenBuilt.type = LPAREN; });
    firstCharToFunction.emplace(L')', [&]() { tokenBuilt.type = RPAREN; });
    firstCharToFunction.emplace(L'-', [&]() { build2CharOp(L'>', OP_MINUS, ARROW); });
    firstCharToFunction.emplace(L',', [&]() { tokenBuilt.type = COMMA; });
    firstCharToFunction.emplace(L'$', [&]() { tokenBuilt.type = DOLLAR_SIGN; });
    firstCharToFunction.emplace(L'=', [&]() { build3CharOp(L'=', L'=', OP_ASSIGN, OP_EQUAL, OP_IDENTICAL); });
    firstCharToFunction.emplace(L'!', [&]() { build3CharOp(L'=', L'=', OP_CONCAT, OP_NOT_EQUAL, OP_NOT_IDENTICAL); });
    firstCharToFunction.emplace(L'.', [&]() { tokenBuilt.type = OP_DOT; });
    firstCharToFunction.emplace(L'@', [&]() { tokenBuilt.type = OP_STR_MULTIPLY; });
    firstCharToFunction.emplace(L'>', [&]() { build2CharOp(L'=', OP_GREATER, OP_GREATER_EQUAL); });
    firstCharToFunction.emplace(L'<', [&]() { build2CharOp(L'=', OP_LESSER, OP_LESSER_EQUAL); });
    firstCharToFunction.emplace(L'+', [&]() { tokenBuilt.type = OP_PLUS; });
    firstCharToFunction.emplace(L'*', [&]() { build2CharOp(L'*', OP_MULTIPLY, OP_EXPONENT); });
    firstCharToFunction.emplace(L'/', [&]() { build2CharOp(L'/', OP_DIVIDE, OP_FLOOR_DIVIDE); });
    firstCharToFunction.emplace(L'%', [&]() { tokenBuilt.type = OP_MODULO; });
    firstCharToFunction.emplace(L'[', [&]() { tokenBuilt.type = LSQUAREBRACE; });
    firstCharToFunction.emplace(L']', [&]() { tokenBuilt.type = RSQUAREBRACE; });
}

void Lexer::build2CharOp(wchar_t second, TokenType oneCharType, TokenType twoCharType)
{
    if(reader.get() == second)
    {
        tokenBuilt.type = twoCharType;
        reader.next();
    }
    else
        tokenBuilt.type = oneCharType;
}

void Lexer::build3CharOp(
    wchar_t second, wchar_t third, TokenType oneCharType, TokenType twoCharType, TokenType threeCharType
)
{
    if(reader.get() == second)
    {
        reader.next();
        if(reader.get() == third)
        {
            tokenBuilt.type = threeCharType;
            reader.next();
        }
        else
            tokenBuilt.type = twoCharType;
    }
    else
        tokenBuilt.type = oneCharType;
}

bool Lexer::buildOperator()
{
    wchar_t firstChar = reader.get();
    if(firstCharToFunction.find(firstChar) == firstCharToFunction.end())
        return false;
    reader.next();
    firstCharToFunction.at(firstChar)();
    return true;
}

Token Lexer::getNextToken()
{
    skipWhitespace();
    tokenBuilt.position = reader.getPosition();
    if(reader.get() == IReader::EOT)
    {
        tokenBuilt.type = TokenType::EOT;
        return tokenBuilt;
    }
    if(buildOperator() || buildNumberLiteral() || buildStringLiteral() || buildComment() || buildIdentifier())
        return tokenBuilt;

    errorHandler.handleError(
        Error::LEXER_UNKNOWN_TOKEN, std::format(L"No known token begins with character {}", reader.get()),
        tokenBuilt.position
    );

    tokenBuilt.type = TokenType::UNKNOWN;
    return tokenBuilt;
}
