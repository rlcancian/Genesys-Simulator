# GenESyS 2026 Test Matrix

## 1. Purpose

Record executed validation, missing coverage, and acceptance criteria for the 2026 consolidation. A green software test does not by itself establish numerical, statistical, biochemical, or biological validity.

## 2. Current executed evidence

| Checkpoint | Run | Result | Integrated state |
|---|---:|---|---|
| PR #469 documentation baseline | `29772903692` | `tests-unit` and GUI GMDD passed | documentation PR still open |
| PR #470 workflow triggers | `29772492281` | `tests-unit` and GUI GMDD passed | merged as `5b24c1e4...` |
| PR #471 AI test aggregation | `29773299985` | aggregate build, CTest, and GUI GMDD passed | merged as `1fca8763...` |

## 3. Baseline matrix

| Validation path | Current evidence | Status | Remaining gap |
|---|---|---|---|
| `tests-unit` configure/build/CTest | multiple current successful PR runs | `validated` for executed commits | exact test count and complete inventory |
| Three focused GUI GMDD tests | 3/3 passed in current runs | `validated` | standalone GUI startup |
| AI plugin tests in ordinary aggregate | PR #471 merged; CTest passed | `validated` | none for ordinary path |
| AI plugin tests in direct kernel runner | run target and dependency merged | `partially-validated` | execute `tests-kernel-unit` explicitly |
| `tests-kernel-unit` | preset/graph confirmed | `needs-local-validation` | configure/build/CTest result |
| `tests-smoke` | preset/sources confirmed | `needs-local-validation` | configure/build/CTest result |
| Debian trigger/path filters | PR #470 merged | `implemented` | package workflow execution/lifecycle |
| Application presets | presets/targets confirmed | `needs-local-validation` | independent build/startup |
| Sanitizers | no current executed presets | `not-started` | ASan/LSan/UBSan/Valgrind |

## 4. Detailed module matrix

