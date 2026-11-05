#!/usr/bin/env bash
# Executed by GitHub Actions with a private Xvfb display; no user installation is touched.
set -Eeuo pipefail

prefix=${1:?Usage: test-standalone-tools-xvfb.sh INSTALL_PREFIX}
gui="$prefix/libexec/genesys/system/bin/genesys-gui"
[[ -x "$gui" ]] || { echo "Missing installed main GUI: $gui" >&2; exit 1; }
for name in genesys-dataanalyser-gui genesys-optimizer-gui genesys-ai-assistant-gui; do
    [[ -x "$prefix/bin/$name" ]] || { echo "Missing installed tool: $name" >&2; exit 1; }
done

: "${DISPLAY:?A private Xvfb DISPLAY is required}"
export QT_QPA_PLATFORM=xcb
gui_pid=""
wm_pid=""
declare -a tool_pids=()

cleanup() {
    local pid
    for pid in "${tool_pids[@]}"; do
        kill -TERM "$pid" 2>/dev/null || true
    done
    if [[ -n "$gui_pid" ]]; then
        kill -TERM "$gui_pid" 2>/dev/null || true
        wait "$gui_pid" 2>/dev/null || true
    fi
    if [[ -n "$wm_pid" ]]; then
        kill -TERM "$wm_pid" 2>/dev/null || true
        wait "$wm_pid" 2>/dev/null || true
    fi
}
trap cleanup EXIT

wait_for_window() {
    local pid=$1
    local attempts=0
    while ((attempts++ < 100)); do
        if ! kill -0 "$pid" 2>/dev/null; then return 1; fi
        local id
        id=$(xdotool search --onlyvisible --pid "$pid" 2>/dev/null | head -n 1 || true)
        if [[ -n "$id" ]]; then printf '%s\n' "$id"; return 0; fi
        sleep 0.2
    done
    return 1
}

# xdotool windowactivate uses the EWMH _NET_ACTIVE_WINDOW protocol. Xvfb
# alone has no window manager and cannot honor activation requests.
openbox --sm-disable >"$RUNNER_TEMP/genesys-openbox.log" 2>&1 &
wm_pid=$!
wm_ready=false
for _ in $(seq 1 100); do
    if ! kill -0 "$wm_pid" 2>/dev/null; then
        cat "$RUNNER_TEMP/genesys-openbox.log" >&2
        echo "Openbox exited before becoming ready" >&2
        exit 1
    fi
    if xprop -root _NET_SUPPORTING_WM_CHECK 2>/dev/null | grep -q 'window id #'; then
        wm_ready=true
        break
    fi
    sleep 0.1
done
if [[ "$wm_ready" != true ]]; then
    cat "$RUNNER_TEMP/genesys-openbox.log" >&2
    echo "No EWMH window manager detected in the private Xvfb display" >&2
    exit 1
fi

# Deliberately do not open a model: Tools must be available from an empty GUI.
"$gui" >"$RUNNER_TEMP/genesys-main-gui.log" 2>&1 &
gui_pid=$!
gui_window=$(wait_for_window "$gui_pid") || {
    cat "$RUNNER_TEMP/genesys-main-gui.log" >&2
    echo "Main GUI did not create a visible window" >&2
    exit 1
}
echo "Main GUI PID=$gui_pid window=$gui_window exe=$(readlink -f "/proc/$gui_pid/exe")"

# mainwindow.ui has seven top-level menus; Tools is the sixth (0-based index 5).
# Qt/xdotool F10 keyboard navigation exercises the real QAction signals and
# MainWindow -> DialogUtilityController -> ToolLauncher path.
tools=(
    "genesys-dataanalyser-gui"
    "genesys-optimizer-gui"
    "genesys-ai-assistant-gui"
)

for index in "${!tools[@]}"; do
    tool="${tools[$index]}"
    xdotool windowactivate --sync "$gui_window"
    xdotool key --clearmodifiers F10
    for _ in 1 2 3 4 5; do xdotool key --clearmodifiers Right; done
    xdotool key --clearmodifiers Down
    for ((n=0; n<index; ++n)); do xdotool key --clearmodifiers Down; done
    xdotool key --clearmodifiers Return

    tool_pid=""
    for _ in $(seq 1 80); do
        tool_pid=$(pgrep -f -- "$prefix/bin/$tool" | head -n 1 || true)
        [[ -z "$tool_pid" ]] || break
        sleep 0.2
    done
    if [[ -z "$tool_pid" ]]; then
        xwininfo -root -tree || true
        cat "$RUNNER_TEMP/genesys-main-gui.log" >&2
        echo "Tools menu failed to launch $tool" >&2
        exit 1
    fi
    tool_pids+=("$tool_pid")
    actual_exe=$(readlink -f "/proc/$tool_pid/exe")
    expected_exe=$(readlink -f "$prefix/bin/$tool")
    [[ "$actual_exe" == "$expected_exe" ]] || {
        echo "Wrong process for $tool: $actual_exe (expected $expected_exe)" >&2
        exit 1
    }
    tool_window=$(wait_for_window "$tool_pid") || {
        echo "$tool did not create a visible window" >&2
        exit 1
    }
    [[ -n "$tool_window" ]] && kill -0 "$gui_pid"
    echo "PASS: $tool PID=$tool_pid window=$tool_window; main GUI PID=$gui_pid alive"

    kill -TERM "$tool_pid"
    for _ in $(seq 1 60); do
        kill -0 "$tool_pid" 2>/dev/null || break
        sleep 0.2
    done
    if kill -0 "$tool_pid" 2>/dev/null; then
        echo "$tool remained alive after termination" >&2
        exit 1
    fi
    kill -0 "$gui_pid"
done

echo "PASS: all three Tools QActions launch distinct visible processes; main GUI survives shutdown"
