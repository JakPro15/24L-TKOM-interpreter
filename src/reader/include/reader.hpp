#include <istream>

#define ETX wchar_t(3)

struct Position
{
    unsigned line, column;
    Position(const Position &) = default;
    Position(unsigned line, unsigned column);
    bool operator==(const Position &other) const;
};

class Reader
{
public:
    Reader(std::wistream &source);

    // Advances to the next wide character of input.
    void next();

    // Returns the current character of input.
    wchar_t get();

    // Returns the position of the current character in input.
    // Line and column numbers start from 1.
    Position getPosition();
private:
    std::wistream &source;
    wchar_t current;
    Position currentPosition;
};
