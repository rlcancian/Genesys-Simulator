#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_FILE="${PROJECT_DIR}/GenesysQtGUI.pro"

BUILD_CONFIG="Debug"
BUILD_DIR=""
JOBS=""

usage() {
    cat <<USAGE
Usage: $(basename "$0") [options]

Options:
  --config <Debug|Release|RelWithDebInfo|MinSizeRel>  Build configuration (default: Debug)
  --build-dir <path>                                  Build directory (default: ./build/cmake-<config>)
  --jobs <n>                                          Parallel jobs for make (default: auto)
  --help                                              Show this help message
USAGE
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --config)
            BUILD_CONFIG="$2"
            shift 2
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --jobs)
            JOBS="$2"
            shift 2
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 1
            ;;
    esac
done

CONFIG_LOWER="$(echo "${BUILD_CONFIG}" | tr '[:upper:]' '[:lower:]')"
case "${CONFIG_LOWER}" in
    debug)
        QMAKE_CONFIG="debug"
        ;;
    release|relwithdebinfo|minsizerel)
        QMAKE_CONFIG="release"
        ;;
    *)
        echo "Unsupported build configuration: ${BUILD_CONFIG}" >&2
        exit 1
        ;;
esac

if [[ -z "${BUILD_DIR}" ]]; then
    BUILD_DIR="${PROJECT_DIR}/build/cmake-${CONFIG_LOWER}"
fi

QMAKE_BIN="${QMAKE_EXECUTABLE:-${QMAKE:-qmake}}"
if ! command -v "${QMAKE_BIN}" >/dev/null 2>&1; then
    echo "qmake not found. Set QMAKE_EXECUTABLE or add qmake to PATH." >&2
    exit 1
fi

if [[ -z "${JOBS}" ]]; then
    if command -v nproc >/dev/null 2>&1; then
        JOBS="$(nproc)"
    else
        JOBS="1"
    fi
fi

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

"${QMAKE_BIN}" "${PROJECT_FILE}" "CONFIG+=${QMAKE_CONFIG}"
make -j"${JOBS}"

echo "GenesysQtGUI built successfully in ${BUILD_DIR}"
