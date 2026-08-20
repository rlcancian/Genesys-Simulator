#!/usr/bin/env bash
set -Eeuo pipefail

PACKAGES_DIR="${1:-genesys-debian-packages}"
EVIDENCE_DIR="${2:-genesys-debian-lifecycle-evidence}"
STUDENT="genesysstudent"
DATA_ROOT="/home/${STUDENT}/.local/share/genesys"
RUNTIME_VERSION="9999.0.1"
RUNTIME_ROOT="${DATA_ROOT}/apps/${RUNTIME_VERSION}"
CURRENT_LINK="${DATA_ROOT}/current"
SYSTEM_CONFIG="/etc/genesys/update.conf"

mkdir -p "${EVIDENCE_DIR}" "${EVIDENCE_DIR}/package-contents" "${EVIDENCE_DIR}/package-info"
exec > >(tee -a "${EVIDENCE_DIR}/lifecycle.log") 2>&1

workdir="$(mktemp -d)"
xvfb_pid=""
cleanup() {
    set +e
    if [[ -n "${xvfb_pid}" ]]; then
        kill "${xvfb_pid}" 2>/dev/null || true
    fi
    sudo pkill -u "${STUDENT}" -x genesys-gui 2>/dev/null || true
    sudo pkill -u "${STUDENT}" -x genesys-worker 2>/dev/null || true
    rm -rf "${workdir}"
}
trap cleanup EXIT

fail() {
    echo "ERROR: $*" >&2
    exit 1
}

