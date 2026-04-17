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
- **Current objective:** Continue KERNEL_GUI work on `WiP20261_KERNEL_GUI` through
  explicit, user-confirmed phases. The latest completed structural base is
  `f232882e` (`Organize plugins by declared category`), the completed runtime GUI
  preferences/theme phase is `a1c2d9fa` (`Add runtime GUI preferences and themes`),
  and the latest completed visual connection phase is `36ffec62`
  (`Add Modern Fusion curved connections`). The latest completed GMDD visual
  hierarchy phase is `0a0ec979` (`Improve GMDD visual hierarchy layout`).
- **Main technical scope:** Qt GUI, Plugin Manager dialog, MainWindow startup flow,
  kernel-facing plugin lifecycle contracts, plugin diagnostics exposed to the GUI,
  and focused tests that cover GUI-facing kernel behavior.

## Permanent Operating Protocol

- **Response format:** every assistant response must start with a first line
  containing only `KERNEL_GUI`; the technical body follows; the final line must
  contain only `----------`.
- **Phase policy:** before acting, recover the current plan and state from this
  memory and reanalyze the real repository state. Identify the next logical phase,
  execute only that phase, then stop and request explicit user confirmation before
  advancing to another phase. Do not automatically continue into the following
  phase.
- **Context memory policy:** at the end of every completed phase, update this file
  with what was done, the current state, relevant commits, remaining limitations,
  and the suggested next phase.
- **Repository reality policy:** do not rely only on remembered state. Re-read the
  working tree, branch state, relevant source files, build files, and tests before
  proposing or implementing technical changes.
- **Stage policy:** stage changes intentionally and in coherent groups. Do not mix
  unrelated changes in the same staged set. Do not leave important completed phase
  changes untracked or unstaged without an explicit technical reason.
- **Commit policy:** create clear, auditable commits for completed phases or
  validated logical subunits. Commit messages must be objective and technically
  representative. Do not mix independent changes in one commit, and record created
  commits in the response.
- **Push/publication policy:** push is separate from stage and commit. Do not push
  automatically just because a commit was created. Publish to the remote only when
  explicitly requested by the user or when an already defined operational flow
  requires publication. Responses must distinguish local commits from remote
  publication.
- **Continuity policy:** preserve continuity with the current plan recorded in this
  memory. If the user changes scope, update the memory and only then proceed within
  the new confirmed phase.

## Canonical Branches

- **Integration base branch:** `WiP20261`
- **KERNEL_GUI working branch:** `WiP20261_KERNEL_GUI`
- **Remote:** `origin`

KERNEL_GUI must work only on `WiP20261_KERNEL_GUI`. The shared branch `WiP20261` is
the integration base and must not be modified or merged into directly unless the
user explicitly requests that operation.

## Current Integration State

- The branch currently carries KERNEL_GUI work on top of the latest locally recorded
  base synchronization, plus later KERNEL_GUI commits.
- The official plugin structural refactor is `f232882e`
  (`Organize plugins by declared category`).
- The latest completed runtime preferences/theme commit is `a1c2d9fa`
  (`Add runtime GUI preferences and themes`).
- Final integration into `WiP20261`, PR state, and remote mergeability must always be
  checked fresh before taking integration action.
- Historical conflict attention remains around
  `source/tests/unit/test_runtime_pluginmanager.cpp`, because multiple plugin-related
  branches have touched that coverage.

## Git Policy

- Work only on `WiP20261_KERNEL_GUI`.
- Treat `WiP20261` as the base branch for integration.
- Use `git status`, diffs, and relevant source inspection before staging or
  committing.
- Stage only coherent sets of changes that belong to the current phase or logical
  subunit.
- Make small, frequent, coherent commits when a phase or validated subunit is
  complete.
- Do not leave a completed phase without a commit unless there is an explicit
  technical reason, and record that reason in the response and in this memory.
- Do not push automatically. Pushing is publication and requires explicit user
  request or a previously established operational flow that clearly calls for it.
- Ask the user before destructive operations, before resolving significant ambiguity
  with high impact, or when there is exceptional risk.
- Synchronization with `origin/WiP20261` may be used for integration preparation
  when it is part of the confirmed phase, but it must not silently expand the phase
  scope.
- Do not rebase shared work unless explicitly instructed.

## Repository Language Policy

- The user communicates in Portuguese.
- Source code, identifiers, comments in code, Doxygen, and internal technical
  repository documentation must remain in English.
- This context memory file is internal technical documentation and must remain in
  English.

## Canonical Workspace And Build Paths

- **Workspace area:** `/home/rafaelcancian/Laboratory/Software/Educational_Projects/GenESyS/GitHub`
- **Canonical clone root:** `/home/rafaelcancian/Laboratory/Software/Educational_Projects/GenESyS/GitHub/WiP20261_KERNEL_GUI/Genesys-Simulator`
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
- The official plugin structural refactor is local and remote at commit `f232882e`
  (`Organize plugins by declared category`).
