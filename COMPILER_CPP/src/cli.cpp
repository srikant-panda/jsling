#include "jsling/cli.hpp"
#include "jsling/lexer.hpp"
#include "jsling/parser.hpp"
#include "jsling/interpreter.hpp"
#include "jsling/errors.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

namespace jsling {

int runSource(const char* source) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto ast = parser.parse();
    Interpreter interp;
    interp.interpret(*ast);
    return 0;
}

int runFile(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file '" << path << "'\n";
        return 1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    return runSource(source.c_str());
}

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

int runRepl() {
    std::cout << "jsling v1.0.0 - JavaScript Runtime (REPL)\n";
    std::cout << "Type \"exit()\" or press Ctrl+D to quit\n\n";
    Interpreter interp;
    std::string buffer;
    while (true) {
        std::cout << (buffer.empty() ? ">>> " : "... ") << std::flush;
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
            interp.interpret(*ast);
        } catch (const JSError& e) {
            std::cerr << e.what() << '\n';
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << '\n';
        }
        buffer.clear();
    }
    return 0;
}

static void printUsage() {
    std::cout << "Usage: jsling [options] [script.js]\n";
    std::cout << "\nOptions:\n";
    std::cout << "  -e <code>    Evaluate JavaScript code\n";
    std::cout << "  --version    Print version\n";
    std::cout << "  --help       Print this help message\n";
    std::cout << "\nWith no arguments, starts an interactive REPL.\n";
}

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
