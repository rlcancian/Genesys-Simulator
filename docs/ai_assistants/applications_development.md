# Applications Development Guidance

## Purpose

This document is the stable AI-assistant reference for GenESyS application-layer work.

Historical notes from `oldies/` must be checked against current source files before being treated as current facts.

## Scope

Primary source areas:

- `source/applications/terminal/`
- `source/applications/web/`
- `source/applications/gui/`
- future application folders such as HTTP worker, MCP, or GUI variants when introduced

## General policy

Application-level changes should remain isolated from the kernel unless a kernel API limitation is proven.

Before changing kernel code for an application need, document the requirement, the existing API limitation, affected callers, and required tests.

## Terminal application

Guidance:

- Prefer preset-based terminal validation.
- Keep terminal example selection explicit and reproducible.
- Avoid compiling every example by default unless build-time impact is acceptable.
- Keep terminal changes separate from GUI and web assumptions.

## Web application

Guidance:

- Treat the current web application as an existing application layer, not as a pending skeleton.
- Keep HTTP/API behavior decoupled from GUI behavior.
- Prefer service-layer changes over direct route-to-kernel coupling.
- Validate API changes with minimal integration or smoke tests when possible.

Known follow-up candidates:

- confirm whether plugin autoload remains part of the target API contract;
- evaluate pause, resume, and stop endpoints if simulation control requires them;
- review JSON parsing, payload-size, timeout, and error-handling behavior before public deployment.

## HTTP worker application direction

The current `source/applications/web/` tree is planned to become semantically clearer as an HTTP/background worker application area.

Planned direction:

- prefer the name `httpworker` for the application concept when folder and target migrations begin;
- keep the HTTP/API runtime decoupled from any graphical frontend;
- treat the graphical Web Worker control window as a GUI application frontend, not as part of `source/tools/`;
- preserve compatibility aliases for existing `web` CMake targets and installed binary names until CI, packaging, documentation, and user workflows have been updated;
- avoid changing HTTP/API behavior during pure folder-renaming commits.

## GUI application

Guidance:

- Keep GUI-specific concerns out of kernel code.
- Prefer controllers and services over direct expansion of MainWindow responsibilities.
- Preserve Qt ownership rules when modifying widgets, dialogs, and controllers.
- Validate GUI changes with the `gui-app` preset in an environment with Qt6 dependencies.
- Keep generic GUI extension infrastructure separate from domain-specific features.

## GUI applications restructuring direction

The planned GUI restructuring separates the main GenESyS editor from graphical frontends for application/tool workflows.

Target conceptual layout:

```text
source/applications/gui/
  genesys/        # main GenESyS Qt GUI
  httpworker/     # graphical control frontend for the HTTP/background worker
  dataanalyser/   # graphical frontend for statistics/data-analysis workflows
  optimizer/      # graphical frontend for optimization workflows
  ai_assistant/   # graphical frontend for AI-assisted modeling workflows
  doexperiments/  # future graphical frontend for DOE/factorial-design workflows
```

Migration policy:

- move the main GUI first only after CMake can scope the GUI source list safely;
- do not place sibling GUI applications below a directory still scanned by broad recursive source collection;
- preserve the existing `genesys_gui` target and `genesys-gui` installed binary name during the initial move;
- use separate executable targets for independent GUI frontends;
- launch independent GUI frontends from the main GUI through a shared process-launching service rather than direct widget construction;
- keep model/context handoff explicit and testable instead of sharing raw pointers across application boundaries;
- avoid changing kernel APIs unless the application requirement and API limitation are documented first.

## GUI-hosted frontend inventory

- Date: 2026-07-03
- Branch: `WiP20261`
- Status: initial inventory; revalidate before moving files.

Confirmed current entry points:

- `MainWindow` exposes tool/action slots for parser grammar checking, Web Worker, experimentation, optimizer, expression builder, AI assistant, and data analyzer.
- `DialogUtilityController` directly includes and constructs `AIAssistantWindow`, `DataAnalyzerWindow`, `OptimizerWindow`, and `ExpressionBuilder`.
- `MainWindow` directly owns a modeless `WebWorkerDialog` through `QPointer<WebWorkerDialog>`.
- The current GUI CMake file gathers all `.cpp` files under the GUI directory using broad recursive source collection. This is a migration blocker for adding sibling GUI applications inside the scanned tree.

Initial classification:

| Current artifact | Current location | Current type | Planned classification | Extraction priority |
| --- | --- | --- | --- | --- |
| Main GenESyS GUI | `source/applications/gui/qt/GenesysQtGUI/` | `QMainWindow` application | `source/applications/gui/genesys/` | first structural move after CMake source scoping is safe |
| Web Worker control window | `source/applications/gui/qt/GenesysQtGUI/dialogs/WebWorkerDialog.*` | `QDialog` owning its own `WebWorkerRuntime` | `source/applications/gui/httpworker/` | after HTTP worker application/core naming is stabilized |
| Data Analyzer | `source/applications/gui/qt/GenesysQtGUI/tools/dataanalyzer/` | `QMainWindow` with file/dataset and optional simulator snapshot workflows | `source/applications/gui/dataanalyser/` | preferred first tool frontend extraction |
| Optimizer | `source/applications/gui/qt/GenesysQtGUI/tools/optimizer/` | `QMainWindow` tied to current simulator/model controls and responses | `source/applications/gui/optimizer/` | after model/context handoff and backend lifetime review |
| AI Assistant | `source/applications/gui/qt/GenesysQtGUI/tools/aiassistant/` | `QMainWindow` using AI assistant backend, simulator facade, secrets, and provider configuration | `source/applications/gui/ai_assistant/` | after secret/configuration/model-context review |
| Expression Builder | `source/applications/gui/qt/GenesysQtGUI/tools/expressionbuilder/` | `QDialog` utility | not in the first standalone application wave | defer; likely remains utility dialog or becomes a small helper app later |
| Experimentation / Do Experiments | slot exists as legacy placeholder; no current exposed action confirmed in this inventory | placeholder | future `source/applications/gui/doexperiments/` frontend over `source/tools/FactorialDesign/` | after workflow specification |

Immediate migration implication:

- Do not create new `main.cpp` files below the current `GenesysQtGUI` tree while broad recursive source collection is active.
- The next safe source patch should introduce an umbrella `source/applications/gui/CMakeLists.txt` without changing behavior, or first constrain the existing GUI source collection before adding sibling GUI applications.
- A future process-launching service should replace direct widget construction in `MainWindow`/`DialogUtilityController` only after each target frontend executable exists and has a minimal startup validation path.

## Folder restructuring

When reorganizing application folders, preserve Linux build stability first.

Suggested migration rules:

- move files in small commits;
- update CMake immediately with each move;
- avoid renaming executable targets casually because packaging and CI may depend on them;
- test terminal, web, and GUI presets independently after structural moves.

## Validation checklist

For application changes, prefer this order:

1. Run unit-test validation.
2. Build the affected application preset.
3. Run a minimal startup or smoke validation for the affected executable.
4. For web changes, validate health/status behavior.
5. For GUI changes, validate startup and a minimal model interaction when a display environment is available.
6. For packaging-related changes, validate executable names and install paths separately.

## Open follow-up tasks

- Revalidate the current application tree and CMake target names.
- Consolidate the proposed application-folder restructuring into a separate migration plan.
- Add lightweight web integration tests if the current test structure supports them.
- Define packaging-facing executable and man-page expectations for Debian/PPA.
- Define the compatibility policy for the planned `web` to `httpworker` transition.
- Define the process-launching contract used by the main GUI to start sibling GUI applications.
- Revalidate the GUI-hosted frontend inventory before each extraction PR.