- The runtime GUI preferences and visual theme phase is local and remote at commit
  `a1c2d9fa` (`Add runtime GUI preferences and themes`).
- The diagram item render strategy phase is local and remote at commit `107a6539`
  (`Add diagram item render strategies`).
- The Modern Fusion interface/body-rendering visual refinement is local and remote
  through commit `366e25a6` (`Record Modern Fusion visual checkpoint`).
- The Modern Fusion curved connection phase is local at commit `36ffec62`
  (`Add Modern Fusion curved connections`).
- The GMDD visual hierarchy phase is local at commit `0a0ec979`
  (`Improve GMDD visual hierarchy layout`) and has not been pushed unless a later
  user request explicitly publishes it.
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
  - `1fe51d47 Improve plugin manager resolve load workflow`
  - `f232882e Organize plugins by declared category`
  - `a1c2d9fa Add runtime GUI preferences and themes`
  - `6baa2cc3 Normalize KERNEL_GUI operating memory`
  - `107a6539 Add diagram item render strategies`
  - `83686a93 Record diagram render strategy phase`
  - `77f86538 Add GUI render strategy unit test`
  - `8cfb0249 Record GUI render strategy test checkpoint`
  - `0b0d1d9c Make Modern Fusion visually distinct`
  - `366e25a6 Record Modern Fusion visual checkpoint`
  - `36ffec62 Add Modern Fusion curved connections`
  - `97c15c66 Record curved connection visual checkpoint`
  - `0a0ec979 Improve GMDD visual hierarchy layout`
- At the start of the diagram item render strategy phase, the branch was one commit
  ahead of `origin/WiP20261_KERNEL_GUI` because of the local memory-normalization
  commit `6baa2cc3`; the functional working tree was otherwise clean.
- The latest merge from `origin/WiP20261` completed without conflicts and only
  brought base-side updates to `TINKERCELL_ContextMemory.md`.
- `source/tests/unit/test_runtime_pluginmanager.cpp` now contains both the KERNEL_GUI
  plugin load diagnostics tests and the GRO `GroProgram`/`BacteriaColony` runtime
  plugin coverage.
- Obsolete KERNEL_GUI memory files have been removed from active use:
  - `documentation/developers/KERNEL_GUI_context.md` was deleted;
  - `ContextMemmory.md` was previously renamed to `KERNEL_GUI_ContextMemory.md`;
  - no generic `ContextMemory.md` exists in the root.

## Historical Pull Request State

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
- Corrective action required at the time: none beyond recording this diagnosis.
- This section is historical context only. Do not treat this PR state as current
  without fresh GitHub and repository inspection.

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
- Current active phase: record the completed GMDD visual hierarchy work in this
  memory. Do not start another technical phase until the user explicitly
  confirms the next step.
- The structural plugin directory refactor is already complete in `f232882e`.
- The runtime GUI preferences/theme phase is already complete in `a1c2d9fa`.
- Proceed with final merge into the base only when the maintainer explicitly directs
  that operation.
- PR `#371` historical notes remain useful as integration context, but future PR or
  merge decisions must be based on fresh repository and GitHub state.
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

- Ask the user to confirm the next technical phase before changing GUI, kernel,
  plugin, build, or test code again.
- Possible next phase: add focused automated GUI-rendering tests or a screenshot
  harness for classic versus organic diagram rendering, if the user wants visual
  regression coverage beyond compile and smoke validation.
- Possible next visual phase: refine connection arrowheads, labels, hit-shape
  affordances, or screenshot-based regression coverage for Classic Desktop versus
  Modern Fusion connection styles.
- Possible next GMDD phase: add screenshot-based visual regression coverage or a
  small scene-level harness that verifies GMDD upper/lower hierarchy placement with
  concrete model fixtures.
- Do not push local commits unless the user explicitly asks for publication.
- When a next phase is confirmed, first re-read the real repository state and the
  relevant source/build/test files, then execute only that phase.

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

- Commit: `a1c2d9fa` (`Add runtime GUI preferences and themes`).
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
- Follow-up completed in `107a6539`: component and `ModelDataDefinition` body
  painting now delegates to a runtime-selected render strategy.

## Latest Modern Fusion Connection Style Work

- Commit: `36ffec62` (`Add Modern Fusion curved connections`).
- The connection visual phase was completed as a focused extension of the existing
  runtime `SystemPreferences::interfaceStyle()` infrastructure.
