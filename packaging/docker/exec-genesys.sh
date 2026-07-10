#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/scripts/common.sh"
load_config

APP_MENU=(
  "GenESyS Terminal Shell|terminal-app|genesys_terminal_application|terminal"
  "GenESyS Qt GUI|gui-app|genesys_gui|gui"
  "HTTP Worker GUI|gui-httpworker|genesys_httpworker_gui_application|httpworker"
  "Data Analyser GUI|gui-dataanalyser|genesys_dataanalyser_gui_application|dataanalyser"
  "Optimizer GUI|gui-optimizer|genesys_optimizer_gui_application|optimizer"
  "AI Assistant GUI|gui-ai-assistant|genesys_ai_assistant_gui_application|ai-assistant"
)

app_binary_path() {
  case "$1" in
    terminal) printf '%s\n' 'build/terminal-app genesys_terminal_application' ;;
    gui) printf '%s\n' 'build/gui-app genesys-gui' ;;
    httpworker) printf '%s\n' 'build/gui-httpworker genesys_httpworker_gui_application' ;;
    dataanalyser) printf '%s\n' 'build/gui-dataanalyser genesys_dataanalyser_gui_application' ;;
    optimizer) printf '%s\n' 'build/gui-optimizer genesys_optimizer_gui_application' ;;
    ai-assistant) printf '%s\n' 'build/gui-ai-assistant genesys_ai_assistant_gui_application' ;;
    *) return 1 ;;
  esac
}

image_supports_app() {
  local mode="$1"
  local descriptor
  descriptor="$(app_binary_path "${mode}")" || return 1
  local build_dir name
  read -r build_dir name <<<"${descriptor}"
  local found
  found="$(docker run --rm --entrypoint sh "${GENESYS_RUNTIME_IMAGE}" -lc "find /src/genesys/${build_dir} -type f -perm -111 -name ${name} -print -quit 2>/dev/null")"
  [[ -n "${found}" ]]
}

usage() {
  cat <<EOF
Usage: exec-genesys.sh [--help] [--rebuild-image] [--pull] [--branch BRANCH] [--repo URL] [--app ID]
EOF
}

image_exists() { docker image inspect "${GENESYS_RUNTIME_IMAGE}" >/dev/null 2>&1; }

build_image() {
  local pull_flag=()
  [[ "${DOCKER_PULL:-0}" == "1" ]] && pull_flag+=(--pull)
  docker build "${pull_flag[@]}" -f "${SCRIPT_DIR}/dockerfiles/runtime.Dockerfile" \
    --build-arg GENESYS_REPOSITORY_URL="${GENESYS_REPOSITORY_URL}" \
    --build-arg GENESYS_RUNTIME_BRANCH="${GENESYS_RUNTIME_BRANCH}" \
    --build-arg GENESYS_BUILD_TYPE="${GENESYS_BUILD_TYPE}" \
    -t "${GENESYS_RUNTIME_IMAGE}" "${SCRIPT_DIR}"
}

select_app() {
  local choice="${1:-}"
  if [[ -n "${choice}" ]]; then printf '%s\n' "${choice}"; return; fi
  echo "Escolha uma aplicação:"
  local i=1
  for item in "${AVAILABLE_APP_MENU[@]}"; do
    IFS='|' read -r name _ <<<"${item}"
    printf '  %d) %s\n' "${i}" "${name}"
    ((i++))
  done
  read -r -p '> ' choice
  printf '%s\n' "${choice}"
}

app_by_choice() {
  local choice="$1"
  if [[ "${choice}" =~ ^[0-9]+$ ]]; then
    (( choice >= 1 && choice <= ${#AVAILABLE_APP_MENU[@]} )) || die "opção inválida: ${choice}"
    printf '%s\n' "${AVAILABLE_APP_MENU[$((choice-1))]}"
    return
  fi
  for item in "${AVAILABLE_APP_MENU[@]}"; do IFS='|' read -r name preset target mode <<<"${item}"; [[ "${choice}" == "${mode}" || "${choice}" == "${name}" ]] && printf '%s\n' "${item}" && return; done
  die "aplicação inválida: ${choice}"
}

main() {
  require_docker
  ensure_dirs
  setup_host_gui_args
  if [[ "${REBUILD_IMAGE:-0}" == "1" ]] || ! image_exists; then
    build_image
  fi
  AVAILABLE_APP_MENU=()
  local item
  for item in "${APP_MENU[@]}"; do
    IFS='|' read -r name preset target mode <<<"${item}"
    if image_supports_app "${mode}"; then
      AVAILABLE_APP_MENU+=("${item}")
    fi
  done
  [[ ${#AVAILABLE_APP_MENU[@]} -gt 0 ]] || die "a imagem runtime não contém aplicações executáveis."
  if [[ -n "${APP_ID:-}" ]]; then
    item="$(app_by_choice "${APP_ID}")"
  else
    item="$(app_by_choice "$(select_app)")"
  fi
  IFS='|' read -r name preset target mode <<<"${item}"
  log "Executando ${name} usando branch ${GENESYS_RUNTIME_BRANCH}."
  if [[ "${mode}" != "terminal" && ${#HOST_GUI_ARGS[@]} -eq 0 ]]; then
    die "nenhum display X11/Wayland foi detectado; não é possível abrir uma aplicação gráfica."
  fi
  docker run --rm -it "${HOST_GUI_ARGS[@]}" -e "QT_X11_NO_MITSHM=1" "${GENESYS_RUNTIME_IMAGE}" "${mode}"
}

APP_ID=""
REBUILD_IMAGE=0
DOCKER_PULL=0
while [[ $# -gt 0 ]]; do
  case "$1" in
    --help) usage; exit 0 ;;
    --rebuild-image) REBUILD_IMAGE=1 ;;
    --pull) DOCKER_PULL=1 ;;
    --branch) GENESYS_RUNTIME_BRANCH="$2"; shift ;;
    --repo) GENESYS_REPOSITORY_URL="$2"; shift ;;
    --app) APP_ID="$2"; shift ;;
    *) die "opção desconhecida: $1" ;;
  esac
  shift
done
main
