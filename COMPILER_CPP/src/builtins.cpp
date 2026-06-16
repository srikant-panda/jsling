#include "jsling/builtins.hpp"
#include "jsling/inspect.hpp"
#include "jsling/errors.hpp"
#include <iostream>
#include <cmath>
#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <random>
#include <limits>

namespace jsling {

void setupBuiltins(std::shared_ptr<Environment> env) {
    // console.log - colored Node.js-like output
    auto console = std::make_shared<JSObject>();
    console->keys.push_back("log");
    console->properties["log"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{
            "log",
            [](const std::vector<JSValue>& args) -> JSValue {
                bool color = isatty(fileno(stdout)) != 0;
                for (size_t i = 0; i < args.size(); ++i) {
                    if (i > 0) std::cout << " ";
                    std::cout << (color ? colorDisplayString(args[i]) : toDisplayString(args[i]));
                }
                std::cout << "\n";
                return JSValue::makeUndefined();
            }
        })
    );
    console->keys.push_back("warn");
    console->properties["warn"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{
            "warn",
            [](const std::vector<JSValue>& args) -> JSValue {
                bool color = isatty(fileno(stderr)) != 0;
                for (size_t i = 0; i < args.size(); ++i) {
                    if (i > 0) std::cerr << " ";
                    if (color) std::cerr << "\033[33m";
                    std::cerr << (color ? colorDisplayString(args[i]) : toDisplayString(args[i]));
                    if (color) std::cerr << "\033[0m";
                }
                std::cerr << "\n";
                return JSValue::makeUndefined();
            }
        })
    );
    console->keys.push_back("error");
    console->properties["error"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{
            "error",
            [](const std::vector<JSValue>& args) -> JSValue {
                bool color = isatty(fileno(stderr)) != 0;
                for (size_t i = 0; i < args.size(); ++i) {
                    if (i > 0) std::cerr << " ";
                    if (color) std::cerr << "\033[31m";
                    std::cerr << (color ? colorDisplayString(args[i]) : toDisplayString(args[i]));
                    if (color) std::cerr << "\033[0m";
                }
                std::cerr << "\n";
                return JSValue::makeUndefined();
            }
        })
    );
    env->define("console", JSValue::makeObject(console));

    // parseInt
    env->define("parseInt", JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{
            "parseInt",
            [](const std::vector<JSValue>& args) -> JSValue {
                if (args.empty()) return JSValue::makeNumber(std::nan(""));
                std::string s = args[0].isString() ? args[0].asString() : toDisplayString(args[0]);
                int radix = args.size() > 1 && args[1].isNumber() ? static_cast<int>(args[1].asNumber()) : 10;
                try {
                    size_t pos;
                    long result = std::stol(s, &pos, radix);
                    return JSValue::makeNumber(static_cast<double>(result));
                } catch (...) {
                    return JSValue::makeNumber(std::nan(""));
                }
            }
        })
    ));

    // parseFloat
    env->define("parseFloat", JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{
            "parseFloat",
            [](const std::vector<JSValue>& args) -> JSValue {
                if (args.empty()) return JSValue::makeNumber(std::nan(""));
                std::string s = args[0].isString() ? args[0].asString() : toDisplayString(args[0]);
                try {
                    size_t pos;
                    double result = std::stod(s, &pos);
                    return JSValue::makeNumber(result);
                } catch (...) {
                    return JSValue::makeNumber(std::nan(""));
                }
            }
        })
    ));

    // isNaN
    env->define("isNaN", JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{
            "isNaN",
            [](const std::vector<JSValue>& args) -> JSValue {
                if (args.empty()) return JSValue::makeBool(true);
                if (args[0].isNumber()) return JSValue::makeBool(std::isnan(args[0].asNumber()));
                return JSValue::makeBool(true);
            }
        })
    ));

    // isFinite
    env->define("isFinite", JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{
            "isFinite",
            [](const std::vector<JSValue>& args) -> JSValue {
                if (args.empty()) return JSValue::makeBool(false);
                if (args[0].isNumber()) {
                    double n = args[0].asNumber();
                    return JSValue::makeBool(!std::isnan(n) && !std::isinf(n));
                }
                return JSValue::makeBool(false);
            }
        })
    ));

    // Number
    env->define("Number", JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{
            "Number",
            [](const std::vector<JSValue>& args) -> JSValue {
                if (args.empty()) return JSValue::makeNumber(0);
                if (args[0].isNumber()) return args[0];
                if (args[0].isBool()) return JSValue::makeNumber(args[0].asBool() ? 1 : 0);
                if (args[0].isString()) {
                    try { return JSValue::makeNumber(std::stod(args[0].asString())); }
                    catch (...) { return JSValue::makeNumber(std::nan("")); }
                }
                if (args[0].isNull()) return JSValue::makeNumber(0);
                if (args[0].isUndefined()) return JSValue::makeNumber(std::nan(""));
                return JSValue::makeNumber(std::nan(""));
            }
        })
    ));

    // String
    env->define("String", JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{
            "String",
            [](const std::vector<JSValue>& args) -> JSValue {
                if (args.empty()) return JSValue::makeString("undefined");
                return JSValue::makeString(toDisplayString(args[0]));
            }
        })
    ));

    // Boolean
    env->define("Boolean", JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{
            "Boolean",
            [](const std::vector<JSValue>& args) -> JSValue {
                if (args.empty()) return JSValue::makeBool(false);
                return JSValue::makeBool(args[0].isTruthy());
            }
        })
    ));

    // Math object
    auto math = std::make_shared<JSObject>();
    math->keys = {"floor", "ceil", "round", "trunc", "abs", "max", "min", "pow", "sqrt", "cbrt",
                  "sign", "random", "PI", "E", "LN2", "LN10", "log", "log2", "log10", "exp", "hypot"};
    math->properties["floor"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{"floor", [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty()) return JSValue::makeNumber(std::nan(""));
            return JSValue::makeNumber(std::floor(args[0].asNumber()));
        }}));
    math->properties["ceil"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{"ceil", [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty()) return JSValue::makeNumber(std::nan(""));
            return JSValue::makeNumber(std::ceil(args[0].asNumber()));
        }}));
    math->properties["round"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{"round", [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty()) return JSValue::makeNumber(std::nan(""));
            return JSValue::makeNumber(std::round(args[0].asNumber()));
        }}));
    math->properties["abs"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{"abs", [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty()) return JSValue::makeNumber(std::nan(""));
            return JSValue::makeNumber(std::abs(args[0].asNumber()));
        }}));
    math->properties["max"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{"max", [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty()) return JSValue::makeNumber(-std::numeric_limits<double>::infinity());
            double m = args[0].asNumber();
            for (size_t i = 1; i < args.size(); ++i) m = std::max(m, args[i].asNumber());
            return JSValue::makeNumber(m);
        }}));
    math->properties["min"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{"min", [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty()) return JSValue::makeNumber(std::numeric_limits<double>::infinity());
            double m = args[0].asNumber();
            for (size_t i = 1; i < args.size(); ++i) m = std::min(m, args[i].asNumber());
            return JSValue::makeNumber(m);
        }}));
    math->properties["pow"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{"pow", [](const std::vector<JSValue>& args) -> JSValue {
            if (args.size() < 2) return JSValue::makeNumber(std::nan(""));
            return JSValue::makeNumber(std::pow(args[0].asNumber(), args[1].asNumber()));
        }}));
    math->properties["sqrt"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{"sqrt", [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty()) return JSValue::makeNumber(std::nan(""));
            return JSValue::makeNumber(std::sqrt(args[0].asNumber()));
        }}));
    math->properties["trunc"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{"trunc", [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty()) return JSValue::makeNumber(std::nan(""));
            return JSValue::makeNumber(std::trunc(args[0].asNumber()));
        }}));
    math->properties["cbrt"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{"cbrt", [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty()) return JSValue::makeNumber(std::nan(""));
            return JSValue::makeNumber(std::cbrt(args[0].asNumber()));
        }}));
    math->properties["sign"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{"sign", [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty()) return JSValue::makeNumber(std::nan(""));
            double n = args[0].asNumber();
            if (std::isnan(n)) return JSValue::makeNumber(std::nan(""));
            if (n > 0) return JSValue::makeNumber(1);
            if (n < 0) return JSValue::makeNumber(-1);
            return JSValue::makeNumber(0);
        }}));
    math->properties["log"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{"log", [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty()) return JSValue::makeNumber(std::nan(""));
            return JSValue::makeNumber(std::log(args[0].asNumber()));
        }}));
    math->properties["log2"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{"log2", [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty()) return JSValue::makeNumber(std::nan(""));
            return JSValue::makeNumber(std::log2(args[0].asNumber()));
        }}));
    math->properties["log10"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{"log10", [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty()) return JSValue::makeNumber(std::nan(""));
            return JSValue::makeNumber(std::log10(args[0].asNumber()));
        }}));
    math->properties["exp"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{"exp", [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty()) return JSValue::makeNumber(std::nan(""));
            return JSValue::makeNumber(std::exp(args[0].asNumber()));
        }}));
    math->properties["hypot"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{"hypot", [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty()) return JSValue::makeNumber(0);
            double sum = 0;
            for (const auto& a : args) sum += a.asNumber() * a.asNumber();
            return JSValue::makeNumber(std::sqrt(sum));
        }}));
    math->properties["PI"] = JSValue::makeNumber(3.141592653589793);
    math->properties["E"] = JSValue::makeNumber(2.718281828459045);
    math->properties["LN2"] = JSValue::makeNumber(0.6931471805599453);
    math->properties["LN10"] = JSValue::makeNumber(2.302585092994046);
    math->properties["random"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{"random", [](const std::vector<JSValue>&) -> JSValue {
            static std::mt19937 gen(std::random_device{}());
            static std::uniform_real_distribution<double> dist(0.0, 1.0);
            return JSValue::makeNumber(dist(gen));
        }}));
    env->define("Math", JSValue::makeObject(math));

    // Date constructor
    auto makeDateObj = [](double ms) -> JSValue {
        auto obj = std::make_shared<JSObject>();
        obj->keys.push_back("__timestamp__");
        obj->properties["__timestamp__"] = JSValue::makeNumber(ms);
        return JSValue::makeObject(obj);
    };

    auto dateMethods = std::make_shared<JSObject>();
    // getFullYear
    auto getFullYearFn = JSValue::makeNativeFunction(std::make_shared<JSNativeFunction>(JSNativeFunction{"getFullYear",
        [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty() || !args[0].isObject()) return JSValue::makeNumber(std::nan(""));
            auto obj = args[0].asObject();
            auto it = obj->properties.find("__timestamp__");
            if (it == obj->properties.end()) return JSValue::makeNumber(std::nan(""));
            auto tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(static_cast<long long>(it->second.asNumber())));
            std::time_t tt = std::chrono::system_clock::to_time_t(tp);
            std::tm tm = *std::localtime(&tt);
            return JSValue::makeNumber(tm.tm_year + 1900);
        }}));
    dateMethods->keys.push_back("getFullYear"); dateMethods->properties["getFullYear"] = getFullYearFn;
    // getMonth
    auto getMonthFn = JSValue::makeNativeFunction(std::make_shared<JSNativeFunction>(JSNativeFunction{"getMonth",
        [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty() || !args[0].isObject()) return JSValue::makeNumber(std::nan(""));
            auto obj = args[0].asObject();
            auto it = obj->properties.find("__timestamp__");
            if (it == obj->properties.end()) return JSValue::makeNumber(std::nan(""));
            auto tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(static_cast<long long>(it->second.asNumber())));
            std::time_t tt = std::chrono::system_clock::to_time_t(tp);
            std::tm tm = *std::localtime(&tt);
            return JSValue::makeNumber(tm.tm_mon);
        }}));
    dateMethods->keys.push_back("getMonth"); dateMethods->properties["getMonth"] = getMonthFn;
    // getDate
    auto getDateFn = JSValue::makeNativeFunction(std::make_shared<JSNativeFunction>(JSNativeFunction{"getDate",
        [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty() || !args[0].isObject()) return JSValue::makeNumber(std::nan(""));
            auto obj = args[0].asObject();
            auto it = obj->properties.find("__timestamp__");
            if (it == obj->properties.end()) return JSValue::makeNumber(std::nan(""));
            auto tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(static_cast<long long>(it->second.asNumber())));
            std::time_t tt = std::chrono::system_clock::to_time_t(tp);
            std::tm tm = *std::localtime(&tt);
            return JSValue::makeNumber(tm.tm_mday);
        }}));
    dateMethods->keys.push_back("getDate"); dateMethods->properties["getDate"] = getDateFn;
    // getDay
    auto getDayFn = JSValue::makeNativeFunction(std::make_shared<JSNativeFunction>(JSNativeFunction{"getDay",
        [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty() || !args[0].isObject()) return JSValue::makeNumber(std::nan(""));
            auto obj = args[0].asObject();
            auto it = obj->properties.find("__timestamp__");
            if (it == obj->properties.end()) return JSValue::makeNumber(std::nan(""));
            auto tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(static_cast<long long>(it->second.asNumber())));
            std::time_t tt = std::chrono::system_clock::to_time_t(tp);
            std::tm tm = *std::localtime(&tt);
            return JSValue::makeNumber(tm.tm_wday);
        }}));
    dateMethods->keys.push_back("getDay"); dateMethods->properties["getDay"] = getDayFn;
    // getHours
    auto getHoursFn = JSValue::makeNativeFunction(std::make_shared<JSNativeFunction>(JSNativeFunction{"getHours",
        [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty() || !args[0].isObject()) return JSValue::makeNumber(std::nan(""));
            auto obj = args[0].asObject();
            auto it = obj->properties.find("__timestamp__");
            if (it == obj->properties.end()) return JSValue::makeNumber(std::nan(""));
            auto tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(static_cast<long long>(it->second.asNumber())));
            std::time_t tt = std::chrono::system_clock::to_time_t(tp);
            std::tm tm = *std::localtime(&tt);
            return JSValue::makeNumber(tm.tm_hour);
        }}));
    dateMethods->keys.push_back("getHours"); dateMethods->properties["getHours"] = getHoursFn;
    // getMinutes
    auto getMinutesFn = JSValue::makeNativeFunction(std::make_shared<JSNativeFunction>(JSNativeFunction{"getMinutes",
        [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty() || !args[0].isObject()) return JSValue::makeNumber(std::nan(""));
            auto obj = args[0].asObject();
            auto it = obj->properties.find("__timestamp__");
            if (it == obj->properties.end()) return JSValue::makeNumber(std::nan(""));
            auto tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(static_cast<long long>(it->second.asNumber())));
            std::time_t tt = std::chrono::system_clock::to_time_t(tp);
            std::tm tm = *std::localtime(&tt);
            return JSValue::makeNumber(tm.tm_min);
        }}));
    dateMethods->keys.push_back("getMinutes"); dateMethods->properties["getMinutes"] = getMinutesFn;
    // getSeconds
    auto getSecondsFn = JSValue::makeNativeFunction(std::make_shared<JSNativeFunction>(JSNativeFunction{"getSeconds",
        [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty() || !args[0].isObject()) return JSValue::makeNumber(std::nan(""));
            auto obj = args[0].asObject();
            auto it = obj->properties.find("__timestamp__");
            if (it == obj->properties.end()) return JSValue::makeNumber(std::nan(""));
            auto tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(static_cast<long long>(it->second.asNumber())));
            std::time_t tt = std::chrono::system_clock::to_time_t(tp);
            std::tm tm = *std::localtime(&tt);
            return JSValue::makeNumber(tm.tm_sec);
        }}));
    dateMethods->keys.push_back("getSeconds"); dateMethods->properties["getSeconds"] = getSecondsFn;
    // getMilliseconds
    auto getMsFn = JSValue::makeNativeFunction(std::make_shared<JSNativeFunction>(JSNativeFunction{"getMilliseconds",
        [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty() || !args[0].isObject()) return JSValue::makeNumber(std::nan(""));
            auto obj = args[0].asObject();
            auto it = obj->properties.find("__timestamp__");
            if (it == obj->properties.end()) return JSValue::makeNumber(std::nan(""));
            long long totalMs = static_cast<long long>(it->second.asNumber());
            return JSValue::makeNumber(totalMs % 1000);
        }}));
    dateMethods->keys.push_back("getMilliseconds"); dateMethods->properties["getMilliseconds"] = getMsFn;
    // getTime
    auto getTimeFn = JSValue::makeNativeFunction(std::make_shared<JSNativeFunction>(JSNativeFunction{"getTime",
        [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty() || !args[0].isObject()) return JSValue::makeNumber(std::nan(""));
            auto obj = args[0].asObject();
            auto it = obj->properties.find("__timestamp__");
            if (it == obj->properties.end()) return JSValue::makeNumber(std::nan(""));
            return it->second;
        }}));
    dateMethods->keys.push_back("getTime"); dateMethods->properties["getTime"] = getTimeFn;
    // toISOString
    auto toIsoFn = JSValue::makeNativeFunction(std::make_shared<JSNativeFunction>(JSNativeFunction{"toISOString",
        [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty() || !args[0].isObject()) return JSValue::makeString("Invalid Date");
            auto obj = args[0].asObject();
            auto it = obj->properties.find("__timestamp__");
            if (it == obj->properties.end()) return JSValue::makeString("Invalid Date");
            auto tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(static_cast<long long>(it->second.asNumber())));
            std::time_t tt = std::chrono::system_clock::to_time_t(tp);
            std::tm tm = *std::gmtime(&tt);
            long long ms = static_cast<long long>(it->second.asNumber()) % 1000;
            char buf[64];
            std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tm);
            std::string result = std::string(buf);
            char msbuf[8];
            std::snprintf(msbuf, sizeof(msbuf), ".%03lldZ", ms < 0 ? -ms : ms);
            return JSValue::makeString(result + msbuf);
        }}));
    dateMethods->keys.push_back("toISOString"); dateMethods->properties["toISOString"] = toIsoFn;
    // toString
    auto dateToStrFn = JSValue::makeNativeFunction(std::make_shared<JSNativeFunction>(JSNativeFunction{"toString",
        [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty() || !args[0].isObject()) return JSValue::makeString("Invalid Date");
            auto obj = args[0].asObject();
            auto it = obj->properties.find("__timestamp__");
            if (it == obj->properties.end()) return JSValue::makeString("Invalid Date");
            auto tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(static_cast<long long>(it->second.asNumber())));
            std::time_t tt = std::chrono::system_clock::to_time_t(tp);
            char buf[128];
            std::strftime(buf, sizeof(buf), "%a %b %d %Y %H:%M:%S GMT%z", std::localtime(&tt));
            return JSValue::makeString(std::string(buf));
        }}));
    dateMethods->keys.push_back("toString"); dateMethods->properties["toString"] = dateToStrFn;
    // toDateString
    auto toDateStrFn = JSValue::makeNativeFunction(std::make_shared<JSNativeFunction>(JSNativeFunction{"toDateString",
        [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty() || !args[0].isObject()) return JSValue::makeString("Invalid Date");
            auto obj = args[0].asObject();
            auto it = obj->properties.find("__timestamp__");
            if (it == obj->properties.end()) return JSValue::makeString("Invalid Date");
            auto tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(static_cast<long long>(it->second.asNumber())));
            std::time_t tt = std::chrono::system_clock::to_time_t(tp);
            char buf[64];
            std::strftime(buf, sizeof(buf), "%a %b %d %Y", std::localtime(&tt));
            return JSValue::makeString(std::string(buf));
        }}));
    dateMethods->keys.push_back("toDateString"); dateMethods->properties["toDateString"] = toDateStrFn;
    // toLocaleDateString
    dateMethods->keys.push_back("toLocaleDateString"); dateMethods->properties["toLocaleDateString"] = toDateStrFn;
    // toTimeString
    auto toTimeStrFn = JSValue::makeNativeFunction(std::make_shared<JSNativeFunction>(JSNativeFunction{"toTimeString",
        [](const std::vector<JSValue>& args) -> JSValue {
            if (args.empty() || !args[0].isObject()) return JSValue::makeString("Invalid Date");
            auto obj = args[0].asObject();
            auto it = obj->properties.find("__timestamp__");
            if (it == obj->properties.end()) return JSValue::makeString("Invalid Date");
            auto tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(static_cast<long long>(it->second.asNumber())));
            std::time_t tt = std::chrono::system_clock::to_time_t(tp);
            char buf[64];
            std::strftime(buf, sizeof(buf), "%H:%M:%S GMT%z", std::localtime(&tt));
            return JSValue::makeString(std::string(buf));
        }}));
    dateMethods->keys.push_back("toTimeString"); dateMethods->properties["toTimeString"] = toTimeStrFn;
    // toJSON (same as toISOString)
    dateMethods->keys.push_back("toJSON"); dateMethods->properties["toJSON"] = toIsoFn;
    // valueOf (same as getTime)
    dateMethods->keys.push_back("valueOf"); dateMethods->properties["valueOf"] = getTimeFn;

    // Date constructor function
    env->define("Date", JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{
            "Date",
            [makeDateObj, dateMethods](const std::vector<JSValue>& args) -> JSValue {
                double ms;
                if (args.empty()) {
                    auto now = std::chrono::system_clock::now();
                    ms = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
                } else if (args.size() == 1) {
                    if (args[0].isNumber()) {
                        ms = args[0].asNumber();
                    } else if (args[0].isString()) {
                        // Try to parse ISO date string - simplified
                        ms = 0; // fallback
                    } else {
                        ms = std::nan("");
                    }
                } else {
                    // new Date(year, month, day, hours, minutes, seconds, ms)
                    int year = static_cast<int>(args[0].asNumber());
                    int month = static_cast<int>(args[1].asNumber());
                    int day = args.size() > 2 ? static_cast<int>(args[2].asNumber()) : 1;
                    int hours = args.size() > 3 ? static_cast<int>(args[3].asNumber()) : 0;
                    int minutes = args.size() > 4 ? static_cast<int>(args[4].asNumber()) : 0;
                    int seconds = args.size() > 5 ? static_cast<int>(args[5].asNumber()) : 0;
                    int millis = args.size() > 6 ? static_cast<int>(args[6].asNumber()) : 0;
                    std::tm tm = {};
                    tm.tm_year = year - 1900;
                    tm.tm_mon = month;
                    tm.tm_mday = day;
                    tm.tm_hour = hours;
                    tm.tm_min = minutes;
                    tm.tm_sec = seconds;
                    tm.tm_isdst = -1;
                    std::time_t tt = std::mktime(&tm);
                    ms = static_cast<double>(tt * 1000 + millis);
                }
                auto dateObj = std::make_shared<JSObject>();
                dateObj->keys.push_back("__timestamp__");
                dateObj->properties["__timestamp__"] = JSValue::makeNumber(ms);
                dateObj->keys.push_back("__isDate__");
                dateObj->properties["__isDate__"] = JSValue::makeBool(true);
                // Add all methods
                for (const auto& key : dateMethods->keys) {
                    dateObj->keys.push_back(key);
                    dateObj->properties[key] = dateMethods->properties[key];
                }
                return JSValue::makeObject(dateObj);
            }
        })
    ));

    // Date.now() as a static method
    auto dateObj = std::make_shared<JSObject>();
    dateObj->keys.push_back("now");
    dateObj->properties["now"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{"now",
            [](const std::vector<JSValue>&) -> JSValue {
                auto now = std::chrono::system_clock::now();
                return JSValue::makeNumber(static_cast<double>(
                    std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()));
            }}));
    // Merge Date.now into the Date constructor by wrapping it
    // Since Date is a function, we need to add `now` as a property on the function object
    // For simplicity, store Date.now as a separate global that's accessible via member access
    // Actually, we can't easily add properties to a function. Let's use a different approach:
    // Store Date as an object with both callable and static methods.
    // For now, let's handle Date.now() specially in the interpreter's evalCall.

    // Object constructor with static methods
    auto objectObj = std::make_shared<JSObject>();
    objectObj->keys = {"keys", "values", "entries"};
    objectObj->properties["keys"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{"keys",
            [](const std::vector<JSValue>& args) -> JSValue {
                if (args.empty() || !args[0].isObject()) return JSValue::makeArray(std::make_shared<std::vector<JSValue>>());
                auto obj = args[0].asObject();
                auto arr = std::make_shared<std::vector<JSValue>>();
                for (const auto& key : obj->keys) {
                    if (key == "__timestamp__" || key == "__isDate__") continue;
                    arr->push_back(JSValue::makeString(key));
                }
                return JSValue::makeArray(arr);
            }}));
    objectObj->properties["values"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{"values",
            [](const std::vector<JSValue>& args) -> JSValue {
                if (args.empty() || !args[0].isObject()) return JSValue::makeArray(std::make_shared<std::vector<JSValue>>());
                auto obj = args[0].asObject();
                auto arr = std::make_shared<std::vector<JSValue>>();
                for (const auto& key : obj->keys) {
                    if (key == "__timestamp__" || key == "__isDate__") continue;
                    arr->push_back(obj->properties.at(key));
                }
                return JSValue::makeArray(arr);
            }}));
    objectObj->properties["entries"] = JSValue::makeNativeFunction(
        std::make_shared<JSNativeFunction>(JSNativeFunction{"entries",
            [](const std::vector<JSValue>& args) -> JSValue {
                if (args.empty() || !args[0].isObject()) return JSValue::makeArray(std::make_shared<std::vector<JSValue>>());
                auto obj = args[0].asObject();
                auto arr = std::make_shared<std::vector<JSValue>>();
                for (const auto& key : obj->keys) {
                    if (key == "__timestamp__" || key == "__isDate__") continue;
                    auto pair = std::make_shared<std::vector<JSValue>>();
                    pair->push_back(JSValue::makeString(key));
                    pair->push_back(obj->properties.at(key));
                    arr->push_back(JSValue::makeArray(pair));
                }
                return JSValue::makeArray(arr);
            }}));
    env->define("Object", JSValue::makeObject(objectObj));
    env->define("__dateMethods__", JSValue::makeObject(dateMethods));

    // undefined and null globals
    env->define("undefined", JSValue::makeUndefined());
    env->define("null", JSValue::makeNull());
    env->define("NaN", JSValue::makeNumber(std::nan("")));
    env->define("Infinity", JSValue::makeNumber(std::numeric_limits<double>::infinity()));
}

} // namespace jsling
