#include "jsling/cli.hpp"
#include "jsling/lexer.hpp"
#include "jsling/parser.hpp"
#include "jsling/interpreter.hpp"
#include "jsling/inspect.hpp"
#include "jsling/errors.hpp"
#include "jsling/token.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

namespace jsling {

// ─── Spell-check keyword list ───────────────────────────────────────────────
static const std::vector<std::string> JS_KEYWORDS = {
    "var", "let", "const", "function", "return", "if", "else", "for", "while", "do",
    "switch", "case", "default", "break", "continue", "throw", "try", "catch",
    "class", "extends", "new", "delete", "typeof", "void", "in", "of", "instanceof",
    "true", "false", "null", "undefined", "this", "console", "Math", "Date",
    "Object", "Array", "String", "Number", "Boolean", "JSON", "parseInt", "parseFloat",
    "NaN", "Infinity", "arguments", "length", "prototype"
};

static std::string getSourceLine(const std::string& source, int lineNum) {
    if (lineNum <= 0 || source.empty()) return "";
    std::istringstream ss(source);
    std::string line;
    int cur = 0;
    while (std::getline(ss, line)) {
        cur++;
        if (cur == lineNum) return line;
    }
    return "";
}

static int extractLineFromMessage(const std::string& msg) {
    auto pos = msg.find("at line ");
    if (pos != std::string::npos) {
        try { return std::stoi(msg.substr(pos + 8)); }
        catch (...) { return -1; }
    }
    return -1;
}

static std::string extractVarNameFromRefError(const std::string& msg) {
    // "name is not defined" → extract "name"
    auto pos = msg.find(" is not defined");
    if (pos != std::string::npos) return msg.substr(0, pos);
    return "";
}

static std::string cleanMessage(const std::string& msg) {
    // Remove " at line N" suffix since we show it separately
    auto pos = msg.find(" at line ");
    if (pos != std::string::npos) return msg.substr(0, pos);
    return msg;
}

static std::string classifyError(const JSError& e) {
    const char* name = typeid(e).name();
    std::string s(name);
    // Demangle common patterns
    if (s.find("SyntaxError") != std::string::npos) return "SyntaxError";
    if (s.find("ReferenceError") != std::string::npos) return "ReferenceError";
    if (s.find("TypeError") != std::string::npos) return "TypeError";
    if (s.find("RangeError") != std::string::npos) return "RangeError";
    return "Error";
}

// ─── Print verbose error ────────────────────────────────────────────────────
static void printVerboseError(const JSError& e, const std::string& source, Interpreter* interp = nullptr) {
    std::string msg = e.what();
    std::string type = classifyError(e);
    std::string cleanMsg = cleanMessage(msg);
    int line = extractLineFromMessage(msg);
    std::string srcLine = getSourceLine(source, line);

    // Spell-check for ReferenceError
    std::string suggestion;
    if (type == "ReferenceError") {
        std::string varName = extractVarNameFromRefError(cleanMsg);
        if (!varName.empty()) {
            // Build candidates: keywords + in-scope variables
            std::vector<std::string> candidates = JS_KEYWORDS;
            if (interp) {
                auto env = interp->getGlobalEnv();
                auto varNames = env->visibleNames();
                candidates.insert(candidates.end(), varNames.begin(), varNames.end());
            }
            suggestion = suggestIdentifier(varName, candidates);
        }
    }

    // Spell-check for SyntaxError with "Unexpected token"
    if (type == "SyntaxError" && cleanMsg.find("Unexpected") != std::string::npos) {
        // Try to find a keyword that the user may have misspelled
        auto qPos = cleanMsg.find('\'');
        if (qPos != std::string::npos) {
            auto qEnd = cleanMsg.find('\'', qPos + 1);
            if (qEnd != std::string::npos) {
                std::string token = cleanMsg.substr(qPos + 1, qEnd - qPos - 1);
                suggestion = suggestIdentifier(token, JS_KEYWORDS);
            }
        }
    }

    std::cerr << formatError(type, cleanMsg, line, srcLine, suggestion);
}

// ─── Run source code ────────────────────────────────────────────────────────
int runSource(const char* source) {
    Interpreter interp;
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(std::move(tokens));
        auto ast = parser.parse();
        interp.interpret(*ast);
    } catch (const JSError& e) {
        printVerboseError(e, source, &interp);
        return 1;
    }
    return 0;
}

