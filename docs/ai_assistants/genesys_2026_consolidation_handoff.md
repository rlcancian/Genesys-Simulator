# Handoff — GenESyS 2026 consolidation

## 1. Final task state

- Repository: `rlcancian/Genesys-Simulator`.
- Active development branch: `WorkInProgress`.
- Current integrated checkpoint: `6aca91a55beaf4ee8838c4843159ec4e06456318`.
- Core Phase 0 baseline: `validated` for the recorded GitHub Actions commits and toolchain.
- Current kernel inventory: 1,721 registered; 1,717 executed/passed; 4 historical duplicates disabled; 0 failures.
- Workflow and Phase 0 infrastructure: merged through PRs #470–#473.
- Legacy solver stabilization: PR #474.
- Search/Remove active coverage: PR #475.
- Queue, Station, Delay, and Resource lifecycle corrections: PRs #476, #478, #479, and #480.
- Runtime lifecycle evidence: PR #481.
- Plugin-completion ownership and focused ASan/LSan: PR #483.
- Optimizer shallow-copy barrier: PR #485.
- Issues #477, #482, and #484: closed as completed.
- Release readiness: not established.
- Numerical, statistical, biochemical, and biological validity: not established by ordinary test success.

## 2. What was done

- Read the root `README.md` and mandatory `docs/ai_assistants/README.md`.
- Mapped CMake/Ninja/C++23, Qt6, applications, plugins, tools, tests, CI, packaging, and high-risk source areas.
- Created consolidation plans, module inventory, test matrix, evidence documents, decision records, and this handoff.
- Corrected stale build/application documentation and active workflow filters.
- Added offline AI plugin tests to ordinary and direct-runner aggregates.
- Added reusable Phase 0 kernel/smoke validation with artifacts.
- Characterized and stabilized legacy Simpson quadrature and unsupported derivative behavior.
- Added mandatory active Search/Remove runtime coverage.
- Corrected first-use statistics/accounting lifecycle defects in Queue, Station, Delay, and Resource.
- Added a focused ASan/LeakSanitizer workflow and removed the plugin-completion temporary-model/helper leaks.
- Blocked shallow copy/move of `OptimizerDefaultImpl1` while its seven `List*` members remain manually owned.
- Preserved bounded red/green checkpoints before production corrections.

## 3. Relevant technical comments

- [Build] Canonical baseline: Ubuntu 24.04, CMake 3.24+, Ninja, C++23, compiler extensions disabled.
- [Executed toolchain] CMake 3.31.6, Ninja 1.13.2, G++ 13.3.0.
- [Qt] Qt6-only is the intended support policy; Qt5 fallback code remains technical debt.
- [CI] Historical records of 16/1657 failures and three failing GUI GMDD tests were not reproduced.
- [Tests] Latest kernel inventory contains 1,721 registered tests: 1,717 passed and 4 duplicate historical blocks disabled.
- [Smoke] Three tests pass: simulator start, continuous system, and LSODE.
- [Sanitizer] Focused plugin-completion ASan/LSan is permanent and green.
- [AI] The three offline AI plugin tests are included in ordinary and direct-runner paths.
- [Solver] Composite Simpson behavior is covered; unsupported derivative overloads throw `std::logic_error`.
- [Runtime] Search/Remove has active focused coverage; four old disabled blocks remain cleanup debt only.
- [Lifecycle] Queue, Station, Delay, and Resource initialize reporting/accounting safely on first public use.
- [Ownership] Temporary plugin-completion Model, Counter responses, StatisticsCollector responses/statistics, and EntityType auxiliary list ownership were corrected.
- [Optimizer] The backend is non-copyable/non-movable but remains a scaffold, not a mature optimizer.
- [Plugins] Current runtime remains statically aggregated; dynamic migration is deferred until target/source overlap and lifecycle contracts are mapped.
- [Worker] Controlled academic intranet remains the intended deployment profile after security hardening.

## 4. Problems encountered

### 4.1 Historical documentation drift

Older documents reported failures no longer reproduced and used obsolete branch, target, path, and application names. Stable guides and the root README were corrected. Historical files remain evidence, not current status.

