#include "parser.hpp"

#include "parserExceptions.hpp"

#include <unordered_map>

using enum TokenType;

Parser::Parser(ILexer &source): source(source), current(source.getNextToken()), next(source.getNextToken()) {}

void Parser::advance()
{
    this->current = this->next;
    this->next = source.getNextToken();
}

void Parser::checkAndAdvance(TokenType type)
{
    checkAndAdvance(type, std::format(L"Expected {}, got {}", type, current));
}

void Parser::checkAndAdvance(TokenType type, std::wstring errorMessage)
{
    if(current.getType() != type)
        throw SyntaxError(errorMessage, current.getPosition());
    advance();
}

std::wstring Parser::loadAndAdvance(TokenType type)
{
    if(current.getType() != type)
        throw SyntaxError(std::format(L"Expected {}, got {}", type, current), current.getPosition());
    std::wstring loaded = std::get<std::wstring>(current.getValue());
    advance();
    return loaded;
}

// PROGRAM = { TOP_STMT } ;
Program Parser::parseProgram()
{
    Program program(current.getPosition());
    while(tryAddTopLevelStatement(program))
        ;
    return program;
}

// TOP_STMT = INCLUDE_STMT
//          | STRUCT_DECL
//          | VARIANT_DECL
//          | FUNCTION_DECL ;
bool Parser::tryAddTopLevelStatement(Program &program)
{
    std::optional<IncludeStatement> includeBuilt;
    if((includeBuilt = parseIncludeStatement()))
    {
        program.includes.push_back(*includeBuilt);
        return true;
    }
    std::optional<std::pair<std::wstring, StructDeclaration>> structBuilt;
    if((structBuilt = parseStructDeclaration()))
    {
        program.structs.insert(std::move(*structBuilt));
        return true;
    }
    std::optional<std::pair<std::wstring, VariantDeclaration>> variantBuilt;
    if((variantBuilt = parseVariantDeclaration()))
    {
        program.variants.insert(std::move(*variantBuilt));
        return true;
    }
    std::optional<std::pair<FunctionIdentification, FunctionDeclaration>> functionBuilt;
    if((functionBuilt = parseFunctionDeclaration()))
    {
        program.functions.insert(std::move(*functionBuilt));
        return true;
    }
    return false;
}

// INCLUDE_STMT = 'include', STRING_LITERAL ;
std::optional<IncludeStatement> Parser::parseIncludeStatement()
{
    if(current.getType() != KW_INCLUDE)
        return std::nullopt;
    Position begin = current.getPosition();
    advance();

    std::wstring filePath = loadAndAdvance(STR_LITERAL);
    checkAndAdvance(SEMICOLON);
    return IncludeStatement(begin, filePath);
}

// STRUCT_DECL = 'struct', DECL_BLOCK ;
std::optional<std::pair<std::wstring, StructDeclaration>> Parser::parseStructDeclaration()
{
    if(current.getType() != KW_STRUCT)
        return std::nullopt;
    Position begin = current.getPosition();
    advance();

    auto [structName, fields] = parseDeclarationBlock();
    return std::pair{structName, StructDeclaration(begin, fields)};
}

// VARIANT_DECL = 'variant', DECL_BLOCK ;
std::optional<std::pair<std::wstring, VariantDeclaration>> Parser::parseVariantDeclaration()
{
    if(current.getType() != KW_VARIANT)
        return std::nullopt;
    Position begin = current.getPosition();
    advance();

    auto [variantName, fields] = parseDeclarationBlock();
    return std::pair{variantName, VariantDeclaration(begin, fields)};
}

// DECL_BLOCK = IDENTIFIER, '{', FIELD_DECL, { FIELD_DECL } , '}' ;
std::pair<std::wstring, std::vector<Field>> Parser::parseDeclarationBlock()
{
    std::wstring name = loadAndAdvance(IDENTIFIER);
    checkAndAdvance(LBRACE);

    std::vector<Field> fields;
    std::optional<Field> fieldBuilt = parseField();
    if(!fieldBuilt.has_value())
        throw SyntaxError(std::format(L"Expected a type identifier, got {}", current), current.getPosition());
    do
    {
        fields.push_back(*fieldBuilt);
    }
    while((fieldBuilt = parseField()));

    checkAndAdvance(RBRACE);
    return {name, fields};
}

