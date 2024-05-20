#include "iLexer.hpp"

class CommentDiscarder: public ILexer
{
public:
    explicit CommentDiscarder(ILexer &lexer);
    std::wstring getSourceName() override;
    Token getNextToken() override;
private:
    ILexer &lexer;
};
