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
| Plugin completion lifetime | `29831405860` | focused ASan/LSan exit 0; no leak markers | PR #483 merged as `6d6dd4ed...` |
| Optimizer ownership contract | `29833758797` | 1,721 registered; 1,717 passed; 4 disabled | PR #485 merged as `6aca91a5...` |
| Standalone shell | `29885199488` | preset/build/argv/plugin-count/exit passed | PR #495 merged as `abb992ec...` |
| Standalone worker health | `29896225187` | preset/build/HTTP 200/exact JSON/clean exit passed | PR #499 merged as `8f42f509...` |
| Standalone Data Analyser GUI | `29911036076` | preset/build/Xvfb window/bounded startup/clean external teardown passed | PR #503 merged as `b3c7f462...` |

## 3. Core baseline matrix

| Validation path | Executed evidence | Status | Remaining gap |
|---|---|---|---|
| `tests-unit` | configure/build/CTest passed on every final head through PR #503 | `validated` | preserve on future code changes |
| Focused GUI GMDD | 3/3 passed on every final head through PR #503 | `validated` | broader standalone GUI workflows |
| `tests-kernel-unit` | configure/build/direct runner/CTest passed | `validated` | 4 historical duplicate tests remain disabled |
| Kernel CTest inventory | 1,721 registered; 1,717 passed; 4 disabled | `validated` | remove or formally retire duplicate disabled blocks |
| Focused plugin lifetime sanitizer | ASan/LSan workflow exit 0 with artifact evidence | `validated` | expand sanitizer scope selectively |
| AI tests ordinary/direct paths | tests #495–#497 passed in recorded Phase 0 baseline | `validated` | provider transport/security coverage |
| Legacy solver regressions | 9 focused tests passed | `validated` software contract | authoritative numerical use beyond the quarantined API |
| Search/Remove runtime | 4 focused active tests passed | `validated` | remove historical disabled duplicates locally |
| Queue statistics lifecycle | 3 focused tests passed | `validated` | broader sanitizer coverage |
| Station statistics lifecycle | 2 focused tests passed | `validated` | broader entity-type combinations |
| Delay statistics lifecycle | 2 focused tests passed | `validated` | nonzero-delay and allocation-category breadth |
| Resource accounting lifecycle | 3 focused tests passed | `validated` | capacity>1 accounting semantics and failure/schedule breadth |
| Optimizer ownership contract | non-copy/non-move traits enforced | `validated` ownership boundary | algorithm and future RAII/value migration |
| `tests-smoke` | 3 registered/executed/passed | `validated` | broader application smoke matrix |
| Standalone shell | preset/build/argv/plugin-count/exit passed | `partially-validated` | model load/run and `autoloadplugins.txt` deployment contract |
| Standalone worker | preset/build/loopback health/clean request-limited exit passed | `partially-validated` | bind policy, authentication, protected endpoints and resource controls |
| Standalone Data Analyser GUI | preset/build/Xvfb window/startup/SIGTERM teardown passed | `partially-validated` | interaction, import, fitting, chart, export and scientific correctness |
| Debian trigger/path filters | integrated | `implemented` | package lifecycle execution |
| Other independent GUI presets | targets/presets confirmed | `needs-local-validation` | Optimizer, AI Assistant and HTTP Worker startup/workflows |
| Broader sanitizers/profiling | only focused plugin-lifetime ASan/LSan executed | `partially-started` | UBSan, broader ASan/LSan, Valgrind, profiling |

## 4. Exact current Phase 0 inventory

### 4.1 Kernel

Latest validated head before PR #485 merge: `e3b884d6401337d532b6cfd5ac08fff8ee8c52df`.

- registered: 1,721;
- executed/passed: 1,717;
- failed: 0;
- disabled: 4;
- Phase 0 run: `29833758797`;
- artifact: `genesys-phase0-tests-kernel-unit`, ID `8496613101`.

Focused tests accumulated after the original Phase 0 baseline:

