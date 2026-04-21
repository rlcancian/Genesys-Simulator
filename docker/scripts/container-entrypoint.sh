#!/usr/bin/env bash
set -euo pipefail

GENESYS_REPO_URL="${GENESYS_REPO_URL:-https://github.com/rlcancian/Genesys-Simulator.git}"
GENESYS_WORKSPACE="${GENESYS_WORKSPACE:-/workspace}"
GENESYS_SOURCES_DIR="${GENESYS_SOURCES_DIR:-${GENESYS_WORKSPACE}/sources}"
GENESYS_WEB_PORT="${GENESYS_WEB_PORT:-8080}"
GENESYS_VNC_GEOMETRY="${GENESYS_VNC_GEOMETRY:-1600x1000x24}"
GENESYS_NOVNC_PORT="6080"

usage() {
    cat <<USAGE
Uso interno:
  genesys-container gui <branch>
  genesys-container ide <branch>
  genesys-container web <branch>

Use "default" como <branch> para resolver automaticamente o branch padrao
apontado pelo HEAD simbolico do repositorio remoto.
USAGE
}

log() {
    printf '[genesys-docker] %s\n' "$*" >&2
}

die() {
    printf '[genesys-docker] erro: %s\n' "$*" >&2
    exit 1
}

is_clean_worktree() {
    git -C "$1" diff --quiet && git -C "$1" diff --cached --quiet
}

ensure_remote_branch() {
    local branch="$1"

    if ! git ls-remote --exit-code --heads "${GENESYS_REPO_URL}" "${branch}" >/dev/null 2>&1; then
        die "branch remoto '${branch}' não encontrado em ${GENESYS_REPO_URL}"
    fi
}

resolve_branch() {
    local requested_branch="$1"
    local symref

    case "${requested_branch}" in
        default|HEAD)
            symref="$(git ls-remote --symref "${GENESYS_REPO_URL}" HEAD 2>/dev/null | awk '/^ref:/ { print $2; exit }')"
            [[ -n "${symref}" ]] || die "não foi possível detectar o branch padrão remoto de ${GENESYS_REPO_URL}"
            printf '%s\n' "${symref#refs/heads/}"
            ;;
        origin)
            die "'origin' é o nome local do repositório remoto, não um branch. Use 'default' para selecionar o branch principal remoto."
            ;;
        *)
            printf '%s\n' "${requested_branch}"
            ;;
    esac
}

sync_repository() {
    local branch="$1"
    local repo_dir="${GENESYS_SOURCES_DIR}/${branch}"

    mkdir -p "${GENESYS_SOURCES_DIR}"
    ensure_remote_branch "${branch}"

    if [[ ! -d "${repo_dir}/.git" ]]; then
        log "Repositório local do branch '${branch}' não encontrado."
        log "Baixando ${GENESYS_REPO_URL} (${branch}). Isso pode levar alguns minutos."
        git clone --branch "${branch}" --single-branch "${GENESYS_REPO_URL}" "${repo_dir}"
        printf '%s\n' "${repo_dir}"
        return
    fi

    log "Verificando atualizações do branch '${branch}'."
    git -C "${repo_dir}" remote set-url origin "${GENESYS_REPO_URL}"
    git -C "${repo_dir}" fetch origin "${branch}"

    if [[ "$(git -C "${repo_dir}" rev-parse --abbrev-ref HEAD)" != "${branch}" ]]; then
        if is_clean_worktree "${repo_dir}"; then
            git -C "${repo_dir}" checkout "${branch}"
        else
            die "o repositório ${repo_dir} tem alterações locais; não posso trocar para o branch ${branch}"
        fi
    fi

    local local_rev
    local remote_rev
    local base_rev
    local_rev="$(git -C "${repo_dir}" rev-parse HEAD)"
    remote_rev="$(git -C "${repo_dir}" rev-parse "origin/${branch}")"
    base_rev="$(git -C "${repo_dir}" merge-base HEAD "origin/${branch}")"

    if [[ "${local_rev}" == "${remote_rev}" ]]; then
        log "Repositório já está atualizado."
    elif [[ "${base_rev}" == "${local_rev}" ]]; then
        if is_clean_worktree "${repo_dir}"; then
            log "Repositório local está atrás do remoto. Baixando atualização."
            git -C "${repo_dir}" pull --ff-only origin "${branch}"
        else
            die "o repositório ${repo_dir} está atrás do remoto, mas há alterações locais não commitadas"
        fi
    elif [[ "${base_rev}" == "${remote_rev}" ]]; then
        log "Repositório local está à frente do remoto; mantendo alterações locais."
    else
        die "branch ${branch} divergiu do remoto; resolva manualmente em ${repo_dir}"
    fi

    printf '%s\n' "${repo_dir}"
}

