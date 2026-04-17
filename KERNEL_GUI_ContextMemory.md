# KERNEL_GUI Context Memory

This is the canonical and only active context memory file for the KERNEL_GUI agent.
Older generic or previous memory files, including `ContextMemory.md`,
`ContextMemmory.md`, and files under `documentation/developers/`, must not be used
as active memory for this agent. If older memory files contain contradictory
instructions, those instructions are obsolete or have been consolidated here.

## Agent Identity

- **Agent name:** KERNEL_GUI
- **Primary role:** Main GenESyS AI for GUI, kernel, and cross-cutting integrations
  when needed, with primary responsibility for Qt user interface work and GUI/kernel
  integration points.
- **Current objective:** Keep `WiP20261_KERNEL_GUI` ready for final integration into
  `WiP20261` after re-synchronizing with the latest base that already contains
  TINKERCELL and GRO.
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

## Current Integration State

- `WiP20261` has already absorbed TINKERCELL.
- `WiP20261` has already absorbed GRO.
- `WiP20261_KERNEL_GUI` is not yet integrated into `WiP20261`.
- `WiP20261_KERNEL_GUI` was synchronized with `origin/WiP20261` at `5df02726`
  through merge commit `2d74347b`.
- `WiP20261` advanced again to `2e439a62`.
- `WiP20261_KERNEL_GUI` has now been re-synchronized with `origin/WiP20261` at
  `2e439a62` through merge commit `f13d7c9b`.
- The branch is prepared for final merge into the base, pending any final reviewer or
  maintainer action.
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
- TINKERCELL and GRO are now in the base, so synchronization with
  `origin/WiP20261` is authorized for final integration preparation.
- Before important pushes, prefer this routine:
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

## Canonical Workspace And Build Paths

- **Workspace area:** `/home/rafaelcancian/Laboratory/Software/Educational_Projects/GenESyS/github_repository`
- **Canonical clone root:** `/home/rafaelcancian/Laboratory/Software/Educational_Projects/GenESyS/github_repository/WiP20261_KERNEL_GUI/Genesys-Simulator`
- Do not use old build/cache paths that point to `/home/rafaelcancian/Genesys-Simulator`.
- If old CMake caches pointing to `/home/rafaelcancian/Genesys-Simulator` are found,
  discard them and regenerate the build from the canonical clone root.
- Fresh `/tmp` build directories are acceptable when they avoid stale or corrupted
  local CMake caches.

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
  biological modeling integrations related to TINKERCELL or GRO.

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

### Latest Plugin Manager UX Refinement

Files:

- `source/applications/gui/qt/GenesysQtGUI/dialogs/dialogpluginmanager.cpp`
- `source/applications/gui/qt/GenesysQtGUI/dialogs/dialogpluginmanager.h`
- `source/applications/gui/qt/GenesysQtGUI/dialogs/dialogpluginmanager.ui`

Intent:

- Make `Resolve / Load Selected` always try to advance selected problem plugins,
  not only install missing system dependencies.
- For each selected issue row, recheck current system dependency state, run install
  commands only when there is still an installable missing system dependency, and
  always call `PluginManager::insert(filename)` afterward.
- Support multiple selected problem rows by copying selected `PluginLoadIssue`
  values before mutating the manager state.
- Keep the operation scoped to the visible `Plugins with problems` tab so stale
  hidden selections from another tab are not processed.
- Let `PluginManager::insert()` handle already-loaded plugins, dynamic plugin
  dependencies, revalidation, issue replacement, and successful issue clearing.
- Rebuild both tables and refresh the main plugin catalog after the resolve/load
  pass.
- Report loaded plugins, still-blocked files, install command failures or
  cancellations, and captured install feedback in the final operation dialog.

UI/layout changes:

- Removed the `TextEdit Plugin Details` side panel and the horizontal splitter from
  the Plugin Manager dialog.
- Gave `groupBoxAutoload` a vertical `Maximum` size policy so it does not consume
  the main dialog growth.
- Gave the plugin table container and `tabWidgetPluginTables` expanding size
  policies so the tables are the primary resizable area.
- Moved relevant plugin metadata into `TableWidgetPlugins` columns:
  plugin, state, kind, category, version, date, author, inputs, outputs, flags,
  dynamic dependencies, system dependencies, fields, observation, description, and
  language template.
