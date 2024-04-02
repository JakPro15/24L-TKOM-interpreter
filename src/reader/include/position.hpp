struct Position
{
    unsigned line, column;
    Position(const Position &) = default;
    Position(unsigned line, unsigned column);
    bool operator==(const Position &other) const;
};
