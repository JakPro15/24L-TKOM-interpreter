#include "position.hpp"

Position::Position(unsigned line, unsigned column): line(line), column(column) {}

bool Position::operator==(const Position &other) const
{
    return this->line == other.line && this->column == other.column;
}
