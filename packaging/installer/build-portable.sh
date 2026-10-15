#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
REPO_ROOT=$(cd -- "$SCRIPT_DIR/../.." && pwd)
BUILD_DIR="$REPO_ROOT/build/portable-installer"
OUTPUT_DIR="$SCRIPT_DIR/dist"
JOBS=$(nproc)
VERSION_OVERRIDE=
SKIP_BUILD=0
KEEP_STAGING=0

usage() {
    cat <<'EOF'
Usage: build-portable.sh [options]

Builds an offline Linux x86_64 user-space GenESyS distribution.

Options:
  --version VERSION   Numeric snapshot version accepted by Version::parse.
  --build-dir DIR     Dedicated CMake/Ninja build directory.
  --output-dir DIR    Artifact directory (default: packaging/installer/dist).
  --jobs N             Parallel build jobs.
  --skip-build        Reuse an already-built dedicated build directory.
  --keep-staging      Keep the generated staging tree beside the archive.
  --help              Show this help.
EOF
}

while (($#)); do
    case "$1" in
        --version) VERSION_OVERRIDE=${2:?--version requires a value}; shift 2 ;;
        --build-dir) BUILD_DIR=$2; shift 2 ;;
        --output-dir) OUTPUT_DIR=$2; shift 2 ;;
        --jobs) JOBS=$2; shift 2 ;;
        --skip-build) SKIP_BUILD=1; shift ;;
        --keep-staging) KEEP_STAGING=1; shift ;;
        --help|-h) usage; exit 0 ;;
        *) echo "Unknown option: $1" >&2; usage >&2; exit 2 ;;
    esac
done

case "$(uname -m)" in
    x86_64) ;;
    *) echo "This distribution builder currently supports Linux x86_64 only." >&2; exit 2 ;;
esac

for command_name in cmake ninja tar sha256sum readelf ldd file; do
    command -v "$command_name" >/dev/null || { echo "Missing required command: $command_name" >&2; exit 2; }
done

if [[ -n "$VERSION_OVERRIDE" ]]; then
    [[ "$VERSION_OVERRIDE" =~ ^[0-9]+(\.[0-9]+)*$ ]] || {
        echo "Portable version must contain numeric dotted components: $VERSION_OVERRIDE" >&2
        exit 2
    }
else
    commit_date=$(git -C "$REPO_ROOT" show -s --format=%cI HEAD | cut -dT -f1 | tr -d '-')
    commit_count=$(git -C "$REPO_ROOT" rev-list --count HEAD)
    VERSION_OVERRIDE="${commit_date:0:4}.${commit_date:4:2}.${commit_date:6:2}.${commit_count}"
fi

COMMIT=$(git -C "$REPO_ROOT" rev-parse HEAD)
SHORT_COMMIT=${COMMIT:0:12}
ARCH=$(uname -m)
OS_ID=$( . /etc/os-release; printf '%s' "${ID:-unknown}" )
OS_VERSION=$( . /etc/os-release; printf '%s' "${VERSION_ID:-unknown}" )
COMPILER_VERSION=$(g++ --version | sed -n '1p')
COMPILER_VERSION_JSON=$(printf '%s' "$COMPILER_VERSION" | sed 's/\\/\\\\/g; s/"/\\"/g')
QT_VERSION=$(qtpaths6 --qt-version 2>/dev/null || printf 'unknown')

if ((SKIP_BUILD == 0)); then
    cmake -S "$REPO_ROOT" -B "$BUILD_DIR" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DGENESYS_BUILD_TESTS=OFF \
        -DGENESYS_BUILD_TERMINAL_APPLICATION=ON \
        -DGENESYS_BUILD_WORKER_APPLICATION=ON \
        -DGENESYS_BUILD_GUI_APPLICATION=ON \
        -DGENESYS_BUILD_GUI_GENESYS=ON \
        -DGENESYS_BUILD_GUI_HTTPWORKER=ON \
        -DGENESYS_BUILD_GUI_DATAANALYSER=ON \
        -DGENESYS_BUILD_GUI_OPTIMIZER=ON \
        -DGENESYS_BUILD_GUI_AI_ASSISTANT=ON \
        -DGENESYS_BUILD_LAUNCHER_APPLICATION=ON \
        -DGENESYS_BUILD_LAUNCHER_TESTS=OFF \
        -DGENESYS_ENABLE_PYTHON_INTEGRATION=OFF \
        -DGENESYS_LAUNCHER_VERSION="$VERSION_OVERRIDE" \
        -DGENESYS_SYSTEM_PACKAGE_VERSION="$VERSION_OVERRIDE"
    cmake --build "$BUILD_DIR" --parallel "$JOBS"
