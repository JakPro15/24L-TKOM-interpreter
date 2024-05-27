#ifndef FAKELEXER_HPP
#define FAKELEXER_HPP

#include "iLexer.hpp"

template <typename TokenContainer>
class FakeLexer: public ILexer
{
public:
    FakeLexer(const TokenContainer &tokens): current(std::begin(tokens)), end(std::end(tokens)) {}

    std::wstring getSourceName() override
    {
        return L"<test>";
    }

    Token getNextToken() override
    {
        if(current != end)
            return *current++;
        else
            return Token(TokenType::EOT, {0, 0});
    }
private:
    decltype(std::begin(std::declval<const TokenContainer &>())) current, end;
};

#endif
