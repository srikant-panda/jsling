#include "jsling/interpreter.hpp"
#include "jsling/inspect.hpp"
#include "jsling/errors.hpp"
#include <cmath>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <set>

namespace jsling {

Interpreter::Interpreter() : globalEnv_(std::make_shared<Environment>()) {
    setupGlobals();
}

void Interpreter::setupGlobals() {
    setupBuiltins(globalEnv_);
}

void Interpreter::interpret(const Program& program) {
    hoistVars(program.body, globalEnv_);
    for (const auto& stmt : program.body) {
        eval(*stmt, globalEnv_);
    }
}

JSValue Interpreter::interpretAndReturn(const Program& program) {
    hoistVars(program.body, globalEnv_);
    JSValue result = JSValue::makeUndefined();
    for (const auto& stmt : program.body) {
        result = eval(*stmt, globalEnv_);
    }
    return result;
}

static std::vector<JSValue> evalArgumentList(const std::vector<std::unique_ptr<ASTNode>>& argNodes,
                                             std::shared_ptr<Environment> env,
                                             Interpreter& interp) {
    std::vector<JSValue> args;
    for (const auto& arg : argNodes) {
        if (auto* spread = dynamic_cast<const SpreadElement*>(arg.get())) {
            JSValue value = interp.eval(*spread->argument, env);
            if (value.isArray()) {
                auto arr = value.asArray();
                args.insert(args.end(), arr->begin(), arr->end());
            } else if (value.isString()) {
                for (char c : value.asString()) {
                    args.push_back(JSValue::makeString(std::string(1, c)));
                }
            } else {
                throw TypeError("Spread argument must be iterable");
            }
        } else {
            args.push_back(interp.eval(*arg, env));
        }
    }
    return args;
}

static void bindFunctionArgs(const FunctionDecl& decl,
                             std::shared_ptr<Environment> fnEnv,
                             const std::vector<JSValue>& args,
                             Interpreter& interp) {
    for (size_t i = 0; i < decl.params.size(); ++i) {
        JSValue value = i < args.size() ? args[i] : JSValue::makeUndefined();
        if (value.isUndefined() && i < decl.defaults.size() && decl.defaults[i]) {
            value = interp.eval(*decl.defaults[i], fnEnv);
        }
        // Check if this param has a destructuring pattern
        if (i < decl.paramPatterns.size() && decl.paramPatterns[i]) {
            interp.bindPattern(decl.paramPatterns[i].get(), value, fnEnv);
        } else {
            fnEnv->define(decl.params[i], value);
        }
    }
    if (decl.hasRest) {
        auto rest = std::make_shared<std::vector<JSValue>>();
        for (size_t i = decl.params.size(); i < args.size(); ++i) {
            rest->push_back(args[i]);
        }
        fnEnv->define(decl.restParam, JSValue::makeArray(rest));
    }
}

JSValue Interpreter::eval(const ASTNode& node, std::shared_ptr<Environment> env) {
    switch (node.kind) {
        case NodeKind::Program: return evalProgram(dynamic_cast<const Program&>(node), env);
        case NodeKind::Block: return evalBlock(dynamic_cast<const BlockStmt&>(node), env);
        case NodeKind::VarDecl: return evalVarDecl(dynamic_cast<const VarDecl&>(node), env);
        case NodeKind::FunctionDecl: return evalFunctionDecl(dynamic_cast<const FunctionDecl&>(node), env);
        case NodeKind::Return: return evalReturn(dynamic_cast<const ReturnStmt&>(node), env);
        case NodeKind::If: return evalIf(dynamic_cast<const IfStmt&>(node), env);
        case NodeKind::For: return evalFor(dynamic_cast<const ForStmt&>(node), env);
        case NodeKind::While: return evalWhile(dynamic_cast<const WhileStmt&>(node), env);
        case NodeKind::DoWhile: return evalDoWhile(dynamic_cast<const DoWhileStmt&>(node), env);
        case NodeKind::Switch: return evalSwitch(dynamic_cast<const SwitchStmt&>(node), env);
        case NodeKind::Break: throw BreakSignal{};
        case NodeKind::Continue: throw ContinueSignal{};
        case NodeKind::ExprStmt: return evalExprStmt(dynamic_cast<const ExprStmt&>(node), env);
        case NodeKind::Binary: return evalBinary(dynamic_cast<const BinaryExpr&>(node), env);
        case NodeKind::Unary: return evalUnary(dynamic_cast<const UnaryExpr&>(node), env);
        case NodeKind::Update: return evalUpdate(dynamic_cast<const UpdateExpr&>(node), env);
        case NodeKind::Assign: return evalAssign(dynamic_cast<const AssignExpr&>(node), env);
        case NodeKind::Logical: return evalLogical(dynamic_cast<const LogicalExpr&>(node), env);
        case NodeKind::Call: return evalCall(dynamic_cast<const CallExpr&>(node), env);
        case NodeKind::Member: return evalMember(dynamic_cast<const MemberExpr&>(node), env);
        case NodeKind::New: return evalNew(dynamic_cast<const NewExpr&>(node), env);
        case NodeKind::Array: return evalArray(dynamic_cast<const ArrayLiteral&>(node), env);
        case NodeKind::Object: return evalObject(dynamic_cast<const ObjectLiteral&>(node), env);
        case NodeKind::Conditional: return evalConditional(dynamic_cast<const ConditionalExpr&>(node), env);
        case NodeKind::Template: return evalTemplate(dynamic_cast<const TemplateLiteral&>(node), env);
        case NodeKind::Identifier: return evalIdentifier(dynamic_cast<const Identifier&>(node), env);
        case NodeKind::Literal: return evalLiteral(dynamic_cast<const Literal&>(node), env);
        case NodeKind::StmtList: return evalStmtList(dynamic_cast<const StmtList&>(node), env);
        default: return JSValue::makeUndefined();
    }
}

// ==================== STATEMENT EVALUATION ====================

JSValue Interpreter::evalProgram(const Program& node, std::shared_ptr<Environment> env) {
    hoistVars(node.body, env);
    JSValue result = JSValue::makeUndefined();
    for (const auto& stmt : node.body) {
        result = eval(*stmt, env);
    }
    return result;
}

JSValue Interpreter::evalBlock(const BlockStmt& node, std::shared_ptr<Environment> env) {
    auto blockEnv = std::make_shared<Environment>(env);
    JSValue result = JSValue::makeUndefined();
    for (const auto& stmt : node.body) {
        result = eval(*stmt, blockEnv);
    }
    return result;
}

JSValue Interpreter::evalVarDecl(const VarDecl& node, std::shared_ptr<Environment> env) {
    JSValue value = JSValue::makeUndefined();
    if (node.initializer) {
        value = eval(*node.initializer, env);
    }
    auto targetEnv = env;
    if (node.kind == "var") {
        targetEnv = env->getFunctionOrGlobalEnv();
    }
    if (node.pattern) {
        bindPattern(node.pattern.get(), value, targetEnv);
    } else {
        targetEnv->define(node.name, value);
    }
    return value;
}

JSValue Interpreter::evalFunctionDecl(const FunctionDecl& node, std::shared_ptr<Environment> env) {
    auto fn = std::make_shared<JSFunction>();
    fn->name = node.name;
    fn->params = node.params;
    fn->closure = env;
    fn->body = nullptr;

    auto declPtr = &node;
    auto closureEnv = env;

    // Named function expression: bind name inside function scope only
    if (node.isExpression && !node.name.empty()) {
        auto nameEnv = std::make_shared<Environment>(closureEnv);
        auto selfRef = std::make_shared<JSValue*>(nullptr);
        auto nativeFn = JSValue::makeNativeFunction(
            std::make_shared<JSNativeFunction>(JSNativeFunction{
                node.name,
                [declPtr, closureEnv, nameEnv, selfRef, this](const std::vector<JSValue>& args) -> JSValue {
                    auto fnEnv = std::make_shared<Environment>(nameEnv);
                    fnEnv->isFunctionOrGlobal = true;
                    bindFunctionArgs(*declPtr, fnEnv, args, *this);
                    hoistVars(declPtr->body->body, fnEnv);
                    try {
                        for (const auto& stmt : declPtr->body->body) {
                            eval(*stmt, fnEnv);
                        }
                    } catch (const ReturnSignal& ret) {
                        return ret.value;
                    }
                    return JSValue::makeUndefined();
                }
            })
        );
        *selfRef = new JSValue(nativeFn);
        nameEnv->define(node.name, **selfRef);
        return **selfRef;
    }

    // Named function declaration: define in outer env
    if (!node.name.empty()) {
        env->define(node.name, JSValue::makeNativeFunction(
            std::make_shared<JSNativeFunction>(JSNativeFunction{
                node.name,
                [declPtr, closureEnv, this](const std::vector<JSValue>& args) -> JSValue {
                    auto fnEnv = std::make_shared<Environment>(closureEnv);
                    fnEnv->isFunctionOrGlobal = true;
                    bindFunctionArgs(*declPtr, fnEnv, args, *this);
                    hoistVars(declPtr->body->body, fnEnv);
                    try {
                        for (const auto& stmt : declPtr->body->body) {
                            eval(*stmt, fnEnv);
                        }
                    } catch (const ReturnSignal& ret) {
                        return ret.value;
                    }
                    return JSValue::makeUndefined();
                }
            })
        ));
        return JSValue::makeUndefined();
    }

    // Anonymous function expression - return it
    return JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{
            "",
            [declPtr, closureEnv, this](const std::vector<JSValue>& args) -> JSValue {
                auto fnEnv = std::make_shared<Environment>(closureEnv);
                fnEnv->isFunctionOrGlobal = true;
                bindFunctionArgs(*declPtr, fnEnv, args, *this);
                hoistVars(declPtr->body->body, fnEnv);
                try {
                    for (const auto& stmt : declPtr->body->body) {
                        eval(*stmt, fnEnv);
                    }
                } catch (const ReturnSignal& ret) {
                    return ret.value;
                }
                return JSValue::makeUndefined();
            }
        })
    );
}

