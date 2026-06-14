#!/usr/bin/env bash
# build-windows-installer.sh
# Cross-compiles jsling for Windows and builds the GUI installer (.exe)
#
# Requirements (Arch Linux):
#   sudo pacman -S mingw-w64-gcc mingw-w64-cmake wine
#   # Then install Inno Setup via Wine (see below)
#
# Requirements (Ubuntu/Debian):
#   sudo apt install mingw-w64 cmake wine
#
# Usage:
#   ./build-windows-installer.sh          # build installer
#   ./build-windows-installer.sh --exe-only  # just cross-compile jsling.exe

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
SRC_DIR="$PROJECT_DIR/COMPILER_CPP"
BUILD_DIR="$SRC_DIR/build-windows"
OUTPUT_DIR="$PROJECT_DIR/dist"
ISS_FILE="$PROJECT_DIR/jsling.iss"
MINGW_TOOLCHAIN="x86_64-w64-mingw32"

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; NC='\033[0m'

info()  { echo -e "${GREEN}[✓]${NC} $*"; }
warn()  { echo -e "${YELLOW}[!]${NC} $*"; }
error() { echo -e "${RED}[✗]${NC} $*" >&2; }

# ─── Step 1: Check dependencies ───────────────────────────────────────────────
check_deps() {
    info "Checking dependencies..."
    local missing=0

    if ! command -v "${MINGW_TOOLCHAIN}-g++" &>/dev/null; then
        error "MinGW cross-compiler not found: ${MINGW_TOOLCHAIN}-g++"
        echo "  Arch:   sudo pacman -S mingw-w64-gcc"
        echo "  Ubuntu: sudo apt install mingw-w64"
        missing=1
    fi

    if ! command -v cmake &>/dev/null; then
        error "cmake not found"
        missing=1
    fi

    if [ "${1:-}" != "--exe-only" ]; then
        # Check for Inno Setup Compiler
        if command -v iscc &>/dev/null; then
            info "Found native Inno Setup Compiler (iscc)"
            ISCC="iscc"
        elif command -v wine &>/dev/null; then
            # Look for Inno Setup installed under Wine
            WINE_ISCC="$HOME/.wine/drive_c/Program Files (x86)/Inno Setup 6/ISCC.exe"
            if [ -f "$WINE_ISCC" ]; then
                info "Found Inno Setup via Wine"
                ISCC="wine \"$WINE_ISCC\""
            else
                warn "Inno Setup not found under Wine"
                echo "  Download: https://jrsoftware.org/isdl.php"
                echo "  Install via: wine InnoSetup-6.x.x.exe"
                echo "  Then re-run this script."
                missing=1
            fi
        else
            warn "Neither iscc nor wine found. Cannot build installer."
            echo "  Install wine and Inno Setup 6, or build on Windows directly."
            missing=1
        fi
    fi

    [ $missing -eq 0 ] || exit 1
}

# ─── Step 2: Cross-compile jsling.exe ─────────────────────────────────────────
build_exe() {
    info "Cross-compiling jsling.exe for Windows (x86_64)..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    cmake "$SRC_DIR" \
        -DCMAKE_SYSTEM_NAME=Windows \
        -DCMAKE_C_COMPILER="${MINGW_TOOLCHAIN}-gcc" \
        -DCMAKE_CXX_COMPILER="${MINGW_TOOLCHAIN}-g++" \
        -DCMAKE_RC_COMPILER="${MINGW_TOOLCHAIN}-windres" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_FIND_ROOT_PATH="/usr/${MINGW_TOOLCHAIN}" \
        -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
        -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
        -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY

    make -j"$(nproc)"
    
    if [ -f "$BUILD_DIR/jsling.exe" ]; then
        info "Build successful: $BUILD_DIR/jsling.exe"
        # Strip debug symbols for smaller binary
        "${MINGW_TOOLCHAIN}-strip" "$BUILD_DIR/jsling.exe" 2>/dev/null || true
        local size
        size=$(du -h "$BUILD_DIR/jsling.exe" | cut -f1)
        info "Binary size: $size (stripped)"
    else
        error "Build failed: jsling.exe not found"
        exit 1
    fi
}

# ─── Step 3: Build installer with Inno Setup ──────────────────────────────────
build_installer() {
    info "Building Windows installer..."
    mkdir -p "$OUTPUT_DIR"

    # Copy jsling.exe to project root for Inno Setup
    cp "$BUILD_DIR/jsling.exe" "$PROJECT_DIR/jsling.exe"

    # Run Inno Setup Compiler
    eval "$ISCC" "$ISS_FILE" -O"$OUTPUT_DIR"

    if [ -f "$OUTPUT_DIR/JSling-Setup.exe" ]; then
        local size
        size=$(du -h "$OUTPUT_DIR/JSling-Setup.exe" | cut -f1)
        info "Installer created: $OUTPUT_DIR/JSling-Setup.exe ($size)"
    else
        # Inno Setup outputs to the script directory by default
        if [ -f "$PROJECT_DIR/Output/JSling-Setup.exe" ]; then
            mv "$PROJECT_DIR/Output/JSling-Setup.exe" "$OUTPUT_DIR/"
            info "Installer created: $OUTPUT_DIR/JSling-Setup.exe"
        else
            warn "Installer not found in expected location. Check Inno Setup output."
        fi
    fi

    # Cleanup temp exe copy
    rm -f "$PROJECT_DIR/jsling.exe"
}

# ─── Main ─────────────────────────────────────────────────────────────────────
main() {
    echo ""
    echo "╔══════════════════════════════════════════════╗"
    echo "║   JSling Windows Installer Builder           ║"
    echo "╚══════════════════════════════════════════════╝"
    echo ""

    check_deps "${1:-}"
    build_exe

    if [ "${1:-}" = "--exe-only" ]; then
        info "Done. jsling.exe is at: $BUILD_DIR/jsling.exe"
        exit 0
    fi

    build_installer

    echo ""
    info "All done! Distribute: dist/JSling-Setup.exe"
    echo ""
}

main "$@"
