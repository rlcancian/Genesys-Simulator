# KERNEL_GUI Context

This is the canonical persistent memory file for the KERNEL_GUI agent. The old shared
`documentation/developers/communication.md` file is no longer used and must not be
recreated by KERNEL_GUI.

## Agent Identity

- **Agent name:** KERNEL_GUI
- **Primary role:** GUI-focused GenESyS developer, responsible for Qt user interface
  work and GUI/kernel integration points.
- **Current objective:** Keep improving plugin diagnostics and recoverable plugin
  dependency handling in the GUI while preserving a clean integration path with the
  GenESyS kernel.
- **Main technical scope:** Qt GUI, Plugin Manager dialog, MainWindow startup flow,
  kernel-facing plugin lifecycle contracts, plugin diagnostics exposed to the GUI,
  and focused tests that cover GUI-facing kernel behavior.

## Canonical Branches

- **Integration base branch:** `WiP20261`
- **KERNEL_GUI working branch:** `WiP20261_KERNEL_GUI`
- **Remote:** `origin`

KERNEL_GUI must work only on `WiP20261_KERNEL_GUI`. The shared branch `WiP20261` is
the integration base and must not be modified or merged into directly unless the
user explicitly requests that operation.

## Current Integration Order

- `WiP20261` has already absorbed the TINKERCELL branch.
- The next branch expected to be integrated into `WiP20261` is `WiP20261_GRO`.
- `WiP20261_KERNEL_GUI` must wait until GRO is integrated before synchronizing with
  `WiP20261`.
- Do not merge `origin/WiP20261` into `WiP20261_KERNEL_GUI` until the user confirms
  that GRO has been integrated into the base.
- The main expected integration conflict for KERNEL_GUI is
  `source/tests/unit/test_runtime_pluginmanager.cpp`.

## Canonical Git Policy

- Work only on `WiP20261_KERNEL_GUI`.
- Treat `WiP20261` as the base branch for integration.
- KERNEL_GUI has autonomy to run routine Git operations without asking the user first:
  - stage;
  - commit;
  - fetch;
  - merge;
  - pull;
  - push.
- Make small, frequent, coherent commits.
- Do not wait for user confirmation for routine Git operations.
- Ask the user before destructive operations, before resolving significant ambiguity
  with high impact, or when there is exceptional risk.
- While GRO is pending, do not synchronize with `origin/WiP20261`.
- After GRO has been integrated and the user confirms synchronization is appropriate,
  prefer this routine before important pushes:
  - fetch `origin`;
  - merge the latest `origin/WiP20261` into `WiP20261_KERNEL_GUI`;
  - resolve conflicts if any;
  - run relevant build/tests;
  - push `WiP20261_KERNEL_GUI`.
- Do not rebase shared work unless explicitly instructed.

## Repository Language Policy

- The user communicates in Portuguese.
- Source code, identifiers, comments in code, Doxygen, and internal technical
  repository documentation must remain in English.
- This context file is internal technical documentation and must remain in English.

## Relevant Interfaces And Modules

- `source/kernel/simulator/PluginManager.h`
- `source/kernel/simulator/PluginManager.cpp`
- `source/kernel/simulator/PluginInformation.*`
- `SystemDependencyResolver` and related system dependency diagnostics types
- `PluginInsertionOptions`
- `PluginConnectorDummyImpl1`
- `source/applications/gui/qt/GenesysQtGUI/mainwindow.cpp`
- `source/applications/gui/qt/GenesysQtGUI/controllers/DialogUtilityController.*`
- `source/applications/gui/qt/GenesysQtGUI/dialogs/dialogpluginmanager.*`
- `source/tests/unit/test_runtime_pluginmanager.cpp`
- `source/tests/unit/generated/test_kernel_simulator_method_inventory.generated.cpp`

## Technical Work Already Discussed

- Plugins may declare system dependencies through `PluginInformation::SystemDependency`.
- A previous kernel implementation already introduced system dependency checks and a
  command-executor abstraction for testability.
- Plugins with missing system dependencies must not be connected silently.
- The terminal application should trace diagnostics, including what dependency failed
  and which install command can resolve it.
- The GUI must not run install commands during early startup, because plugin loading
  happens before the main window is stable.
- Failed plugin load attempts should be recorded by the kernel in a retrievable list.
- After the main window is shown, the GUI can use that list to open the Plugin Manager
  and focus the user on problem plugins.
- Dependency repair should be concentrated in the Plugin Manager dialog, with clear
  details and explicit user action before running install commands.
- The Plugin Manager should show both connected plugins and plugins with problems.
- Future dependency-sensitive plugins may include R, Scilab, Octave, libSBML, and
  biological modeling integrations from TINKERCELL or GROW.

## Current Branch Implementation Summary

The branch `WiP20261_KERNEL_GUI` currently contains a completed local and remote
implementation of recoverable plugin dependency diagnostics.

### Kernel Changes

Files:

- `source/kernel/simulator/PluginManager.h`
- `source/kernel/simulator/PluginManager.cpp`

Intent:

- Add persistent plugin load diagnostics owned by `PluginManager`.
- Introduce `PluginLoadIssue` to represent plugin load failures such as invalid
  plugins, missing system dependencies, dynamic dependency failures, connection
  failures, insertion failures, and exceptions.
- Expose a getter for plugin load issues so GUI and terminal flows can inspect failed
  plugin load attempts after startup.
- Clear a stored issue when the same plugin is later inserted successfully.
- Preserve existing dynamic library dependency behavior.
- Keep UI out of the kernel; the kernel records diagnostic facts and trace messages,
  not Qt dialogs.

### GUI Changes

Files:

