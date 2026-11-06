#!/usr/bin/env bash
# Executed by GitHub Actions with a private Xvfb display; no user installation is touched.
set -Eeuo pipefail

prefix=${1:?Usage: test-standalone-tools-xvfb.sh INSTALL_PREFIX}
script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
repo_root=${GITHUB_WORKSPACE:-$(cd -- "$script_dir/.." && pwd)}
runner_temp=${RUNNER_TEMP:-${TMPDIR:-/tmp}}
mkdir -p "$runner_temp"
export GITHUB_WORKSPACE=$repo_root
export RUNNER_TEMP=$runner_temp

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
declare -A tool_executables=()

find_tool_pids() {
    local expected_exe=$1 actual_exe pid tool_name=${1##*/}
    while read -r pid; do
        [[ -n "$pid" ]] || continue
        actual_exe=$(readlink -f -- "/proc/$pid/exe" 2>/dev/null || true)
        if [[ "$actual_exe" == "$expected_exe" ]]; then
            printf '%s\n' "$pid"
        fi
    done < <(pgrep -f -- "$tool_name" || true)
}

xdotool_bounded() {
    timeout 5s xdotool "$@"
}

cleanup() {
    local pid expected_exe actual_exe
    for pid in "${tool_pids[@]:-}"; do
        [[ -n "$pid" ]] || continue
        expected_exe=${tool_executables[$pid]:-}
        actual_exe=$(readlink -f -- "/proc/$pid/exe" 2>/dev/null || true)
        if [[ -n "$expected_exe" && "$actual_exe" == "$expected_exe" ]]; then
            kill -TERM "$pid" 2>/dev/null || true
            for _ in $(seq 1 20); do
                actual_exe=$(readlink -f -- "/proc/$pid/exe" 2>/dev/null || true)
                [[ "$actual_exe" != "$expected_exe" ]] && break
                sleep 0.1
            done
            actual_exe=$(readlink -f -- "/proc/$pid/exe" 2>/dev/null || true)
            [[ "$actual_exe" != "$expected_exe" ]] || kill -KILL "$pid" 2>/dev/null || true
        fi
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
    local pid=$1 attempts=0 id
    while ((attempts++ < 100)); do
        if ! kill -0 "$pid" 2>/dev/null; then return 1; fi
        id=$(timeout 3s xdotool search --onlyvisible --pid "$pid" 2>/dev/null | head -n 1 || true)
        if [[ -n "$id" ]]; then printf '%s\n' "$id"; return 0; fi
        sleep 0.2
    done
    return 1
}

wait_for_named_window() {
    local pid=$1 name=$2 attempts=0 id
    while ((attempts++ < 100)); do
        if ! kill -0 "$pid" 2>/dev/null; then return 1; fi
        id=$(timeout 3s xdotool search --onlyvisible --pid "$pid" --name "$name" 2>/dev/null | head -n 1 || true)
        if [[ -n "$id" ]]; then printf '%s\n' "$id"; return 0; fi
        sleep 0.2
    done
    return 1
}

# Prefer Openbox on CI; allow the installed Xfce window manager for local runs.
case "${GENESYS_TEST_WINDOW_MANAGER:-auto}" in
    auto)
        if command -v openbox >/dev/null 2>&1; then
            wm_name=Openbox
            wm_command=(openbox --sm-disable)
        elif command -v xfwm4 >/dev/null 2>&1; then
            wm_name=xfwm4
            wm_command=(xfwm4 --replace --compositor=off)
        else
            echo "Install Openbox or xfwm4 to activate the Tools menu under Xvfb" >&2
            exit 1
        fi
        ;;
    openbox)
        command -v openbox >/dev/null 2>&1 || { echo "Openbox is not installed" >&2; exit 1; }
        wm_name=Openbox
        wm_command=(openbox --sm-disable)
        ;;
    xfwm4)
        command -v xfwm4 >/dev/null 2>&1 || { echo "xfwm4 is not installed" >&2; exit 1; }
        wm_name=xfwm4
        wm_command=(xfwm4 --replace --compositor=off)
        ;;
    *)
        echo "Unsupported GENESYS_TEST_WINDOW_MANAGER: ${GENESYS_TEST_WINDOW_MANAGER}" >&2
        exit 2
        ;;
esac

# xdotool windowactivate uses the EWMH _NET_ACTIVE_WINDOW protocol. Xvfb
# alone has no window manager and cannot honor activation requests.
"${wm_command[@]}" >"$RUNNER_TEMP/genesys-window-manager.log" 2>&1 &
wm_pid=$!
wm_ready=false
for _ in $(seq 1 100); do
    if ! kill -0 "$wm_pid" 2>/dev/null; then
        cat "$RUNNER_TEMP/genesys-window-manager.log" >&2
        echo "$wm_name exited before becoming ready" >&2
        exit 1
    fi
    if xprop -root _NET_SUPPORTING_WM_CHECK 2>/dev/null | grep -q 'window id #'; then
        wm_ready=true
        break
    fi
    sleep 0.1
done
if [[ "$wm_ready" != true ]]; then
    cat "$RUNNER_TEMP/genesys-window-manager.log" >&2
    echo "No EWMH window manager detected in the private Xvfb display" >&2
    exit 1
fi
tools_menu_x=${GENESYS_TEST_TOOLS_MENU_X:-370}
[[ "$tools_menu_x" =~ ^[0-9]+$ ]] || {
    echo "GENESYS_TEST_TOOLS_MENU_X must be a non-negative integer" >&2
    exit 2
}
echo "Using $wm_name; Tools menu x=$tools_menu_x on the 1280px virtual display"

