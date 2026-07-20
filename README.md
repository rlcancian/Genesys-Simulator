# GenESyS — Generic and Expansible System Simulator

GenESyS (**Ge**neric and E**xpansible** **Sy**stem **S**imulator) is a C++ platform for modeling and simulation built around a reusable kernel, parser infrastructure, model components/data definitions, optional applications, and automated tests.

The current repository is a development codebase. A successful build or unit-test run does not imply that every experimental, numerical, statistical, biological, optimization, or distributed feature is mature or scientifically validated.

## 1. Current technical baseline

The canonical build flow is:

- Ubuntu 24.04 as the primary CI/development baseline;
- CMake 3.24 or newer;
- Ninja;
- C++23 with compiler extensions disabled;
- Qt6 for graphical applications;
- Google Test through a system package when compatible, with the bundled source fallback otherwise.

Primary build entry points:

```text
CMakeLists.txt
CMakePresets.json
```

Older documentation that treats `project/`, `projects/`, qmake, Qt5, `source/applications/terminal/`, or `source/applications/web/` as the current canonical structure is historical unless explicitly marked as a compatibility path.

AI assistants must read `docs/ai_assistants/README.md` before changing code, CMake, tests, CI, packaging, documentation, plugin architecture, numerical algorithms, or scientific behavior.

## 2. Quick validation

### 2.1 Ordinary unit baseline

```bash
cmake --preset tests-unit
cmake --build --preset tests-unit --parallel "$(nproc)"
ctest --preset tests-unit --output-on-failure
```

This is the ordinary CI baseline. It configures under `build/tests-unit` and builds target `genesys_kernel_unit_tests`.

To inspect the registered tests without executing them:

```bash
ctest --preset tests-unit -N
```

### 2.2 Kernel-focused baseline

```bash
cmake --preset tests-kernel-unit
cmake --build --preset tests-kernel-unit --parallel "$(nproc)"
ctest --preset tests-kernel-unit --output-on-failure
```

### 2.3 Smoke baseline

```bash
cmake --preset tests-smoke
cmake --build --preset tests-smoke --parallel "$(nproc)"
ctest --preset tests-smoke --output-on-failure
```

The smoke preset is separate from the ordinary unit preset.

## 3. Applications

### 3.1 GenESyS shell

```bash
cmake --preset genesys_shell
cmake --build --preset genesys_shell --parallel "$(nproc)"
```

Target:

```text
genesys_shell
```

Source area:

```text
source/applications/shell/
```

### 3.2 Worker HTTP application

```bash
cmake --preset genesys_worker_app
cmake --build --preset genesys_worker_app --parallel "$(nproc)"
```

Targets/output:

```text
genesys_worker_core
genesys_worker_app
genesys-worker
```

Source area:

```text
source/applications/worker/
```

The worker is intended for a controlled academic intranet after its authentication, cryptographic randomness, resource limits, isolation, and audit controls are implemented and validated. Direct public-Internet exposure is not the default approved profile.

### 3.3 Main Qt GUI

```bash
cmake --preset gui-app
cmake --build --preset gui-app --parallel "$(nproc)"
```

Target/output:

```text
genesys_gui
genesys-gui
```

The supported Qt baseline is Qt6. Remaining Qt5 fallback code is legacy implementation debt scheduled for removal; Qt5 is not part of the intended platform contract.

### 3.4 Independent GUI applications

Current configure/build presets include:

```text
gui-httpworker
gui-dataanalyser
gui-optimizer
gui-ai-assistant
```

Examples:

```bash
cmake --preset gui-dataanalyser
cmake --build --preset gui-dataanalyser --parallel "$(nproc)"

cmake --preset gui-optimizer
cmake --build --preset gui-optimizer --parallel "$(nproc)"
```

These presets build independent GUI targets under:

```text
source/applications/gui/
```

The `doexperiments` GUI is planned but not implemented. Enabling `GENESYS_BUILD_GUI_DOEXPERIMENTS` currently fails configuration intentionally rather than creating a misleading empty application.

### 3.5 Model-specific applications

Current presets include:

```text
terminal-smart
terminal-model-specific
genesys_modelspecific_app
terminal-smart-hold-search-remove
terminal-smart-ai-assistant
```

The selected source/model is controlled through the corresponding CMake cache variables recorded in `CMakePresets.json`.

## 4. Main CMake options

The root build exposes these principal options:

| Option | Purpose |
|---|---|
| `GENESYS_BUILD_KERNEL` | Build kernel libraries |
| `GENESYS_BUILD_PARSER` | Build parser library |
| `GENESYS_BUILD_PLUGINS` | Build the current statically aggregated plugin libraries |
| `GENESYS_BUILD_TESTS` | Build automated tests |
| `GENESYS_BUILD_TERMINAL_APPLICATION` | Build the shell application |
| `GENESYS_BUILD_TERMINAL_EXAMPLES` | Build selected model-specific/terminal examples |
| `GENESYS_BUILD_WORKER_APPLICATION` | Build the worker HTTP application |
| `GENESYS_BUILD_GUI_APPLICATION` | Enter the GUI applications umbrella |
| `GENESYS_BUILD_TOOLS` | Build reusable tool backends |
| `GENESYS_ENABLE_PYTHON_INTEGRATION` | Enable experimental embedded Python support when development files exist |

