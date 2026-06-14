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
  - variables with `var`, `let`, `const`, and comma-separated declarations (`let a = 1, b = 2`)
  - functions, closures, recursion, and higher-order calls
  - arrow functions: `x => x * 2`, `(a, b) => a + b`, `() => 42`
  - default parameters: `function f(x = 1)`
  - rest parameters: `function f(...args)`
  - spread in calls, arrays, and object literals: `f(...arr)`, `[...a, ...b]`, `{...obj, z: 3}`
  - template literals with expression interpolation: `` `Hello ${name}!` ``
  - nested template literals: `` `outer ${`inner ${x}`}` ``
  - `in` operator: `"name" in obj`
  - bitwise operators: `&`, `|`, `^`, `<<`, `>>`, `>>>`
  - postfix and prefix increment/decrement: `i++`, `++i`
  - loops, conditionals, switch, break, and continue
  - loose and strict equality (`==`, `===`)
  - ternary operator: `x ? a : b`
  - common array, string, number, Math, Date, Object, and console built-ins
- Node-like value display for `console.log`
- Windows GUI installer (`JSling-Setup.exe`) with PATH integration

Some larger JavaScript features are still in progress, including destructuring, classes, `try/catch`, and full prototype chains.

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

**Option A — GUI Installer (recommended):**

Download `JSling-Setup.exe` from the [releases page](https://github.com/srikant-panda/jsling/releases) and run it. The installer adds jsling to your PATH automatically.

**Option B — Build from source:**

Open **Developer Command Prompt for VS** and run:

```cmd
cd COMPILER_CPP
mkdir build-windows && cd build-windows
cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
nmake
```

The binary is at `COMPILER_CPP\build-windows\jsling.exe`.

**Option C — PowerShell:**

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\install-windows.ps1 -AddToPath
```

## Project Layout

```text
.
├── README.md                  # Project overview and commands
├── INSTALL.md                 # Detailed installation guide
├── CPP_IMPLEMENTATION.md      # Implementation blueprint and status
├── jsling.iss                 # Inno Setup script for Windows installer
├── build-installer.bat        # Automated Windows build script
├── assets/                    # Icons and branding
│   ├── jsling.ico             # Windows installer icon
│   └── jsling.svg             # Source SVG
└── COMPILER_CPP/
    ├── CMakeLists.txt
    ├── include/jsling/        # Public headers
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
