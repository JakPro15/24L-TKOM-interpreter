#ifndef ILEXER_HPP
#define ILEXER_HPP

#include "token.hpp"

class ILexer
{
public:
    virtual Token getNextToken() = 0;
};

#endif