JSValue Interpreter::evalReturn(const ReturnStmt& node, std::shared_ptr<Environment> env) {
    JSValue value = JSValue::makeUndefined();
    if (node.argument) {
        value = eval(*node.argument, env);
    }
    throw ReturnSignal{value};
}

JSValue Interpreter::evalIf(const IfStmt& node, std::shared_ptr<Environment> env) {
    JSValue test = eval(*node.test, env);
    if (test.isTruthy()) {
        return eval(*node.consequent, env);
    } else if (node.alternate) {
        return eval(*node.alternate, env);
    }
    return JSValue::makeUndefined();
}

JSValue Interpreter::evalFor(const ForStmt& node, std::shared_ptr<Environment> env) {
    auto forEnv = std::make_shared<Environment>(env);
    if (node.init) eval(*node.init, forEnv);
    while (true) {
        if (node.test) {
            JSValue test = eval(*node.test, forEnv);
            if (!test.isTruthy()) break;
        }
        try {
            eval(*node.body, forEnv);
        } catch (const BreakSignal&) {
            break;
        } catch (const ContinueSignal&) {
            // continue to update
        }
        if (node.update) eval(*node.update, forEnv);
    }
    return JSValue::makeUndefined();
}

JSValue Interpreter::evalWhile(const WhileStmt& node, std::shared_ptr<Environment> env) {
    while (true) {
        JSValue test = eval(*node.test, env);
        if (!test.isTruthy()) break;
        try {
            eval(*node.body, env);
        } catch (const BreakSignal&) {
            break;
        } catch (const ContinueSignal&) {
            continue;
        }
    }
    return JSValue::makeUndefined();
}

JSValue Interpreter::evalDoWhile(const DoWhileStmt& node, std::shared_ptr<Environment> env) {
    do {
        try {
            eval(*node.body, env);
        } catch (const BreakSignal&) {
            break;
        } catch (const ContinueSignal&) {
            continue;
        }
        JSValue test = eval(*node.test, env);
        if (!test.isTruthy()) break;
    } while (true);
    return JSValue::makeUndefined();
}

JSValue Interpreter::evalSwitch(const SwitchStmt& node, std::shared_ptr<Environment> env) {
    JSValue discriminant = eval(*node.discriminant, env);
    bool matched = false;
    try {
        for (const auto& sc : node.cases) {
            if (!matched && sc.test) {
                JSValue testVal = eval(*sc.test, env);
                if (jsStrictEquals(discriminant, testVal)) matched = true;
            } else if (!sc.test) {
                matched = true; // default case
            }
            if (matched) {
                for (const auto& stmt : sc.consequent) {
                    eval(*stmt, env);
                }
            }
        }
    } catch (const BreakSignal&) {
        // break exits switch
    }
    return JSValue::makeUndefined();
}

JSValue Interpreter::evalExprStmt(const ExprStmt& node, std::shared_ptr<Environment> env) {
    if (node.expression) return eval(*node.expression, env);
    return JSValue::makeUndefined();
}

// ==================== EXPRESSION EVALUATION ====================

