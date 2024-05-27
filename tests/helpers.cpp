#include "helpers.hpp"

std::unique_ptr<Literal> makeLiteral(Position position, std::variant<std::wstring, int32_t, double, bool> value)
{
    return std::make_unique<Literal>(position, value);
}
