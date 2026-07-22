# Handoff — GenESyS 2026 consolidation

## 1. Final task state

- Repository: `rlcancian/Genesys-Simulator`.
- Active development branch: `WorkInProgress`.
- Current integrated checkpoint: `8f42f50992b0bd53759018f09e46f48434839bf7`.
- Core Phase 0 baseline: `validated` for the recorded GitHub Actions commits and toolchain.
- Current kernel inventory: 1,721 registered; 1,717 executed/passed; 4 historical duplicates disabled; 0 failures.
- Runtime lifecycle, focused ownership sanitizer, and optimizer copy/move barrier: integrated.
- Static plugin target inventory and generated codemodel/link/symbol workflows: integrated through PR #491.
- Standalone shell validation: integrated through PR #495.
- Standalone worker public-health validation: integrated through PR #499.
- Plugin architecture implementation: blocked on human decision issue #492.
- Shell `autoloadplugins.txt` deployment: blocked on human decision issue #496.
- Worker bind-address policy: blocked on human security decision issue #500.
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
- Added a focused standalone shell build/argv workflow.
- Added a focused standalone worker build/localhost-health workflow.
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
- [Shell] Preset/build/argv/plugin-count/exit are validated; `autoloadplugins.txt` deployment remains unresolved.
- [Worker] Preset/build/loopback health/clean one-request exit are validated. The actual listener is `0.0.0.0`, which conflicts with the intended explicit-private-interface profile until issue #500 is resolved.

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

The temporary `Model` was not registered in `ModelManager` and was never destroyed. Focused LeakSanitizer evidence measured 27,533 bytes / 470 allocations initially and zero after bounded ownership corrections.

### 4.7 Optimizer shallow-copy hazard

Seven owning raw list pointers were manually deleted, while implicit copy semantics could duplicate them. Copy and move construction/assignment are now explicitly deleted. Algorithms remain deferred.

### 4.8 Static plugin target duplication and GLPK divergence

Generated evidence confirms:

- full and minimal compile the same 84 `.cpp` sources;
- minimal is the runtime-effective archive in the validated codemodel;
- full receives `GENESYS_HAVE_GLPK`; minimal does not;
- the focused diffusion test links both archives;
- their symbols diverge when GLPK is present.

No duplicate-symbol linker failure was demonstrated because the focused build succeeds. The graph remains semantically ambiguous and duplicates compilation/storage.

### 4.9 Shell autoload deployment gap

The shell workflow succeeded through the static plugin fallback, but the artifact recorded failure to open `autoloadplugins.txt` beside the executable. Issue #496 is the decision boundary for file placement/fallback policy.

### 4.10 Worker wildcard bind

The worker workflow succeeded through loopback, but `ss` recorded `0.0.0.0:<port>`. The selected controlled-intranet profile requires an explicitly selected private interface/address and deny-by-default behavior. Issue #500 is the decision boundary for the exact bind contract.

### 4.11 Remaining scientific and architectural blockers

Authoritative statistical validation, worker authentication/security, optimizer maturity, whole-cell scientific claims, GUI/package validation, Qt5 cleanup, and dynamic plugin ABI remain unresolved.

## 5. Integrated changes

Principal integrated PRs:

- #470–#473: workflow/test aggregation and Phase 0 evidence;
- #474–#480: solver and runtime lifecycle stabilization;
- #483: plugin-completion lifetime and sanitizer;
- #485: optimizer copy/move ownership barrier;
- #488–#491: plugin target/source/codemodel/link/symbol evidence;
- #495: standalone shell validation workflow;
- #499: standalone worker validation workflow.

No plugin ABI migration, package redesign, worker security redesign, optimizer algorithm, whole-cell model, or public persistence format was changed.

## 6. Principal evidence and workflows

Evidence documents:

- `genesys_2026_test_matrix.md`;
- `genesys_2026_runtime_lifecycle_evidence_20260721.md`;
- `genesys_2026_ownership_evidence_20260721.md`;
- `genesys_plugin_target_overlap_inventory_20260721.md`;
- `genesys_plugin_target_introspection_evidence_20260721.md`;
- `genesys_plugin_target_link_evidence_20260722.md`;
- `genesys_shell_validation_evidence_20260722.md`;
- `genesys_worker_validation_evidence_20260722.md`;
- this handoff.

Principal workflows:

- `.github/workflows/genesys-ci.yml`;
- `.github/workflows/genesys-phase0-validation.yml`;
- `.github/workflows/genesys-plugin-lifetime-sanitizer.yml`;
- `.github/workflows/genesys-plugin-target-introspection.yml`;
- `.github/workflows/genesys-plugin-target-link-evidence.yml`;
- `.github/workflows/genesys-shell-validation.yml`;
- `.github/workflows/genesys-worker-validation.yml`.

