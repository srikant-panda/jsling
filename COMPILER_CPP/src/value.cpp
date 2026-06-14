#include "jsling/value.hpp"
#include <cmath>

namespace jsling {

JSValue JSValue::makeNull() { return JSValue{JSNull{}}; }
JSValue JSValue::makeUndefined() { return JSValue{JSUndefined{}}; }
JSValue JSValue::makeNumber(double n) { return JSValue{n}; }
JSValue JSValue::makeString(std::string s) { return JSValue{std::move(s)}; }
JSValue JSValue::makeBool(bool b) { return JSValue{b}; }
JSValue JSValue::makeArray(std::shared_ptr<std::vector<JSValue>> arr) { return JSValue{std::move(arr)}; }
JSValue JSValue::makeObject(std::shared_ptr<JSObject> obj) { return JSValue{std::move(obj)}; }
JSValue JSValue::makeFunction(std::shared_ptr<JSFunction> fn) { return JSValue{std::move(fn)}; }
JSValue JSValue::makeNativeFunction(std::shared_ptr<JSNativeFunction> fn) { return JSValue{std::move(fn)}; }

bool JSValue::isNull() const { return std::holds_alternative<JSNull>(data); }
bool JSValue::isUndefined() const { return std::holds_alternative<JSUndefined>(data) || std::holds_alternative<std::monostate>(data); }
bool JSValue::isNumber() const { return std::holds_alternative<double>(data); }
bool JSValue::isString() const { return std::holds_alternative<std::string>(data); }
bool JSValue::isBool() const { return std::holds_alternative<bool>(data); }
bool JSValue::isArray() const { return std::holds_alternative<std::shared_ptr<std::vector<JSValue>>>(data); }
bool JSValue::isObject() const { return std::holds_alternative<std::shared_ptr<JSObject>>(data); }
bool JSValue::isFunction() const { return std::holds_alternative<std::shared_ptr<JSFunction>>(data); }
bool JSValue::isNativeFunction() const { return std::holds_alternative<std::shared_ptr<JSNativeFunction>>(data); }

bool JSValue::isTruthy() const {
    if (isNull() || isUndefined()) return false;
    if (isBool()) return asBool();
    if (isNumber()) { double n = asNumber(); return n != 0 && !std::isnan(n); }
    if (isString()) return !asString().empty();
    return true;
}

double JSValue::asNumber() const { return std::get<double>(data); }
const std::string& JSValue::asString() const { return std::get<std::string>(data); }
bool JSValue::asBool() const { return std::get<bool>(data); }
std::shared_ptr<std::vector<JSValue>> JSValue::asArray() const { return std::get<std::shared_ptr<std::vector<JSValue>>>(data); }
std::shared_ptr<JSObject> JSValue::asObject() const { return std::get<std::shared_ptr<JSObject>>(data); }
std::shared_ptr<JSFunction> JSValue::asFunction() const { return std::get<std::shared_ptr<JSFunction>>(data); }
std::shared_ptr<JSNativeFunction> JSValue::asNativeFunction() const { return std::get<std::shared_ptr<JSNativeFunction>>(data); }

} // namespace jsling