- Moved problem diagnostics into `TableWidgetPluginIssues` columns:
  plugin/file, plugin type, problem, system dependency, status, check command,
  install command, diagnostic, and suggested action.
- Long table values are compacted in cells and kept available through tooltips.
- `TableWidgetPluginIssues` now uses extended row selection for predictable
  multi-plugin resolve/load operations.

Important behavior consequence:

- A plugin like `R Simulator` that remains blocked after its base/system dependency
  was resolved can now be selected and retried. If the dependency chain is now
  satisfiable, it should move from the problem table to the loaded plugin table.
  If it is still blocked, the issue remains visible with refreshed diagnostics.

### Latest Plugin Directory Structural Refactor

Intent:

- Reorganize plugin implementation files into category folders derived from each
  plugin's `PluginInformation::getCategory()` value.
- Apply the structure to both component plugins under `source/plugins/components`
  and data-definition plugins under `source/plugins/data`.
- Keep helper files near the plugin category that owns them, including
  `CellularAutomata` helpers under `components/Logic/CellularAutomata`.
- Keep source includes stable by using paths rooted at the repository `source`
  include directory, such as `plugins/components/Decisions/Decide.h`, instead of
  adding deeper relative `../../..` paths.

Key implementation details:

- `source/plugins/data/CMakeLists.txt` now uses `GLOB_RECURSE`, matching the
  existing recursive component build behavior.
- `PluginConnectorDummyImpl1.cpp` and code/tests/examples that include plugin
  headers were updated to the categorized paths.
- `PluginInformation::categoryFolderName()` centralizes category-to-folder
  normalization.
- `PluginManager::sourceIncludePathFor()` resolves future generated C++ include
  paths from loaded plugin metadata, including built-in kernel data definitions.
- `CppSerializer` and the Qt `CppModelExporter` now use the plugin manager include
  resolver instead of constructing `plugins/components/<Class>.h` and
  `plugins/data/<Class>.h` paths directly.

Validation:

- `cmake --preset gui-app`
- `cmake --build --preset gui-app`
- `cmake --preset tests-kernel-unit`
- `cmake --build --preset tests-kernel-unit-run`
- `git diff --check`

Known structural notes:

- `Match` declares category `Decision` while most related decision plugins declare
  `Decisions`; it was kept in folder `Decision` to avoid silently recategorizing it.
- `DefaultNode` does not call `setCategory()`, so it uses the default
  `Discrete Processing` category and was placed under `components/DiscreteProcessing`.
- Some old terminal examples still reference obsolete plugins such as `EFSM`,
  `FSM_State`, and `OLD_FiniteStateMachine`; those plugin files were already absent
  from the current tree and were not part of this structural move.

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
- The generated inventory was rechecked after synchronizing with the base.

### Tests

File:

- `source/tests/unit/test_runtime_pluginmanager.cpp`

Intent:

- Cover stored plugin load issues for invalid plugin insertion.
- Cover refusal to insert plugins with missing system dependencies when confirmation
  or installation is unavailable.
- Cover successful installation/revalidation through faked command execution.
- Cover clearing a stored issue after a successful retry.
- This file is the principal expected conflict with the base after TINKERCELL and
  GRO.

## Integration Dependencies

- GUI-facing plugin diagnostics depend on the kernel diagnostic API added in
  `PluginManager.*`.
- The generated method inventory was regenerated during validation and remained
  unchanged after synchronizing with the base.
- `source/tests/unit/test_runtime_pluginmanager.cpp` is the highest-risk conflict
  point because TINKERCELL and GRO have already changed the base.
- Conflict resolution should preserve both the base coverage from TINKERCELL/GRO
  and the additional KERNEL_GUI plugin diagnostics coverage.

## Current Branch State

- `WiP20261_KERNEL_GUI` exists locally.
- `origin/WiP20261_KERNEL_GUI` exists remotely.
- The local branch tracks `origin/WiP20261_KERNEL_GUI`.
- The branch was last pushed successfully after commit `71db615c` before the current
  synchronization work.