fi

require_file() {
    [[ -f "$1" && -x "$1" ]] || { echo "Required executable is missing: $1" >&2; exit 1; }
}

PAYLOAD_BINARIES=(
    "$BUILD_DIR/source/applications/launcher/genesys-launcher"
    "$BUILD_DIR/source/applications/launcher/genesys-dispatch"
    "$BUILD_DIR/source/applications/gui/genesys/genesys-gui"
    "$BUILD_DIR/source/applications/shell/genesys_shell"
    "$BUILD_DIR/source/applications/worker/genesys-worker"
    "$BUILD_DIR/source/applications/gui/httpworker/genesys-httpworker-gui"
    "$BUILD_DIR/source/applications/gui/dataanalyser/genesys-dataanalyser-gui"
    "$BUILD_DIR/source/applications/gui/optimizer/genesys-optimizer-gui"
    "$BUILD_DIR/source/applications/gui/ai_assistant/genesys-ai-assistant-gui"
)
for binary in "${PAYLOAD_BINARIES[@]}"; do require_file "$binary"; done

mkdir -p "$OUTPUT_DIR"
STAGING=$(mktemp -d "$OUTPUT_DIR/.staging.XXXXXX")
cleanup() {
    if ((KEEP_STAGING == 0)); then rm -rf -- "$STAGING"; fi
}
trap cleanup EXIT

PAYLOAD="$STAGING/payload"
SYSTEM_BIN="$PAYLOAD/libexec/genesys/system/bin"
mkdir -p "$PAYLOAD/bin" "$SYSTEM_BIN" "$PAYLOAD/lib" "$PAYLOAD/etc/genesys" \
    "$PAYLOAD/share/genesys/models" "$PAYLOAD/share/applications" \
    "$PAYLOAD/share/icons/hicolor/scalable/apps" "$PAYLOAD/lib/qt6/plugins"

cp -- "$BUILD_DIR/source/applications/launcher/genesys-launcher" "$PAYLOAD/libexec/genesys/genesys-launcher"
cp -- "$BUILD_DIR/source/applications/launcher/genesys-dispatch" "$PAYLOAD/libexec/genesys/genesys-dispatch"
cp -- "$BUILD_DIR/source/applications/gui/genesys/genesys-gui" "$SYSTEM_BIN/genesys-gui"
cp -- "$BUILD_DIR/source/applications/shell/genesys_shell" "$SYSTEM_BIN/genesys-shell"
cp -- "$BUILD_DIR/source/applications/worker/genesys-worker" "$SYSTEM_BIN/genesys-worker"
cp -- "$BUILD_DIR/source/applications/gui/httpworker/genesys-httpworker-gui" "$SYSTEM_BIN/genesys-httpworker-gui"
cp -- "$BUILD_DIR/source/applications/gui/dataanalyser/genesys-dataanalyser-gui" "$SYSTEM_BIN/genesys-dataanalyser-gui"
cp -- "$BUILD_DIR/source/applications/gui/optimizer/genesys-optimizer-gui" "$SYSTEM_BIN/genesys-optimizer-gui"
cp -- "$BUILD_DIR/source/applications/gui/ai_assistant/genesys-ai-assistant-gui" "$SYSTEM_BIN/genesys-ai-assistant-gui"

cat > "$SYSTEM_BIN/genesys-web" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
exec "$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)/genesys-worker" "$@"
EOF
cat > "$SYSTEM_BIN/genesys-mcp" <<'EOF'
#!/usr/bin/env bash
printf '%s\n' 'genesys-mcp is not included as a functional server in this portable distribution.' >&2
printf '%s\n' 'The GenESyS MCP Python entry point requires a separately provisioned MCP environment and Worker.' >&2
exit 127
EOF
chmod 0755 "$SYSTEM_BIN/genesys-web" "$SYSTEM_BIN/genesys-mcp"

cat > "$PAYLOAD/bin/genesys-gui" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
ROOT=$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
export GENESYS_PORTABLE_ROOT="$ROOT"
export LD_LIBRARY_PATH="$ROOT/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export QT_PLUGIN_PATH="$ROOT/lib/qt6/plugins"
export QT_QPA_PLATFORM_PLUGIN_PATH="$ROOT/lib/qt6/plugins/platforms"
exec "$ROOT/libexec/genesys/genesys-launcher" --app genesys-gui --no-update -- "$@"
EOF
for application in genesys-shell genesys-worker genesys-web genesys-mcp; do
    cat > "$PAYLOAD/bin/$application" <<EOF