// FIELD_DECL = TYPE_IDENT, IDENTIFIER, ';' ;
std::optional<Field> Parser::parseField()
{
    Position begin = current.getPosition();
    std::optional<std::wstring> type;
    if(!(type = parseTypeIdentifier()))
        return std::nullopt;

    std::wstring name = loadAndAdvance(IDENTIFIER);
    checkAndAdvance(SEMICOLON);
    return Field(begin, *type, name);
}

// TYPE_IDENT = BUILTIN_TYPE
//            | IDENTIFIER ;
std::optional<std::wstring> Parser::parseTypeIdentifier()
{
    std::optional<std::wstring> builtinType;
    if((builtinType = parseBuiltinType()))
        return *builtinType;
    if(current.getType() != IDENTIFIER)
        return std::nullopt;
    std::wstring type = std::get<std::wstring>(current.getValue());
    advance();
    return type;
}

// BUILTIN_TYPE = 'int'
//              | 'float'
//              | 'str'
//              | 'bool' ;
std::optional<std::wstring> Parser::parseBuiltinType()
{
    if(current.getType() != KW_INT && current.getType() != KW_FLOAT && current.getType() != KW_STR &&
       current.getType() != KW_BOOL)
        return std::nullopt;
    std::wstring typeAsString = std::format(L"{}", current);
    advance();
    return typeAsString;
}

// FUNCTION_DECL = 'func', IDENTIFIER, '(', [ PARAMETERS ], ')', [ '->', TYPE_IDENT ] , INSTR_BLOCK ;
std::optional<std::pair<FunctionIdentification, FunctionDeclaration>> Parser::parseFunctionDeclaration()
{
    if(current.getType() != KW_FUNC)
        return std::nullopt;
    Position begin = current.getPosition();
    advance();

    std::wstring name = loadAndAdvance(IDENTIFIER);
    checkAndAdvance(LPAREN);
    std::vector<VariableDeclaration> parameters = parseParametersDeclaration();
    checkAndAdvance(RPAREN);
    std::optional<std::wstring> returnType;
    if(current.getType() == ARROW)
    {
        advance();
        if(!(returnType = parseTypeIdentifier()))
            throw SyntaxError(
                std::format(L"Expected type identifier, got {}", current.getType()), current.getPosition()
            );
    }
    std::vector<std::unique_ptr<Instruction>> body = parseInstructionBlock();
    std::vector<std::wstring> parameterTypes;
    for(VariableDeclaration parameter: parameters)
        parameterTypes.push_back(parameter.type);

    return std::pair{
        FunctionIdentification(name, parameterTypes),
        FunctionDeclaration(begin, parameters, returnType, std::move(body))
    };
}

// PARAMETERS = VARIABLE_DECL, {',', VARIABLE_DECL};
std::vector<VariableDeclaration> Parser::parseParametersDeclaration()
{
    std::optional<VariableDeclaration> parameter;
    if(!(parameter = parseVariableDeclaration()))
        return {};
    std::vector<VariableDeclaration> parameters{*parameter};
    while(current.getType() == COMMA)
    {
        advance();
        if(!(parameter = parseVariableDeclaration()))
            throw SyntaxError(
                std::format(L"Expected type identifier, got {}", current.getType()), current.getPosition()
            );
        parameters.push_back(*parameter);
    }
    return parameters;
}

// VARIABLE_DECL = TYPE_IDENT, VAR_DECL_BODY ;
std::optional<VariableDeclaration> Parser::parseVariableDeclaration()
{
    Position begin = current.getPosition();
    std::optional<std::wstring> type;
    if(!(type = parseTypeIdentifier()))
        return std::nullopt;

    auto [isMutable, name] = parseVariableDeclarationBody();
    return VariableDeclaration(begin, *type, name, isMutable);
}

// VAR_DECL_BODY = IDENTIFIER
//               | '$', IDENTIFIER ;
std::pair<bool, std::wstring> Parser::parseVariableDeclarationBody()
{
    bool isMutable = false;
    if(current.getType() == DOLLAR_SIGN)
    {
        isMutable = true;
        advance();
    }
    std::wstring name = loadAndAdvance(IDENTIFIER);
    return {isMutable, name};
}