- #1–#9: legacy solver quadrature and unsupported-derivative contract;
- #10–#13: active Search/Remove scenarios;
- #14–#16: Queue statistics lifecycle;
- #17–#18: Station statistics lifecycle;
- #19–#20: Delay statistics lifecycle;
- #21–#23: Resource accounting lifecycle;
- #24: plugin-completion temporary-model lifetime under ASan/LSan;
- #25: optimizer compile-time ownership contract.

Historical disabled duplicate runtime tests remain registered:

- SearchQueue found/rank/found-port path;
- SearchQueue not-found/zero-rank path;
- Remove single-rank interval path;
- Remove range path.

These are no longer behavioral coverage gaps because equivalent focused tests are active and mandatory. They remain source-cleanup debt.

### 4.2 Focused ASan/LeakSanitizer

Final validated head for PR #483: `2a007c45fad8b451fe60dac67c4390b777be86e7`.

- workflow: `GenESyS Plugin Lifetime Sanitizer`;
- final run: `29831405860`;
- exit code: 0;
- artifact: `genesys-plugin-completion-lifetime-sanitizer`, ID `8495543015`;
- no AddressSanitizer, LeakSanitizer, use-after-free, double-free, or SEGV markers.

The red/green sequence reduced the focused leak from 27,533 bytes/470 allocations to zero.

### 4.3 Smoke

- registered/executed/passed: 3/3/3;
- failed: 0;
- tests: simulator start, continuous system, LSODE.

### 4.4 Standalone shell

PR #495 focused run `29885199488`:

- preset configure/build: passed;
- deterministic argv workflow: passed;
- static fallback exposed 123 plugins;
- clean exit: passed;
- artifact ID: `8516303352`.

The artifact also recorded the unresolved missing `autoloadplugins.txt` deployment contract tracked by issue #496.

### 4.5 Standalone worker

PR #499 focused run `29896225187`:

- preset configure/build: passed;
- listener: `0.0.0.0:44559`;
- request path: `127.0.0.1:44559/health`;
- HTTP status: 200;
- exact body: `{"ok":true,"status":"up"}`;
- clean exit after `--max-requests 1`: passed;
- no residual process: passed;
- artifact ID: `8520123900`.

The wildcard bind is evidence of current behavior, not an approved deployment policy. Issue #500 tracks the required bind-address contract.

### 4.6 Standalone Data Analyser GUI

PR #503 final focused run `29911036076`:

- `gui-dataanalyser` preset configure/build: passed;
- exactly one `genesys-dataanalyser-gui` executable: found;
- private Xvfb display with TCP disabled: started;
- Qt6/XCB process: started and remained alive through the bounded interval;
- PID-associated X11 window: `2097158`;
- SIGTERM exit code: `143`;
- residual application process: none;
- artifact ID: `8525948784`.

Ordinary run `29911036020` passed `tests-unit` configure/build/CTest and focused GUI GMDD diagnostics.

This is process/window startup evidence only. It does not validate Data Analyser interaction, imports, fitting, charts, exports, persistence, or numerical/statistical correctness.

## 5. Detailed module matrix