# Give the GUI an isolated first-run profile with plugin autoload disabled.
# Missing optional system plugins can otherwise open a diagnostic window on
# startup and steal focus from the Tools menu test. This test is about process
# launching, not plugin discovery.
export XDG_CONFIG_HOME="$RUNNER_TEMP/genesys-standalone-tools-config"
mkdir -p "$XDG_CONFIG_HOME/GenESyS/Genesys-Simulator"
cat >"$XDG_CONFIG_HOME/GenESyS/Genesys-Simulator/preferences.json" <<'JSON'
{"plugins":{"autoLoad":false},"startup":{"startMaximized":false}}
JSON

# Deliberately do not open a model: Tools must be available from an empty GUI.
"$gui" >"$RUNNER_TEMP/genesys-main-gui.log" 2>&1 &
gui_pid=$!
gui_window=$(wait_for_named_window "$gui_pid" 'GenESyS - Generic and Expansible System Simulator') || {
    cat "$RUNNER_TEMP/genesys-main-gui.log" >&2
    echo "Main GUI did not create its visible main window" >&2
    exit 1
}
expected_gui=$(readlink -f -- "$gui")
actual_gui=$(readlink -f -- "/proc/$gui_pid/exe")
[[ "$actual_gui" == "$expected_gui" ]] || {
    echo "Wrong main GUI process: $actual_gui (expected $expected_gui)" >&2
    exit 1
}
echo "Main GUI PID=$gui_pid window=$gui_window exe=$actual_gui"

xdotool_bounded windowactivate --sync "$gui_window"

# Invoke each real QAction through the Tools menu -> controller -> ToolLauncher path.
tools=(
    "genesys-dataanalyser-gui"
    "genesys-optimizer-gui"
    "genesys-ai-assistant-gui"
)

for index in "${!tools[@]}"; do
    tool=${tools[$index]}
    expected_exe=$(readlink -f -- "$prefix/bin/$tool")
    matching_pids=$(find_tool_pids "$expected_exe")
    [[ -z "$matching_pids" ]] || {
        printf 'Found an existing %s process before the menu action: %s\n' "$tool" "$matching_pids" >&2
        exit 1
    }

    xdotool_bounded windowactivate --sync "$gui_window"
    # F10 does not activate QMenuBar reliably in the Qt6/Xfwm4 combination.
    # The virtual display is fixed at 1280px and the English Tools label is at
    # x=370 in the current QMenuBar. Derive y from X11 so WM decorations remain
    # variable; CI may override x if the UI layout changes.
    window_y=$(xwininfo -id "$gui_window" | awk '/Absolute upper-left Y:/ { print $4 }')
    xdotool_bounded mousemove "$tools_menu_x" "$((window_y + 15))" click 1
    sleep 0.15
    xdotool_bounded key --clearmodifiers Down
    sleep 0.1
    for ((n=0; n<index; ++n)); do
        xdotool_bounded key --clearmodifiers Down
        sleep 0.05
    done
    xdotool_bounded key --clearmodifiers Return

    tool_pid=""
    matching_pids=""
    for _ in $(seq 1 80); do
        matching_pids=$(find_tool_pids "$expected_exe")
        [[ -z "$matching_pids" ]] || break
        sleep 0.2
    done
    if [[ -z "$matching_pids" ]]; then
        xwininfo -root -tree || true
        cat "$RUNNER_TEMP/genesys-main-gui.log" >&2
        echo "Tools menu failed to launch $tool" >&2
        exit 1
    fi
    mapfile -t started_tool_pids <<<"$matching_pids"
    if ((${#started_tool_pids[@]} != 1)); then
        printf 'Expected exactly one process for %s, found: %s\n' "$tool" "$matching_pids" >&2
        exit 1
    fi
    tool_pid=${started_tool_pids[0]}
    tool_pids+=("$tool_pid")
    tool_executables[$tool_pid]=$expected_exe
    actual_exe=$(readlink -f -- "/proc/$tool_pid/exe")
    [[ "$actual_exe" == "$expected_exe" ]] || {
        echo "Wrong process for $tool: $actual_exe (expected $expected_exe)" >&2
        exit 1
    }
    tool_window=$(wait_for_window "$tool_pid") || {
        echo "$tool did not create a visible window" >&2
        exit 1
    }
    kill -0 "$gui_pid"
    echo "PASS: $tool PID=$tool_pid window=$tool_window; main GUI PID=$gui_pid alive"

    # Exercise the normal window close shortcut; windowclose's synthetic ICCCM
    # message hides this Qt window without ending its event loop in this setup.
    xdotool_bounded windowactivate --sync "$tool_window"
    xdotool_bounded key --clearmodifiers alt+F4
    tool_closed=false
    for _ in $(seq 1 100); do
        actual_exe=$(readlink -f -- "/proc/$tool_pid/exe" 2>/dev/null || true)
        [[ "$actual_exe" == "$expected_exe" ]] || { tool_closed=true; break; }
        sleep 0.2
    done
    if [[ "$tool_closed" != true ]]; then
        echo "$tool remained alive after Alt+F4 closed its window" >&2
        ps -p "$tool_pid" -o pid,ppid,state,args >&2 || true
        xdotool search --onlyvisible --pid "$tool_pid" >&2 || true
        exit 1
    fi
    matching_pids=$(find_tool_pids "$expected_exe")
    if [[ -n "$matching_pids" ]]; then
        printf 'Orphaned %s process(es) remained after window close: %s\n' "$tool" "$matching_pids" >&2
        exit 1
    fi
    kill -0 "$gui_pid"
done

echo "PASS: all three Tools QActions launch distinct visible processes; main GUI survives window close"
