#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
ARCHIVE=${1:-}
if [[ -z "$ARCHIVE" ]]; then
    ARCHIVE=$(find "$SCRIPT_DIR/dist" -maxdepth 1 -type f -name 'genesys-*-portable.tar.gz' -printf '%T@ %p\n' 2>/dev/null | sort -nr | awk 'NR==1 {sub(/^[^ ]+ /, ""); print}')
fi
[[ -f "$ARCHIVE" ]] || { echo 'Usage: test-portable.sh ARCHIVE.tar.gz' >&2; exit 2; }
command -v xvfb-run >/dev/null || { echo 'SKIPPED: xvfb-run is unavailable.' >&2; exit 77; }

ROOT=$(mktemp -d)
HOME_DIR="$ROOT/home with spaces"
EXTRACT_DIR="$ROOT/extracted"
PREFIX="$ROOT/install with spaces"
mkdir -p -- "$HOME_DIR" "$EXTRACT_DIR"
trap 'rm -rf -- "$ROOT"' EXIT

tar -xzf "$ARCHIVE" -C "$EXTRACT_DIR"
export HOME="$HOME_DIR"
unset LD_LIBRARY_PATH QT_PLUGIN_PATH QT_QPA_PLATFORM_PLUGIN_PATH CMAKE_PREFIX_PATH CMAKE_COMMAND NINJA_STATUS
export PATH=/usr/bin:/bin
cd -- "$ROOT"

"$EXTRACT_DIR/install.sh" --prefix "$PREFIX" --no-desktop
"$EXTRACT_DIR/install.sh" --prefix "$PREFIX" --no-desktop
[[ -f "$PREFIX/.genesys-portable-install" ]]
[[ -x "$PREFIX/bin/genesys-gui" && -x "$PREFIX/bin/genesys-shell" && -x "$PREFIX/bin/genesys-worker" ]]
"$PREFIX/bin/genesys-dispatch" 2>/dev/null && exit 1 || true
"$PREFIX/libexec/genesys/genesys-dispatch" --help >/dev/null
"$PREFIX/libexec/genesys/genesys-launcher" --version >/dev/null

MODEL="$PREFIX/share/genesys/models/ExampleBasicOrderShipping.gen"
[[ -f "$MODEL" ]]
cp -- "$MODEL" "$ROOT/ExampleBasicOrderShipping.gen"
"$PREFIX/bin/genesys-shell" "model load $ROOT/ExampleBasicOrderShipping.gen" "model show" exit >"$ROOT/shell.log" 2>&1
grep -q 'Model loaded' "$ROOT/shell.log" || { cat "$ROOT/shell.log" >&2; exit 1; }

set +e
xvfb-run -a -s '-nolisten tcp' timeout 8s "$PREFIX/bin/genesys-gui" >"$ROOT/gui.log" 2>&1
gui_status=$?
set -e
[[ "$gui_status" -eq 124 ]] || { cat "$ROOT/gui.log" >&2; exit 1; }
if grep -Eiq 'could not find the Qt platform plugin|failed to load the Qt platform plugin|qt platform plugin.*could not be initialized|fatal error' "$ROOT/gui.log"; then
    cat "$ROOT/gui.log" >&2
    exit 1
fi

"$PREFIX/bin/genesys-worker" --port 0 --max-requests 1 >"$ROOT/worker.log" 2>&1 &
worker_pid=$!
sleep 1
kill -0 "$worker_pid"
kill "$worker_pid" 2>/dev/null || true
wait "$worker_pid" 2>/dev/null || true

"$EXTRACT_DIR/uninstall.sh" --prefix "$PREFIX" --no-desktop
[[ ! -e "$PREFIX" ]]
printf 'Portable lifecycle test PASS: %s\n' "$ARCHIVE"
