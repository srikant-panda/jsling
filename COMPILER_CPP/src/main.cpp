#include "jsling/cli.hpp"
#include "jsling/errors.hpp"
#include "jsling/inspect.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        return jsling::runCli(argc, argv);
    } catch (const jsling::JSError& e) {
        // Top-level fallback — most errors handled inside runSource/runRepl
        std::cerr << "\033[1;31mError:\033[0m\033[1m " << e.what() << "\033[0m\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "\033[1;31mInternal error:\033[0m\033[1m " << e.what() << "\033[0m\n";
        return 1;
    }
}
