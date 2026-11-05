# Standalone Tools GUI: process, build and distribution contract

Task: AUTO-GUI-002 / issue #561. Initial base: `WorkInProgress` SHA `987e145a36948cdf349840ab3350f576adfbd1fd`. This document describes inspected source contracts and proposed validation; the PR is **draft** until CI artifacts are reviewed.

## Application and UI ownership

| Tool | Main/target | Window and source | Consumer / ownership |
| --- | --- | --- | --- |
| Data Analyzer | `source/applications/gui/dataanalyser/main.cpp` / `genesys_dataanalyser_gui_application` | `genesys/tools/dataanalyzer/DataAnalyzerWindow.{h,cpp}` | Standalone Data Analyser target only; owns `HistogramPreview` and `DoePreviewPlot` nested widgets and local dialogs |
| Optimization | `source/applications/gui/optimizer/main.cpp` / `genesys_optimizer_gui_application` | `genesys/tools/optimizer/OptimizerWindow.{h,cpp}` | Standalone Optimizer target only; owns `ProgressPlot` nested widget and local dialogs |
| AI Assistant | `source/applications/gui/ai_assistant/main.cpp` / `source/applications/gui/ai_assistant/AIAssistantWindow.{h,cpp}` | `ai_assistant/AIAssistantWindow.{h,cpp}` | Standalone AI Assistant target only |

Each `main()` creates its own `QApplication`, local `Simulator`, `QMainWindow`, and event loop. The main editor target excludes the two historical window `.cpp` units explicitly; the AI Assistant window is not below the main editor source tree. The main GUI's `Tools` actions delegate through `mainwindow_controller.cpp` and `DialogUtilityController.cpp` to `ToolLauncher::launchDetached()`; they do not construct these windows.

**Physical-source migration:** `DataAnalyzerWindow.{h,cpp}` and `OptimizerWindow.{h,cpp}` remain under `genesys/tools/` but are application-specific, not shared GUI runtime code. Migration into their respective sibling application directories is desirable but deferred. Before moving, update each standalone `CMakeLists.txt`, all include paths and the main-GUI source-glob exclusions, then build all three presets and the main editor, test ownership/Qt widgets, and compare UI behavior. Do not delete these classes; they are the active standalone windows.

## Paths and package layouts

| Environment | Main editor executable | Tool executables | Discovery strategy |
| --- | --- | --- | --- |
| Development `gui-app` preset | `build/gui-app/source/applications/gui/genesys/genesys-gui` | Each independent preset has its own build tree | Explicit sibling/preset-relative fallback paths in the handlers; no guarantee that a `gui-app` build alone provides all tools |
| CMake install (`GNUInstallDirs` defaults) | `<prefix>/libexec/genesys/system/bin/genesys-gui` | `<prefix>/bin/genesys-{dataanalyser,optimizer,ai-assistant}-gui` | Same directory, then install-prefix `bin` from the documented main-GUI suffix; no `PATH` execution |
| Debian (current split-package baseline) | `/usr/libexec/genesys/system/bin/genesys-gui`, via public Launcher wrapper | `/usr/bin/genesys-{dataanalyser,optimizer,ai-assistant}-gui` in the `genesys-gui` package | Install-prefix `bin` lookup; no package layout change for the main app |
| Portable Linux | `<prefix>/libexec/genesys/system/bin/genesys-gui` | Same runtime `system/bin` directory, with public wrappers in `<prefix>/bin` | Colocated executable wins; inherited Launcher/portable environment is preserved |

On Windows, a same-directory `.exe` candidate remains supported; Windows packaging is not independently certified by this task. The launcher rejects executable names containing a directory, shell syntax, or unrelated command names, and does not consult `PATH` or the current working directory. An absent or non-executable target generates a GUI diagnostic with searched locations. A successful `QProcess::startDetached()` only means process creation was initiated; use PID and window checks for executable evidence.

The existing CMake presets `gui-dataanalyser`, `gui-optimizer`, and `gui-ai-assistant` intentionally build isolated GUIs. The `gui-app` preset intentionally builds only the main GUI. The Debian package build enables all three optional tools; the portable builder already enables and stages all three. Draft PR #524 proposes a different (unified) Debian packaging contract; reconcile that PR's exact source before merging this change.

## Model and scientific boundary

The standalone applications have their own `Simulator` and therefore do not receive the main GUI's `Model` pointer. The current Tools menu does not transfer model data, and this task does not introduce IPC or serialization. Data analysis, optimization, and AI provider workflows are not promoted to scientific validity by these launcher fixes.

## Validation ledger and follow-up

- Source-level guard: `python3 scripts/validate-standalone-tools.py` verifies three mains/targets, menu delegates, filenames, GUI-source exclusions and distribution manifests.
- C++ focused tests: `genesys_tool_launcher_tests` use an isolated Qt6/GTest build and exercise portable, Debian-prefix and development lookup, missing executable, permission and path-traversal cases.
- Integration workflow: `.github/workflows/genesys-standalone-tools-integration.yml` compiles four GUIs, installs into a disposable prefix containing spaces, runs CTest, starts the main GUI from arbitrary cwd under private Xvfb, invokes three Tools menu actions, checks PID/`/proc/<pid>/exe`, visible windows, main GUI liveness and child termination.
- Existing individual GUIs already have dedicated bounded startup workflows; they are not substitutes for the combined menu/installation test.
- Package lifecycle, all CI statuses and manual PDF regeneration remain `UNKNOWN` until reviewed at the final PR head.

Follow-up: move the two window source pairs only after a separate, focused migration demonstrates equivalent CMake, Qt automoc, UI and runtime behavior. No broad legacy source deletion is warranted by this audit.
