#!/usr/bin/env bash
set -Eeuo pipefail

WORKFLOW="genesys-debian-package.yml"
BRANCH="${1:-$(git branch --show-current 2>/dev/null || true)}"
OUT="${2:-.genesys-debian-artifacts}"

die() { echo "ERROR: $*" >&2; exit 1; }
info() { echo; echo "==> $*"; }

command -v git >/dev/null || die "git not found"
command -v gh >/dev/null || die "GitHub CLI (gh) not found"

repo_root="$(git rev-parse --show-toplevel 2>/dev/null)" || die "run this helper inside the GenESyS repository"
cd "${repo_root}"

[[ -n "${BRANCH}" ]] || die "no branch was supplied and the current checkout has no branch name"
[[ -f ".github/workflows/${WORKFLOW}" ]] || die "workflow not found: .github/workflows/${WORKFLOW}"

info "Dispatching ${WORKFLOW} on ${BRANCH}"
gh workflow run "${WORKFLOW}" --ref "${BRANCH}"

# Select only a manual run for the requested branch. Retry for a bounded period
# because the Actions run can take a few seconds to become visible through the API.
run_id=""
for _ in {1..20}; do
    run_id="$(gh run list \
        --workflow "${WORKFLOW}" \
        --branch "${BRANCH}" \
        --event workflow_dispatch \
        --limit 1 \
        --json databaseId \
        --jq '.[0].databaseId // empty')"
    [[ -n "${run_id}" ]] && break
    sleep 1
done
[[ -n "${run_id}" ]] || die "could not resolve the workflow_dispatch run id"

printf '%s\n' "${run_id}" > .genesys-last-debian-run-id
info "Watching run ${run_id}"

set +e
gh run watch "${run_id}" --exit-status
status=$?
set -e

rm -rf "${OUT}"
mkdir -p "${OUT}"

info "Downloading workflow artifacts"
set +e
gh run download "${run_id}" --dir "${OUT}"
download_status=$?
gh run view "${run_id}" --log > "${OUT}/run-${run_id}.log"
set -e

{
    echo "WORKFLOW=${WORKFLOW}"
    echo "BRANCH=${BRANCH}"
    echo "RUN_ID=${run_id}"
    echo "RUN_STATUS=${status}"
    echo "DOWNLOAD_STATUS=${download_status}"
    echo
    gh run view "${run_id}" || true
    echo
    echo "Artifacts downloaded under ${OUT}:"
    find "${OUT}" -maxdepth 4 -type f | sort || true
} | tee "${OUT}/feedback.txt"

exit "${status}"
