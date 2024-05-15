#include "tokenType.hpp"

using enum TokenType;

std::wstring toString(TokenType tokenType)
{
    switch(tokenType)
    {
    case KW_INCLUDE:
        return L"include";
    case KW_STRUCT:
        return L"struct";
    case KW_VARIANT:
        return L"variant";
    case KW_FUNC:
        return L"func";
    case KW_CONTINUE:
        return L"continue";
    case KW_BREAK:
        return L"break";
    case KW_RETURN:
        return L"return";
    case KW_IF:
        return L"if";
    case KW_ELIF:
        return L"elif";
    case KW_ELSE:
        return L"else";
    case KW_WHILE:
        return L"while";
    case KW_DO:
        return L"do";
    case KW_IS:
        return L"is";
    case KW_OR:
        return L"or";
    case KW_XOR:
        return L"xor";
    case KW_AND:
        return L"and";
    case KW_NOT:
        return L"not";
    case KW_INT:
        return L"int";
    case KW_FLOAT:
        return L"float";
    case KW_BOOL:
        return L"bool";
    case KW_STR:
        return L"str";
    case LBRACE:
        return L"{";
    case RBRACE:
        return L"}";
    case SEMICOLON:
        return L";";
    case LPAREN:
        return L"(";
    case RPAREN:
        return L")";
    case ARROW:
        return L"->";
    case COMMA:
        return L",";
    case DOLLAR_SIGN:
        return L"$";
    case OP_ASSIGN:
        return L"=";
    case OP_DOT:
        return L".";
    case OP_EQUAL:
        return L"==";
    case OP_NOT_EQUAL:
        return L"!=";
    case OP_IDENTICAL:
        return L"===";
    case OP_NOT_IDENTICAL:
        return L"!==";
    case OP_CONCAT:
        return L"!";
    case OP_STR_MULTIPLY:
        return L"@";
    case OP_GREATER:
        return L">";
    case OP_LESSER:
        return L"<";
    case OP_GREATER_EQUAL:
        return L">=";
    case OP_LESSER_EQUAL:
        return L"<=";
    case OP_PLUS:
        return L"+";
    case OP_MINUS:
        return L"-";
    case OP_MULTIPLY:
        return L"*";
    case OP_DIVIDE:
        return L"/";
    case OP_FLOOR_DIVIDE:
        return L"//";
    case OP_MODULO:
        return L"%";
    case OP_EXPONENT:
        return L"**";
    case LSQUAREBRACE:
        return L"[";
    case RSQUAREBRACE:
        return L"]";
    case IDENTIFIER:
        return L"identifier";
    case STR_LITERAL:
        return L"string literal";
    case INT_LITERAL:
        return L"integer literal";
    case FLOAT_LITERAL:
        return L"floating-point literal";
    case TRUE_LITERAL:
        return L"true";
    case FALSE_LITERAL:
        return L"false";
    case COMMENT:
        return L"comment";
    case EOT:
        return L"end of text";
    default:
        return L"unknown";
    }
}

std::wostream &operator<<(std::wostream &out, TokenType tokenType)
{
    out << toString(tokenType);
    return out;
}