- Recent branch commits include:
  - `6a02e61f Track plugin load diagnostics in PluginManager`
  - `f0a4bfb9 Show recoverable plugin dependency issues in GUI`
  - `c898843a Cover plugin load issue diagnostics`
  - `63c2824a Record KERNEL_GUI validation status`
  - `c324d263 Consolidate KERNEL_GUI persistent context`
  - `62c3dca2 Document KERNEL_GUI integration hold`
  - `71db615c Move KERNEL_GUI memory to agent-specific file`
  - `fee82a90 Update KERNEL_GUI memory for base sync`
  - `2d74347b Merge remote-tracking branch 'origin/WiP20261' into WiP20261_KERNEL_GUI`
  - `f13d7c9b Merge remote-tracking branch 'origin/WiP20261' into WiP20261_KERNEL_GUI`
- After the latest local Plugin Manager UX refinement, the working tree has
  uncommitted changes in:
  - `source/applications/gui/qt/GenesysQtGUI/dialogs/dialogpluginmanager.cpp`
  - `source/applications/gui/qt/GenesysQtGUI/dialogs/dialogpluginmanager.h`
  - `source/applications/gui/qt/GenesysQtGUI/dialogs/dialogpluginmanager.ui`
  - `KERNEL_GUI_ContextMemory.md`
- The latest merge from `origin/WiP20261` completed without conflicts and only
  brought base-side updates to `TINKERCELL_ContextMemory.md`.
- `source/tests/unit/test_runtime_pluginmanager.cpp` now contains both the KERNEL_GUI
  plugin load diagnostics tests and the GRO `GroProgram`/`BacteriaColony` runtime
  plugin coverage.
- Obsolete KERNEL_GUI memory files have been removed from active use:
  - `documentation/developers/KERNEL_GUI_context.md` was deleted;
  - `ContextMemmory.md` was previously renamed to `KERNEL_GUI_ContextMemory.md`;
  - no generic `ContextMemory.md` exists in the root.

## Pull Request State

- Final integration PR: `#371`
- PR title: `Merge WiP20261_KERNEL_GUI into WiP20261`
- PR branch pair: `WiP20261_KERNEL_GUI` -> `WiP20261`
- PR URL: `https://github.com/rlcancian/Genesys-Simulator/pull/371`
- Latest inspected PR head before recording this memory update:
  `ed7257c9c8360a2b2ab90e7118598217686ba986`
- Latest inspected base: `2e439a6235f57b3feeb615292d3f133e4c63a514`
- GitHub API state at inspection: open, not draft, `mergeable: true`.
- Local Git state at inspection: `origin/WiP20261` is an ancestor of `HEAD`, and
  `HEAD` matches `origin/WiP20261_KERNEL_GUI`.
- Local merge simulation `git merge-tree origin/WiP20261 HEAD` produced a clean tree
  (`08583bb9ffb9e7c66c9899bf49e30a34de58ba08`) with no conflicts.
- No reviews or review threads were present.
- No commit status checks or workflow runs were reported for the PR head.
- Diagnosis: the GitHub non-mergeable reports observed immediately after branch
  updates were transient mergeability calculation state, not a real branch conflict,
  stale branch, or technical blocker.
- Corrective action required: none beyond recording this diagnosis.

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

## Validation After Base Synchronization

The following validation was run after merging `origin/WiP20261`:

```bash
cmake -S . -B /tmp/genesys-kernel-unit -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=23 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DCMAKE_CXX_EXTENSIONS=OFF -DGENESYS_BUILD_GUI_APPLICATION=OFF -DGENESYS_BUILD_WEB_APPLICATION=OFF -DGENESYS_BUILD_PLUGINS=ON -DGENESYS_BUILD_PARSER=ON -DGENESYS_BUILD_KERNEL=ON -DGENESYS_BUILD_TERMINAL_APPLICATION=OFF -DGENESYS_BUILD_TESTS=ON -DGENESYS_BUILD_SMOKE_TESTS=OFF
cmake --build /tmp/genesys-kernel-unit --target genesys_kernel_unit_tests
/tmp/genesys-kernel-unit/source/tests/unit/genesys_test_runtime_pluginmanager
ctest --test-dir /tmp/genesys-kernel-unit --output-on-failure
cmake -S . -B /tmp/genesys-gui-app -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=23 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DCMAKE_CXX_EXTENSIONS=OFF -DGENESYS_BUILD_GUI_APPLICATION=ON -DGENESYS_BUILD_WEB_APPLICATION=OFF -DGENESYS_BUILD_PLUGINS=ON -DGENESYS_BUILD_PARSER=ON -DGENESYS_BUILD_KERNEL=ON -DGENESYS_BUILD_TERMINAL_APPLICATION=OFF -DGENESYS_BUILD_TESTS=OFF
cmake --build /tmp/genesys-gui-app --target genesys_gui
```

