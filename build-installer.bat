@echo off
REM build-installer.bat
REM Builds jsling.exe and creates the Windows GUI installer
REM Run this from the project root directory on Windows
REM
REM Prerequisites (install these first):
REM   1. CMake:       https://cmake.org/download/
REM   2. Visual Studio Build Tools or MinGW (see below)
REM   3. Inno Setup:  https://jrsoftware.org/isdl.php
REM
REM Usage:
REM   build-installer.bat          Full build (exe + installer)
REM   build-installer.bat exe      Just build jsling.exe
REM   build-installer.bat setup    Just build installer (needs jsling.exe already built)

setlocal enabledelayedexpansion

set "PROJECT_DIR=%~dp0"
set "SRC_DIR=%PROJECT_DIR%COMPILER_CPP"
set "BUILD_DIR=%SRC_DIR%\build-windows"
set "DIST_DIR=%PROJECT_DIR%dist"
set "ISS_FILE=%PROJECT_DIR%jsling.iss"

echo.
echo ========================================
echo   JSling Windows Installer Builder
echo ========================================
echo.

REM ── Detect compiler ──────────────────────────────────────────────────────
set "COMPILER_FOUND=0"

REM Check for Visual Studio (MSVC)
where cl.exe >nul 2>&1
if %ERRORLEVEL%==0 (
    echo [OK] Found MSVC compiler
    set "COMPILER_FOUND=1"
    set "CMAKE_GENERATOR=Visual Studio 17 2022"
    goto :compiler_done
)

REM Check for MinGW g++
where g++.exe >nul 2>&1
if %ERRORLEVEL%==0 (
    echo [OK] Found MinGW g++ compiler
    set "COMPILER_FOUND=1"
    set "CMAKE_GENERATOR=MinGW Makefiles"
    goto :compiler_done
)

REM Try to find Visual Studio Build Tools automatically
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
    for /f "tokens=*" %%i in ('"%VSWHERE%" -latest -property installationPath 2^>nul') do (
        set "VS_PATH=%%i"
    )
    if defined VS_PATH (
        echo [OK] Found Visual Studio at: !VS_PATH!
        call "!VS_PATH!\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
        set "COMPILER_FOUND=1"
        set "CMAKE_GENERATOR=Visual Studio 17 2022"
        goto :compiler_done
    )
)

echo [ERROR] No C++ compiler found!
echo.
echo Install one of these:
echo   Option A - Visual Studio Build Tools (recommended):
echo     https://visualstudio.microsoft.com/visual-cpp-build-tools/
echo     Select "Desktop development with C++"
echo.
echo   Option B - MinGW-w64:
echo     https://github.com/niXman/mingw-builds-binaries/releases
echo     Extract and add bin\ to your PATH
echo.
exit /b 1

:compiler_done

REM ── Check CMake ──────────────────────────────────────────────────────────
where cmake.exe >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake not found!
    echo   Download: https://cmake.org/download/
    exit /b 1
)
echo [OK] Found CMake

REM ── Step 1: Build jsling.exe ─────────────────────────────────────────────
if /i "%~1"=="setup" goto :build_setup

echo.
echo [*] Building jsling.exe...
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

cmake "%SRC_DIR%" -G "%CMAKE_GENERATOR%" -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake configuration failed!
    exit /b 1
)

cmake --build . --config Release
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Build failed!
    exit /b 1
)

REM Find the built exe (MSVC puts it in Release\ subfolder)
if exist "%BUILD_DIR%\Release\jsling.exe" (
    set "EXE_PATH=%BUILD_DIR%\Release\jsling.exe"
) else if exist "%BUILD_DIR%\jsling.exe" (
    set "EXE_PATH=%BUILD_DIR%\jsling.exe"
) else (
    echo [ERROR] jsling.exe not found after build!
    exit /b 1
)

echo [OK] Built: !EXE_PATH!

REM Copy exe to project root for Inno Setup
copy /y "!EXE_PATH!" "%PROJECT_DIR%jsling.exe" >nul

if /i "%~1"=="exe" (
    echo.
    echo [DONE] jsling.exe is ready: !EXE_PATH!
    exit /b 0
)

:build_setup
REM ── Step 2: Build installer ──────────────────────────────────────────────
echo.
echo [*] Building installer with Inno Setup...

REM Find Inno Setup Compiler
set "ISCC="
if exist "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" (
    set "ISCC=C:\Program Files (x86)\Inno Setup 6\ISCC.exe"
)
if exist "C:\Program Files\Inno Setup 6\ISCC.exe" (
    set "ISCC=C:\Program Files\Inno Setup 6\ISCC.exe"
)
where iscc.exe >nul 2>&1
if %ERRORLEVEL%==0 set "ISCC=iscc.exe"

if not defined ISCC (
    echo [ERROR] Inno Setup 6 not found!
    echo   Download and install: https://jrsoftware.org/isdl.php
    echo   Then re-run this script.
    exit /b 1
)

echo [OK] Found Inno Setup: !ISCC!

REM Make sure jsling.exe is in project root
if not exist "%PROJECT_DIR%jsling.exe" (
    if defined EXE_PATH (
        copy /y "!EXE_PATH!" "%PROJECT_DIR%jsling.exe" >nul
    ) else (
        echo [ERROR] jsling.exe not found. Build it first: %~nx0 exe
        exit /b 1
    )
)

REM Create dist directory
if not exist "%DIST_DIR%" mkdir "%DIST_DIR%"

REM Run Inno Setup Compiler
"!ISCC!" "%ISS_FILE%"
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Inno Setup compilation failed!
    exit /b 1
)

REM Move output to dist
if exist "%PROJECT_DIR%dist\JSling-Setup.exe" (
    echo.
    echo ========================================
    echo   SUCCESS!
    echo ========================================
    echo   Installer: dist\JSling-Setup.exe
    echo.
    for %%F in ("%DIST_DIR%\JSling-Setup.exe") do echo   Size: %%~zF bytes
    echo.
) else (
    echo [WARN] Installer may be in a different location. Check Inno Setup output.
)

REM Cleanup
del /q "%PROJECT_DIR%jsling.exe" >nul 2>&1

echo [DONE]
exit /b 0
