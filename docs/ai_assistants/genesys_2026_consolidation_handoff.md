# Handoff — GenESyS 2026 consolidation

## 1. Final task state

- Repository: `rlcancian/Genesys-Simulator`.
- Active development branch: `WorkInProgress`.
- Current integrated checkpoint: `802b8aec7ac129559692bd574e70fd9991aaec1d`.
- Core Phase 0 baseline: `validated` for the recorded GitHub Actions commits and toolchain.
- Consolidation documentation: merged through PR #469 as `9c52b61532b847668adc3be92c780966301bcf7c`.
- Workflow-trigger corrections: merged through PR #470 as `5b24c1e4f31f1b001cd0bd6910fb2c134108fc77`.
- AI-test aggregation correction: merged through PR #471 as `1fca8763cb7a6449cab719950ff74fb59e149b1e`.
- Reusable Phase 0 kernel/smoke workflow: merged through PR #472 as `802b8aec7ac129559692bd574e70fd9991aaec1d`.
- Release readiness: not established.
- Numerical, statistical, biochemical, and biological validity: not established by ordinary test success.

## 2. What was done

- Read the root `README.md` and mandatory `docs/ai_assistants/README.md`.
- Mapped CMake/Ninja/C++23, Qt6, applications, plugins, tools, tests, CI, packaging, and high-risk source areas.
- Created the 2026 consolidation plan, module inventory, test matrix, decision records, research-direction documents, Phase 0 evidence, and this handoff.
- Corrected stale build/application documentation.
- Corrected active workflow filters after `2026-1` was renamed to `20261`.
- Corrected the Debian workflow path filter to the root-level `debian/` tree.
- Added the existing offline AI plugin tests to the ordinary aggregate and kernel direct runner.
- Added `.github/workflows/genesys-phase0-validation.yml` for reusable kernel/smoke validation and evidence artifacts.
- Executed all three primary baseline presets through GitHub Actions.

## 3. Relevant technical comments

- [Build] Canonical baseline: Ubuntu 24.04, CMake 3.24+, Ninja, C++23, compiler extensions disabled.
- [Executed toolchain] CMake 3.31.6, Ninja 1.13.2, G++ 13.3.0.
- [Qt] Qt6-only is the intended support policy; Qt5 fallback code remains technical debt.
- [CI] Historical records of 16/1657 failures and three failing GUI GMDD tests were not reproduced.
- [Tests] Kernel inventory contains 1,696 registered tests: 1,692 passed and 4 were disabled.
- [Tests] Smoke inventory contains 3 tests; all passed.
- [AI] The three offline AI plugin tests are present as tests #495–#497 and passed.
- [Plugins] Current runtime remains statically aggregated; dynamic migration is deferred until target overlap and lifecycle contracts are mapped.
- [Numerics] Green tests do not invalidate the confirmed defects in `SolverDefaultImpl1`.
- [Worker] Controlled academic intranet is the intended deployment profile after security hardening.
- [Optimizer] The current backend remains a scaffold and must not be presented as a mature optimizer.

## 4. Problems encountered

### 4.1 Historical documentation drift

Old documents reported failures no longer reproduced and used obsolete branch, target, path, and application names. Stable guides and the root README were corrected. Historical files remain historical evidence rather than current status.

### 4.2 AI-test aggregation omission

`genesys_test_ai_plugins` existed but was missing from aggregate/direct-runner orchestration. The first cross-directory `add_custom_command(TARGET ...)` attempt failed during CMake configuration. The final solution uses a dedicated run target and valid dependency edges. Both ordinary and kernel paths passed.

### 4.3 Incomplete Phase 0 CI coverage

The ordinary workflow covered only `tests-unit` and GUI GMDD. PR #472 added a separate reusable matrix workflow for `tests-kernel-unit` and `tests-smoke`, exact inventory, logs, versions, and artifacts.

### 4.4 Remaining disabled tests

Four simulator runtime tests are registered but disabled:

1. `SimulatorRuntimeTest.SearchQueueFindsEntityInRangeSavesRankAndRoutesToFoundPort`;
2. `SimulatorRuntimeTest.SearchQueueNotFoundRoutesToPortZeroAndSavesZeroRank`;
3. `SimulatorRuntimeTest.RemoveEqualStartAndEndRankRemovesExactlyOneAndRoutesCorrectly`;
4. `SimulatorRuntimeTest.RemoveRangeRemovesOnlyEntitiesInsideConfiguredInterval`.

These are explicit coverage gaps, not failures.

### 4.5 Scientific/numerical blockers

`SolverDefaultImpl1`, statistical reference validation, plugin target overlap, worker security, optimizer ownership/maturity, and whole-cell scientific claims remain unresolved.

## 5. Corrections and adjustments made

Integrated changes:

- `.github/workflows/genesys-ci.yml`: active branch filter uses `20261`.
- `.github/workflows/genesys-debian-package.yml`: active branch filter uses `20261`; Debian path filter uses `debian/**`.
- `source/tests/CMakeLists.txt`: AI plugin tests included in ordinary and direct-runner graphs.
- `.github/workflows/genesys-phase0-validation.yml`: kernel/smoke matrix validation with inventory and artifacts.
- root and AI-assistant documentation aligned with current architecture and evidence.

No numerical algorithm, plugin ABI, application behavior, package recipe, worker security behavior, or scientific model was modified in this consolidation round.

## 6. Files created or changed

Principal consolidated documentation:

