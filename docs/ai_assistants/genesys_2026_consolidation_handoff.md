# Handoff — GenESyS 2026 consolidation

## 1. Final task state

- Repository: `rlcancian/Genesys-Simulator`.
- Active development branch: `WorkInProgress`.
- Current integrated checkpoint: `4f98a909d941ac31582205904c597fb345d3527f`.
- Core Phase 0 baseline: `validated` for the recorded GitHub Actions commits and toolchain.
- Current kernel inventory: 1,719 registered; 1,715 executed/passed; 4 historical duplicates disabled; 0 failures.
- Consolidation documentation: merged through PR #469.
- Workflow and Phase 0 infrastructure: merged through PRs #470–#473.
- Legacy solver stabilization: merged through PR #474.
- Search/Remove active coverage: merged through PR #475.
- Queue, Station, Delay, and Resource lifecycle corrections: merged through PRs #476, #478, #479, and #480.
- Runtime statistics lifecycle issue #477: closed as completed.
- Release readiness: not established.
- Numerical, statistical, biochemical, and biological validity: not established by ordinary test success.

## 2. What was done

- Read the root `README.md` and mandatory `docs/ai_assistants/README.md`.
- Mapped CMake/Ninja/C++23, Qt6, applications, plugins, tools, tests, CI, packaging, and high-risk source areas.
- Created the 2026 consolidation plan, module inventory, test matrix, decision records, research-direction documents, Phase 0 evidence, and this handoff.
- Corrected stale build/application documentation and active workflow filters.
- Added offline AI plugin tests to ordinary and direct-runner aggregates.
- Added reusable Phase 0 kernel/smoke validation with artifacts.
- Characterized and stabilized `SolverDefaultImpl1` quadrature; unsupported derivative overloads now fail explicitly.
- Added mandatory active coverage for four Search/Remove runtime scenarios.
- Reproduced and corrected first-use statistics/accounting lifecycle defects in `Queue`, `Station`, `Delay`, and `Resource`.
- Preserved red checkpoints before production fixes for Station, Delay, and Resource.
- Executed ordinary, GUI GMDD, kernel direct-runner/CTest, and smoke validation after each bounded correction.

## 3. Relevant technical comments

- [Build] Canonical baseline: Ubuntu 24.04, CMake 3.24+, Ninja, C++23, compiler extensions disabled.
- [Executed toolchain] CMake 3.31.6, Ninja 1.13.2, G++ 13.3.0.
- [Qt] Qt6-only is the intended support policy; Qt5 fallback code remains technical debt.
- [CI] Historical records of 16/1657 failures and three failing GUI GMDD tests were not reproduced.
- [Tests] Latest kernel inventory contains 1,719 registered tests: 1,715 passed and 4 historical duplicate blocks were disabled.
- [Tests] Smoke inventory contains 3 tests; all passed.
- [AI] The three offline AI plugin tests are included in ordinary and direct-runner paths.
- [Solver] Composite Simpson behavior is covered by focused tests; derivative overloads without a defined step-size contract throw `std::logic_error`.
- [Runtime] Search/Remove has active focused coverage; the old four disabled blocks are duplicate cleanup debt, not current behavioral gaps.
- [Lifecycle] Queue, Station, Delay, and Resource create statistics/accounting data idempotently on first statistics-enabled public use.
- [Disabled statistics] Audited operations no longer dereference collectors/counters that intentionally do not exist.
- [Resource] Replication-end accounting callback is registered once and guarded for disabled/incomplete state.
- [Plugins] Current runtime remains statically aggregated; dynamic migration is deferred until target overlap and lifecycle contracts are mapped.
- [Worker] Controlled academic intranet is the intended deployment profile after security hardening.
- [Optimizer] The current backend remains a scaffold and must not be presented as a mature optimizer.

## 4. Problems encountered

### 4.1 Historical documentation drift

Older documents reported failures no longer reproduced and used obsolete branch, target, path, and application names. Stable guides and the root README were corrected. Historical files remain evidence, not current status.

### 4.2 Test aggregation omissions

