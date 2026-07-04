# Current Plans

## Documentation directory migration

- Date: 2026-07-03
- Branch: `WiP20261`
- Scope: migrate repository documentation from `documentation/` to `docs/`.
- Status: structural migration completed; Doxygen policy corrected; initial semantic consolidation guides created.

### Target layout

- `docs/ManualGenESyS.pdf`
- `docs/ai_assistants/`
- `docs/users/`
- `docs/developers/`

After the structural migration, `docs/ManualGenESyS.pdf` is the only ordinary documentation file directly under `docs/`.

### Decisions recorded

- Remove historical `Doxyfile.bak`.
- Do not version Doxygen intermediate/generated trees under `docs/users/generated/` or `docs/developers/generated/`.
- Version only final PDF documentation artifacts under `docs/users/` and `docs/developers/`.
- Generate Doxygen working output under `build/doxygen/...`.
- Generate Doxygen man pages under the build tree for Debian/PPA packaging workflows.
- Move historical Markdown sources temporarily to `docs/ai_assistants/oldies/` after consolidation.
- Move `TINKERCELL_context.md` to `docs/ai_assistants/oldies/`.

### Doxygen configuration policy

The historical full Doxygen configurations are preserved as `.legacy` files. The canonical Doxyfiles are small wrappers that include the legacy configuration and override repository-relative paths for the current `docs/` layout.

Run Doxygen from the repository root:

```bash
doxygen docs/users/DoxyfileUser
doxygen docs/developers/DoxyfileDeveloper
```

Canonical final PDF locations:

- `docs/users/GenESyS-User-Documentation.pdf`
- `docs/developers/GenESyS-Developer-Documentation.pdf`

Doxygen intermediate outputs remain ignored in:

- `docs/users/generated/`
- `docs/developers/generated/`

### AI-assistant stable guides created

Initial stable guides now exist for:

- build, CI, and tests;
- kernel development;
- plugin development and plugin domains;
- application development;
- tools and statistics;
- modal and hybrid simulation;
- whole-cell and SBML;
- Python integration;
- documentation governance.

### Oldies retention

`docs/ai_assistants/oldies/` is temporary. It and its contents should be removed after 2026-11-01, after relevant content has been consolidated into the main AI assistant documents or explicitly marked obsolete.

### Pending follow-up

- Review each historical Markdown file in `oldies/` and mark it as consolidated, still pending, obsolete, or discard-after-review.
- Validate Doxygen generation from the repository root.
- Validate CMake/Ninja/CTest in a local checkout.
- Adjust Debian packaging/build scripts to generate or collect Doxygen man pages from the build tree.
- Evaluate whether Debian command man pages, if required for executables, should be maintained separately as section 1 man pages instead of relying only on Doxygen API man pages.

## GUI applications separation and handoff

- Date started: 2026-07-03.
- Last handoff update: 2026-07-03.
- Base branch: `WiP20261`.
- Active WIP branch: `wip/gui-genesys-move-local-salvage-20260703-182121`.
- Active PR: #451, `WIP: preserve main GUI move attempt`.
- Current status: `httpworker`, `dataanalyser`, and `optimizer` are now split as standalone GUI targets; PR must remain draft.
- Current user decision: continue incrementally after phase 1, but keep each extraction small and validated before moving to the next GUI.

### Macro objective

The objective is to split graphical frontends currently hosted by or coupled to the main GenESyS Qt GUI into independent GUI application folders under `source/applications/gui/`, while preserving kernel, parser, plugins, tools, CMake/Ninja builds, tests, Debian packaging, and installed binary compatibility.

Target umbrella layout:

```text
source/applications/gui/
  CMakeLists.txt
  genesys/
  httpworker/
  dataanalyser/
  optimizer/
  ai_assistant/
  doexperiments/
```

### Planned GUI applications

| Build option | Planned folder | Purpose | Current status |
|---|---|---|---|
| `GENESYS_BUILD_GUI_GENESYS` | `source/applications/gui/genesys` | Main GenESyS Qt GUI | Physically moved in PR #451; still under validation |
| `GENESYS_BUILD_GUI_HTTPWORKER` | `source/applications/gui/httpworker` | Graphical control frontend for HTTP/background worker | Stand-alone app added; build validated |
| `GENESYS_BUILD_GUI_DATAANALYSER` | `source/applications/gui/dataanalyser` | Data-analysis/statistics graphical frontend | Not implemented |
| `GENESYS_BUILD_GUI_OPTIMIZER` | `source/applications/gui/optimizer` | Optimization graphical frontend | Not implemented |
| `GENESYS_BUILD_GUI_AI_ASSISTANT` | `source/applications/gui/ai_assistant` | AI-assisted modeling graphical frontend | Stand-alone app added; build validated |
| `GENESYS_BUILD_GUI_DOEXPERIMENTS` | `source/applications/gui/doexperiments` | DOE/factorial-design graphical frontend | Not implemented |