- `README.md`;
- `docs/ai_assistants/README.md`;
- `docs/ai_assistants/build_ci_tests.md`;
- `docs/ai_assistants/branch_workflow.md`;
- `docs/ai_assistants/applications_development.md`;
- `docs/ai_assistants/plugins_development.md`;
- `docs/ai_assistants/tools_and_statistics.md`;
- `docs/ai_assistants/whole_cell_and_sbml.md`;
- `docs/ai_assistants/genesys_2026_consolidation_plan.md`;
- `docs/ai_assistants/genesys_2026_module_inventory.md`;
- `docs/ai_assistants/genesys_2026_test_matrix.md`;
- `docs/ai_assistants/genesys_2026_phase0_ci_evidence_20260720.md`;
- this handoff and associated decision/research plans.

Integrated non-document files:

- `.github/workflows/genesys-ci.yml`;
- `.github/workflows/genesys-debian-package.yml`;
- `.github/workflows/genesys-phase0-validation.yml`;
- `source/tests/CMakeLists.txt`.

## 7. Validation performed

### 7.1 Ordinary unit and GUI checkpoint

- PR #472 ordinary CI run: `29780136801`, run 121.
- `tests-unit` configure/build/CTest: passed.
- Focused GUI GMDD job: passed.

### 7.2 Kernel-focused checkpoint

- Phase 0 run: `29780136722`, run 1.
- Tested PR merge commit: `0d5f32b48510f6c1776ccfb1572f51fb452a6538`.
- Configure: passed.
- Build and direct runner: passed.
- CTest inventory: 1,696.
- Executed/passed: 1,692.
- Disabled: 4.
- Failed: 0.
- Real CTest time: 26.90 seconds.
- Artifact: `genesys-phase0-tests-kernel-unit`, ID `8476435822`.
- Digest: `sha256:bf803a8432328e0bb7555d61c4c01d72e6cafe20391b57380099a8db440bc673`.

AI tests confirmed and passed:

- #495 `AIConversationServiceTest.KeepsIndependentBoundedHistories`;
- #496 `AIPluginTest.BuiltInConnectorExposesSupportAndComponentMetadata`;
- #497 `AIPluginTest.PromptTemplateEvaluatesExpressionsAndEscapesLiteralBraces`.

### 7.3 Smoke checkpoint

- Configure/build/CTest: passed.
- Inventory/executed/passed: 3/3/3.
- Tests: `smoke_simulator_start`, `test_continuous_system`, `test_lsode`.
- Real time: 0.68 seconds.
- Artifact: `genesys-phase0-tests-smoke`, ID `8476325130`.
- Digest: `sha256:3d0f40bb68d05ec0c738c8a34d92d2f4380cf743323015220c25706157871776`.

## 8. State of the consolidation plan

- Phase 0 core baseline: `validated`.
  - `tests-unit`: validated;
  - focused GUI GMDD: validated;
  - `tests-kernel-unit`: validated;
  - `tests-smoke`: validated;
  - kernel and smoke inventories: captured;
  - AI ordinary/direct-runner inclusion: validated.
- Phase 0 broader release evidence: incomplete.
  - applications, package lifecycle, sanitizers, profiling, network/security, and scientific references remain pending.
- Phase 1 numerical/build stabilization: ready to begin with focused failing regression tests.
- Dynamic plugins: deferred.
- Qt5 removal: bounded implementation pending.

## 9. Pending work

Immediate:

1. Investigate and enable or formally retire the four disabled Search/Remove runtime tests.
2. Validate representative shell, worker, main GUI, and independent GUI presets.
3. Execute Debian package build/install/start/uninstall validation.
4. Add opt-in ASan/LSan/UBSan validation for high-risk suites.

Priority technical work:

- add regression tests for every defective `SolverDefaultImpl1` path;
- apply the minimum numerical correction only after those tests fail for the expected reasons;
- generate the plugin source-to-target/link map;
- remove Qt5 fallback in a separate PR;
- harden worker tokens and secret handling;
- close optimizer ownership/copy hazards;
- verify the temporary `Model` lifetime finding;
- decide parser `FunctionRegistry` disposition.

## 10. Recommended next actions

1. Merge the documentation evidence update after ordinary CI passes.
2. Open a focused test-only PR for `SolverDefaultImpl1`.
3. Keep the first solver PR limited to regression tests and characterization; do not repair the implementation in the same first commit.
4. Address the four disabled runtime tests in a separate bounded PR unless directly required by the solver work.
5. Keep Qt6 cleanup, plugin architecture, worker security, and optimizer work isolated.

## 11. Guidance for the next AI assistant

First read `docs/ai_assistants/README.md`, then the Phase 0 evidence, test matrix, consolidation plan, module inventory, decisions addendum, and this handoff.

Use `20261`, not `2026-1`, except for historical quotations. Treat the core baseline as green only for the recorded commits/toolchain. Do not infer scientific correctness from the test count. Do not alter `SolverDefaultImpl1` before adding focused failing tests. Do not begin dynamic plugin migration before mapping current source overlap and lifecycle/ownership boundaries.

## 12. Limitations and uncertainties

- Validation used GitHub-hosted Ubuntu runners, not the maintainer's local machine.
- Four tests remain disabled.
- Application startup and user workflows were not executed.
- Debian package lifecycle was not revalidated after trigger correction.
- Sanitizers, Valgrind, profiling, worker networking, and authentication were not executed.
- Scientific correctness still requires independent references and domain review.

## 13. Operational result

Result: the GenESyS core Phase 0 baseline is green and reproducible through integrated GitHub Actions workflows. The repository now has validated ordinary, kernel-focused, smoke, GUI GMDD, and AI plugin test paths. The next safe implementation phase is focused regression testing for `SolverDefaultImpl1`, while application, packaging, sanitizer, security, plugin-architecture, and scientific validation remain separate workstreams.