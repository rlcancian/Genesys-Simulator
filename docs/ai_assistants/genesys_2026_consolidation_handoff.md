# Handoff — GenESyS 2026 consolidation

## 1. Final task state

- Overall status: `partially-validated`.
- Repository: `rlcancian/Genesys-Simulator`.
- Consolidation base branch: `WorkInProgress`.
- Original inspected base commit: `b25a4d2ec31abe27f1bf3597a5135fa4828fbc35`.
- Documentation branch: `audit/genesys-2026-consolidation-plan-20260720`.
- Documentation pull request: PR #469, targeting `WorkInProgress`.
- PR #469 remains documentation-only.
- PR #470 was merged into `WorkInProgress` at `5b24c1e4f31f1b001cd0bd6910fb2c134108fc77`.
- PR #471 was merged into `WorkInProgress` at `1fca8763cb7a6449cab719950ff74fb59e149b1e`.
- Ordinary `tests-unit` CI is green for the documentation, workflow-trigger, and AI-test-aggregation checkpoints.
- `tests-kernel-unit` and `tests-smoke` are still not explicitly executed in the available checkpoints.
- Release readiness and scientific correctness are not established.

## 2. What was done

- Read the root `README.md` and mandatory `docs/ai_assistants/README.md`.
- Inspected the CMake/Ninja/C++23 graph, presets, applications, plugins, tools, tests, CI, and selected high-risk sources.
- Created the consolidation plan, module inventory, test matrix, human-decision records, future research plans, Phase 0 evidence, and this handoff.
- Corrected the root README and stable build/CI guidance to use current targets, paths, options, and presets.
- Verified that `20261` is the renamed former `2026-1` branch.
- Recorded that promotion to `20262` is an end-of-semester activity, not a current release gate.
- Verified current green ordinary CI and focused GUI GMDD tests.
- Opened, validated, and integrated PR #470 for active workflow trigger corrections.
- Opened, diagnosed, corrected, validated, and integrated PR #471 so offline AI plugin tests participate in the ordinary aggregate and kernel direct-runner graph.

## 3. Relevant technical comments

- [Build] Canonical build: CMake 3.24+, Ninja, C++23, extensions disabled.
- [Qt] Qt6-only is the intended baseline; remaining Qt5 fallbacks are implementation debt.
- [CI] Historical reports of 16/1657 failures and three GUI GMDD failures were not reproduced by current successful runs.
- [CI] PR #470 changed only activation filters; no build command, dependency, job, preset, or test behavior changed.
- [Tests] `genesys_test_ai_plugins` uses local fakes/dummy connectors and requires no network or credential.
- [Tests] PR #471 now builds and discovers the AI tests in ordinary `tests-unit`; the explicit `tests-kernel-unit` execution remains pending.
- [Plugins] Current runtime is still statically aggregated; future independent dynamic plugins require a stable C ABI with opaque handles and versioned function tables.
- [Numerics] Green unit tests do not invalidate the confirmed defects in `SolverDefaultImpl1`.
- [Statistics] Scientific validity requires authoritative references, explicit parameterizations, and independent test vectors.
- [Worker] Intended profile is a controlled academic intranet after authentication and secret/token hardening.
- [Optimizer] Current backend remains an internal scaffold, not a complete optimization capability.
- [Whole-cell] Current implementation must not overclaim predictive biological validity.

## 4. Problems encountered

### 4.1 No local execution environment

- No local compiler, Qt runtime, package manager, sanitizer, or Git worktree was available.
- GitHub Actions was used for the executable checkpoints available through the connector.
- Status: `needs-local-validation` for paths not present in CI.

### 4.2 Partial Phase 0 coverage

Validated:

- `tests-unit` configure/build/CTest;
- focused GUI GMDD executable and three selected tests;
- ordinary aggregate after AI plugin inclusion.

Still missing:

- `tests-kernel-unit`;
- `tests-smoke`;
- exact `ctest -N` inventory;
- application startup matrix;
- sanitizers;
- package lifecycle.

### 4.3 AI test aggregation defect