AI plugin tests existed but were missing from aggregate/direct-runner orchestration. Dedicated dependency/run targets were added and validated.

### 4.3 Disabled Search/Remove tests

Four large historical runtime tests were disabled. A focused active executable now covers equivalent behavior while respecting Queue initialization and asynchronous event-calendar routing. The disabled blocks remain only because the GitHub connector is unsuitable for safely editing the approximately 10,000-line historical test source.

### 4.4 Legacy solver defects

The old Simpson implementation accepted invalid odd subinterval counts, duplicated loops, used non-standard VLAs, and exposed undefined derivative behavior. Focused red/green tests now enforce the corrected quadrature and fail-fast derivative boundary.

### 4.5 Statistics/accounting lifecycle defects

Public operations could execute before model-wide related-data creation:

- `Queue::insertElement()` / `removeElement()`;
- `Station::enter()` / `leave()`;
- `Delay::_onDispatchEvent()`;
- `Resource::seize()` / `release()` and related accounting paths.

The first three dereferenced null statistics collectors. `Resource` also dereferenced cost counters when statistics were disabled and retained a replication-end callback after internal objects could be cleared.

### 4.6 Remaining scientific and architectural blockers

Authoritative statistical validation, plugin target overlap, worker security, optimizer ownership/maturity, whole-cell scientific claims, and dynamic plugin ABI remain unresolved.

## 5. Corrections and adjustments made

Integrated changes include:

- PR #470: active branch and Debian trigger/path filters;
- PR #471: AI plugin test aggregation;
- PR #472: reusable Phase 0 workflow;
- PR #473: complete Phase 0 evidence documentation;
- PR #474: legacy solver quadrature/fail-fast derivative contract;
- PR #475: active Search/Remove runtime coverage;
- PR #476: Queue statistics first-use lifecycle and `_lastTimeNumberInQueueChanged` initialization;
- PR #478: Station statistics first-use lifecycle and `_enterIntoStationComponent=nullptr`;
- PR #479: Delay statistics first-use lifecycle;
- PR #480: Resource statistics/accounting first-use lifecycle, disabled-statistics guards, and replication-end callback safety.

No plugin ABI migration, application redesign, package recipe redesign, worker security redesign, optimizer algorithm, whole-cell model, or public persistence format was changed in this round.

## 6. Files created or changed

Principal consolidated documentation:

- `README.md`;
- stable documents under `docs/ai_assistants/`;
- `genesys_2026_consolidation_plan.md`;
- `genesys_2026_module_inventory.md`;
- `genesys_2026_test_matrix.md`;
- Phase 0 and runtime-lifecycle evidence documents;
- this handoff.

Principal test additions:

- `source/tests/unit/test_tools_legacy_solver_regression.cpp`;
- `source/tests/unit/test_search_remove_runtime.cpp`;
- `source/tests/unit/test_queue_statistics_lifecycle.cpp`;
- `source/tests/unit/test_station_statistics_lifecycle.cpp`;
- `source/tests/unit/test_delay_statistics_lifecycle.cpp`;
- `source/tests/unit/test_resource_accounting_lifecycle.cpp`.

Principal production areas changed:

- `source/tools/Continuous/SolverDefaultImpl1.*`;
- `source/plugins/data/DiscreteProcessing/Queue.*`;
- `source/plugins/data/MaterialHandling/Station.*`;
- `source/plugins/components/DiscreteProcessing/Delay.*`;
- `source/plugins/data/DiscreteProcessing/Resource.*`;
- `source/tests/CMakeLists.txt`.

## 7. Validation performed

### 7.1 Latest ordinary and GUI checkpoint

PR #480 ordinary run `29795009959`:

- `tests-unit` configure: passed;
- aggregate build: passed;
- CTest: passed;
- focused GUI GMDD diagnostics: passed.

### 7.2 Latest kernel-focused checkpoint

PR #480 Phase 0 run `29795009950`:

- validated head: `eddca45a8ab0c48b94b2e53f553c18a56729fc99`;
- configure: passed;
- build/direct runner: passed;
- CTest: passed;
- registered: 1,719;
- executed/passed: 1,715;
- disabled: 4;
- failed: 0;
- artifact: `genesys-phase0-tests-kernel-unit`, ID `8481811952`.

