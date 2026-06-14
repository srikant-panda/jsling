# Install jsling

`jsling` is a C++17 JavaScript-like runtime with a Node-style command line interface.

This page shows how to install it on Linux, macOS, and Windows.

## Requirements

Before installing, make sure your system has:

- CMake 3.16 or newer
- A C++17 compiler
- Git
- A build tool supported by CMake

## Linux and macOS

From the project directory:

```bash
cd COMPILER_CPP
bash scripts/install-local.sh
```

This builds `jsling` in release mode and installs it to an auto-detected prefix:

- `/usr/local/bin` when writable
- otherwise `$HOME/.local/bin`

### Custom Install Location

```bash
cd COMPILER_CPP
bash scripts/install-local.sh --prefix "$HOME/.local"
```

### Uninstall

```bash
cd COMPILER_CPP
bash scripts/install-local.sh --uninstall
```

## Windows

Open PowerShell in the `COMPILER_CPP` directory.

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\install-windows.ps1
```

By default, this installs:

```text
%LOCALAPPDATA%\jsling\bin\jsling.exe
```

### Add jsling to PATH

```powershell
.\scripts\install-windows.ps1 -AddToPath
```

After this finishes, open a new terminal and run:

```powershell
jsling --version
```

### Custom Install Location

```powershell
.\scripts\install-windows.ps1 -Prefix "$env:USERPROFILE\bin\jsling" -AddToPath
```

### Uninstall

```powershell
.\scripts\install-windows.ps1 -Uninstall
```

## Verify Installation

```bash
jsling --version
jsling -e "console.log(1 + 2)"
```

Expected output:

```text
jsling v1.0.0
3
```

## Run a Script

Create a file named `hello.js`:

```javascript
console.log("hello from jsling");
```

Run it:

```bash
jsling hello.js
```

## Start the REPL

```bash
jsling
```

Use `exit()` or press `Ctrl+D` to leave the REPL.

## Troubleshooting

### `cmake` is not found

Install CMake:

```bash
# Ubuntu/Debian
sudo apt-get install -y cmake

# macOS with Homebrew
brew install cmake
```

On Windows:

```powershell
winget install Kitware.CMake
```

### C++ compiler is not found

Linux:

```bash
sudo apt-get install -y g++ make
```

macOS:

```bash
xcode-select --install
```

Windows:

Install one of:

- Visual Studio Build Tools
- LLVM/Clang
- MinGW/MSYS2

### `jsling` is not recognized

The install directory is probably not in your `PATH`.

Linux/macOS:

```bash
export PATH="$HOME/.local/bin:$PATH"
```

Windows:

Run the installer with:

```powershell
.\scripts\install-windows.ps1 -AddToPath
```

Then open a new terminal.
