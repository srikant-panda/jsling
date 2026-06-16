#!/usr/bin/env bash
# run-hackathon-testcase.sh
# Runs the 5 hackathon evaluation test cases with verbose output.
# Usage: bash scripts/run-hackathon-testcase.sh
#
# Each test case is worth 20 points (total 100).
# Shows source code, actual output, expected output, and pass/fail status.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
TEST_DIR="$PROJECT_DIR/tests/hackathon_testcase"
BINARY="$BUILD_DIR/jsling"

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; CYAN='\033[0;36m'; BOLD='\033[1m'; NC='\033[0m'

PASS=0; FAIL=0; TOTAL=5
SCORE=0

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
echo -e "${BOLD}${CYAN}╔══════════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BOLD}${CYAN}║           JSling Hackathon Evaluation Test Suite                ║${NC}"
echo -e "${BOLD}${CYAN}║              5 Test Cases  ×  20 Points  =  100 Points          ║${NC}"
echo -e "${BOLD}${CYAN}╚══════════════════════════════════════════════════════════════════╝${NC}"
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

declare -a CONCEPTS=(
    "if/else, modulo, string concatenation"
    "Nested for loops, string building"
    "while loop, Math.floor, ** operator, functions"
    "Spread [...arr], .reverse(), .join()"
    ".split(\"\"), .reverse(), .join(\"\"), strict equality"
)

declare -a FILES=(tc1_odd_even tc2_triangle tc3_armstrong tc4_array_reverse tc5_palindrome)
declare -a POINTS=(20 20 20 20 20)

# ─── Run each test ────────────────────────────────────────────────────────────
for i in 0 1 2 3 4; do
    JS_FILE="$TEST_DIR/${FILES[$i]}.js"
    EXPECTED_FILE="$TEST_DIR/${FILES[$i]}.expected"
    NAME="${NAMES[$i]}"
    CONCEPT="${CONCEPTS[$i]}"
    PTS="${POINTS[$i]}"

    echo -e "${BOLD}┌─ ${NAME} (${PTS} pts) ─────────────────────────────────────${NC}"
    echo -e "│ ${CYAN}Concept:${NC} ${CONCEPT}"

    if [ ! -f "$JS_FILE" ]; then
        echo -e "│ ${RED}✗ JS file not found: $JS_FILE${NC}"
        echo -e "└─ ${RED}MISSING${NC}"
        FAIL=$((FAIL + 1))
        echo ""
        continue
    fi

    # Show source code
    echo -e "│"
    echo -e "│ ${CYAN}Source Code:${NC}"
    while IFS= read -r line; do
        echo -e "│   ${NC}$line"
    done < "$JS_FILE"
    echo -e "│"

    # Run the test
    ACTUAL=$("$BINARY" "$JS_FILE" 2>&1) || true

    echo -e "│ ${GREEN}Actual Output:${NC}"
    echo "$ACTUAL" | while IFS= read -r line; do
        echo -e "│   $line"
    done
    echo -e "│"

    # Compare with expected
    if [ -f "$EXPECTED_FILE" ]; then
        EXPECTED=$(cat "$EXPECTED_FILE" | sed '/^$/d')
        ACTUAL_CLEAN=$(echo "$ACTUAL" | sed '/^$/d')

        echo -e "│ ${YELLOW}Expected Output:${NC}"
        echo "$EXPECTED" | while IFS= read -r line; do
            echo -e "│   $line"
        done
        echo -e "│"

        if [ "$ACTUAL_CLEAN" = "$EXPECTED" ]; then
            echo -e "└─ ${GREEN}${BOLD}✓ PASS  (+${PTS} pts)${NC}"
            PASS=$((PASS + 1))
            SCORE=$((SCORE + PTS))
        else
            echo -e "│ ${RED}⚠ Output does not match expected!${NC}"
            echo -e "└─ ${RED}${BOLD}✗ FAIL  (0 pts)${NC}"
            FAIL=$((FAIL + 1))
        fi
    else
        echo -e "│ ${YELLOW}No .expected file found — showing output only${NC}"
        echo -e "└─ ${YELLOW}SKIPPED${NC}"
    fi
    echo ""
done

# ─── Summary ──────────────────────────────────────────────────────────────────
echo -e "${BOLD}${CYAN}╔══════════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BOLD}${CYAN}║                       RESULTS SUMMARY                           ║${NC}"
echo -e "${BOLD}${CYAN}╠══════════════════════════════════════════════════════════════════╣${NC}"
echo -e "${BOLD}${CYAN}║${NC}  Passed:  ${GREEN}${BOLD}${PASS}${NC} / ${TOTAL}                                           ${BOLD}${CYAN}║${NC}"
echo -e "${BOLD}${CYAN}║${NC}  Failed:  ${RED}${BOLD}${FAIL}${NC} / ${TOTAL}                                           ${BOLD}${CYAN}║${NC}"
echo -e "${BOLD}${CYAN}║${NC}  Score:   ${BOLD}${SCORE}${NC} / 100 points                                   ${BOLD}${CYAN}║${NC}"
echo -e "${BOLD}${CYAN}╠══════════════════════════════════════════════════════════════════╣${NC}"

# Per-test breakdown
for i in 0 1 2 3 4; do
    JS_FILE="$TEST_DIR/${FILES[$i]}.js"
    EXPECTED_FILE="$TEST_DIR/${FILES[$i]}.expected"
    NAME="${NAMES[$i]}"

    if [ -f "$JS_FILE" ] && [ -f "$EXPECTED_FILE" ]; then
        ACTUAL=$("$BINARY" "$JS_FILE" 2>&1) || true
        EXPECTED=$(cat "$EXPECTED_FILE" | sed '/^$/d')
        ACTUAL_CLEAN=$(echo "$ACTUAL" | sed '/^$/d')
        if [ "$ACTUAL_CLEAN" = "$EXPECTED" ]; then
            echo -e "${BOLD}${CYAN}║${NC}  ${GREEN}✓${NC} ${NAME}                        ${GREEN}+20${NC}   ${BOLD}${CYAN}║${NC}"
        else
            echo -e "${BOLD}${CYAN}║${NC}  ${RED}✗${NC} ${NAME}                        ${RED} 0${NC}    ${BOLD}${CYAN}║${NC}"
        fi
    else
        echo -e "${BOLD}${CYAN}║${NC}  ${YELLOW}?${NC} ${NAME}                        ${YELLOW}SKIP${NC}  ${BOLD}${CYAN}║${NC}"
    fi
done

echo -e "${BOLD}${CYAN}╚══════════════════════════════════════════════════════════════════╝${NC}"
echo ""

if [ $FAIL -eq 0 ]; then
    echo -e "${GREEN}${BOLD}All 5 test cases passed! Score: 100/100${NC}"
else
    echo -e "${RED}${BOLD}${FAIL} test case(s) failed. Score: ${SCORE}/100${NC}"
fi
echo ""

exit $FAIL
