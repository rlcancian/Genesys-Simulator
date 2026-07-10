# Kernel Development Guidance

## Purpose

This document is the stable AI-assistant reference for GenESyS kernel modernization, ownership, and lifetime review.

It consolidates historical notes from `docs/ai_assistants/oldies/`. Historical findings must be revalidated against current source files before they are treated as current facts.

## Scope

Primary source tree:

- `source/kernel/`
- especially `source/kernel/simulator/`

Historical source documents:

- `old_kernel-cpp23-modernization-audit.md`
- `old_kernel-memory-leak-review-2026-03-30.md`
- `old_modelsimulation-cpp23-review.md`
- `old_phase2-kernel-simulator-inventory.md`

## Current direction

The repository CMake preset baseline uses C++23. Kernel modernization should therefore be incremental, local, behavior-preserving, and validated.

Prefer small changes such as:

- `nullptr` for null pointer literals.
- `override` for virtual overrides.
- `static_cast` instead of C-style casts.
- range-based loops where iterator identity is not required.
- RAII for new ownership code.
- `std::unique_ptr` when ownership is exclusive and call sites are mapped.

Avoid broad style rewrites without a concrete correctness or maintenance benefit.

## Ownership policy

When modifying kernel classes:

1. Identify who owns each object before changing signatures.
2. Separate observing raw pointers from owning raw pointers.
3. Preserve destruction order unless tests prove it is safe to change.
4. Avoid replacing containers of raw pointers until all call sites are mapped.
5. Prefer narrow fixes with tests over broad ownership migration.

## High-attention classes from historical reviews

### `Model`

Historical reviews treated `Model` as a major ownership hub. Later notes indicate explicit teardown exists, but raw-pointer ownership remains a residual risk.

Guidance: do not refactor `Model` ownership broadly without mapping managers, events, parser, persistence, model data, and components.

### `ModelSimulation`

Historical notes recorded a move toward explicit ownership for simulation events and identified remaining manual ownership areas around controls, counters, breakpoints, and reporter ownership.

Guidance: preserve reporter ownership semantics and validate changes around controls, breakpoints, and event notification.

### `SimulationScenario`

Historical notes indicate that explicit cleanup mitigated earlier concerns, but internal data still relies on manual ownership patterns.

Guidance: treat copying, replacement, and destruction semantics as coupled.

### `TraceManager`

Historical notes flagged this class as lifetime-sensitive.

Guidance: inspect constructor, destructor, and handler/list ownership before editing.

### `PluginManager`

Historical notes flagged plugin ownership and registration behavior as important.

Guidance: avoid plugin-manager changes without checking plugin loading, registration, and connector ownership paths.

## Validation checklist

For kernel changes, prefer this order:

1. Configure the unit-test preset.
2. Build the unit-test preset.
3. Run CTest with the unit-test preset.
4. Repeat with the kernel-focused preset when the change is kernel-specific.
5. For ownership changes, consider additional runtime diagnostic builds if supported by current presets.

## AI-assistant review rules

- Cite exact files and methods inspected.
- Separate current facts from historical notes.
- Do not claim a lifetime problem exists unless ownership and cleanup paths were checked.
- Do not claim a lifetime problem is fixed unless the relevant teardown path was checked.
- Prefer one small commit per ownership or modernization concern.
- Always report which build/test validation was performed.

## Open follow-up tasks

- Revalidate the current state of `Model`, `ModelSimulation`, `SimulationScenario`, `TraceManager`, and `PluginManager` directly against source code.
- Add or restore dedicated diagnostic build presets only after local validation.
- Expand kernel unit tests around lifecycle-heavy classes before broad ownership refactors.
- Build a current ownership map for `source/kernel/simulator` and retire outdated historical notes after review.
