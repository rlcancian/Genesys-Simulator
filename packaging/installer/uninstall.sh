#!/usr/bin/env bash
set -Eeuo pipefail

PREFIX="${HOME:-}/.local/opt/genesys"
NO_DESKTOP=0

while (($#)); do
    case "$1" in
        --prefix) PREFIX=$2; shift 2 ;;
        --no-desktop) NO_DESKTOP=1; shift ;;
        --help|-h) printf 'Usage: ./uninstall.sh [--prefix DIR] [--no-desktop]\n'; exit 0 ;;
        *) echo "Unknown option: $1" >&2; exit 2 ;;
    esac
done

[[ $(id -u) -ne 0 ]] || { echo 'Refusing to uninstall as root.' >&2; exit 2; }
[[ "$PREFIX" = /* ]] || { echo '--prefix must be an absolute path.' >&2; exit 2; }
PREFIX=$(realpath -m -- "$PREFIX")
case "$PREFIX" in
    /|/usr|/usr/*|/opt|/opt/*|/etc|/etc/*) echo "Unsafe system prefix: $PREFIX" >&2; exit 2 ;;
esac
[[ -f "$PREFIX/.genesys-portable-install" ]] || {
    echo "No GenESyS portable installation marker found at $PREFIX" >&2
    exit 1
}
[[ $(stat -c '%u' "$PREFIX") -eq $(id -u) ]] || { echo 'Installation is not owned by the current user.' >&2; exit 1; }

if ((NO_DESKTOP == 0)); then
    DESKTOP_FILE="${XDG_DATA_HOME:-$HOME/.local/share}/applications/io.github.rlcancian.genesys.desktop"
    if [[ -f "$DESKTOP_FILE" ]] && grep -q '^X-GenESyS-Portable=true$' "$DESKTOP_FILE"; then
        rm -f -- "$DESKTOP_FILE"
    fi
fi
rm -rf -- "$PREFIX"
printf 'Removed GenESyS portable runtime from %s; user XDG data and models were preserved.\n' "$PREFIX"
