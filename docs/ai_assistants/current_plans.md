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

## GUI applications refactoring plan

- Date: 2026-07-03
- Branch: `WiP20261`
- Scope: progressively decouple graphical applications currently hosted by the main GenESyS Qt GUI.
- Status: CMake umbrella scaffold, per-GUI build options, and logical `gui/genesys` routing added; user reported successful compilation after the CMake routing changes.

### Architectural intent

The application layer should evolve toward a clearer split between executable applications and reusable backend libraries:

- `source/tools/` remains the backend/tooling area for statistics, optimization, AI assistant services, DOE/factorial design, continuous solvers, and related reusable logic.
- `source/applications/terminal/` remains the terminal application area.
- `source/applications/httpworker/` is the planned semantic replacement for the current `source/applications/web/` application tree, representing the HTTP/background worker application.
- `source/applications/gui/` becomes the umbrella for graphical applications, including the main GenESyS GUI and independent graphical frontends for selected tools or applications.

### Target application layout

The planned target layout is:

```text
source/
  applications/
    terminal/
    httpworker/          # planned rename of the current web application tree
    mcp/                 # future/independent agent-facing application area when introduced
    gui/
      CMakeLists.txt     # umbrella only; delegates to each GUI application
      genesys/           # main GenESyS Qt GUI, replacing the current qt/GenesysQtGUI location
      httpworker/        # graphical control frontend for the HTTP/background worker
      dataanalyser/      # graphical frontend for data-analysis/statistics workflows
      optimizer/         # graphical frontend for optimization workflows
      ai_assistant/      # graphical frontend for AI-assisted modeling workflows
      doexperiments/     # future graphical frontend for DOE/factorial-design workflows
```

Names above are architectural targets. Existing executable target names and installed binary names should be preserved or transitioned with compatibility aliases until CMake, CI, packaging, and documentation have been updated.

### Current GUI build switches

The GUI umbrella now owns per-GUI-application build options:

- `GENESYS_BUILD_GUI_GENESYS`, default `ON`, currently builds the existing main GUI through `source/applications/gui/genesys`, which delegates to the historical `qt/GenesysQtGUI` implementation directory.
- `GENESYS_BUILD_GUI_HTTPWORKER`, default `OFF`, reserved for the future HTTP worker control GUI.
- `GENESYS_BUILD_GUI_DATAANALYSER`, default `OFF`, reserved for the future Data Analyser GUI application.
- `GENESYS_BUILD_GUI_OPTIMIZER`, default `OFF`, reserved for the future Optimizer GUI application.
- `GENESYS_BUILD_GUI_AI_ASSISTANT`, default `OFF`, reserved for the future AI Assistant GUI application.
- `GENESYS_BUILD_GUI_DOEXPERIMENTS`, default `OFF`, reserved for the future Do Experiments GUI application.

Reserved options currently fail configuration intentionally if enabled before their source directories exist. This prevents silent success for not-yet-implemented GUI applications.

### Migration sequence

1. Add documentation and CMake scaffolding without moving implementation files. Status: completed for the first GUI umbrella scaffold.
2. Introduce `source/applications/gui/CMakeLists.txt` as the umbrella for GUI applications. Status: completed.
3. Introduce `source/applications/gui/genesys/CMakeLists.txt` as the logical stable entrypoint for the main GUI. Status: completed; it delegates temporarily to `../qt/GenesysQtGUI`.
4. Move the main GUI implementation from the current Qt-specific subdirectory to `source/applications/gui/genesys/`, preserving the `genesys_gui` build target and `genesys-gui` installed binary name.
5. Keep GUI source collection scoped to each application directory. The GUI umbrella must not recursively collect all GUI `.cpp` files.
6. Rename or mirror `source/applications/web/` as `source/applications/httpworker/`, preserving temporary compatibility aliases for existing web targets and binary names.
7. Extract the Web Worker control window into `source/applications/gui/httpworker/` and link it to the HTTP worker service/core library.
8. Change the main GUI from hosting the Web Worker control dialog directly to launching the graphical HTTP worker frontend as a separate process.
9. Extract Data Analyser as a standalone-leaning graphical application that can run from files/datasets before deeper live-model integration is attempted.
10. Extract Optimizer as a graphical application only after model/context handoff and backend pointer/lifetime assumptions have been reviewed.
11. Extract AI Assistant as a graphical application after provider configuration, audit logging, and model-context handoff are reviewed.
12. Add the future Do Experiments graphical application on top of the FactorialDesign backend when its workflow is specified.

### Initial inventory result

The initial GUI-hosted frontend inventory is recorded in `docs/ai_assistants/applications_development.md`.

Current inventory conclusions:

- The main GUI still exposes direct action slots for Web Worker, Optimizer, AI Assistant, and Data Analyzer.
- `DialogUtilityController` still directly constructs several tool frontends.
- `WebWorkerDialog` is already more independent than before because it owns its own runtime, but it is still directly hosted by `MainWindow`.
- Data Analyser, Optimizer, and AI Assistant are already `QMainWindow`-based frontends and are better candidates for eventual standalone GUI applications than small modal dialogs.
- Expression Builder is out of scope for this GUI-application refactoring. It should not be treated as a tool or application frontend and should be removed or replaced in a dedicated GUI cleanup/property-editor integration change.
- The GUI umbrella CMake must only delegate to each GUI application CMake file. It must not collect sources recursively across GUI application siblings.

### Current constraints and validation gates

- Application-layer restructuring should not require kernel changes unless an explicit kernel API limitation is documented.
- Folder moves should be done in small commits, updating CMake in the same commit that moves files.
- Existing presets and install names should be treated as compatibility surfaces until packaging and CI are updated.
- Each structural step must validate unit tests first, then the affected application preset, then smoke/startup behavior when the environment supports it.
- GUI validation requires an environment with Qt dependencies; headless checks should use `QT_QPA_PLATFORM=offscreen` only for startup/smoke scenarios that are compatible with it.

### Open follow-up tasks

- Validate the full test preset after GUI CMake routing changes.
- Decide the compatibility period for `web`/`httpworker` CMake target aliases and installed binary names.
- Define a process-launch service in the main GUI for launching sibling GUI applications consistently.
- Define model/context handoff mechanisms for Data Analyser, Optimizer, AI Assistant, and future Do Experiments.
- Update Debian/PPA packaging expectations after executable names and install paths are stable.
- Prepare the physical move of main GUI implementation files from `qt/GenesysQtGUI` to `gui/genesys`.
