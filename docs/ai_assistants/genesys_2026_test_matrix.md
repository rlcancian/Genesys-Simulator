# GenESyS 2026 Test Matrix

## 1. Purpose

Record executed software validation, remaining coverage, and acceptance criteria. Passing tests do not by themselves establish numerical, statistical, biochemical, or biological validity.

## 2. Current executed checkpoints

| Checkpoint | Run | Result | Integrated state |
|---|---:|---|---|
| Consolidation documentation | `29779857979` | `tests-unit` and GUI GMDD passed | PR #469 merged as `9c52b615...` |
| Workflow triggers | `29772492281` | `tests-unit` and GUI GMDD passed | PR #470 merged as `5b24c1e4...` |
| AI test aggregation | `29773299985` | aggregate/CTest/GUI passed | PR #471 merged as `1fca8763...` |
| Phase 0 kernel/smoke | `29780136722` | both matrix jobs passed | PR #472 merged as `802b8aec...` |
| PR #472 ordinary CI | `29780136801` | `tests-unit` and GUI passed | workflow addition validated |

## 3. Core baseline matrix

| Validation path | Executed evidence | Status | Remaining gap |
|---|---|---|---|
| `tests-unit` | configure/build/CTest passed | `validated` | preserve on future code changes |
| Focused GUI GMDD | 3/3 passed | `validated` | standalone GUI startup/workflow |
| `tests-kernel-unit` | configure/build/direct runner/CTest passed | `validated` | 4 disabled runtime tests |
| Kernel CTest inventory | 1,696 registered; 1,692 passed; 4 disabled | `validated` | investigate disabled tests |
| AI tests ordinary/direct paths | tests #495–#497 passed | `validated` | provider transport/security coverage |
| `tests-smoke` | 3 registered/executed/passed | `validated` | broader application smoke matrix |
| Debian trigger/path filters | integrated | `implemented` | package lifecycle execution |
| Application presets | targets/presets confirmed | `needs-local-validation` | independent build/startup/workflows |
| Sanitizers/profiling | not executed | `not-started` | ASan/LSan/UBSan/Valgrind/profiling |

## 4. Exact Phase 0 inventory

### 4.1 Kernel

- registered: 1,696;
- executed/passed: 1,692;
- failed: 0;
- disabled: 4;
- CTest real time: 26.90 seconds.

AI tests:

- #495 `AIConversationServiceTest.KeepsIndependentBoundedHistories`;
- #496 `AIPluginTest.BuiltInConnectorExposesSupportAndComponentMetadata`;
- #497 `AIPluginTest.PromptTemplateEvaluatesExpressionsAndEscapesLiteralBraces`.

Disabled runtime tests:

- #225 SearchQueue found/rank/found-port path;
- #226 SearchQueue not-found/zero-rank path;
- #229 Remove single-rank interval path;
- #230 Remove range path.

### 4.2 Smoke

- registered/executed/passed: 3/3/3;
- failed: 0;
- real time: 0.68 seconds;
- tests: simulator start, continuous system, LSODE.

## 5. Detailed module matrix

