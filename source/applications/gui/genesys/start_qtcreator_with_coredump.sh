#!/usr/bin/env bash
set -euo pipefail

QTCREATOR_BIN="${QTCREATOR_BIN:-qtcreator}"

if ! command -v "${QTCREATOR_BIN}" >/dev/null 2>&1; then
    echo "[ERROR] Qt Creator not found in PATH."
    echo "        Set QTCREATOR_BIN=/full/path/to/qtcreator or add qtcreator to PATH."
    exit 1
fi

# Enable unlimited core dump size for this shell and child processes.
ulimit -c unlimited

echo "[INFO] Core dump size limit for this shell: $(ulimit -c)"
echo "[INFO] Launching Qt Creator from: ${QTCREATOR_BIN}"
echo "[INFO] After a crash, run ./inspect_latest_genesys_coredump.sh"
echo

"${QTCREATOR_BIN}" "$@" &
QT_PID=$!

echo "[INFO] Qt Creator PID: ${QT_PID}"
echo "[INFO] Shell prompt is free again for diagnostic commands."
