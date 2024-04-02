#ifndef POSITION_HPP
#define POSITION_HPP

struct Position
{
    unsigned line, column;
    Position(const Position &) = default;
    Position(unsigned line, unsigned column);
    bool operator==(const Position &other) const;
};

#endif
