# Handoff — GenESyS 2026 consolidation

## 1. Final task state

- Repository: `rlcancian/Genesys-Simulator`.
- Active development branch: `WorkInProgress`.
- Current integrated checkpoint: `8520c542a80697aad16527a89cec6b4d675d5797`.
- Core Phase 0 baseline: `validated` for the recorded GitHub Actions commits and toolchain.
- Current kernel inventory: 1,721 registered; 1,717 executed/passed; 4 historical duplicates disabled; 0 failures.
- Runtime lifecycle, focused ownership sanitizer, and optimizer copy/move barrier: integrated.
- Static plugin target inventory and generated codemodel/link/symbol workflows: integrated through PR #491.
- Plugin architecture implementation: blocked on human decision issue #492.
- Release readiness: not established.
- Numerical, statistical, biochemical, and biological validity: not established by ordinary test success.

## 2. What was done

- Read the root `README.md` and mandatory `docs/ai_assistants/README.md`.
- Mapped CMake/Ninja/C++23, Qt6, applications, plugins, tools, tests, CI, packaging, and high-risk source areas.
- Corrected stale build/application documentation and active workflow filters.
- Added offline AI plugin tests to ordinary and direct-runner aggregates.
- Added reusable Phase 0 kernel/smoke validation with artifacts.
- Characterized and stabilized legacy Simpson quadrature and unsupported derivative behavior.
- Added mandatory active Search/Remove runtime coverage.
- Corrected first-use statistics/accounting lifecycle defects in Queue, Station, Delay, and Resource.
- Added focused ASan/LeakSanitizer validation and removed the plugin-completion temporary-model/helper leaks.
- Blocked shallow copy/move of `OptimizerDefaultImpl1` while its seven `List*` members remain manually owned.
- Mapped full/minimal static plugin sources, consumers, compile definitions, symbols, and final link fragments.
- Added permanent diagnostic workflows for CMake codemodel and GLPK-present/absent archive/link evidence.
- Preserved bounded red/green checkpoints before production corrections.

## 3. Relevant technical comments

- [Build] Canonical baseline: Ubuntu 24.04, CMake 3.24+, Ninja, C++23, compiler extensions disabled.
- [Executed toolchain] CMake 3.31.6, Ninja 1.13.2, G++ 13.3.0.
- [Qt] Qt6-only is the intended support policy; Qt5 fallback code remains technical debt.
- [Tests] Latest kernel inventory contains 1,721 registered tests: 1,717 passed and 4 duplicate historical blocks disabled.
- [Smoke] Three tests pass: simulator start, continuous system, and LSODE.
- [Sanitizer] Focused plugin-completion ASan/LSan is permanent and green.
- [Solver] Composite Simpson behavior is covered; unsupported derivative overloads throw `std::logic_error`.
- [Runtime] Search/Remove has active focused coverage; four old disabled blocks remain cleanup debt only.
- [Lifecycle] Queue, Station, Delay, and Resource initialize reporting/accounting safely on first public use.
- [Ownership] Temporary plugin-completion Model, Counter responses, StatisticsCollector responses/statistics, and EntityType auxiliary-list ownership were corrected.
- [Optimizer] The backend is non-copyable/non-movable but remains a scaffold, not a mature optimizer.
- [Plugins] Full and minimal targets compile the same 84 component sources. Runtime primarily uses minimal; optional GLPK is applied only to full.
- [Plugins] The focused diffusion test links both archives. Without GLPK their global symbol sets are identical; with GLPK they expose different FBA implementations.
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

### 4.8 Static plugin target duplication and GLPK divergence

Generated evidence confirms:

- `genesys_plugins_components` and `genesys_plugins_components_minimal` compile the same 84 `.cpp` sources;
- minimal is the runtime-effective archive in the validated codemodel;
- full receives `GENESYS_HAVE_GLPK`; minimal does not;
- the focused diffusion test links both archives;
- without GLPK, both archives expose 26,258 identical global symbols;
- with GLPK, full exposes 26,205 symbols, minimal 26,258, with 26,198 common, 7 full-only, and 60 minimal-only symbols;
- full-only includes `GlpkFluxBalanceSolver::solve(...)`;
- minimal-only includes the built-in basis-enumeration solver and helpers.

No duplicate-symbol linker failure was demonstrated because the focused build succeeds. The graph remains semantically ambiguous and duplicates compilation/storage.

### 4.9 Remaining scientific and architectural blockers

Authoritative statistical validation, worker security, optimizer maturity, whole-cell scientific claims, application/package validation, Qt5 cleanup, and dynamic plugin ABI remain unresolved.

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
- PR #485: optimizer non-copy/non-move ownership contract;
- PR #486: ownership/sanitizer baseline documentation;
- PR #488: static plugin target overlap inventory;
- PR #489: generated CMake File API target introspection workflow;
- PR #490: generated codemodel evidence;
- PR #491: GLPK-present/absent archive-symbol and final-link evidence workflow.

No plugin ABI migration, application redesign, package recipe redesign, worker security redesign, optimizer algorithm, whole-cell model, or public persistence format was changed.

## 6. Principal evidence and tests

Principal evidence:

- `docs/ai_assistants/genesys_2026_test_matrix.md`;
- `genesys_2026_runtime_lifecycle_evidence_20260721.md`;
- `genesys_2026_ownership_evidence_20260721.md`;
- `genesys_plugin_target_overlap_inventory_20260721.md`;
- `genesys_plugin_target_introspection_evidence_20260721.md`;
- `genesys_plugin_target_link_evidence_20260722.md`;
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

Principal diagnostic workflows:

- `.github/workflows/genesys-phase0-validation.yml`;
- `.github/workflows/genesys-plugin-lifetime-sanitizer.yml`;
- `.github/workflows/genesys-plugin-target-introspection.yml`;
- `.github/workflows/genesys-plugin-target-link-evidence.yml`.

## 7. Validation performed

### 7.1 Core baseline

PR #485 ordinary run `29833758692`:

- `tests-unit` configure/build/CTest: passed;
- focused GUI GMDD diagnostics: passed.

PR #485 Phase 0 run `29833758797`:

- configure/build/direct runner/inventory/CTest: passed;
- registered: 1,721;
- executed/passed: 1,717;
- disabled: 4;
- failed: 0;
- artifact ID: `8496613101`.

### 7.2 Focused sanitizer

PR #483 final focused run `29831405860`:

- ASan/LeakSanitizer execution: passed;
- exit code: 0;
- no leak, use-after-free, double-free, or SEGV markers;
- artifact ID: `8495543015`.

### 7.3 Generated plugin codemodel

PR #489:

- ordinary run `29856007581`: passed;
- introspection run `29856007683`: passed;
- artifact ID: `8505306513`.

### 7.4 GLPK-present/absent link evidence

PR #491 validated head: `0e40d892d7fe9f0a34b4152f32710eab0825638e`.

- ordinary run `29873203393`: configure/build/CTest and GUI GMDD passed;
- diagnostic run `29873203342`: both matrix jobs passed;
- GLPK-present artifact ID: `8512049198`;
- GLPK-absent artifact ID: `8512054888`.

## 8. State of the consolidation plan

- Phase 0 core baseline: `validated`.
- Bounded stabilization completed for solver, Search/Remove, Queue/Station/Delay/Resource lifecycle, plugin-completion ownership, and optimizer copy/move ownership.
- Static plugin full/minimal inventory and generated link/symbol evidence: `complete`.
- Static plugin target correction: `blocked on issue #492`.
- Broader release evidence remains incomplete: applications, package lifecycle, broader sanitizers/profiling, network/security, and scientific references.
- Dynamic plugins: deferred until static target decision/correction is complete.
- Qt5 removal: pending.

## 9. Current decision blocker

Issue #492 requires selection of the intended static architecture:

- **A:** one canonical static archive — recommended;
- **B:** true explicit minimal subset plus full archive;
- **C:** object-library/common-core composition;
- **D:** defer until dynamic migration — not recommended.

Do not modify target names, source ownership, GLPK definitions, or link dependencies until this decision is recorded.

## 10. Other pending work

- remove or formally retire the four historical disabled duplicate blocks locally;
- validate shell, worker, main GUI, and independent GUI presets;
- execute Debian package lifecycle validation;
- add selective UBSan and broader ASan/LSan coverage;
- remove Qt5 fallback in a separate PR;
- harden worker tokens and secret handling;
- implement and validate optimizer algorithms separately from ownership work;
- decide parser `FunctionRegistry` disposition;
- validate statistical/biochemical/whole-cell claims against authoritative references.

## 11. Recommended next actions

1. Merge the plugin link evidence documentation after ordinary CI passes.
2. Obtain a human decision on issue #492.
3. If Option A is approved, create `WiPYYYYMMDD/plugin-static-canonical-target`.
4. Implement only the canonical static target correction and compatibility handling.
5. Validate GLPK-present and GLPK-absent modes, ordinary CI, Phase 0, GUI GMDD, sanitizer, Whole-Cell/FBA, and application builds.
6. Keep dynamic ABI migration, Qt cleanup, worker security, and scientific validation in separate PRs.

## 12. Guidance for the next AI assistant

First read `docs/ai_assistants/README.md`, then the latest plugin link evidence, generated introspection evidence, overlap inventory, ownership evidence, runtime-lifecycle evidence, test matrix, consolidation plan, module inventory, and this handoff.

Use `20261`, not `2026-1`, except for historical quotations. Treat the baseline as green only for recorded heads/toolchains. Do not infer scientific correctness from test counts. Preserve the red/green pattern used in prior bounded corrections. Do not select the static plugin architecture implicitly; issue #492 is the authoritative decision boundary.

## 13. Limitations and uncertainties

- Validation used GitHub-hosted Ubuntu runners, not the maintainer's local machine.
- Four duplicate historical tests remain disabled, although equivalent focused tests are active.
- Application startup and user workflows were not executed.
- Debian package lifecycle was not revalidated after trigger correction.
- Sanitizer coverage is focused, not repository-wide.
- Worker networking/authentication and dynamic plugin loading were not executed.
- Optimizer algorithms remain largely unimplemented.
- Scientific correctness still requires independent references and domain review.
- External downstream dependence on either static target name has not been measured.

## 14. Operational result

Result: core validation and bounded ownership/runtime corrections are green. Static plugin target evidence is complete and reproducible: the current full/minimal pair compiles the same source set, the focused test links both archives, and GLPK changes their FBA symbol implementation. Further target changes require the explicit human architecture decision recorded in issue #492.