// INSTR_BLOCK = '{', { INSTRUCTION } , '}' ;
std::vector<std::unique_ptr<Instruction>> Parser::parseInstructionBlock()
{
    checkAndAdvance(LBRACE);
    std::vector<std::unique_ptr<Instruction>> instructions;
    std::unique_ptr<Instruction> instruction;
    while((instruction = parseInstruction()))
        instructions.push_back(std::move(instruction));
    checkAndAdvance(RBRACE, std::format(L"Expected instruction or }}, got {}", current));
    return instructions;
}

// INSTRUCTION =   IDENTIFIER, DECL_OR_ASSIGN_OR_FUNCALL, ';'
//               | BUILTIN_DECL
//               | RETURN_STMT
//               | 'continue', ';'
//               | 'break', ';'
//               | IF_STMT
//               | WHILE_STMT
//               | DO_WHILE_STMT ;
std::unique_ptr<Instruction> Parser::parseInstruction()
{
    std::unique_ptr<Instruction> instruction;
    if((instruction = parseContinueStatement()) || (instruction = parseBreakStatement()) ||
       (instruction = parseReturnStatement()) || (instruction = parseDeclOrAssignOrFunCall()) ||
       (instruction = parseBuiltinDeclStatement()) || (instruction = parseIfStatement()) ||
       (instruction = parseWhileStatement()) || (instruction = parseDoWhileStatement()))
        return instruction;
    return std::unique_ptr<Instruction>(nullptr);
}

std::unique_ptr<Instruction> Parser::parseDeclOrAssignOrFunCall()
{
    if(current.getType() != IDENTIFIER)
        return std::unique_ptr<Instruction>(nullptr);
    Token firstIdentifier = current;
    advance();

    std::unique_ptr<Instruction> instruction;
    if(!(instruction = parseVariableDeclStatement(firstIdentifier)) &&
       !(instruction = parseAssignmentStatement(firstIdentifier)) &&
       !(instruction = parseFunctionCall(firstIdentifier)))
        throw SyntaxError(
            std::format(L"Expected a variable declaration, an assignment or a function call, got {}", current),
            current.getPosition()
        );
    checkAndAdvance(SEMICOLON);
    return instruction;
}

// IDENTIFIER, NO_TYPE_DECL
std::unique_ptr<VariableDeclStatement> Parser::parseVariableDeclStatement(Token firstToken)
{
    if(current.getType() != DOLLAR_SIGN && current.getType() != IDENTIFIER)
        return std::unique_ptr<VariableDeclStatement>(nullptr);

    Position begin = firstToken.getPosition();
    std::wstring type = std::get<std::wstring>(firstToken.getValue());
    auto [isMutable, name, value] = parseNoTypeDecl();

    return std::make_unique<VariableDeclStatement>(
        begin, VariableDeclaration(begin, type, name, isMutable), std::move(value)
    );
}

// BUILTIN_DECL =  BUILTIN_TYPE, NO_TYPE_DECL, ';' ;
std::unique_ptr<VariableDeclStatement> Parser::parseBuiltinDeclStatement()
{
    Position begin = current.getPosition();
    std::optional<std::wstring> type;
    if(!(type = parseBuiltinType()))
        return std::unique_ptr<VariableDeclStatement>(nullptr);
    auto [isMutable, name, value] = parseNoTypeDecl();
    checkAndAdvance(SEMICOLON);

    return std::make_unique<VariableDeclStatement>(
        begin, VariableDeclaration(begin, *type, name, isMutable), std::move(value)
    );
}

// NO_TYPE_DECL = VAR_DECL_BODY, '=', EXPRESSION ;
std::tuple<bool, std::wstring, std::unique_ptr<Expression>> Parser::parseNoTypeDecl()
{
    auto [isMutable, name] = parseVariableDeclarationBody();
    checkAndAdvance(OP_ASSIGN);
    std::unique_ptr<Expression> value;
    if(!(value = parseExpression()))
        throw SyntaxError(std::format(L"Expected expression, got {}", current), current.getPosition());
    return std::tuple{isMutable, name, std::move(value)};
}

