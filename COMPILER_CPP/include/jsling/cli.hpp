#pragma once

namespace jsling {

int runCli(int argc, char* argv[]);
int runFile(const char* path);
int runSource(const char* source);
int runRepl();

} // namespace jsling
