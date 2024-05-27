#include <iostream>

class Interpreter
{
public:
    Interpreter(std::istream &input, std::ostream &output);
private:
    std::istream &input;
    std::ostream &output;
};
