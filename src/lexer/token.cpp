#include "token.hpp"

#include "lexerExceptions.hpp"

#include <array>
#include <ostream>

using enum TokenType;

Token::Token(TokenType type, Position position): type(type), position(position)
{
    if(type == IDENTIFIER || type == COMMENT || type == STR_LITERAL || type == INT_LITERAL || type == FLOAT_LITERAL)
        throw InvalidTokenValueError(L"A value needs to be given for this type of token", position);
}

Token::Token(TokenType type, Position position, std::wstring value): type(type), position(position), value(value)
{
    if(type != IDENTIFIER && type != COMMENT && type != STR_LITERAL)
        throw InvalidTokenValueError(L"Only identifier and comment tokens can contain std::wstring value", position);
}

Token::Token(TokenType type, Position position, int32_t value): type(type), position(position), value(value)
{
    if(type != INT_LITERAL)
        throw InvalidTokenValueError(L"Only integer literal tokens can contain int32_t value", position);
}

Token::Token(TokenType type, Position position, double value): type(type), position(position), value(value)
{
    if(type != FLOAT_LITERAL)
        throw InvalidTokenValueError(L"Only float literal tokens can contain double value", position);
}

TokenType Token::getType() const
{
    return type;
}

Position Token::getPosition() const
{
    return position;
}

std::variant<std::monostate, std::wstring, int32_t, double> Token::getValue() const
{
    return value;
}

std::wostream &operator<<(std::wostream &out, Token token)
{
    switch(token.type)
    {
    case KW_INCLUDE:
        out << L"include";
        break;
    case KW_STRUCT:
        out << L"struct";
        break;
    case KW_VARIANT:
        out << L"variant";
        break;
    case KW_FUNC:
        out << L"func";
        break;
    case KW_CONTINUE:
        out << L"continue";
        break;
    case KW_BREAK:
        out << L"break";
        break;
    case KW_RETURN:
        out << L"return";
        break;
    case KW_IF:
        out << L"if";
        break;
    case KW_ELIF:
        out << L"elif";
        break;
    case KW_ELSE:
        out << L"else";
        break;
    case KW_WHILE:
        out << L"while";
        break;
    case KW_DO:
        out << L"do";
        break;
    case KW_IS:
        out << L"is";
        break;
    case KW_OR:
        out << L"or";
        break;
    case KW_XOR:
        out << L"xor";
        break;
    case KW_AND:
        out << L"and";
        break;
    case KW_NOT:
        out << L"not";
        break;
    case KW_INT:
        out << L"int";
        break;
    case KW_FLOAT:
        out << L"float";
        break;
    case KW_BOOL:
        out << L"bool";
        break;
    case KW_STR:
        out << L"str";
        break;
    case LBRACE:
        out << L"{";
        break;
    case RBRACE:
        out << L"}";
        break;
    case SEMICOLON:
        out << L";";
        break;
    case LPAREN:
        out << L"(";
        break;
    case RPAREN:
        out << L")";
        break;
    case ARROW:
        out << L"->";
        break;
    case COMMA:
        out << L",";
        break;
    case DOLLAR_SIGN:
        out << L"$";
        break;
    case OP_ASSIGN:
        out << L"=";
        break;
    case OP_DOT:
        out << L".";
        break;
    case OP_EQUAL:
        out << L"==";
        break;
    case OP_NOT_EQUAL:
        out << L"!=";
        break;
    case OP_IDENTICAL:
        out << L"===";
        break;
    case OP_NOT_IDENTICAL:
        out << L"!==";
        break;
    case OP_CONCAT:
        out << L"!";
        break;
    case OP_STR_MULTIPLY:
        out << L"@";
        break;
    case OP_GREATER:
        out << L">";
        break;
    case OP_LESSER:
        out << L"<";
        break;
    case OP_GREATER_EQUAL:
        out << L">=";
        break;
    case OP_LESSER_EQUAL:
        out << L"<=";
        break;
    case OP_PLUS:
        out << L"+";
        break;
    case OP_MINUS:
        out << L"-";
        break;
    case OP_MULTIPLY:
        out << L"*";
        break;
    case OP_DIVIDE:
        out << L"/";
        break;
    case OP_FLOOR_DIVIDE:
        out << L"//";
        break;
    case OP_MODULO:
        out << L"%";
        break;
    case OP_EXPONENT:
        out << L"**";
        break;
    case LSQUAREBRACE:
        out << L"[";
        break;
    case RSQUAREBRACE:
        out << L"]";
        break;
    case IDENTIFIER:
        out << std::get<std::wstring>(token.getValue());
        break;
    case STR_LITERAL:
        out << L"\"" << std::get<std::wstring>(token.getValue()) << L"\"";
        break;
    case INT_LITERAL:
        out << std::get<int32_t>(token.getValue());
        break;
    case FLOAT_LITERAL:
        out << std::get<double>(token.getValue());
        break;
    case TRUE_LITERAL:
        out << L"true";
        break;
    case FALSE_LITERAL:
        out << L"false";
        break;
    case COMMENT:
        out << L"#" << std::get<std::wstring>(token.getValue());
        break;
    case EOT:
        out << L"end of text";
        break;
    }
    return out;
}
