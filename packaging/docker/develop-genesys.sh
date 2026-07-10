#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/scripts/common.sh"
load_config

usage() {
  cat <<EOF
Usage: develop-genesys.sh [--help] [--rebuild-image] [--ide qtcreator] [--shell]
EOF
}

image_exists() { docker image inspect "${GENESYS_DEVELOPMENT_IMAGE}" >/dev/null 2>&1; }

build_image() {
  docker build -f "${SCRIPT_DIR}/dockerfiles/development.Dockerfile" -t "${GENESYS_DEVELOPMENT_IMAGE}" "${SCRIPT_DIR}"
}

main() {
  require_docker
  ensure_dirs
  setup_host_gui_args
  if [[ "${REBUILD_IMAGE:-0}" == "1" ]] || ! image_exists; then
    build_image
  fi
  if [[ "${OPEN_SHELL:-0}" == "1" ]]; then
    exec docker run --rm -it "${HOST_GUI_ARGS[@]}" -v "${GENESYS_REPO_ROOT}:/workspace" -w /workspace "${GENESYS_DEVELOPMENT_IMAGE}" bash
  fi
  if [[ ${#HOST_GUI_ARGS[@]} -eq 0 ]]; then
    die "nenhum display X11/Wayland foi detectado; não é possível abrir o Qt Creator."
  fi
  exec docker run --rm -it "${HOST_GUI_ARGS[@]}" -v "${GENESYS_REPO_ROOT}:/workspace" -w /workspace "${GENESYS_DEVELOPMENT_IMAGE}" "${GENESYS_DEFAULT_IDE}" /workspace/CMakeLists.txt
}

REBUILD_IMAGE=0
OPEN_SHELL=0
while [[ $# -gt 0 ]]; do
  case "$1" in
    --help) usage; exit 0 ;;
    --rebuild-image) REBUILD_IMAGE=1 ;;
    --ide) GENESYS_DEFAULT_IDE="$2"; shift ;;
    --shell) OPEN_SHELL=1 ;;
    *) die "opção desconhecida: $1" ;;
  esac
  shift
done
main
