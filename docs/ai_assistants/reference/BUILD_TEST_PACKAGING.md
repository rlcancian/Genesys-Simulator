---
document_type: reference
authority: technical-reference
owner: project-maintainer
last_reviewed: 2026-07-22
review_cadence: on-build-contract-change
status: active
tracks: 511
---

# Build, Test, CI, and Packaging Reference

## 1. Scope

This document is the detailed operational reference for the current GenESyS CMake/Ninja build, automated tests, application presets, CI evidence, Docker helpers, Debian packaging, and generated documentation.

Policy and promotion rules remain in [`../GOVERNANCE.md`](../GOVERNANCE.md). Current validated state remains in [`../STATUS.md`](../STATUS.md).

## 2. Supported baseline

- primary platform: Ubuntu 24.04;
- CMake: 3.24 or newer;
- generator: Ninja;
- language: C++23 with compiler extensions disabled;
- GUI framework: Qt6;
- tests: Google Test through the configured system/bundled fallback.

Canonical entry points:

```text
CMakeLists.txt
CMakePresets.json
```

Never infer a current command, target or option from an old report without checking these files.

## 3. Primary test presets

### Ordinary unit baseline

```bash
cmake --preset tests-unit
cmake --build --preset tests-unit --parallel "$(nproc)"
ctest --preset tests-unit --output-on-failure
```

### Kernel/direct-runner baseline

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

Inventory without execution:

```bash
ctest --preset tests-unit -N
ctest --preset tests-kernel-unit -N
ctest --preset tests-smoke -N
```

Never reuse a historical count as a current count. Record the branch, head, preset and toolchain with every new exact inventory.

## 4. Validation order by impact

For cross-cutting changes:

1. record branch, commit and toolchain;
2. configure the narrowest affected preset;
3. build with Ninja;
4. run focused tests;
5. run ordinary `tests-unit`;
6. run `tests-kernel-unit` for kernel/parser/plugin/tool behavior;
7. run `tests-smoke` for runtime/continuous/startup behavior;
8. build/start affected application presets;
9. run sanitizers for ownership/UB/resource changes;
10. validate packaging separately when package/install behavior changes.

Configuration, compilation, test registration, test execution, startup, interaction, packaging and scientific validation are distinct checkpoints.

## 5. Application presets

Current principal presets include:

```text
genesys_shell
genesys_worker_app
gui-app
gui-httpworker
gui-dataanalyser
gui-optimizer
gui-ai-assistant
terminal-smart
terminal-model-specific
genesys_modelspecific_app
terminal-smart-hold-search-remove
terminal-smart-ai-assistant
```

Use the exact target and output discovered from current CMake rather than assuming that preset, target and executable names are identical.

Standalone GUI startup validation should use a private Xvfb display, Qt6/XCB, exact executable discovery, PID-associated window evidence, bounded liveness, controlled teardown and residual-process checks. Startup evidence does not validate feature workflows.

## 6. GitHub Actions

Core workflows currently include:

- `.github/workflows/genesys-ci.yml`;
- `.github/workflows/genesys-phase0-validation.yml`;
- focused sanitizer and application-validation workflows;
- `.github/workflows/genesys-debian-package.yml`.

Workflows complement each other. A focused application or sanitizer workflow does not replace ordinary regression CI.

When using GitHub-only execution, record:

- PR head and merge ref;
- workflow/run/job conclusion;
- artifact name, ID and digest when material;
- exact scope and non-claims.

## 7. Sanitizers and diagnostics

Use focused ASan/LSan/UBSan targets when broad instrumentation would destabilize unrelated archives or make diagnosis ambiguous.

A sanitizer result proves only its executable, input path, toolchain and options. Preserve red checkpoints when they explain the correction.

Valgrind, clang-tidy, cppcheck and profiling are optional diagnostic tools. Introduce them only when compatible with the real build and useful for a defined question.

## 8. Docker helpers

Docker is an operational helper, not the canonical build architecture.

Rules:

- keep Dockerfiles/scripts under the maintained packaging/script locations;
- pin or record the base image/toolchain used for evidence;
- keep X11 forwarding opt-in and local;
- never place secrets in images or command history;
- preserve host filesystem ownership for generated artifacts;
- do not treat a container-only success as proof of the supported host/package workflow.

Inspect current scripts before publishing commands because paths and helper names have changed historically.

## 9. Debian package lifecycle

Package validation is separate from package creation. A complete lifecycle should cover:

1. build source/binary packages;
2. inspect metadata and dependencies;
3. install on a clean supported environment;
4. discover installed binaries, desktop files, icons, AppStream/metainfo and configuration;
5. start supported executables;
6. reinstall/upgrade when relevant;
7. uninstall/purge;
8. inspect residual files/processes/services;
9. record versions, logs and artifacts.

PPA publication is a human/release step and is not implied by a green `.deb` build.

## 10. Doxygen and generated documentation

Run from the repository root:

```bash
doxygen docs/users/DoxyfileUser
doxygen docs/developers/DoxyfileDeveloper
```

Policies:

- user entry points/final user artifacts: `docs/users/`;
- developer entry points/final developer artifacts: `docs/developers/`;
- intermediate HTML/XML/LaTeX/man output: build tree;
- generated/intermediate trees must not be versioned;
- package man pages may be generated/collected during packaging but need lifecycle validation.

## 11. Evidence routing

- current baseline: [`../STATUS.md`](../STATUS.md);
- monthly executed ledger: [`../history/evidence/2026/07/VALIDATION_LEDGER.md`](../history/evidence/2026/07/VALIDATION_LEDGER.md);
- pending autonomous validation: [`../BACKLOG_AUTONOMOUS.md`](../BACKLOG_AUTONOMOUS.md);
- release decisions: [`../BACKLOG_HUMAN.md`](../BACKLOG_HUMAN.md).
