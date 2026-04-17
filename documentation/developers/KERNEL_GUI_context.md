# KERNEL_GUI Context

This document records the current KERNEL_GUI branch state for integration
coordination. It must not be used as permission to merge into `WiP20261` before the
GRO integration is complete.

## Branches

- **Integration base branch:** `WiP20261`
- **KERNEL_GUI working branch:** `WiP20261_KERNEL_GUI`
- **Remote:** `origin`

KERNEL_GUI works only on `WiP20261_KERNEL_GUI`. The shared branch `WiP20261` is the
integration base and must not be modified directly by KERNEL_GUI.

## Current Integration Order

- `WiP20261` has already absorbed TINKERCELL.
- The next branch expected to be integrated into `WiP20261` is `WiP20261_GRO`.
- `WiP20261_KERNEL_GUI` must wait for GRO to be integrated before synchronizing with
  the base.
- Do not merge `origin/WiP20261` into `WiP20261_KERNEL_GUI` yet.
- Resolve the KERNEL_GUI/base conflict only once, after GRO is present in
  `WiP20261`.
- The main expected conflict point is
  `source/tests/unit/test_runtime_pluginmanager.cpp`.

## Technical Summary

The `WiP20261_KERNEL_GUI` branch contains the recoverable plugin dependency
diagnostics work for the Qt GUI and the kernel-facing plugin lifecycle.

### Qt GUI

- Updates the Qt Plugin Manager dialog to present loaded plugins separately from
  plugins with recoverable load problems.
- Opens the Plugin Manager on the problem-focused view when startup detects stored
  plugin load issues.
- Keeps install and repair actions out of early startup and exposes them through
  explicit user action in the Plugin Manager.
- Uses `DialogUtilityController.*`, `dialogpluginmanager.*`, and `mainwindow.cpp`
  as the main GUI integration points.

### PluginManager

- Updates `source/kernel/simulator/PluginManager.h` and
  `source/kernel/simulator/PluginManager.cpp`.
- Adds persistent plugin load diagnostics owned by `PluginManager`.
- Records invalid plugin insertion, missing system dependencies, dynamic dependency
  failures, connection failures, insertion failures, and exception cases.
- Exposes stored plugin load issues so GUI and terminal flows can inspect failed
  plugin load attempts after startup.
- Clears a stored issue after the same plugin is inserted successfully.
- Preserves the kernel/GUI boundary: the kernel records diagnostic facts and trace
  messages, while the GUI decides how to present repair actions.

### Generated Inventory

- Updates
  `source/tests/unit/generated/test_kernel_simulator_method_inventory.generated.cpp`
  for the new public kernel diagnostics surface.
- This generated inventory must be checked again after synchronizing with the
  post-GRO base.

### Tests

- Updates `source/tests/unit/test_runtime_pluginmanager.cpp`.
- Covers stored plugin load issues for invalid plugin insertion.
- Covers missing system dependency refusal when confirmation or installation is
  unavailable.
- Covers successful installation and revalidation through faked command execution.
- Covers clearing stored plugin load issues after successful retry.
- This file is the principal expected integration conflict with the base.

### Integration Dependencies

- The GUI work depends on the `PluginManager.*` diagnostic API added by this branch.
- The generated method inventory depends on the final post-GRO public kernel surface.
- TINKERCELL is already present in `WiP20261`; GRO must be integrated before
  KERNEL_GUI synchronizes with the base.
- KERNEL_GUI should remain stable and documented until the user authorizes the
  post-GRO synchronization.

## Validation Snapshot

The plugin diagnostics implementation was previously validated with:

```bash
cmake --build build-gui --target genesys_test_runtime_pluginmanager genesys_qt_gui_application
./build-gui/source/tests/unit/genesys_test_runtime_pluginmanager
ctest --test-dir build-gui --output-on-failure
git diff --check HEAD
```

Observed status from that validation:

- Focused runtime plugin manager test passed.
- Full configured `ctest` run passed.
- R-related tests were skipped because `Rscript` was unavailable.

No new feature implementation is being done in the current documentation-only step.
