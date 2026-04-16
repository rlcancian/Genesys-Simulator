#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

GENESYS_IMAGE="${GENESYS_IMAGE:-genesys-simulator:local}"
GENESYS_REPO_URL="${GENESYS_REPO_URL:-https://github.com/rlcancian/Genesys-Simulator.git}"
GENESYS_STATE_DIR="${GENESYS_STATE_DIR:-${SCRIPT_DIR}/.genesys-state}"
GENESYS_NOVNC_PORT="${GENESYS_NOVNC_PORT:-6080}"
GENESYS_WEB_PORT="${GENESYS_WEB_PORT:-8080}"
GENESYS_DEFAULT_BRANCH="default"

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
    docker image inspect "${GENESYS_IMAGE}" >/dev/null 2>&1
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
    local url="http://localhost:${GENESYS_NOVNC_PORT}/vnc.html?autoconnect=1&resize=scale"

    log "${label}"
    if [[ "${branch}" == "${GENESYS_DEFAULT_BRANCH}" ]]; then
        log "O branch padrão do repositório remoto será detectado antes da execução."
    else
        log "O branch remoto '${branch}' será verificado antes da execução."
    fi
    log "A interface gráfica abrirá no navegador: ${url}"
    open_url_when_ready "${url}" "${GENESYS_NOVNC_PORT}"

    docker run --rm -it \
        --name "genesys-${mode}" \
        --publish "127.0.0.1:${GENESYS_NOVNC_PORT}:6080" \
        --env "GENESYS_REPO_URL=${GENESYS_REPO_URL}" \
        --volume "${GENESYS_STATE_DIR}:/workspace" \
        --volume "${GENESYS_STATE_DIR}/home:/home/genesys" \
        "${GENESYS_IMAGE}" "${mode}" "${branch}"
}

run_web_mode() {
    local branch="${GENESYS_DEFAULT_BRANCH}"

    log "Iniciando servidor web do GenESyS."
    log "O branch padrão do repositório remoto será detectado antes da execução."
    log "Servidor disponível em http://localhost:${GENESYS_WEB_PORT}"

    docker run --rm -it \
        --name "genesys-web" \
        --publish "${GENESYS_WEB_PORT}:${GENESYS_WEB_PORT}" \
        --env "GENESYS_REPO_URL=${GENESYS_REPO_URL}" \
        --env "GENESYS_WEB_PORT=${GENESYS_WEB_PORT}" \
        --volume "${GENESYS_STATE_DIR}:/workspace" \
        --volume "${GENESYS_STATE_DIR}/home:/home/genesys" \
        "${GENESYS_IMAGE}" web "${branch}"
}

show_menu() {
    cat <<MENU

GenESyS Docker
==============
1. Usar o GenESyS como usuário
2. Usar o GenESyS como desenvolvedor
3. Iniciar o servidor web do GenESyS
4. Sair

MENU
}

main() {
    require_docker
    mkdir -p "${GENESYS_STATE_DIR}/home"
    ensure_image

    while true; do
        show_menu
        read -r -p "Escolha uma opção: " option

        case "${option}" in
            1)
                run_graphical_mode "gui" "${GENESYS_DEFAULT_BRANCH}" "Compilando e abrindo GenesysQtGUI."
                ;;
            2)
                run_graphical_mode "ide" "currentStable" "Abrindo QtCreator para desenvolvimento do GenESyS."
                ;;
            3)
                run_web_mode
                ;;
            4)
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
