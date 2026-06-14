#include "jsling/cli.hpp"
#include "jsling/errors.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        return jsling::runCli(argc, argv);
    } catch (const jsling::JSError& e) {
        std::cerr << e.what() << '\n';
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Internal error: " << e.what() << '\n';
        return 1;
    }
}
