#include <istream>

#define ETX wchar_t(3)

class Reader
{
public:
    Reader(std::wistream &source);
    void next();
    wchar_t get();
private:
    std::wistream &source;
    wchar_t current;
};