shopt -s nullglob
debs=("${PACKAGES_DIR}"/*.deb)
[[ ${#debs[@]} -eq 5 ]] || fail "expected exactly 5 Debian packages, found ${#debs[@]}"

expected_packages=(genesys-common genesys-shell genesys-worker genesys-gui genesys-web)
printf '%s\n' "${expected_packages[@]}" > "${EVIDENCE_DIR}/expected-packages.txt"

printf 'package\tversion\tarchitecture\tdepends\n' > "${EVIDENCE_DIR}/package-inventory.tsv"
for deb in "${debs[@]}"; do
    package="$(dpkg-deb -f "${deb}" Package)"
    version="$(dpkg-deb -f "${deb}" Version)"
    architecture="$(dpkg-deb -f "${deb}" Architecture)"
    depends="$(dpkg-deb -f "${deb}" Depends 2>/dev/null || true)"
    printf '%s\t%s\t%s\t%s\n' "${package}" "${version}" "${architecture}" "${depends}" >> "${EVIDENCE_DIR}/package-inventory.tsv"
    dpkg-deb --info "${deb}" > "${EVIDENCE_DIR}/package-info/${package}.txt"
    dpkg-deb --contents "${deb}" > "${EVIDENCE_DIR}/package-contents/${package}.txt"
    mkdir -p "${workdir}/extract/${package}"
    dpkg-deb --extract "${deb}" "${workdir}/extract/${package}"
done

for package in "${expected_packages[@]}"; do
    count="$(awk -F '\t' -v package="${package}" 'NR > 1 && $1 == package { count++ } END { print count + 0 }' "${EVIDENCE_DIR}/package-inventory.tsv")"
    [[ "${count}" -eq 1 ]] || fail "package inventory does not contain exactly one ${package}"
done

common_root="${workdir}/extract/genesys-common"
shell_root="${workdir}/extract/genesys-shell"
worker_root="${workdir}/extract/genesys-worker"
gui_root="${workdir}/extract/genesys-gui"
web_root="${workdir}/extract/genesys-web"

expected_paths=(
    "${common_root}/usr/libexec/genesys/genesys-launcher"
    "${common_root}/usr/libexec/genesys/genesys-dispatch"
    "${common_root}/usr/bin/genesys-mcp"
    "${common_root}/etc/genesys/update.conf"
    "${shell_root}/usr/bin/genesys-shell"
    "${shell_root}/usr/libexec/genesys/system/bin/genesys-shell"
    "${worker_root}/usr/bin/genesys-worker"
    "${worker_root}/usr/bin/genesys-web"
    "${worker_root}/usr/libexec/genesys/system/bin/genesys-worker"
    "${gui_root}/usr/bin/genesys-gui"
    "${gui_root}/usr/libexec/genesys/system/bin/genesys-gui"
    "${gui_root}/usr/share/applications/io.github.rlcancian.genesys.desktop"
    "${gui_root}/usr/share/metainfo/io.github.rlcancian.genesys.metainfo.xml"
    "${gui_root}/usr/share/icons/hicolor/scalable/apps/io.github.rlcancian.genesys.svg"
)
for path in "${expected_paths[@]}"; do
    [[ -e "${path}" ]] || fail "expected package path missing: ${path#${workdir}/extract/}"
done

if [[ -e "${web_root}/usr/bin" || -e "${web_root}/usr/libexec" || -e "${web_root}/etc" ]]; then
    fail "transitional genesys-web package unexpectedly owns executable/configuration payload"
fi
unexpected_web_payload="$(find "${web_root}" \( -type f -o -type l \) -print | grep -v "^${web_root}/usr/share/doc/genesys-web/" || true)"
[[ -z "${unexpected_web_payload}" ]] || fail "transitional genesys-web package owns unexpected payload: ${unexpected_web_payload}"

wrapper_files=(
    "${gui_root}/usr/bin/genesys-gui"
    "${shell_root}/usr/bin/genesys-shell"
    "${worker_root}/usr/bin/genesys-worker"
    "${worker_root}/usr/bin/genesys-web"
    "${common_root}/usr/bin/genesys-mcp"
)
: > "${EVIDENCE_DIR}/wrapper-contracts.txt"
for wrapper in "${wrapper_files[@]}"; do
    [[ -x "${wrapper}" ]] || fail "public wrapper is not executable: ${wrapper}"
    if grep -Eq '\$HOME|~/|\.local/share/genesys' "${wrapper}"; then
        fail "public wrapper contains a user-home path: ${wrapper}"
    fi
    printf '\n===== %s =====\n' "${wrapper#${workdir}/extract/}" >> "${EVIDENCE_DIR}/wrapper-contracts.txt"
    cat "${wrapper}" >> "${EVIDENCE_DIR}/wrapper-contracts.txt"
done

grep -Fq 'exec "/usr/libexec/genesys/genesys-launcher" --app "genesys-gui" --interactive-update' "${gui_root}/usr/bin/genesys-gui" \
    || fail "genesys-gui does not invoke the stable launcher contract"
for app in genesys-shell genesys-worker genesys-web; do
    package_root="${app}"
    [[ "${app}" == "genesys-web" ]] && package_root="genesys-worker"
    grep -Fq "exec \"/usr/libexec/genesys/genesys-dispatch\" --app \"${app}\" --" "${workdir}/extract/${package_root}/usr/bin/${app}" \
        || fail "${app} does not invoke the stable dispatcher contract"
done
grep -Fq 'exec "/usr/libexec/genesys/genesys-dispatch" --app "genesys-mcp" --' "${common_root}/usr/bin/genesys-mcp" \
    || fail "genesys-mcp does not invoke the stable dispatcher contract"

cat "${common_root}/etc/genesys/update.conf" > "${EVIDENCE_DIR}/packaged-update.conf"
grep -Fxq 'enabled=false' "${EVIDENCE_DIR}/packaged-update.conf" || fail "packaged remote updates are not fail-closed"
grep -Fxq 'require_signature=true' "${EVIDENCE_DIR}/packaged-update.conf" || fail "packaged signature policy is not fail-closed"
if grep -Eq '^manifest_url=' "${EVIDENCE_DIR}/packaged-update.conf"; then
    fail "packaged update policy unexpectedly contains a manifest URL"
fi

if grep -R -E '(build-debian|CMakeCache\.txt|build\.ninja|/\.git/|\.o$|\.a$|\.deb$|genesys-debian-diagnostics)' "${EVIDENCE_DIR}/package-contents"; then
    fail "forbidden build/CI artifact found in package contents"
fi

printf '%s\n' "${debs[@]}" | xargs -n1 realpath > "${EVIDENCE_DIR}/package-files.txt"
mapfile -t absolute_debs < "${EVIDENCE_DIR}/package-files.txt"

sudo apt-get update
sudo apt-get install -y --no-install-recommends "${absolute_debs[@]}" 2>&1 | tee "${EVIDENCE_DIR}/install.log"

dpkg-query -W -f='${Package}\t${Version}\t${Architecture}\t${Status}\n' "${expected_packages[@]}" \
    | tee "${EVIDENCE_DIR}/installed-packages.tsv"
for package in "${expected_packages[@]}"; do
    dpkg-query -L "${package}" | sort > "${EVIDENCE_DIR}/installed-files-${package}.txt"
done

dpkg-query -W -f='${Conffiles}\n' genesys-common | tee "${EVIDENCE_DIR}/common-conffiles.txt"
grep -Fq '/etc/genesys/update.conf' "${EVIDENCE_DIR}/common-conffiles.txt" \
    || fail "/etc/genesys/update.conf is not registered as a conffile"

assert_owner() {
    local expected_package="$1"
    local path="$2"
    local owner
    owner="$(dpkg-query -S "${path}" | head -n1 | cut -d: -f1)"
    printf '%s\t%s\n' "${owner}" "${path}" >> "${EVIDENCE_DIR}/ownership-checks.tsv"
    [[ "${owner}" == "${expected_package}" ]] || fail "${path} is owned by ${owner}, expected ${expected_package}"
}
printf 'package\tpath\n' > "${EVIDENCE_DIR}/ownership-checks.tsv"
assert_owner genesys-gui /usr/bin/genesys-gui
assert_owner genesys-shell /usr/bin/genesys-shell
assert_owner genesys-worker /usr/bin/genesys-worker
assert_owner genesys-worker /usr/bin/genesys-web
assert_owner genesys-common /usr/bin/genesys-mcp
assert_owner genesys-common /usr/libexec/genesys/genesys-launcher
assert_owner genesys-common /usr/libexec/genesys/genesys-dispatch
assert_owner genesys-gui /usr/libexec/genesys/system/bin/genesys-gui
assert_owner genesys-shell /usr/libexec/genesys/system/bin/genesys-shell
assert_owner genesys-worker /usr/libexec/genesys/system/bin/genesys-worker
assert_owner genesys-common /etc/genesys/update.conf

if ! id -u "${STUDENT}" >/dev/null 2>&1; then
    sudo useradd --create-home --shell /bin/bash "${STUDENT}"
fi

sudo -u "${STUDENT}" -H timeout 30s /usr/bin/genesys-shell \
    "facade get-version" "plugin count" "exit" \
    > "${EVIDENCE_DIR}/system-shell.log" 2>&1
grep -Fq 'Installed plugins:' "${EVIDENCE_DIR}/system-shell.log" || fail "system fallback shell did not execute"
grep -Fq 'Quiting. Bye.' "${EVIDENCE_DIR}/system-shell.log" || fail "system fallback shell did not exit cleanly"

set +e
sudo -u "${STUDENT}" -H timeout 10s /usr/bin/genesys-mcp > "${EVIDENCE_DIR}/system-mcp.log" 2>&1
mcp_status=$?
set -e
[[ "${mcp_status}" -eq 127 ]] || fail "missing system MCP should return 127, got ${mcp_status}"
grep -Fq 'genesys-mcp is not available' "${EVIDENCE_DIR}/system-mcp.log" || fail "missing MCP diagnostic is not controlled"

Xvfb :99 -screen 0 1280x800x24 -nolisten tcp -ac > "${EVIDENCE_DIR}/xvfb.log" 2>&1 &
xvfb_pid=$!
for _ in {1..30}; do
    if DISPLAY=:99 xdpyinfo >/dev/null 2>&1; then
        break
    fi
    sleep 0.2
done
DISPLAY=:99 xdpyinfo >/dev/null 2>&1 || fail "Xvfb did not become ready"

sudo -u "${STUDENT}" -H env DISPLAY=:99 QT_QPA_PLATFORM=xcb \
    /usr/bin/genesys-gui --no-update > "${EVIDENCE_DIR}/system-gui.log" 2>&1 &
gui_sudo_pid=$!
gui_pids=()
for _ in {1..50}; do
    mapfile -t gui_pids < <(pgrep -u "${STUDENT}" -x genesys-gui || true)
    [[ ${#gui_pids[@]} -ge 1 ]] && break
    sleep 0.2
done
[[ ${#gui_pids[@]} -eq 1 ]] || fail "expected exactly one system genesys-gui process, found ${#gui_pids[@]}"
gui_exe="$(sudo readlink -f "/proc/${gui_pids[0]}/exe")"
echo "${gui_exe}" | tee "${EVIDENCE_DIR}/system-gui-executable.txt"
[[ "${gui_exe}" == "/usr/libexec/genesys/system/bin/genesys-gui" ]] \
    || fail "public GUI did not reach the system fallback executable"
sudo kill -TERM "${gui_pids[0]}"
wait "${gui_sudo_pid}" 2>/dev/null || true
sleep 1
if pgrep -u "${STUDENT}" -x genesys-gui >/dev/null; then
    fail "residual genesys-gui process after bounded test"
fi

run_worker_health() {
    local public_command="$1"
    local label="$2"
    local port
    port="$(python3 - <<'PY'
import socket
s = socket.socket()
s.bind(('127.0.0.1', 0))
print(s.getsockname()[1])
s.close()
PY
)"
    sudo -u "${STUDENT}" -H "${public_command}" --port "${port}" --max-requests 1 \
        > "${EVIDENCE_DIR}/${label}.log" 2>&1 &
    local sudo_pid=$!
    for _ in {1..50}; do
        if ss -ltn | grep -Eq ":${port}[[:space:]]"; then
            break
        fi
        sleep 0.2
    done
    ss -ltn | grep -E ":${port}[[:space:]]" | tee "${EVIDENCE_DIR}/${label}-listener.txt" \
        || fail "${label} did not open the bounded test listener"
    curl --fail --silent --show-error "http://127.0.0.1:${port}/health" \
        | tee "${EVIDENCE_DIR}/${label}-health.json"
    wait "${sudo_pid}"
    grep -Fxq '{"ok":true,"status":"up"}' "${EVIDENCE_DIR}/${label}-health.json" \
        || fail "${label} health body did not match the validated contract"
    sleep 0.2
    if pgrep -u "${STUDENT}" -x genesys-worker >/dev/null; then
        fail "residual genesys-worker process after ${label} bounded request"
    fi
}
run_worker_health /usr/bin/genesys-worker system-worker
run_worker_health /usr/bin/genesys-web system-web-compat

snapshot_package_files() {
    local output="$1"
    : > "${output}"
    for package in genesys-common genesys-shell genesys-worker genesys-gui; do
        while IFS= read -r path; do
            if sudo test -f "${path}" && ! sudo test -L "${path}"; then
                sudo sha256sum "${path}"
            fi
        done < <(dpkg-query -L "${package}" | sort -u)
    done | sort -k2 > "${output}"
}

snapshot_package_files "${EVIDENCE_DIR}/package-checksums-before-user-runtime.txt"
sudo cp -a "${SYSTEM_CONFIG}" "${workdir}/update.conf.original"

create_valid_runtime() {
    sudo rm -rf "${DATA_ROOT}/apps"
    sudo rm -f "${CURRENT_LINK}"
    sudo -u "${STUDENT}" -H mkdir -p "${RUNTIME_ROOT}/bin"
    printf '%s\n' "${RUNTIME_VERSION}" | sudo -u "${STUDENT}" -H tee "${RUNTIME_ROOT}/VERSION" >/dev/null
    cat <<JSON | sudo -u "${STUDENT}" -H tee "${RUNTIME_ROOT}/MANIFEST.json" >/dev/null
{"schema_version":1,"product":"genesys-simulator","version":"${RUNTIME_VERSION}","applications":["genesys-gui","genesys-shell","genesys-worker","genesys-web","genesys-mcp"]}
JSON
    for app in genesys-gui genesys-shell genesys-worker genesys-web genesys-mcp; do
        cat <<SCRIPT | sudo -u "${STUDENT}" -H tee "${RUNTIME_ROOT}/bin/${app}" >/dev/null
#!/bin/sh
printf 'USER_RUNTIME:${app}\\n'
printf 'ARGC:%s\\n' "\$#"
for arg in "\$@"; do printf 'ARG:[%s]\\n' "\$arg"; done
SCRIPT
        sudo -u "${STUDENT}" -H chmod 0755 "${RUNTIME_ROOT}/bin/${app}"
    done
    sudo -u "${STUDENT}" -H ln -s "apps/${RUNTIME_VERSION}" "${CURRENT_LINK}"
}

create_valid_runtime
sudo -u "${STUDENT}" -H timeout 15s /usr/bin/genesys-shell 'alpha beta' '--flag=value' \
    > "${EVIDENCE_DIR}/user-runtime-shell.log" 2>&1
grep -Fxq 'USER_RUNTIME:genesys-shell' "${EVIDENCE_DIR}/user-runtime-shell.log" || fail "dispatcher did not select user shell runtime"
grep -Fxq 'ARG:[alpha beta]' "${EVIDENCE_DIR}/user-runtime-shell.log" || fail "dispatcher did not preserve spaced shell argument"
grep -Fxq 'ARG:[--flag=value]' "${EVIDENCE_DIR}/user-runtime-shell.log" || fail "dispatcher did not preserve shell option argument"

sudo -u "${STUDENT}" -H timeout 15s /usr/bin/genesys-worker 'worker arg' > "${EVIDENCE_DIR}/user-runtime-worker.log" 2>&1
grep -Fxq 'USER_RUNTIME:genesys-worker' "${EVIDENCE_DIR}/user-runtime-worker.log" || fail "dispatcher did not select user worker runtime"

sudo -u "${STUDENT}" -H timeout 15s /usr/bin/genesys-web 'legacy arg' > "${EVIDENCE_DIR}/user-runtime-web.log" 2>&1
grep -Fxq 'USER_RUNTIME:genesys-web' "${EVIDENCE_DIR}/user-runtime-web.log" || fail "legacy web wrapper did not select user runtime"

sudo -u "${STUDENT}" -H timeout 15s /usr/bin/genesys-mcp 'mcp arg' > "${EVIDENCE_DIR}/user-runtime-mcp.log" 2>&1
grep -Fxq 'USER_RUNTIME:genesys-mcp' "${EVIDENCE_DIR}/user-runtime-mcp.log" || fail "MCP wrapper did not select user runtime"

sudo -u "${STUDENT}" -H env DISPLAY=:99 QT_QPA_PLATFORM=xcb \
    timeout 15s /usr/bin/genesys-gui --no-update -- 'gui arg' > "${EVIDENCE_DIR}/user-runtime-gui.log" 2>&1
grep -Fxq 'USER_RUNTIME:genesys-gui' "${EVIDENCE_DIR}/user-runtime-gui.log" || fail "launcher did not select user GUI runtime"
grep -Fxq 'ARG:[gui arg]' "${EVIDENCE_DIR}/user-runtime-gui.log" || fail "launcher did not preserve GUI passthrough argument"

cat > "${workdir}/update.conf.policy-override" <<'EOF'
[updates]
enabled=false
channel=stable
allow_user_runtime=false
require_signature=true
check_on_gui_startup=false
fallback_to_system=true
minimum_check_interval_hours=12
max_user_versions=2
EOF
sudo install -m 0644 "${workdir}/update.conf.policy-override" "${SYSTEM_CONFIG}"
sudo -u "${STUDENT}" -H timeout 30s /usr/bin/genesys-shell "facade get-version" "plugin count" "exit" \
    > "${EVIDENCE_DIR}/policy-override-shell.log" 2>&1
if grep -Fq 'USER_RUNTIME:' "${EVIDENCE_DIR}/policy-override-shell.log"; then
    fail "administrative allow_user_runtime=false did not override the active user runtime"
fi
grep -Fq 'Installed plugins:' "${EVIDENCE_DIR}/policy-override-shell.log" || fail "policy override did not select system fallback"
sudo install -m 0644 "${workdir}/update.conf.original" "${SYSTEM_CONFIG}"

run_invalid_runtime_case() {
    local case_name="$1"
    sudo -u "${STUDENT}" -H timeout 30s /usr/bin/genesys-shell "facade get-version" "exit" \
        > "${EVIDENCE_DIR}/invalid-${case_name}.log" 2>&1
    if grep -Fq 'USER_RUNTIME:' "${EVIDENCE_DIR}/invalid-${case_name}.log"; then
        fail "invalid runtime case ${case_name} unexpectedly executed user runtime"
    fi
    grep -Fq 'Quiting. Bye.' "${EVIDENCE_DIR}/invalid-${case_name}.log" \
        || fail "invalid runtime case ${case_name} did not fall back cleanly"
}

create_valid_runtime
sudo rm -f "${CURRENT_LINK}"
run_invalid_runtime_case current-absent

create_valid_runtime
sudo rm -f "${CURRENT_LINK}"
sudo -u "${STUDENT}" -H ln -s 'apps/does-not-exist' "${CURRENT_LINK}"
run_invalid_runtime_case broken-current

create_valid_runtime
sudo rm -f "${RUNTIME_ROOT}/VERSION"
run_invalid_runtime_case version-absent

create_valid_runtime
sudo rm -f "${RUNTIME_ROOT}/MANIFEST.json"
run_invalid_runtime_case manifest-absent

create_valid_runtime
sudo rm -f "${RUNTIME_ROOT}/bin/genesys-shell"
run_invalid_runtime_case app-absent

create_valid_runtime
sudo chmod 0644 "${RUNTIME_ROOT}/bin/genesys-shell"
run_invalid_runtime_case app-not-executable

create_valid_runtime
if sudo test -f "${DATA_ROOT}/logs/launcher.log"; then
    sudo cp "${DATA_ROOT}/logs/launcher.log" "${EVIDENCE_DIR}/launcher-dispatch.log"
    sudo chown "$(id -u):$(id -g)" "${EVIDENCE_DIR}/launcher-dispatch.log"
fi
snapshot_package_files "${EVIDENCE_DIR}/package-checksums-after-user-runtime.txt"
cmp "${EVIDENCE_DIR}/package-checksums-before-user-runtime.txt" "${EVIDENCE_DIR}/package-checksums-after-user-runtime.txt" \
    || fail "package-owned file content changed during user-runtime tests"

: > "${EVIDENCE_DIR}/dpkg-verify.txt"
for package in genesys-common genesys-shell genesys-worker genesys-gui genesys-web; do
    dpkg -V "${package}" | tee -a "${EVIDENCE_DIR}/dpkg-verify.txt"
done
[[ ! -s "${EVIDENCE_DIR}/dpkg-verify.txt" ]] || fail "dpkg -V reported package file changes"

printf '\n# lifecycle-admin-local-change\n' | sudo tee -a "${SYSTEM_CONFIG}" >/dev/null
sudo apt-get install --reinstall -y --no-install-recommends "${absolute_debs[@]}" 2>&1 \
    | tee "${EVIDENCE_DIR}/reinstall.log"
grep -Fxq '# lifecycle-admin-local-change' "${SYSTEM_CONFIG}" \
    || fail "package reinstall overwrote a local administrative conffile modification"
sudo test -L "${CURRENT_LINK}" || fail "package reinstall modified the user's active runtime link"
sudo -u "${STUDENT}" -H timeout 15s /usr/bin/genesys-shell 'after reinstall' > "${EVIDENCE_DIR}/user-runtime-after-reinstall.log" 2>&1
grep -Fxq 'USER_RUNTIME:genesys-shell' "${EVIDENCE_DIR}/user-runtime-after-reinstall.log" \
    || fail "user runtime no longer works after package reinstall"

sudo apt-get remove -y genesys-gui genesys-web genesys-shell genesys-worker genesys-common 2>&1 \
    | tee "${EVIDENCE_DIR}/remove.log"

removed_before_purge=(
    /usr/bin/genesys-gui
    /usr/bin/genesys-shell
    /usr/bin/genesys-worker
    /usr/bin/genesys-web
    /usr/bin/genesys-mcp
    /usr/libexec/genesys/genesys-launcher
    /usr/libexec/genesys/genesys-dispatch
    /usr/libexec/genesys/system/bin/genesys-gui
    /usr/libexec/genesys/system/bin/genesys-shell
    /usr/libexec/genesys/system/bin/genesys-worker
    /usr/share/applications/io.github.rlcancian.genesys.desktop
    /usr/share/metainfo/io.github.rlcancian.genesys.metainfo.xml
    /usr/share/icons/hicolor/scalable/apps/io.github.rlcancian.genesys.svg
)
for path in "${removed_before_purge[@]}"; do
    [[ ! -e "${path}" && ! -L "${path}" ]] || fail "package-owned runtime path remains after remove: ${path}"
done
[[ -f "${SYSTEM_CONFIG}" ]] || fail "conffile did not remain after apt remove"
grep -Fxq '# lifecycle-admin-local-change' "${SYSTEM_CONFIG}" || fail "modified conffile content was lost after apt remove"
sudo test -L "${CURRENT_LINK}" || fail "package remove deleted the user's active runtime link"
sudo test -x "${RUNTIME_ROOT}/bin/genesys-shell" || fail "package remove deleted user runtime data"

# Packages without a conffile are already fully removed (not just in
# "config-files" state) by the preceding "apt-get remove", since apt/dpkg
# have nothing left to preserve for them. A locally-installed package (never
# indexed by an apt repository) that is no longer present in dpkg's status
# database cannot be resolved by name through apt's dependency solver, so
# "apt-get purge" would fail with "Unable to locate package" for those. Use
# dpkg directly: it purges any remaining conffile-bearing package and warns
# (without failing) about names that are already fully removed.
sudo dpkg --purge genesys-gui genesys-web genesys-shell genesys-worker genesys-common 2>&1 \
    | tee "${EVIDENCE_DIR}/purge.log"
[[ ! -e "${SYSTEM_CONFIG}" ]] || fail "administrative conffile remains after purge"
sudo test -L "${CURRENT_LINK}" || fail "package purge removed the user's active runtime link"
sudo test -x "${RUNTIME_ROOT}/bin/genesys-shell" || fail "package purge removed user runtime data"

for package in "${expected_packages[@]}"; do
    if dpkg-query -W -f='${db:Status-Status}' "${package}" 2>/dev/null | grep -Eq '^(installed|config-files)$'; then
        fail "package ${package} remains installed/config-files after purge"
    fi
done

echo 'Debian package lifecycle validation completed successfully.' | tee "${EVIDENCE_DIR}/result.txt"
