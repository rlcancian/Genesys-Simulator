# Applications Development Guidance

## Purpose

This document is the stable AI-assistant reference for GenESyS application-layer work.

Historical notes from `oldies/` must be checked against current source files before being treated as current facts.

## Scope

Primary source areas:

- `source/applications/shell/`
- `source/applications/worker/`
- `source/applications/gui/`
- `source/applications/modelSpecific/`
- future application folders such as MCP or additional GUI variants when introduced

## General policy

Application-level changes should remain isolated from the kernel unless a kernel API limitation is proven.

Before changing kernel code for an application need, document the requirement, the existing API limitation, affected callers, and required tests.

## Qt baseline policy

Decision date: 2026-07-20.

GenESyS supports Qt6 only. Qt5 compatibility is no longer part of the intended platform contract.

Status:

- Qt6-only policy: `decided`;
- removal of remaining Qt5 fallback code and documentation: `needs-implementation`.

Implementation implications:

- GUI targets should use `find_package(Qt6 REQUIRED ...)` without Qt5 fallback;
- active Qt5 compatibility branches, build-script references, tests, README instructions, and packaging dependencies should be removed in a bounded change;
- retained historical documents under `oldies/` may continue to mention Qt5 when clearly historical;
- all GUI targets and GUI tests must be validated after the removal;
- no assistant should preserve a Qt5 fallback merely because it still exists in current CMake/source files.

The acceptance criteria and currently known Qt5 reference areas are recorded in `genesys_2026_human_decisions.md`.

## Terminal application

Guidance:

- Prefer preset-based terminal/shell validation.
- Keep terminal example selection explicit and reproducible.
- Avoid compiling every example by default unless build-time impact is acceptable.
- Keep terminal changes separate from GUI and worker assumptions.

## Worker HTTP/API application

Guidance:

- Treat the current worker as an existing application layer, not as a pending skeleton.
- Keep HTTP/API behavior decoupled from GUI behavior.
- Prefer service-layer changes over direct route-to-kernel coupling.
- Validate API changes with minimal integration or smoke tests when possible.

Known follow-up candidates:

- confirm whether plugin autoload remains part of the target API contract;
- evaluate pause, resume, and stop endpoints if simulation control requires them;
- review JSON parsing, payload-size, timeout, authentication, and error-handling behavior before intranet deployment.

## HTTP worker application direction

The current `source/applications/worker/` tree is the HTTP/background worker application area.

Direction:

- keep the HTTP/API runtime decoupled from any graphical frontend;
- treat the graphical Web Worker control window as a GUI application frontend, not as part of `source/tools/`;
- preserve compatibility aliases for existing `web` CMake options/target names only until CI, packaging, documentation, and user workflows have been updated;
- avoid changing HTTP/API behavior during pure folder/target-renaming commits.

## Worker deployment and exposure policy

The intended primary deployment profile is a controlled academic intranet: a worker in a computer laboratory accepts parallel simulation requests from authorized computers on the private institutional/laboratory network.

Public Internet exposure is not an approved default.

Supported planning profiles are:

1. local-only loopback execution;
2. controlled academic intranet;
3. hardened Internet-facing service;
4. outbound/pull worker connected to a coordinator.

For the controlled-intranet profile, the minimum security direction is:

- explicit private-interface binding and firewall allowlists;
- TLS for credentials and simulation data;
- unique, expiring, rotatable, revocable client/user credentials;
- preferably mutual TLS for managed laboratory machines, or short-lived signed tokens over TLS;
- OS-backed cryptographically secure randomness;
- request/job quotas, timeouts, payload limits, and concurrent-job limits;
- no arbitrary shell command or unrestricted user code execution through the worker API;
- dedicated service identity, restricted filesystem access, controlled temporary directories, and CPU/RAM/time limits;
- authenticated job ownership, audit logging, request IDs, result provenance, and API versioning;
- deny-by-default behavior when authentication or required security configuration is missing;
- secrets absent from command-line arguments, logs, models, and client-visible configuration.