JSValue Interpreter::evalBinary(const BinaryExpr& node, std::shared_ptr<Environment> env) {
    JSValue left = eval(*node.left, env);
    JSValue right = eval(*node.right, env);

    if (node.op == "+") return jsAdd(left, right);
    if (node.op == "-") return JSValue::makeNumber(toNumber(left) - toNumber(right));
    if (node.op == "*") return JSValue::makeNumber(toNumber(left) * toNumber(right));
    if (node.op == "/") return JSValue::makeNumber(toNumber(left) / toNumber(right));
    if (node.op == "%") return JSValue::makeNumber(std::fmod(toNumber(left), toNumber(right)));
    if (node.op == "**") return JSValue::makeNumber(std::pow(toNumber(left), toNumber(right)));
    
    if (node.op == "==") return JSValue::makeBool(jsEquals(left, right));
    if (node.op == "!=") return JSValue::makeBool(!jsEquals(left, right));
    if (node.op == "===") return JSValue::makeBool(jsStrictEquals(left, right));
    if (node.op == "!==") return JSValue::makeBool(!jsStrictEquals(left, right));
    
    if (node.op == "<") return JSValue::makeBool(toNumber(left) < toNumber(right));
    if (node.op == ">") return JSValue::makeBool(toNumber(left) > toNumber(right));
    if (node.op == "<=") return JSValue::makeBool(toNumber(left) <= toNumber(right));
    if (node.op == ">=") return JSValue::makeBool(toNumber(left) >= toNumber(right));

    // Bitwise operators
    if (node.op == "&") return JSValue::makeNumber(static_cast<int>(toNumber(left)) & static_cast<int>(toNumber(right)));
    if (node.op == "|") return JSValue::makeNumber(static_cast<int>(toNumber(left)) | static_cast<int>(toNumber(right)));
    if (node.op == "^") return JSValue::makeNumber(static_cast<int>(toNumber(left)) ^ static_cast<int>(toNumber(right)));
    if (node.op == "<<") return JSValue::makeNumber(static_cast<int>(toNumber(left)) << static_cast<int>(toNumber(right)));
    if (node.op == ">>") return JSValue::makeNumber(static_cast<int>(toNumber(left)) >> static_cast<int>(toNumber(right)));
    if (node.op == ">>>") return JSValue::makeNumber(static_cast<unsigned int>(toNumber(left)) >> static_cast<int>(toNumber(right)));

    // in operator: check if property exists in object
    if (node.op == "in") {
        std::string prop = toString(left);
        if (right.isObject()) {
            auto obj = right.asObject();
            return JSValue::makeBool(obj->properties.find(prop) != obj->properties.end());
        }
        if (right.isArray()) {
            // For arrays, check if the key is a valid index
            try {
                size_t idx = std::stoul(prop);
                return JSValue::makeBool(idx < right.asArray()->size());
            } catch (...) {
                if (prop == "length") return JSValue::makeBool(true);
                return JSValue::makeBool(false);
            }
        }
        throw TypeError("Cannot use 'in' operator to search for '" + prop + "' in " + toString(right));
    }

    // instanceof operator (basic support)
    if (node.op == "instanceof") {
        // For now, always return false since we don't have prototype chains
        return JSValue::makeBool(false);
    }

    return JSValue::makeUndefined();
}

JSValue Interpreter::evalUnary(const UnaryExpr& node, std::shared_ptr<Environment> env) {
    if (node.op == "typeof") {
        // typeof needs special handling for undefined variables
        if (auto* id = dynamic_cast<const Identifier*>(node.argument.get())) {
            if (!env->has(id->name)) return JSValue::makeString("undefined");
        }
        JSValue val = eval(*node.argument, env);
        if (val.isNumber()) return JSValue::makeString("number");
        if (val.isString()) return JSValue::makeString("string");
        if (val.isBool()) return JSValue::makeString("boolean");
        if (val.isNull()) return JSValue::makeString("object");
        if (val.isUndefined()) return JSValue::makeString("undefined");
        if (val.isFunction() || val.isNativeFunction()) return JSValue::makeString("function");
        if (val.isObject()) return JSValue::makeString("object");
        if (val.isArray()) return JSValue::makeString("object");
        return JSValue::makeString("undefined");
    }
    JSValue val = eval(*node.argument, env);
    if (node.op == "-") return JSValue::makeNumber(-toNumber(val));
    if (node.op == "+") return JSValue::makeNumber(toNumber(val));
    if (node.op == "!") return JSValue::makeBool(!toBoolean(val));
    if (node.op == "void") return JSValue::makeUndefined();
    return JSValue::makeUndefined();
}

JSValue Interpreter::evalUpdate(const UpdateExpr& node, std::shared_ptr<Environment> env) {
    auto* id = dynamic_cast<const Identifier*>(node.argument.get());
    if (!id) throw TypeError("Invalid left-hand side in assignment");
    JSValue oldVal = env->get(id->name);
    double oldNum = toNumber(oldVal);
    double newNum = node.op == "++" ? oldNum + 1 : oldNum - 1;
    env->set(id->name, JSValue::makeNumber(newNum));
    return node.prefix ? JSValue::makeNumber(newNum) : JSValue::makeNumber(oldNum);
}

JSValue Interpreter::evalAssign(const AssignExpr& node, std::shared_ptr<Environment> env) {
    JSValue value = eval(*node.value, env);
    auto* id = dynamic_cast<const Identifier*>(node.target.get());
    auto* member = dynamic_cast<const MemberExpr*>(node.target.get());
    
    if (id) {
        if (node.op == "=") {
            env->set(id->name, value);
        } else {
            JSValue oldVal = env->get(id->name);
            if (node.op == "+=") env->set(id->name, jsAdd(oldVal, value));
            else if (node.op == "-=") env->set(id->name, JSValue::makeNumber(toNumber(oldVal) - toNumber(value)));
            else if (node.op == "*=") env->set(id->name, JSValue::makeNumber(toNumber(oldVal) * toNumber(value)));
            else if (node.op == "/=") env->set(id->name, JSValue::makeNumber(toNumber(oldVal) / toNumber(value)));
            else if (node.op == "%=") env->set(id->name, JSValue::makeNumber(std::fmod(toNumber(oldVal), toNumber(value))));
        }
        return value;
    } else if (member) {
        JSValue obj = eval(*member->object, env);
        std::string prop = member->computed ? toString(eval(*member->property, env)) 
                                            : dynamic_cast<const Identifier*>(member->property.get())->name;
        if (obj.isObject()) {
            auto objPtr = obj.asObject();
            if (node.op == "=") {
                if (objPtr->properties.find(prop) == objPtr->properties.end()) {
                    objPtr->keys.push_back(prop);
                }
                objPtr->properties[prop] = value;
            }
        } else if (obj.isArray()) {
            auto arr = obj.asArray();
            size_t idx = static_cast<size_t>(toNumber(eval(*member->property, env)));
            while (arr->size() <= idx) arr->push_back(JSValue::makeUndefined());
            (*arr)[idx] = value;
        }
        return value;
    }
    // Destructuring assignment: [a, b] = ... or {a, b} = ...
    if (node.op == "=" && (dynamic_cast<const ArrayLiteral*>(node.target.get()) ||
                            dynamic_cast<const ObjectLiteral*>(node.target.get()))) {
        assignFromPattern(node.target.get(), value, env);
        return value;
    }
    throw TypeError("Invalid left-hand side in assignment");
}

