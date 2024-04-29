struct Literal;
struct IsExpression;
struct OrExpression;
struct XorExpression;
struct AndExpression;
struct EqualExpression;
struct NotEqualExpression;
struct IdenticalExpression;
struct NotIdenticalExpression;
struct ConcatExpression;
struct StringMultiplyExpression;
struct GreaterExpression;
struct LesserExpression;
struct GreaterEqualExpression;
struct LesserEqualExpression;
struct PlusExpression;
struct MinusExpression;
struct MultiplyExpression;
struct DivideExpression;
struct FloorDivideExpression;
struct ModuloExpression;
struct ExponentExpression;
struct UnaryMinusExpression;
struct NotExpression;
struct SubscriptExpression;
struct DotExpression;
struct StructExpression;
struct VariableDeclaration;
struct Assignable;
struct AssignmentStatement;
struct FunctionCall;
struct ReturnStatement;
struct ContinueStatement;
struct BreakStatement;
struct IfStatement;
struct WhileStatement;
struct DoWhileStatement;
struct FunctionIdentification;
struct Field;
struct FunctionDeclaration;
struct IncludeStatement;
struct Program;

class IVisitor
{
public:
    virtual ~IVisitor() = default;
    virtual void visit(Literal &visited) = 0;
    virtual void visit(IsExpression &visited) = 0;
    virtual void visit(OrExpression &visited) = 0;
    virtual void visit(XorExpression &visited) = 0;
    virtual void visit(AndExpression &visited) = 0;
    virtual void visit(EqualExpression &visited) = 0;
    virtual void visit(NotEqualExpression &visited) = 0;
    virtual void visit(IdenticalExpression &visited) = 0;
    virtual void visit(NotIdenticalExpression &visited) = 0;
    virtual void visit(ConcatExpression &visited) = 0;
    virtual void visit(StringMultiplyExpression &visited) = 0;
    virtual void visit(GreaterExpression &visited) = 0;
    virtual void visit(LesserExpression &visited) = 0;
    virtual void visit(GreaterEqualExpression &visited) = 0;
    virtual void visit(LesserEqualExpression &visited) = 0;
    virtual void visit(PlusExpression &visited) = 0;
    virtual void visit(MinusExpression &visited) = 0;
    virtual void visit(MultiplyExpression &visited) = 0;
    virtual void visit(DivideExpression &visited) = 0;
    virtual void visit(FloorDivideExpression &visited) = 0;
    virtual void visit(ModuloExpression &visited) = 0;
    virtual void visit(ExponentExpression &visited) = 0;
    virtual void visit(UnaryMinusExpression &visited) = 0;
    virtual void visit(NotExpression &visited) = 0;
    virtual void visit(SubscriptExpression &visited) = 0;
    virtual void visit(DotExpression &visited) = 0;
    virtual void visit(StructExpression &visited) = 0;
    virtual void visit(VariableDeclaration &visited) = 0;
    virtual void visit(Assignable &visited) = 0;
    virtual void visit(AssignmentStatement &visited) = 0;
    virtual void visit(FunctionCall &visited) = 0;
    virtual void visit(ReturnStatement &visited) = 0;
    virtual void visit(ContinueStatement &visited) = 0;
    virtual void visit(BreakStatement &visited) = 0;
    virtual void visit(IfStatement &visited) = 0;
    virtual void visit(WhileStatement &visited) = 0;
    virtual void visit(DoWhileStatement &visited) = 0;
    virtual void visit(FunctionIdentification &visited) = 0;
    virtual void visit(Field &visited) = 0;
    virtual void visit(FunctionDeclaration &visited) = 0;
    virtual void visit(IncludeStatement &visited) = 0;
    virtual void visit(Program &visited) = 0;
};