// IDENTIFIER, { '.', IDENTIFIER }, '=', EXPRESSION
std::unique_ptr<AssignmentStatement> Parser::parseAssignmentStatement(Token firstToken)
{
    if(current.getType() != OP_DOT && current.getType() != OP_ASSIGN)
        return std::unique_ptr<AssignmentStatement>(nullptr);

    Position begin = firstToken.getPosition();
    Assignable leftAssignable(begin, std::get<std::wstring>(firstToken.getValue()));
    while(current.getType() == OP_DOT)
    {
        advance();
        std::wstring right = loadAndAdvance(IDENTIFIER);
        leftAssignable = Assignable(begin, std::make_unique<Assignable>(std::move(leftAssignable)), right);
    }
    checkAndAdvance(OP_ASSIGN);

    std::unique_ptr<Expression> value;
    if(!(value = parseExpression()))
        throw SyntaxError(std::format(L"Expected expression, got {}", current), current.getPosition());
    return std::make_unique<AssignmentStatement>(begin, std::move(leftAssignable), std::move(value));
}

// IDENTIFIER, '(', [ EXPRESSION, { ',', EXPRESSION } ] , ')'
std::unique_ptr<FunctionCall> Parser::parseFunctionCall(Token functionNameToken)
{
    if(current.getType() != LPAREN)
        return std::unique_ptr<FunctionCall>(nullptr);
    advance();

    Position begin = functionNameToken.getPosition();
    std::wstring name = std::get<std::wstring>(functionNameToken.getValue());

    std::vector<std::unique_ptr<Expression>> parameters = parseParameters();
    checkAndAdvance(RPAREN);
    return std::make_unique<FunctionCall>(begin, name, std::move(parameters));
}

// [ EXPRESSION, { ',', EXPRESSION } ]
std::vector<std::unique_ptr<Expression>> Parser::parseParameters()
{
    std::vector<std::unique_ptr<Expression>> parameters;
    std::unique_ptr<Expression> parameterBuilt;
    if((parameterBuilt = parseExpression()))
    {
        parameters.push_back(std::move(parameterBuilt));
        while(current.getType() == COMMA)
        {
            advance();
            if(!(parameterBuilt = parseExpression()))
                throw SyntaxError(std::format(L"Expected expression, got {}", current), current.getPosition());
            parameters.push_back(std::move(parameterBuilt));
        }
    }
    return parameters;
}

// 'continue', ';'
std::unique_ptr<ContinueStatement> Parser::parseContinueStatement()
{
    if(current.getType() != KW_CONTINUE)
        return std::unique_ptr<ContinueStatement>(nullptr);

    Position begin = current.getPosition();
    advance();
    checkAndAdvance(SEMICOLON);
    return std::make_unique<ContinueStatement>(begin);
}

// 'break', ';'
std::unique_ptr<BreakStatement> Parser::parseBreakStatement()
{
    if(current.getType() != KW_BREAK)
        return std::unique_ptr<BreakStatement>(nullptr);

    Position begin = current.getPosition();
    advance();
    checkAndAdvance(SEMICOLON);
    return std::make_unique<BreakStatement>(begin);
}

// RETURN_STMT = 'return', [ EXPRESSION ] , ';' ;
std::unique_ptr<ReturnStatement> Parser::parseReturnStatement()
{
    if(current.getType() != KW_RETURN)
        return std::unique_ptr<ReturnStatement>(nullptr);

    Position begin = current.getPosition();
    advance();
    std::unique_ptr<Expression> returnValue = parseExpression();
    checkAndAdvance(SEMICOLON);
    return std::make_unique<ReturnStatement>(begin, std::move(returnValue));
}