JSValue Interpreter::evalLogical(const LogicalExpr& node, std::shared_ptr<Environment> env) {
    JSValue left = eval(*node.left, env);
    if (node.op == "&&") return left.isTruthy() ? eval(*node.right, env) : left;
    if (node.op == "||") return left.isTruthy() ? left : eval(*node.right, env);
    return JSValue::makeUndefined();
}

JSValue Interpreter::evalCall(const CallExpr& node, std::shared_ptr<Environment> env) {
    // Handle member calls like console.log or date.getFullYear()
    if (auto* member = dynamic_cast<const MemberExpr*>(node.callee.get())) {
        JSValue obj = eval(*member->object, env);
        std::string method = member->computed ? toString(eval(*member->property, env))
                                              : dynamic_cast<const Identifier*>(member->property.get())->name;
        std::vector<JSValue> args = evalArgumentList(node.arguments, env, *this);

        // Handle Date.now() - static method on Date constructor
        if (method == "now" && obj.isNativeFunction() && obj.asNativeFunction()->name == "Date") {
            auto now = std::chrono::system_clock::now();
            return JSValue::makeNumber(static_cast<double>(
                std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()));
        }
        
        // If the property is a function, call it with `this` (obj) as first arg
        JSValue prop = getProperty(obj, method);
        if (prop.isNativeFunction()) {
            // For Date/object methods, prepend `this` as first argument
            if (obj.isObject() && obj.asObject()->properties.count("__timestamp__") &&
                (method == "getFullYear" || method == "getMonth" || method == "getDate" ||
                 method == "getDay" || method == "getHours" || method == "getMinutes" ||
                 method == "getSeconds" || method == "getMilliseconds" || method == "getTime" ||
                 method == "toISOString" || method == "toString" || method == "toDateString" ||
                 method == "toTimeString" || method == "toJSON" || method == "valueOf" ||
                 method == "toLocaleDateString")) {
                std::vector<JSValue> thisArgs = {obj};
                thisArgs.insert(thisArgs.end(), args.begin(), args.end());
                return prop.asNativeFunction()->fn(thisArgs);
            }
            return prop.asNativeFunction()->fn(args);
        } else if (prop.isFunction()) {
            return callFunction(prop.asFunction(), args);
        }
        
        // Method dispatch for built-in types
        return callMethod(obj, method, args);
    }
    
    JSValue callee = eval(*node.callee, env);
    std::vector<JSValue> args = evalArgumentList(node.arguments, env, *this);
    
    if (callee.isNativeFunction()) {
        return callee.asNativeFunction()->fn(args);
    } else if (callee.isFunction()) {
        return callFunction(callee.asFunction(), args);
    }
    throw TypeError(callee.isUndefined() ? "Cannot call undefined" : "Not a function");
}

JSValue Interpreter::evalMember(const MemberExpr& node, std::shared_ptr<Environment> env) {
    JSValue obj = eval(*node.object, env);
    std::string prop = node.computed ? toString(eval(*node.property, env))
                                     : dynamic_cast<const Identifier*>(node.property.get())->name;
    return getProperty(obj, prop);
}

JSValue Interpreter::evalNew(const NewExpr& node, std::shared_ptr<Environment> env) {
    JSValue callee = eval(*node.callee, env);
    std::vector<JSValue> args = evalArgumentList(node.arguments, env, *this);
    // For now, just call the constructor
    if (callee.isNativeFunction()) {
        return callee.asNativeFunction()->fn(args);
    }
    throw TypeError("Not a constructor");
}

JSValue Interpreter::evalArray(const ArrayLiteral& node, std::shared_ptr<Environment> env) {
    auto arr = std::make_shared<std::vector<JSValue>>();
    for (const auto& elem : node.elements) {
        if (auto* spread = dynamic_cast<const SpreadElement*>(elem.get())) {
            JSValue value = eval(*spread->argument, env);
            if (value.isArray()) {
                auto source = value.asArray();
                arr->insert(arr->end(), source->begin(), source->end());
            } else if (value.isString()) {
                for (char c : value.asString()) {
                    arr->push_back(JSValue::makeString(std::string(1, c)));
                }
            } else {
                throw TypeError("Spread element must be iterable");
            }
        } else {
            arr->push_back(eval(*elem, env));
        }
    }
    return JSValue::makeArray(arr);
}

JSValue Interpreter::evalObject(const ObjectLiteral& node, std::shared_ptr<Environment> env) {
    auto obj = std::make_shared<JSObject>();
    for (const auto& prop : node.properties) {
        if (!prop.key && prop.value && prop.value->kind == NodeKind::Spread) {
            auto* spread = dynamic_cast<const SpreadElement*>(prop.value.get());
            JSValue spreadValue = eval(*spread->argument, env);
            if (!spreadValue.isObject()) throw TypeError("Object spread source must be an object");
            auto source = spreadValue.asObject();
            for (const auto& key : source->keys) {
                if (obj->properties.find(key) == obj->properties.end()) {
                    obj->keys.push_back(key);
                }
                obj->properties[key] = source->properties.at(key);
            }
            continue;
        }
        std::string key;
        if (prop.computed) {
            JSValue keyValue = eval(*prop.key, env);
            key = toString(keyValue);
        } else if (auto* id = dynamic_cast<const Identifier*>(prop.key.get())) {
            key = id->name;
        } else if (auto* lit = dynamic_cast<const Literal*>(prop.key.get())) {
            key = lit->stringValue;
        }
        obj->keys.push_back(key);
        obj->properties[key] = eval(*prop.value, env);
    }
    return JSValue::makeObject(obj);
}

JSValue Interpreter::evalConditional(const ConditionalExpr& node, std::shared_ptr<Environment> env) {
    JSValue test = eval(*node.test, env);
    return test.isTruthy() ? eval(*node.consequent, env) : eval(*node.alternate, env);
}

JSValue Interpreter::evalTemplate(const TemplateLiteral& node, std::shared_ptr<Environment> env) {
    std::string result;
    for (size_t i = 0; i < node.strings.size(); ++i) {
        result += node.strings[i];
        if (i < node.expressions.size()) {
            result += toString(eval(*node.expressions[i], env));
        }
    }
    return JSValue::makeString(result);
}

JSValue Interpreter::evalIdentifier(const Identifier& node, std::shared_ptr<Environment> env) {
    return env->get(node.name);
}

JSValue Interpreter::evalLiteral(const Literal& node, std::shared_ptr<Environment>) {
    switch (node.literalType) {
        case LiteralType::Number: return JSValue::makeNumber(node.numberValue);
        case LiteralType::String: return JSValue::makeString(node.stringValue);
        case LiteralType::Boolean: return JSValue::makeBool(node.boolValue);
        case LiteralType::Null: return JSValue::makeNull();
        case LiteralType::Undefined: return JSValue::makeUndefined();
    }
    return JSValue::makeUndefined();
}

