#include <iostream>
#include <string>
#include <vector>

class Interpreter
{
public:
    Interpreter(std::vector<std::wstring> arguments, std::istream &input, std::ostream &output);
private:
    std::vector<std::wstring> arguments;
    std::istream &input;
    std::ostream &output;
};
