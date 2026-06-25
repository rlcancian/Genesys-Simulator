#!/usr/bin/env bash
#
# Runs the GenESyS distributed-simulation demo end to end: a local-only baseline and a
# distributed run across local workers, then prints a clean side-by-side comparison.
# The simulator's startup banner (LICENCE, LIMITS, etc.) is filtered out.
#
# Usage (from the project root):
#   source/applications/distributed/run-distributed-demo.sh [replications]
#
# Environment overrides:
#   BUILD_DIR     build directory                 (default: build/distributed)
#   REPLICATIONS  total replications              (default: 120000, or $1)
#   WORKER_PORTS  space-separated worker ports    (default: "8101 8102")
#   MODEL         model file to use               (default: a generated temp model)
#
# Note: this script runs `pkill -f genesys_web_app` to free the worker ports, which will
# also stop any unrelated genesys_web_app instances you may have running.

set -u

# Use a fixed numeric locale so awk/printf emit dot decimals (locales with comma decimals
# would otherwise produce "3,84" and break the numeric comparison below).
export LC_ALL=C

REPLICATIONS="${1:-${REPLICATIONS:-120000}}"
BUILD_DIR="${BUILD_DIR:-build/distributed}"
WORKER_PORTS="${WORKER_PORTS:-8101 8102}"

APP="$BUILD_DIR/source/applications/distributed/genesys_distributed_app"
WEB="$BUILD_DIR/source/applications/web/genesys_web_app"

if [[ ! -x "$APP" || ! -x "$WEB" ]]; then
    echo "error: binaries not found under '$BUILD_DIR'." >&2
    echo "Build them first:" >&2
    echo "  cmake -S . -B $BUILD_DIR -G Ninja -DGENESYS_BUILD_DISTRIBUTED=ON -DGENESYS_BUILD_WEB_APPLICATION=ON" >&2
    echo "  cmake --build $BUILD_DIR --target genesys_distributed_app genesys_web_app" >&2
    exit 1
fi

# Model: use $MODEL if provided, otherwise generate a temporary minimal model.
CLEAN_MODEL=0
if [[ -n "${MODEL:-}" ]]; then
    MODEL_FILE="$MODEL"
else
    MODEL_FILE="$(mktemp)"
    CLEAN_MODEL=1
    printf '0   ModelInfo  "DistributedDemo" version="1.0" projectTitle="" description="" analystName="" 0   ModelSimulation "" traceLevel=0 replicationLength=10.000000 numberOfReplications=%s 63  Create "Create_1" entityType="entitytype" nextId=73 73  Dispose "Dispose_1" nexts=0\n' "$REPLICATIONS" > "$MODEL_FILE"
fi

WORKER_PIDS=()
cleanup() {
    for pid in "${WORKER_PIDS[@]:-}"; do kill "$pid" 2>/dev/null; done
    [[ "$CLEAN_MODEL" == "1" ]] && rm -f "$MODEL_FILE"
}
trap cleanup EXIT

# Keep only the comparison-relevant summary lines (drop the startup banner).
filter() { grep -E 'Replications:|Statistics \(|Counters \(|Failures \(|^[[:space:]]+- '; }

LAST=""
run_case() {
    local start end out
    start=$(date +%s.%N)
    out="$("$APP" "$@" 2>/dev/null)"
    end=$(date +%s.%N)
    echo "$out" | filter
    LAST=$(awk "BEGIN{printf \"%.4f\", $end - $start}")
    awk "BEGIN{printf \"  wall-clock: %.2fs\n\", $end - $start}"
}

NUM_WORKERS=$(echo "$WORKER_PORTS" | wc -w | tr -d ' ')

echo "=== GenESyS distributed simulation demo ==="
echo "replications : $REPLICATIONS"
echo "workers      : $WORKER_PORTS  (+ local engine)"
echo

echo "--- Local only (1 engine) ---"
run_case --model "$MODEL_FILE" --replications "$REPLICATIONS" --local
LOCAL_TIME="$LAST"
echo

# Start the workers on free ports, redirecting their output to log files.
pkill -f genesys_web_app 2>/dev/null
sleep 1
WORKER_ARGS=()
for port in $WORKER_PORTS; do
    "$WEB" --port "$port" --max-requests 1000 >"/tmp/genesys_worker_${port}.log" 2>&1 &
    WORKER_PIDS+=("$!")
    WORKER_ARGS+=(--worker "127.0.0.1:${port}")
done
# Wait until each worker is listening (or give up after ~4s).
for port in $WORKER_PORTS; do
    for _ in $(seq 1 20); do
        grep -q "listening on port ${port}" "/tmp/genesys_worker_${port}.log" 2>/dev/null && break
        sleep 0.2
    done
done

echo "--- Distributed (${NUM_WORKERS} workers + local) ---"
run_case --model "$MODEL_FILE" --replications "$REPLICATIONS" "${WORKER_ARGS[@]}" --local
DIST_TIME="$LAST"
echo

echo "=== Comparison ==="
awk "BEGIN{
    printf \"local       : %.2fs\n\", $LOCAL_TIME;
    printf \"distributed : %.2fs (%d workers + local)\n\", $DIST_TIME, $NUM_WORKERS;
    if ($DIST_TIME > 0) printf \"speedup     : %.2fx\n\", $LOCAL_TIME / $DIST_TIME;
}"