JSValue Interpreter::evalStmtList(const StmtList& node, std::shared_ptr<Environment> env) {
    JSValue result = JSValue::makeUndefined();
    for (const auto& stmt : node.stmts) {
        result = eval(*stmt, env);
    }
    return result;
}

// ==================== TYPE COERCION ====================

double Interpreter::toNumber(const JSValue& v) {
    if (v.isNumber()) return v.asNumber();
    if (v.isBool()) return v.asBool() ? 1 : 0;
    if (v.isNull()) return 0;
    if (v.isUndefined()) return std::nan("");
    if (v.isString()) {
        try { return std::stod(v.asString()); }
        catch (...) { return std::nan(""); }
    }
    return std::nan("");
}

std::string Interpreter::toString(const JSValue& v) {
    if (v.isString()) return v.asString();
    if (v.isNumber()) return formatNumber(v.asNumber());
    if (v.isBool()) return v.asBool() ? "true" : "false";
    if (v.isNull()) return "null";
    if (v.isUndefined()) return "undefined";
    if (v.isArray()) return inspect(v);
    if (v.isObject()) return inspect(v);
    if (v.isFunction()) return "[Function]";
    return "undefined";
}

bool Interpreter::toBoolean(const JSValue& v) {
    return v.isTruthy();
}

JSValue Interpreter::jsAdd(const JSValue& left, const JSValue& right) {
    if (left.isString() || right.isString()) {
        return JSValue::makeString(toString(left) + toString(right));
    }
    return JSValue::makeNumber(toNumber(left) + toNumber(right));
}

bool Interpreter::jsEquals(const JSValue& left, const JSValue& right) {
    if (left.isNull() && right.isUndefined()) return true;
    if (left.isUndefined() && right.isNull()) return true;
    if (left.isNull() || left.isUndefined()) return right.isNull() || right.isUndefined();
    if (right.isNull() || right.isUndefined()) return false;
    if (left.isNumber() && right.isString()) return toNumber(left) == toNumber(right);
    if (left.isString() && right.isNumber()) return toNumber(left) == toNumber(right);
    if (left.isBool()) return jsEquals(JSValue::makeNumber(toNumber(left)), right);
    if (right.isBool()) return jsEquals(left, JSValue::makeNumber(toNumber(right)));
    return jsStrictEquals(left, right);
}

bool Interpreter::jsStrictEquals(const JSValue& left, const JSValue& right) {
    if (left.isNull() && right.isNull()) return true;
    if (left.isUndefined() && right.isUndefined()) return true;
    if (left.isNumber() && right.isNumber()) {
        double a = left.asNumber(), b = right.asNumber();
        if (std::isnan(a) || std::isnan(b)) return false;
        return a == b;
    }
    if (left.isString() && right.isString()) return left.asString() == right.asString();
    if (left.isBool() && right.isBool()) return left.asBool() == right.asBool();
    return false;
}

// ==================== METHOD DISPATCH ====================

JSValue Interpreter::getProperty(const JSValue& object, const std::string& prop) {
    if (object.isObject()) {
        auto obj = object.asObject();
        auto it = obj->properties.find(prop);
        if (it != obj->properties.end()) return it->second;
        return JSValue::makeUndefined();
    }
    if (object.isArray()) {
        if (prop == "length") return JSValue::makeNumber(static_cast<double>(object.asArray()->size()));
        // Numeric index access: arr[0], arr[1], etc.
        try {
            std::size_t idx = std::stoul(prop);
            auto arr = object.asArray();
            if (idx < arr->size()) return (*arr)[idx];
            return JSValue::makeUndefined();
        } catch (...) {}
    }
    if (object.isString()) {
        if (prop == "length") return JSValue::makeNumber(static_cast<double>(object.asString().size()));
        try {
            std::size_t idx = std::stoul(prop);
            if (idx < object.asString().size()) return JSValue::makeString(std::string(1, object.asString()[idx]));
            return JSValue::makeUndefined();
        } catch (...) {}
    }
    // Static methods on Number
    if (object.isNativeFunction() && object.asNativeFunction()->name == "Number") {
        if (prop == "isNaN") {
            return JSValue::makeNativeFunction(std::make_shared<JSNativeFunction>(JSNativeFunction{
                "isNaN",
                [](const std::vector<JSValue>& args) -> JSValue {
                    if (args.empty()) return JSValue::makeBool(false);
                    return JSValue::makeBool(args[0].isNumber() && std::isnan(args[0].asNumber()));
                }
            }));
        }
        if (prop == "isInteger") {
            return JSValue::makeNativeFunction(std::make_shared<JSNativeFunction>(JSNativeFunction{
                "isInteger",
                [](const std::vector<JSValue>& args) -> JSValue {
                    if (args.empty() || !args[0].isNumber()) return JSValue::makeBool(false);
                    double n = args[0].asNumber();
                    return JSValue::makeBool(std::isfinite(n) && std::floor(n) == n);
                }
            }));
        }
        if (prop == "isFinite") {
            return JSValue::makeNativeFunction(std::make_shared<JSNativeFunction>(JSNativeFunction{
                "isFinite",
                [](const std::vector<JSValue>& args) -> JSValue {
                    if (args.empty() || !args[0].isNumber()) return JSValue::makeBool(false);
                    return JSValue::makeBool(std::isfinite(args[0].asNumber()));
                }
            }));
        }
        if (prop == "MAX_SAFE_INTEGER") return JSValue::makeNumber(9007199254740991.0);
        if (prop == "MIN_SAFE_INTEGER") return JSValue::makeNumber(-9007199254740991.0);
        if (prop == "EPSILON") return JSValue::makeNumber(2.220446049250313e-16);
    }
    return JSValue::makeUndefined();
}

