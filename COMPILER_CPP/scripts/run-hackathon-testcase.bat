@echo off
REM =============================================================================
REM jsling Hackathon Evaluation Test Suite (Windows)
REM Usage: scripts\run-hackathon-testcase.bat
REM
REM Runs the 5 hackathon evaluation test cases with verbose output.
REM Each test case is worth of 20 points (total 100).
REM =============================================================================
setlocal enabledelayedexpansion

REM --- ANSI colors (Windows 10+ Terminal) ---
for /F "delims=#" %%E in ('"prompt #$E# & for %%a in (1) do rem"') do set "ESC=%%E"
set "RED=!ESC![0;31m"
set "GREEN=!ESC![0;32m"
set "YELLOW=!ESC![1;33m"
set "CYAN=!ESC![0;36m"
set "BOLD=!ESC![1m"
set "DIM=!ESC![2m"
set "RESET=!ESC![0m"

set "SCRIPT_DIR=%~dp0"
pushd "%SCRIPT_DIR%.."
set "PROJECT_DIR=%CD%"
popd
set "BUILD_DIR=%PROJECT_DIR%\build"
set "TEST_DIR=%PROJECT_DIR%\tests\hackathon_testcase"

REM Resolve repo root for bin\ lookup
pushd "%PROJECT_DIR%\.."
set "REPO_ROOT=%CD%"
popd

REM Look for binary: prefer bin\ (shipped), then build\ (dev)
if exist "%REPO_ROOT%\bin\jsling.exe" (
    set "BINARY=%REPO_ROOT%\bin\jsling.exe"
) else (
    set "BINARY=%BUILD_DIR%\jsling.exe"
)

set /a PASS=0
set /a FAIL=0
set /a SCORE=0

REM --- Test definitions ---
set "NAME1=TC-1: Odd / Even Checker"
set "NAME2=TC-2: Triangle Pattern (For Loop)"
set "NAME3=TC-3: Armstrong Number"
set "NAME4=TC-4: Array Reverse (Spread)"
set "NAME5=TC-5: String Palindrome Check"

set "CONCEPT1=if/else, modulo, string concatenation"
set "CONCEPT2=Nested for loops, string building"
set "CONCEPT3=while loop, Math.floor, ** operator, functions"
set "CONCEPT4=Spread [...arr], .reverse(), .join()"
set "CONCEPT5=.split, .reverse(), .join, strict equality"

set "FILE1=tc1_odd_even"
set "FILE2=tc2_triangle"
set "FILE3=tc3_armstrong"
set "FILE4=tc4_array_reverse"
set "FILE5=tc5_palindrome"

REM --- Build if needed ---
if not exist "%BINARY%" (
    echo %YELLOW%[!]%RESET% Binary not found, building...
    if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
    set "AUTO_GEN="
    set "AUTO_MAKE="
    where mingw32-make >nul 2>&1
    if !ERRORLEVEL! equ 0 (
        set "AUTO_GEN=-G MinGW Makefiles"
        set "AUTO_MAKE=mingw32-make"
    )
    if defined AUTO_GEN (
        cmake !AUTO_GEN! -DCMAKE_BUILD_TYPE=Release -S "%PROJECT_DIR%" -B "%BUILD_DIR%" >nul 2>&1
    ) else (
        cmake -DCMAKE_BUILD_TYPE=Release -S "%PROJECT_DIR%" -B "%BUILD_DIR%" >nul 2>&1
    )
    if !ERRORLEVEL! neq 0 (
        echo %RED%[error]%RESET% CMake configure failed.
        exit /b 1
    )
    if defined AUTO_MAKE (
        !AUTO_MAKE! -C "%BUILD_DIR%" -j%NUMBER_OF_PROCESSORS% >nul 2>&1
    ) else (
        cmake --build "%BUILD_DIR%" -j%NUMBER_OF_PROCESSORS% >nul 2>&1
    )
    if not exist "%BINARY%" (
        echo %RED%[error]%RESET% Build failed.
        exit /b 1
    )
    echo %GREEN%[ok]%RESET% Build complete
)

echo.
echo %BOLD%%CYAN%======================================================================%RESET%
echo %BOLD%%CYAN%           JSling Hackathon Evaluation Test Suite%RESET%
echo %BOLD%%CYAN%              5 Test Cases  x  20 Points  =  100 Points%RESET%
echo %BOLD%%CYAN%======================================================================%RESET%
echo.
echo Binary:  %BOLD%%BINARY%%RESET%
echo Tests:   %BOLD%%TEST_DIR%%RESET%
echo.

REM --- Temp files for comparison ---
set "ACTUAL_TMP=%TEMP%\jsling_hack_actual_%RANDOM%.tmp"
set "EXPECTED_TMP=%TEMP%\jsling_hack_expected_%RANDOM%.tmp"

REM --- Run each test via subroutine ---
for /L %%i in (1,1,5) do call :run_hack_test %%i

REM --- Cleanup temp files ---
if exist "%ACTUAL_TMP%" del "%ACTUAL_TMP%" >nul 2>&1
if exist "%EXPECTED_TMP%" del "%EXPECTED_TMP%" >nul 2>&1

