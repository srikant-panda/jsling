@echo off
REM =============================================================================
REM jsling build script for MinGW (not Visual Studio)
REM Usage: scripts\build.bat [Release|Debug]
REM =============================================================================
setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "PROJECT_DIR=%SCRIPT_DIR%.."
set "BUILD_DIR=%PROJECT_DIR%\build"

REM Default to Release build
set "BUILD_TYPE=Release"
if not "%~1"=="" set "BUILD_TYPE=%~1"

echo [info] Building jsling (%BUILD_TYPE%) with MinGW...

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM Configure with MinGW Makefiles generator (not Visual Studio)
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -S "%PROJECT_DIR%" -B "%BUILD_DIR%"
if %ERRORLEVEL% neq 0 (
    echo [error] CMake configure failed. Make sure MinGW is in your PATH.
    exit /b 1
)

REM Build: prefer mingw32-make, fall back to make
where mingw32-make >nul 2>&1
if %ERRORLEVEL% equ 0 (
    mingw32-make -C "%BUILD_DIR%" -j%NUMBER_OF_PROCESSORS%
) else (
    where make >nul 2>&1
    if !ERRORLEVEL! equ 0 (
        make -C "%BUILD_DIR%" -j%NUMBER_OF_PROCESSORS%
    ) else (
        cmake --build "%BUILD_DIR%" -j%NUMBER_OF_PROCESSORS%
    )
)

if %ERRORLEVEL% neq 0 (
    echo [error] Build failed.
    exit /b 1
)

echo.
echo [done] Build complete: build\jsling.exe
exit /b 0
