#!/usr/bin/env bash
#
# Demonstrates every item required by Tema 13, section 14 ("O que os testes devem demonstrar").
# For each requirement it prints: the requirement text, the live evidence (orchestrator output or
# extracted facts), and a PASS/FAIL verdict.
#
#   1. discovery / registration of multiple workers
#   2. correct job submission to remote workers
#   3. correct distribution of replications across targets
#   4. aggregation of partial results into a single final result
#   5. timeout / failure handling for a worker
#   6. job re-routing (failover) on failure
#   7. coherence between the distributed result and the equivalent local result
#
# Points 1-5 and 7 use live runs of the orchestrator + workers; points 6 and the exactness behind
# 4/7 use the targeted unit tests (failover and pooled-aggregation are best shown in isolation).
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
APP="$BUILD_DIR/source/applications/distributed/genesys_distributed_app"
WEB="$BUILD_DIR/source/applications/web/genesys_web_app"
TESTDIR="$BUILD_DIR/source/tests/unit"

# ---- presentation helpers --------------------------------------------------
if [ -t 1 ]; then
    BOLD=$'\e[1m'; DIM=$'\e[2m'; GREEN=$'\e[32m'; RED=$'\e[31m'; CYAN=$'\e[36m'; YEL=$'\e[33m'; RST=$'\e[0m'
else
    BOLD=; DIM=; GREEN=; RED=; CYAN=; YEL=; RST=
fi

PASS=0; FAIL=0
declare -a RESULTS=()
section() { printf '\n%s\n%s %s\n' "${CYAN}════════════════════════════════════════════════════════════════════════${RST}" \
                                   "${BOLD}${CYAN}▌${RST}" "${BOLD}$1${RST}"; }
req()     { printf '  %sTema 13 §14:%s %s\n' "$DIM" "$RST" "$1"; }
info()    { printf '  %s%s%s\n' "$DIM" "$1" "$RST"; }
evidence(){ printf '%s\n' "$1" | sed 's/^/      /'; }
check()   { # check <exit-code> "<short label>"
    if [ "$1" -eq 0 ]; then printf '  %s✔ PASS%s  %s\n' "$GREEN" "$RST" "$2"; PASS=$((PASS+1)); RESULTS+=("PASS|$2");
    else printf '  %s✘ FAIL%s  %s\n' "$RED" "$RST" "$2"; FAIL=$((FAIL+1)); RESULTS+=("FAIL|$2"); fi
}

if [[ ! -x "$APP" || ! -x "$WEB" ]]; then
    printf '%serror:%s binaries not found under "%s" (build them first; see header).\n' "$RED" "$RST" "$BUILD_DIR" >&2
    exit 1
fi

# ---- model (multi-line: the .gen parser reads one record per line) ----------
MODEL_FILE="$(mktemp)"
printf '%s\n' \
    '# Genesys Simulation Model' \
    '0   ModelInfo  "Demo" version="1.0" projectTitle="" description="" analystName=""' \
    "0   ModelSimulation \"\" traceLevel=0 replicationLength=10.000000 numberOfReplications=${REPLICATIONS}" \
    '62  EntityType "Part"' \
    '61  Create     "Create_1" entityType="Part" nextId=64 timeBetweenCreations="norm(1.5,0.5)"' \
    '64  Delay      "Delay_1" delayExpression="norm(1.0,0.2)" nextId=63' \
    '63  Dispose    "Dispose_1" nexts=0' \
    > "$MODEL_FILE"

# ---- worker lifecycle ------------------------------------------------------
WORKER_PIDS=()
cleanup() { for pid in "${WORKER_PIDS[@]:-}"; do kill "$pid" 2>/dev/null; done; rm -f "$MODEL_FILE"; }
trap cleanup EXIT

start_workers() {
    pkill -f genesys_web_app 2>/dev/null; sleep 1; WORKER_PIDS=()
    for port in "$@"; do
        "$WEB" --port "$port" --max-requests 1000 >"/tmp/genesys_worker_${port}.log" 2>&1 &
        WORKER_PIDS+=("$!")
    done
    for port in "$@"; do
        for _ in $(seq 1 25); do
            grep -q "listening on port ${port}" "/tmp/genesys_worker_${port}.log" 2>/dev/null && break
            sleep 0.2
        done
    done
}
stop_workers() { for pid in "${WORKER_PIDS[@]:-}"; do kill "$pid" 2>/dev/null; done; WORKER_PIDS=(); sleep 0.5; }