JSValue Interpreter::callMethod(const JSValue& object, const std::string& method,
                                const std::vector<JSValue>& args) {
    // String methods
    if (object.isString()) {
        const std::string& s = object.asString();
        if (method == "toUpperCase") return JSValue::makeString([&] { std::string r = s; std::transform(r.begin(), r.end(), r.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); }); return r; }());
        if (method == "toLowerCase") return JSValue::makeString([&] { std::string r = s; std::transform(r.begin(), r.end(), r.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); }); return r; }());
        if (method == "trim") return JSValue::makeString([&] { size_t start = s.find_first_not_of(" \t\n\r"); size_t end = s.find_last_not_of(" \t\n\r"); return start == std::string::npos ? "" : s.substr(start, end - start + 1); }());
        if (method == "includes") return JSValue::makeBool(s.find(args.empty() ? "" : args[0].asString()) != std::string::npos);
        if (method == "indexOf") { auto pos = s.find(args.empty() ? "" : args[0].asString()); return JSValue::makeNumber(pos == std::string::npos ? -1 : static_cast<double>(pos)); }
        if (method == "charAt") { size_t i = args.empty() ? 0 : static_cast<size_t>(args[0].asNumber()); return JSValue::makeString(i < s.size() ? std::string(1, s[i]) : ""); }
        if (method == "slice") {
            size_t len = s.size();
            int start = args.empty() ? 0 : static_cast<int>(args[0].asNumber());
            int end = args.size() < 2 ? static_cast<int>(len) : static_cast<int>(args[1].asNumber());
            if (start < 0) start = std::max(0, static_cast<int>(len) + start);
            if (end < 0) end = std::max(0, static_cast<int>(len) + end);
            return JSValue::makeString(s.substr(start, end - start));
        }
        if (method == "split") {
            std::string delim = args.empty() ? "" : args[0].asString();
            auto arr = std::make_shared<std::vector<JSValue>>();
            if (delim.empty()) {
                for (char c : s) arr->push_back(JSValue::makeString(std::string(1, c)));
            } else {
                size_t pos = 0, prev = 0;
                while ((pos = s.find(delim, prev)) != std::string::npos) {
                    arr->push_back(JSValue::makeString(s.substr(prev, pos - prev)));
                    prev = pos + delim.size();
                }
                arr->push_back(JSValue::makeString(s.substr(prev)));
            }
            return JSValue::makeArray(arr);
        }
        if (method == "replace") {
            std::string search = args[0].asString();
            std::string replace = args[1].asString();
            std::string result = s;
            size_t pos = result.find(search);
            if (pos != std::string::npos) result.replace(pos, search.size(), replace);
            return JSValue::makeString(result);
        }
        if (method == "substring") {
            size_t start = args.empty() ? 0 : static_cast<size_t>(args[0].asNumber());
            size_t end = args.size() < 2 ? s.size() : static_cast<size_t>(args[1].asNumber());
            return JSValue::makeString(s.substr(start, end - start));
        }
        if (method == "startsWith") return JSValue::makeBool(s.find(args[0].asString()) == 0);
        if (method == "endsWith") { std::string sub = args[0].asString(); return JSValue::makeBool(s.size() >= sub.size() && s.compare(s.size() - sub.size(), sub.size(), sub) == 0); }
        if (method == "repeat") { int n = static_cast<int>(args[0].asNumber()); std::string r; for (int i = 0; i < n; ++i) r += s; return JSValue::makeString(r); }
        if (method == "concat") { std::string r = s; for (const auto& a : args) r += toString(a); return JSValue::makeString(r); }
        if (method == "replaceAll") {
            if (args.size() < 2) throw TypeError("replaceAll requires 2 arguments");
            std::string search = args[0].asString();
            std::string replace = args[1].asString();
            std::string result = s;
            size_t pos = 0;
            while ((pos = result.find(search, pos)) != std::string::npos) {
                result.replace(pos, search.size(), replace);
                pos += replace.size();
            }
            return JSValue::makeString(result);
        }
        if (method == "padStart") {
            size_t targetLen = static_cast<size_t>(args[0].asNumber());
            std::string padStr = args.size() > 1 ? args[1].asString() : " ";
            if (s.size() >= targetLen || padStr.empty()) return JSValue::makeString(s);
            std::string result = s;
            while (result.size() < targetLen) result = padStr + result;
            return JSValue::makeString(result.substr(result.size() - targetLen > targetLen ? result.size() - targetLen : 0));
        }
        if (method == "padEnd") {
            size_t targetLen = static_cast<size_t>(args[0].asNumber());
            std::string padStr = args.size() > 1 ? args[1].asString() : " ";
            if (s.size() >= targetLen || padStr.empty()) return JSValue::makeString(s);
            std::string result = s;
            while (result.size() < targetLen) result += padStr;
            return JSValue::makeString(result.substr(0, targetLen));
        }
    }

    // Array methods
    if (object.isArray()) {
        auto arr = object.asArray();
        if (method == "push") { for (const auto& a : args) arr->push_back(a); return JSValue::makeNumber(static_cast<double>(arr->size())); }
        if (method == "pop") { if (arr->empty()) return JSValue::makeUndefined(); JSValue v = arr->back(); arr->pop_back(); return v; }
        if (method == "shift") { if (arr->empty()) return JSValue::makeUndefined(); JSValue v = arr->front(); arr->erase(arr->begin()); return v; }
        if (method == "unshift") { arr->insert(arr->begin(), args.begin(), args.end()); return JSValue::makeNumber(static_cast<double>(arr->size())); }
        if (method == "join") {
            std::string sep = args.empty() ? "," : args[0].asString();
            std::string result;
            for (size_t i = 0; i < arr->size(); ++i) {
                if (i > 0) result += sep;
                result += toString((*arr)[i]);
            }
            return JSValue::makeString(result);
        }
        if (method == "reverse") { std::reverse(arr->begin(), arr->end()); return object; }
        if (method == "includes") {
            for (const auto& v : *arr) if (jsStrictEquals(v, args[0])) return JSValue::makeBool(true);
            return JSValue::makeBool(false);
        }
        if (method == "indexOf") {
            for (size_t i = 0; i < arr->size(); ++i) if (jsStrictEquals((*arr)[i], args[0])) return JSValue::makeNumber(static_cast<double>(i));
            return JSValue::makeNumber(-1);
        }
        if (method == "slice") {
            size_t len = arr->size();
            int start = args.empty() ? 0 : static_cast<int>(args[0].asNumber());
            int end = args.size() < 2 ? static_cast<int>(len) : static_cast<int>(args[1].asNumber());
            if (start < 0) start = std::max(0, static_cast<int>(len) + start);
            if (end < 0) end = std::max(0, static_cast<int>(len) + end);
            auto newArr = std::make_shared<std::vector<JSValue>>();
            for (int i = start; i < end && i < static_cast<int>(len); ++i) newArr->push_back((*arr)[i]);
            return JSValue::makeArray(newArr);
        }
        if (method == "concat") {
            auto newArr = std::make_shared<std::vector<JSValue>>(*arr);
            for (const auto& a : args) {
                if (a.isArray()) { for (const auto& v : *a.asArray()) newArr->push_back(v); }
                else newArr->push_back(a);
            }
            return JSValue::makeArray(newArr);
        }
        if (method == "map") {
            auto fn = args[0].asNativeFunction();
            auto newArr = std::make_shared<std::vector<JSValue>>();
            for (size_t i = 0; i < arr->size(); ++i) {
                newArr->push_back(fn->fn({(*arr)[i], JSValue::makeNumber(static_cast<double>(i)), object}));
            }
            return JSValue::makeArray(newArr);
        }
        if (method == "filter") {
            auto fn = args[0].asNativeFunction();
            auto newArr = std::make_shared<std::vector<JSValue>>();
            for (size_t i = 0; i < arr->size(); ++i) {
                if (fn->fn({(*arr)[i], JSValue::makeNumber(static_cast<double>(i)), object}).isTruthy()) {
                    newArr->push_back((*arr)[i]);
                }
            }
            return JSValue::makeArray(newArr);
        }
        if (method == "reduce") {
            auto fn = args[0].asNativeFunction();
            JSValue acc = args.size() > 1 ? args[1] : (*arr)[0];
            size_t start = args.size() > 1 ? 0 : 1;
            for (size_t i = start; i < arr->size(); ++i) {
                acc = fn->fn({acc, (*arr)[i], JSValue::makeNumber(static_cast<double>(i)), object});
            }
            return acc;
        }
        if (method == "forEach") {
            auto fn = args[0].asNativeFunction();
            for (size_t i = 0; i < arr->size(); ++i) {
                fn->fn({(*arr)[i], JSValue::makeNumber(static_cast<double>(i)), object});
            }
            return JSValue::makeUndefined();
        }
        if (method == "find") {
            auto fn = args[0].asNativeFunction();
            for (size_t i = 0; i < arr->size(); ++i) {
                if (fn->fn({(*arr)[i], JSValue::makeNumber(static_cast<double>(i)), object}).isTruthy()) {
                    return (*arr)[i];
                }
            }
            return JSValue::makeUndefined();
        }
        if (method == "some") {
            auto fn = args[0].asNativeFunction();
            for (size_t i = 0; i < arr->size(); ++i) {
                if (fn->fn({(*arr)[i], JSValue::makeNumber(static_cast<double>(i)), object}).isTruthy()) {
                    return JSValue::makeBool(true);
                }
            }
            return JSValue::makeBool(false);
        }
        if (method == "every") {
            auto fn = args[0].asNativeFunction();
            for (size_t i = 0; i < arr->size(); ++i) {
                if (!fn->fn({(*arr)[i], JSValue::makeNumber(static_cast<double>(i)), object}).isTruthy()) {
                    return JSValue::makeBool(false);
                }
            }
            return JSValue::makeBool(true);
        }
        if (method == "sort") {
            std::sort(arr->begin(), arr->end(), [this](const JSValue& a, const JSValue& b) {
                return toString(a) < toString(b);
            });
            return object;
        }
        if (method == "splice") {
            size_t start = static_cast<size_t>(args[0].asNumber());
            size_t deleteCount = args.size() > 1 ? static_cast<size_t>(args[1].asNumber()) : arr->size() - start;
            auto removed = std::make_shared<std::vector<JSValue>>();
            for (size_t i = 0; i < deleteCount && start < arr->size(); ++i) {
                removed->push_back((*arr)[start]);
                arr->erase(arr->begin() + start);
            }
            for (size_t i = 2; i < args.size(); ++i) {
                arr->insert(arr->begin() + start + i - 2, args[i]);
            }
            return JSValue::makeArray(removed);
        }
    }

    // Number methods
    if (object.isNumber()) {
        double n = object.asNumber();
        if (method == "toFixed") {
            int digits = args.empty() ? 0 : static_cast<int>(args[0].asNumber());
            std::ostringstream oss;
            oss.precision(digits);
            oss << std::fixed << n;
            return JSValue::makeString(oss.str());
        }
        if (method == "toString") {
            int radix = args.empty() ? 10 : static_cast<int>(args[0].asNumber());
            if (radix == 10) return JSValue::makeString(formatNumber(n));
            // Simple radix conversion for integers
            long val = static_cast<long>(n);
            if (radix == 16) { std::ostringstream oss; oss << std::hex << val; return JSValue::makeString(oss.str()); }
            if (radix == 8) { std::ostringstream oss; oss << std::oct << val; return JSValue::makeString(oss.str()); }
            if (radix == 2) { std::string r; while (val > 0) { r = (val % 2 ? "1" : "0") + r; val /= 2; } return JSValue::makeString(r.empty() ? "0" : r); }
            return JSValue::makeString(formatNumber(n));
        }
    }

    throw TypeError("Method " + method + " not found");
}

