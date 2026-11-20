#!/usr/bin/env bash
set -Eeuo pipefail

PACKAGE_ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
PREFIX="${HOME:-}/.local/opt/genesys"
NO_DESKTOP=0
DRY_RUN=0

usage() {
    cat <<'EOF'
Usage: ./install.sh [--prefix DIR] [--no-desktop] [--dry-run] [--version]

Installs the bundled GenESyS portable runtime without root or network access.
The runtime uses XDG user data for Launcher-managed versions; --prefix only
selects the stable bootstrap and fallback runtime location.
EOF
}

while (($#)); do
    case "$1" in
        --prefix) PREFIX=$2; shift 2 ;;
        --no-desktop) NO_DESKTOP=1; shift ;;
        --dry-run) DRY_RUN=1; shift ;;
        --version) sed -n 's/.*"version": "\([^"]*\)".*/\1/p' "$PACKAGE_ROOT/PORTABLE-MANIFEST.json"; exit 0 ;;
        --help|-h) usage; exit 0 ;;
        *) echo "Unknown option: $1" >&2; usage >&2; exit 2 ;;
    esac
done

[[ $(id -u) -ne 0 ]] || { echo 'Refusing to install as root.' >&2; exit 2; }
[[ -n "${HOME:-}" && "$HOME" = /* ]] || { echo 'HOME must be an absolute path.' >&2; exit 2; }
[[ "$PREFIX" = /* ]] || { echo '--prefix must be an absolute path.' >&2; exit 2; }
PREFIX=$(realpath -m -- "$PREFIX")
case "$PREFIX" in
    /|/usr|/usr/*|/opt|/opt/*|/etc|/etc/*) echo "Unsafe system prefix: $PREFIX" >&2; exit 2 ;;
esac
[[ ! -L "$PREFIX" ]] || { echo 'Refusing a symlink prefix.' >&2; exit 2; }

[[ -f "$PACKAGE_ROOT/PORTABLE-MANIFEST.json" && -f "$PACKAGE_ROOT/sha256sums" ]] || {
    echo 'Package metadata is missing.' >&2; exit 1;
}
(cd "$PACKAGE_ROOT" && sha256sum -c sha256sums >/dev/null)

PARENT=$(dirname -- "$PREFIX")
mkdir -p -- "$PARENT"
[[ -w "$PARENT" ]] || { echo "Prefix parent is not writable: $PARENT" >&2; exit 1; }
if [[ -e "$PREFIX" ]]; then
    [[ -f "$PREFIX/.genesys-portable-install" ]] || {
        echo "Refusing to overwrite a directory not created by GenESyS: $PREFIX" >&2
        exit 1
    }
    [[ $(stat -c '%u' "$PREFIX") -eq $(id -u) ]] || {
        echo "Prefix is not owned by the current user: $PREFIX" >&2
        exit 1
    }
fi

if ((DRY_RUN)); then
    printf 'Would install GenESyS %s into %s\n' "$(sed -n 's/.*"version": "\([^"]*\)".*/\1/p' "$PACKAGE_ROOT/PORTABLE-MANIFEST.json")" "$PREFIX"
    exit 0
fi

STAGE=$(mktemp -d "$PARENT/.genesys-portable-stage.XXXXXX")
BACKUP="$PARENT/.genesys-portable-backup.$$.${RANDOM}"
OLD_MOVED=0
NEW_MOVED=0
DESKTOP_STAGE=""
cleanup() {
    local status=$?
    [[ -z "$DESKTOP_STAGE" ]] || rm -f -- "$DESKTOP_STAGE" 2>/dev/null || true
    if ((status != 0)); then
        rm -rf -- "$STAGE" 2>/dev/null || true
        if ((NEW_MOVED)) && [[ -e "$PREFIX" ]]; then rm -rf -- "$PREFIX"; fi
        if ((OLD_MOVED)) && [[ -e "$BACKUP" && ! -e "$PREFIX" ]]; then mv -- "$BACKUP" "$PREFIX"; fi
    else
        rm -rf -- "$STAGE" "$BACKUP" 2>/dev/null || true
    fi
}
trap cleanup EXIT

cp -a -- "$PACKAGE_ROOT/payload/." "$STAGE/"
printf 'GenESyS portable installation\nversion=%s\ncommit=%s\n' \
    "$(sed -n 's/.*"version": "\([^"]*\)".*/\1/p' "$PACKAGE_ROOT/PORTABLE-MANIFEST.json")" \
    "$(sed -n 's/.*"commit": "\([^"]*\)".*/\1/p' "$PACKAGE_ROOT/PORTABLE-MANIFEST.json")" > "$STAGE/.genesys-portable-install"

if [[ -e "$PREFIX" ]]; then
    mv -- "$PREFIX" "$BACKUP"
    OLD_MOVED=1
fi
mv -- "$STAGE" "$PREFIX"
NEW_MOVED=1

if ((NO_DESKTOP == 0)); then
    DATA_HOME=${XDG_DATA_HOME:-$HOME/.local/share}
    DESKTOP_DIR="$DATA_HOME/applications"
    DESKTOP_FILE="$DESKTOP_DIR/io.github.rlcancian.genesys.desktop"
    mkdir -p -- "$DESKTOP_DIR"
    if [[ -e "$DESKTOP_FILE" ]] && ! grep -q '^X-GenESyS-Portable=true$' "$DESKTOP_FILE"; then
        echo "Refusing to overwrite non-GenESyS desktop entry: $DESKTOP_FILE" >&2
        exit 1
    fi
    DESKTOP_STAGE=$(mktemp "$DESKTOP_DIR/.io.github.rlcancian.genesys.desktop.XXXXXX")
    {
        cat "$PREFIX/share/applications/io.github.rlcancian.genesys.desktop"
        printf 'X-GenESyS-Portable=true\n'
    } > "$DESKTOP_STAGE"
    mv -f -- "$DESKTOP_STAGE" "$DESKTOP_FILE"
fi

printf 'Installed GenESyS portable runtime in %s\n' "$PREFIX"
printf 'Add %s/bin to PATH to use the public commands.\n' "$PREFIX"
