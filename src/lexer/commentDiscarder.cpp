#include "commentDiscarder.hpp"

CommentDiscarder::CommentDiscarder(ILexer &lexer): lexer(lexer) {}

std::wstring CommentDiscarder::getSourceName()
{
    return lexer.getSourceName();
}

Token CommentDiscarder::getNextToken()
{
    Token returned = lexer.getNextToken();
    while(returned.getType() == TokenType::COMMENT)
        returned = lexer.getNextToken();
    return returned;
}
