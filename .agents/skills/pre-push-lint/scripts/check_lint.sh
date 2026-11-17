#!/bin/bash
# Pre-Push Formatting and Linting Verification Script
# Validates Python and firmware code before committing and pushing.

set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
cd "$REPO_ROOT"

AUTO_FIX=false
CHECK_FIRMWARE=false

for arg in "$@"; do
    case "$arg" in
        --fix)
            AUTO_FIX=true
            ;;
        --firmware)
            CHECK_FIRMWARE=true
            ;;
        --help|-h)
            echo "Usage: $0 [--fix] [--firmware]"
            echo "  --fix       Automatically format and fix autofixable lint issues"
            echo "  --firmware  Also run PlatformIO firmware static checks (requires pio and network)"
            exit 0
            ;;
    esac
done

echo "============================================================"
echo "  Running Pre-Push Formatting and Linting Checks"
echo "============================================================"

# 1. Python Formatting and Linting
if [ "$AUTO_FIX" = true ]; then
    echo "[INFO] Auto-fixing Python linting issues with ruff..."
    ruff check --fix python
    echo "[INFO] Formatting Python code with ruff..."
    ruff format python
fi

echo "[CHECK 1/2] Verifying Python linting (ruff check)..."
ruff check python

echo "[CHECK 2/2] Verifying Python formatting (ruff format --check)..."
ruff format --check python

# 2. Firmware Checks (Optional)
if [ "$CHECK_FIRMWARE" = true ]; then
    echo "[CHECK 3] Running PlatformIO firmware checks..."
    if command -v pio >/dev/null 2>&1; then
        echo "  Checking stm32 environment..."
        (cd stm32 && pio check -e stm32)
        echo "  Checking esp32 environment..."
        (cd esp32 && pio check -e release)
    else
        echo "[WARNING] PlatformIO ('pio') not found in PATH, skipping firmware checks."
    fi
fi

echo "============================================================"
echo "  SUCCESS: All pre-push checks passed cleanly!"
echo "============================================================"
