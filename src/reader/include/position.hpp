#ifndef POSITION_HPP
#define POSITION_HPP

#include <compare>

struct Position
{
    unsigned line, column;
    Position() = default;
    Position(const Position &) = default;
    Position(unsigned line, unsigned column);
    auto operator<=>(const Position &other) const = default;
};

#endif