Compatibility aliases may still exist for older option names. New work should use the current options above.

## 5. Current repository map

```text
Genesys-Simulator/
├── CMakeLists.txt
├── CMakePresets.json
├── autoloadplugins.txt
├── debian/
├── docs/
│   ├── ManualGenESyS.pdf
│   ├── ai_assistants/
│   ├── developers/
│   └── users/
├── models/
├── packaging/
│   ├── docker/
│   └── linux/
├── source/
│   ├── applications/
│   │   ├── gui/
│   │   │   ├── genesys/
│   │   │   ├── httpworker/
│   │   │   ├── dataanalyser/
│   │   │   ├── optimizer/
│   │   │   └── ai_assistant/
│   │   ├── modelSpecific/
│   │   ├── shell/
│   │   └── worker/
│   ├── kernel/
│   │   ├── simulator/
│   │   ├── statistics/
│   │   └── util/
│   ├── parser/
│   ├── plugins/
│   ├── tests/
│   └── tools/
└── build/                 # generated out-of-source build directories
```

`build/` and other generated build/package output directories are not source-tree architecture and must not be versioned.

## 6. Plugin architecture status

The current production build aggregates component and data-definition plugins into static libraries and links a static connector into the simulator runtime.

This means:

- current plugins are not independently distributed dynamic packages;
- the immediate consolidation objective is to remove unjustified source overlap and document registration/lifecycle contracts;
- a broad dynamic migration must not begin during baseline stabilization.

The approved future boundary for independently built in-process plugins is a stable C ABI using opaque handles, explicit ownership operations, versioned function/capability tables, and no STL/Qt/C++ exceptions crossing the binary boundary.

See:

```text
docs/ai_assistants/plugins_development.md
docs/ai_assistants/genesys_2026_human_decisions.md
docs/ai_assistants/genesys_2026_decisions_addendum_20260720.md
```

## 7. Tests and CI status

The ordinary GitHub Actions workflow is:

```text
.github/workflows/genesys-ci.yml
```

It currently executes:

- configure/build/CTest for `tests-unit`;
- a focused GUI GMDD diagnostic executable.

A recorded Phase 0 checkpoint on Ubuntu 24.04.4 passed the ordinary `tests-unit` job and all three focused GUI GMDD tests. That result applies only to the recorded commit and registered test scope.

Still required for a complete baseline:

- `tests-kernel-unit` execution;
- `tests-smoke` execution;
- exact CTest/test-target inclusion inventory;
- representative application startup validation;
- sanitizer runs;
- package lifecycle validation.

Detailed current evidence:

```text
docs/ai_assistants/genesys_2026_phase0_ci_evidence_20260720.md
docs/ai_assistants/genesys_2026_test_matrix.md
docs/ai_assistants/genesys_2026_consolidation_handoff.md
```

## 8. Numerical, statistical, optimization, and biological maturity

Correct compilation is not sufficient evidence for scientific correctness.

Current policy requires:

- authoritative mathematical/statistical references;
- explicit parameterizations and valid domains;
- analytical invariants or trusted reference vectors;
- documented tolerances;
- deterministic seeds and reproducibility controls;
- benchmark datasets/models;
- no unsupported scientific or predictive claims.

Supported functionality should reach at least **Level 3 — Beta** before prioritized capabilities advance to **Level 4 — Stable user feature**.

The current Optimizer backend remains an internal scaffold. Future work is planned around evolutionary multiobjective optimization, hypervolume-based methods, ETH Zürich/PISA-related work, and algorithms/benchmarks from the project maintainer's doctoral research.

Whole-cell and biochemical evolution follows a neuro-symbolic-mechanistic AI virtual-cell research direction while preserving mechanistic simulators, invariants, curated data, and verifiable tools.

## 9. Documentation

AI/developer operational guidance:

```text
docs/ai_assistants/README.md
```

User and developer Doxygen entry points:

```text
docs/users/DoxyfileUser
docs/developers/DoxyfileDeveloper
```

Run from the repository root:

```bash
doxygen docs/users/DoxyfileUser
doxygen docs/developers/DoxyfileDeveloper
```

Doxygen intermediate output belongs under the build tree. Only deliberately maintained final documentation artifacts should be versioned.

## 10. Branch policy

Current branch roles are documented in:

```text
docs/ai_assistants/branch_workflow.md
```

Important naming:

- `20261` is the renamed former `2026-1` semester-stable branch;
- `WorkInProgress` is the active development line;
- `20262` is reserved for the stable result promoted near the end of the second semester of 2026;
- `currentStable` and `master` remain progressively more conservative branches.

Do not promote or merge stable branches implicitly. Use bounded branches, small commits, CI evidence, and explicit human approval.