- `GraphicalConnectionStyle` was introduced as the central geometry/style helper for
  connection paths:
  - Classic Desktop model connections keep the traditional direct/orthogonal route.
  - Modern Fusion model connections use cubic `QPainterPath` curves with a
    perpendicular bend so the path is visibly distinct from straight or orthogonal
    routes.
  - Classic Desktop diagram/data-definition connections keep straight dashed lines.
  - Modern Fusion diagram/data-definition connections use quadratic curved paths.
  - `pointAtProgress()` samples `QPainterPath` by path length, so animation progress
    follows the same geometric path that is painted.
- `GraphicalConnection` now delegates path construction to `GraphicalConnectionStyle`
  and exposes `animationPathForImage()` for animation code. Its selected handles and
  pen styling remain classic in Classic Desktop and become rounded/blue/antialiased
  in Modern Fusion.
- `AnimationTransition` now preserves the legacy segment interpolation for Classic
  Desktop and uses the actual curved `QPainterPath` for Modern Fusion entity
  movement. This avoids the old mismatch where a curved visual edge could still have
  an invisible segmented animation route.
- `GraphicalDiagramConnection` now draws diagram/data-definition links from the same
  style helper, overrides `boundingRect()` and `shape()` for curved hit geometry, and
  orients its diamond marker from the curve tangent.
- `source/tests/unit/test_gui_render_strategy.cpp` now covers modern curved model
  connection geometry and modern curved diagram connection geometry.
- Validation performed:
  - `git diff --check` passed.
  - `cmake --build --preset gui-app` passed.
  - `cmake --build --preset tests-kernel-unit-run` passed, including the new GUI
    connection style tests.
  - Offscreen smoke test with temporary JSON preferences setting
    `view.interfaceStyle = "modern"` and no plugin autoload started the GUI without
    startup crash and ran until the expected timeout.
- Remaining limitation:
  - No screenshot-based visual regression harness exists yet for connection styles;
    validation is currently compile/unit/smoke based.

## Latest GMDD Visual Hierarchy Work

- Commit: `0a0ec979` (`Improve GMDD visual hierarchy layout`).
- Scope: focused improvements to `GraphicalModelDataDefinition` presentation and
  initial placement in `GraphicalModelBuilder`; no plugin structure or unrelated GUI
  areas were changed.
- Non-editable GMDD visual distinction:
  - `GraphicalModelDataDefinition::renderContext()` now lowers only the fill alpha
    for non-editable data definitions.
  - Text, border, selection handles, and the render strategy remain opaque enough to
    preserve readability.
  - Selected non-editable GMDD use a slightly stronger fill alpha than unselected
    non-editable GMDD, but remain less opaque than editable GMDD.
  - `setEditableInPropertyEditor()` now schedules repaint so editability changes are
    reflected immediately.
- GMDD placement:
  - `GraphicalModelBuilder` now marks GMDD editability during the data-definition
    layer synchronization rather than waiting for later property-editor selection.
  - Direct editable/internal GMDD are initially placed in an upper arc around their
    owning `GraphicalModelComponent`.
  - Direct shared/statistics/non-editable GMDD are initially placed in a lower arc
    around their owning `GraphicalModelComponent`.
  - Recursive children of editable GMDD are placed in an upper arc around the GMDD
    parent.
  - Recursive children of non-editable GMDD are placed in a lower arc around the
    GMDD parent.
  - Existing/manual/persisted positions are preserved because the new layout only
    positions newly created GMDD during synchronization.
  - A per-sync positioned set prevents the same new GMDD from being repositioned by
    multiple relationships.
- Validation performed:
  - `git diff --check` passed.
  - `cmake --build --preset gui-app` passed.
  - `cmake --build --preset tests-kernel-unit-run` passed.
  - Offscreen smoke test with temporary modern-style JSON preferences started the
    GUI without startup crash and ran until the expected timeout.
- Remaining limitation:
  - There is still no screenshot-based visual regression harness for GMDD placement;
    improvement was validated by code inspection, build, unit suite, and offscreen
    startup smoke.

## Latest Diagram Item Render Strategy Work

- Commit: `107a6539` (`Add diagram item render strategies`).
- Introduced `GraphicalModelItemRenderStrategy` and `GraphicalModelItemRenderer`
  under `source/applications/gui/qt/GenesysQtGUI/graphicals/`.
- `GraphicalModelDataDefinition` and `GraphicalModelComponent` now build a
  `GraphicalModelItemRenderContext` and delegate `paint()` and `shape()` to the
  renderer.
- The classic strategy preserves the existing rectangular/raised-path visual and
  keeps rectangular hit shape for compatibility.
- The organic strategy uses antialiased oval/capsule bodies, radial gradients,
  softer highlights, oval selection handles, and strategy-specific `shape()` for
  hit testing while keeping the existing `boundingRect()` and port geometry stable.
- Strategy selection is tied to `SystemPreferences::interfaceStyle()`:
  - `classic` uses the traditional strategy.
  - `modern` uses the organic strategy.
- `DialogSystemPreferences` now labels the options as `Classic rectangular` and
  `Modern organic`.