### 4.2 Test aggregation omissions

AI tests and later focused regression executables initially did not participate in all aggregate/direct-runner paths. Dedicated dependencies and runner targets were added.

### 4.3 Disabled Search/Remove tests

Four large historical blocks remain disabled. Equivalent focused tests are active and mandatory. Safe removal should be performed in a local checkout because the historical source file is very large.

### 4.4 Legacy solver defects

Composite Simpson accepted invalid odd subinterval counts, duplicated loops, used non-standard VLAs, and exposed undefined derivative behavior. Focused red/green tests now enforce the corrected quadrature and fail-fast boundary.

### 4.5 Statistics/accounting lifecycle defects

Public operations could execute before related-data creation in Queue, Station, Delay, and Resource. Null collectors/counters and a retained replication-end callback were corrected with class-specific lifecycle guards.

### 4.6 Plugin-completion ownership leak

The temporary `Model` was not registered in `ModelManager` and was never destroyed. Focused LeakSanitizer evidence measured:

- 27,533 bytes / 470 allocations initially;
- 2,564 bytes / 38 after temporary-model RAII;
- 80 bytes / 2 after Counter/StatisticsCollector ownership;
- zero after EntityType auxiliary-list cleanup.

### 4.7 Optimizer shallow-copy hazard

Seven owning raw list pointers were manually deleted, while implicit copy semantics could duplicate them. Copy and move construction/assignment are now explicitly deleted. Algorithms remain deferred.

### 4.8 Remaining scientific and architectural blockers

Authoritative statistical validation, plugin target overlap, worker security, optimizer maturity, whole-cell scientific claims, application/package validation, and dynamic plugin ABI remain unresolved.

## 5. Corrections and adjustments made

Integrated changes include:

- PR #470: active branch and Debian trigger/path filters;
- PR #471: AI plugin test aggregation;
- PR #472: reusable Phase 0 workflow;
- PR #473: Phase 0 evidence documentation;
- PR #474: legacy solver quadrature/fail-fast derivative contract;
- PR #475: active Search/Remove runtime coverage;
- PR #476: Queue statistics lifecycle;
- PR #478: Station statistics lifecycle;
- PR #479: Delay statistics lifecycle;
- PR #480: Resource statistics/accounting lifecycle;
- PR #481: runtime lifecycle evidence;
- PR #483: plugin-completion lifetime, helper ownership, and focused sanitizer workflow;
- PR #485: optimizer non-copy/non-move ownership contract.

No plugin ABI migration, application redesign, package recipe redesign, worker security redesign, optimizer algorithm, whole-cell model, or public persistence format was changed.

## 6. Files created or changed

Principal evidence:

- `docs/ai_assistants/genesys_2026_test_matrix.md`;
- `genesys_2026_runtime_lifecycle_evidence_20260721.md`;
- `genesys_2026_ownership_evidence_20260721.md`;
- this handoff.

Principal focused tests:

- `test_tools_legacy_solver_regression.cpp`;
- `test_search_remove_runtime.cpp`;
- `test_queue_statistics_lifecycle.cpp`;
- `test_station_statistics_lifecycle.cpp`;
- `test_delay_statistics_lifecycle.cpp`;
- `test_resource_accounting_lifecycle.cpp`;
- `test_simulator_plugin_completion_lifetime.cpp`;
- `test_optimizer_ownership_contract.cpp`.

Principal ownership production areas:

- `source/kernel/simulator/Simulator.cpp`;
- `source/kernel/simulator/essentialPlugins/Counter.*`;
- `source/kernel/simulator/essentialPlugins/StatisticsCollector.*`;
- `source/kernel/simulator/essentialPlugins/EntityType.cpp`;
- `source/tools/Optimization/OptimizerDefaultImpl1.h`.

## 7. Validation performed

### 7.1 Latest ordinary and GUI checkpoint

PR #485 ordinary run `29833758692`:

- `tests-unit` configure: passed;
- aggregate build: passed;
- CTest: passed;
- focused GUI GMDD diagnostics: passed.