#!/usr/bin/env bash
set -euo pipefail
ROOT=\$(CDPATH= cd -- "\$(dirname -- "\${BASH_SOURCE[0]}")/.." && pwd)
export GENESYS_PORTABLE_ROOT="\$ROOT"
export LD_LIBRARY_PATH="\$ROOT/lib\${LD_LIBRARY_PATH:+:\$LD_LIBRARY_PATH}"
export QT_PLUGIN_PATH="\$ROOT/lib/qt6/plugins"
export QT_QPA_PLATFORM_PLUGIN_PATH="\$ROOT/lib/qt6/plugins/platforms"
exec "\$ROOT/libexec/genesys/genesys-dispatch" --app $application -- "\$@"
EOF
done
for application in genesys-launcher genesys-dispatch; do
    cat > "$PAYLOAD/bin/$application" <<EOF
#!/usr/bin/env bash
set -euo pipefail
ROOT=\$(CDPATH= cd -- "\$(dirname -- "\${BASH_SOURCE[0]}")/.." && pwd)
export GENESYS_PORTABLE_ROOT="\$ROOT"
export LD_LIBRARY_PATH="\$ROOT/lib\${LD_LIBRARY_PATH:+:\$LD_LIBRARY_PATH}"
export QT_PLUGIN_PATH="\$ROOT/lib/qt6/plugins"
export QT_QPA_PLATFORM_PLUGIN_PATH="\$ROOT/lib/qt6/plugins/platforms"
exec "\$ROOT/libexec/genesys/$application" "\$@"
EOF
done
for application in genesys-httpworker-gui genesys-dataanalyser-gui genesys-optimizer-gui genesys-ai-assistant-gui; do
    cat > "$PAYLOAD/bin/$application" <<EOF