// IF_STMT = 'if', '(', IF_CONDITION, ')', INSTR_BLOCK,
//           { 'elif', '(', IF_CONDITION, ')', INSTR_BLOCK } ,
//           [ 'else', INSTR_BLOCK ] ;
std::unique_ptr<IfStatement> Parser::parseIfStatement()
{
    if(current.getType() != KW_IF)
        return std::unique_ptr<IfStatement>(nullptr);
    Position begin = current.getPosition();

    std::vector<SingleIfCase> cases;
    do
    {
        Position caseBegin = current.getPosition();
        advance();
        checkAndAdvance(LPAREN);
        std::variant<VariableDeclStatement, std::unique_ptr<Expression>> condition = parseIfCondition();
        checkAndAdvance(RPAREN);
        std::vector<std::unique_ptr<Instruction>> body = parseInstructionBlock();
        cases.emplace_back(caseBegin, std::move(condition), std::move(body));
    }
    while(current.getType() == KW_ELIF);

    std::vector<std::unique_ptr<Instruction>> elseCaseBody;
    if(current.getType() == KW_ELSE)
    {
        advance();
        elseCaseBody = parseInstructionBlock();
    }
    return std::make_unique<IfStatement>(begin, std::move(cases), std::move(elseCaseBody));
}

// IF_CONDITION = EXPRESSION
//              | VARIABLE_DECL, '=', EXPRESSION ;
std::variant<VariableDeclStatement, std::unique_ptr<Expression>> Parser::parseIfCondition()
{
    if(current.getType() == IDENTIFIER && (next.getType() == IDENTIFIER || next.getType() == DOLLAR_SIGN))
    { // special case of looking at next token for disambiguation,
      // as both EXPRESSION and VARIABLE_DECL may begin with IDENTIFIER
        return parseIfConditionDeclaration();
    }
    std::unique_ptr<Expression> condition;
    if((condition = parseExpression()))
        return condition;
    return parseIfConditionDeclaration();
}

// VARIABLE_DECL, '=', EXPRESSION
VariableDeclStatement Parser::parseIfConditionDeclaration()
{
    Position begin = current.getPosition();
    std::optional<std::wstring> type;
    if(!(type = parseTypeIdentifier()))
        throw SyntaxError(
            std::format(L"Expected a variable declaration or expression, got {}", current), current.getPosition()
        );
    auto [isMutable, name, value] = parseNoTypeDecl();
    return VariableDeclStatement(begin, VariableDeclaration(begin, *type, name, isMutable), std::move(value));
}

// WHILE_STMT = 'while', '(', EXPRESSION, ')', INSTR_BLOCK ;
std::unique_ptr<WhileStatement> Parser::parseWhileStatement()
{
    if(current.getType() != KW_WHILE)
        return std::unique_ptr<WhileStatement>(nullptr);
    Position begin = current.getPosition();
    advance();
    checkAndAdvance(LPAREN);
    std::unique_ptr<Expression> condition;
    if(!(condition = parseExpression()))
        throw SyntaxError(std::format(L"Expected expression, got {}", current), current.getPosition());
    checkAndAdvance(RPAREN);
    std::vector<std::unique_ptr<Instruction>> body = parseInstructionBlock();
    return std::make_unique<WhileStatement>(begin, std::move(condition), std::move(body));
}

// DO_WHILE_STMT = 'do', INSTR_BLOCK, 'while', '(', EXPRESSION, ')' ;
std::unique_ptr<DoWhileStatement> Parser::parseDoWhileStatement()
{
    if(current.getType() != KW_DO)
        return std::unique_ptr<DoWhileStatement>(nullptr);
    Position begin = current.getPosition();
    advance();
    std::vector<std::unique_ptr<Instruction>> body = parseInstructionBlock();
    checkAndAdvance(KW_WHILE);
    checkAndAdvance(LPAREN);
    std::unique_ptr<Expression> condition;
    if(!(condition = parseExpression()))
        throw SyntaxError(std::format(L"Expected expression, got {}", current), current.getPosition());
    checkAndAdvance(RPAREN);
    return std::make_unique<DoWhileStatement>(begin, std::move(condition), std::move(body));
}

// EXPRESSION = XOR_EXPR, { 'or', XOR_EXPR } ;
std::unique_ptr<Expression> Parser::parseExpression()
{
    Position begin = current.getPosition();
    std::unique_ptr<Expression> left = parseXorExpression();
    while(current.getType() == KW_OR)
    {
        std::unique_ptr<Expression> right = parseXorExpression();
        left = std::make_unique<OrExpression>(begin, std::move(left), std::move(right));
    }
    return left;
}

