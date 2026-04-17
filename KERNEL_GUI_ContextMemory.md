# KERNEL_GUI Context Memory

This is the canonical and only active context memory file for the KERNEL_GUI agent.
Older generic or previous memory files, including `ContextMemory.md`,
`ContextMemmory.md`, and files under `documentation/developers/`, must not be used
as active memory for this agent. If older memory files contain contradictory
instructions, those instructions are obsolete or have been consolidated here.

## Agent Identity

- **Agent name:** KERNEL_GUI
- **Primary role:** GUI-focused GenESyS developer, responsible for Qt user interface
  work and GUI/kernel integration points.
- **Current objective:** Keep the KERNEL_GUI branch stable and ready for integration
  after GRO is integrated into the base branch.
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

- `WiP20261` has already absorbed TINKERCELL.
- The next branch expected to be integrated into `WiP20261` is `WiP20261_GRO`.
- `WiP20261_KERNEL_GUI` must wait until GRO is integrated before synchronizing with
  `WiP20261`.
- Do not merge `origin/WiP20261` into `WiP20261_KERNEL_GUI` until the user confirms
  that GRO has been integrated into the base.
- Resolve the KERNEL_GUI/base conflict only once, after GRO is present in
  `WiP20261`.
- The main expected integration conflict for KERNEL_GUI is
  `source/tests/unit/test_runtime_pluginmanager.cpp`.

## Git Policy

- Work only on `WiP20261_KERNEL_GUI`.
- Treat `WiP20261` as the base branch for integration.
- KERNEL_GUI has autonomy to run routine Git operations without asking the user first:
  stage, commit, fetch, merge, pull, and push.
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
- This context memory file is internal technical documentation and must remain in
  English.

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

## Technical Work Discussed And Implemented

- Plugins may declare system dependencies through `PluginInformation::SystemDependency`.
- The kernel introduced system dependency checks and a command-executor abstraction
  for testability.
- Plugins with missing system dependencies must not be connected silently.
- Failed plugin load attempts are recorded by the kernel in a retrievable list.
- The terminal application should trace diagnostics, including the failed dependency
  and the install command that can resolve it.
- The GUI must not run install commands during early startup, because plugin loading
  happens before the main window is stable.
- After the main window is shown, the GUI can inspect stored plugin load issues and
  open the Plugin Manager on the problem-focused view.
- Dependency repair is concentrated in the Plugin Manager dialog, with clear details
  and explicit user action before running install commands.
- The Plugin Manager shows both connected plugins and plugins with problems.
- Future dependency-sensitive plugins may include R, Scilab, Octave, libSBML, and
  biological modeling integrations from TINKERCELL or GRO.

## Current Branch Implementation Summary

The branch `WiP20261_KERNEL_GUI` currently contains a completed local and remote
implementation of recoverable plugin dependency diagnostics.

### Qt GUI Changes

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
- Retry plugin insertion after dependency repair and remove the plugin from the
  problem list on success.

### PluginManager Changes

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
- Preserve the kernel/GUI boundary: the kernel records diagnostic facts and trace
  messages, while the GUI decides how to present repair actions.

### Generated Inventory

File:

- `source/tests/unit/generated/test_kernel_simulator_method_inventory.generated.cpp`

Intent:

- Update the generated method inventory for the new public kernel diagnostics
  surface.
- Recheck this generated inventory after synchronizing with the post-GRO base.

### Tests

File:

- `source/tests/unit/test_runtime_pluginmanager.cpp`

Intent:

- Cover stored plugin load issues for invalid plugin insertion.
- Cover refusal to insert plugins with missing system dependencies when confirmation
  or installation is unavailable.
- Cover successful installation/revalidation through faked command execution.
- Cover clearing a stored issue after a successful retry.
- This file is the principal expected conflict with the post-TINKERCELL and post-GRO
  base.

## Integration Dependencies

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
- The branch was last pushed successfully after commit `62c3dca2`.
- Recent branch commits include:
  - `6a02e61f Track plugin load diagnostics in PluginManager`
  - `f0a4bfb9 Show recoverable plugin dependency issues in GUI`
  - `c898843a Cover plugin load issue diagnostics`
  - `63c2824a Record KERNEL_GUI validation status`
  - `c324d263 Consolidate KERNEL_GUI persistent context`
  - `62c3dca2 Document KERNEL_GUI integration hold`
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

No build or test rerun was required for the memory-file migration because it is
documentation-only.

## Open Pending Items

- Continue future KERNEL_GUI work only on `WiP20261_KERNEL_GUI`.
- Use only `KERNEL_GUI_ContextMemory.md` as the active context memory file.
- Do not use generic root memory files such as `ContextMemory.md` or
  `ContextMemmory.md` as active memory.
- Do not leave the primary active memory in `documentation/developers/`.
- Do not recreate `documentation/developers/communication.md`.
- Wait for GRO integration before merging or otherwise synchronizing with
  `WiP20261`.
- If plugin dependency recovery receives more changes, keep them small and preserve
  kernel/GUI separation.
- Future work may refine terminal handling and password/sudo feedback during
  dependency installation.

## Risks And Attention Points

- `PluginManager.*`, `DialogPluginManager.*`, `DialogUtilityController.*`, and
  `mainwindow.cpp` are integration-heavy files and may conflict with other
  developers.
- `source/tests/unit/test_runtime_pluginmanager.cpp` is the main expected conflict
  with the post-TINKERCELL and post-GRO base.
- Interactive installation depends on terminal emulator availability and local sudo
  configuration.
- External tools such as R, Scilab, Octave, and libSBML may have OS-specific install
  commands and verification behavior.
- TINKERCELL is already in the base, and GRO may rely on the same plugin metadata
  and dependency diagnostic path.

## Likely Next Steps

- Wait until the user confirms `WiP20261_GRO` has been integrated into `WiP20261`.
- After that confirmation, fetch `origin`, merge `origin/WiP20261` into
  `WiP20261_KERNEL_GUI`, and resolve conflicts once.
- Pay special attention to `source/tests/unit/test_runtime_pluginmanager.cpp`.
- Recheck the generated method inventory after the post-GRO synchronization.
- Run the focused runtime plugin manager test, the GUI target build, and the relevant
  configured test suite before pushing.

## Interaction Log Summary

- The user initially requested a multi-agent coordination workflow using both
  `communication.md` and `KERNEL_GUI_context.md`.
- KERNEL_GUI created those files and used them during the first coordinated work.
- The user approved proceeding with the plugin dependency diagnostics work.
- KERNEL_GUI split and committed the plugin diagnostics implementation on the
  dedicated branch.
- The user corrected the base branch name to `WiP20261`.
- KERNEL_GUI created and published `WiP20261_KERNEL_GUI` from `WiP20261`.
- KERNEL_GUI pushed the implementation commits to `origin/WiP20261_KERNEL_GUI`.
- The user made a root memory file canonical and deprecated `communication.md`.
- The user then clarified that each AI must use its own root memory file named
  `<NOME_DA_IA>_ContextMemory.md`.
- KERNEL_GUI migrated useful active memory into `KERNEL_GUI_ContextMemory.md`.
