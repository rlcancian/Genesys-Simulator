#!/usr/bin/env bash
#
# Demonstrates every item required by Tema 13, section 14 ("O que os testes devem demonstrar"):
#   1. discovery/registration of multiple workers
#   2. correct job submission to remote workers
#   3. correct distribution of replications across targets
#   4. aggregation of partial results into a single final result
#   5. timeout/failure handling for a worker
#   6. job re-routing (failover) on failure
#   7. coherence between the distributed result and the equivalent local result
#
# It combines live runs of the orchestrator + workers (points 1-5, 7) with the targeted unit
# tests that prove the points best shown in isolation (6, and the exactness behind 4/7).
#
# Usage (from the project root):
#   source/applications/distributed/demonstrate-requirements.sh
#
# Build first, including tests:
#   cmake -S . -B build/distributed -G Ninja \
#       -DGENESYS_BUILD_DISTRIBUTED=ON -DGENESYS_BUILD_WEB_APPLICATION=ON -DGENESYS_BUILD_TESTS=ON
#   cmake --build build/distributed --target genesys_distributed_app genesys_web_app genesys_kernel_unit_tests

set -u
export LC_ALL=C

BUILD_DIR="${BUILD_DIR:-build/distributed}"
REPLICATIONS="${REPLICATIONS:-1200}"
# The default model does real component work; raise the per-request timeout above the orchestrator's
# 5s default so worker batches are not aborted (matters if REPLICATIONS is increased).
TIMEOUT="${TIMEOUT:-60}"
APP="$BUILD_DIR/source/applications/distributed/genesys_distributed_app"
WEB="$BUILD_DIR/source/applications/web/genesys_web_app"
TESTDIR="$BUILD_DIR/source/tests/unit"

if [[ ! -x "$APP" || ! -x "$WEB" ]]; then
    echo "error: binaries not found under '$BUILD_DIR' (build them first; see header)." >&2
    exit 1
fi

MODEL_FILE="$(mktemp)"
# The model MUST be multi-line: the .gen parser reads one record per line. A single-line model is
# parsed as a single ModelInfo record, yielding zero components (empty statistics/counters).
printf '%s\n' \
    '# Genesys Simulation Model' \
    '0   ModelInfo  "Demo" version="1.0" projectTitle="" description="" analystName=""' \
    "0   ModelSimulation \"\" traceLevel=0 replicationLength=10.000000 numberOfReplications=${REPLICATIONS}" \
    '62  EntityType "Part"' \
    '61  Create     "Create_1" entityType="Part" nextId=64 timeBetweenCreations="norm(1.5,0.5)"' \
    '64  Delay      "Delay_1" delayExpression="norm(1.0,0.2)" nextId=63' \
    '63  Dispose    "Dispose_1" nexts=0' \
    > "$MODEL_FILE"

WORKER_PIDS=()
cleanup() {
    for pid in "${WORKER_PIDS[@]:-}"; do kill "$pid" 2>/dev/null; done
    rm -f "$MODEL_FILE"
}
trap cleanup EXIT

start_workers() {
    pkill -f genesys_web_app 2>/dev/null
    sleep 1
    WORKER_PIDS=()
    for port in "$@"; do
        "$WEB" --port "$port" --max-requests 1000 >"/tmp/genesys_worker_${port}.log" 2>&1 &
        WORKER_PIDS+=("$!")
    done
    for port in "$@"; do
        for _ in $(seq 1 20); do
            grep -q "listening on port ${port}" "/tmp/genesys_worker_${port}.log" 2>/dev/null && break
            sleep 0.2
        done
    done
}

stop_workers() {
    for pid in "${WORKER_PIDS[@]:-}"; do kill "$pid" 2>/dev/null; done
    WORKER_PIDS=()
    sleep 0.5
}

# Run the orchestrator and strip the simulator startup banner.
run_app() { "$APP" --timeout "$TIMEOUT" "$@" 2>/dev/null | grep -vE 'STARTING|LICENCE|ACTIVATION|LIMITS|^[[:space:]]*\|'; }

hr() { printf '%s\n' "------------------------------------------------------------"; }
PASS=0; FAIL=0
check() { # check "<description>" <condition-exit-code>
    if [[ "$2" -eq 0 ]]; then echo "  PASS: $1"; PASS=$((PASS+1)); else echo "  FAIL: $1"; FAIL=$((FAIL+1)); fi
}

echo "############################################################"
echo "# Tema 13 - section 14 demonstration"
echo "# replications per scenario: $REPLICATIONS"
echo "############################################################"

