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
    goto :found
)
if exist "%ROOT%COMPILER_CPP\build\jsling.exe" (
    set "BINARY=%ROOT%COMPILER_CPP\build\jsling.exe"
    goto :found
)

echo.
echo  [!] jsling.exe not found.
echo.
echo      Expected locations:
echo        bin\jsling.exe                   (pre-built, shipped with repo)
echo        COMPILER_CPP\build\jsling.exe    (built from source)
echo.
echo      To build from source: scripts\build.bat  (requires MinGW + GCC 7+)
echo.
pause
exit /b 1

:found
title jsling - JavaScript Runtime

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

if "%CHOICE%"=="1" goto :demo
if "%CHOICE%"=="2" goto :hackathon
if "%CHOICE%"=="3" goto :fulltest
if "%CHOICE%"=="4" goto :repl
if "%CHOICE%"=="5" goto :eval
goto :invalid

:demo
echo.
echo  --- Running demo.js ---
echo.
if not exist "%ROOT%demo.js" (
    echo let name = "Hackathon";> "%ROOT%demo.js"
    echo console.log("Hello, " + name + "!");>> "%ROOT%demo.js"
    echo let double = x =^> x * 2;>> "%ROOT%demo.js"
    echo console.log("double(21) = " + double(21));>> "%ROOT%demo.js"
    echo let arr = [1, 2, 3, 4, 5];>> "%ROOT%demo.js"
    echo console.log("Reversed: " + [...arr].reverse().join(", "));>> "%ROOT%demo.js"
)
"%BINARY%" "%ROOT%demo.js"
echo.
pause
goto :done

:hackathon
echo.
call "%ROOT%COMPILER_CPP\scripts\run-hackathon-testcase.bat"
echo.
pause
goto :done

:fulltest
echo.
call "%ROOT%COMPILER_CPP\scripts\run-tests.bat"
echo.
pause
goto :done

:repl
echo.
"%BINARY%"
goto :done

:eval
echo.
set /p EXPR="  Expression: "
echo.
"%BINARY%" -e "!EXPR!"
echo.
pause
goto :done

:invalid
echo.
echo  Invalid choice.
pause
goto :done

:done