- `GuiThemeManager::applyModelGraphicsTheme()` now always updates the scene after
  preference changes, even when only the render style changed and diagram theme
  colors are disabled.
- Validation performed:
  - `git diff --check` passed.
  - `cmake --build --preset gui-app` passed.
  - `cmake --build --preset tests-kernel-unit-run` passed.
  - Offscreen smoke test with temporary JSON under
    `/tmp/genesys-render-pref/GenESyS/Genesys-Simulator/preferences.json` loaded
    dark/modern/no-model preferences and stayed running until the expected timeout
    without startup crash.
- Remaining limitation: no automated screenshot/pixel regression test exists yet
  for comparing classic and organic rendering. Validation is currently build,
  code-path, preference-load, and startup-smoke based.

## Latest GUI Render Strategy Test Work

- Commit: `77f86538` (`Add GUI render strategy unit test`).
- Added `source/tests/unit/test_gui_render_strategy.cpp`, a focused Qt offscreen
  unit test for the diagram item render strategy infrastructure.
- The test covers:
  - runtime strategy selection from `SystemPreferences::InterfaceStyle`;
  - shape/hit-area difference between classic rectangular and organic
    oval/capsule rendering;
  - nonblank pixel rendering and visible pixel-level difference between classic
    and organic output.
- `source/tests/unit/CMakeLists.txt` now creates `genesys_test_gui_render_strategy`
  when Qt Core/Gui/Widgets is available. The target compiles only the render
  strategy and `SystemPreferences` implementation needed for this focused GUI
  check, without building the full Qt application under the kernel-unit preset.
- The test is integrated into `genesys_kernel_unit_tests` and into
  `genesys_kernel_unit_tests_run` as an additional command when the Qt target
  exists.
- Validation performed:
  - `cmake --preset tests-kernel-unit` passed.
  - `cmake --build --preset tests-kernel-unit-run` passed, including the 3 new
    `GuiRenderStrategy` tests.
  - `git diff --check` passed.
  - `cmake --build --preset gui-app` passed with no work required.
- Current state: this phase has a stable local checkpoint. No push was performed.
- Suggested next phase, only after explicit user confirmation: either pause at
  this checkpoint, or continue with a manual/screenshot-level GUI validation
  harness for the actual scene if visual regression coverage needs to move
  beyond unit-level QImage pixel checks.

## Latest Modern Fusion Visual Completion Work

- Commit: `0b0d1d9c` (`Make Modern Fusion visually distinct`).
- `GuiThemeManager::applyApplicationTheme()` now makes `SystemPreferences::Modern`
  visibly different from classic desktop mode:
  - captures the original application style before switching to Fusion so classic
    light mode can return to the original desktop style;
  - applies a dedicated Modern Fusion stylesheet for light and dark themes;
  - restyles menus, menubars, toolbars, status bars, group boxes, tabs, table/tree
    headers, editable controls, buttons, and scrollbars;
  - increases the application font point size by one point in Modern Fusion when
    no explicit font preference is configured;
  - repolishes existing widgets so changes are visible immediately after applying
    preferences.
- `DialogSystemPreferences` now labels the interface style options as
  `Classic Desktop` and `Modern Fusion`.
- `GraphicalModelItemRenderStrategy` now makes the organic rendering more visibly
  distinct:
  - component bodies use stronger blue accent outlines;
  - data definition bodies use green accent outlines;
  - selected items have heavier accent strokes and stronger shadows;
  - organic items draw an additional internal accent contour while preserving the
    existing bounding rectangle and port geometry.
- `GuiThemeManager::applyModelGraphicsTheme()` now explicitly updates every
  `QGraphicsItem`, the scene, and the graphics view viewport, ensuring preference
  changes repaint component and data-definition items immediately.
- Diagnosis for this phase: the preference and render strategy existed, but Modern
  Fusion was not perceptible enough because light modern mode only selected Fusion
  style plus a nearly minimal stylesheet, and the scene refresh did not explicitly
  update each item.
- Validation performed:
  - `git diff --check` passed.
  - `cmake --build --preset gui-app` passed.
  - `cmake --build --preset tests-kernel-unit-run` passed, including the
    `GuiRenderStrategy` pixel-difference tests.
  - Offscreen startup smoke with temporary JSON under
    `/tmp/genesys-modern-fusion/GenESyS/Genesys-Simulator/preferences.json`
    loaded `interfaceStyle=modern`, `theme=light`, `modelAtStart=none`, and stayed
    running until the expected `timeout 5` exit code.
- Current state: the phase is complete in local commits. No push was performed.
- Suggested next phase, only after explicit user confirmation: add a real
  scene-level screenshot regression harness that creates a small model, renders the
  `QGraphicsScene`, and compares classic versus Modern Fusion image characteristics.