### 7.2 Latest kernel-focused checkpoint

PR #485 Phase 0 run `29833758797`:

- validated head: `e3b884d6401337d532b6cfd5ac08fff8ee8c52df`;
- configure/build/direct runner/inventory/CTest: passed;
- registered: 1,721;
- executed/passed: 1,717;
- disabled: 4;
- failed: 0;
- optimizer ownership test: #25;
- artifact ID: `8496613101`.

### 7.3 Focused sanitizer checkpoint

PR #483 final focused run `29831405860`:

- ASan/LeakSanitizer execution: passed;
- exit code: 0;
- no leak, use-after-free, double-free, or SEGV markers;
- artifact ID: `8495543015`.

The focused sanitizer remained green on PR #485 run `29833758605`.

### 7.4 Smoke checkpoint

- configure/build/CTest: passed;
- inventory/executed/passed: 3/3/3.

## 8. State of the consolidation plan

- Phase 0 core baseline: `validated`.
- Phase 1 bounded stabilization completed for:
  - legacy solver contract;
  - active Search/Remove coverage;
  - Queue/Station/Delay/Resource lifecycle;
  - plugin-completion ownership;
  - optimizer copy/move ownership barrier.
- Broader release evidence remains incomplete:
  - applications;
  - package lifecycle;
  - broader sanitizers/profiling;
  - network/security;
  - scientific references.
- Dynamic plugins: deferred.
- Qt5 removal: pending.

## 9. Pending work

Immediate P1 architectural work:

1. Map the exact sources compiled into `genesys_plugins_components` and `genesys_plugins_components_minimal`.
2. Map every consumer and link group/order.
3. Identify duplicate compilation and potential ODR/link risk.
4. Produce an evidence-only source-to-target/link document before modifying CMake.

Other priorities:

- remove or formally retire the four historical disabled duplicate blocks locally;
- validate shell, worker, main GUI, and independent GUI presets;
- execute Debian package lifecycle validation;
- add selective UBSan and broader ASan/LSan coverage;
- remove Qt5 fallback in a separate PR;
- harden worker tokens and secret handling;
- implement and validate optimizer algorithms separately from ownership work;
- decide parser `FunctionRegistry` disposition;
- validate statistical/biochemical/whole-cell claims against authoritative references.

## 10. Recommended next actions

1. Merge the ownership evidence documentation after ordinary CI passes.
2. Open an inventory-only issue/branch for plugin source-to-target overlap.
3. Record source lists, generated target membership, consumers, and link groups without changing target behavior.
4. Classify each overlap as intentional, accidental, or transitional.
5. Only then propose a small target ownership correction or a pilot library split.

## 11. Guidance for the next AI assistant

First read `docs/ai_assistants/README.md`, then the latest ownership evidence, runtime-lifecycle evidence, test matrix, consolidation plan, module inventory, and this handoff.

Use `20261`, not `2026-1`, except for historical quotations. Treat the baseline as green only for recorded heads/toolchains. Do not infer scientific correctness from test counts. Preserve the red/green pattern used in PRs #474–#485. Do not begin dynamic plugin migration before the overlap inventory is complete.

## 12. Limitations and uncertainties

- Validation used GitHub-hosted Ubuntu runners, not the maintainer's local machine.
- Four duplicate historical tests remain disabled, although equivalent focused tests are active.
- Application startup and user workflows were not executed.
- Debian package lifecycle was not revalidated after trigger correction.
- Sanitizer coverage is focused, not repository-wide.
- Worker networking/authentication and plugin dynamic loading were not executed.
- Optimizer algorithms remain largely unimplemented.
- Scientific correctness still requires independent references and domain review.

## 13. Operational result

Result: the core baseline and bounded solver, runtime lifecycle, ownership, and compile-time safety corrections are green and reproducible through GitHub Actions. The repository now has focused ASan/LeakSanitizer evidence and a 1,721-test kernel inventory. The next safe architectural workstream is an evidence-only map of full/minimal plugin source overlap and link consumers before any CMake or ABI change.
