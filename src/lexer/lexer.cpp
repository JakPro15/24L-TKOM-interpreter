#include "lexer.hpp"

#include "lexerExceptions.hpp"
#include "token.hpp"

#include <cmath>
#include <format>
#include <sstream>
#include <string>
#include <unordered_map>

using enum TokenType;

const std::unordered_map<std::wstring, TokenType> keywordToTokenType = {
    {L"include", KW_INCLUDE},
    {L"struct", KW_STRUCT},
    {L"variant", KW_VARIANT},
    {L"func", KW_FUNC},
    {L"continue", KW_CONTINUE},
    {L"break", KW_BREAK},
    {L"return", KW_RETURN},
    {L"if", KW_IF},
    {L"elif", KW_ELIF},
    {L"else", KW_ELSE},
    {L"while", KW_WHILE},
    {L"do", KW_DO},
    {L"is", KW_IS},
    {L"or", KW_OR},
    {L"xor", KW_XOR},
    {L"and", KW_AND},
    {L"not", KW_NOT},
    {L"int", KW_INT},
    {L"float", KW_FLOAT},
    {L"bool", KW_BOOL},
    {L"str", KW_STR},
    {L"true", TRUE_LITERAL},
    {L"false", FALSE_LITERAL},
};

Lexer::Lexer(IReader &reader): reader(reader)
{
    prepareOperatorMap();
}

void Lexer::skipWhitespace()
{
    while(std::iswspace(reader.get()))
        reader.next();
}

bool Lexer::tryBuildIdentifier()
{
    if(!(std::iswalpha(reader.get()) || reader.get() == L'_'))
        return false;

    std::wstringstream tokenValue;
    tokenValue.put(reader.get());
    reader.next();

    while(std::isalnum(reader.get()) || reader.get() == L'_' || reader.get() == L'\'')
    {
        tokenValue.put(reader.get());
        if(tokenValue.tellp() > MAX_IDENTIFIER_SIZE)
            throw IdentifierTooLongError(L"Maximum identifier size exceeded", tokenBuilt.position);
        reader.next();
    }
    std::wstring result = tokenValue.str();
    if(keywordToTokenType.find(result) != keywordToTokenType.end())
        tokenBuilt.type = keywordToTokenType.at(result);
    else
    {
        tokenBuilt.type = IDENTIFIER;
        tokenBuilt.value = result;
    }
    return true;
}

bool Lexer::tryBuildComment()
{
    if(reader.get() != L'#')
        return false;

    reader.next();

    std::wstringstream tokenValue;
    while(reader.get() != L'\n' && reader.get() != IReader::EOT)
    {
        tokenValue.put(reader.get());
        if(tokenValue.tellp() > MAX_COMMENT_SIZE)
            throw CommentTooLongError(L"Maximum comment size exceeded", tokenBuilt.position);
        reader.next();
    }
    tokenBuilt.type = COMMENT;
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
    throw InvalidHexCharError(
        std::format(L"The character {} is not a valid hex digit", character), tokenBuilt.position
    );
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
        throw UnknownEscapeSequenceError(
            std::format(L"\\{} is not a valid escape sequence", reader.get()), tokenBuilt.position
        );
    }
}

bool Lexer::tryBuildString()
{
    if(reader.get() != L'"')
        return false;
    reader.next();

    std::wstringstream tokenValue;
    while(reader.get() != L'"')
    {
        if(reader.get() == L'\n')
            throw NewlineInStringError(L"Newline character in string literal encountered", tokenBuilt.position);
        else if(reader.get() == IReader::EOT)
            throw UnterminatedStringError(L"String literal not terminated", tokenBuilt.position);
        else if(reader.get() == L'\\')
        {
            reader.next();
            buildEscapeSequence(tokenValue);
        }
        else
        {
            tokenValue.put(reader.get());
            if(tokenValue.tellp() > MAX_STRING_SIZE)
                throw StringTooLongError(L"Maximum string literal size exceeded", tokenBuilt.position);
        }
        reader.next();
    }
    reader.next();

    tokenBuilt.type = STR_LITERAL;
    tokenBuilt.value = tokenValue.str();
    return true;
}

int32_t Lexer::buildInteger(bool leadingZeroPermitted)
{
    if(!leadingZeroPermitted && reader.get() == L'0')
    {
        reader.next();
        if(std::iswdigit(reader.get()))
            throw IntWithLeadingZeroError(L"Leading zeros in numeric constant are not permitted", tokenBuilt.position);
        return 0;
    }
    int32_t value = 0;
    do
    {
        int nextDigit = reader.get() - L'0';
        if(value > (INT32_MAX - nextDigit) / 10)
            throw IntTooLargeError(L"Maximum integer literal size exceeded", tokenBuilt.position);
        value *= 10;
        value += nextDigit;
        reader.next();
    }
    while(std::iswdigit(reader.get()));
    return value;
}

int32_t Lexer::buildExponent()
{
    bool exponentNegative = false;
    int32_t exponent = 0;
    if(reader.get() == L'e' || reader.get() == L'E')
    {
        reader.next();
        if(reader.get() == L'-')
        {
            exponentNegative = true;
            reader.next();
        }
        if(std::iswdigit(reader.get()))
            exponent = buildInteger(true);
    }
    if(exponentNegative)
        exponent *= -1;
    return exponent;
}

bool Lexer::tryBuildNumber()
{
    if(!std::iswdigit(reader.get()))
        return false;
    int32_t integralPart = buildInteger();

    if(reader.get() != L'.' && reader.get() != L'e' && reader.get() != L'E')
    {
        tokenBuilt.type = INT_LITERAL;
        tokenBuilt.value = integralPart;
        return true;
    }

    int32_t fractionalPart = 0;
    int fractionalPartDigits = 0;
    if(reader.get() == L'.')
    {
        reader.next();
        if(std::iswdigit(reader.get()))
        {
            int columnBefore = static_cast<int>(reader.getPosition().column);
            fractionalPart = buildInteger(true);
            fractionalPartDigits = static_cast<int>(reader.getPosition().column) - columnBefore;
        }
    }
    int32_t exponent = buildExponent();
    double value = integralPart * std::pow(10., exponent) +
                   static_cast<double>(fractionalPart) * std::pow(10., exponent - fractionalPartDigits);
    tokenBuilt.type = FLOAT_LITERAL;
    tokenBuilt.value = value;
    return true;
}

void Lexer::prepareOperatorMap()
{
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

bool Lexer::tryBuildOperator()
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
        tokenBuilt.type = EOT;
        return tokenBuilt;
    }
    if(tryBuildOperator() || tryBuildNumber() || tryBuildString() || tryBuildComment() || tryBuildIdentifier())
        return tokenBuilt;

    throw UnknownTokenError(std::format(L"No known token begins with character {}", reader.get()), tokenBuilt.position);
}