REM --- Summary ---
echo %BOLD%%CYAN%======================================================================%RESET%
echo %BOLD%%CYAN%                       RESULTS SUMMARY%RESET%
echo %BOLD%%CYAN%======================================================================%RESET%
echo   Passed:  %GREEN%%BOLD%%PASS%%RESET% / 5
echo   Failed:  %RED%%BOLD%%FAIL%%RESET% / 5
echo   Score:   %BOLD%%SCORE%%RESET% / 100 points
echo %BOLD%%CYAN%----------------------------------------------------------------------%RESET%

REM Per-test breakdown
for /L %%i in (1,1,5) do call :summary_line %%i

echo %BOLD%%CYAN%======================================================================%RESET%
echo.

if %FAIL% equ 0 (
    echo %GREEN%%BOLD%All 5 test cases passed! Score: 100/100%RESET%
) else (
    echo %RED%%BOLD%%FAIL% test case^(s^) failed. Score: %SCORE%/100%RESET%
)
echo.
exit /b %FAIL%

REM =============================================================================
REM  Subroutine: run a single hackathon test
REM  %1 = test index (1-5)
REM =============================================================================
:run_hack_test
set "IDX=%1"
set "JS_FILE=%TEST_DIR%\!FILE%IDX%!.js"
set "EXP_FILE=%TEST_DIR%\!FILE%IDX%!.expected"

echo %BOLD%-- !NAME%IDX%! [20 pts] ------------------------------------------%RESET%
echo %CYAN%Concept:%RESET% !CONCEPT%IDX%!
echo ----------------------------------------------------------------------

if not exist "!JS_FILE!" (
    echo   %RED%MISSING%RESET% JS file not found: !JS_FILE!
    set /a FAIL+=1
    echo.
    goto :eof
)

echo.
echo   %CYAN%Source Code:%RESET%
for /f "delims=" %%L in ('findstr /N "^" "!JS_FILE!"') do (
    set "SRCLINE=%%L"
    echo     !SRCLINE:*:=!
)
echo.

REM Run jsling and capture output
"%BINARY%" "!JS_FILE!" > "!ACTUAL_TMP!" 2>&1

echo   %GREEN%Actual Output:%RESET%
for /f "delims=" %%L in ('findstr /N "^" "!ACTUAL_TMP!"') do (
    set "OUTLINE=%%L"
    echo     !OUTLINE:*:=!
)
echo.

if not exist "!EXP_FILE!" (
    echo   %YELLOW%No .expected file found -- showing output only%RESET%
    echo.
    goto :eof
)

echo   %YELLOW%Expected Output:%RESET%
for /f "delims=" %%L in ('findstr /N "^" "!EXP_FILE!"') do (
    set "EXPLINE=%%L"
    echo     !EXPLINE:*:=!
)
echo.

REM Compare: strip blank lines from both then use fc
findstr /R /V "^$" "!ACTUAL_TMP!" > "!ACTUAL_TMP!.clean" 2>nul
findstr /R /V "^$" "!EXP_FILE!" > "!EXPECTED_TMP!.clean" 2>nul

fc /B "!ACTUAL_TMP!.clean" "!EXPECTED_TMP!.clean" >nul 2>&1
if !ERRORLEVEL! equ 0 (
    echo   %GREEN%%BOLD%PASS  +20 pts%RESET%
    set /a PASS+=1
    set /a SCORE+=20
) else (
    echo   %RED%%BOLD%FAIL  [0 pts]%RESET%  Output does not match expected
    set /a FAIL+=1
)

if exist "!ACTUAL_TMP!.clean" del "!ACTUAL_TMP!.clean" >nul 2>&1
if exist "!EXPECTED_TMP!.clean" del "!EXPECTED_TMP!.clean" >nul 2>&1
echo.
goto :eof

REM =============================================================================
REM  Subroutine: summary breakdown line
REM  %1 = test index (1-5)
REM =============================================================================
:summary_line
set "IDX=%1"
set "JS_FILE=%TEST_DIR%\!FILE%IDX%!.js"
set "EXP_FILE=%TEST_DIR%\!FILE%IDX%!.expected"

if not exist "!JS_FILE!" (
    echo   %YELLOW%SKIP%RESET% !NAME%IDX%!
    goto :eof
)
if not exist "!EXP_FILE!" (
    echo   %YELLOW%SKIP%RESET% !NAME%IDX%!
    goto :eof
)

"%BINARY%" "!JS_FILE!" > "!ACTUAL_TMP!" 2>&1
findstr /R /V "^$" "!ACTUAL_TMP!" > "!ACTUAL_TMP!.clean" 2>nul
findstr /R /V "^$" "!EXP_FILE!" > "!EXPECTED_TMP!.clean" 2>nul
fc /B "!ACTUAL_TMP!.clean" "!EXPECTED_TMP!.clean" >nul 2>&1
if !ERRORLEVEL! equ 0 (
    echo   %GREEN%PASS%RESET% !NAME%IDX%!                         +20
) else (
    echo   %RED%FAIL%RESET% !NAME%IDX%!                           0
)
if exist "!ACTUAL_TMP!" del "!ACTUAL_TMP!" >nul 2>&1
if exist "!ACTUAL_TMP!.clean" del "!ACTUAL_TMP!.clean" >nul 2>&1
if exist "!EXPECTED_TMP!.clean" del "!EXPECTED_TMP!.clean" >nul 2>&1
goto :eof