// XOR_EXPR = AND_EXPR, { 'xor', AND_EXPR } ;
std::unique_ptr<Expression> Parser::parseXorExpression()
{
    Position begin = current.getPosition();
    std::unique_ptr<Expression> left = parseAndExpression();
    while(current.getType() == KW_XOR)
    {
        std::unique_ptr<Expression> right = parseAndExpression();
        left = std::make_unique<XorExpression>(begin, std::move(left), std::move(right));
    }
    return left;
}

// AND_EXPR = EQUALITY_EXPR, { 'and', EQUALITY_EXPR } ;
std::unique_ptr<Expression> Parser::parseAndExpression()
{
    Position begin = current.getPosition();
    std::unique_ptr<Expression> left = parseEqualityExpression();
    while(current.getType() == KW_AND)
    {
        std::unique_ptr<Expression> right = parseEqualityExpression();
        left = std::make_unique<AndExpression>(begin, std::move(left), std::move(right));
    }
    return left;
}

// EQUALITY_EXPR = CONCAT_EXPR, [ EQUALITY_OP, CONCAT_EXPR ] ;
// EQUALITY_OP = '=='
//             | '!='
//             | '==='
//             | '!==' ;
std::unique_ptr<Expression> Parser::parseEqualityExpression()
{
    Position begin = current.getPosition();
    std::unique_ptr<Expression> left = parseConcatExpression();
    if(current.getType() == OP_EQUAL || current.getType() == OP_NOT_EQUAL || current.getType() == OP_IDENTICAL ||
       current.getType() == OP_NOT_IDENTICAL)
    {
        TokenType operation = current.getType();
        advance();
        std::unique_ptr<Expression> right = parseConcatExpression();
        if(operation == OP_EQUAL)
            left = std::make_unique<EqualExpression>(begin, std::move(left), std::move(right));
        else if(operation == OP_NOT_EQUAL)
            left = std::make_unique<NotEqualExpression>(begin, std::move(left), std::move(right));
        else if(operation == OP_IDENTICAL)
            left = std::make_unique<IdenticalExpression>(begin, std::move(left), std::move(right));
        else if(operation == OP_NOT_IDENTICAL)
            left = std::make_unique<NotIdenticalExpression>(begin, std::move(left), std::move(right));
    }
    return left;
}

// CONCAT_EXPR = STR_MUL_EXPR, { '!', STR_MUL_EXPR } ;
std::unique_ptr<Expression> Parser::parseConcatExpression()
{
    Position begin = current.getPosition();
    std::unique_ptr<Expression> left = parseStringMultiplyExpression();
    while(current.getType() == OP_CONCAT)
    {
        std::unique_ptr<Expression> right = parseStringMultiplyExpression();
        left = std::make_unique<ConcatExpression>(begin, std::move(left), std::move(right));
    }
    return left;
}

// STR_MUL_EXPR = COMPARE_EXPR, { '@', COMPARE_EXPR } ;
std::unique_ptr<Expression> Parser::parseStringMultiplyExpression()
{
    Position begin = current.getPosition();
    std::unique_ptr<Expression> left = parseCompareExpression();
    while(current.getType() == OP_STR_MULTIPLY)
    {
        std::unique_ptr<Expression> right = parseCompareExpression();
        left = std::make_unique<StringMultiplyExpression>(begin, std::move(left), std::move(right));
    }
    return left;
}

// COMPARE_EXPR = ADDITIVE_EXPR, [ COMPARISON_OP, ADDITIVE_EXPR ] ;
// COMPARISON_OP = '>'
//               | '<'
//               | '>='
//               | '<=' ;
std::unique_ptr<Expression> Parser::parseCompareExpression()
{
    Position begin = current.getPosition();
    std::unique_ptr<Expression> left = parseAdditiveExpression();
    if(current.getType() == OP_GREATER || current.getType() == OP_LESSER || current.getType() == OP_GREATER_EQUAL ||
       current.getType() == OP_LESSER_EQUAL)
    {
        TokenType operation = current.getType();
        advance();
        std::unique_ptr<Expression> right = parseAdditiveExpression();
        if(operation == OP_GREATER)
            left = std::make_unique<GreaterExpression>(begin, std::move(left), std::move(right));
        else if(operation == OP_LESSER)
            left = std::make_unique<LesserExpression>(begin, std::move(left), std::move(right));
        else if(operation == OP_GREATER_EQUAL)
            left = std::make_unique<GreaterEqualExpression>(begin, std::move(left), std::move(right));
        else if(operation == OP_LESSER_EQUAL)
            left = std::make_unique<LesserEqualExpression>(begin, std::move(left), std::move(right));
    }
    return left;
}

