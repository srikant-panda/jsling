@echo off
REM =============================================================================
REM jsling build script for Windows (MinGW or any GCC toolchain)
REM Usage: scripts\build.bat [Release|Debug]
REM =============================================================================
setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "PROJECT_DIR=%SCRIPT_DIR%.."
set "BUILD_DIR=%PROJECT_DIR%\build"

REM Default to Release build
set "BUILD_TYPE=Release"
if not "%~1"=="" set "BUILD_TYPE=%~1"

echo [info] Building jsling (%BUILD_TYPE%)...

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM --- Auto-detect available build tool ---
set "CMAKE_GEN="
set "MAKE_CMD="

where mingw32-make >nul 2>&1
if %ERRORLEVEL% equ 0 (
    set "CMAKE_GEN=-G MinGW Makefiles"
    set "MAKE_CMD=mingw32-make"
    goto :configure
)

where make >nul 2>&1
if %ERRORLEVEL% equ 0 (
    set "CMAKE_GEN=-G MinGW Makefiles"
    set "MAKE_CMD=make"
    goto :configure
)

where gcc >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo [info] gcc found but no make tool detected, using cmake --build
    goto :configure
)

echo [error] No C++ compiler found. Install MinGW-w64 and add it to your PATH.
exit /b 1

:configure
echo [info] Generator: %CMAKE_GEN%   Build tool: %MAKE_CMD%

REM --- Configure ---
if defined CMAKE_GEN (
    cmake %CMAKE_GEN% -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -S "%PROJECT_DIR%" -B "%BUILD_DIR%"
) else (
    cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -S "%PROJECT_DIR%" -B "%BUILD_DIR%"
)

if %ERRORLEVEL% neq 0 (
    echo.
    echo [error] CMake configure failed.
    echo         Make sure a C++ compiler ^(gcc/g++^) is in your PATH.
    exit /b 1
)

REM --- Build ---
if defined MAKE_CMD (
    %MAKE_CMD% -C "%BUILD_DIR%" -j%NUMBER_OF_PROCESSORS%
) else (
    cmake --build "%BUILD_DIR%" -j%NUMBER_OF_PROCESSORS%
)

if %ERRORLEVEL% neq 0 (
    echo [error] Build failed.
    pause
    exit /b 1
)

echo.
echo [done] Build complete: build\jsling.exe
pause
exit /b 0
