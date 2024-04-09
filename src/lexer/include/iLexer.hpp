#ifndef ILEXER_HPP
#define ILEXER_HPP

#include "token.hpp"

class ILexer
{
public:
    // Returns next token constructed from input. After input ends, returns EOT token.
    virtual Token getNextToken() = 0;

    virtual ~ILexer() {}
};

#endif
