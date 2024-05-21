#ifndef PARSER_HPP
#define PARSER_HPP

#include "documentTree.hpp"
#include "iLexer.hpp"
#include "parserExceptions.hpp"

class Parser
{
public:
    explicit Parser(ILexer &source);
    Program parseProgram();
private:
    ILexer &source;
    std::wstring sourceName;
    Token current, next;
    void advance();
    void checkAndAdvance(TokenType type);
    void checkAndAdvance(TokenType type, std::wstring errorMessage);
    std::wstring loadAndAdvance(TokenType type);
    auto mustBePresent(auto built, std::wstring_view expectedMessage);
    void checkForEOT();

    std::optional<IncludeStatement> parseIncludeStatement();
    std::optional<std::pair<std::wstring, StructDeclaration>> parseStructDeclaration();
    std::optional<std::pair<std::wstring, VariantDeclaration>> parseVariantDeclaration();
    std::optional<std::pair<FunctionIdentification, FunctionDeclaration>> parseFunctionDeclaration();
    std::pair<std::wstring, std::vector<Field>> parseDeclarationBlock();
    std::optional<Field> parseField();
    std::optional<Type> parseTypeIdentifier();
    std::optional<Type> parseBuiltinType();
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
    std::optional<FunctionCall> parseFunctionCall(Token functionNameToken);
    std::unique_ptr<FunctionCall> parseFunctionCallExpression(Token firstToken);
    std::unique_ptr<FunctionCallInstruction> parseFunctionCallInstruction(Token firstToken);
    std::vector<std::unique_ptr<Expression>> parseArguments();
    std::unique_ptr<ContinueStatement> parseContinueStatement();
    std::unique_ptr<BreakStatement> parseBreakStatement();
    std::unique_ptr<ReturnStatement> parseReturnStatement();
    std::unique_ptr<IfStatement> parseIfStatement();
    std::variant<VariableDeclStatement, std::unique_ptr<Expression>> parseIfCondition();
    VariableDeclStatement parseIfConditionDeclaration();
    std::unique_ptr<WhileStatement> parseWhileStatement();
    std::unique_ptr<DoWhileStatement> parseDoWhileStatement();
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseXorExpression();
    std::unique_ptr<Expression> parseAndExpression();
    std::unique_ptr<Expression> parseEqualityExpression();
    std::unique_ptr<Expression> parseConcatExpression();
    std::unique_ptr<Expression> parseStringMultiplyExpression();
    std::unique_ptr<Expression> parseCompareExpression();
    std::unique_ptr<Expression> parseAdditiveExpression();
    std::unique_ptr<Expression> parseMultiplicativeExpression();
    std::unique_ptr<Expression> parseExponentExpression();
    std::unique_ptr<Expression> parseUnaryExpression();
    std::unique_ptr<Expression> parseIsExpression();
    std::unique_ptr<Expression> parseSubscriptExpression();
    std::unique_ptr<Expression> parseDotExpression();
    std::unique_ptr<Expression> parseStructExpression();
    std::unique_ptr<Expression> parseParenthExpression();
    std::unique_ptr<CastExpression> parseExplicitCast();
    std::unique_ptr<Expression> parseVariableOrFunCall();
    std::unique_ptr<Expression> parseExpressionInParentheses();
    std::unique_ptr<Literal> parseLiteral();
};

#endif
