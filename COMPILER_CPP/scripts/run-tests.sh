#!/usr/bin/env bash
# =============================================================================
# jsling automated test runner
# Usage: bash scripts/run-tests.sh [--verbose] [--filter <pattern>]
# =============================================================================
set -euo pipefail

# --- Colors ---
if [ -t 1 ]; then
    RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
    BLUE='\033[0;34m'; BOLD='\033[1m'; DIM='\033[2m'; RESET='\033[0m'
else
    RED='' GREEN='' YELLOW='' BLUE='' BOLD='' DIM='' RESET=''
fi

# --- Configuration ---
VERBOSE=false
FILTER=""
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
TEST_DIR="$PROJECT_DIR/tests"
JSLING="$BUILD_DIR/jsling"

# --- Parse arguments ---
while [ $# -gt 0 ]; do
    case "$1" in
        --verbose|-v)  VERBOSE=true; shift;;
        --filter|-f)   FILTER="$2"; shift 2;;
        --help|-h)
            echo "Usage: bash scripts/run-tests.sh [--verbose] [--filter <pattern>]"
            exit 0
            ;;
        *) echo "Unknown: $1"; exit 1;;
    esac
done

# --- Build if needed ---
if [ ! -f "$JSLING" ]; then
    echo -e "${BLUE}info${RESET}  Building jsling..."
    mkdir -p "$BUILD_DIR"
    cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Debug > /dev/null 2>&1
    cmake --build "$BUILD_DIR" -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)" > /dev/null 2>&1 || {
        echo -e "${RED}error${RESET} Build failed"
        exit 1
    }
fi

# --- Counters ---
TOTAL=0
PASSED=0
FAILED=0
SKIPPED=0

# --- Test result tracking ---
FAIL_DETAILS=()

# --- Diff helper ---
run_test() {
    local js_file="$1"
    local base_name ext expected_file skip_file
    base_name="$(basename "$js_file")"
    ext="${base_name##*.}"
    expected_file="${js_file%.js}.expected"
    skip_file="${js_file%.js}.skip"

    TOTAL=$((TOTAL + 1))
    local display_name="$base_name"

    # Skip filter
    if [ -n "$FILTER" ]; then
        if ! echo "$base_name" | grep -qiF "$FILTER"; then
            SKIPPED=$((SKIPPED + 1))
            return
        fi
    fi

    # Skip if marked
    if [ -f "$skip_file" ]; then
        local reason
        reason="$(cat "$skip_file")"
        printf "  ${YELLOW}SKIP${RESET}  %s  ${DIM}(%s)${RESET}\n" "$display_name" "$reason"
        SKIPPED=$((SKIPPED + 1))
        return
    fi

    # Check expected file exists
    if [ ! -f "$expected_file" ]; then
        printf "  ${YELLOW}SKIP${RESET}  %s  ${DIM}(no .expected file)${RESET}\n" "$display_name"
        SKIPPED=$((SKIPPED + 1))
        return
    fi

    # Run the test
    local actual exit_code
    exit_code=0
    actual="$("$JSLING" "$js_file" 2>&1)" || exit_code=$?

    local expected
    expected="$(cat "$expected_file")"

    # Handle pattern matching for non-deterministic output
    # Lines starting with ~ are regex patterns instead of exact match
    local expected_processed=""
    local line_num=0
    local all_match=true

    while IFS= read -r exp_line || [ -n "$exp_line" ]; do
        line_num=$((line_num + 1))
        local act_line
        act_line="$(echo "$actual" | sed -n "${line_num}p")"

        if [[ "$exp_line" == "~"* ]]; then
            # Pattern match (strip leading ~)
            local pattern="${exp_line:1}"
            if ! echo "$act_line" | grep -qE "$pattern"; then
                all_match=false
                break
            fi
        else
            if [ "$act_line" != "$exp_line" ]; then
                all_match=false
                break
            fi
        fi
    done <<< "$expected"

    # Check line count matches
    local exp_lines act_lines
    exp_lines="$(echo "$expected" | wc -l | tr -d ' ')"
    act_lines="$(echo "$actual" | wc -l | tr -d ' ')"
    if [ "$exp_lines" != "$act_lines" ]; then
        all_match=false
    fi

    if [ "$all_match" = true ]; then
        printf "  ${GREEN}PASS${RESET}  %s\n" "$display_name"
        PASSED=$((PASSED + 1))
    else
        printf "  ${RED}FAIL${RESET}  %s\n" "$display_name"
        FAILED=$((FAILED + 1))
        FAIL_DETAILS+=("$js_file|$actual|$expected")
    fi
}

# --- Main ---
echo ""
echo -e "${BOLD}jsling test suite${RESET}"
echo "=================="
echo ""

# Run all test files
for js_file in "$TEST_DIR"/*.js; do
    [ -f "$js_file" ] || continue
    run_test "$js_file"
done

echo ""
echo -e "${BOLD}Results:${RESET}  ${GREEN}$PASSED passed${RESET}, ${RED}$FAILED failed${RESET}, ${YELLOW}$SKIPPED skipped${RESET}  (total: $TOTAL)"

# Show failure details
if [ ${#FAIL_DETAILS[@]} -gt 0 ]; then
    echo ""
    echo -e "${BOLD}Failure details:${RESET}"
    for detail in "${FAIL_DETAILS[@]}"; do
        IFS='|' read -r js_file actual expected <<< "$detail"
        echo ""
        echo -e "  ${RED}$(basename "$js_file")${RESET}"
        echo "  Expected:"
        echo "$expected" | sed 's/^/    /'
        echo "  Actual:"
        echo "$actual" | sed 's/^/    /'
        echo ""
        echo -e "  ${DIM}Diff:${RESET}"
        diff --color=always \
            <(echo "$expected") \
            <(echo "$actual") \
            2>/dev/null | sed 's/^/    /' || true
    done
fi

echo ""
[ $FAILED -eq 0 ] && exit 0 || exit 1
