#!/usr/bin/env bash
set -Eeuo pipefail

GENESYS_DOCKER_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
GENESYS_REPO_ROOT="$(cd "${GENESYS_DOCKER_ROOT}/../.." && pwd)"

load_config() {
    local config="${GENESYS_DOCKER_ROOT}/config/genesys-docker.conf"
    [[ -f "${config}" ]] || die "configuração não encontrada: ${config}"
    # shellcheck disable=SC1090
    source "${config}"
}

log() { printf '[genesys-docker] %s\n' "$*"; }
die() { printf '[genesys-docker] erro: %s\n' "$*" >&2; exit 1; }

require_docker() {
    command -v docker >/dev/null 2>&1 || die "Docker não está instalado."
    docker info >/dev/null 2>&1 || die "não foi possível acessar o daemon Docker."
}

docker_platform_args() {
    [[ -n "${GENESYS_DOCKER_PLATFORM:-}" ]] && printf -- '--platform=%s' "${GENESYS_DOCKER_PLATFORM}"
}

have_display() {
    [[ -n "${DISPLAY:-}" ]] || [[ -n "${WAYLAND_DISPLAY:-}" ]]
}

setup_host_gui_args() {
    HOST_GUI_ARGS=()
    if [[ -n "${WAYLAND_DISPLAY:-}" && "${GENESYS_ENABLE_WAYLAND}" == "1" ]]; then
        HOST_GUI_ARGS+=(--env "WAYLAND_DISPLAY=${WAYLAND_DISPLAY}")
        [[ -n "${XDG_RUNTIME_DIR:-}" ]] && HOST_GUI_ARGS+=(--env "XDG_RUNTIME_DIR=${XDG_RUNTIME_DIR}" --volume "${XDG_RUNTIME_DIR}:${XDG_RUNTIME_DIR}")
        return
    fi
    if [[ -n "${DISPLAY:-}" ]]; then
        HOST_GUI_ARGS+=(--env "DISPLAY=${DISPLAY}" --env "QT_X11_NO_MITSHM=1" --volume /tmp/.X11-unix:/tmp/.X11-unix)
        [[ -f "${GENESYS_XAUTH_FILE}" ]] && HOST_GUI_ARGS+=(--env "XAUTHORITY=${GENESYS_XAUTH_FILE}" --volume "${GENESYS_XAUTH_FILE}:${GENESYS_XAUTH_FILE}:ro")
        HOST_GUI_ARGS+=(--env "QT_QPA_PLATFORM=xcb")
        return
    fi
    HOST_GUI_ARGS=()
}

ensure_dirs() {
    mkdir -p "${GENESYS_STATE_DIR}" "${GENESYS_CACHE_DIR}" "${GENESYS_SOURCE_CACHE_DIR}" "${GENESYS_BUILD_CACHE_DIR}"
}

menu_select() {
    local prompt="$1"; shift
    local options=("$@")
    local i=1
    printf '%s\n' "${prompt}"
    for opt in "${options[@]}"; do printf '  %d) %s\n' "${i}" "${opt}"; ((i++)); done
    printf '> '
}
