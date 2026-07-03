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

## GUI application

Guidance:

- Keep GUI-specific concerns out of kernel code.
- Prefer controllers and services over direct expansion of MainWindow responsibilities.
- Preserve Qt ownership rules when modifying widgets, dialogs, and controllers.
- Validate GUI changes with the `gui-app` preset in an environment with Qt6 dependencies.
- Keep generic GUI extension infrastructure separate from domain-specific features.

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
