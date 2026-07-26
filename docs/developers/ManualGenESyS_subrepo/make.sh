#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DOCS_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
OUTPUT_PDF="${DOCS_DIR}/ManualGenESyS.pdf"
SOURCE_PDF="${SCRIPT_DIR}/ManualGenESyS.pdf"

if ! command -v latexmk >/dev/null 2>&1; then
  echo "Error: latexmk is required to build the GenESyS manual." >&2
  exit 1
fi

cd "${SCRIPT_DIR}"
latexmk -cd -xelatex -file-line-error -interaction=errorstopmode -halt-on-error "ManualGenESyS.tex"

if [[ ! -f "${SOURCE_PDF}" ]]; then
  echo "Error: expected PDF was not generated: ${SOURCE_PDF}" >&2
  exit 1
fi

tmp_output="$(mktemp "${DOCS_DIR}/ManualGenESyS.pdf.XXXXXX")"
cp -f "${SOURCE_PDF}" "${tmp_output}"
mv -f "${tmp_output}" "${OUTPUT_PDF}"
echo "Updated ${OUTPUT_PDF}"
