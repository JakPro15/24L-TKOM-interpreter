#include "documentTree.hpp"

typedef std::pair<FunctionIdentification, BuiltinFunctionDeclaration> BuiltinFunction;

BuiltinFunction builtinNoArguments(const std::vector<std::wstring> &arguments);
BuiltinFunction builtinArgument(const std::vector<std::wstring> &arguments);
BuiltinFunction builtinPrint(std::wostream &output);
BuiltinFunction builtinPrintln(std::wostream &output);
BuiltinFunction builtinInputLine(std::wistream &input);
BuiltinFunction builtinInput(std::wistream &input);
extern const BuiltinFunction builtinLen, builtinAbsFloat, builtinAbsInt, builtinMaxFloat, builtinMaxInt,
    builtinMinFloat, builtinMinInt;
