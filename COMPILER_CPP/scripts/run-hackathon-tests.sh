#!/usr/bin/env bash
# run-hackathon-tests.sh
# Runs the 5 hackathon evaluation test cases with verbose output.
# Usage: bash scripts/run-hackathon-tests.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
TEST_DIR="$PROJECT_DIR/tests/hackathon"
BINARY="$BUILD_DIR/jsling"

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; CYAN='\033[0;36m'; BOLD='\033[1m'; NC='\033[0m'

PASS=0; FAIL=0; TOTAL=5

# ─── Build if needed ──────────────────────────────────────────────────────────
if [ ! -f "$BINARY" ]; then
    echo -e "${YELLOW}[!]${NC} Binary not found, building..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake "$PROJECT_DIR" -DCMAKE_BUILD_TYPE=Release >/dev/null 2>&1
    make -j"$(nproc)" >/dev/null 2>&1
    echo -e "${GREEN}[✓]${NC} Build complete"
    cd "$PROJECT_DIR"
fi

echo ""
echo -e "${BOLD}${CYAN}╔════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BOLD}${CYAN}║          JSling Hackathon Test Suite (5 Cases)            ║${NC}"
echo -e "${BOLD}${CYAN}╚════════════════════════════════════════════════════════════╝${NC}"
echo ""
echo -e "Binary:  ${BOLD}$BINARY${NC}"
echo -e "Tests:   ${BOLD}$TEST_DIR${NC}"
echo ""

# ─── Test definitions ─────────────────────────────────────────────────────────
declare -a NAMES=(
    "TC-1: Odd / Even Checker"
    "TC-2: Triangle Pattern (For Loop)"
    "TC-3: Armstrong Number"
    "TC-4: Array Reverse (Spread)"
    "TC-5: String Palindrome Check"
)

declare -a FILES=(tc1_odd_even tc2_triangle tc3_armstrong tc4_array_reverse tc5_palindrome)
declare -a POINTS=(20 20 20 20 20)

# ─── Run each test ────────────────────────────────────────────────────────────
for i in 0 1 2 3 4; do
    JS_FILE="$TEST_DIR/${FILES[$i]}.js"
    EXPECTED_FILE="$TEST_DIR/${FILES[$i]}.expected"
    NAME="${NAMES[$i]}"
    PTS="${POINTS[$i]}"

    echo -e "${BOLD}┌─ ${NAME} (${PTS} pts) ─────────────────────────────────${NC}"

    if [ ! -f "$JS_FILE" ]; then
        echo -e "│ ${RED}✗ JS file not found: $JS_FILE${NC}"
        echo -e "└─ ${RED}MISSING${NC}"
        FAIL=$((FAIL + 1))
        echo ""
        continue
    fi

    # Show source code
    echo -e "│ ${CYAN}Source:${NC}"
    while IFS= read -r line; do
        echo -e "│   $line"
    done < "$JS_FILE"
    echo -e "│"

    # Run the test
    ACTUAL=$("$BINARY" "$JS_FILE" 2>&1) || true
    EXIT_CODE=$?

    echo -e "│ ${CYAN}Output:${NC}"
    echo "$ACTUAL" | while IFS= read -r line; do
        echo -e "│   $line"
    done
    echo -e "│"

    # Compare with expected
    if [ -f "$EXPECTED_FILE" ]; then
        EXPECTED=$(cat "$EXPECTED_FILE" | sed '/^$/d')
        ACTUAL_CLEAN=$(echo "$ACTUAL" | sed '/^$/d')

        if [ "$ACTUAL_CLEAN" = "$EXPECTED" ]; then
            echo -e "│ ${CYAN}Expected:${NC}"
            echo "$EXPECTED" | while IFS= read -r line; do
                echo -e "│   $line"
            done
            echo -e "│"
            echo -e "└─ ${GREEN}${BOLD}✓ PASS  (+${PTS} pts)${NC}"
            PASS=$((PASS + 1))
        else
            echo -e "│ ${YELLOW}Expected:${NC}"
            echo "$EXPECTED" | while IFS= read -r line; do
                echo -e "│   $line"
            done
            echo -e "│"
            echo -e "│ ${RED}Actual output does not match expected.${NC}"
            echo -e "└─ ${RED}${BOLD}✗ FAIL${NC}"
            FAIL=$((FAIL + 1))
        fi
    else
        echo -e "│ ${YELLOW}No .expected file found — showing output only${NC}"
        echo -e "└─ ${YELLOW}SKIPPED${NC}"
    fi
    echo ""
done

# ─── Summary ──────────────────────────────────────────────────────────────────
SCORE=$((PASS * 20))
echo -e "${BOLD}${CYAN}╔════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BOLD}${CYAN}║                     RESULTS SUMMARY                       ║${NC}"
echo -e "${BOLD}${CYAN}╠════════════════════════════════════════════════════════════╣${NC}"
echo -e "${BOLD}${CYAN}║${NC}  Passed:  ${GREEN}${BOLD}${PASS}${NC} / ${TOTAL}                                       ${BOLD}${CYAN}║${NC}"
echo -e "${BOLD}${CYAN}║${NC}  Failed:  ${RED}${BOLD}${FAIL}${NC} / ${TOTAL}                                       ${BOLD}${CYAN}║${NC}"
echo -e "${BOLD}${CYAN}║${NC}  Score:   ${BOLD}${SCORE}${NC} / 100                                    ${BOLD}${CYAN}║${NC}"
echo -e "${BOLD}${CYAN}╚════════════════════════════════════════════════════════════╝${NC}"
echo ""

if [ $FAIL -eq 0 ]; then
    echo -e "${GREEN}${BOLD}All test cases passed!${NC}"
else
    echo -e "${RED}${BOLD}${FAIL} test case(s) failed.${NC}"
fi
echo ""

exit $FAIL