// ADDITIVE_EXPR = TERM, { ADDITIVE_OP, TERM } ;
// ADDITIVE_OP = '+'
//             | '-' ;
std::unique_ptr<Expression> Parser::parseAdditiveExpression()
{
    Position begin = current.getPosition();
    std::unique_ptr<Expression> left = parseMultiplicativeExpression();
    while(current.getType() == OP_PLUS || current.getType() == OP_MINUS)
    {
        TokenType operation = current.getType();
        advance();
        std::unique_ptr<Expression> right = parseMultiplicativeExpression();
        if(operation == OP_PLUS)
            left = std::make_unique<PlusExpression>(begin, std::move(left), std::move(right));
        else if(operation == OP_MINUS)
            left = std::make_unique<MinusExpression>(begin, std::move(left), std::move(right));
    }
    return left;
}

// TERM = FACTOR, { MULTIPL_OP, FACTOR } ;
// MULTIPL_OP =    '*'
//               | '/'
//               | '//'
//               | '%' ;
std::unique_ptr<Expression> Parser::parseMultiplicativeExpression()
{
    Position begin = current.getPosition();
    std::unique_ptr<Expression> left = parseExponentExpression();
    while(current.getType() == OP_MULTIPLY || current.getType() == OP_DIVIDE || current.getType() == OP_FLOOR_DIVIDE ||
          current.getType() == OP_MODULO)
    {
        TokenType operation = current.getType();
        advance();
        std::unique_ptr<Expression> right = parseExponentExpression();
        if(operation == OP_MULTIPLY)
            left = std::make_unique<MultiplyExpression>(begin, std::move(left), std::move(right));
        else if(operation == OP_DIVIDE)
            left = std::make_unique<DivideExpression>(begin, std::move(left), std::move(right));
        else if(operation == OP_FLOOR_DIVIDE)
            left = std::make_unique<FloorDivideExpression>(begin, std::move(left), std::move(right));
        else if(operation == OP_MODULO)
            left = std::make_unique<ModuloExpression>(begin, std::move(left), std::move(right));
    }
    return left;
}

// FACTOR = UNARY_EXPR, { '**', UNARY_EXPR } ;
std::unique_ptr<Expression> Parser::parseExponentExpression()
{
    Position begin = current.getPosition();
    std::unique_ptr<Expression> left = parseUnaryExpression();
    while(current.getType() == OP_EXPONENT)
    {
        std::unique_ptr<Expression> right = parseUnaryExpression();
        left = std::make_unique<ExponentExpression>(begin, std::move(left), std::move(right));
    }
    return left;
}

// UNARY_EXPR = { UNARY_OP } , IS_EXPR ;
// UNARY_OP = '-'
//          | 'not' ;
std::unique_ptr<Expression> Parser::parseUnaryExpression()
{
    Position begin = current.getPosition();
    if(current.getType() == OP_MINUS || current.getType() == KW_NOT)
    {
        TokenType operation = current.getType();
        advance();
        std::unique_ptr<Expression> right = parseUnaryExpression();
        if(operation == OP_MINUS)
            return std::make_unique<UnaryMinusExpression>(begin, std::move(right));
        if(operation == KW_NOT)
            return std::make_unique<NotExpression>(begin, std::move(right));
    }
    return parseIsExpression();
}

// IS_EXPR = SUBSCRPT_EXPR, [ 'is', TYPE_IDENT ] ;
std::unique_ptr<Expression> Parser::parseIsExpression()
{
    Position begin = current.getPosition();
    std::unique_ptr<Expression> left = parseSubscriptExpression();
    if(current.getType() == KW_IS)
    {
        advance();
        std::optional<std::wstring> right;
        if(!(right = parseTypeIdentifier()))
            throw SyntaxError(std::format(L"Expected type identifier, got {}", current), current.getPosition());
        return std::make_unique<IsExpression>(begin, std::move(left), *right);
    }
    return left;
}

