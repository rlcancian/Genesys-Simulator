#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${ROOT_DIR}"

if ! rg -q '\\newcommand\{\\figureplaceholder\}' structure.tex; then
  echo "Error: missing \\figureplaceholder macro in structure.tex" >&2
  exit 1
fi

mapfile -t figure_files < <(rg -l '\\begin\{figure\}' --glob '*.tex' . || true)

if [[ "${#figure_files[@]}" -eq 0 ]]; then
  echo "No figure blocks found."
  exit 0
fi

status=0

for file in "${figure_files[@]}"; do
  if ! grep -q 'FIGURE-SPEC-BEGIN' "${file}"; then
    echo "Error: missing FIGURE-SPEC-BEGIN in ${file}" >&2
    status=1
  fi
  if ! grep -q 'FIGURE-SPEC-END' "${file}"; then
    echo "Error: missing FIGURE-SPEC-END in ${file}" >&2
    status=1
  fi
  if ! grep -q '\\caption{' "${file}"; then
    echo "Error: missing \\caption in ${file}" >&2
    status=1
  fi
  if ! grep -q '\\label{' "${file}"; then
    echo "Error: missing \\label in ${file}" >&2
    status=1
  fi
done

while IFS= read -r label; do
  if [[ -z "${label}" ]]; then
    continue
  fi
  if ! rg -F -q "\\ref{${label}}" --glob '*.tex' .; then
    echo "Error: figure label not referenced in prose: ${label}" >&2
    status=1
  fi
done < <(
  perl -0ne 'while (/\\begin\{figure\}.*?\\label\{([^}]+)\}.*?\\end\{figure\}/sg) { print "$1\n" }' "${figure_files[@]}" | sort -u
)

exit "${status}"