# ---- output parsing (the orchestrator's pretty summary, banner stripped) ----
run_app()   { "$APP" "$@" 2>/dev/null | grep -vE 'STARTING|LICENCE|ACTIVATION|LIMITS|^[[:space:]]*\|'; }
worker_lines() { printf '%s\n' "$1" | grep -E '[0-9]+ reps'; }                  # one line per target
completed_of() { printf '%s\n' "$1" | sed -nE 's/.*Replications[[:space:]]+([0-9]+) \/ ([0-9]+).*/\1/p'; }
requested_of() { printf '%s\n' "$1" | sed -nE 's/.*Replications[[:space:]]+([0-9]+) \/ ([0-9]+).*/\2/p'; }
num_stats_of() { printf '%s\n' "$1" | sed -nE 's/.*STATISTICS[^0-9]*([0-9]+).*/\1/p'; }

printf '%s\n' "${BOLD}╔══════════════════════════════════════════════════════════════════════╗${RST}"
printf '%s\n' "${BOLD}║         Tema 13 — Distributed Simulation: requirements demo          ║${RST}"
printf '%s\n' "${BOLD}╚══════════════════════════════════════════════════════════════════════╝${RST}"
printf '  replications per scenario: %s%s%s\n' "$BOLD" "$REPLICATIONS" "$RST"

# ===========================================================================
# Requirements 1-4: one distributed run (2 workers + local) drives all four.
# ===========================================================================
start_workers 8101 8102
OUT="$(run_app --model "$MODEL_FILE" --replications "$REPLICATIONS" \
        --worker 127.0.0.1:8101 --worker 127.0.0.1:8102 --local)"
stop_workers

WLINES="$(worker_lines "$OUT")"
AVAIL=$(printf '%s\n' "$WLINES" | awk '$2=="available"' | wc -l | tr -d ' ')
TARGETS=$(printf '%s\n' "$WLINES" | grep -c .)
ZERO_REMOTE=$(printf '%s\n' "$WLINES" | awk '$2=="available" && $3==0' | wc -l | tr -d ' ')
ZERO_ANY=$(printf '%s\n' "$WLINES" | awk '$3==0' | wc -l | tr -d ' ')
COMPLETED=$(completed_of "$OUT"); REQUESTED=$(requested_of "$OUT"); NSTATS=$(num_stats_of "$OUT")

section "Requirement 1 · Discovery & registration of multiple workers"
req "discover/register the available workers from the configured list."
info "Workers reported by the orchestrator:"
evidence "$WLINES"
info "Available remote workers discovered: ${BOLD}${AVAIL}${RST}${DIM} / 2"
check "$([ "${AVAIL:-0}" -eq 2 ]; echo $?)" "two remote workers discovered and registered as available"

section "Requirement 2 · Correct job submission to remote workers"
req "submit a configured job to each remote worker and run it there."
info "Each available remote worker must report replications > 0 (it actually ran a job)."
check "$([ "${ZERO_REMOTE:-1}" -eq 0 ] && [ "${AVAIL:-0}" -eq 2 ]; echo $?)" "both remote workers executed their submitted jobs"

section "Requirement 3 · Distribution of replications across targets"
req "partition N replications across the targets without overlap."
info "Per-target share (endpoint → replications):"
evidence "$(printf '%s\n' "$WLINES" | awk '{printf "%-18s → %s reps\n", $1, $3}')"
info "Targets that received work: ${BOLD}${TARGETS}${RST}${DIM} (expected 3: 2 remote + local), none with 0."
check "$([ "${TARGETS:-0}" -eq 3 ] && [ "${ZERO_ANY:-1}" -eq 0 ]; echo $?)" "load split across all 3 targets, every target non-empty"

section "Requirement 4 · Aggregation into a single final result"
req "merge the partial results into one unified result (stats + counters + totals)."
info "Aggregated totals: ${BOLD}${COMPLETED}${RST}${DIM} / ${REQUESTED} replications completed."
info "Aggregated statistics collectors in the final result: ${BOLD}${NSTATS}${RST}${DIM}."
info "Final aggregated statistics block:"
evidence "$(printf '%s\n' "$OUT" | awk '/STATISTICS/{p=1} /COUNTERS/{p=0} p')"
check "$([ "${COMPLETED:-0}" = "${REQUESTED:-x}" ] && [ -n "${COMPLETED:-}" ] && [ "${NSTATS:-0}" -gt 0 ]; echo $?)" \
      "single aggregated result: ${COMPLETED}/${REQUESTED} reps and ${NSTATS} pooled collectors"

# ===========================================================================
# Requirement 5: a dead worker in the list must be handled gracefully.
# ===========================================================================
start_workers 8101
OUT5="$(run_app --model "$MODEL_FILE" --replications "$REPLICATIONS" \
        --worker 127.0.0.1:8101 --worker 127.0.0.1:9999 --local)"
stop_workers
W5="$(worker_lines "$OUT5")"
DEAD_UNAVAIL=$(printf '%s\n' "$W5" | awk '$1=="127.0.0.1:9999" && $2=="unavailable"' | wc -l | tr -d ' ')
C5=$(completed_of "$OUT5"); R5=$(requested_of "$OUT5")