// SUBSCRPT_EXPR = DOT_EXPR, { '[', EXPRESSION, ']' } ;
std::unique_ptr<Expression> Parser::parseSubscriptExpression()
{
    Position begin = current.getPosition();
    std::unique_ptr<Expression> left = parseDotExpression();
    while(current.getType() == LSQUAREBRACE)
    {
        advance();
        std::unique_ptr<Expression> right = parseExpression();
        checkAndAdvance(RSQUAREBRACE);
        left = std::make_unique<SubscriptExpression>(begin, std::move(left), std::move(right));
    }
    return left;
}

// DOT_EXPR = STRUCT_EXPR, { '.', IDENTIFIER } ;
std::unique_ptr<Expression> Parser::parseDotExpression()
{
    Position begin = current.getPosition();
    std::unique_ptr<Expression> left = parseStructExpression();
    while(current.getType() == OP_DOT)
    {
        advance();
        std::wstring right = loadAndAdvance(IDENTIFIER);
        left = std::make_unique<DotExpression>(begin, std::move(left), right);
    }
    return left;
}

// STRUCT_EXPR = '{', EXPRESSION, { ',', EXPRESSION } , '}'
//             | PARENTH_EXPR ;
std::unique_ptr<Expression> Parser::parseStructExpression()
{
    if(current.getType() == LBRACE)
    {
        Position begin = current.getPosition();
        advance();
        std::vector<std::unique_ptr<Expression>> parameters = parseParameters();
        if(parameters.size() < 1)
            throw SyntaxError(L"Expected at least 1 parameter in StructExpression", begin);
        checkAndAdvance(RBRACE);
        return std::make_unique<StructExpression>(begin, std::move(parameters));
    }
    return parseParenthExpression();
}

// PARENTH_EXPR =  IDENTIFIER, [ '(', [ EXPRESSION, { ',', EXPRESSION } ] , ')' ]
//               | '(', EXPRESSION, ')'
//               | LITERAL ;
std::unique_ptr<Expression> Parser::parseParenthExpression()
{
    std::unique_ptr<Expression> built;
    if((built = parseVariableOrFunCall()))
        return built;
    if((built = parseExpressionInParentheses()))
        return built;
    return parseLiteral();
}

// IDENTIFIER, [ '(', [ EXPRESSION, { ',', EXPRESSION } ] , ')' ]
std::unique_ptr<Expression> Parser::parseVariableOrFunCall()
{
    if(current.getType() != IDENTIFIER)
        return nullptr;
    Token firstIdentifier = current;
    advance();
    std::unique_ptr<FunctionCall> call;
    if((call = parseFunctionCall(firstIdentifier)))
        return call;
    return std::make_unique<Variable>(
        firstIdentifier.getPosition(), std::get<std::wstring>(firstIdentifier.getValue())
    );
}

// '(', EXPRESSION, ')'
std::unique_ptr<Expression> Parser::parseExpressionInParentheses()
{
    if(current.getType() != LPAREN)
        return nullptr;
    advance();
    std::unique_ptr<Expression> expression = parseExpression();
    if(!expression)
        throw SyntaxError(std::format(L"Expected expression, got {}", current), current.getPosition());
    checkAndAdvance(RPAREN);
    return expression;
}

// LITERAL = STRING_LITERAL
//         | INT_LITERAL
//         | FLOAT_LITERAL
//         | BOOL_LITERAL ;
// BOOL_LITERAL = 'true'
//              | 'false' ;
std::unique_ptr<Literal> Parser::parseLiteral()
{
    std::variant<std::wstring, int32_t, double, bool> value;
    switch(current.getType())
    {
    case STR_LITERAL:
        value = std::get<std::wstring>(current.getValue());
        break;
    case INT_LITERAL:
        value = std::get<int32_t>(current.getValue());
        break;
    case FLOAT_LITERAL:
        value = std::get<double>(current.getValue());
        break;
    case TRUE_LITERAL:
        value = true;
        break;
    case FALSE_LITERAL:
        value = false;
        break;
    default:
        return nullptr;
    }
    Position begin = current.getPosition();
    advance();
    return std::make_unique<Literal>(begin, value);
}
