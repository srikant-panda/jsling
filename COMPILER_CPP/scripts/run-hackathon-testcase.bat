@echo off
REM =============================================================================
REM jsling Hackathon Evaluation Test Suite (MinGW / Windows)
REM Usage: scripts\run-hackathon-testcase.bat
REM
REM Runs the 5 hackathon evaluation test cases with verbose output.
REM Each test case is worth 20 points (total 100).
REM =============================================================================
setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "PROJECT_DIR=%SCRIPT_DIR%.."
set "BUILD_DIR=%PROJECT_DIR%\build"
set "TEST_DIR=%PROJECT_DIR%\tests\hackathon_testcase"

REM Look for binary: prefer bin\ (shipped), then build\ (dev)
if exist "%PROJECT_DIR%\..\bin\jsling.exe" (
    set "BINARY=%PROJECT_DIR%\..\bin\jsling.exe"
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
set "CONCEPT5=.split(""), .reverse(), .join(""), strict equality"

set "FILE1=tc1_odd_even"
set "FILE2=tc2_triangle"
set "FILE3=tc3_armstrong"
set "FILE4=tc4_array_reverse"
set "FILE5=tc5_palindrome"

REM --- Build if needed (auto-detect toolchain) ---
if not exist "%BINARY%" (
    echo [!] Binary not found, building...
    if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

    REM Detect available make tool
    set "AUTO_GEN="
    set "AUTO_MAKE="
    where mingw32-make >nul 2>&1
    if !ERRORLEVEL! equ 0 (
        set "AUTO_GEN=-G MinGW Makefiles"
        set "AUTO_MAKE=mingw32-make"
    ) else (
        where make >nul 2>&1
        if !ERRORLEVEL! equ 0 (
            set "AUTO_GEN=-G MinGW Makefiles"
            set "AUTO_MAKE=make"
        )
    )

    if defined AUTO_GEN (
        cmake !AUTO_GEN! -DCMAKE_BUILD_TYPE=Release -S "%PROJECT_DIR%" -B "%BUILD_DIR%" >nul 2>&1
    ) else (
        cmake -DCMAKE_BUILD_TYPE=Release -S "%PROJECT_DIR%" -B "%BUILD_DIR%" >nul 2>&1
    )
    if !ERRORLEVEL! neq 0 (
        echo [error] CMake configure failed. Make sure gcc/g++ is in your PATH.
        exit /b 1
    )

    if defined AUTO_MAKE (
        !AUTO_MAKE! -C "%BUILD_DIR%" -j%NUMBER_OF_PROCESSORS% >nul 2>&1
    ) else (
        cmake --build "%BUILD_DIR%" -j%NUMBER_OF_PROCESSORS% >nul 2>&1
    )

    if not exist "%BINARY%" (
        echo [error] Build failed. Make sure a C++ compiler is in your PATH.
        exit /b 1
    )
    echo [ok] Build complete
)

echo.
echo ======================================================================
echo          JSling Hackathon Evaluation Test Suite
echo          5 Test Cases  x  20 Points  =  100 Points
echo ======================================================================
echo.
echo Binary:  %BINARY%
echo Tests:   %TEST_DIR%
echo.

REM --- Temp files for comparison ---
set "ACTUAL_TMP=%TEMP%\jsling_hack_actual_%RANDOM%.tmp"
set "EXPECTED_TMP=%TEMP%\jsling_hack_expected_%RANDOM%.tmp"

REM --- Run each test ---
for /L %%i in (1,1,5) do (
    set "JS_FILE=%TEST_DIR%\!FILE%%i!.js"
    set "EXP_FILE=%TEST_DIR%\!FILE%%i!.expected"

    echo ----------------------------------------------------------------------
    echo   !NAME%%i!  ^(20 pts^)
    echo   Concept: !CONCEPT%%i!
    echo ----------------------------------------------------------------------

    if not exist "!JS_FILE!" (
        echo   [MISSING] JS file not found: !JS_FILE!
        set /a FAIL+=1
        echo.
    ) else (
        echo.
        echo   Source Code:
        for /f "delims=" %%L in ('findstr /N "^" "!JS_FILE!"') do (
            set "SRCLINE=%%L"
            echo     !SRCLINE:*:=!
        )
        echo.

        REM Run jsling and capture output (strip CR)
        "%BINARY%" "!JS_FILE!" 2>&1 | find /v "" > "%ACTUAL_TMP%"

        echo   Actual Output:
        for /f "delims=" %%L in ('findstr /N "^" "%ACTUAL_TMP%"') do (
            set "OUTLINE=%%L"
            echo     !OUTLINE:*:=!
        )
        echo.

        if exist "!EXP_FILE!" (
            echo   Expected Output:
            for /f "delims=" %%L in ('findstr /N "^" "!EXP_FILE!"') do (
                set "EXPLINE=%%L"
                echo     !EXPLINE:*:=!
            )
            echo.

            REM Compare: strip blank lines from both files then use fc
            findstr /R /V "^$" "%ACTUAL_TMP%" > "%ACTUAL_TMP%.clean" 2>nul
            findstr /R /V "^$" "!EXP_FILE!" > "%EXPECTED_TMP%.clean" 2>nul

            fc /B "%ACTUAL_TMP%.clean" "%EXPECTED_TMP%.clean" >nul 2>&1
            if !ERRORLEVEL! equ 0 (
                echo   [PASS] +20 pts
                set /a PASS+=1
                set /a SCORE+=20
            ) else (
                echo   [FAIL] Output does not match expected!  (0 pts)
                set /a FAIL+=1
            )

            if exist "%ACTUAL_TMP%.clean" del "%ACTUAL_TMP%.clean" >nul 2>&1
            if exist "%EXPECTED_TMP%.clean" del "%EXPECTED_TMP%.clean" >nul 2>&1
        ) else (
            echo   [SKIP] No .expected file found
        )
        echo.
    )
)

REM --- Cleanup temp files ---
if exist "%ACTUAL_TMP%" del "%ACTUAL_TMP%" >nul 2>&1
if exist "%EXPECTED_TMP%" del "%EXPECTED_TMP%" >nul 2>&1

REM --- Summary ---
echo ======================================================================
echo                        RESULTS SUMMARY
echo ======================================================================
echo   Passed:  %PASS% / 5
echo   Failed:  %FAIL% / 5
echo   Score:   %SCORE% / 100 points
echo ======================================================================

REM Per-test breakdown
for /L %%i in (1,1,5) do (
    set "JS_FILE=%TEST_DIR%\!FILE%%i!.js"
    set "EXP_FILE=%TEST_DIR%\!FILE%%i!.expected"

    if exist "!JS_FILE!" if exist "!EXP_FILE!" (
        "%BINARY%" "!JS_FILE!" 2>&1 | find /v "" > "%ACTUAL_TMP%"
        findstr /R /V "^$" "%ACTUAL_TMP%" > "%ACTUAL_TMP%.clean" 2>nul
        findstr /R /V "^$" "!EXP_FILE!" > "%EXPECTED_TMP%.clean" 2>nul
        fc /B "%ACTUAL_TMP%.clean" "%EXPECTED_TMP%.clean" >nul 2>&1
        if !ERRORLEVEL! equ 0 (
            echo   [PASS] !NAME%%i!                         +20
        ) else (
            echo   [FAIL] !NAME%%i!                          0
        )
        if exist "%ACTUAL_TMP%" del "%ACTUAL_TMP%" >nul 2>&1
        if exist "%ACTUAL_TMP%.clean" del "%ACTUAL_TMP%.clean" >nul 2>&1
        if exist "%EXPECTED_TMP%.clean" del "%EXPECTED_TMP%.clean" >nul 2>&1
    ) else (
        echo   [SKIP] !NAME%%i!
    )
)

echo ======================================================================
echo.

if %FAIL% equ 0 (
    echo All 5 test cases passed! Score: 100/100
) else (
    echo %FAIL% test case(s) failed. Score: %SCORE%/100
)
echo.

exit /b %FAIL%
