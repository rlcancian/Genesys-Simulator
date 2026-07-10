#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
ROOT_DIR=$(cd -- "${SCRIPT_DIR}/.." && pwd)
OVERRIDES_FILE="${ROOT_DIR}/source/applications/modelSpecific/modelSpecificApps.tsv"
LOG_ROOT="${ROOT_DIR}/build/modelspecific-validation"
LOG_DIR="${LOG_ROOT}/logs"
MODELS_DIR="${ROOT_DIR}/models"
PRESET_NAME="genesys_modelspecific_app"

DRY_RUN=0
VERBOSE=0
LIST_ONLY=0
CLEAN_MODELS=1
KEEP_EXISTING=0
ONLY_FILTER=""
RUN_TIMEOUT_SECONDS=60

usage() {
    cat <<'EOF'
Usage: scripts/regenerate-models-from-modelspecific.sh [options]

Options:
  --help                Show this help text.
  --dry-run             Print the actions without configuring, building, cleaning, or running.
  --clean-models        Remove generated files under models/ before running apps. This is the default.
  --keep-existing       Preserve existing files in models/ and only validate/build/run.
  --only <id>           Restrict to one app by relative .cpp path, path without .cpp, stem, or class name.
  --list                List discovered model-specific apps and exit.
  --timeout <seconds>   Abort each app execution after the given number of seconds. Use 0 to disable. Default: 60.
  --verbose             Stream configure/build/run logs to stdout while also saving them to build/modelspecific-validation/logs.
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --help)
            usage
            exit 0
            ;;
        --dry-run)
            DRY_RUN=1
            shift
            ;;
        --clean-models)
            CLEAN_MODELS=1
            KEEP_EXISTING=0
            shift
            ;;
        --keep-existing)
            KEEP_EXISTING=1
            CLEAN_MODELS=0
            shift
            ;;
        --only)
            if [[ $# -lt 2 ]]; then
                echo "error: --only requires a relative app id" >&2
                exit 2
            fi
            ONLY_FILTER="$2"
            shift 2
            ;;
        --list)
            LIST_ONLY=1
            shift
            ;;
        --timeout)
            if [[ $# -lt 2 || ! "$2" =~ ^[0-9]+$ ]]; then
                echo "error: --timeout requires an integer number of seconds" >&2
                exit 2
            fi
            RUN_TIMEOUT_SECONDS="$2"
            shift 2
            ;;
        --verbose)
            VERBOSE=1
            shift
            ;;
        *)
            echo "error: unknown option '$1'" >&2
            usage >&2
            exit 2
            ;;
    esac
done

discover_apps() {
    python3 - "$ROOT_DIR" "$OVERRIDES_FILE" "$ONLY_FILTER" <<'PY'
from pathlib import Path
import re
import sys

root = Path(sys.argv[1])
overrides_path = Path(sys.argv[2])
only_filter = sys.argv[3].strip()
apps_root = root / "source/applications/modelSpecific"

overrides = {}
if overrides_path.exists():
    for raw_line in overrides_path.read_text().splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue
        fields = line.split("|")
        if len(fields) != 7:
            raise SystemExit(f"Invalid override entry: {raw_line}")
        app, cls, header, should_generate, should_validate, expected_model, note = fields
        overrides[app] = {
            "class": cls,
            "header": header,
            "should_generate": should_generate.lower() == "yes",
            "should_validate": should_validate.lower() == "yes",
            "expected_model": expected_model,
            "note": note,
        }

rows = []
for source in sorted(apps_root.rglob("*.cpp")):
    if source.name == "main.cpp":
        continue

    rel = source.relative_to(apps_root).as_posix()
    header_path = source.with_suffix(".h")
    header_rel = header_path.relative_to(apps_root).as_posix() if header_path.exists() else ""
    class_name = ""
    if header_path.exists():
        match = re.search(
            r'class\s+(\w+)\s*:\s*public\s+BaseGenesysTerminalApplication',
            header_path.read_text(errors="ignore"),
        )
        if match:
            class_name = match.group(1)

    expected_model = ""
    for line in source.read_text(errors="ignore").splitlines():
        stripped = line.strip()
        if stripped.startswith("//"):
            continue
        match = re.search(r'model->save\s*\(\s*"([^"]+)"', stripped)
        if match:
            expected_model = match.group(1)
            break

    row = {
        "app": rel,
        "class": class_name or source.stem,
        "header": header_rel,
        "should_generate": bool(expected_model),
        "should_validate": True,
        "expected_model": expected_model,
        "note": "",
    }

    override = overrides.get(rel)
    if override:
        for key, value in override.items():
            row[key] = value

    rows.append(row)

if only_filter:
    filtered = []
    for row in rows:
        app_no_ext = row["app"][:-4] if row["app"].endswith(".cpp") else row["app"]
        if only_filter in {row["app"], app_no_ext, Path(row["app"]).stem, row["class"]}:
            filtered.append(row)
    rows = filtered

for row in rows:
    fields = [
        row["app"],
        row["class"],
        row["header"],
        "yes" if row["should_generate"] else "no",
        "yes" if row["should_validate"] else "no",
        row["expected_model"],
        row["note"],
    ]
    print("\t".join(fields))
PY
}