Status:

- intended controlled-intranet profile: recorded;
- public Internet exposure by default: not approved;
- security hardening for intranet deployment: `needs-implementation`;
- exact authentication choice: `needs-human-decision`.

See `genesys_2026_human_decisions.md` for the tradeoffs among static bearer tokens, short-lived signed tokens, mutual TLS, institutional identity, and outbound/pull deployment.

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
  CMakeLists.txt  # umbrella only; does not collect application sources
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

## Per-GUI-application CMake policy

Every GUI application must have its own `CMakeLists.txt` and target ownership.

The `source/applications/gui/CMakeLists.txt` file is an umbrella only. It may call `add_subdirectory(...)` for GUI application subdirectories, but it must not recursively collect `.cpp` files from all GUI applications into one executable.

Allowed pattern:

```cmake
add_subdirectory(genesys)

if(GENESYS_BUILD_GUI_HTTPWORKER)
    add_subdirectory(httpworker)
endif()
```

Disallowed pattern at the umbrella level:

```cmake
file(GLOB_RECURSE ALL_GUI_SOURCES CONFIGURE_DEPENDS "*.cpp")
add_executable(some_gui ${ALL_GUI_SOURCES})
```

Each GUI application may decide whether to use explicit source lists or a scoped source collection inside its own directory. If a scoped source collection is used, its scope must be limited to that single application directory and must not include sibling applications.

## GUI-hosted frontend inventory

- Date: 2026-07-03
- Branch: `WiP20261`
- Status: historical initial inventory; revalidate against the current branch before using paths or implementation status.

The original inventory recorded direct hosting of Web Worker, Data Analyzer, Optimizer, and AI Assistant windows in the main GUI. Current branch structure and CMake targets must take precedence over that historical inventory.

Current conceptual classifications remain:

| Artifact | Planned/current application classification | Consolidation concern |
|---|---|---|
| Main GenESyS GUI | `source/applications/gui/genesys/` | Qt6 build, GUI tests, packaging |
| Web Worker control frontend | `source/applications/gui/httpworker/` | secure process/API handoff |
| Data Analyser | `source/applications/gui/dataanalyser/` | backend/frontend separation |
| Optimizer | `source/applications/gui/optimizer/` | model context and incomplete backend |
| AI Assistant | `source/applications/gui/ai_assistant/` | secret/configuration/model context |
| Do Experiments | future `source/applications/gui/doexperiments/` | workflow not yet specified |

## Folder restructuring

When reorganizing application folders, preserve Linux build stability first.

Suggested migration rules:

- move files in small commits;
- update CMake immediately with each move;
- avoid renaming executable targets casually because packaging and CI may depend on them;
- test shell, worker, and GUI presets independently after structural moves.

## Validation checklist

For application changes, prefer this order:

1. Run unit-test validation.
2. Build the affected application preset.
3. Run a minimal startup or smoke validation for the affected executable.
4. For worker changes, validate health/status, authentication, authorization, payload limits, and failure behavior.
5. For GUI changes, validate startup and a minimal model interaction when a display environment is available.
6. For packaging-related changes, validate executable names and install paths separately.
7. For Qt compatibility removal, search active source/build/docs for Qt5 and validate every Qt6 GUI target.
8. For network-facing changes, validate firewall/bind assumptions and avoid public exposure during tests.

## Open follow-up tasks

- Remove active Qt5 fallback code and validate the Qt6-only build matrix.
- Revalidate the current application tree and CMake target names.
- Add lightweight worker integration/security tests if the current test structure supports them.
- Define packaging-facing executable and man-page expectations for Debian/PPA.
- Complete the compatibility cleanup for historical `web` names after CI and packaging are updated.
- Define the process-launching contract used by the main GUI to start sibling GUI applications.
- Revalidate GUI-hosted frontend behavior before each extraction or cleanup PR.
- Remove or replace the current Expression Builder menu entry in a dedicated GUI cleanup change.
- Select the controlled-intranet worker authentication mechanism.
