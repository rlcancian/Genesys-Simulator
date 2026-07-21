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
| Phase 0 evidence documentation | ordinary CI passed | evidence synchronized | PR #473 merged as `a56b92cc...` |
| Legacy solver contract | `29782025242` | 1,705 registered; 1,701 passed; 4 disabled | PR #474 merged as `82af912d...` |
| Search/Remove active coverage | final ordinary and Phase 0 workflows passed | 4 focused runtime tests active | PR #475 merged as `c34c6bb5...` |
| Queue lifecycle | `29791177003` | 1,712 registered; 1,708 passed; 4 disabled | PR #476 merged as `3f8a343c...` |
| Station lifecycle | `29792383322` | 1,714 registered; 1,710 passed; 4 disabled | PR #478 merged as `96b0cbd5...` |
| Delay lifecycle | `29793447956` | 1,716 registered; 1,712 passed; 4 disabled | PR #479 merged as `a14a274b...` |
| Resource lifecycle/accounting | `29795009950` | 1,719 registered; 1,715 passed; 4 disabled | PR #480 merged as `4f98a909...` |

## 3. Core baseline matrix

| Validation path | Executed evidence | Status | Remaining gap |
|---|---|---|---|
| `tests-unit` | configure/build/CTest passed on every PR #474–#480 | `validated` | preserve on future code changes |
| Focused GUI GMDD | 3/3 passed on every PR #474–#480 | `validated` | standalone GUI startup/workflow |
| `tests-kernel-unit` | configure/build/direct runner/CTest passed | `validated` | 4 historical duplicate tests remain disabled |
| Kernel CTest inventory | 1,719 registered; 1,715 passed; 4 disabled | `validated` | remove or formally retire duplicate disabled blocks |
| AI tests ordinary/direct paths | tests #495–#497 passed in recorded Phase 0 baseline | `validated` | provider transport/security coverage |
| Legacy solver regressions | 9 focused tests passed | `validated` software contract | authoritative numerical use beyond the quarantined API |
| Search/Remove runtime | 4 focused active tests passed | `validated` | remove historical disabled duplicates locally |
| Queue statistics lifecycle | 3 focused tests passed | `validated` | sanitizer coverage |
| Station statistics lifecycle | 2 focused tests passed | `validated` | sanitizer coverage and broader entity-type combinations |
| Delay statistics lifecycle | 2 focused tests passed | `validated` | nonzero-delay and allocation-category breadth |
| Resource accounting lifecycle | 3 focused tests passed | `validated` | capacity>1 accounting semantics and failure/schedule breadth |
| `tests-smoke` | 3 registered/executed/passed | `validated` | broader application smoke matrix |
| Debian trigger/path filters | integrated | `implemented` | package lifecycle execution |
| Application presets | targets/presets confirmed | `needs-local-validation` | independent build/startup/workflows |
| Sanitizers/profiling | not executed | `not-started` | ASan/LSan/UBSan/Valgrind/profiling |

## 4. Exact current Phase 0 inventory

### 4.1 Kernel

Latest validated head before merge: `eddca45a8ab0c48b94b2e53f553c18a56729fc99`.

- registered: 1,719;
- executed/passed: 1,715;
- failed: 0;
- disabled: 4;
- Phase 0 run: `29795009950`;
- artifact: `genesys-phase0-tests-kernel-unit`, ID `8481811952`.

New focused runtime tests accumulated after the original Phase 0 baseline:

- #1–#9: legacy solver quadrature and unsupported-derivative contract;
- #10–#13: active Search/Remove scenarios;
- #14–#16: Queue statistics lifecycle;
- #17–#18: Station statistics lifecycle;
- #19–#20: Delay statistics lifecycle;
- #21–#23: Resource accounting lifecycle.

Historical disabled duplicate runtime tests remain registered:

- SearchQueue found/rank/found-port path;
- SearchQueue not-found/zero-rank path;
- Remove single-rank interval path;
- Remove range path.

These are no longer behavioral coverage gaps because equivalent focused tests are active and mandatory. They remain source-cleanup debt.

### 4.2 Smoke

- registered/executed/passed: 3/3/3;
- failed: 0;
- tests: simulator start, continuous system, LSODE.

## 5. Detailed module matrix

| Module / concern | Current evidence | Missing coverage | Priority | Acceptance criterion | Status |
|---|---|---|---|---|---|
| Utilities | baseline green | edge cases and sanitizers | P2 | deterministic edge behavior; no findings | `partially-validated` |
| Simulator support/runtime | Search/Remove plus Queue/Station/Delay/Resource lifecycle suites green | temporary-model lifetime, sanitizers, broader failure paths | P0/P1 | no leak/UB and explicit lifecycle ownership | `partially-validated` |
| Persistence | baseline green | cross-plugin and compatibility fixtures | P1 | supported fields round-trip; unsupported diagnosed | `partially-validated` |
| PluginManager | baseline green | ownership, duplicate insertion, future ABI/load/unload | P1 | deterministic lifecycle and diagnostics | `partially-validated` |
| Parser expressions | baseline green | unknown functions and historical compatibility | P1 | deterministic errors and `.gen` compatibility | `partially-validated` |
| FunctionRegistry | historical PR only | current disposition/design | P1 | explicit decision and compatibility tests | `not-started` |
| Kernel statistics | runtime collector lifecycle now covered for four classes | authoritative references and degenerate inputs | P0/P1 | declared methods match references/tolerances | `partially-validated` |
| Hypothesis testing | current tests green | authoritative statistical references | P0/P1 | reference-backed p-values/intervals | `partially-validated` scientifically |
| `SolverDefaultImpl1` | 9 regression tests and fail-fast derivative boundary | replacement ODE API if required | P1 | no silent unsupported numerical result | `stabilized` |
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

## 6. Aggregation and lifecycle findings

Resolved:

- AI plugin executable belongs to ordinary and direct-runner graphs;
- exact kernel and smoke inventories are captured;
- legacy solver silent/undefined paths are quarantined or corrected;
- active Search/Remove behavior is mandatory;
- Queue, Station, Delay, and Resource initialize statistics/accounting safely on first public use;
- statistics-disabled operations no longer dereference absent accounting objects in the audited classes.

Still open:

1. four disabled Search/Remove blocks remain as historical duplicates;
2. temporary `Model` lifetime in `Simulator::_completePluginsFieldsAndTemplate()`;
3. repeated runtime libraries in selected link lists;
4. full/minimal plugin source overlap;
5. GUI test source aggregation/build cost;
6. generated method inventory is structural inventory, not behavioral proof.

## 7. Numerical and scientific evidence policy

Every correctness-sensitive method must define its mathematics, parameterization, domain, units, oracle/reference, tolerances, degenerate behavior, deterministic seeds, dependency mode, and exact/approximate/heuristic classification.

## 8. Next bounded execution

The next P0 workstream should characterize the temporary-model ownership/lifetime path in `Simulator::_completePluginsFieldsAndTemplate()` without mixing unrelated refactoring. Use a test-only or sanitizer-backed red checkpoint where feasible, then apply the smallest RAII/lifetime correction. Application, package, broader sanitizer, Qt6 cleanup, plugin architecture, worker security, and optimizer work remain separate workstreams.
