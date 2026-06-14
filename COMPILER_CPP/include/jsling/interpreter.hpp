#pragma once
#include "ast.hpp"
#include "environment.hpp"
#include "builtins.hpp"
#include <memory>

namespace jsling {

class Interpreter {
public:
    Interpreter();
    void interpret(const Program& program);
    JSValue eval(const ASTNode& node, std::shared_ptr<Environment> env);
    JSValue callFunction(const std::shared_ptr<JSFunction>& fn,
                         const std::vector<JSValue>& args,
                         JSValue thisVal = JSValue::makeUndefined());
    std::shared_ptr<Environment> getGlobalEnv() { return globalEnv_; }

private:
    std::shared_ptr<Environment> globalEnv_;
    void setupGlobals();

    JSValue evalProgram(const Program& node, std::shared_ptr<Environment> env);
    JSValue evalBlock(const BlockStmt& node, std::shared_ptr<Environment> env);
    JSValue evalVarDecl(const VarDecl& node, std::shared_ptr<Environment> env);
    JSValue evalFunctionDecl(const FunctionDecl& node, std::shared_ptr<Environment> env);
    JSValue evalReturn(const ReturnStmt& node, std::shared_ptr<Environment> env);
    JSValue evalIf(const IfStmt& node, std::shared_ptr<Environment> env);
    JSValue evalFor(const ForStmt& node, std::shared_ptr<Environment> env);
    JSValue evalWhile(const WhileStmt& node, std::shared_ptr<Environment> env);
    JSValue evalDoWhile(const DoWhileStmt& node, std::shared_ptr<Environment> env);
    JSValue evalSwitch(const SwitchStmt& node, std::shared_ptr<Environment> env);
    JSValue evalExprStmt(const ExprStmt& node, std::shared_ptr<Environment> env);
    JSValue evalBinary(const BinaryExpr& node, std::shared_ptr<Environment> env);
    JSValue evalUnary(const UnaryExpr& node, std::shared_ptr<Environment> env);
    JSValue evalUpdate(const UpdateExpr& node, std::shared_ptr<Environment> env);
    JSValue evalAssign(const AssignExpr& node, std::shared_ptr<Environment> env);
    JSValue evalLogical(const LogicalExpr& node, std::shared_ptr<Environment> env);
    JSValue evalCall(const CallExpr& node, std::shared_ptr<Environment> env);
    JSValue evalMember(const MemberExpr& node, std::shared_ptr<Environment> env);
    JSValue evalNew(const NewExpr& node, std::shared_ptr<Environment> env);
    JSValue evalArray(const ArrayLiteral& node, std::shared_ptr<Environment> env);
    JSValue evalObject(const ObjectLiteral& node, std::shared_ptr<Environment> env);
    JSValue evalConditional(const ConditionalExpr& node, std::shared_ptr<Environment> env);
    JSValue evalTemplate(const TemplateLiteral& node, std::shared_ptr<Environment> env);
    JSValue evalIdentifier(const Identifier& node, std::shared_ptr<Environment> env);
    JSValue evalLiteral(const Literal& node, std::shared_ptr<Environment> env);

    // Type coercion helpers
    double toNumber(const JSValue& v);
    std::string toString(const JSValue& v);
    bool toBoolean(const JSValue& v);
    JSValue jsAdd(const JSValue& left, const JSValue& right);
    bool jsEquals(const JSValue& left, const JSValue& right);
    bool jsStrictEquals(const JSValue& left, const JSValue& right);

    // Method dispatch
    JSValue callMethod(const JSValue& object, const std::string& method,
                       const std::vector<JSValue>& args);
    JSValue getProperty(const JSValue& object, const std::string& prop);
};

} // namespace jsling
