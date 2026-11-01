#!/usr/bin/env bash
# run.sh — autotests del harness (STD-SH). Uso: tests/run.sh [--e2e]
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly SCRIPT_DIR

usage() {
    echo "Uso: $(basename "$0") [--e2e]   (--e2e: recorrido completo de las 4 fases)"
}

main() {
    case "${1:-}" in
        -h | --help)
            usage
            exit 0
            ;;
        --e2e) export H_E2E=1 ;;
        "") ;;
        *)
            usage >&2
            exit 2
            ;;
    esac
    python3 -m py_compile "${SCRIPT_DIR}"/../scripts/*.py "${SCRIPT_DIR}"/../scripts/hlib/*.py \
        "${SCRIPT_DIR}"/../hooks/*.py
    python3 "${SCRIPT_DIR}/test_harness.py"
}

main "$@"
