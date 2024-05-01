#ifndef PARSER_HPP
#define PARSER_HPP

#include "documentTree.hpp"
#include "iLexer.hpp"
#include "parserExceptions.hpp"

class Parser
{
public:
    Parser(ILexer &source);
    Program parseProgram();
private:
    ILexer &source;
    Token current, next;
    void advance();
    void checkAndAdvance(TokenType type);
    void checkAndAdvance(TokenType type, std::wstring errorMessage);
    std::wstring loadAndAdvance(TokenType type);

    bool tryAddTopLevelStatement(Program &program);
    std::optional<IncludeStatement> parseIncludeStatement();
    std::optional<std::pair<std::wstring, StructDeclaration>> parseStructDeclaration();
    std::optional<std::pair<std::wstring, VariantDeclaration>> parseVariantDeclaration();
    std::optional<std::pair<FunctionIdentification, FunctionDeclaration>> parseFunctionDeclaration();
    std::pair<std::wstring, std::vector<Field>> parseDeclarationBlock();
    std::optional<Field> parseField();
    std::optional<std::wstring> parseTypeIdentifier();
    std::optional<std::wstring> parseBuiltinType();
    std::vector<VariableDeclaration> parseParameters();
    std::optional<VariableDeclaration> parseVariableDeclaration();
    std::vector<std::unique_ptr<Instruction>> parseInstructionBlock();
    std::unique_ptr<Instruction> parseInstruction();
    std::pair<bool, std::wstring> parseVariableDeclarationBody();
    std::unique_ptr<Instruction> parseDeclOrAssignOrFunCall();
    std::unique_ptr<VariableDeclStatement> parseVariableDeclStatement(Token firstToken);
    std::unique_ptr<VariableDeclStatement> parseBuiltinDeclStatement();
    std::tuple<bool, std::wstring, std::unique_ptr<Expression>> parseNoTypeDecl();
    std::unique_ptr<AssignmentStatement> parseAssignmentStatement(Token firstToken);
    std::unique_ptr<FunctionCall> parseFunctionCall(Token firstToken);
    std::unique_ptr<ContinueStatement> parseContinueStatement();
    std::unique_ptr<BreakStatement> parseBreakStatement();
    std::unique_ptr<ReturnStatement> parseReturnStatement();
    std::unique_ptr<IfStatement> parseIfStatement();
    std::variant<VariableDeclStatement, std::unique_ptr<Expression>> parseIfCondition();
    VariableDeclStatement parseIfConditionDeclaration();
    std::unique_ptr<WhileStatement> parseWhileStatement();
    std::unique_ptr<DoWhileStatement> parseDoWhileStatement();
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Literal> parseLiteral();
};

#endif