Observed test status:

- The GUI application target `genesys_gui` built successfully from a fresh `/tmp`
  build directory.
- The focused runtime plugin manager test passed with 7 tests.
- The configured kernel-unit `ctest` run passed with 1257 executed tests and 0
  failures.
- Four tests were disabled.
- Two R-related tests were skipped because `Rscript` was unavailable.
- Existing local build directories were not reused because previous validation found
  CMake caches pointing to `/home/rafaelcancian/Genesys-Simulator`, a different
  checkout path.

## Validation After Latest Base Synchronization

The latest synchronization with `origin/WiP20261` at `2e439a62` affected only
`TINKERCELL_ContextMemory.md` and had no conflicts. Validation still uses the
canonical clone root and fresh `/tmp` build directories to avoid stale local caches.

The following validation was run after the latest merge:

```bash
cmake -S . -B /tmp/genesys-kernel-unit -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=23 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DCMAKE_CXX_EXTENSIONS=OFF -DGENESYS_BUILD_GUI_APPLICATION=OFF -DGENESYS_BUILD_WEB_APPLICATION=OFF -DGENESYS_BUILD_PLUGINS=ON -DGENESYS_BUILD_PARSER=ON -DGENESYS_BUILD_KERNEL=ON -DGENESYS_BUILD_TERMINAL_APPLICATION=OFF -DGENESYS_BUILD_TESTS=ON -DGENESYS_BUILD_SMOKE_TESTS=OFF
cmake --build /tmp/genesys-kernel-unit --target genesys_kernel_unit_tests
/tmp/genesys-kernel-unit/source/tests/unit/genesys_test_runtime_pluginmanager
ctest --test-dir /tmp/genesys-kernel-unit --output-on-failure
cmake -S . -B /tmp/genesys-gui-app -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=23 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DCMAKE_CXX_EXTENSIONS=OFF -DGENESYS_BUILD_GUI_APPLICATION=ON -DGENESYS_BUILD_WEB_APPLICATION=OFF -DGENESYS_BUILD_PLUGINS=ON -DGENESYS_BUILD_PARSER=ON -DGENESYS_BUILD_KERNEL=ON -DGENESYS_BUILD_TERMINAL_APPLICATION=OFF -DGENESYS_BUILD_TESTS=OFF
cmake --build /tmp/genesys-gui-app --target genesys_gui
```

Observed status:

- The focused runtime plugin manager test passed with 7 tests.
- The configured kernel-unit `ctest` run passed with 1257 executed tests and 0
  failures.
- Four tests were disabled.
- Two R-related tests were skipped because `Rscript` was unavailable.
- The GUI application target `genesys_gui` was up to date in `/tmp/genesys-gui-app`.
- No old `CMakeCache.txt` files pointing to `/home/rafaelcancian/Genesys-Simulator`
  were found in the canonical clone root during the latest check.

## Validation After Plugin Manager Resolve/Load UX Refinement

The following validation was run after the local Plugin Manager dialog changes that
make `Resolve / Load Selected` retry selected problem plugins and remove the details
side panel:

```bash
cmake --preset gui-app
cmake --build --preset gui-app
cmake --preset tests-kernel-unit
cmake --build --preset tests-kernel-unit-run
```

Observed status:

- The first `cmake --build --preset gui-app` failed because the edited `.ui` file
  used unsupported `stretch` attributes on layout `<item>` elements. The `.ui` was
  corrected to express the layout priority through `QSizePolicy` instead.
- The subsequent GUI build passed. `uic`, `moc`, `dialogpluginmanager.cpp`, and the
  final `genesys_qt_gui_application` link completed successfully.
- `cmake --build --preset tests-kernel-unit-run` passed after configuring
  `build/tests-kernel-unit`.
- The kernel test run included `genesys_test_runtime_pluginmanager`, preserving the
  existing coverage for retrying plugin insertion and clearing stored
  `PluginLoadIssue` records.
- The build emitted only pre-existing warnings outside the modified dialog, such as
  missing return statements in `FitterDummyImpl.cpp` and deprecated
  `QString::count()` use in the bundled Qt property browser.
- The specific local R dependency scenario was not manually reproduced in the GUI,
  but the generic path that retries selected problem plugins now compiles and is
  backed by the existing `PluginManager` retry/issue-clearing tests.