JSValue Interpreter::callFunction(const std::shared_ptr<JSFunction>& fn,
                                   const std::vector<JSValue>& args,
                                   JSValue) {
    auto fnEnv = std::make_shared<Environment>(fn->closure);
    fnEnv->isFunctionOrGlobal = true;
    for (size_t i = 0; i < fn->params.size(); ++i) {
        fnEnv->define(fn->params[i], i < args.size() ? args[i] : JSValue::makeUndefined());
    }
    try {
        // fn->body is not used in our current implementation since we use native wrappers
        return JSValue::makeUndefined();
    } catch (const ReturnSignal& ret) {
        return ret.value;
    }
}

// ─── var hoisting ────────────────────────────────────────────────────────────
void Interpreter::hoistVars(const std::vector<std::unique_ptr<ASTNode>>& stmts,
                            std::shared_ptr<Environment> env) {
    auto targetEnv = env->getFunctionOrGlobalEnv();
    // First pass: hoist function declarations (they take priority)
    for (const auto& stmt : stmts) {
        if (auto* fnDecl = dynamic_cast<const FunctionDecl*>(stmt.get())) {
            if (!fnDecl->isExpression && !fnDecl->name.empty()) {
                // Evaluate the function declaration and define it
                eval(*stmt, env);
            }
        }
    }
    // Second pass: hoist var declarations
    for (const auto& stmt : stmts) {
        if (auto* varDecl = dynamic_cast<const VarDecl*>(stmt.get())) {
            if (varDecl->kind == "var") {
                if (varDecl->pattern) {
                    std::vector<std::string> names;
                    collectPatternNames(varDecl->pattern.get(), names);
                    for (const auto& n : names) {
                        if (!targetEnv->has(n))
                            targetEnv->define(n, JSValue::makeUndefined());
                    }
                } else if (!varDecl->name.empty() && !targetEnv->has(varDecl->name)) {
                    targetEnv->define(varDecl->name, JSValue::makeUndefined());
                }
            }
        }
        // Recurse into blocks, if/else, loops, switch — but NOT into nested functions
        else if (auto* block = dynamic_cast<const BlockStmt*>(stmt.get())) {
            hoistVars(block->body, env);
        }
        else if (auto* ifStmt = dynamic_cast<const IfStmt*>(stmt.get())) {
            if (ifStmt->consequent) {
                if (auto* b = dynamic_cast<const BlockStmt*>(ifStmt->consequent.get()))
                    hoistVars(b->body, env);
            }
            if (ifStmt->alternate) {
                if (auto* b = dynamic_cast<const BlockStmt*>(ifStmt->alternate.get()))
                    hoistVars(b->body, env);
            }
        }
        else if (auto* forStmt = dynamic_cast<const ForStmt*>(stmt.get())) {
            if (forStmt->body) {
                if (auto* b = dynamic_cast<const BlockStmt*>(forStmt->body.get()))
                    hoistVars(b->body, env);
            }
        }
        else if (auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt.get())) {
            if (whileStmt->body) {
                if (auto* b = dynamic_cast<const BlockStmt*>(whileStmt->body.get()))
                    hoistVars(b->body, env);
            }
        }
        else if (auto* doWhile = dynamic_cast<const DoWhileStmt*>(stmt.get())) {
            if (doWhile->body) {
                if (auto* b = dynamic_cast<const BlockStmt*>(doWhile->body.get()))
                    hoistVars(b->body, env);
            }
        }
        else if (auto* switchStmt = dynamic_cast<const SwitchStmt*>(stmt.get())) {
            for (const auto& c : switchStmt->cases) {
                hoistVars(c.consequent, env);
            }
        }
        // Do NOT recurse into FunctionDecl — those have their own scope
    }
}

