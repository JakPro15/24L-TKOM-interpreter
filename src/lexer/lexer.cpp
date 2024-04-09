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
    while(std::iswspace(reader.get().first))
        reader.next();
}

std::optional<Token> Lexer::tryBuildIdentifier()
{
    if(!(std::iswalpha(reader.get().first) || reader.get().first == L'_'))
        return std::nullopt;

    std::wstringstream tokenValue;
    tokenValue.put(reader.get().first);
    reader.next();

    while(std::isalnum(reader.get().first) || reader.get().first == L'_' || reader.get().first == L'\'')
    {
        tokenValue.put(reader.get().first);
        if(tokenValue.tellp() > MAX_IDENTIFIER_SIZE)
            throw IdentifierTooLongError(L"Maximum identifier size exceeded", tokenStart);
        reader.next();
    }
    std::wstring result = tokenValue.str();
    if(keywordToTokenType.find(result) != keywordToTokenType.end())
        return Token{keywordToTokenType.at(result), tokenStart};
    else
        return Token{IDENTIFIER, tokenStart, result};
}

std::optional<Token> Lexer::tryBuildComment()
{
    if(reader.get().first != L'#')
        return std::nullopt;

    reader.next();

    std::wstringstream tokenValue;
    while(reader.get().first != L'\n' && reader.get().first != IReader::EOT)
    {
        tokenValue.put(reader.get().first);
        if(tokenValue.tellp() > MAX_COMMENT_SIZE)
            throw CommentTooLongError(L"Maximum comment size exceeded", tokenStart);
        reader.next();
    }
    return Token{COMMENT, tokenStart, tokenValue.str()};
}

unsigned Lexer::hexToNumber(wchar_t character)
{
    if(character >= L'0' && character <= L'9')
        return character - L'0';
    if(character >= L'a' && character <= L'f')
        return character - L'a' + 10;
    if(character >= L'A' && character <= L'F')
        return character - L'A' + 10;
    throw InvalidHexCharError(std::format(L"The character {} is not a valid hex digit", character), tokenStart);
}

wchar_t Lexer::buildHexChar()
{
    unsigned first = hexToNumber(reader.get().first);
    unsigned second = hexToNumber(reader.next().first);
    return static_cast<wchar_t>(first * 16 + second);
}

void Lexer::buildEscapeSequence(std::wstringstream &tokenValue)
{
    switch(reader.get().first)
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
    case L'\n':
        throw NewlineInStringError(L"Newline character in string literal encountered", tokenStart);
    case IReader::EOT:
        throw UnterminatedStringError(L"String literal not terminated", tokenStart);
    default:
        throw UnknownEscapeSequenceError(
            std::format(L"\\{} is not a valid escape sequence", reader.get().first), tokenStart
        );
    }
}

std::optional<Token> Lexer::tryBuildString()
{
    if(reader.get().first != L'"')
        return std::nullopt;
    reader.next();

    std::wstringstream tokenValue;
    while(reader.get().first != L'"')
    {
        if(reader.get().first == L'\n')
            throw NewlineInStringError(L"Newline character in string literal encountered", tokenStart);
        else if(reader.get().first == IReader::EOT)
            throw UnterminatedStringError(L"String literal not terminated", tokenStart);
        else if(reader.get().first == L'\\')
        {
            reader.next();
            buildEscapeSequence(tokenValue);
        }
        else
        {
            tokenValue.put(reader.get().first);
            if(tokenValue.tellp() > MAX_STRING_SIZE)
                throw StringTooLongError(L"Maximum string literal size exceeded", tokenStart);
        }
        reader.next();
    }
    reader.next();
    return Token{STR_LITERAL, tokenStart, tokenValue.str()};
}

std::pair<int32_t, int> Lexer::buildIntegerWithLeadingZeros()
{
    int leadingZeros = 0;
    while(reader.get().first == L'0')
    {
        leadingZeros += 1;
        reader.next();
    }
    int32_t value = 0;
    while(std::iswdigit(reader.get().first))
    {
        int nextDigit = reader.get().first - L'0';
        if(value > (INT32_MAX - nextDigit) / 10)
            throw IntTooLargeError(L"Maximum integer literal size exceeded", tokenStart);
        value *= 10;
        value += nextDigit;
        reader.next();
    }
    return {value, leadingZeros};
}

std::optional<std::pair<int32_t, int>> Lexer::tryBuildFractionalPart()
{
    int32_t fractionalPart = 0;
    int digits = 0;
    if(reader.get().first != L'.')
        return std::nullopt;

    if(std::iswdigit(reader.next().first))
    {
        int columnBefore = static_cast<int>(reader.get().second.column);
        fractionalPart = buildIntegerWithLeadingZeros().first;
        digits = static_cast<int>(reader.get().second.column) - columnBefore;
    }
    return {{fractionalPart, digits}};
}