// ─── Run file ───────────────────────────────────────────────────────────────
int runFile(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "\033[1;31mError:\033[0m\033[1m Cannot open file '" << path << "'\033[0m\n";
        return 1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    return runSource(source.c_str());
}

// ─── REPL brace-balancing ───────────────────────────────────────────────────
static bool isBalanced(const std::string& code) {
    int braces = 0, parens = 0, brackets = 0;
    bool inString = false;
    char stringChar = 0;
    for (size_t i = 0; i < code.size(); ++i) {
        char c = code[i];
        if (inString) {
            if (c == '\\') { ++i; continue; }
            if (c == stringChar) inString = false;
            continue;
        }
        if (c == '\'' || c == '"' || c == '`') { inString = true; stringChar = c; continue; }
        if (c == '{') braces++;
        else if (c == '}') braces--;
        else if (c == '(') parens++;
        else if (c == ')') parens--;
        else if (c == '[') brackets++;
        else if (c == ']') brackets--;
    }
    return braces <= 0 && parens <= 0 && brackets <= 0;
}

// ─── Check if the input is a pure expression (for REPL result display) ─────
static bool isExpressionInput(const std::string& code) {
    // Trim whitespace
    std::string trimmed = code;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);

    // If it starts with a keyword that's a statement, don't show result
    static const std::vector<std::string> stmtKeywords = {
        "var ", "let ", "const ", "function ", "if", "for", "while",
        "do ", "switch", "return", "class ", "throw"
    };
    for (const auto& kw : stmtKeywords) {
        if (trimmed.substr(0, kw.size()) == kw) return false;
    }
    return true;
}

// ─── REPL ───────────────────────────────────────────────────────────────────
int runRepl() {
    // Check for terminal color support
    const char* term = std::getenv("TERM");
    bool hasColor = term && std::string(term) != "dumb";

    std::cout << "\033[1;36mjsling\033[0m \033[90mv1.0.0\033[0m - JavaScript Runtime (REPL)\n";
    std::cout << "Type \033[33m\"exit()\"\033[0m or press \033[33mCtrl+D\033[0m to quit\n\n";

    Interpreter interp;
    std::string buffer;
    while (true) {
        if (hasColor) {
            std::cout << (buffer.empty() ? "\033[1;32m> \033[0m" : "\033[1;32m... \033[0m") << std::flush;
        } else {
            std::cout << (buffer.empty() ? "> " : "... ") << std::flush;
        }
        std::string line;
        if (!std::getline(std::cin, line)) { std::cout << '\n'; break; }
        if (buffer.empty() && (line == "exit()" || line == "quit()")) break;
        if (line.empty() && buffer.empty()) continue;
        if (!buffer.empty()) buffer += "\n";
        buffer += line;
        if (!isBalanced(buffer)) continue;

        try {
            Lexer lexer(buffer);
            auto tokens = lexer.tokenize();
            Parser parser(std::move(tokens));
            auto ast = parser.parse();

            JSValue result = interp.interpretAndReturn(*ast);

            // Show result for expression inputs (like Node.js)
            if (isExpressionInput(buffer) && !result.isUndefined()) {
                if (hasColor) {
                    std::cout << colorInspect(result, 0) << "\n";
                } else {
                    std::cout << inspect(result, 0) << "\n";
                }
            }
        } catch (const JSError& e) {
            printVerboseError(e, buffer, &interp);
        } catch (const std::exception& e) {
            std::cerr << "\033[1;31mInternal Error:\033[0m\033[1m " << e.what() << "\033[0m\n";
        }
        buffer.clear();
    }
    return 0;
}

// ─── Usage ──────────────────────────────────────────────────────────────────
static void printUsage() {
    std::cout << "Usage: jsling [options] [script.js]\n";
    std::cout << "\nOptions:\n";
    std::cout << "  -e <code>    Evaluate JavaScript code\n";
    std::cout << "  --version    Print version\n";
    std::cout << "  --help       Print this help message\n";
    std::cout << "\nWith no arguments, starts an interactive REPL.\n";
}

// ─── CLI entry ──────────────────────────────────────────────────────────────
int runCli(int argc, char* argv[]) {
    if (argc == 1) return runRepl();
    std::string arg = argv[1];
    if (arg == "-e") {
        if (argc < 3) { printUsage(); return 1; }
        return runSource(argv[2]);
    }
    if (arg == "--version") {
        std::cout << "jsling v1.0.0\n";
        return 0;
    }
    if (arg == "--help") {
        printUsage();
        return 0;
    }
    return runFile(argv[1]);
}

} // namespace jsling