// ─── Destructuring helpers ──────────────────────────────────────────────────
void Interpreter::collectPatternNames(const ASTNode* pattern, std::vector<std::string>& names) {
    if (auto* op = dynamic_cast<const ObjPatternNode*>(pattern)) {
        for (const auto& e : op->elements) {
            if (e.isRest) { names.push_back(e.name); continue; }
            if (!e.name.empty()) names.push_back(e.name);
            if (e.subPattern) collectPatternNames(e.subPattern.get(), names);
        }
    } else if (auto* ap = dynamic_cast<const ArrPatternNode*>(pattern)) {
        for (const auto& e : ap->elements) {
            if (e.isHole) continue;
            if (e.isRest) { names.push_back(e.name); continue; }
            if (!e.name.empty()) names.push_back(e.name);
            if (e.subPattern) collectPatternNames(e.subPattern.get(), names);
        }
    }
}

void Interpreter::bindPattern(const ASTNode* pattern, const JSValue& value,
                               std::shared_ptr<Environment> env) {
    if (auto* op = dynamic_cast<const ObjPatternNode*>(pattern)) {
        if (!value.isObject()) throw TypeError("Cannot destructure non-object value");
        auto obj = value.asObject();
        for (const auto& e : op->elements) {
            if (e.isRest) {
                // Rest: collect all keys not matched by other patterns
                auto rest = std::make_shared<JSObject>();
                std::set<std::string> matched;
                for (const auto& el : op->elements) {
                    if (el.isRest) continue;
                    if (auto* id = dynamic_cast<const Identifier*>(el.key.get()))
                        matched.insert(id->name);
                }
                for (const auto& k : obj->keys) {
                    if (matched.find(k) == matched.end()) {
                        rest->keys.push_back(k);
                        rest->properties[k] = obj->properties.at(k);
                    }
                }
                env->define(e.name, JSValue::makeObject(rest));
                continue;
            }
            // Get key name
            std::string keyName;
            if (auto* id = dynamic_cast<const Identifier*>(e.key.get()))
                keyName = id->name;
            JSValue propVal = JSValue::makeUndefined();
            if (obj->properties.count(keyName))
                propVal = obj->properties.at(keyName);
            else if (e.defaultVal)
                propVal = eval(*e.defaultVal, env);
            if (e.subPattern) {
                bindPattern(e.subPattern.get(), propVal, env);
            } else {
                env->define(e.name, propVal);
            }
        }
    } else if (auto* ap = dynamic_cast<const ArrPatternNode*>(pattern)) {
        if (!value.isArray()) throw TypeError("Cannot destructure non-array value");
        auto arr = value.asArray();
        int idx = 0;
        for (const auto& e : ap->elements) {
            if (e.isHole) { idx++; continue; }
            if (e.isRest) {
                auto restArr = std::make_shared<std::vector<JSValue>>();
                for (size_t i = idx; i < arr->size(); ++i)
                    restArr->push_back((*arr)[i]);
                env->define(e.name, JSValue::makeArray(restArr));
                return; // rest consumes the rest
            }
            JSValue elemVal = JSValue::makeUndefined();
            if (idx < static_cast<int>(arr->size()))
                elemVal = (*arr)[idx];
            else if (e.defaultVal)
                elemVal = eval(*e.defaultVal, env);
            if (e.subPattern) {
                bindPattern(e.subPattern.get(), elemVal, env);
            } else {
                env->define(e.name, elemVal);
            }
            idx++;
        }
    }
}

void Interpreter::assignFromPattern(const ASTNode* target, const JSValue& value,
                                     std::shared_ptr<Environment> env) {
    if (auto* arrLit = dynamic_cast<const ArrayLiteral*>(target)) {
        if (!value.isArray()) throw TypeError("Cannot destructure non-array value");
        auto arr = value.asArray();
        int idx = 0;
        for (const auto& elem : arrLit->elements) {
            if (!elem) { idx++; continue; } // hole
            if (elem->kind == NodeKind::Spread) {
                auto* sp = dynamic_cast<const SpreadElement*>(elem.get());
                auto restArr = std::make_shared<std::vector<JSValue>>();
                for (size_t i = idx; i < arr->size(); ++i)
                    restArr->push_back((*arr)[i]);
                if (auto* id = dynamic_cast<const Identifier*>(sp->argument.get()))
                    env->set(id->name, JSValue::makeArray(restArr));
                return;
            }
            JSValue elemVal = (idx < static_cast<int>(arr->size())) ? (*arr)[idx] : JSValue::makeUndefined();
            if (auto* id = dynamic_cast<const Identifier*>(elem.get())) {
                env->set(id->name, elemVal);
            } else if (dynamic_cast<const ArrayLiteral*>(elem.get()) || dynamic_cast<const ObjectLiteral*>(elem.get())) {
                assignFromPattern(elem.get(), elemVal, env);
            }
            idx++;
        }
    } else if (auto* objLit = dynamic_cast<const ObjectLiteral*>(target)) {
        if (!value.isObject()) throw TypeError("Cannot destructure non-object value");
        auto obj = value.asObject();
        for (const auto& prop : objLit->properties) {
            std::string keyName;
            if (auto* id = dynamic_cast<const Identifier*>(prop.key.get()))
                keyName = id->name;
            else if (prop.computed)
                keyName = toString(eval(*prop.key, env));
            JSValue propVal = obj->properties.count(keyName) ? obj->properties.at(keyName) : JSValue::makeUndefined();
            if (auto* id = dynamic_cast<const Identifier*>(prop.value.get())) {
                env->set(id->name, propVal);
            } else if (dynamic_cast<const ArrayLiteral*>(prop.value.get()) || dynamic_cast<const ObjectLiteral*>(prop.value.get())) {
                assignFromPattern(prop.value.get(), propVal, env);
            }
        }
    }
}

} // namespace jsling