std::optional<int32_t> Lexer::tryBuildExponent()
{
    if(reader.get().first != L'e' && reader.get().first != L'E')
        return std::nullopt;

    bool exponentNegative = false;
    int32_t exponent = 0;
    if(reader.next().first == L'-')
    {
        exponentNegative = true;
        if(!std::iswdigit(reader.next().first))
            throw InvalidExponentError(L"Exponent consisting of only a minus sign is not permitted", tokenStart);
    }
    if(std::iswdigit(reader.get().first))
        exponent = buildIntegerWithLeadingZeros().first;

    if(exponentNegative)
        exponent *= -1;
    return exponent;
}

std::optional<Token> Lexer::tryBuildFraction(int32_t integralPart)
{
    auto fractionalPart = tryBuildFractionalPart();
    auto exponent = tryBuildExponent();
    if(!fractionalPart && !exponent)
        return std::nullopt;
    if(!fractionalPart)
        fractionalPart = {0, 0};
    if(!exponent)
        exponent = 0;

    double value = integralPart * std::pow(10., *exponent) +
                   static_cast<double>(fractionalPart->first) * std::pow(10., *exponent - fractionalPart->second);
    return Token{FLOAT_LITERAL, tokenStart, value};
}

std::optional<Token> Lexer::tryBuildNumber()
{
    if(!std::iswdigit(reader.get().first))
        return std::nullopt;
    auto [integralPart, leadingZeros] = buildIntegerWithLeadingZeros();
    if((integralPart != 0 && leadingZeros > 0) || leadingZeros > 1)
        throw IntWithLeadingZeroError(L"Leading zeros in numeric constant are not permitted", tokenStart);

    std::optional<Token> tokenBuilt;
    if((tokenBuilt = tryBuildFraction(integralPart)))
        return tokenBuilt;

    return Token{INT_LITERAL, tokenStart, integralPart};
}

#define ADD_OPERATOR(firstChar, returnedType) \
    firstCharToFunction.emplace(L##firstChar, [&]() { return Token{returnedType, tokenStart}; });

void Lexer::prepareOperatorMap()
{
    ADD_OPERATOR('{', LBRACE);
    ADD_OPERATOR('}', RBRACE);
    ADD_OPERATOR(';', SEMICOLON);
    ADD_OPERATOR('(', LPAREN);
    ADD_OPERATOR(')', RPAREN);
    ADD_OPERATOR('-', build2CharOp(L'>', OP_MINUS, ARROW));
    ADD_OPERATOR(',', COMMA);
    ADD_OPERATOR('$', DOLLAR_SIGN);
    ADD_OPERATOR('=', build3CharOp(L'=', L'=', OP_ASSIGN, OP_EQUAL, OP_IDENTICAL));
    ADD_OPERATOR('!', build3CharOp(L'=', L'=', OP_CONCAT, OP_NOT_EQUAL, OP_NOT_IDENTICAL));
    ADD_OPERATOR('.', OP_DOT);
    ADD_OPERATOR('@', OP_STR_MULTIPLY);
    ADD_OPERATOR('>', build2CharOp(L'=', OP_GREATER, OP_GREATER_EQUAL));
    ADD_OPERATOR('<', build2CharOp(L'=', OP_LESSER, OP_LESSER_EQUAL));
    ADD_OPERATOR('+', OP_PLUS);
    ADD_OPERATOR('*', build2CharOp(L'*', OP_MULTIPLY, OP_EXPONENT));
    ADD_OPERATOR('/', build2CharOp(L'/', OP_DIVIDE, OP_FLOOR_DIVIDE));
    ADD_OPERATOR('%', OP_MODULO);
    ADD_OPERATOR('[', LSQUAREBRACE);
    ADD_OPERATOR(']', RSQUAREBRACE);
}

TokenType Lexer::build2CharOp(wchar_t second, TokenType oneCharType, TokenType twoCharType)
{
    if(reader.get().first == second)
    {
        reader.next();
        return twoCharType;
    }
    else
        return oneCharType;
}

TokenType Lexer::build3CharOp(
    wchar_t second, wchar_t third, TokenType oneCharType, TokenType twoCharType, TokenType threeCharType
)
{
    if(reader.get().first == second)
    {
        if(reader.next().first == third)
        {
            reader.next();
            return threeCharType;
        }
        else
            return twoCharType;
    }
    else
        return oneCharType;
}

std::optional<Token> Lexer::tryBuildOperator()
{
    wchar_t firstChar = reader.get().first;
    if(firstCharToFunction.find(firstChar) == firstCharToFunction.end())
        return std::nullopt;
    reader.next();
    return firstCharToFunction.at(firstChar)();
}

Token Lexer::getNextToken()
{
    skipWhitespace();
    tokenStart = reader.get().second;
    if(reader.get().first == IReader::EOT)
        return Token{EOT, tokenStart};

    std::optional<Token> tokenBuilt;
    if((tokenBuilt = tryBuildOperator()))
        return *tokenBuilt;
    if((tokenBuilt = tryBuildNumber()))
        return *tokenBuilt;
    if((tokenBuilt = tryBuildString()))
        return *tokenBuilt;
    if((tokenBuilt = tryBuildComment()))
        return *tokenBuilt;
    if((tokenBuilt = tryBuildIdentifier()))
        return *tokenBuilt;

    throw UnknownTokenError(std::format(L"No known token begins with character {}", reader.get().first), tokenStart);
}