mapfile -t APP_ROWS < <(discover_apps)

if [[ ${#APP_ROWS[@]} -eq 0 ]]; then
    echo "No model-specific applications matched the current filter." >&2
    exit 1
fi

if [[ $LIST_ONLY -eq 1 ]]; then
    for row in "${APP_ROWS[@]}"; do
        IFS=$'\t' read -r app class_name header should_generate should_validate expected_model note <<<"$row"
        printf '%-70s class=%-32s generate=%-3s validate=%-3s expected=%s' \
            "$app" "$class_name" "$should_generate" "$should_validate" "${expected_model:-<none>}"
        if [[ -n "$note" ]]; then
            printf ' note=%s' "$note"
        fi
        printf '\n'
    done
    exit 0
fi

mkdir -p "$LOG_DIR"

run_and_capture() {
    local logfile="$1"
    shift

    if [[ $DRY_RUN -eq 1 ]]; then
        printf '[dry-run] %q' "$1"
        shift
        for arg in "$@"; do
            printf ' %q' "$arg"
        done
        printf '\n'
        return 0
    fi

    if [[ $VERBOSE -eq 1 ]]; then
        set +e
        "$@" 2>&1 | tee "$logfile"
        local status=${PIPESTATUS[0]}
        set -e
        return "$status"
    fi

    "$@" >"$logfile" 2>&1
}

clean_selected_outputs() {
    local removed=0
    for row in "${APP_ROWS[@]}"; do
        IFS=$'\t' read -r _app _class _header should_generate _should_validate expected_model _note <<<"$row"
        if [[ "$should_generate" != "yes" || -z "$expected_model" ]]; then
            continue
        fi
        if [[ "$expected_model" != ./models/* ]]; then
            continue
        fi
        local abs_path="${ROOT_DIR}/${expected_model#./}"
        if [[ -f "$abs_path" ]]; then
            echo "Removing stale generated file: ${expected_model}"
            if [[ $DRY_RUN -eq 0 ]]; then
                rm -f -- "$abs_path"
            fi
            removed=1
        fi
    done
    return "$removed"
}

clean_models_directory() {
    echo "Current files under models/ before cleanup:"
    if [[ -d "$MODELS_DIR" ]]; then
        find "$MODELS_DIR" -maxdepth 1 -type f | sort
    fi
    echo "Cleanup scope is limited to regular files under models/."

    if [[ -n "$ONLY_FILTER" ]]; then
        clean_selected_outputs || true
        return
    fi

    while IFS= read -r path; do
        local_name=$(basename "$path")
        case "$local_name" in
            .directory|.gitkeep|README|README.*)
                echo "Preserving ${path#$ROOT_DIR/}"
                ;;
            *)
                echo "Removing ${path#$ROOT_DIR/}"
                if [[ $DRY_RUN -eq 0 ]]; then
                    rm -f -- "$path"
                fi
                ;;
        esac
    done < <(find "$MODELS_DIR" -maxdepth 1 -type f | sort)
}

if [[ ! -d "$MODELS_DIR" ]]; then
    echo "models/ does not exist; creating it."
    if [[ $DRY_RUN -eq 0 ]]; then
        mkdir -p "$MODELS_DIR"
    fi
fi

if [[ $KEEP_EXISTING -eq 0 && $CLEAN_MODELS -eq 1 ]]; then
    clean_models_directory
fi

declare -a PASSED_APPS=()
declare -a FAILED_APPS=()
declare -a GENERATED_MODELS=()
declare -a NON_GENERATORS=()
declare -a SKIPPED_APPS=()

for row in "${APP_ROWS[@]}"; do
    IFS=$'\t' read -r app class_name header should_generate should_validate expected_model note <<<"$row"

    if [[ "$should_validate" != "yes" ]]; then
        SKIPPED_APPS+=("${app} :: ${note:-validation disabled}")
        continue
    fi

    safe_name=${app//\//__}
    safe_name=${safe_name%.cpp}
    configure_log="${LOG_DIR}/${safe_name}.configure.log"
    build_log="${LOG_DIR}/${safe_name}.build.log"
    run_log="${LOG_DIR}/${safe_name}.run.log"

    echo
    echo "=== ${app} ==="
    [[ -n "$note" ]] && echo "note: ${note}"

    configure_cmd=(
        cmake --preset "$PRESET_NAME"
        "-DGENESYS_MODELSPECIFIC_APP=${app}"
        "-DGENESYS_MODELSPECIFIC_APP_CLASS=${class_name}"
        "-DGENESYS_MODELSPECIFIC_APP_HEADER=${header}"
    )

    if ! run_and_capture "$configure_log" "${configure_cmd[@]}"; then
        FAILED_APPS+=("${app} :: configure failed (log: ${configure_log#$ROOT_DIR/})")
        continue
    fi

    if ! run_and_capture "$build_log" cmake --build --preset "$PRESET_NAME"; then
        FAILED_APPS+=("${app} :: build failed (log: ${build_log#$ROOT_DIR/})")
        continue
    fi

    binary_path="${ROOT_DIR}/build/${PRESET_NAME}/source/applications/modelSpecific/genesys_modelspecific_app"
    if [[ ! -x "$binary_path" ]]; then
        FAILED_APPS+=("${app} :: expected binary not found at ${binary_path#$ROOT_DIR/}")
        continue
    fi

    if [[ "$should_generate" == "yes" && -n "$expected_model" && "$expected_model" == ./models/* ]]; then
        rm -f -- "${ROOT_DIR}/${expected_model#./}"
    fi

    if [[ $DRY_RUN -eq 1 ]]; then
        if [[ $RUN_TIMEOUT_SECONDS -gt 0 ]]; then
            echo "[dry-run] (cd ${ROOT_DIR} && timeout --signal=TERM --kill-after=5s ${RUN_TIMEOUT_SECONDS}s ${binary_path#$ROOT_DIR/})"
        else
            echo "[dry-run] (cd ${ROOT_DIR} && ${binary_path#$ROOT_DIR/})"
        fi
        PASSED_APPS+=("$app")
        if [[ "$should_generate" == "yes" ]]; then
            GENERATED_MODELS+=("${expected_model} :: ${app} [dry-run]")
        else
            NON_GENERATORS+=("${app} :: ${note:-no generated model expected}")
        fi
        continue
    else
        run_cmd=("$binary_path")
        if [[ $RUN_TIMEOUT_SECONDS -gt 0 ]]; then
            run_cmd=(timeout --signal=TERM --kill-after=5s "${RUN_TIMEOUT_SECONDS}s" "${run_cmd[@]}")
        fi
        if [[ $VERBOSE -eq 1 ]]; then
            set +e
            (
                cd "$ROOT_DIR"
                "${run_cmd[@]}"
            ) 2>&1 | tee "$run_log"
            run_status=${PIPESTATUS[0]}
            set -e
        else
            set +e
            (
                cd "$ROOT_DIR"
                "${run_cmd[@]}"
            ) >"$run_log" 2>&1
            run_status=$?
            set -e
        fi
        if [[ $run_status -ne 0 ]]; then
            if [[ "$should_generate" == "yes" && -n "$expected_model" && -f "${ROOT_DIR}/${expected_model#./}" ]]; then
                GENERATED_MODELS+=("${expected_model} :: ${app} [generated before validation failure]")
            fi
            if [[ $run_status -eq 124 ]]; then
                FAILED_APPS+=("${app} :: run timed out after ${RUN_TIMEOUT_SECONDS}s (log: ${run_log#$ROOT_DIR/})")
            else
                FAILED_APPS+=("${app} :: run failed with exit ${run_status} (log: ${run_log#$ROOT_DIR/})")
            fi
            continue
        fi
    fi

    PASSED_APPS+=("$app")
    if [[ "$should_generate" == "yes" ]]; then
        if [[ -n "$expected_model" && -f "${ROOT_DIR}/${expected_model#./}" ]]; then
            GENERATED_MODELS+=("$expected_model :: ${app}")
        else
            FAILED_APPS+=("${app} :: expected generated file missing (${expected_model})")
        fi
    else
        NON_GENERATORS+=("${app} :: ${note:-no generated model expected}")
    fi
done

echo
echo "=== Summary ==="
echo "Validated apps: ${#PASSED_APPS[@]}"
echo "Failed apps: ${#FAILED_APPS[@]}"
echo "Generated models: ${#GENERATED_MODELS[@]}"
echo "Non-generators: ${#NON_GENERATORS[@]}"
echo "Skipped apps: ${#SKIPPED_APPS[@]}"

if [[ $DRY_RUN -eq 1 ]]; then
    echo "Dry-run only: no files were removed, built, or executed."
fi

if [[ ${#GENERATED_MODELS[@]} -gt 0 ]]; then
    echo
    echo "Generated models:"
    printf '  %s\n' "${GENERATED_MODELS[@]}"
fi

if [[ ${#NON_GENERATORS[@]} -gt 0 ]]; then
    echo
    echo "Validated apps without model generation:"
    printf '  %s\n' "${NON_GENERATORS[@]}"
fi

if [[ ${#FAILED_APPS[@]} -gt 0 ]]; then
    echo
    echo "Failures:"
    printf '  %s\n' "${FAILED_APPS[@]}"
    exit 1
fi
