@echo off
REM =============================================================================
REM jsling automated test runner for Windows
REM Usage: scripts\run-tests.bat [--filter <pattern>]
REM =============================================================================
setlocal enabledelayedexpansion

REM Capture CR character for CRLF stripping
for /f %%a in ('copy /Z "%~f0" nul') do set "CR=%%a"

REM --- ANSI colors (Windows 10+ Terminal) ---
for /F %%a in ('echo prompt $E ^| cmd') do set "ESC=%%a"
set "RED=%ESC%[0;31m"
set "GREEN=%ESC%[0;32m"
set "YELLOW=%ESC%[1;33m"
set "CYAN=%ESC%[0;36m"
set "BOLD=%ESC%[1m"
set "DIM=%ESC%[2m"
set "RESET=%ESC%[0m"

set "SCRIPT_DIR=%~dp0"
pushd "%SCRIPT_DIR%.."
set "PROJECT_DIR=%CD%"
popd
set "BUILD_DIR=%PROJECT_DIR%\build"
set "TEST_DIR=%PROJECT_DIR%\tests"

REM Resolve repo root for bin\ lookup
pushd "%PROJECT_DIR%\.."
set "REPO_ROOT=%CD%"
popd

REM Look for binary: prefer bin\ (shipped), then build\ (dev)
if exist "%REPO_ROOT%\bin\jsling.exe" (
    set "JSLING=%REPO_ROOT%\bin\jsling.exe"
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
    echo %CYAN%[info]%RESET% Building jsling...
    if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

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
        echo %RED%[error]%RESET% CMake configure failed. Install Visual Studio Build Tools or MinGW-w64.
        exit /b 1
    )

    if defined AUTO_MAKE (
        !AUTO_MAKE! -C "%BUILD_DIR%" -j%NUMBER_OF_PROCESSORS% >nul 2>&1
    ) else (
        cmake --build "%BUILD_DIR%" -j%NUMBER_OF_PROCESSORS% >nul 2>&1
    )

    if not exist "%JSLING%" (
        echo %RED%[error]%RESET% Build failed. Make sure a C++ compiler is in your PATH.
        exit /b 1
    )
    echo %GREEN%[ok]%RESET% Build complete
)

REM --- Counters ---
set /a TOTAL=0
set /a PASSED=0
set /a FAILED=0
set /a SKIPPED=0
set "FAIL_LIST="

REM --- Temp file ---
set "ACTUAL_FILE=%TEMP%\jsling_actual_%RANDOM%.tmp"

echo.
echo %BOLD%jsling test suite%RESET%
echo ==================
echo.
echo %DIM%Binary:  %JSLING%%RESET%
echo %DIM%Tests:   %TEST_DIR%%RESET%
if defined FILTER echo %DIM%Filter:  %FILTER%%RESET%
echo.

REM --- Run all .js test files ---
for %%F in ("%TEST_DIR%\*.js") do call :run_test "%%~fF"

REM --- Summary ---
if exist "%ACTUAL_FILE%" del "%ACTUAL_FILE%" >nul 2>&1

echo.
echo %BOLD%Results:%RESET%  %GREEN%%PASSED% passed%RESET%, %RED%%FAILED% failed%RESET%, %YELLOW%%SKIPPED% skipped%RESET%  (total: %TOTAL%)

REM Show failure details
if %FAILED% gtr 0 (
    echo.
    echo %BOLD%%RED%Failure details:%RESET%
    for %%N in (!FAIL_LIST!) do (
        echo.
        echo   %RED%%%%~nN%%RESET%
        echo   Expected:
        for /f "delims=" %%L in ('findstr /N "^" "%%~dNpN.expected"') do (
            set "LINE=%%L"
            set "LINE=!LINE:*:=!"
            set "LINE=!LINE:%CR%=!"
            echo     !LINE!
        )
        echo   Actual:
        "%JSLING%" "%%N" 2>&1 | find /v "" | findstr /N "^" | for /f "delims=" %%L in ('more') do (
            set "LINE=%%L"
            set "LINE=!LINE:*:=!"
            set "LINE=!LINE:%CR%=!"
            echo     !LINE!
        )
    )
    echo.
)

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
    echo   %YELLOW%SKIP%RESET%  %BASE_NAME%  %DIM%(!SKIP_REASON!)%RESET%
    set /a SKIPPED+=1
    goto :eof
)

REM Skip if no .expected file
if not exist "%EXPECTED_FILE%" (
    echo   %YELLOW%SKIP%RESET%  %BASE_NAME%  %DIM%(no .expected file)%RESET%
    set /a SKIPPED+=1
    goto :eof
)

REM Run jsling and capture stdout+stderr
"%JSLING%" "%JS_FILE%" 2>&1 | find /v "" > "%ACTUAL_FILE%"

REM --- Compare using arrays to handle ~ pattern lines ---
set /a EXP_N=0
for /f "delims=" %%L in ('findstr /N "^" "%EXPECTED_FILE%"') do (
    set "RAWLINE=%%L"
    set "STRIPPED=!RAWLINE:*:=!"
    set "STRIPPED=!STRIPPED:%CR%=!"
    set /a EXP_N+=1
    set "EXP_LINE_!EXP_N!=!STRIPPED!"
)

set /a ACT_N=0
for /f "delims=" %%L in ('findstr /N "^" "%ACTUAL_FILE%"') do (
    set "RAWLINE=%%L"
    set "STRIPPED=!RAWLINE:*:=!"
    set "STRIPPED=!STRIPPED:%CR%=!"
    set /a ACT_N+=1
    set "ACT_LINE_!ACT_N!=!STRIPPED!"
)

set "ALL_MATCH=true"
set "FIRST_DIFF="

REM Line count must match
if !EXP_N! neq !ACT_N! (
    set "ALL_MATCH=false"
    set "FIRST_DIFF=line count: expected !EXP_N!, got !ACT_N!"
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
            if errorlevel 1 (
                set "ALL_MATCH=false"
                set "FIRST_DIFF=line %%i: pattern '!PATTERN!' not found in '!ALINE!'"
            )
        ) else (
            if "!ALINE!" neq "!ELINE!" (
                set "ALL_MATCH=false"
                set "FIRST_DIFF=line %%i: expected '!ELINE!', got '!ALINE!'"
            )
        )
    )
)
:compare_done

REM Clean up arrays
for /L %%i in (1,1,!EXP_N!) do set "EXP_LINE_%%i="
for /L %%i in (1,1,!ACT_N!) do set "ACT_LINE_%%i="

if "!ALL_MATCH!"=="true" (
    echo   %GREEN%PASS%RESET%  %BASE_NAME%
    set /a PASSED+=1
) else (
    echo   %RED%FAIL%RESET%  %BASE_NAME%  %DIM%(!FIRST_DIFF!)%RESET%
    set /a FAILED+=1
    set "FAIL_LIST=!FAIL_LIST! "%JS_FILE%""
)

goto :eof
