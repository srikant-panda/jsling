# jsling

`jsling` is a from-scratch JavaScript-like runtime written in C++17. It is designed to behave like a small Node.js-style CLI for the supported language subset, without embedding V8, QuickJS, or any other JavaScript engine.

The active implementation lives in `COMPILER_CPP/`.

## Features

- Node-like CLI:
  - `jsling script.js`
  - `jsling -e "console.log(1 + 2)"`
  - `jsling` for the REPL
- Lexer, parser, AST, and tree-walking interpreter implemented in C++
- JavaScript-style values: numbers, strings, booleans, arrays, objects, functions, `null`, and `undefined`
- Core language support:
  - variables with `var`, `let`, and `const`
  - functions, closures, recursion, and higher-order calls
  - arrow functions
  - default parameters and rest parameters
  - spread in calls, arrays, and object literals
  - loops, conditionals, switch, break, and continue
  - loose and strict equality
  - common array, string, number, Math, Date, Object, and console behavior
- Node-like value display for `console.log`

Some larger JavaScript features are still in progress, including destructuring, template expressions, classes, and fuller error handling.

## Requirements

- CMake 3.16 or newer
- A C++17 compiler:
  - Linux/macOS: `g++` or `clang++`
  - Windows: Visual Studio Build Tools, LLVM/Clang, or MinGW
- A build tool supported by CMake, such as Make, Ninja, or MSBuild

## Build

From the C++ project directory:

```bash
cd COMPILER_CPP
bash scripts/build.sh
```

The binary is created at:

```text
COMPILER_CPP/build/jsling
```

On Windows, CMake may place the executable under a configuration directory such as:

```text
COMPILER_CPP/build/Release/jsling.exe
```

## Run

```bash
cd COMPILER_CPP

./build/jsling --version
./build/jsling -e "console.log(1 + 2)"
./build/jsling path/to/script.js
./build/jsling
```

## Test

```bash
cd COMPILER_CPP
bash scripts/run-tests.sh
```

The test runner builds the binary if needed, runs each `.js` file in `COMPILER_CPP/tests/`, and compares output with the matching `.expected` file. Tests with a `.skip` file are skipped with the reason shown in the output.

## Install

### Linux/macOS

Install from the local source tree:

```bash
cd COMPILER_CPP
bash scripts/install-local.sh
```

Choose a custom prefix:

```bash
bash scripts/install-local.sh --prefix "$HOME/.local"
```

Uninstall:

```bash
bash scripts/install-local.sh --uninstall
```

### Windows

From PowerShell in `COMPILER_CPP`:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\install-windows.ps1
```

Install and add the install directory to your user PATH:

```powershell
.\scripts\install-windows.ps1 -AddToPath
```

Use a custom prefix:

```powershell
.\scripts\install-windows.ps1 -Prefix "$env:USERPROFILE\bin\jsling" -AddToPath
```

Uninstall:

```powershell
.\scripts\install-windows.ps1 -Uninstall
```

## Project Layout

```text
.
├── CPP_IMPLEMENTATION.md      # Detailed implementation blueprint
├── README.md                  # Project overview and commands
└── COMPILER_CPP/
    ├── CMakeLists.txt
    ├── include/jsling/        # Public headers for lexer, parser, AST, values, interpreter
    ├── src/                   # C++ implementation
    ├── scripts/               # Build, test, and installer scripts
    └── tests/                 # JavaScript tests and expected output
```

## Architecture

```text
JavaScript source
  -> Lexer
  -> Parser
  -> AST
  -> Interpreter
  -> Environment + built-ins
  -> stdout/stderr
```

The design notes and implementation roadmap are documented in `CPP_IMPLEMENTATION.md`.