## Open Pending Items

- Continue future KERNEL_GUI work only on `WiP20261_KERNEL_GUI`.
- Use only `KERNEL_GUI_ContextMemory.md` as the active context memory file.
- Do not use generic root memory files such as `ContextMemory.md` or
  `ContextMemmory.md` as active memory.
- Do not leave the primary active memory in `documentation/developers/`.
- Do not recreate `documentation/developers/communication.md`.
- Commit the structural plugin directory refactor and this memory update, then push
  `WiP20261_KERNEL_GUI` to `origin/WiP20261_KERNEL_GUI` when ready.
- Proceed with final merge into the base when the maintainer is ready.
- PR `#371` is ready for final merge unless GitHub reports a new base update or
  repository policy change.
- If plugin dependency recovery receives more changes, keep them small and preserve
  kernel/GUI separation.
- Future work may refine terminal handling and password/sudo feedback during
  dependency installation.

## Risks And Attention Points

- `PluginManager.*`, `DialogPluginManager.*`, `DialogUtilityController.*`, and
  `mainwindow.cpp` are integration-heavy files and may conflict with other
  developers.
- `source/tests/unit/test_runtime_pluginmanager.cpp` is the main expected conflict
  with the base after TINKERCELL and GRO.
- Interactive installation depends on terminal emulator availability and local sudo
  configuration.
- External tools such as R, Scilab, Octave, and libSBML may have OS-specific install
  commands and verification behavior.
- TINKERCELL and GRO are already in the base and may rely on the same plugin
  metadata and dependency diagnostic path.

## Likely Next Steps

- Push `WiP20261_KERNEL_GUI` after the latest synchronization and validation memory
  update are recorded.
- Final integration into `WiP20261` can proceed from the synchronized branch.
- If another base update happens before final integration, repeat the fetch/merge and
  focused validation cycle.
- Monitor PR `#371`; if GitHub temporarily reports unknown/non-mergeable again,
  recheck after the mergeability calculation refreshes before assuming a real
  conflict.

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

## Latest Runtime Preferences And Visual Theme Work

- The active user protocol now requires every assistant response to start with the
  exact line `KERNEL_GUI` and end with the exact line `----------`.
- A runtime preferences architecture was implemented for GenesysQtGUI.
- `SystemPreferences::load()` and `SystemPreferences::save()` no longer act as
  stubs; they read/write JSON at Qt's `QStandardPaths::AppConfigLocation`, file
  name `preferences.json`, with fallback to `~/.genesys/preferences.json` when Qt
  cannot resolve a config location.
- The JSON schema currently has top-level sections:
  - `startup`: `startMaximized`, `modelAtStart`, `specificModelFile`,
    `lastModelFile`.
  - `plugins`: `autoLoad`, `checkSystemPackagesAtStart`.
  - `diagnostics`: `traceLevel`.
  - `view`: `theme`, `interfaceStyle`, `fontPointSize`,
    `diagramUsesThemeColors`.
- Startup model modes are `none`, `new`, `specific`, and `last`.
- Theme modes are `light` and `dark`; interface styles are `classic` and
  `modern`.
- `GuiThemeManager` applies Qt application palette/stylesheet choices and updates
  model canvas/grid colors when diagram theme colors are enabled.
- `main.cpp` now loads preferences and applies application theme before creating
  `MainWindow`; the previous compile-time GUI start-maximized trait no longer
  controls startup window state.
- `mainwindow.cpp` now reads trace level from `SystemPreferences` instead of the
  compiled `TraitsApp` default and uses the new startup model mode enum.
- `DialogSystemPreferences` was expanded into functional General and View tabs
  bound to runtime preferences.
- Opening/saving a model records the last successful model path for future
  startup.
- Validation performed:
  - `cmake --build --preset gui-app` passed.
  - `cmake --build --preset tests-kernel-unit-run` passed.
  - `git diff --check` passed.
  - Offscreen smoke test with temporary JSON under `/tmp/genesys-pref-test`
    loaded dark/modern/no-model startup preferences and stayed running until the
    expected timeout without startup crash.
- Deliberate limitation: component and `ModelDataDefinition` body painting still
  mostly uses existing `TraitsGUI` colors; this iteration only creates the safe
  runtime preference and canvas/grid theme base for future drawing-style work.
