@echo off
REM =============================================================================
REM jsling automated test runner for MinGW / Windows (not Visual Studio)
REM Usage: scripts\run-tests.bat [--filter <pattern>]
REM =============================================================================
setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "PROJECT_DIR=%SCRIPT_DIR%.."
set "BUILD_DIR=%PROJECT_DIR%\build"
set "TEST_DIR=%PROJECT_DIR%\tests"

REM Look for binary: prefer bin\ (shipped), then build\ (dev)
if exist "%PROJECT_DIR%\..\bin\jsling.exe" (
    set "JSLING=%PROJECT_DIR%\..\bin\jsling.exe"
) else (
    set "JSLING=%BUILD_DIR%\jsling.exe"
)
set "FILTER="

REM --- Parse arguments ---
:parse_args
if "%~1"=="" goto :args_done
if /i "%~1"=="--filter" (set "FILTER=%~2" & shift & shift & goto :parse_args)
if /i "%~1"=="-f"       (set "FILTER=%~2" & shift & shift & goto :parse_args)
shift
goto :parse_args
:args_done

REM --- Build if needed (auto-detect toolchain) ---
if not exist "%JSLING%" (
    echo [info] Building jsling...
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

    if not exist "%JSLING%" (
        echo [error] Build failed. Make sure a C++ compiler is in your PATH.
        exit /b 1
    )
)

REM --- Counters ---
set /a TOTAL=0
set /a PASSED=0
set /a FAILED=0
set /a SKIPPED=0

REM --- Temp file ---
set "ACTUAL_FILE=%TEMP%\jsling_actual_%RANDOM%.tmp"

echo.
echo jsling test suite
echo ==================
echo.

REM --- Run all .js test files ---
for %%F in ("%TEST_DIR%\*.js") do call :run_test "%%~fF"

REM --- Summary ---
if exist "%ACTUAL_FILE%" del "%ACTUAL_FILE%" >nul 2>&1

echo.
echo Results:  %PASSED% passed, %FAILED% failed, %SKIPPED% skipped  (total: %TOTAL%)
echo.

if %FAILED% equ 0 (exit /b 0) else (exit /b 1)

REM =============================================================================
REM  Test runner subroutine
REM  %~1 = full path to .js test file
REM =============================================================================
:run_test
set "JS_FILE=%~1"
for %%I in ("%~1") do set "BASE_NAME=%%~nxI"
set "EXPECTED_FILE=%JS_FILE:.js=.expected%"
set "SKIP_FILE=%JS_FILE:.js=.skip%"

set /a TOTAL+=1

REM Apply filter
if defined FILTER (
    echo !BASE_NAME! | findstr /i /c:"%FILTER%" >nul
    if errorlevel 1 (
        set /a SKIPPED+=1
        goto :eof
    )
)

REM Skip if .skip file exists
if exist "%SKIP_FILE%" (
    set /p SKIP_REASON=<"%SKIP_FILE%"
    echo   SKIP  %BASE_NAME%  ^(!SKIP_REASON!^)
    set /a SKIPPED+=1
    goto :eof
)

REM Skip if no .expected file
if not exist "%EXPECTED_FILE%" (
    echo   SKIP  %BASE_NAME%  (no .expected file)
    set /a SKIPPED+=1
    goto :eof
)

REM Run jsling and capture stdout+stderr (strip CR for CRLF normalization)
"%JSLING%" "%JS_FILE%" 2>&1 | find /v "" > "%ACTUAL_FILE%"

REM --- Compare using arrays to handle ~ pattern lines ---
set /a EXP_N=0
for /f "delims=" %%L in ('findstr /N "^" "%EXPECTED_FILE%"') do (
    set "RAWLINE=%%L"
    set "STRIPPED=!RAWLINE:*:=!"
    set /a EXP_N+=1
    set "EXP_LINE_!EXP_N!=!STRIPPED!"
)

set /a ACT_N=0
for /f "delims=" %%L in ('findstr /N "^" "%ACTUAL_FILE%"') do (
    set "RAWLINE=%%L"
    set "STRIPPED=!RAWLINE:*:=!"
    set /a ACT_N+=1
    set "ACT_LINE_!ACT_N!=!STRIPPED!"
)

set "ALL_MATCH=true"

REM Line count must match
if !EXP_N! neq !ACT_N! (
    set "ALL_MATCH=false"
    goto :compare_done
)

REM Compare line by line
for /L %%i in (1,1,!EXP_N!) do (
    if "!ALL_MATCH!"=="true" (
        set "ELINE=!EXP_LINE_%%i!"
        set "ALINE=!ACT_LINE_%%i!"
        if "!ELINE:~0,1!"=="~" (
            REM Pattern match: strip leading ~ and check substring containment
            set "PATTERN=!ELINE:~1!"
            echo !ALINE! | findstr /C:"!PATTERN!" >nul 2>&1
            if errorlevel 1 set "ALL_MATCH=false"
        ) else (
            if "!ALINE!" neq "!ELINE!" set "ALL_MATCH=false"
        )
    )
)
:compare_done

REM Clean up arrays to avoid leaking into next test
for /L %%i in (1,1,!EXP_N!) do set "EXP_LINE_%%i="
for /L %%i in (1,1,!ACT_N!) do set "ACT_LINE_%%i="

if "!ALL_MATCH!"=="true" (
    echo   PASS  %BASE_NAME%
    set /a PASSED+=1
) else (
    echo   FAIL  %BASE_NAME%
    set /a FAILED+=1
    echo.
    echo   --- Expected ---
    type "%EXPECTED_FILE%"
    echo.
    echo   --- Actual ---
    type "%ACTUAL_FILE%"
    echo.
)

goto :eof