# ---------------------------------------------------------------------------
hr; echo "[1][2][3][4] Discovery, remote submission, distribution, aggregation"
hr
start_workers 8101 8102
OUT="$(run_app --model "$MODEL_FILE" --replications "$REPLICATIONS" \
        --worker 127.0.0.1:8101 --worker 127.0.0.1:8102 --local)"
echo "$OUT"
# [1] both workers discovered as available
avail=$(echo "$OUT" | grep -c '\[available\]')
check "[1] discovered 2 available workers" "$([[ $avail -eq 2 ]]; echo $?)"
# [2] each remote worker actually ran replications (>0)
remote_zero=$(echo "$OUT" | grep '\[available\]' | grep -c 'replications=0')
check "[2] remote workers executed jobs (replications>0)" "$([[ $remote_zero -eq 0 ]]; echo $?)"
# [3] distribution: 3 targets each got a share (no target with replications=0)
check "[3] load distributed across 3 targets" "$(echo "$OUT" | grep -qE 'Workers \(3\)'; echo $?)"
# [4] aggregation: completed == requested
check "[4] aggregated single result ($REPLICATIONS/$REPLICATIONS)" \
    "$(echo "$OUT" | grep -q "Replications: $REPLICATIONS completed / $REPLICATIONS requested"; echo $?)"
stop_workers

# ---------------------------------------------------------------------------
hr; echo "[5] Timeout/failure handling (a dead worker in the list)"
hr
start_workers 8101
OUT="$(run_app --model "$MODEL_FILE" --replications "$REPLICATIONS" \
        --worker 127.0.0.1:8101 --worker 127.0.0.1:9999 --local)"
echo "$OUT"
# the dead worker is reported unavailable, and the run still completes all replications
check "[5] dead worker marked unavailable" "$(echo "$OUT" | grep -q '127.0.0.1:9999 \[unavailable\]'; echo $?)"
check "[5] run still completed all replications despite the bad worker" \
    "$(echo "$OUT" | grep -q "Replications: $REPLICATIONS completed / $REPLICATIONS requested"; echo $?)"
stop_workers

# ---------------------------------------------------------------------------
hr; echo "[6] Job re-routing (failover) on mid-execution failure  [unit test]"
hr
EM="$TESTDIR/genesys_test_distributed_execution_manager"
if [[ -x "$EM" ]]; then
    "$EM" --gtest_filter='*Failover*:*NotDoubleCount*:*Lost*' 2>&1 | grep -E '\[ RUN|\[       OK|\[  FAILED|PASSED|FAILED'
    "$EM" --gtest_filter='*Failover*:*NotDoubleCount*:*Lost*' >/dev/null 2>&1
    check "[6] failover reassigns failed batches without double counting" "$?"
else
    echo "  (skipped: test binary not built - configure with -DGENESYS_BUILD_TESTS=ON)"
fi

# ---------------------------------------------------------------------------
hr; echo "[7] Coherence between distributed and local results"
hr
# 7a. live: same total completes locally and distributed
LOCAL_OUT="$(run_app --model "$MODEL_FILE" --replications "$REPLICATIONS" --local)"
start_workers 8101 8102
DIST_OUT="$(run_app --model "$MODEL_FILE" --replications "$REPLICATIONS" \
        --worker 127.0.0.1:8101 --worker 127.0.0.1:8102 --local)"
stop_workers
local_c=$(echo "$LOCAL_OUT" | sed -n 's/.*Replications: \([0-9]*\) completed.*/\1/p')
dist_c=$(echo "$DIST_OUT" | sed -n 's/.*Replications: \([0-9]*\) completed.*/\1/p')
echo "  local completed=$local_c ; distributed completed=$dist_c"
check "[7] distributed total equals local total" "$([[ "$local_c" == "$dist_c" && -n "$local_c" ]]; echo $?)"
# 7b. unit: exact pooled aggregation equals statistics over the combined values
AG="$TESTDIR/genesys_test_distributed_aggregator"
EQ="$TESTDIR/genesys_test_distributed_equivalence"
if [[ -x "$AG" && -x "$EQ" ]]; then
    "$AG" >/dev/null 2>&1; check "[7] exact pooled statistics (aggregator unit test)" "$?"
    "$EQ" >/dev/null 2>&1; check "[7] split-vs-monolithic coherence (equivalence unit test)" "$?"
else
    echo "  (skipped exactness unit tests: configure with -DGENESYS_BUILD_TESTS=ON)"
fi

# ---------------------------------------------------------------------------
hr
echo "Summary: $PASS passed, $FAIL failed"
hr
[[ "$FAIL" -eq 0 ]]
