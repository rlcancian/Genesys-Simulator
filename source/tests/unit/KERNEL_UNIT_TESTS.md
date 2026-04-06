# Kernel Simulator Unit Tests - Build & Run Guide

This document describes a **ready-to-use CMake/CLion setup** for running all unit tests related to the Kernel Simulator package.

## What to use (recommended)

Use these presets from `CMakePresets.json`:

- **Configure preset:** `tests-kernel-unit`
- **Build preset:** `tests-kernel-unit-run`
- **Build target (executed by the preset):** `genesys_kernel_unit_tests_run`

The `genesys_kernel_unit_tests_run` target:
1. Builds all kernel unit-test executables;
2. Runs all of them automatically (including the generated method-inventory test executable).

---

## CLion setup (single-click workflow)

1. Open **Settings/Preferences > Build, Execution, Deployment > CMake**.
2. Enable **Load CMake presets** (if not already enabled).
3. Select **Configure preset = `tests-kernel-unit`**.
4. In the run/build target selector, choose **Build preset = `tests-kernel-unit-run`**.
5. Build that preset.

Expected behavior: CLion builds everything required and then runs all kernel unit-test binaries automatically in sequence.

---

## Command line equivalent

```bash
cmake --preset tests-kernel-unit
cmake --build --preset tests-kernel-unit-run
```

This is equivalent to running the single target `genesys_kernel_unit_tests_run`.

---

## Executables included in the run target

The run target executes the following unit-test executables:

- `genesys_test_util`
- `genesys_test_simulator_support`
- `genesys_test_simulator_runtime`
- `genesys_test_support_modelmanager`
- `genesys_test_support_persistence`
- `genesys_test_runtime_pluginmanager`
- `genesys_test_support_oneventmanager`
- `genesys_test_support_parsermanager`
- `genesys_test_support_tracemanager`
- `genesys_test_support_connectionmanager`
- `genesys_test_support_simulationscenario`
- `genesys_test_support_experimentmanager`
- `genesys_test_kernel_simulator_method_inventory` (auto-generated inventory scaffold)

---

## Notes

- The generated inventory test executable is built from:
  - generator script: `source/tests/unit/tools/generate_kernel_simulator_test_stubs.py`
  - generated source: `source/tests/unit/generated/test_kernel_simulator_method_inventory.generated.cpp`
- If headers in `source/kernel/simulator` change, the generated inventory test file is automatically regenerated during build.
