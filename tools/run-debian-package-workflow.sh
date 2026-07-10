#!/usr/bin/env bash
set -Eeuo pipefail

BRANCH="WiP20261"
WORKFLOW=".github/workflows/genesys-ci.yml"
OUT=".genesys-ci-artifacts"

die() { echo "ERROR: $*" >&2; exit 1; }
info() { echo; echo "==> $*"; }

command -v git >/dev/null || die "git não encontrado"
command -v gh >/dev/null || die "gh não encontrado"
command -v python3 >/dev/null || die "python3 não encontrado"

cd "$(git rev-parse --show-toplevel 2>/dev/null)" || die "Execute dentro do repositório GenESyS"

info "Garantindo branch ${BRANCH}"
git switch "${BRANCH}"
git fetch origin "${BRANCH}"
git pull --ff-only origin "${BRANCH}"

[[ -f "${WORKFLOW}" ]] || die "Workflow não encontrado: ${WORKFLOW}"
[[ -d debian ]] || die "Diretório debian/ não encontrado"
[[ -f debian/control ]] || die "debian/control não encontrado"
[[ -f debian/rules ]] || die "debian/rules não encontrado"

info "Atualizando job Debian no workflow de forma idempotente"
python3 - <<'PY'
from pathlib import Path
import re

p = Path(".github/workflows/genesys-ci.yml")
s = p.read_text()

if "workflow_dispatch:" not in s:
    raise SystemExit("workflow_dispatch não encontrado; não alterado.")

job = r'''
  debian-package:
    name: Build Debian packages
    runs-on: ubuntu-24.04
    timeout-minutes: 60

    steps:
      - name: Checkout repository
        uses: actions/checkout@v4

      - name: Install Debian packaging dependencies
        run: |
          set -Eeuo pipefail
          sudo apt-get update
          sudo apt-get install -y --no-install-recommends \
            build-essential \
            debhelper \
            devscripts \
            dpkg-dev \
            cmake \
            ninja-build \
            g++ \
            pkgconf \
            qt6-base-dev \
            qt6-base-dev-tools \
            libgl-dev \
            libgtest-dev \
            lintian \
            appstream

      - name: Build Debian binary packages
        run: |
          set -Eeuo pipefail
          dpkg-buildpackage -us -uc -b 2>&1 | tee ../genesys-debian-build.log

      - name: Validate AppStream metadata
        if: success()
        run: |
          set -Eeuo pipefail
          appstreamcli validate --no-net packaging/linux/io.github.rlcancian.genesys.metainfo.xml

      - name: Run lintian
        if: always()
        run: |
          set +e
          lintian ../*.deb 2>&1 | tee ../genesys-lintian.log
          exit 0

      - name: Collect Debian diagnostics
        if: always()
        run: |
          set +e
          mkdir -p ../genesys-debian-diagnostics
          cp -v ../genesys-debian-build.log ../genesys-debian-diagnostics/ 2>/dev/null || true
          cp -v ../genesys-lintian.log ../genesys-debian-diagnostics/ 2>/dev/null || true
          cp -v ../*.build ../genesys-debian-diagnostics/ 2>/dev/null || true
          cp -v ../*.buildinfo ../genesys-debian-diagnostics/ 2>/dev/null || true
          cp -v ../*.changes ../genesys-debian-diagnostics/ 2>/dev/null || true
          cp -v build-debian/CMakeFiles/CMakeOutput.log ../genesys-debian-diagnostics/ 2>/dev/null || true
          cp -v build-debian/CMakeFiles/CMakeError.log ../genesys-debian-diagnostics/ 2>/dev/null || true
          find build-debian -type f \( -name "*.log" -o -name "LastTest.log" \) -exec cp -v {} ../genesys-debian-diagnostics/ \; 2>/dev/null || true
          find .. -maxdepth 2 -type f \( -name "*.deb" -o -name "*.changes" -o -name "*.buildinfo" -o -name "*.build" -o -name "*.log" \) | sort

      - name: Upload Debian packages
        if: success()
        uses: actions/upload-artifact@v4
        with:
          name: genesys-debian-packages
          path: |
            ../genesys-gui_*.deb
            ../genesys-web_*.deb
            ../genesys-simulator_*.buildinfo
            ../genesys-simulator_*.changes

      - name: Upload Debian diagnostics
        if: always()
        uses: actions/upload-artifact@v4
        with:
          name: genesys-debian-diagnostics
          path: ../genesys-debian-diagnostics/
'''

lines = s.splitlines()
out = []
i = 0
while i < len(lines):
    if re.match(r"^  debian-package:\s*$", lines[i]):
        i += 1
        while i < len(lines) and not re.match(r"^  [A-Za-z0-9_-]+:\s*$", lines[i]):
            i += 1
        continue
    out.append(lines[i])
    i += 1

ns = "\n".join(out).rstrip() + "\n" + job + "\n"
if ns != s:
    p.write_text(ns)
PY

info "Commit/push se necessário"
if ! git diff --quiet -- "${WORKFLOW}"; then
  git add "${WORKFLOW}"
  git commit -m "ci: improve Debian package build diagnostics"
  git push origin "${BRANCH}"
else
  echo "Workflow já estava atualizado."
  git push origin "${BRANCH}"
fi

info "Disparando workflow"
gh workflow run genesys-ci.yml --ref "${BRANCH}"
sleep 12

RUN_ID="$(gh run list --workflow genesys-ci.yml --branch "${BRANCH}" --event workflow_dispatch --limit 1 --json databaseId --jq '.[0].databaseId')"
[[ -n "${RUN_ID}" && "${RUN_ID}" != "null" ]] || die "Não consegui obter RUN_ID"

echo "${RUN_ID}" > .genesys-last-run-id
info "RUN_ID=${RUN_ID}"

set +e
gh run watch "${RUN_ID}" --exit-status
STATUS=$?
set -e

rm -rf "${OUT}"
mkdir -p "${OUT}"

set +e
gh run download "${RUN_ID}" --dir "${OUT}" >/dev/null 2>&1
gh run view "${RUN_ID}" --log > "${OUT}/run-${RUN_ID}.log"
set -e

{
  echo "===== FEEDBACK ====="
  echo "BRANCH=${BRANCH}"
  echo "RUN_ID=${RUN_ID}"
  echo "STATUS=${STATUS}"
  echo
  echo "===== RUN SUMMARY ====="
  gh run view "${RUN_ID}" || true
  echo
  echo "===== ERROR SUMMARY ====="
  if [[ -f "${OUT}/run-${RUN_ID}.log" ]]; then
    grep -nEi "error:|fatal:|failed|failure|ctest|dpkg-buildpackage|dh_auto|missing|not found|No such file|CMake Error|FAILED|The following tests FAILED|Errors while running CTest|returned exit code|Process completed" "${OUT}/run-${RUN_ID}.log" | tail -n 200 || true
  else
    echo "Log completo não foi salvo."
  fi
  echo
  echo "===== COLLECTED FILES ====="
  find "${OUT}" -maxdepth 5 -type f | sort || true
} | tee "${OUT}/feedback.txt"

exit "${STATUS}"