- `source/applications/gui/qt/GenesysQtGUI/dialogs/dialogpluginmanager.cpp`
- `source/applications/gui/qt/GenesysQtGUI/dialogs/dialogpluginmanager.h`
- `source/applications/gui/qt/GenesysQtGUI/dialogs/dialogpluginmanager.ui`
- `source/applications/gui/qt/GenesysQtGUI/controllers/DialogUtilityController.cpp`
- `source/applications/gui/qt/GenesysQtGUI/controllers/DialogUtilityController.h`
- `source/applications/gui/qt/GenesysQtGUI/mainwindow.cpp`

Intent:

- Replace the single plugin table with a two-tab Plugin Manager view:
  - loaded plugins;
  - plugins with problems.
- Populate the problem tab from `PluginManager::getPluginLoadIssues()`.
- Open the Plugin Manager on the problem tab when startup detects stored load issues.
- Avoid install prompts during early startup/autoload.
- Add a dependency resolution action in the Plugin Manager, where the user can inspect
  missing dependencies, check commands, install commands, and diagnostic output.
- Run install commands only after explicit action from the Plugin Manager.
- Attempt to use a graphical terminal for interactive installation, with a defensive
  fallback that captures command output when no terminal is available.
- Retry plugin insertion after dependency repair and remove the plugin from the problem
  list on success.

### Test Changes

Files:

- `source/tests/unit/test_runtime_pluginmanager.cpp`
- `source/tests/unit/generated/test_kernel_simulator_method_inventory.generated.cpp`

Intent:

- Cover stored plugin load issues for invalid plugin insertion.
- Cover refusal to insert plugins with missing system dependencies when confirmation
  or installation is unavailable.
- Cover successful installation/revalidation through faked command execution.
- Cover clearing a stored issue after a successful retry.
- Update generated method inventory for the new public kernel diagnostics surface.

### Integration Dependencies

- GUI-facing plugin diagnostics depend on the kernel diagnostic API added in
  `PluginManager.*`.
- The generated method inventory must remain aligned with the public kernel surface
  once KERNEL_GUI is synchronized with the post-GRO base.
- `source/tests/unit/test_runtime_pluginmanager.cpp` is the highest-risk conflict
  point because TINKERCELL has already changed the base and GRO is expected to be
  integrated before KERNEL_GUI.
- Conflict resolution should happen only once, after GRO is present in `WiP20261`.

## Current Branch State

- `WiP20261_KERNEL_GUI` exists locally.
- `origin/WiP20261_KERNEL_GUI` exists remotely.
- The local branch tracks `origin/WiP20261_KERNEL_GUI`.
- The branch was last pushed successfully after commit `c324d263`.
- The implementation commits currently on the branch include:
  - `3743bc01 Document KERNEL_GUI coordination branch`
  - `6a02e61f Track plugin load diagnostics in PluginManager`
  - `f0a4bfb9 Show recoverable plugin dependency issues in GUI`
  - `c898843a Cover plugin load issue diagnostics`
  - `63c2824a Record KERNEL_GUI validation status`
  - `c324d263 Consolidate KERNEL_GUI persistent context`
- The branch is intentionally waiting for `WiP20261_GRO` to be integrated before
  another synchronization with `WiP20261`.

## Validation Already Run

The following commands passed after the plugin diagnostics implementation:

```bash
git fetch origin WiP20261
git merge origin/WiP20261
cmake --build build-gui --target genesys_test_runtime_pluginmanager genesys_qt_gui_application
./build-gui/source/tests/unit/genesys_test_runtime_pluginmanager
ctest --test-dir build-gui --output-on-failure
git diff --check HEAD
```

Observed test status:

- The focused runtime plugin manager test passed with 6 tests.
- The full configured `ctest` run passed with 1257 executed tests and 0 failures.
- Four tests were disabled.
- Two R-related tests were skipped because `Rscript` was unavailable.

## Open Pending Items

- Continue future KERNEL_GUI work only on `WiP20261_KERNEL_GUI`.
- Keep this file as the only KERNEL_GUI persistent memory.
- Do not recreate `documentation/developers/communication.md`.
- Wait for GRO integration before merging or otherwise synchronizing with
  `WiP20261`.
- If plugin dependency recovery receives more changes, keep them small and preserve
  kernel/GUI separation.
- Future work may refine terminal handling and password/sudo feedback during dependency
  installation.

## Risks And Attention Points

- `PluginManager.*`, `DialogPluginManager.*`, `DialogUtilityController.*`, and
  `mainwindow.cpp` are integration-heavy files and may conflict with other developers.
- `source/tests/unit/test_runtime_pluginmanager.cpp` is the main expected conflict
  with the post-TINKERCELL and post-GRO base.
- Interactive installation depends on terminal emulator availability and local sudo
  configuration.
- External tools such as R, Scilab, Octave, and libSBML may have OS-specific install
  commands and verification behavior.
- TINKERCELL is already in the base, and GRO may rely on the same plugin metadata
  and dependency diagnostic path.

## Interaction Log Summary

- The user initially requested a multi-agent coordination workflow using both
  `communication.md` and `KERNEL_GUI_context.md`.
- KERNEL_GUI created those files and used them during the first coordinated work.
- The user approved proceeding with the plugin dependency diagnostics work.
- KERNEL_GUI split and committed the plugin diagnostics implementation on the dedicated
  branch.
- The user corrected the base branch name to `WiP20261`.
- KERNEL_GUI created and published `WiP20261_KERNEL_GUI` from `WiP20261`.
- KERNEL_GUI pushed the implementation commits to `origin/WiP20261_KERNEL_GUI`.
- The user then made this file the only canonical persistent memory and deprecated
  `communication.md`.