The main application, `genesys`, is physically moved. `httpworker`, `dataanalyser`, `optimizer`, and `ai_assistant` now have standalone GUI application targets and are launched from the main GUI by `QProcess`. `doexperiments` remains planned. It intentionally fails configuration if enabled before its source directory and target exist.

### Current macro position

The work is now in macro phase 5 of 6:

1. Separate and validate `source/applications/gui/genesys`. Status: in progress.
2. Separate `source/applications/gui/httpworker`. Status: complete and build-validated.
3. Separate `source/applications/gui/dataanalyser`. Status: complete and build-validated.
4. Separate `source/applications/gui/optimizer`. Status: complete and build-validated.
5. Separate `source/applications/gui/ai_assistant`. Status: complete and build-validated.
6. Separate `source/applications/gui/doexperiments`. Status: not started.

The next agent should keep the same incremental pattern and move on only after the current extraction is validated.

### What has been completed in phase 1

- Preserved a local move attempt in branch `wip/gui-genesys-move-local-salvage-20260703-182121`.
- Opened draft PR #451 against `WiP20261`.
- Moved the main GUI implementation from `source/applications/gui/qt/GenesysQtGUI` to `source/applications/gui/genesys`.
- Added a GUI umbrella CMake layer at `source/applications/gui/CMakeLists.txt`.
- Added a CMake entrypoint for the moved main GUI at `source/applications/gui/genesys/CMakeLists.txt`.
- Preserved executable target `genesys_qt_gui_application` and compatibility custom target `genesys_gui`.
- Preserved installed binary output name `genesys-gui`.
- Added a standalone GUI application target for the WebWorker controller under `source/applications/gui/httpworker`.
- Added a `ToolLauncher` helper so the main GUI can start the WebWorker app by `QProcess` instead of embedding its dialog.
- Added the `gui-httpworker` CMake configure/build presets.
- Removed the direct `MainWindow` ownership of `WebWorkerDialog`.
- Added a standalone GUI application target for the Data Analyzer workstation under `source/applications/gui/dataanalyser`.
- Added the `gui-dataanalyser` CMake configure/build presets.
- Routed the Data Analyzer menu action through `QProcess` instead of constructing its window inside the main GUI.
- Added a standalone GUI application target for the Optimizer workstation under `source/applications/gui/optimizer`.
- Added the `gui-optimizer` CMake configure/build presets.
- Routed the Optimizer menu action through `QProcess` instead of constructing its window inside the main GUI.
- Added a standalone GUI application target for the AI Assistant workstation under `source/applications/gui/ai_assistant`.
- Added the `gui-ai-assistant` CMake configure/build preset.
- Routed the AI Assistant menu action through `QProcess` instead of constructing its window inside the main GUI.
- Repaired concatenated include lines introduced by the local move.
- Removed accidentally versioned `.qtc_clangd/` files from the moved GUI tree.
- Added a `.gitignore` rule under the moved GUI tree for `.qtc_clangd/`.
- Added temporary `MovedGuiCompatibilityIncludes.h` support to keep the moved GUI buildable while direct includes are cleaned incrementally.
- Scoped the temporary forced include to GUI test targets in unit-test CMake, not globally.
- Confirmed that the focused GUI GMDD target can compile.
- Reached CTest in a full `tests-unit` validation run, meaning the main blocker is currently test behavior, not compilation.

### Current known problems in the active branch

#### Temporary diagnostic workflow

`.github/workflows/genesys-ci.yml` is currently a diagnostic workflow focused on the failing GUI GMDD tests. It is not the normal full `tests-unit` CI workflow.

Before merge readiness, restore the normal CI workflow and remove diagnostic-only artifact logic.

#### GUI GMDD tests still failing

The focused workflow currently exercises these failing tests:

