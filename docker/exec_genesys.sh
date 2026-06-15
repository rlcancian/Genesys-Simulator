#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

GENESYS_IMAGE="${GENESYS_IMAGE:-genesys-simulator:local}"
GENESYS_REPO_URL="${GENESYS_REPO_URL:-https://github.com/rlcancian/Genesys-Simulator.git}"
GENESYS_STATE_DIR="${GENESYS_STATE_DIR:-${SCRIPT_DIR}/.genesys-state}"
GENESYS_NOVNC_PORT="${GENESYS_NOVNC_PORT:-6080}"
GENESYS_WEB_PORT="${GENESYS_WEB_PORT:-8080}"
GENESYS_DEFAULT_BRANCH="default"
GENESYS_MODE="${GENESYS_MODE:-local}"
GENESYS_PROJECT_DIR="${GENESYS_PROJECT_DIR:-$(cd "${SCRIPT_DIR}/.." && pwd)}"

DISPLAY_ARGS="" # Configuration changes depending on the host's window manager
GENESYS_VNC="${GENESYS_VNC:-auto}" # auto|0|1  — auto detecta pelo display do host

log() {
    printf '[exec_genesys] %s\n' "$*"
}

die() {
    printf '[exec_genesys] erro: %s\n' "$*" >&2
    exit 1
}

require_docker() {
    if ! command -v docker >/dev/null 2>&1; then
        die "Docker não está disponível. Instale o Docker antes de executar este script."
    fi

    if ! docker info >/dev/null 2>&1; then
        die "não foi possível acessar o Docker. Verifique se o serviço está ativo e se seu usuário tem permissão para usá-lo."
    fi
}

image_exists() {
    return 1
}

build_image() {
    log "Imagem Docker '${GENESYS_IMAGE}' não encontrada."
    log "Montando a imagem do GenESyS. Essa etapa pode levar alguns minutos na primeira execução."

    docker build \
        --build-arg USER_ID="$(id -u)" \
        --build-arg GROUP_ID="$(id -g)" \
        --tag "${GENESYS_IMAGE}" \
        "${SCRIPT_DIR}"
}

ensure_image() {
    if ! image_exists; then
        build_image
    fi
}

open_url() {
    local url="$1"

    if command -v xdg-open >/dev/null 2>&1; then
        xdg-open "${url}" >/dev/null 2>&1 || true
    elif command -v open >/dev/null 2>&1; then
        open "${url}" >/dev/null 2>&1 || true
    fi
}

local_extra_args() {
    LOCAL_EXTRA_ARGS=()
    if [[ "${GENESYS_MODE}" == "local" ]]; then
        LOCAL_EXTRA_ARGS+=(
            --volume "${GENESYS_PROJECT_DIR}:/workspace/sources/local:Z"
            --env GENESYS_LOCAL=1
        )
    fi
}

open_url_when_ready() {
    local url="$1"
    local port="$2"

    (
        for _ in $(seq 1 180); do
            if (echo >/dev/tcp/127.0.0.1/"${port}") >/dev/null 2>&1; then
                open_url "${url}"
                exit 0
            fi
            sleep 2
        done
    ) &
}

run_graphical_mode() {
    local mode="$1"
    local branch="$2"
    local label="$3"

    log "${label}"
    if [[ "${GENESYS_MODE}" == "local" ]]; then
        log "Modo local: usando código em '${GENESYS_PROJECT_DIR}'."
    elif [[ "${branch}" == "${GENESYS_DEFAULT_BRANCH}" ]]; then
        log "O branch padrão do repositório remoto será detectado antes da execução."
    else
        log "O branch remoto '${branch}' será verificado antes da execução."
    fi

    local vnc_docker_args=()
    if [[ "${GENESYS_VNC}" == "1" ]]; then
        local url="http://localhost:${GENESYS_NOVNC_PORT}/vnc.html?autoconnect=1&resize=scale"
        log "VNC ativo — interface gráfica disponível em: ${url}"
        open_url_when_ready "${url}" "${GENESYS_NOVNC_PORT}"
        vnc_docker_args+=(--publish "127.0.0.1:${GENESYS_NOVNC_PORT}:6080")
    else
        log "Display direto — usando display do host."
    fi

    local_extra_args
    docker run --rm -it \
        --name "genesys-${mode}" \
        --env "GENESYS_VNC=${GENESYS_VNC}" \
        --env "GENESYS_REPO_URL=${GENESYS_REPO_URL}" \
        --volume "${GENESYS_STATE_DIR}/home:/home/genesys" \
        --user 0:0 \
        "${vnc_docker_args[@]}" \
        $DISPLAY_ARGS \
        "${LOCAL_EXTRA_ARGS[@]}" \
        "${GENESYS_IMAGE}" "${mode}" "${branch}"
}

run_gui_release() {
    log "Iniciando o GenESyS (versão Release) no Docker."

    local vnc_docker_args=()
    if [[ "${GENESYS_VNC}" == "1" ]]; then
        local url="http://localhost:${GENESYS_NOVNC_PORT}/vnc.html?autoconnect=1&resize=scale"
        log "VNC ativo — interface gráfica disponível em: ${url}"
        open_url_when_ready "${url}" "${GENESYS_NOVNC_PORT}"
        vnc_docker_args+=(--publish "127.0.0.1:${GENESYS_NOVNC_PORT}:6080")
    else
        log "Display direto — usando display do host."
    fi

    local_extra_args
    docker run --rm -it \
        --name "genesys-release" \
        --env "GENESYS_VNC=${GENESYS_VNC}" \
        --env "GENESYS_REPO_URL=${GENESYS_REPO_URL}" \
        --volume "${GENESYS_STATE_DIR}/home:/home/genesys" \
        --user 0:0 \
        "${vnc_docker_args[@]}" \
        $DISPLAY_ARGS \
        "${LOCAL_EXTRA_ARGS[@]}" \
        "${GENESYS_IMAGE}" release
}

