# GenESyS runtime lifecycle evidence — 2026-07-21

## 1. Scope

This document records the bounded red/green work completed after the original Phase 0 baseline for:

- legacy solver contract stabilization;
- active Search/Remove runtime coverage;
- Queue statistics lifecycle;
- Station statistics lifecycle;
- Delay statistics lifecycle;
- Resource statistics and accounting lifecycle.

The work was performed against `WorkInProgress` and integrated through independent pull requests. It does not establish release readiness or scientific validity.

## 2. Integrated sequence

| PR | Scope | Merge commit |
|---:|---|---|
| #474 | legacy Simpson quadrature and unsupported derivative boundary | `82af912d369f92d9365536ec5a6ba5ee75f3414d` |
| #475 | active Search/Remove runtime scenarios | `c34c6bb542149787cde7329345a460a40b73befe` |
| #476 | Queue statistics lifecycle | `3f8a343ce5c26c751d3841883d8cb18fbf59c3f1` |
| #478 | Station statistics lifecycle | `96b0cbd503b30a9ab4fede054a50959267e42a4a` |
| #479 | Delay statistics lifecycle | `a14a274bf711dad6be90a829c897ddc146e4fd21` |
| #480 | Resource statistics/accounting lifecycle | `4f98a909d941ac31582205904c597fb345d3527f` |

Tracking issue #477 was closed as completed after PR #480 merged.

## 3. Red checkpoints

### 3.1 Legacy solver

Initial test-only commits caused the new regression executable to fail in ordinary CTest and the kernel direct runner while smoke and GUI remained green.

Confirmed defects included:

- odd composite-Simpson subinterval count;
- incorrect result for `x^2` over `[0,1]` with three configured subintervals;
- duplicated integration loops and non-standard VLAs;
- undefined derivative step-size/state contract;
- silent zero or undefined results from derivative overloads.

### 3.2 Search/Remove

Phase 0 diagnostic run `29784531618` captured four SIGSEGV cases. The failure occurred before Search/Remove behavior because Queue statistics were enabled but collectors had not been created. The focused fixture disabled Queue statistics because they were not the subject under test and drained the asynchronous event calendar before assertions.

### 3.3 Station

The test-only head failed in the kernel direct runner while smoke remained green. `Station::enter()` reached `_cstatNumberInStation` before model-wide related-data creation.

### 3.4 Delay

The test-only head failed in the kernel direct runner while smoke remained green. The fixture supplied EntityType, allocation attribute, output connection, zero-time delay, and event-calendar draining, leaving `_cstatWaitTime` lifecycle as the isolated defect.

### 3.5 Resource

Phase 0 run `29794139286` passed configure and smoke but failed during the test-only kernel direct runner.

Two contracts were broken:

- with statistics enabled, collectors/counters were null before `_createAttachedAttributes()`;
- with statistics disabled, cost counters were still dereferenced unconditionally.

The replication-end callback also remained registered after accounting internals could be cleared.

## 4. Corrections

### 4.1 Legacy solver

- one composite-Simpson implementation shared by all overloads;
- effective subinterval count normalized to at least two and even;
- zero-width interval returns zero;
- non-standard VLAs removed;
- unsupported derivative overloads throw `std::logic_error` rather than returning silent/undefined values.

### 4.2 Search/Remove

- four focused active tests added;
- asynchronous routing explicitly drained before sink assertions;
- no production Search/Remove behavior changed;
- historical disabled duplicates retained temporarily for later local cleanup.

### 4.3 Queue

- idempotent collector creation on first statistics-enabled insertion/removal;
- independent creation of `NumberInQueue` and `TimeInQueue`;
- `_lastTimeNumberInQueueChanged` initialized to `0.0`;
- no collector creation when statistics are disabled.

### 4.4 Station

- idempotent collector creation on first statistics-enabled enter/leave;
- independent creation of `NumberInStation` and `TimeInStation`;
- `_enterIntoStationComponent` initialized to `nullptr`;
- no collector creation when statistics are disabled.

### 4.5 Delay

- idempotent creation of `DelayTime` before the first statistics-enabled dispatch;
- routing, allocation, delay-time calculation, and EntityType behavior preserved;
- no collector creation when statistics are disabled.

### 4.6 Resource

- complete 11-object accounting graph created on first statistics-enabled operation;
- partial state repaired by independent object checks;
- cost-per-use, idle-cost, and busy-cost counters used only when reporting is enabled;
- `getSeizedUtilization()` safe without the total-time counter;
- replication-end handler registered once;
- replication-end callback guarded for disabled statistics, missing internals, null event, and non-positive simulated time;
- capacity, busy count, state, schedules, failures, release handlers, and persistence format preserved.

## 5. Active focused tests

| Range in latest kernel inventory | Coverage |
|---|---|
| #1–#9 | legacy solver quadrature and unsupported derivatives |
| #10–#13 | Search/Remove runtime behavior |
| #14–#16 | Queue statistics lifecycle |
| #17–#18 | Station statistics lifecycle |
| #19–#20 | Delay statistics lifecycle |
| #21–#23 | Resource accounting lifecycle |

## 6. Final validation

Latest validated head before PR #480 merge:

```text
eddca45a8ab0c48b94b2e53f553c18a56729fc99
```

### Ordinary CI

Run `29795009959`:

- `tests-unit` configure: passed;
- aggregate build: passed;
- CTest: passed;
- focused GUI GMDD diagnostics: passed.

### Phase 0

Run `29795009950`:

- kernel configure: passed;
- aggregate/direct runner: passed;
- CTest inventory: passed;
- CTest execution: passed;
- smoke: passed;
- registered: 1,719;
- executed/passed: 1,715;
- disabled: 4;
- failed: 0.

Artifact:

- name: `genesys-phase0-tests-kernel-unit`;
- ID: `8481811952`.

Executed toolchain:

- Ubuntu 24.04 runner;
- CMake 3.31.6;
- Ninja 1.13.2;
- G++ 13.3.0.

## 7. Interpretation of the four disabled tests

The four disabled tests are historical duplicate blocks for the same Search/Remove scenarios now covered by the active focused executable. They remain source-cleanup debt because safe removal from the very large historical test file is better performed in a local checkout.

They must not be reported as four missing Search/Remove behaviors. Equivalent behavior is active, mandatory, and green.

## 8. Remaining risks

- suspected temporary `Model` lifetime/leak in `Simulator::_completePluginsFieldsAndTemplate()`;
- no ASan/LSan/UBSan or Valgrind execution for these paths;
- Resource capacity>1 timing/accounting semantics remain historical debt;
- application startup and end-to-end workflows remain unvalidated;
- Debian package lifecycle remains unvalidated after trigger correction;
- worker security, plugin target overlap/dynamic ABI, optimizer maturity, and scientific reference validation remain separate workstreams.

## 9. Next bounded P0 workstream

Inspect and characterize `Simulator::_completePluginsFieldsAndTemplate()`:

1. map ownership of the temporary `Model` and every exit path;
2. determine whether `ModelManager` or `Simulator` adopts it;
3. add a focused lifetime/leak regression or sanitizer-backed checkpoint where technically feasible;
4. apply the smallest RAII or explicit-destruction correction;
5. validate ordinary CI, GUI GMDD, kernel direct runner/CTest, smoke, and preferably an opt-in LeakSanitizer path.