| Module / concern | Current evidence | Missing coverage | Priority | Acceptance criterion | Status |
|---|---|---|---|---|---|
| Utilities | ordinary baseline green | edge cases and sanitizer coverage | P2 | deterministic edge behavior, no sanitizer findings | `partially-validated` |
| Simulator support/runtime | ordinary baseline green | repeated lifecycle, exception paths, leak/UB checks | P0/P1 | repeatable create/destroy with no leak/UB | `partially-validated` |
| Persistence | current tests green | cross-plugin round trip and compatibility fixtures | P1 | supported fields round-trip; unsupported fields diagnosed | `partially-validated` |
| PluginManager | current tests green | ownership injection, duplicate insertion, load/unload, future ABI mismatch | P1 | deterministic lifecycle and diagnostics | `partially-validated` |
| Parser expressions | current tests green | unknown-function and old-model compatibility | P1 | deterministic errors and `.gen` compatibility | `partially-validated` |
| FunctionRegistry | historical PR only | current disposition/design | P1 | explicit decision plus compatibility tests | `not-started` |
| Kernel statistics | ordinary baseline green | authoritative references and degenerate inputs | P0/P1 | results agree with declared references/tolerances | `partially-validated` |
| Hypothesis testing | no current failure | legacy integrator dependency and missing reference vectors | P0 | authoritative p-values/intervals and invalid-domain handling | `blocked` scientifically |
| `SolverDefaultImpl1` | source defects confirmed | dedicated tests for every overload/precondition | P0 | failing regressions first, then minimum correction | `blocked` |
| ODE solver factory | analytical/convergence tests exist | stiffness, long-time, event-boundary, NaN/Inf | P1 | documented contract and analytical convergence | `partially-validated` |
| Diffusion/continuous integration | current aggregate green | conservation, mesh convergence, target overlap, hybrid time | P0/P1 | reference solutions and no ODR/link overlap | `blocked` architecturally |
| Cellular automata | neighborhood target green | historical full inventory reconciliation | P1 | current and historical test scope reconciled | `partially-validated` |
| Whole-cell plugins | no failure in current ordinary run | explicit inclusion evidence, GLPK/fallback, reproducibility | P0/P1 | deterministic fixtures pass in supported modes | `partially-validated` |
| SBML/biochemical | sources/dependency exist | support matrix, round trip, diagnostics | P1 | no silent loss and explicit unsupported constructs | `not-started` |
| Modal/hybrid | domain sources exist | deterministic state/time fixtures | P1 | explicit time/state contracts pass | `not-started` |
| AI plugins | offline tests now aggregated and green | explicit kernel preset runner execution | P1 | ordinary and direct-runner paths both pass | `partially-validated` |
| AI provider clients | provider code compiles | fake transport, timeout, parsing, redaction | P1 | deterministic offline tests, no real API requirement | `needs-local-validation` |
| AI secret storage | source risk identified | process/Secret Service boundary tests | P1 | secret absent from argv/logs; safe failure | `not-started` |
| Worker API/token service | partial router tests; token source inspected | auth, quotas, timeout, CSPRNG, expiry/rotation | P1 | secure controlled-intranet contract | `not-started` security hardening |
| Optimizer | scaffold source exists | algorithm, ranking, constraints, ownership | P1 | Level 3 contract and benchmark suite | `partially-implemented` |
| Data Analyser | related tests/targets exist | reference fits and import/export | P1 | reference-backed analysis workflow | `partially-implemented` |
| Do Experiments | backend fragments; GUI absent | DOE specification, backend tests, workflow | P1/P2 | known designs/analysis match references | `partially-implemented` |
| Shell | target/preset confirmed | scripted startup/model/exit | P1 | minimal command/model completes and exits cleanly | `needs-local-validation` |
| Main GUI | focused GMDD tests green | standalone configure/build/startup/model interaction | P1 | target starts and focused tests remain green | `partially-validated` |
| Independent GUIs | targets/presets confirmed | startup and minimal workflows | P1 | each starts independently and completes a basic flow | `needs-local-validation` |
| Packaging | workflow/assets exist; trigger corrected | build/install/start/uninstall/version | P2 | packages generated and lifecycle passes | `needs-ci-validation` |
| Qt6-only cleanup | policy decided | remove Qt5 fallback across GUI/tests/scripts | P1/P2 | Qt6 build/tests pass; no Qt5 discovery remains | `not-started` |
| Dynamic plugins | future ABI direction recorded | non-overlapping targets, lifecycle, ABI tests | P1/P2 | pilot plugin loads/unloads with version checks | `deferred` |

## 5. Current aggregation findings

Resolved:

- `genesys_test_ai_plugins` now participates in the ordinary aggregate.
- `genesys_test_ai_plugins_run` is attached to the kernel direct-runner graph.

Still open:

1. capture exact `ctest -N` counts;
2. execute `tests-kernel-unit` to prove the direct runner;
3. investigate repeated runtime libraries in some link lists;
4. map full/minimal plugin source overlap;
5. avoid treating generated method inventory as behavioral coverage.

## 6. Numerical and scientific evidence policy

For correctness-sensitive algorithms, record:

- mathematical definition and parameterization;
- valid domain, units, and preconditions;
- analytical invariant or independent comparator;
- authoritative reference;
- absolute/relative tolerances and rationale;
- empty, small, degenerate, repeated, and non-finite cases;
- deterministic seeds and stochastic replication policy;
- optional dependency mode;
- exact/asymptotic/approximate/heuristic classification.

## 7. Next bounded execution

```bash
cmake --preset tests-kernel-unit
cmake --build --preset tests-kernel-unit --parallel "$(nproc)"
ctest --preset tests-kernel-unit --output-on-failure

cmake --preset tests-smoke
cmake --build --preset tests-smoke --parallel "$(nproc)"
ctest --preset tests-smoke --output-on-failure

ctest --preset tests-unit -N
ctest --preset tests-kernel-unit -N
ctest --preset tests-smoke -N
```

Do not begin broad refactoring merely because the current ordinary checkpoint is green.