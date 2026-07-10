#!/usr/bin/env bash
set -euo pipefail

COMM="${1:-GenesysQtGUI}"
MODE="${2:-batch}"

if ! command -v coredumpctl >/dev/null 2>&1; then
    echo "[ERROR] coredumpctl not found."
    echo "        This workflow requires systemd-coredump/coredumpctl."
    exit 1
fi

echo "============================================================"
echo "[INFO] Listing the most recent matching core dump for: ${COMM}"
echo "============================================================"
if ! coredumpctl -1 --no-pager list "${COMM}"; then
    echo "[ERROR] No matching core dump was found for '${COMM}'."
    echo "        Possibilities:"
    echo "        - the crash was not captured by systemd-coredump"
    echo "        - the executable/COMM name does not match"
    echo "        - the core dump has already been removed"
    exit 1
fi

echo
echo "============================================================"
echo "[INFO] Detailed information about the most recent matching core dump"
echo "============================================================"
coredumpctl -1 --no-pager info "${COMM}"

echo
if [[ "${MODE}" == "--interactive" ]]; then
    echo "============================================================"
    echo "[INFO] Opening interactive GDB session on the most recent core dump"
    echo "============================================================"
    exec coredumpctl -1 --debugger=gdb debug "${COMM}"
else
    echo "============================================================"
    echo "[INFO] Running non-interactive GDB backtrace collection"
    echo "============================================================"
    DEBUGGER_ARGS="-batch -ex 'set pagination off' -ex 'bt' -ex 'bt full' -ex 'thread apply all bt full'"
    coredumpctl -1 --debugger=gdb --debugger-arguments="${DEBUGGER_ARGS}" debug "${COMM}"
fi