section "Requirement 5 · Timeout / failure handling for a worker"
req "a worker that does not respond must not abort the run; it is marked unavailable."
info "Workers (one endpoint, :9999, has nothing listening):"
evidence "$W5"
info "Dead worker flagged unavailable: $([ "${DEAD_UNAVAIL:-0}" -eq 1 ] && echo yes || echo no); run completed ${BOLD}${C5}${RST}${DIM}/${R5}."
check "$([ "${DEAD_UNAVAIL:-0}" -eq 1 ] && [ "${C5:-0}" = "${R5:-x}" ] && [ -n "${C5:-}" ]; echo $?)" \
      "dead worker isolated, all ${R5} replications still completed"

# ===========================================================================
# Requirement 6: failover re-routing (best shown by the execution-manager test).
# ===========================================================================
section "Requirement 6 · Job re-routing (failover) on mid-execution failure"
req "a batch whose worker fails mid-run is reassigned to another target, no double counting."
EM="$TESTDIR/genesys_test_distributed_execution_manager"
if [[ -x "$EM" ]]; then
    info "Running the failover unit tests (failover / no-double-count / lost-batch):"
    evidence "$("$EM" --gtest_filter='*Failover*:*NotDoubleCount*:*Lost*' 2>&1 | grep -E '\[ RUN|\[       OK|\[  FAILED|PASSED|FAILED')"
    "$EM" --gtest_filter='*Failover*:*NotDoubleCount*:*Lost*' >/dev/null 2>&1
    check "$?" "failover reassigns failed batches without double counting (unit test)"
else
    info "(skipped: test binary not built — configure with -DGENESYS_BUILD_TESTS=ON)"
    RESULTS+=("SKIP|failover unit test not built")
fi

# ===========================================================================
# Requirement 7: distributed result is coherent with the local equivalent.
# ===========================================================================
LOCAL_OUT="$(run_app --model "$MODEL_FILE" --replications "$REPLICATIONS" --local)"
start_workers 8101 8102
DIST_OUT="$(run_app --model "$MODEL_FILE" --replications "$REPLICATIONS" \
        --worker 127.0.0.1:8101 --worker 127.0.0.1:8102 --local)"
stop_workers
LC=$(completed_of "$LOCAL_OUT"); DC=$(completed_of "$DIST_OUT")

section "Requirement 7 · Coherence between distributed and local results"
req "the distributed result matches running the same workload locally."
info "Local run completed:       ${BOLD}${LC}${RST}"
info "Distributed run completed: ${BOLD}${DC}${RST}"
check "$([ "${LC:-x}" = "${DC:-y}" ] && [ -n "${LC:-}" ]; echo $?)" "live: distributed total equals local total (${LC} = ${DC})"

AG="$TESTDIR/genesys_test_distributed_aggregator"
EQ="$TESTDIR/genesys_test_distributed_equivalence"
if [[ -x "$AG" && -x "$EQ" ]]; then
    info "Exactness of the aggregation is proven numerically by unit tests:"
    "$AG" >/dev/null 2>&1; check "$?" "pooled statistics equal stats over the combined values (aggregator test)"
    "$EQ" >/dev/null 2>&1; check "$?" "split-vs-monolithic coherence (equivalence test)"
else
    info "(skipped exactness unit tests: configure with -DGENESYS_BUILD_TESTS=ON)"
fi

# ===========================================================================
# Summary table
# ===========================================================================
printf '\n%s\n' "${BOLD}════════════════════════════════════════════════════════════════════════${RST}"
printf '%s\n' "${BOLD}  SUMMARY${RST}"
printf '%s\n' "  ──────────────────────────────────────────────────────────────────────"
for entry in "${RESULTS[@]}"; do
    verdict="${entry%%|*}"; label="${entry#*|}"
    case "$verdict" in
        PASS) printf '  %s✔%s  %s\n' "$GREEN" "$RST" "$label" ;;
        FAIL) printf '  %s✘%s  %s\n' "$RED" "$RST" "$label" ;;
        *)    printf '  %s•%s  %s\n' "$YEL" "$RST" "$label" ;;
    esac
done
printf '%s\n' "  ──────────────────────────────────────────────────────────────────────"
if [ "$FAIL" -eq 0 ]; then
    printf '  %s%d passed, %d failed%s\n' "$GREEN" "$PASS" "$FAIL" "$RST"
else
    printf '  %s%d passed, %d failed%s\n' "$RED" "$PASS" "$FAIL" "$RST"
fi
printf '%s\n' "${BOLD}════════════════════════════════════════════════════════════════════════${RST}"
[ "$FAIL" -eq 0 ]