| Module / concern | Current evidence | Missing coverage | Priority | Acceptance criterion | Status |
|---|---|---|---|---|---|
| Utilities | baseline green | edge cases and sanitizers | P2 | deterministic edge behavior; no findings | `partially-validated` |
| Simulator support/runtime | baseline green | disabled Search/Remove tests; lifecycle/leak paths | P0/P1 | all intended runtime tests enabled and green; no UB/leak | `partially-validated` |
| Persistence | baseline green | cross-plugin and compatibility fixtures | P1 | supported fields round-trip; unsupported diagnosed | `partially-validated` |
| PluginManager | baseline green | ownership, duplicate insertion, future ABI/load/unload | P1 | deterministic lifecycle and diagnostics | `partially-validated` |
| Parser expressions | baseline green | unknown functions and historical compatibility | P1 | deterministic errors and `.gen` compatibility | `partially-validated` |
| FunctionRegistry | historical PR only | current disposition/design | P1 | explicit decision and compatibility tests | `not-started` |
| Kernel statistics | baseline green | authoritative references and degenerate inputs | P0/P1 | declared methods match references/tolerances | `partially-validated` |
| Hypothesis testing | current tests green | suspect integrator and missing references | P0 | reference-backed p-values/intervals | `blocked` scientifically |
| `SolverDefaultImpl1` | source defects confirmed | dedicated characterization/regression tests | P0 | failing tests isolate every defect before repair | `blocked` |
| ODE factory | analytical/convergence tests green | stiffness, long-time, NaN/Inf, event boundaries | P1 | explicit solver contract and reference convergence | `partially-validated` |
| Diffusion/continuous | aggregate and smoke green | conservation, mesh convergence, target overlap, hybrid time | P0/P1 | reference solution and no ODR/link overlap | `blocked` architecturally |
| Cellular automata | current neighborhood tests green | historical full inventory reconciliation | P1 | current/historical scope reconciled | `partially-validated` |
| Whole-cell | current registered suite green | GLPK/fallback, persistence, stochastic reproducibility | P0/P1 | deterministic fixtures in supported modes | `partially-validated` |
| SBML/biochemical | sources/dependency exist | support matrix, round trip, diagnostics | P1 | no silent loss; unsupported constructs explicit | `not-started` |
| Modal/hybrid | domain sources exist | state/time contract fixtures | P1 | deterministic event-boundary behavior | `not-started` |
| AI plugins | ordinary and direct paths green | transport and error/security cases | P1 | offline deterministic coverage and redaction | `partially-validated` |
| AI secret storage | source risk identified | process/Secret Service tests | P1 | secret absent from argv/logs | `not-started` |
| Worker API/tokens | partial tests | auth, quotas, CSPRNG, expiry/rotation | P1 | controlled-intranet security contract | `not-started` hardening |
| Optimizer | scaffold | algorithm/ranking/constraints/ownership | P1 | Level 3 contract and benchmark suite | `partially-implemented` |
| Data Analyser | related tests exist | reference fits/import/export | P1 | reference-backed workflow | `partially-implemented` |
| Do Experiments | backend fragments; GUI absent | DOE specification/tests/workflow | P1/P2 | known designs and analyses match references | `partially-implemented` |
| Shell | preset confirmed | scripted startup/model/exit | P1 | minimal model completes cleanly | `needs-local-validation` |
| Main GUI | focused tests green | standalone startup/model interaction | P1 | target starts; workflow smoke passes | `partially-validated` |
| Independent GUIs | presets confirmed | basic startup/workflows | P1 | each application starts independently | `needs-local-validation` |
| Packaging | triggers corrected | build/install/start/uninstall/version | P2 | packages complete lifecycle | `needs-ci-validation` |
| Qt6-only cleanup | policy decided | remove Qt5 fallback across CMake/tests/scripts | P1/P2 | Qt6 baseline remains green; no fallback discovery | `not-started` |
| Dynamic plugins | ABI direction recorded | target/lifecycle map and pilot | P1/P2 | versioned pilot loads/unloads safely | `deferred` |

## 6. Aggregation findings

Resolved:

- AI plugin executable is part of the ordinary aggregate;
- AI direct-runner target executes through `tests-kernel-unit`;
- exact kernel and smoke inventories are captured.

Still open:

1. four disabled Search/Remove tests;
2. repeated runtime libraries in selected link lists;
3. full/minimal plugin source overlap;
4. GUI test source aggregation/build cost;
5. generated method inventory is structural inventory, not behavioral proof.

## 7. Numerical and scientific evidence policy

Every correctness-sensitive method must define its mathematics, parameterization, domain, units, oracle/reference, tolerances, degenerate behavior, deterministic seeds, dependency mode, and exact/approximate/heuristic classification.

## 8. Next bounded execution

The next code PR should add focused regression tests for `SolverDefaultImpl1` without changing its implementation. Application, package, sanitizer, Qt6 cleanup, plugin architecture, and worker security validation remain separate workstreams.