- Confirmed defect: `genesys_test_ai_plugins` existed but was omitted from aggregate/direct-runner orchestration.
- First correction attempt failed because `add_custom_command(TARGET ...)` cannot modify a target created in another CMake directory.
- Final correction: dedicated `genesys_test_ai_plugins_run` target plus cross-directory dependencies.
- Ordinary CI result: success.
- Status: `implemented-and-validated` for `tests-unit`; direct runner still needs the explicit kernel preset run.

### 4.4 Workflow activation drift

- Ordinary CI filtered the obsolete branch name `2026-1` instead of `20261`.
- Debian workflow used the non-matching path `../../packaging/debian/**` instead of `debian/**`.
- PR #470 corrected both and passed ordinary CI.
- Debian package execution itself remains unvalidated.

### 4.5 Numerical correctness blocker

- `SolverDefaultImpl1` has confirmed undefined-behavior/correctness concerns.
- Statistical hypothesis code depends on this legacy integrator.
- Status: `blocked` for scientific validity until focused regression tests exist.

### 4.6 Plugin target overlap

- Full and minimal static plugin targets compile overlapping recursive component sources.
- Status: mapping required before target restructuring or dynamic-plugin work.

### 4.7 Security-sensitive implementation

- Worker tokens use a non-cryptographic PRNG.
- AI secret storage composes a secret-bearing shell command.
- Status: blocks approved deployment until hardened and tested.

## 5. Corrections and adjustments made

Integrated functional/build-governance corrections:

- PR #470:
  - `.github/workflows/genesys-ci.yml`: `2026-1` → `20261`;
  - `.github/workflows/genesys-debian-package.yml`: `2026-1` → `20261`;
  - Debian trigger path: `../../packaging/debian/**` → `debian/**`.
- PR #471:
  - adds `genesys_test_ai_plugins` to `genesys_kernel_unit_tests` dependencies;
  - creates a dedicated offline AI-test run target;
  - adds it to `genesys_kernel_unit_tests_run` dependencies.

Documentation corrections in PR #469 include:

- current README build/application instructions;
- current branch names and promotion policy;
- current CI evidence;
- current GUI GMDD status;
- current test aggregation status;
- explicit distinction between software validation and scientific validity.

## 6. Files created or changed

Principal documentation artifacts:

- `docs/ai_assistants/genesys_2026_consolidation_plan.md`;
- `docs/ai_assistants/genesys_2026_test_matrix.md`;
- `docs/ai_assistants/genesys_2026_module_inventory.md`;
- `docs/ai_assistants/genesys_2026_consolidation_handoff.md`;
- `docs/ai_assistants/genesys_2026_phase0_ci_evidence_20260720.md`;
- `docs/ai_assistants/genesys_2026_human_decisions.md`;
- `docs/ai_assistants/genesys_2026_decisions_addendum_20260720.md`;
- `docs/ai_assistants/genesys_numerical_statistical_references_plan.md`;
- `docs/ai_assistants/genesys_multiobjective_optimizer_future_plan.md`;
- `docs/ai_assistants/genesys_ai_virtual_cell_research_direction.md`.

Integrated codebase files outside PR #469:

- `.github/workflows/genesys-ci.yml`;
- `.github/workflows/genesys-debian-package.yml`;
- `source/tests/CMakeLists.txt`.

## 7. Validation performed

### 7.1 Documentation checkpoint

- PR #469 workflow run `29772903692`, run 115.
- Merge commit tested: `bf8bfd93a2b002c2098811cc339900eef516942d`.
- Configure `tests-unit`: passed.
- Build `tests-unit`: passed.
- CTest `tests-unit`: passed.
- GUI GMDD diagnostic job: passed.

### 7.2 Workflow-trigger correction

- PR #470 workflow run `29772492281`, run 113.
- Tested merge commit: `c30139e8ff06111f91d95b7f69a863aaf203d219`.
- Configure/build/CTest: passed.
- GUI GMDD job: passed.
- Merged as `5b24c1e4f31f1b001cd0bd6910fb2c134108fc77`.

