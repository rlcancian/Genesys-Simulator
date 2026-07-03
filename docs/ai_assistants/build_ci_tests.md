# Build, CI, and Tests

## Purpose

This document is the stable AI-assistant reference for GenESyS build and test workflows. Historical recommendations from `docs/ai_assistants/oldies/` must be revalidated against current repository files before they are treated as current facts.

## Current confirmed baseline

### Build system

- CMake presets use schema version 6 and require CMake 3.24.
- The base preset uses the Ninja generator.
- The base preset configures C++23, requires the standard, and disables compiler extensions.
- The base preset enables kernel, parser, and plugins by default.
- GUI, web, and terminal applications are opt-in through dedicated presets.

### Unit-test validation

Run from the repository root:

    cmake --list-presets=all
    cmake --preset tests-unit
    cmake --build --preset tests-unit
    ctest --preset tests-unit --output-on-failure

The `tests-unit` configure preset writes to `build/tests-unit`, enables tests and web application support, disables smoke tests, and disables the terminal application.

The `tests-unit` build preset targets `genesys_kernel_unit_tests`.

The `tests-unit` test preset filters CTest by the `unit` label and enables output on failure.

### Kernel-focused validation

Run from the repository root:

    cmake --preset tests-kernel-unit
    cmake --build --preset tests-kernel-unit
    ctest --preset tests-kernel-unit --output-on-failure

The `tests-kernel-unit` configure preset inherits from `tests-unit`, writes to `build/tests-kernel-unit`, and keeps smoke and terminal application support disabled.

The `tests-kernel-unit` build preset targets `genesys_kernel_unit_tests_run`.

### Terminal presets

Current terminal presets include:

- `terminal-app`: builds `genesys_terminal_application` as the shell flow.
- `terminal-smart`: builds a selected smart example through `GENESYS_TERMINAL_EXAMPLE`.
- `terminal-example`: builds a selected non-shell example.
- `terminal-smart-hold-search-remove`: builds `Smart_HoldSearchRemove` through the smart terminal preset chain.

### GUI and web presets

Current application build presets include:

- `web-app`, targeting `genesys_web_app`.
- `genesys_web_app`, targeting `genesys_web_app` through its own configure preset.
- `gui-app`, targeting `genesys_gui`.

Use GUI and web presets only when the required Qt6 dependencies are available.

### CI workflow

The current GitHub Actions workflow is `.github/workflows/genesys-ci.yml`.

It runs on Ubuntu 24.04 for pull requests targeting `2026-1`, `WorkInProgress`, `WiP20261`, `currentStable`, and `master`. It also supports manual workflow dispatch.

The workflow installs build-essential, CMake, Ninja, Python 3, Qt6 base/tools packages, and OpenGL development dependencies. It then lists presets, configures `tests-unit`, builds `tests-unit`, and runs CTest with the `tests-unit` preset.

## Historical notes consolidated from oldies

### Build/test roadmap

`old_kernel-tests-build-roadmap-2026-1.md` recorded a stabilization roadmap for root CMake, incremental kernel build, and unit test coverage expansion. Treat that roadmap as historical unless each item is checked against current CMake files.

### Terminal build strategy

`old_terminal-build-strategy-2026-04-01.md` recorded that the terminal application flow had moved toward a pragmatic single executable target and CMake presets. It also recorded older open ideas around more granular example selection and generated configuration headers. Those ideas are optional improvements, not current blockers.

## Safe validation order

Before merging build-system changes, prefer this order:

1. List all presets.
2. Configure `tests-unit`.
3. Build `tests-unit`.
4. Run CTest with `tests-unit`.
5. If kernel-specific, repeat with `tests-kernel-unit`.
6. If terminal-specific, configure and build the relevant terminal preset.
7. If GUI or web-specific, validate the relevant preset in an environment with Qt6 dependencies.

## Open follow-up tasks

- Revalidate whether sanitizer presets such as ASan and UBSan should be added or restored as first-class CMake presets.
- Decide whether terminal example selection should remain preset/cache-variable based or move toward generated headers.
- Expand CI to include selected non-unit presets only after local build time and dependency impact are understood.
- Keep Debian/PPA packaging validation separate from ordinary unit-test CI unless packaging dependencies and runtime are controlled.
