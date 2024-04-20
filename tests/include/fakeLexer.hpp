#include "iLexer.hpp"

template <typename TokenContainer>
class FakeLexer: public ILexer
{
public:
    FakeLexer(const TokenContainer &tokens): current(std::begin(tokens)), end(std::end(tokens)) {}

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