- `GuiGmddLayout.SeizeEditableReferencesStayAboveAndLowerDefinitionsUseTwoRows`.
- `GuiGmddLayout.SerializerRoundTripRestoresComponentColorAndDataDefinitionPosition`.
- `GuiGmddLayout.SerializerRoundTripRestoresViewStateGeometriesAndGroups`.

The latest focused run after commit `f4e1b932e41df7cc0f4c0fda733376cfb667975e` still failed all three tests and uploaded a diagnostic artifact.

#### Layout attempts did not resolve the first GUI GMDD failure

Two commits changed `source/applications/gui/genesys/services/GraphicalDataDefinitionLayout.cpp`:

- `fix: separate lower GMDD radial layers`.
- `fix: split dense lower GMDD arcs into rows`.

These were attempts to separate lower GMDD rows vertically. They did not solve the failing test. A future agent should not keep increasing spacing blindly.

Likely next diagnostic point: `GraphicalModelBuilder.cpp`, specifically how these lists are classified and ordered before layout:

- `upperLinks`;
- `lowerStatisticsLinks`;
- `lowerSharedLinks`;
- child/shared links collected from component and data-definition references.

Observed failure pattern: shared lower GMDDs remain above the expected statistics row bottom. The test expects shared lower definitions to be below the statistics row.

#### Serializer color round-trip failures

Two serializer tests still fail because loaded component color becomes `#828282`, while tests expect different values:

- category color `#008000` in `SerializerRoundTripRestoresComponentColorAndDataDefinitionPosition`;
- saved visual color `#232b57` in `SerializerRoundTripRestoresViewStateGeometriesAndGroups`.

Do not resolve this by simply changing both expectations to `#828282`.

The intended behavior appears to differ by context:

- ordinary model rebuild should use the plugin category palette;
- full visual round-trip with explicit saved visual state should preserve the saved component color.

Likely code area: `GraphicalModelSerializer.cpp`, around persisted component state restoration after `_generateGraphicalModelFromModel`.

#### Full CTest has non-GUI failures too

A prior full CTest run reported 16 failures out of 1657 tests:

- 3 GUI GMDD failures;
- 13 non-GUI runtime/whole-cell/metabolic failures.

Treat the 13 non-GUI failures as outside this GUI separation scope unless evidence proves the GUI move caused them.

### Recommended continuation plan for another AI

1. Reassess whether the two unsuccessful `GraphicalDataDefinitionLayout.cpp` commits should be reverted or replaced.
2. Inspect `GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer` and trace the failing test's Queue/Resource/Counter/StatisticsCollector/shared Queue items into the lower layout lists.
3. Fix the GMDD lower-row classification or ordering at the builder level if that is the real cause.
4. Inspect `GraphicalModelSerializer.cpp` to distinguish category color restoration from persisted visual color restoration.
5. Rerun only the focused GUI GMDD test target before running the full suite.
6. Restore the normal `.github/workflows/genesys-ci.yml` after focused GUI diagnostics are no longer needed.
7. Run full `tests-unit` validation and classify remaining non-GUI failures explicitly as in-scope or out-of-scope.
8. Validate Debian packaging only after workflow and source state are no longer diagnostic-only.

### Validation order

Recommended order:

1. `cmake --preset tests-unit`.
2. `cmake --build --preset tests-unit --parallel $(nproc)`.
3. Run `genesys_test_gui_gmdd_layout` with the three-test filter.
4. `ctest --preset tests-unit --output-on-failure`.
5. Check Debian package workflow after normal CI is restored.

### Do not do now

- Do not mark PR #451 ready for review.
- Do not merge PR #451 into `WiP20261` while diagnostic workflow changes remain.
- Do not start the next GUI application extraction.
- Do not fix whole-cell/metabolic tests in this PR unless causality with the GUI move is demonstrated.
- Do not treat the two layout spacing commits as proven correct; they are unvalidated attempts and may need replacement.

### Acceptable stopping states for phase 1

Preferred stopping state:

- main GUI moved and compiles;
- GUI GMDD tests pass;
- normal CI restored;
- Debian workflow passes;
- PR #451 is ready for review.

Acceptable WIP stopping state:

- main GUI moved and compiles;
- GUI GMDD blockers documented;
- temporary workflow restored or clearly documented;
- PR #451 remains draft.

Current state at this handoff:

- main GUI moved;
- build blockers mostly cleared;
- focused GUI GMDD tests still fail;
- diagnostic workflow still active;
- PR #451 must remain draft.
