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

### Build the GUI Installer (.exe)

The project includes an Inno Setup script (`jsling.iss`) that produces a professional Windows installer (`JSling-Setup.exe`) with:
- Wizard-style GUI installation
- Automatic PATH integration
- Desktop shortcut option
- Clean uninstaller

**Prerequisites:**
- Visual Studio Build Tools (with "Desktop development with C++")
- CMake (https://cmake.org/download/)
- Inno Setup 6 (https://jrsoftware.org/isdl.php)

**Steps (in Developer Command Prompt for VS):**

```cmd
cd C:\path\to\jsling\COMPILER_CPP
mkdir build-windows && cd build-windows
cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
nmake
cd ..\..
mkdir dist
copy COMPILER_CPP\build-windows\jsling.exe jsling.exe
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" jsling.iss
```

The installer is generated at `dist\JSling-Setup.exe`.

**Or use the automated script:**

```cmd
build-installer.bat
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
