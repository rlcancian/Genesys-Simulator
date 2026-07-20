# Build, CI, and Tests

## 1. Purpose

This document is the stable AI-assistant reference for current GenESyS build, CI, and automated-test workflows.

Historical recommendations under `docs/ai_assistants/oldies/` and older sections of `current_plans.md` must be revalidated against the current CMake files and the latest dated execution-evidence document before they are treated as current facts.

For the current consolidation checkpoint, also read:

- `genesys_2026_test_matrix.md`;
- `genesys_2026_phase0_ci_evidence_20260720.md`;
- `genesys_2026_consolidation_handoff.md`.

## 2. Current confirmed build baseline

### 2.1 Toolchain and build system

Confirmed in `CMakeLists.txt` and `CMakePresets.json`:

- minimum CMake: 3.24;
- preset schema: version 6;
- generator: Ninja;
- C++ standard: C++23;
- C++ standard required: yes;
- compiler extensions: disabled;
- kernel, parser, and plugins enabled in the base preset;
- GUI, worker, shell, model-specific applications, tests, and smoke tests controlled by dedicated options/presets.

Qt policy:

- project policy is Qt6-only;
- current source/CMake may still contain Qt5 fallback paths;
- those fallbacks are legacy implementation debt and must not be interpreted as a supported-platform requirement.

## 3. Current configure/build/test presets

### 3.1 Unit baseline

Run from the repository root:

```bash
cmake --list-presets=all
cmake --preset tests-unit
cmake --build --preset tests-unit --parallel "$(nproc)"
ctest --preset tests-unit --output-on-failure
```

Confirmed preset behavior:

- build directory: `build/tests-unit`;
- tests enabled;
- worker application support enabled because worker tests depend on it;
- smoke tests disabled;
- terminal examples disabled;
- shell application disabled;
- build target: `genesys_kernel_unit_tests`;
- CTest preset filters by the `unit` label.

Executed evidence:

- PR #469 head: `a60ca65aae4147a7d9b14bdadd9e8d39958bcaaf`;
- GitHub Actions run: `29771060564`;
- Ubuntu runner: 24.04.4 LTS;
- configure: passed;
- build: passed;
- CTest: passed.

Scope limitation:

A green `tests-unit` run proves only the targets registered and executed by that preset. It does not prove that every declared test executable is included. In particular, the current aggregation status of `genesys_test_ai_plugins` must still be verified explicitly.

### 3.2 Kernel-focused baseline

```bash
cmake --preset tests-kernel-unit
cmake --build --preset tests-kernel-unit --parallel "$(nproc)"
ctest --preset tests-kernel-unit --output-on-failure
```

Confirmed preset behavior:

- build directory: `build/tests-kernel-unit`;
- configure preset inherits from `tests-unit`;
- build target: `genesys_kernel_unit_tests_run`.

Current validation status:

- preset and target confirmed by repository inspection;
- not executed by the recorded Phase 0 GitHub Actions run;
- status: `needs-local-validation` or additional CI validation.

### 3.3 Smoke baseline

```bash
cmake --preset tests-smoke
cmake --build --preset tests-smoke --parallel "$(nproc)"
ctest --preset tests-smoke --output-on-failure
```

Confirmed preset behavior:

- build directory: `build/tests-smoke`;
- tests enabled;
- worker support enabled;
- smoke tests enabled;
- shell application disabled.

Current validation status:

- preset and smoke targets confirmed by repository inspection;
- not executed by the recorded Phase 0 GitHub Actions run;
- status: `needs-local-validation` or additional CI validation.

### 3.4 Test inventory commands

Every baseline run should also capture the registered tests:

```bash
ctest --preset tests-unit -N
ctest --preset tests-kernel-unit -N
ctest --preset tests-smoke -N
```

Do not reuse a historical test count as the current count. Record the exact count from the current commit and environment.

## 4. Current application presets

### 4.1 Shell and model-specific applications

Current presets include:

- `genesys_shell` — builds target `genesys_shell`;
- `terminal-smart` — builds a selected smart example through `GENESYS_TERMINAL_EXAMPLE`;
- `terminal-model-specific` — builds a selected model-specific source;
- `genesys_modelspecific_app` — builds target `genesys_modelspecific_app`;
- `terminal-smart-hold-search-remove` — selected smart example;
- `terminal-smart-ai-assistant` — selected AI-assistant smart example.

`terminal-app` remains a hidden compatibility/configuration preset. Prefer the explicit public preset `genesys_shell` for the standalone shell.

### 4.2 Worker

Current presets:

- `worker-app`;
- `genesys_worker_app`.

Both build the worker application target `genesys_worker_app` through the current `source/applications/worker/` tree.

Do not refer to the current application as the active `web-app` tree unless discussing historical names or compatibility aliases.

### 4.3 GUI applications

Current GUI presets include:

- `gui-app` — main GUI target `genesys_gui`;
- `gui-httpworker` — target `genesys_httpworker_gui_application`;
- `gui-dataanalyser` — standalone Data Analyser GUI;
- `gui-optimizer` — standalone Optimizer GUI;
- `gui-ai-assistant` — standalone AI Assistant GUI.

Use Qt6 dependencies. For headless CI or test execution, use `QT_QPA_PLATFORM=offscreen` where appropriate.

A successful focused GUI unit executable is not equivalent to a standalone GUI configure/build/startup validation.

## 5. Current GitHub Actions ordinary CI

Workflow:

```text
.github/workflows/genesys-ci.yml
```

Current job structure:

1. `Configure, build and test tests-unit`;
2. `Diagnose GUI GMDD tests`.

The ordinary job:

- runs on Ubuntu 24.04;
- installs CMake, Ninja, GCC/build-essential, Python development files, Qt6, libSBML, ngspice, R, Octave, and OpenGL dependencies;
- configures `tests-unit`;
- builds the `tests-unit` preset;
- runs CTest with `tests-unit`.

The GUI diagnostic job:

- configures `tests-unit`;
- builds only `genesys_test_gui_gmdd_layout`;
- executes three focused GMDD tests;
- uploads an artifact regardless of success/failure.

Current execution evidence:

- workflow run `29771060564` completed successfully;
- ordinary unit job passed;
- all three GUI GMDD tests passed;
- diagnostic artifact ID: `8472919224`.

Older documentation that calls those three tests currently failing is superseded by `genesys_2026_phase0_ci_evidence_20260720.md` for the validated head.

## 6. Branch-trigger policy

Current branch names include:

- `20261` — renamed former `2026-1` semester-stable branch;
- `WorkInProgress` — active development line;
- `WiP20261` — maintainer working branch;
- `currentStable`;
- `master`.

At the time of this documentation update, the `WorkInProgress` version of `.github/workflows/genesys-ci.yml` still contains the obsolete target filter `2026-1`.

A separate bounded PR corrects active workflow filters:

- PR #470: `ci: align renamed semester and Debian triggers`;
- ordinary CI filter: `2026-1` -> `20261`;
- Debian PR filter: `2026-1` -> `20261`;
- Debian path filter: `../../packaging/debian/**` -> `debian/**`.

Do not document those trigger corrections as integrated until PR #470 is merged.

`20262` must not be added casually. Its promotion and workflow policy will be decided near the end of the second semester of 2026.

## 7. Debian package workflow

Workflow:

```text
.github/workflows/genesys-debian-package.yml
```

The active Debian packaging tree is at repository-root `debian/`, including `debian/rules`.

The package workflow:

- builds binary packages with `dpkg-buildpackage`;
- validates AppStream metadata;
- runs lintian diagnostically;
- uploads package and diagnostic artifacts;
- fails if no `.deb` is produced.

Current validation status:

- prior package artifacts exist in project history;
- the trigger-only corrections in PR #470 are not yet validated by a Debian run;
- PR #470 targets `WorkInProgress`, while the Debian workflow currently filters PR bases to `WiP20261` and `20261`;
- validate through manual dispatch, a future eligible packaging PR, or a separate decision to include `WorkInProgress`.

Packaging validation remains separate from ordinary unit CI.

## 8. Safe validation order

For a build-system or cross-cutting code change:

1. record branch and commit;
2. list presets;
3. configure/build/test `tests-unit`;
4. capture `ctest --preset tests-unit -N`;
5. configure/build/test `tests-kernel-unit` when kernel/tool/plugin code is affected;
6. configure/build/test `tests-smoke` when runtime behavior is affected;
7. build the affected shell, worker, GUI, or model-specific preset independently;
8. run focused regression tests before and after the patch;
9. run sanitizers for ownership, lifetime, UB, or numerical-memory concerns;
10. validate Debian packaging separately when build/install/package behavior changes.

## 9. Interpretation rules

- A successful configure does not prove build success.
- A successful build does not prove test success.
- A green aggregate does not prove every declared executable was included.
- A passing numerical unit test does not establish scientific correctness without a defined oracle and tolerance.
- A focused GUI test does not prove GUI startup or user workflow behavior.
- Historical CI results apply only to their recorded branch, commit, toolchain, dependencies, and test registration.
- Later dated execution evidence supersedes older current-status claims.

## 10. Open follow-up tasks

- execute and record `tests-kernel-unit`;
- execute and record `tests-smoke`;
- verify the exact test count and intended target inclusion;
- ensure `genesys_test_ai_plugins` is part of the ordinary baseline;
- add or restore opt-in ASan/LSan/UBSan presets after the ordinary baseline is complete;
- validate representative application presets independently;
- validate PR #470 ordinary CI and later validate its Debian workflow trigger corrections;
- determine whether the separate GUI GMDD diagnostic job remains useful now that the focused tests pass;
- keep Debian/PPA validation separate until its dependencies, duration, and branch policy are deliberate.
