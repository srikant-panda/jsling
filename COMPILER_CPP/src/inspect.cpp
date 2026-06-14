#include "jsling/inspect.hpp"
#include <cmath>
#include <sstream>
#include <cstdio>
#include <chrono>
#include <ctime>

namespace jsling {

std::string formatNumber(double n) {
    if (std::isnan(n)) return "NaN";
    if (std::isinf(n)) return n > 0 ? "Infinity" : "-Infinity";
    if (n == 0.0) return "0";
    // Integer-valued doubles: print without decimal point
    if (std::floor(n) == n && std::abs(n) < 1e15) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.0f", n);
        return std::string(buf);
    }
    // Use shortest representation that round-trips (like Node.js)
    char buf[64];
    for (int prec = 1; prec <= 17; ++prec) {
        std::snprintf(buf, sizeof(buf), "%.*g", prec, n);
        char *end;
        double parsed = std::strtod(buf, &end);
        if (parsed == n) return std::string(buf);
    }
    std::snprintf(buf, sizeof(buf), "%.17g", n);
    return std::string(buf);
}

std::string escapeString(const std::string& s) {
    std::string result;
    for (char c : s) {
        switch (c) {
            case '\'': result += "\\'"; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c;
        }
    }
    return result;
}

std::string inspect(const JSValue& value, int depth) {
    if (value.isNull()) return "null";
    if (value.isUndefined()) return "undefined";
    if (value.isBool()) return value.asBool() ? "true" : "false";
    if (value.isNumber()) return formatNumber(value.asNumber());
    if (value.isString()) return "'" + escapeString(value.asString()) + "'";
    if (value.isArray()) {
        auto arr = value.asArray();
        if (arr->empty()) return "[]";
        std::string s = "[ ";
        for (size_t i = 0; i < arr->size(); ++i) {
            if (i > 0) s += ", ";
            s += inspect((*arr)[i], depth + 1);
        }
        return s + " ]";
    }
    if (value.isObject()) {
        auto obj = value.asObject();
        // Check if this is a Date object
        auto dateIt = obj->properties.find("__isDate__");
        if (dateIt != obj->properties.end() && dateIt->second.isBool() && dateIt->second.asBool()) {
            auto tsIt = obj->properties.find("__timestamp__");
            if (tsIt != obj->properties.end() && tsIt->second.isNumber()) {
                auto tp = std::chrono::system_clock::time_point(
                    std::chrono::milliseconds(static_cast<long long>(tsIt->second.asNumber())));
                std::time_t tt = std::chrono::system_clock::to_time_t(tp);
                char buf[128];
                std::strftime(buf, sizeof(buf), "%a %b %d %Y %H:%M:%S GMT%z", std::localtime(&tt));
                return std::string("Date(") + buf + ")";
            }
            return "Date(Invalid Date)";
        }
        // Regular object - skip internal properties
        std::string s;
        bool first = true;
        for (const auto& key : obj->keys) {
            if (key == "__timestamp__" || key == "__isDate__") continue;
            if (!first) s += ", ";
            s += key + ": " + inspect(obj->properties.at(key), depth + 1);
            first = false;
        }
        if (first) return "{}";
        return "{ " + s + " }";
    }
    if (value.isFunction()) return "[Function: " + value.asFunction()->name + "]";
    if (value.isNativeFunction()) return "[Function: " + value.asNativeFunction()->name + "]";
    return "undefined";
}

std::string toDisplayString(const JSValue& value) {
    if (value.isString()) return value.asString();
    return inspect(value, 0);
}

} // namespace jsling
