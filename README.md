# jsling

A from-scratch JavaScript runtime written in C++17 — no V8, no QuickJS, no embedded engines. Built for **Thunder Hackathon 2.0**.

`jsling` lexes, parses, and interprets JavaScript source code with a tree-walking interpreter, supporting a growing subset of ES6+ features.

## Quick Start

### Linux / macOS

```bash
git clone https://github.com/srikant-panda/jsling.git
cd jsling/COMPILER_CPP
bash scripts/build.sh
```

### Windows

Open **Developer Command Prompt for VS 2022+**, then:

```cmd
git clone https://github.com/srikant-panda/jsling.git
cd jsling\COMPILER_CPP
mkdir build && cd build
cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
nmake
```

Or just download `JSling-Setup.exe` from the [releases page](https://github.com/srikant-panda/jsling/releases) and install.

## Run It

```bash
# Run a script
./build/jsling script.js

# Evaluate an expression
./build/jsling -e "console.log(1 + 2)"

# Start the REPL
./build/jsling
```

On Windows, use `build\jsling.exe` (or `build-windows\jsling.exe`).

## Try It Out

Create a file `demo.js`:

```javascript
// Template literals
let name = "World";
console.log(`Hello, ${name}!`);

// Arrow functions
let double = x => x * 2;
console.log(double(21)); // 42

// Rest & spread
function sum(...nums) {
    let total = 0;
    for (let i = 0; i < nums.length; i++) {
        total = total + nums[i];
    }
    return total;
}
let arr = [1, 2, 3, 4];
console.log(sum(...arr)); // 10

// Array methods
let names = ["Alice", "Bob", "Charlie"];
names.filter(n => n.length > 3).forEach(n => console.log(n));

// Objects
let person = { name: "JSling", version: "1.0.0" };
console.log(`Running ${person.name} v${person.version}`);
```

```bash
./build/jsling demo.js
```

## Features

**Language:**
- `var`, `let`, `const` with comma-separated declarations (`let a = 1, b = 2`)
- Functions, closures, recursion, higher-order functions
- Arrow functions: `x => x * 2`, `(a, b) => a + b`, `() => 42`
- Default parameters: `function f(x = 1)`
- Rest parameters: `function f(...args)`
- Spread: `f(...arr)`, `[...a, ...b]`, `{...obj, z: 3}`
- Template literals: `` `Hello ${name}!` `` (including nested)
- `in` operator, bitwise ops (`&`, `|`, `^`, `<<`, `>>`, `>>>`)
- Prefix/postfix `++`/`--`, ternary `? :`
- `for`, `while`, `do-while`, `switch`, `break`, `continue`
- `==`, `===`, `!=`, `!==`, `<`, `>`, `<=`, `>=`

**Built-ins:**
- `console.log`, `Math.*`, `Date`, `parseInt`, `parseFloat`
- Array: `map`, `filter`, `reduce`, `forEach`, `find`, `some`, `every`, `sort`, `splice`, `slice`, `join`, `includes`, `indexOf`, `push`, `pop`, `shift`, `unshift`, `reverse`, `concat`
- String: `split`, `slice`, `includes`, `indexOf`, `replace`, `replaceAll`, `trim`, `toUpperCase`, `toLowerCase`, `startsWith`, `endsWith`, `repeat`, `padStart`, `padEnd`, `charAt`, `substring`, `concat`
- Number: `toFixed`, `toString(radix)`
- Object: `keys`, `values`, `entries`, `assign`, `freeze`

## Test

```bash
cd COMPILER_CPP
bash scripts/run-tests.sh
```

Runs all `.js` files in `tests/` and compares output against `.expected` files.

## Install System-Wide

**Linux/macOS:**
```bash
cd COMPILER_CPP
bash scripts/install-local.sh    # installs to /usr/local/bin or ~/.local/bin
```

**Windows:** Download `JSling-Setup.exe` from releases, or:
```cmd
build-installer.bat
```

## Architecture

```
JavaScript source
  → Lexer (tokenization)
  → Parser (recursive descent + precedence climbing)
  → AST
  → Interpreter (tree-walking)
  → Environment chain + built-ins
  → Output
```

## Project Layout

```
.
├── README.md                  # This file
├── INSTALL.md                 # Detailed installation guide
├── CPP_IMPLEMENTATION.md      # Implementation blueprint & status
├── jsling.iss                 # Windows installer (Inno Setup)
├── build-installer.bat        # Automated Windows build
├── assets/                    # Icons & branding
└── COMPILER_CPP/
    ├── CMakeLists.txt
    ├── include/jsling/        # Headers (lexer, parser, AST, interpreter, values)
    ├── src/                   # C++ implementation
    ├── scripts/               # Build, test, & install scripts
    └── tests/                 # Test suite
```

## Status

Core interpreter is **complete and working** (Phases 1–4). ES6+ features like arrow functions, template literals, rest/spread, and the `in` operator are implemented. Planned: destructuring, `try/catch`, classes, `for...of`, prototype chains.

See [CPP_IMPLEMENTATION.md](CPP_IMPLEMENTATION.md) for the full status table.

---

*Built for Thunder Hackathon 2.0 — June 2026*