### 7.3 AI test aggregation correction

- PR #471 workflow run `29773299985`, run 117.
- Tested merge commit: `7ee8eeb73490da5fb1aae3cb256a9339f2da7f59`.
- Configure/build/CTest: passed after the CMake-scope correction.
- Aggregate build included `genesys_test_ai_plugins`.
- GUI GMDD job: passed.
- Merged as `1fca8763cb7a6449cab719950ff74fb59e149b1e`.

## 8. State of the consolidation plan

- Phase 0: `partially-validated`.
  - ordinary `tests-unit`: validated;
  - focused GUI GMDD tests: validated;
  - active CI branch filters: corrected and integrated;
  - ordinary AI plugin test aggregation: corrected and integrated;
  - `tests-kernel-unit`: pending;
  - `tests-smoke`: pending;
  - exact test inventory: pending;
  - application/package/sanitizer matrix: pending.
- Phase 1 functional stabilization: not started as a consolidated implementation phase.
- Dynamic plugin implementation: deferred pending source/target and lifecycle mapping.
- Qt5 removal: policy decided; bounded implementation PR pending.
- Numerical/statistical references: acquisition plan recorded; implementation validation pending.

## 9. Pending work

### 9.1 Immediate validation

1. Run `tests-kernel-unit`.
2. Run `tests-smoke`.
3. Capture `ctest -N` for all baseline presets.
4. Confirm direct execution of `genesys_test_ai_plugins_run` through the kernel preset.
5. Build representative application presets.
6. Execute the Debian package workflow and package lifecycle.

### 9.2 Priority technical work

- add failing regression tests for `SolverDefaultImpl1`;
- apply the minimum solver correction or quarantine invalid overloads;
- map and remove unjustified plugin target overlap;
- remove Qt5 fallback in a dedicated PR;
- replace worker token randomness and secret-command handling;
- close optimizer ownership/copy hazards;
- verify the temporary `Model` lifetime finding;
- decide parser `FunctionRegistry` disposition.

## 10. Recommended next actions

1. Revalidate PR #469 against the new `WorkInProgress` base containing merges #470 and #471.
2. Merge PR #469 only after the new documentation head is green.
3. Add explicit CI/manual-dispatch paths for `tests-kernel-unit` and `tests-smoke`, or run them locally and attach logs.
4. Open a dedicated Qt6-only cleanup PR.
5. Open a dedicated regression-test PR for `SolverDefaultImpl1` before changing numerical code.
6. Keep plugin architecture, worker security, optimizer development, and scientific reference work in separate reviewable changes.

## 11. Guidance for the next AI assistant

First read:

1. `docs/ai_assistants/README.md`;
2. `genesys_2026_decisions_addendum_20260720.md`;
3. `genesys_2026_phase0_ci_evidence_20260720.md`;
4. `genesys_2026_consolidation_plan.md`;
5. `genesys_2026_test_matrix.md`;
6. `genesys_2026_module_inventory.md`;
7. this handoff.

Use `20261`, not `2026-1`, except when quoting historical identifiers. Do not claim Phase 0 complete until kernel/smoke and exact inventory are captured. Do not infer scientific correctness from green unit tests. Add regression tests before changing numerical algorithms. Do not start dynamic plugin migration before the current static graph and ownership/lifecycle contracts are mapped.

## 12. Limitations and uncertainties

- No local compiler/runtime was available.
- Exact CTest counts were not extracted.
- `tests-kernel-unit` and `tests-smoke` were not executed.
- Debian package generation was not executed after the trigger correction.
- Application startup, network behavior, sanitizers, and profiling remain unvalidated.
- Numerical, statistical, biochemical, and biological correctness requires independent references and domain review.
- PR #469 must receive a new CI result after this update.

## 13. Operational result

Result: workflow activation and AI plugin test aggregation defects were corrected, validated, and merged into `WorkInProgress`; the consolidation documentation was updated to reflect those integrations. Phase 0 remains partially validated because kernel-focused, smoke, exact inventory, application, sanitizer, and package checkpoints are still pending.