#!/usr/bin/env bash
set -euo pipefail
ROOT=\$(CDPATH= cd -- "\$(dirname -- "\${BASH_SOURCE[0]}")/.." && pwd)
export GENESYS_PORTABLE_ROOT="\$ROOT"
export LD_LIBRARY_PATH="\$ROOT/lib\${LD_LIBRARY_PATH:+:\$LD_LIBRARY_PATH}"
export QT_PLUGIN_PATH="\$ROOT/lib/qt6/plugins"
export QT_QPA_PLATFORM_PLUGIN_PATH="\$ROOT/lib/qt6/plugins/platforms"
exec "\$ROOT/libexec/genesys/system/bin/$application" "\$@"
EOF
done
chmod 0755 "$PAYLOAD/bin"/*

cp -- "$SCRIPT_DIR/install.sh" "$SCRIPT_DIR/uninstall.sh" "$SCRIPT_DIR/test-portable.sh" "$SCRIPT_DIR/README.md" "$STAGING/"
chmod 0755 "$STAGING/install.sh" "$STAGING/uninstall.sh" "$STAGING/test-portable.sh"

cp -a "$REPO_ROOT/models/." "$PAYLOAD/share/genesys/models/"
cp -- "$REPO_ROOT/packaging/linux/update.conf" "$PAYLOAD/etc/genesys/update.conf"
cp -- "$REPO_ROOT/packaging/linux/io.github.rlcancian.genesys.desktop" "$PAYLOAD/share/applications/"
cp -- "$REPO_ROOT/packaging/linux/icons/hicolor/scalable/apps/io.github.rlcancian.genesys.svg" \
    "$PAYLOAD/share/icons/hicolor/scalable/apps/"

APPLICATIONS='["genesys-gui","genesys-shell","genesys-worker","genesys-web","genesys-mcp","genesys-httpworker-gui","genesys-dataanalyser-gui","genesys-optimizer-gui","genesys-ai-assistant-gui"]'
printf '%s\n' "$VERSION_OVERRIDE" > "$SYSTEM_BIN/VERSION"
printf '{"schema_version":1,"product":"genesys-simulator","version":"%s","applications":%s}\n' \
    "$VERSION_OVERRIDE" "$APPLICATIONS" > "$SYSTEM_BIN/MANIFEST.json"

QT_PLUGIN_DIR=$(qtpaths6 --plugin-dir 2>/dev/null || true)
if [[ -z "$QT_PLUGIN_DIR" || ! -d "$QT_PLUGIN_DIR/platforms" ]]; then
    echo 'Qt6 plugin directory/platforms was not found.' >&2
    exit 1
fi
for plugin_group in platforms imageformats iconengines printsupport styles; do
    if [[ -d "$QT_PLUGIN_DIR/$plugin_group" ]]; then
        cp -a "$QT_PLUGIN_DIR/$plugin_group" "$PAYLOAD/lib/qt6/plugins/"
    fi
done

is_glibc_library() {
    case "$(basename -- "$1")" in
        libc.so.*|libm.so.*|libpthread.so.*|libdl.so.*|librt.so.*|libresolv.so.*|libnsl.so.*|libutil.so.*|libcrypt.so.*|libanl.so.*|libmvec.so.*|ld-linux*.so.*|linux-vdso.so.*) return 0 ;;
    esac
    return 1
}

copy_elf_dependencies() {
    local queue="$STAGING/elf-queue"
    : > "$queue"
    find "$PAYLOAD" -type f -perm /111 -print >> "$queue"
    find "$PAYLOAD/lib/qt6/plugins" -type f -name '*.so*' -print >> "$queue"
    declare -A seen=()
    while IFS= read -r elf; do
        [[ -n "$elf" && -z "${seen[$elf]+yes}" ]] || continue
        seen[$elf]=1
        file "$elf" | grep -q 'ELF' || continue
        while IFS= read -r dependency; do
            [[ -n "$dependency" && -f "$dependency" ]] || continue
            is_glibc_library "$dependency" && continue
            local name="$PAYLOAD/lib/$(basename -- "$dependency")"
            if [[ ! -e "$name" ]]; then
                cp -L -- "$dependency" "$name"
                printf '%s\n' "$name" >> "$queue"
            fi
        done < <(ldd "$elf" | awk '$1 ~ /^\// {print $1} $3 ~ /^\// {print $3}')
    done < "$queue"
}
copy_elf_dependencies
rm -f -- "$STAGING/elf-queue"

find "$PAYLOAD" -type f -print0 | sort -z | while IFS= read -r -d '' path; do
    file "$path" | grep -q 'ELF' || continue
    if ldd "$path" 2>&1 | grep -q 'not found'; then
        echo "Unresolved ELF dependency: $path" >&2
        ldd "$path" >&2 || true
        exit 1
    fi
done

cat > "$STAGING/PORTABLE-MANIFEST.json" <<EOF
{
  "schema_version": 1,
  "product": "genesys-simulator",
  "distribution": "portable-user-space",
  "version": "$VERSION_OVERRIDE",
  "commit": "$COMMIT",
  "generated_at_utc": "$(date -u +%Y-%m-%dT%H:%M:%SZ)",
  "architecture": "$ARCH",
  "build_system": "$OS_ID $OS_VERSION",
  "compiler": "$COMPILER_VERSION_JSON",
  "qt": "$QT_VERSION",
  "glibc_baseline": "Ubuntu 24.04 development host; system glibc and graphics drivers are not bundled",
  "applications": $APPLICATIONS,
  "mcp_functional": false,
  "remote_updates_enabled": false,
  "require_signature": true
}
EOF

(cd "$STAGING" && find . -type f ! -name sha256sums -print0 | sort -z | xargs -0 sha256sum > sha256sums)

ARCHIVE="$OUTPUT_DIR/genesys-${VERSION_OVERRIDE}-linux-${ARCH}-portable.tar.gz"
rm -f -- "$ARCHIVE" "$ARCHIVE.sha256"
tar --sort=name --mtime='UTC 1970-01-01' --owner=0 --group=0 --numeric-owner \
    -C "$STAGING" -czf "$ARCHIVE" .
sha256sum "$ARCHIVE" > "$ARCHIVE.sha256"

if ((KEEP_STAGING == 1)); then
    SAVED_STAGING="$OUTPUT_DIR/staging-${VERSION_OVERRIDE}-${SHORT_COMMIT}"
    rm -rf -- "$SAVED_STAGING"
    mv -- "$STAGING" "$SAVED_STAGING"
    STAGING="$SAVED_STAGING"
fi

printf 'Portable archive: %s\n' "$ARCHIVE"
printf 'SHA-256 file:     %s\n' "$ARCHIVE.sha256"
printf 'Version:          %s\n' "$VERSION_OVERRIDE"
printf 'Commit:           %s\n' "$COMMIT"
