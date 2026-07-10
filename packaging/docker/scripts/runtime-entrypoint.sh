#!/usr/bin/env bash
set -Eeuo pipefail

find_executable() {
  local build_dir="$1"
  local name="$2"
  find "/src/genesys/${build_dir}" -type f -perm -111 -name "${name}" -print -quit
}

case "${1:-}" in
  terminal)
    exec "$(find_executable build/terminal-app genesys_terminal_application)"
    ;;
  gui)
    export QT_X11_NO_MITSHM=1
    exec "$(find_executable build/gui-app genesys-gui)"
    ;;
  httpworker)
    export QT_X11_NO_MITSHM=1
    exec "$(find_executable build/gui-httpworker genesys_httpworker_gui_application)"
    ;;
  dataanalyser)
    export QT_X11_NO_MITSHM=1
    exec "$(find_executable build/gui-dataanalyser genesys_dataanalyser_gui_application)"
    ;;
  optimizer)
    export QT_X11_NO_MITSHM=1
    exec "$(find_executable build/gui-optimizer genesys_optimizer_gui_application)"
    ;;
  ai-assistant)
    export QT_X11_NO_MITSHM=1
    exec "$(find_executable build/gui-ai-assistant genesys_ai_assistant_gui_application)"
    ;;
  *)
    echo "usage: terminal|gui|httpworker|dataanalyser|optimizer|ai-assistant" >&2
    exit 2
    ;;
esac
