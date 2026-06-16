#pragma once
#include <memory>
#include <string>
#include <vector>
#include "value.hpp"

namespace jsling {

enum class NodeKind {
    Program, Block, VarDecl, FunctionDecl, Return, If, For, While, DoWhile,
    Switch, Break, Continue, ExprStmt,
    Binary, Unary, Update, Assign, Logical, Call, Member, New,
    Array, Object, Conditional, Template, Spread,
    Identifier, Literal, StmtList,
    ObjPattern, ArrPattern
};

struct ASTNode {
    NodeKind kind;
    virtual ~ASTNode() = default;
    explicit ASTNode(NodeKind k) : kind(k) {}
};

struct Program : ASTNode {
    std::vector<std::unique_ptr<ASTNode>> body;
    Program() : ASTNode(NodeKind::Program) {}
};

struct BlockStmt : ASTNode {
    std::vector<std::unique_ptr<ASTNode>> body;
    BlockStmt() : ASTNode(NodeKind::Block) {}
};

// Pattern element for destructuring
struct Pattern {
    std::string name;                        // target variable name (for leaf patterns)
    std::unique_ptr<ASTNode> key;            // key expression (for object patterns)
    std::unique_ptr<ASTNode> defaultVal;     // default value expression
    std::unique_ptr<ASTNode> subPattern;     // nested object/array pattern
    bool isRest = false;                     // ...rest
    bool isHole = false;                     // array hole: [a, , b]
    Pattern() = default;
    Pattern(Pattern&&) = default;
    Pattern& operator=(Pattern&&) = default;
};

struct ObjPatternNode : ASTNode {
    std::vector<Pattern> elements;
    ObjPatternNode() : ASTNode(NodeKind::ObjPattern) {}
};

struct ArrPatternNode : ASTNode {
    std::vector<Pattern> elements;
    ArrPatternNode() : ASTNode(NodeKind::ArrPattern) {}
};

struct VarDecl : ASTNode {
    std::string kind; // "var", "let", "const"
    std::string name;
    std::unique_ptr<ASTNode> initializer;
    std::unique_ptr<ASTNode> pattern; // null for simple, ObjPatternNode or ArrPatternNode for destructuring
    VarDecl() : ASTNode(NodeKind::VarDecl) {}
};

struct FunctionDecl : ASTNode {
    std::string name;
    std::vector<std::string> params;
    std::vector<std::unique_ptr<ASTNode>> defaults; // default value per param (nullptr = no default)
    bool hasRest = false;
    std::string restParam; // name of ...rest parameter
    std::unique_ptr<BlockStmt> body;
    bool isExpression = false; // true for function expressions (const f = function name() {})
    std::vector<std::unique_ptr<ASTNode>> paramPatterns; // destructuring patterns per param (null if simple)
    FunctionDecl() : ASTNode(NodeKind::FunctionDecl) {}
};

struct ReturnStmt : ASTNode {
    std::unique_ptr<ASTNode> argument;
    ReturnStmt() : ASTNode(NodeKind::Return) {}
};

struct IfStmt : ASTNode {
    std::unique_ptr<ASTNode> test;
    std::unique_ptr<ASTNode> consequent;
    std::unique_ptr<ASTNode> alternate;
    IfStmt() : ASTNode(NodeKind::If) {}
};

struct ForStmt : ASTNode {
    std::unique_ptr<ASTNode> init;
    std::unique_ptr<ASTNode> test;
    std::unique_ptr<ASTNode> update;
    std::unique_ptr<ASTNode> body;
    ForStmt() : ASTNode(NodeKind::For) {}
};

struct WhileStmt : ASTNode {
    std::unique_ptr<ASTNode> test;
    std::unique_ptr<ASTNode> body;
    WhileStmt() : ASTNode(NodeKind::While) {}
};

struct DoWhileStmt : ASTNode {
    std::unique_ptr<ASTNode> test;
    std::unique_ptr<ASTNode> body;
    DoWhileStmt() : ASTNode(NodeKind::DoWhile) {}
};

struct SwitchCase {
    std::unique_ptr<ASTNode> test;
    std::vector<std::unique_ptr<ASTNode>> consequent;
};

struct SwitchStmt : ASTNode {
    std::unique_ptr<ASTNode> discriminant;
    std::vector<SwitchCase> cases;
    SwitchStmt() : ASTNode(NodeKind::Switch) {}
};

struct BreakStmt : ASTNode {
    BreakStmt() : ASTNode(NodeKind::Break) {}
};

struct ContinueStmt : ASTNode {
    ContinueStmt() : ASTNode(NodeKind::Continue) {}
};

struct ExprStmt : ASTNode {
    std::unique_ptr<ASTNode> expression;
    ExprStmt() : ASTNode(NodeKind::ExprStmt) {}
};

struct BinaryExpr : ASTNode {
    std::string op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    BinaryExpr() : ASTNode(NodeKind::Binary) {}
};

struct UnaryExpr : ASTNode {
    std::string op;
    std::unique_ptr<ASTNode> argument;
    bool prefix = true;
    UnaryExpr() : ASTNode(NodeKind::Unary) {}
};

struct UpdateExpr : ASTNode {
    std::string op;
    std::unique_ptr<ASTNode> argument;
    bool prefix = true;
    UpdateExpr() : ASTNode(NodeKind::Update) {}
};

struct AssignExpr : ASTNode {
    std::string op;
    std::unique_ptr<ASTNode> target;
    std::unique_ptr<ASTNode> value;
    AssignExpr() : ASTNode(NodeKind::Assign) {}
};

struct LogicalExpr : ASTNode {
    std::string op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    LogicalExpr() : ASTNode(NodeKind::Logical) {}
};

struct CallExpr : ASTNode {
    std::unique_ptr<ASTNode> callee;
    std::vector<std::unique_ptr<ASTNode>> arguments;
    CallExpr() : ASTNode(NodeKind::Call) {}
};

struct MemberExpr : ASTNode {
    std::unique_ptr<ASTNode> object;
    std::unique_ptr<ASTNode> property;
    bool computed = false;
    MemberExpr() : ASTNode(NodeKind::Member) {}
};

struct NewExpr : ASTNode {
    std::unique_ptr<ASTNode> callee;
    std::vector<std::unique_ptr<ASTNode>> arguments;
    NewExpr() : ASTNode(NodeKind::New) {}
};

struct ArrayLiteral : ASTNode {
    std::vector<std::unique_ptr<ASTNode>> elements;
    ArrayLiteral() : ASTNode(NodeKind::Array) {}
};

struct ObjectProperty {
    std::unique_ptr<ASTNode> key;
    std::unique_ptr<ASTNode> value;
    bool computed = false;
};

struct ObjectLiteral : ASTNode {
    std::vector<ObjectProperty> properties;
    ObjectLiteral() : ASTNode(NodeKind::Object) {}
};

struct ConditionalExpr : ASTNode {
    std::unique_ptr<ASTNode> test;
    std::unique_ptr<ASTNode> consequent;
    std::unique_ptr<ASTNode> alternate;
    ConditionalExpr() : ASTNode(NodeKind::Conditional) {}
};

struct TemplateLiteral : ASTNode {
    std::vector<std::string> strings;
    std::vector<std::unique_ptr<ASTNode>> expressions;
    TemplateLiteral() : ASTNode(NodeKind::Template) {}
};

struct SpreadElement : ASTNode {
    std::unique_ptr<ASTNode> argument;
    SpreadElement() : ASTNode(NodeKind::Spread) {}
};

struct Identifier : ASTNode {
    std::string name;
    Identifier() : ASTNode(NodeKind::Identifier) {}
};

enum class LiteralType { Number, String, Boolean, Null, Undefined };

struct Literal : ASTNode {
    LiteralType literalType;
    double numberValue = 0;
    std::string stringValue;
    bool boolValue = false;
    Literal() : ASTNode(NodeKind::Literal) {}
};

// Statement list (no scope) - used for comma-separated var decls
struct StmtList : ASTNode {
    std::vector<std::unique_ptr<ASTNode>> stmts;
    StmtList() : ASTNode(NodeKind::StmtList) {}
};

} // namespace jsling
