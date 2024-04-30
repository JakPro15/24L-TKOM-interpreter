#include "position.hpp"

Position::Position(unsigned line, unsigned column): line(line), column(column) {}

std::wostream &operator<<(std::wostream &out, const Position &position)
{
    out << L"<line: " << position.line << L", col: " << position.column << L">";
    return out;
}