Focused additions after the original Phase 0 baseline:

- tests #1–#9: legacy solver;
- tests #10–#13: Search/Remove;
- tests #14–#16: Queue;
- tests #17–#18: Station;
- tests #19–#20: Delay;
- tests #21–#23: Resource.

### 7.3 Smoke checkpoint

- configure/build/CTest: passed;
- inventory/executed/passed: 3/3/3;
- tests: simulator start, continuous system, LSODE.

## 8. State of the consolidation plan

- Phase 0 core baseline: `validated`.
- Phase 1 bounded stabilization has progressed:
  - legacy solver contract: stabilized;
  - Search/Remove active coverage: validated;
  - runtime statistics/accounting lifecycle for Queue/Station/Delay/Resource: validated.
- Phase 0 broader release evidence remains incomplete:
  - applications;
  - package lifecycle;
  - sanitizers/profiling;
  - network/security;
  - scientific references.
- Dynamic plugins: deferred.
- Qt5 removal: bounded implementation pending.

## 9. Pending work

Immediate P0/P1:

1. Verify the temporary `Model` lifetime finding in `Simulator::_completePluginsFieldsAndTemplate()`.
2. Add a focused leak/lifetime regression or sanitizer-backed checkpoint where technically feasible.
3. Apply the smallest ownership correction only after the path is reproduced or proven by direct code ownership analysis.
4. Remove or formally retire the four historical disabled Search/Remove duplicate blocks using a local checkout.

Other priority work:

- validate representative shell, worker, main GUI, and independent GUI presets;
- execute Debian package build/install/start/uninstall validation;
- add opt-in ASan/LSan/UBSan validation for high-risk suites;
- generate the plugin source-to-target/link map;
- remove Qt5 fallback in a separate PR;
- harden worker tokens and secret handling;
- close optimizer ownership/copy hazards;
- decide parser `FunctionRegistry` disposition;
- validate statistical/biochemical/whole-cell claims against authoritative references.

## 10. Recommended next actions

1. Merge the runtime-lifecycle evidence documentation after ordinary CI passes.
2. Open a focused issue and test-only branch for the temporary-model lifetime path.
3. Inspect every exit path of `Simulator::_completePluginsFieldsAndTemplate()` and the ownership semantics of `Model`/`ModelManager`.
4. Avoid broad Simulator ownership refactoring until the isolated path is understood.
5. Keep application, package, sanitizer, worker-security, plugin-architecture, and optimizer work isolated.

## 11. Guidance for the next AI assistant

First read `docs/ai_assistants/README.md`, then the latest runtime-lifecycle evidence, test matrix, consolidation plan, module inventory, decisions addendum, and this handoff.

Use `20261`, not `2026-1`, except for historical quotations. Treat the core baseline as green only for the recorded commits/toolchain. Do not infer scientific correctness from the test count. Preserve the red/green pattern used in PRs #474–#480. Do not begin dynamic plugin migration before mapping source overlap and lifecycle/ownership boundaries.

## 12. Limitations and uncertainties

- Validation used GitHub-hosted Ubuntu runners, not the maintainer's local machine.
- Four duplicate historical tests remain disabled, although equivalent focused tests are active.
- Application startup and user workflows were not executed.
- Debian package lifecycle was not revalidated after trigger correction.
- Sanitizers, Valgrind, profiling, worker networking, and authentication were not executed.
- The suspected temporary-model leak has not yet been reproduced with LeakSanitizer.
- Scientific correctness still requires independent references and domain review.

## 13. Operational result

Result: the GenESyS core baseline and the bounded runtime lifecycle corrections are green and reproducible through GitHub Actions. The repository has validated ordinary, kernel-focused, smoke, GUI GMDD, AI plugin, legacy solver, Search/Remove, Queue, Station, Delay, and Resource test paths. The next safe P0 workstream is the temporary-model ownership/lifetime path in `Simulator::_completePluginsFieldsAndTemplate()`, isolated from broader Simulator refactoring.