run_web_mode() {
    local branch="${GENESYS_DEFAULT_BRANCH}"

    log "Iniciando servidor web do GenESyS."
    if [[ "${GENESYS_MODE}" == "local" ]]; then
        log "Modo local: usando código em '${GENESYS_PROJECT_DIR}'."
    else
        log "O branch padrão do repositório remoto será detectado antes da execução."
    fi
    log "Servidor disponível em http://localhost:${GENESYS_WEB_PORT}"

    local_extra_args
    docker run --rm -it \
        --name "genesys-web" \
        --publish "${GENESYS_WEB_PORT}:${GENESYS_WEB_PORT}" \
        --env "GENESYS_REPO_URL=${GENESYS_REPO_URL}" \
        --env "GENESYS_WEB_PORT=${GENESYS_WEB_PORT}" \
        --volume "${GENESYS_STATE_DIR}/home:/home/genesys" \
        --user 0:0 \
        "${LOCAL_EXTRA_ARGS[@]}" \
        "${GENESYS_IMAGE}" web "${branch}"
}

run_attach_mode() {
    log "Abrindo bash interativo no container."
    if [[ "${GENESYS_MODE}" == "local" ]]; then
        log "Modo local: código em '${GENESYS_PROJECT_DIR}' disponível em /workspace/sources/local."
    fi

    local_extra_args
    docker run --rm -it \
        --name "genesys-attach" \
        --volume "${GENESYS_STATE_DIR}/home:/home/genesys" \
        --user 0:0 \
        "${LOCAL_EXTRA_ARGS[@]}" \
        "${GENESYS_IMAGE}" attach
}

detect_window_manager() {
    # Verifica se o host tem GPU acessível para o container
    local gpu_args=""
    if [[ -d "/dev/dri" ]]; then
        gpu_args="--device /dev/dri"
    else
        gpu_args="-e LIBGL_ALWAYS_SOFTWARE=1"
    fi

    if [[ -v WAYLAND_DISPLAY && -n "$WAYLAND_DISPLAY" ]]; then
        DISPLAY_ARGS="-e WAYLAND_DISPLAY=$WAYLAND_DISPLAY \
            -e XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR \
            -v $XDG_RUNTIME_DIR/$WAYLAND_DISPLAY:$XDG_RUNTIME_DIR/$WAYLAND_DISPLAY \
            ${gpu_args}"
        [[ "${GENESYS_VNC}" == "auto" ]] && GENESYS_VNC=0
    elif [[ -v DISPLAY && -n "$DISPLAY" ]]; then
        local xauth_file="${XAUTHORITY:-${HOME}/.Xauthority}"
        # Permite que o container acesse o X server do host
        xhost +local:docker >/dev/null 2>&1 || true
        DISPLAY_ARGS="-e DISPLAY=$DISPLAY \
            -v /tmp/.X11-unix:/tmp/.X11-unix \
            ${gpu_args}"
        if [[ -f "${xauth_file}" ]]; then
            DISPLAY_ARGS="${DISPLAY_ARGS} \
                -e XAUTHORITY=${xauth_file} \
                -v ${xauth_file}:${xauth_file}:ro"
        fi
        [[ "${GENESYS_VNC}" == "auto" ]] && GENESYS_VNC=0
    else
        [[ "${GENESYS_VNC}" == "auto" ]] && GENESYS_VNC=1
    fi
}

show_menu() {
    local mode_label vnc_label
    if [[ "${GENESYS_MODE}" == "local" ]]; then
        mode_label="local  →  ${GENESYS_PROJECT_DIR}"
    else
        mode_label="remoto →  ${GENESYS_REPO_URL}"
    fi
    if [[ "${GENESYS_VNC}" == "1" ]]; then
        vnc_label="VNC (http://localhost:${GENESYS_NOVNC_PORT})"
    else
        vnc_label="display direto (${DISPLAY:-${WAYLAND_DISPLAY:-sem display}})"
    fi

    cat <<MENU

GenESyS Docker
==============
Modo atual: ${mode_label}
Display:    ${vnc_label}

1. Usar o GenESyS como usuário
2. Usar o GenESyS como desenvolvedor
3. Iniciar o servidor web do GenESyS
4. Abrir bash no container (attach)
5. Alternar modo local/remoto
6. Alternar VNC / display direto
7. Sair

MENU
}

main() {
    require_docker
    mkdir -p "${GENESYS_STATE_DIR}/home"
    ensure_image
    detect_window_manager

    while true; do
        show_menu
        read -r -p "Escolha uma opção: " option

        case "${option}" in
            1)
                run_gui_release
                ;;
            2)
                run_graphical_mode "ide" "currentStable" "Abrindo QtCreator para desenvolvimento do GenESyS."
                ;;
            3)
                run_web_mode
                ;;
            4)
                run_attach_mode
                ;;
            5)
                if [[ "${GENESYS_MODE}" == "local" ]]; then
                    GENESYS_MODE="remote"
                    log "Modo alterado para: remoto."
                else
                    GENESYS_MODE="local"
                    log "Modo alterado para: local (${GENESYS_PROJECT_DIR})."
                fi
                ;;
            6)
                if [[ "${GENESYS_VNC}" == "1" ]]; then
                    GENESYS_VNC=0
                    log "VNC desativado — display direto será usado."
                else
                    GENESYS_VNC=1
                    log "VNC ativado — acesse http://localhost:${GENESYS_NOVNC_PORT}"
                fi
                ;;
            7)
                log "Encerrando."
                exit 0
                ;;
            *)
                log "Opção inválida."
                ;;
        esac
    done
}

main "$@"
