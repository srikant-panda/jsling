#include "jsling/inspect.hpp"
#include <cmath>
#include <sstream>
#include <cstdio>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <vector>
#include <string>
#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

namespace jsling {

// ─── TTY detection ──────────────────────────────────────────────────────────
static bool stderrIsTTY() { return isatty(fileno(stderr)) != 0; }

// ─── ANSI color codes ────────────────────────────────────────────────────────
namespace ansi {
    const char* RESET   = "\033[0m";
    const char* BOLD    = "\033[1m";
    const char* DIM     = "\033[2m";
    const char* RED     = "\033[31m";
    const char* GREEN   = "\033[32m";
    const char* YELLOW  = "\033[33m";
    const char* BLUE    = "\033[34m";
    const char* MAGENTA = "\033[35m";
    const char* CYAN    = "\033[36m";
    const char* GRAY    = "\033[90m";
    const char* BOLD_RED    = "\033[1;31m";
    const char* BOLD_YELLOW = "\033[1;33m";
    const char* BOLD_CYAN   = "\033[1;36m";
    const char* UNDERLINE   = "\033[4m";
}

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

// ─── Colored inspect (Node.js-like) ─────────────────────────────────────────
std::string colorInspect(const JSValue& value, int depth) {
    using namespace ansi;
    if (value.isNull())      return std::string(BOLD) + GRAY + "null" + RESET;
    if (value.isUndefined()) return std::string(GRAY) + "undefined" + RESET;
    if (value.isBool())      return std::string(YELLOW) + (value.asBool() ? "true" : "false") + RESET;
    if (value.isNumber())    return std::string(YELLOW) + formatNumber(value.asNumber()) + RESET;
    if (value.isString())    return std::string(GREEN) + "'" + escapeString(value.asString()) + "'" + RESET;
    if (value.isArray()) {
        auto arr = value.asArray();
        if (arr->empty()) return "[]";
        std::string s = "[ ";
        for (size_t i = 0; i < arr->size(); ++i) {
            if (i > 0) s += ", ";
            s += colorInspect((*arr)[i], depth + 1);
        }
        return s + " ]";
    }
    if (value.isObject()) {
        auto obj = value.asObject();
        // Date
        auto dateIt = obj->properties.find("__isDate__");
        if (dateIt != obj->properties.end() && dateIt->second.isBool() && dateIt->second.asBool()) {
            auto tsIt = obj->properties.find("__timestamp__");
            if (tsIt != obj->properties.end() && tsIt->second.isNumber()) {
                auto tp = std::chrono::system_clock::time_point(
                    std::chrono::milliseconds(static_cast<long long>(tsIt->second.asNumber())));
                std::time_t tt = std::chrono::system_clock::to_time_t(tp);
                char buf[128];
                std::strftime(buf, sizeof(buf), "%a %b %d %Y %H:%M:%S GMT%z", std::localtime(&tt));
                return std::string(MAGENTA) + "Date(" + buf + ")" + RESET;
            }
            return std::string(MAGENTA) + "Date(Invalid Date)" + RESET;
        }
        std::string s;
        bool first = true;
        for (const auto& key : obj->keys) {
            if (key == "__timestamp__" || key == "__isDate__") continue;
            if (!first) s += ", ";
            s += std::string(GREEN) + key + RESET + ": " + colorInspect(obj->properties.at(key), depth + 1);
            first = false;
        }
        if (first) return "{}";
        return "{ " + s + " }";
    }
    if (value.isFunction())
        return std::string(GRAY) + "[Function: " + value.asFunction()->name + "]" + RESET;
    if (value.isNativeFunction())
        return std::string(GRAY) + "[Function: " + value.asNativeFunction()->name + "]" + RESET;
    return std::string(GRAY) + "undefined" + RESET;
}

std::string colorDisplayString(const JSValue& value) {
    if (value.isString()) return value.asString();
    return colorInspect(value, 0);
}

// ─── Levenshtein distance ───────────────────────────────────────────────────
static int levenshtein(const std::string& a, const std::string& b) {
    int m = a.size(), n = b.size();
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1));
    for (int i = 0; i <= m; i++) dp[i][0] = i;
    for (int j = 0; j <= n; j++) dp[0][j] = j;
    for (int i = 1; i <= m; i++)
        for (int j = 1; j <= n; j++)
            dp[i][j] = std::min({dp[i-1][j]+1, dp[i][j-1]+1, dp[i-1][j-1] + (a[i-1]==b[j-1] ? 0 : 1)});
    return dp[m][n];
}

std::string suggestIdentifier(const std::string& name, const std::vector<std::string>& candidates) {
    if (candidates.empty()) return "";
    std::string best;
    int bestDist = 999;
    int threshold = std::max(2, (int)(name.size() / 2));
    for (const auto& c : candidates) {
        int d = levenshtein(name, c);
        if (d < bestDist) { bestDist = d; best = c; }
    }
    if (bestDist <= threshold) return best;
    return "";
}

// ─── Error formatting ───────────────────────────────────────────────────────
std::string formatError(const std::string& type, const std::string& message, int line,
                        const std::string& sourceLine, const std::string& suggestion) {
    using namespace ansi;
    bool color = stderrIsTTY();
    std::ostringstream out;
    if (color) out << BOLD_RED;
    out << type << ": ";
    if (color) out << BOLD;
    out << message;
    if (color) out << RESET;
    out << "\n";
    if (!sourceLine.empty()) {
        if (color) out << DIM;
        out << "  > ";
        if (color) out << RESET;
        out << sourceLine << "\n";
    }
    if (line > 0) {
        if (color) out << DIM;
        out << "    at line " << line;
        if (color) out << RESET;
        out << "\n";
    }
    if (!suggestion.empty()) {
        if (color) out << BOLD_YELLOW;
        out << "  hint: Did you mean '" << suggestion << "'?";
        if (color) out << RESET;
        out << "\n";
    }
    return out.str();
}

std::string formatRuntimeError(const std::string& type, const std::string& message,
                               const std::string& source, int line) {
    using namespace ansi;
    bool color = stderrIsTTY();
    std::ostringstream out;
    if (color) out << BOLD_RED;
    out << type << ": ";
    if (color) out << BOLD;
    out << message;
    if (color) out << RESET;
    out << "\n";
    if (line > 0 && !source.empty()) {
        // Find the source line
        std::istringstream ss(source);
        std::string srcLine;
        int curLine = 0;
        while (std::getline(ss, srcLine)) {
            curLine++;
            if (curLine == line) break;
        }
        if (curLine == line && !srcLine.empty()) {
            out << DIM << "  > " << RESET << srcLine << "\n";
            out << DIM << "    at line " << line << RESET << "\n";
        }
    } else if (line > 0) {
        out << DIM << "    at line " << line << RESET << "\n";
    }
    return out.str();
}

} // namespace jsling
