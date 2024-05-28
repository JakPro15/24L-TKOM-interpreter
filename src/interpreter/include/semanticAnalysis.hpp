#ifndef SEMANTICANALYZER_HPP
#define SEMANTICANALYZER_HPP

#include "documentTree.hpp"
#include "documentTreeVisitor.hpp"

Type::Builtin getTargetTypeForEquality(Type::Builtin leftType, Type::Builtin rightType);
void doSemanticAnalysis(Program &program);

#endif
