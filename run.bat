@echo off
REM =============================================================================
REM  jsling - One-click launcher for Hackathon Judges
REM  No build tools, no MinGW, no Visual Studio needed.
REM  Just double-click this file.
REM =============================================================================
setlocal enabledelayedexpansion

set "ROOT=%~dp0"

REM Look for jsling.exe: prefer bin\ (shipped), then build\ (dev)
if exist "%ROOT%bin\jsling.exe" (
    set "BINARY=%ROOT%bin\jsling.exe"
) else if exist "%ROOT%COMPILER_CPP\build\jsling.exe" (
    set "BINARY=%ROOT%COMPILER_CPP\build\jsling.exe"
) else (
    set "BINARY="
)

title jsling - JavaScript Runtime

if not defined BINARY (
    echo.
    echo  [!] jsling.exe not found.
    echo.
    echo      Expected locations:
    echo        bin\jsling.exe                   ^(pre-built, shipped with repo^)
    echo        COMPILER_CPP\build\jsling.exe    ^(built from source^)
    echo.
    echo      To build from source: scripts\build.bat  ^(requires MinGW + GCC 7+^)
    echo.
    pause
    exit /b 1
)

echo.
echo  ========================================
echo   jsling - JavaScript Runtime (C++17)
echo   Thunder Hackathon 2.0
echo  ========================================
echo.
echo  1. Run demo script       (demo.js)
echo  2. Run hackathon tests   (5 test cases, 100 pts)
echo  3. Run full test suite   (all tests)
echo  4. Start interactive REPL
echo  5. Evaluate an expression
echo.

set /p CHOICE="  Enter choice (1-5): "

if "%CHOICE%"=="1" (
    echo.
    echo  --- Running demo.js ---
    echo.
    if exist "%ROOT%demo.js" (
        "%BINARY%" "%ROOT%demo.js"
    ) else (
        echo  demo.js not found. Creating one...
        echo.
        (
            echo let name = "Hackathon";
            echo console.log("Hello, " + name + "!");
            echo let double = x =^> x * 2;
            echo console.log("double(21) = " + double(21^)^);
            echo let arr = [1, 2, 3, 4, 5];
            echo console.log("Reversed: " + [...arr].reverse(^).join(", "^)^);
        ) > "%ROOT%demo.js"
        "%BINARY%" "%ROOT%demo.js"
    )
    echo.
    pause
) else if "%CHOICE%"=="2" (
    echo.
    call "%ROOT%COMPILER_CPP\scripts\run-hackathon-testcase.bat"
    echo.
    pause
) else if "%CHOICE%"=="3" (
    echo.
    call "%ROOT%COMPILER_CPP\scripts\run-tests.bat"
    echo.
    pause
) else if "%CHOICE%"=="4" (
    echo.
    "%BINARY%"
) else if "%CHOICE%"=="5" (
    echo.
    set /p EXPR="  Expression: "
    echo.
    "%BINARY%" -e "!EXPR!"
    echo.
    pause
) else (
    echo.
    echo  Invalid choice.
    pause
)