| Module / concern | Current evidence | Missing coverage | Priority | Acceptance criterion | Status |
|---|---|---|---|---|---|
| Utilities | baseline green | edge cases and broader sanitizers | P2 | deterministic edge behavior; no findings | `partially-validated` |
| Simulator support/runtime | Search/Remove, lifecycle suites, and temporary-model sanitizer green | broader ownership and failure paths | P1 | no leak/UB and explicit lifecycle ownership | `partially-validated` |
| Essential Counter/StatisticsCollector/EntityType ownership | focused ASan/LSan and full suite green | broader repeated-create/destroy stress | P1 | no leak/double-delete/use-after-free | `validated` for exercised path |
| Persistence | baseline green | cross-plugin and compatibility fixtures | P1 | supported fields round-trip; unsupported diagnosed | `partially-validated` |
| PluginManager | completion lifetime fixed; baseline green | duplicate insertion, target overlap, future ABI/load/unload | P1 | deterministic lifecycle and diagnostics | `partially-validated` |
| Parser expressions | baseline green | unknown functions and historical compatibility | P1 | deterministic errors and `.gen` compatibility | `partially-validated` |
| FunctionRegistry | historical PR only | current disposition/design | P1 | explicit decision and compatibility tests | `not-started` |
| Kernel statistics | runtime collector lifecycle and focused ownership covered | authoritative references and degenerate inputs | P0/P1 | declared methods match references/tolerances | `partially-validated` |
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
| Worker API/tokens | public health workflow validated; current wildcard listener recorded | bind contract, auth, quotas, CSPRNG, expiry/rotation, protected endpoints | P1 | controlled-intranet security contract | `partially-validated`, hardening blocked |
| Optimizer | shallow-copy hazard blocked; backend remains scaffold | algorithm/ranking/constraints and later RAII migration | P1 | Level 3 contract and benchmark suite | `partially-implemented` |
| Data Analyser | standalone configure/build/window startup green | reference imports/fits/charts/exports and interaction | P1 | reference-backed workflow | `partially-validated` startup; functionally incomplete |
| Do Experiments | backend fragments; GUI absent | DOE specification/tests/workflow | P1/P2 | known designs and analyses match references | `partially-implemented` |
| Shell | preset/build/argv/plugin-count/exit green | model workflow and autoload deployment | P1 | minimal model completes cleanly | `partially-validated` |
| Main GUI | focused tests green | standalone startup/model interaction | P1 | target starts; workflow smoke passes | `partially-validated` |
| Independent GUIs | Data Analyser startup validated | Optimizer, AI Assistant and HTTP Worker startup/workflows | P1 | each application starts independently | `partially-validated` |
| Packaging | triggers corrected | build/install/start/uninstall/version | P2 | packages complete lifecycle | `needs-ci-validation` |
| Qt6-only cleanup | policy decided | remove Qt5 fallback across CMake/tests/scripts | P1/P2 | Qt6 baseline remains green; no fallback discovery | `not-started` |
| Dynamic plugins | ABI direction recorded | target/source overlap decision and pilot | P1/P2 | versioned pilot loads/unloads safely | `deferred` |

## 6. Aggregation and lifecycle findings

Resolved:

- AI plugin executable belongs to ordinary and direct-runner graphs;
- exact kernel and smoke inventories are captured;
- legacy solver silent/undefined paths are quarantined or corrected;
- active Search/Remove behavior is mandatory;
- Queue, Station, Delay, and Resource initialize statistics/accounting safely on first public use;
- statistics-disabled operations no longer dereference absent accounting objects in the audited classes;
- temporary plugin-completion Model and helper-owned responses/statistics are released;
- focused ASan/LeakSanitizer guard is permanent;
- `OptimizerDefaultImpl1` shallow copy/move is prohibited;
- standalone shell and worker public-health startup paths are covered by focused workflows;
- standalone Data Analyser configure/build/X11-window startup is covered by a focused workflow.

Still open:

1. four disabled Search/Remove blocks remain as historical duplicates;
2. repeated runtime libraries in selected link lists;
3. full/minimal plugin architecture decision #492;
4. GUI test source aggregation/build cost;
5. generated method inventory is structural inventory, not behavioral proof;
6. optimizer algorithm and container modernization remain future work;
7. shell autoload deployment decision #496;
8. worker bind-address decision #500 and broader security hardening;
9. Data Analyser functional/scientific workflow validation.

## 7. Numerical and scientific evidence policy

Every correctness-sensitive method must define its mathematics, parameterization, domain, units, oracle/reference, tolerances, degenerate behavior, deterministic seeds, dependency mode, and exact/approximate/heuristic classification.

## 8. Next bounded execution

While decisions #492, #496, and #500 remain open, continue standalone Qt6 application validation. Issue #504 defines the next bounded candidate: configure/build and Xvfb startup/clean teardown of the existing `gui-optimizer` preset. Do not implement optimizer algorithms, claim Level 3 maturity, remove Qt5 fallback, or alter plugin/worker behavior in that PR.