configure_and_build() {
    local repo_dir="$1"
    local preset="$2"
    local build_label="$3"
    local target="$4"

    log "Configurando build '${preset}'."
    cmake -S "${repo_dir}" --preset "${preset}"

    log "Compilando '${build_label}'."
    cmake --build "${repo_dir}/build/${preset}" --target "${target}" --parallel "$(nproc)"
}

build_gui() {
    configure_and_build "$1" "gui-app" "genesys-gui" "genesys_gui"
}

build_web() {
    configure_and_build "$1" "web-app" "genesys-web" "genesys_webhook"
}

find_executable() {
    local repo_dir="$1"
    local build_dir="$2"
    local name="$3"

    local found
    found="$(find "${repo_dir}/${build_dir}" -type f -perm -111 -name "${name}" -print -quit)"
    [[ -n "${found}" ]] || die "executável ${name} não encontrado em ${repo_dir}/${build_dir}"
    printf '%s\n' "${found}"
}

start_novnc() {
    export DISPLAY=:99
    export XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-/tmp/runtime-genesys}"
    mkdir -p "${XDG_RUNTIME_DIR}"

    log "Iniciando desktop virtual para aplicações gráficas."
    Xvfb "${DISPLAY}" -screen 0 "${GENESYS_VNC_GEOMETRY}" >/tmp/genesys-xvfb.log 2>&1 &
    sleep 1
    openbox >/tmp/genesys-openbox.log 2>&1 &
    x11vnc -display "${DISPLAY}" -forever -shared -nopw -quiet -listen 0.0.0.0 >/tmp/genesys-x11vnc.log 2>&1 &
    websockify --web=/usr/share/novnc 0.0.0.0:${GENESYS_NOVNC_PORT} localhost:5900 >/tmp/genesys-novnc.log 2>&1 &
    sleep 2

    log "Interface gráfica disponível em http://localhost:${GENESYS_NOVNC_PORT}/vnc.html?autoconnect=1&resize=scale"
}

run_gui() {
    local repo_dir="$1"
    build_gui "${repo_dir}"
    local executable
    executable="$(find_executable "${repo_dir}" "build/gui-app" "genesys_qt_gui_application")"
    start_novnc
    log "Abrindo GenesysQtGUI."
    exec "${executable}"
}

run_ide() {
    local repo_dir="$1"
    log "Pré-configurando projeto CMake para QtCreator."
    cmake -S "${repo_dir}" --preset gui-app
    start_novnc
    log "Abrindo QtCreator com o projeto GenESyS."
    exec qtcreator "${repo_dir}/CMakeLists.txt"
}

run_web() {
    local repo_dir="$1"
    build_web "${repo_dir}"
    local executable
    executable="$(find_executable "${repo_dir}" "build/web-app" "genesys_webhook")"
    log "Iniciando servidor web na porta ${GENESYS_WEB_PORT}."
    exec "${executable}" --port "${GENESYS_WEB_PORT}"
}

main() {
    if [[ $# -ne 2 ]]; then
        usage >&2
        exit 2
    fi

    local mode="$1"
    local requested_branch="$2"
    local branch
    local repo_dir

    branch="$(resolve_branch "${requested_branch}")"
    if [[ "${requested_branch}" != "${branch}" ]]; then
        log "Branch padrão remoto resolvido como '${branch}'."
    fi

    repo_dir="$(sync_repository "${branch}")"

    case "${mode}" in
        gui)
            run_gui "${repo_dir}"
            ;;
        ide)
            run_ide "${repo_dir}"
            ;;
        web)
            run_web "${repo_dir}"
            ;;
        *)
            usage >&2
            exit 2
            ;;
    esac
}

main "$@"
