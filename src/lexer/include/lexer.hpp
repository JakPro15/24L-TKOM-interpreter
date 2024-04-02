#ifndef LEXER_HPP
#define LEXER_HPP

#include "ilexer.hpp"
#include "ireader.hpp"

class Lexer: ILexer
{
public:
    Lexer(IReader &reader);
    Token getNextToken() override;
};

#endif
