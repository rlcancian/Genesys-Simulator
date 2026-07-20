# Build, CI, and Tests

## 1. Purpose

Stable reference for the current GenESyS build, CI, and test workflows. Historical files and older `current_plans.md` sections must be checked against current CMake and the latest execution evidence.

Also read:

- `genesys_2026_phase0_ci_evidence_20260720.md`;
- `genesys_2026_test_matrix.md`;
- `genesys_2026_consolidation_handoff.md`.

## 2. Confirmed build baseline

- Ubuntu 24.04 primary CI baseline;
- minimum CMake 3.24;
- Ninja generator;
- C++23 required;
- compiler extensions disabled;
- Qt6-only project policy;
- Google Test system package when compatible, bundled fallback otherwise.

Executed Phase 0 toolchain:

- CMake 3.31.6;
- Ninja 1.13.2;
- G++ 13.3.0.

Qt5 fallback code may still exist but is not part of the intended supported-platform contract.

## 3. Primary validation presets

### 3.1 Ordinary unit baseline

```bash
cmake --preset tests-unit
cmake --build --preset tests-unit --parallel "$(nproc)"
ctest --preset tests-unit --output-on-failure
```

- directory: `build/tests-unit`;
- target: `genesys_kernel_unit_tests`;
- CTest label: `unit`;
- worker support enabled for worker tests;
- smoke and shell disabled.

Status: `validated` in current GitHub Actions checkpoints.

### 3.2 Kernel-focused baseline

```bash
cmake --preset tests-kernel-unit
cmake --build --preset tests-kernel-unit --parallel "$(nproc)"
ctest --preset tests-kernel-unit --output-on-failure
```

- directory: `build/tests-kernel-unit`;
- target: `genesys_kernel_unit_tests_run`;
- build target executes the direct test-runner graph;
- CTest then executes the registered unit inventory.

Current evidence:

- 1,696 registered;
- 1,692 passed;
- 4 disabled;
- 0 failed;
- AI plugin tests #495–#497 passed.

Status: `validated` for Phase 0 run `29780136722`.

### 3.3 Smoke baseline

```bash
cmake --preset tests-smoke
cmake --build --preset tests-smoke --parallel "$(nproc)"
ctest --preset tests-smoke --output-on-failure
```

Current evidence:

- 3 registered/executed/passed;
- simulator start;
- continuous system;
- LSODE.

Status: `validated` for Phase 0 run `29780136722`.

### 3.4 Inventory commands

```bash
ctest --preset tests-unit -N
ctest --preset tests-kernel-unit -N
ctest --preset tests-smoke -N
```

Never reuse a historical count as a current count.

## 4. GitHub Actions workflows

### 4.1 Ordinary CI

File:

```text
.github/workflows/genesys-ci.yml
```

Jobs:

1. configure/build/CTest `tests-unit`;
2. build/run three focused GUI GMDD tests and upload diagnostics.

Current PR target branches include `20261`, `WorkInProgress`, `WiP20261`, `currentStable`, and `master`.

The old `2026-1` filter was corrected through PR #470.

### 4.2 Phase 0 validation

File:

```text
.github/workflows/genesys-phase0-validation.yml
```

Purpose:

- execute `tests-kernel-unit` and `tests-smoke` in isolated matrix jobs;
- capture CTest inventory;
- execute CTest;
- capture toolchain/HEAD/diagnostics;
- upload per-preset artifacts.

Triggers:

- `workflow_dispatch`;
- relevant PR changes against `WorkInProgress`.

This workflow complements ordinary CI; it does not replace it.

### 4.3 Debian package workflow

File:

```text
.github/workflows/genesys-debian-package.yml
```

The active package tree is root-level `debian/`. PR #470 corrected the branch name and path filter. The package lifecycle still requires a new execution after that correction.

## 5. Current applications and presets

### Shell/model-specific

- `genesys_shell`;
- `terminal-smart`;
- `terminal-model-specific`;
- `genesys_modelspecific_app`;
- `terminal-smart-hold-search-remove`;
- `terminal-smart-ai-assistant`.

### Worker

- `worker-app`;
- `genesys_worker_app`.

Current source tree: `source/applications/worker/`. Do not use the historical `web-app` name as current architecture.

### GUI

- `gui-app`;
- `gui-httpworker`;
- `gui-dataanalyser`;
- `gui-optimizer`;
- `gui-ai-assistant`.

Use Qt6. Focused GUI unit success is not equivalent to standalone application startup validation.

## 6. Current disabled tests

The kernel inventory contains four disabled simulator runtime tests concerning SearchQueue and Remove behavior. These are explicit coverage gaps and should be addressed in a separate bounded PR.

## 7. Safe validation order

For cross-cutting code changes:

1. record branch/commit/toolchain;
2. run ordinary `tests-unit`;
3. run focused tests for the changed area;
4. run `tests-kernel-unit` when kernel/parser/plugin/tool behavior is affected;
5. run `tests-smoke` when runtime/continuous behavior is affected;
6. build/start the affected application preset;
7. run sanitizers for ownership/UB/memory changes;
8. validate packaging separately when build/install/package behavior changes.

## 8. Interpretation rules

- Configure success does not prove build success.
- Build success does not prove test success.
- A green CTest inventory does not prove unregistered code paths.
- Generated method inventory is structural, not behavioral coverage.
- Numerical/statistical passing tests require a defined oracle before they support scientific claims.
- Focused GUI tests do not prove application startup or user workflow.
- Later dated evidence supersedes older current-status claims.

## 9. Open follow-up work

- investigate the four disabled runtime tests;
- validate shell, worker, main GUI, independent GUIs, and representative model-specific applications;
- execute Debian package build/install/start/uninstall;
- add opt-in ASan/LSan/UBSan paths;
- characterize `SolverDefaultImpl1` with focused failing tests;
- map overlapping full/minimal plugin targets;
- remove Qt5 fallback in a dedicated PR;
- validate worker authentication/security separately.