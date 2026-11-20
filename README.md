# GenESyS — Generic and Expansible System Simulator

GenESyS (**Ge**neric and E**xpansible** **Sy**stem **S**imulator) is a C++ platform for modeling and simulation built around a reusable kernel, expression parser, model components/data definitions, statistical and analytical tools, optional applications, and automated tests.

The repository is an active development codebase. Successful configuration, compilation, startup, or unit tests do not imply that every experimental, numerical, statistical, biochemical, optimization, AI, worker, or distributed feature is mature or scientifically validated.

## 1. Technical baseline

The intended primary baseline is:

- Ubuntu 24.04;
- CMake 3.24 or newer;
- Ninja;
- C++23 with compiler extensions disabled;
- Qt6 for graphical applications;
- Google Test through the configured system package/bundled fallback.

Canonical build entry points:

```text
CMakeLists.txt
CMakePresets.json
```

Historical documentation that treats qmake, Qt5, `project/`, `projects/`, `source/applications/terminal/`, or `source/applications/web/` as the current canonical architecture must be revalidated against the current tree.

## 2. Mandatory guidance for AI assistants

Before changing code, CMake, tests, CI, packaging, documentation, plugin architecture, numerical algorithms, security boundaries, or scientific behavior, read:

```text
docs/ai_assistants/README.md
docs/ai_assistants/GOVERNANCE.md
docs/ai_assistants/STATUS.md
```

Read `docs/ai_assistants/ARCHITECTURE.md`, the applicable backlog/runbook, and task-specific files under `docs/ai_assistants/reference/` when required by the task.

Do not use historical files or old PR reports as current policy/state.

## 3. Quick validation

### Ordinary unit baseline

```bash
cmake --preset tests-unit
cmake --build --preset tests-unit --parallel "$(nproc)"
ctest --preset tests-unit --output-on-failure
```

### Kernel-focused baseline

```bash
cmake --preset tests-kernel-unit
cmake --build --preset tests-kernel-unit --parallel "$(nproc)"
ctest --preset tests-kernel-unit --output-on-failure
```

### Smoke baseline

```bash
cmake --preset tests-smoke
cmake --build --preset tests-smoke --parallel "$(nproc)"
ctest --preset tests-smoke --output-on-failure
```

The current exact counts and bounded application/sanitizer results are recorded in:

```text
docs/ai_assistants/STATUS.md
docs/ai_assistants/history/evidence/
```

## 4. Repository map

```text
Genesys-Simulator/
├── CMakeLists.txt
├── CMakePresets.json
├── debian/
├── docs/
│   ├── ai_assistants/
│   ├── developers/
│   └── users/
├── models/
├── packaging/
├── scripts/
└── source/
    ├── applications/
    │   ├── gui/
    │   ├── modelSpecific/
    │   ├── shell/
    │   └── worker/
    ├── kernel/
    ├── parser/
    ├── plugins/
    ├── tests/
    └── tools/
```

Generated build/package output is not source architecture and must not be versioned.

## 5. Applications

### Shell

```bash
cmake --preset genesys_shell
cmake --build --preset genesys_shell --parallel "$(nproc)"
```

### Worker

```bash
cmake --preset genesys_worker_app
cmake --build --preset genesys_worker_app --parallel "$(nproc)"
```

The worker is not approved for direct public-Internet exposure. Its intended future profile is a controlled academic intranet after bind, authentication, TLS, quotas, isolation, audit, and resource controls are implemented and validated.

### Main Qt6 GUI

```bash
cmake --preset gui-app
cmake --build --preset gui-app --parallel "$(nproc)"
```

### Independent Qt6 GUIs

Current presets include:

```text
gui-httpworker
gui-dataanalyser
gui-optimizer
gui-ai-assistant
```

The Data Analyser, Optimizer, and AI Assistant have bounded startup evidence. Startup does not prove their complete functional or scientific workflows. `doexperiments` remains planned and intentionally unavailable rather than represented by an empty target.

### Model-specific applications

Current presets include model-specific/smart-terminal variants defined in `CMakePresets.json`. Treat any historical sweep as a snapshot and revalidate the selected source/model and generated `.gen` output.

## 6. Architecture summary

- `source/kernel/`: simulator runtime/support, statistics, utilities;
- `source/parser/`: expression parsing/evaluation;
- `source/plugins/`: model components and data definitions;
- `source/tools/`: reusable statistical, numerical, optimization, AI, diffusion, and results backends;
- `source/applications/`: shell, worker, model-specific programs and Qt6 GUIs;
- `source/tests/`: unit, focused, integration-light and smoke validation.

Current plugins are statically aggregated. A broad dynamic migration is not authorized during consolidation. The approved future in-process boundary is a versioned stable C ABI with opaque handles and no STL/Qt/C++ exceptions across the package boundary.

Detailed architecture:

```text
docs/ai_assistants/ARCHITECTURE.md
docs/ai_assistants/reference/PLUGINS.md
```

## 7. Numerical, statistical, optimization, and biological maturity

Scientific correctness requires explicit formulations, parameterizations, domains, units, references, fixtures, expected values, tolerances, reproducibility, provenance, limitations, and independent review where appropriate.

Supported functionality should reach at least Level 3 — Beta before inclusion in a semester-stable supported set. Software maturity and scientific claim level are independent.

The Optimizer remains a scaffold pending a selected algorithm/benchmark package. Whole-cell, biochemical, SBML, and AI virtual-cell work remains experimental/research-oriented and must not be overclaimed.

See:

```text
docs/ai_assistants/reference/SCIENTIFIC_DOMAINS.md
docs/ai_assistants/BACKLOG_HUMAN.md
```

## 8. Documentation

AI/developer operational guidance:

```text
docs/ai_assistants/README.md
```

Doxygen entry points:

```text
docs/users/DoxyfileUser
docs/developers/DoxyfileDeveloper
```

Run Doxygen from the repository root. Intermediate generated output belongs under the build tree; only deliberately maintained final artifacts should be versioned.

## 9. Branch policy

The formal promotion flow is:

```text
feature branches -> WorkInProgress -> YYYYs -> currentStable -> master
```

For current work, bounded AI-assisted branches use:

```text
WiPYYYYMMDD/<scope>
```

Stable branch promotion, waivers, release scope, and branch roles are defined only in:

```text
docs/ai_assistants/GOVERNANCE.md
```