## 7. Validation performed

### 7.1 Core baseline

PR #485 ordinary run `29833758692`: `tests-unit` configure/build/CTest and focused GUI GMDD passed.

PR #485 Phase 0 run `29833758797`:

- registered: 1,721;
- executed/passed: 1,717;
- disabled: 4;
- failed: 0;
- artifact ID: `8496613101`.

### 7.2 Focused sanitizer

PR #483 run `29831405860`: ASan/LeakSanitizer passed with exit code 0 and artifact ID `8495543015`.

### 7.3 Plugin graph evidence

- PR #489 codemodel artifact ID: `8505306513`;
- PR #491 GLPK-present artifact ID: `8512049198`;
- PR #491 GLPK-absent artifact ID: `8512054888`.

### 7.4 Standalone shell

PR #495:

- focused run: `29885199488`;
- ordinary run: green;
- artifact ID: `8516303352`;
- build/argv/plugin-count/exit passed;
- autoload deployment unresolved.

### 7.5 Standalone worker

PR #499:

- focused run: `29896225187`;
- ordinary run: `29896225132`;
- artifact ID: `8520123900`;
- listener: `0.0.0.0:44559`;
- request: `127.0.0.1:44559/health`;
- HTTP 200 and exact JSON passed;
- request-limited exit and no residual process passed.

## 8. State of the consolidation plan

- Phase 0 core baseline: `validated`.
- Bounded stabilization completed for solver, Search/Remove, lifecycle, plugin-completion ownership, and optimizer ownership.
- Static plugin evidence: `complete`; correction blocked on issue #492.
- Standalone shell: `partially-validated`; autoload deployment blocked on issue #496.
- Standalone worker: `partially-validated`; bind policy blocked on issue #500; broader security hardening pending.
- Main and independent Qt6 GUIs: startup/workflow validation pending.
- Debian package lifecycle: pending.
- Dynamic plugins: deferred.
- Qt5 removal: pending.

## 9. Current decision blockers

### Issue #492 — plugin target architecture

- A: one canonical static archive — recommended;
- B: true explicit minimal subset plus full archive;
- C: object-library/common-core composition;
- D: defer until dynamic migration — not recommended.

### Issue #496 — shell autoload deployment

Select file placement/fallback/diagnostic behavior before changing production.

### Issue #500 — worker bind contract

Select explicit required address, loopback default with private override, deployment-only configuration, or wildcard preservation. Loopback default with explicit private override is recommended; wildcard preservation is not recommended.

## 10. Other pending work

- remove or formally retire the four historical disabled duplicate blocks locally;
- validate main GUI and independent GUI presets;
- execute Debian package lifecycle validation;
- add selective UBSan and broader ASan/LSan coverage;
- remove Qt5 fallback in a separate PR;
- harden worker authentication, tokens, quotas, TLS and isolation;
- implement and validate optimizer algorithms separately from ownership work;
- decide parser `FunctionRegistry` disposition;
- validate statistical/biochemical/whole-cell claims against authoritative references.

## 11. Recommended next actions

1. Merge the worker evidence documentation after ordinary CI passes.
2. Keep issues #492, #496, and #500 as explicit human decision boundaries.
3. Start one bounded standalone Qt6 GUI validation using an existing preset.
4. Validate configure/build and a headless startup/clean-exit path under `QT_QPA_PLATFORM=offscreen` or Xvfb.
5. Do not remove Qt5 fallback or alter plugin/worker behavior in the GUI-validation PR.
6. Update matrix and handoff from the resulting artifact.

## 12. Guidance for the next AI assistant

First read `docs/ai_assistants/README.md`, then the latest worker and shell evidence, plugin link evidence, test matrix, consolidation plan, module inventory, and this handoff.

Use `20261`, not `2026-1`, except for historical quotations. Treat the baseline as green only for recorded heads/toolchains. Do not infer scientific correctness from test counts. Preserve bounded changes and explicit human decision boundaries.

## 13. Limitations and uncertainties

- Validation used GitHub-hosted Ubuntu runners, not the maintainer's local machine.
- Four duplicate historical tests remain disabled, although equivalent focused tests are active.
- Shell model load/run was not executed.
- Worker protected endpoints and security controls were not executed.
- Main and independent GUI startup were not executed.
- Debian package lifecycle was not revalidated.
- Sanitizer coverage is focused, not repository-wide.
- Optimizer algorithms remain largely unimplemented.
- Scientific correctness still requires independent references and domain review.

## 14. Operational result

Result: core validation, bounded ownership/runtime corrections, plugin evidence, shell startup, and worker public-health startup are green and reproducible. The next safe independent workstream is a bounded standalone Qt6 GUI validation. Plugin architecture, shell autoload deployment, and worker bind/security changes remain blocked on explicit decisions.