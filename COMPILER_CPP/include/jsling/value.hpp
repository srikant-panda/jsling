#pragma once
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <variant>

namespace jsling {

struct JSNull {};
struct JSUndefined {};
struct JSObject;
struct JSFunction;
struct JSNativeFunction;

struct JSValue {
    std::variant<
        std::monostate,
        JSNull,
        JSUndefined,
        bool,
        double,
        std::string,
        std::shared_ptr<std::vector<JSValue>>,
        std::shared_ptr<JSObject>,
        std::shared_ptr<JSFunction>,
        std::shared_ptr<JSNativeFunction>
    > data;

    static JSValue makeNull();
    static JSValue makeUndefined();
    static JSValue makeNumber(double n);
    static JSValue makeString(std::string s);
    static JSValue makeBool(bool b);
    static JSValue makeArray(std::shared_ptr<std::vector<JSValue>> arr);
    static JSValue makeObject(std::shared_ptr<JSObject> obj);
    static JSValue makeFunction(std::shared_ptr<JSFunction> fn);
    static JSValue makeNativeFunction(std::shared_ptr<JSNativeFunction> fn);

    bool isNull() const;
    bool isUndefined() const;
    bool isNumber() const;
    bool isString() const;
    bool isBool() const;
    bool isArray() const;
    bool isObject() const;
    bool isFunction() const;
    bool isNativeFunction() const;
    bool isTruthy() const;

    double asNumber() const;
    const std::string& asString() const;
    bool asBool() const;
    std::shared_ptr<std::vector<JSValue>> asArray() const;
    std::shared_ptr<JSObject> asObject() const;
    std::shared_ptr<JSFunction> asFunction() const;
    std::shared_ptr<JSNativeFunction> asNativeFunction() const;
};

struct JSObject {
    std::vector<std::string> keys;
    std::unordered_map<std::string, JSValue> properties;
};

struct JSFunction {
    std::string name;
    std::vector<std::string> params;
    std::shared_ptr<struct ASTNode> body;
    std::shared_ptr<class Environment> closure;
};

struct JSNativeFunction {
    std::string name;
    std::function<JSValue(const std::vector<JSValue>&)> fn;
};

// ReturnSignal with actual JSValue
struct ReturnSignal {
    JSValue value;
    explicit ReturnSignal(JSValue v) : value(std::move(v)) {}
};

struct BreakSignal {};
struct ContinueSignal {};

} // namespace jsling
