#include "iLexer.hpp"

class CommentDiscarder: public ILexer
{
public:
    explicit CommentDiscarder(ILexer &lexer);
    Token getNextToken() override;
private:
    ILexer &lexer;
};
