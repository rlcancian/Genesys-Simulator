---
document_type: implementation-plan
authority: technical-reference
owner: project-maintainer
last_reviewed: 2026-11-11
status: planned-not-started
tracks: AUTO-GUI-003
---

# Main GUI Graphical Extensions — Implementation Plan

## 1. Objective, authorization and status

Implement the [approved graphical extension architecture](GUI_EXTENSIONS_ARCHITECTURE.md) incrementally, **without** bringing optional simulation-specific animation, renderer, dialog or menu implementation into generic GUI host code. Preserve the simulation kernel/parser/plugin independence from all Qt6 applications.

Decision: `HUM-ARCH-004` in [BACKLOG_HUMAN](../BACKLOG_HUMAN.md), 2026-11-11; execution tracker: `AUTO-GUI-003` in [BACKLOG_AUTONOMOUS](../BACKLOG_AUTONOMOUS.md), initially **blocked-dependency / not ready** pending an explicit bounded implementation activation and current-code revalidation. Documentation approval and merge do not authorize unattended implementation. Track each implemented slice with a separate bounded branch/PR after activation, not one giant code rewrite.

**Supported baseline**: Ubuntu 24.04, GCC/toolchain-supported C++23, CMake + Ninja, Qt6, Google Test and existing test/GUI presets (`tests-unit`, `tests-kernel-unit`, `tests-smoke`, `gui-app`), per [README](../../../README.md), [governance](../GOVERNANCE.md), [current status](../STATUS.md) and [GitHub runbook](../runbooks/GITHUB_AGENT.md). Do not claim local build or UI tests from a remote-only agent.

## 2. Scope and explicit non-goals

In scope:
- evolve `GuiExtensionPlugin`/`GuiExtensionRegistry` and dependency metadata without a parallel plugin framework;
- remove *generic host* coupling to concrete model plugins for animation tools/renderers and keep optional concrete code in application-local graphical extensions;
- register scene animation tools/factories, component renderers and alternative GUI service providers;
- reuse existing GUI event/controller contracts to provide thread-safe state/event notifications to interested extensions;
- persist arbitrary extension-owned animation data and retain unknown/missing-extension records by default, with explicit user-selected discard when saving;
- introduce end-to-end exemplar(s): queue/resource/station migration and later BacteriaColony/CellularAutomata scene visualizations, GUI-only contribution, alternative report/Expression Builder provider;
- phase out obsolete Tools/Extensions viewers **after** equivalent scene-based functionality is validated; maintain user-visible compatibility during migration;
- maintain future dynamic-loading feasibility at the dependency/identifier/contract level while building statically.

Out of scope now:
- `QPluginLoader`/`dlopen`, unloading, hot reload and unload guards; versioned C ABI implementation, package/SDK distribution and separate shared-library CMake targets;
- rewriting kernel simulation semantics, parser, whole-cell models or CellularAutomata -> DefaultNetwork backend migration;
- moving all GUI directories, redesigning Property Editor, creating a parallel scene/network editor, refactoring plugin target overlap (#492) or rewriting the UI framework;
- wholesale adoption of PR #436 or student ZIPs; they are review-only reference material.

## 3. Preconditions and baseline audit (G0; first execution slice)

Prior to each change, re-read `/README.md` and `/docs/ai_assistants/README.md`, governance, architecture, status, both backlogs and this plan. Confirm current `WorkInProgress` HEAD; the source inventory in the architecture reference is an **2026-11-11 observation**, not guaranteed current indefinitely.

Inventory/traceability:
1. inspect `extensions/GuiExtensionContracts.h`, `GuiExtensionManager.*`, `GuiExtensionPluginCatalog.*` and concrete extensions, including registration and `requiredModelPlugins`;
2. locate all core GUI includes of concrete optional plugin headers (and hidden dependencies through helper services), hardcoded drawing modes/`switch`/`dynamic_cast`, event-driven special cases in `ModelGraphicsScene`/`SceneToolController`/`SimulationEventController`;
3. map animation creation, scene ownership, deletion/undo/group/resize, event callback lifetimes, saved `.gui` grammar, `AnimationPlaceholder`, serialization and load fallback;
4. inspect plugin metadata/dependencies in `PluginInformation`, current statically registered plugins, CMake compile-source graph (`file(GLOB_RECURSE)`) and GUI dialog/icon/UI resources;
5. identify at least one model fixture per animation family; record event ordering, model IDs, missing dependency scenarios and GUI lifecycle.

**G0 acceptance:** a source-path/line-backed coupling inventory, a before/after responsibility matrix, current format fixture list, ownership diagram, implementation entry points and a test plan; no broad code modifications. Reconcile other active GUI changes/PRs before starting.

## 4. Milestones and gates (each can be a bounded PR)

### G1 — Core extension registration + dependencies

- Preserve the C++ pure-abstract `GuiExtensionPlugin` and `registerContributions(GuiExtensionRegistry*)`; avoid mandatory virtual methods for each category.
- Extend registry with `addAnimation`, `addComponentRenderer`, `addServiceProvider` **only as needed by subsequent working slices**. Existing `addAction`/`addWindow`/`addDock` remain compatible.
- Design typed dependency declarations for simulation/model plugins, GUI extensions, and system capabilities. Existing `requiredModelPlugins()` is the initial compatibility route; validate required vs optional behavior; no installation commands.
- Detect duplicate contribution IDs/extension IDs, missing mandatory plugin(s) and ambiguous registration deterministically; GUI can launch with a disabled/absent optional extension.
- Do not expose `Ui::MainWindow` as a new external plugin contract; adopt a minimal capability/context abstraction incrementally without broad API churn.
- **Acceptance:** focused registry/metadata/disabled/duplicate tests; existing contributions unchanged; smoke GUI startup with and without optional registered extension; generic GUI code does not gain concrete plugin includes.

### G2 — Insertable animation tool + generic scene factory

- Make registered animations discoverable via menu `View → Animations` and GUI animation toolbar (adapt existing GUI placement rather than creating competing UI controls); handlers activate the correct tool.
- Generic scene invokes the registered animation factory for click/draw or drag/drop insertion; host manages geometry, selection, undo and existing item lifecycle; extension handles domain rendering.
- Remove per-plugin animation enums, hardcoded class branches and plugin-specific includes from host paths *as they are migrated*. Do not break existing graphics/clock/counter/variable/plot animations.
- Check whether `AnimationPlaceholder` supports custom extension-owned visual state; do not force it if contract cannot preserve independent factories.
- **Acceptance:** an independent *static exemplar* adds a new animation tool and visual item without modifying `MainWindow`, `ModelGraphicsScene`, generic controllers or kernel; focused drawing/drag-drop/undo/delete/resize tests; GUI build and no behavior regression for existing tools.

### G3 — Simulation notifications and graphical persistence

- Reuse `OnEventManager` through existing GUI `SimulationEventController` registrations. Bridge events/snapshots from GUI to active graphical instances rather than letting every item register transient kernel callbacks.
- Verify pre/post-event sequence, time, thread affinity, model switching and replication lifecycle, with special attention to aggregated/continuous state versus per-event visualization.
- Define a versioned generic graphical record with stable extension/type/item/target IDs, geometry and extension-specific opaque payload; preserve compatible legacy `.gui` records and allow type-specific restore.
- If a graphical extension is missing, retain the unknown record *as data* and optionally render a non-operational placeholder; on resave offer `Preserve` (default) or explicit `Discard`. Never discard unrecognized fields without user consent.
- **Acceptance:** save/reload of known animation; **load -> save while extension absent -> reload with extension available** preserves payload, identity, target and geometry; tests for unknown schema, duplicate names, missing model target, missing extension, cancel/discard and corrupt record; event order and GUI-thread update tests; no change to scientific state.

### G4 — Migrate existing domain animations (separate small PRs)

- Migrate Queue, Resource and Station creation/render/event handling from GUI host to corresponding concrete graphical extensions, preserving actual semantics (resource acquisition/release is not assumed from any `Seize`/`Release` event without a state oracle; station entering/leaving likewise).
- Preserve existing model/link and historical-current `.gui` fixture behavior. Once migrated, eliminate remaining host concrete plugin includes/cases/calls for that domain; stage each migration separately and keep a regression fixture.
- Prefer concrete extensions at `source/applications/gui/genesys/extensions/plugins/`, leaving framework interfaces/registry in `extensions/`. Avoid source moves until build/resources/includes are mapped; continue static build.
- **Acceptance:** same model outputs with and without graphical extensions; identical supported animation states at event checkpoints; no hardcoded target plugin classes in generic GUI; disabled extension leaves simulation intact; save/reopen/undo/delete pass.

### G5 — Bacteria Colony + Cellular Automata scene visualizations

- BacteriaColony graphical plugin declares the model plugin dependency, registers scene animation and View/Animations + toolbar contribution; binds a particular colony by persistent target identity; draws and updates in the **model scene** rather than relying on a Tools/Extensions window.
- CellularAutomata graphical plugin similarly contributes a configurable grid animation bound to the proper model data; no unrelated CA backend/DefaultNetwork refactor.
- Progressively retire the old Tools/Extensions viewers only after the in-scene experience and data persistence are complete.
- **Acceptance:** integration scenarios for insert, select/configure, start/pause/step/restart, closed/reopened model, extension disabled/missing, model change and UI repaint/latency; verify simulation results unchanged. Large biology snapshots may be throttled only if their visual semantics are declared.

### G6 — Component renderer + replaceable GUI service providers

- Register optional renderers by simulation type and style context (Classic Desktop/Modern Fusion or actual supported styling settings) with generic host fallback; ensure graphical themes do not change domain semantics.
- Register alternative providers for output reports, charts, Expression Builder etc. by stable service ID, with an explicit selected provider and deterministic no-provider default.
- Keep GUI menu/action identity stable; do not allow overriding arbitrary private widgets or action handlers.
- **Acceptance:** two implementations for the same service are selectable, predictable across save/settings when applicable, and default is restored on extension absence; style variations and persist/reopen tests. Verify no unapproved GUI startup/regression changes.

### G7 — Documentation, review and integration

- Each functional PR should update affected Developer/User Manual content and regenerate canonical PDF(s) when user-visible or architectural changes require it; record exact scope/versions/results and links in the mandated governance evidence locations.
- Close only the actual accepted slice in backlog/status. Mark the overall task `done_confirmed` only after complete acceptance, including negative/error cases and required regression evidence.
- Merge each validated bounded PR into `WorkInProgress`; do not promote to semester-stable or master as part of this roadmap.

## 5. Required verification matrix

| Invariant / scenario | Verification / oracle |
| --- | --- |
| Kernel/app separation | Build/test kernel and shell with GUI extensions disabled; inspect compile and link dependencies; no new Qt includes in kernel/plugins |
| Zero optional concrete classes in host | Source/include graph search plus compiler dependency confirmation; new exemplar requires no edits to generic scene/mainwindow/controllers |
| Registration/dependencies | Nominal, duplicate ID, missing required/optional model and system capability, GUI-only extension |
| Action/menu/toolbar | GUI interaction and Qt test: correct placement, handler, enable/visibility, checked/state and teardown |
| Animation creation | scene insert via tool and drag/drop, item position/size, undo/redo/delete, multiple instances and context switch |
| Runtime events | Correct pre/post state, model/replication/reset, event order, off-GUI-thread delivery and bounded refresh |
| Persistence | Original `.gui` fixtures; known and missing extension round-trip including unknown fields; explicit confirm discard |
| Renderers/services | deterministic selected provider, fallback, theme/style variants, two extensions contending |
| Safety/regression | CMake + Ninja Qt6 GUI preset, Google Test affected suites, unit/kernel/smoke as impact requires; ASan/UBSan/LSan on lifetime changes |

Report **PASS/FAIL/UNKNOWN**, with branch/head, environment, commands, fixture names, expected outputs and exact CI runs/artifacts. Do not infer PASS from build-only or PR merge. A remote-only agent must use CI evidence and cannot claim an interactive local GUI session.

## 6. Dependencies, risk control and rollback

- Prerequisite: current source/branch inspection, including ongoing GUI/editor modifications; `HUM-ARCH-001` / #492 is a blocker for eventual dynamic package migration, *not* for the current static GUI registration design.
- Preserve current registration and legacy GUI serialization while adding new generic constructs; avoid broad renaming/relocation that breaks model files or resources.
- Risk: plugin-provided `QGraphicsItem` ownership vs scene and QObject parents. Map create/adopt/delete/undo/rebuild before touching pointer lifetimes; focused sanitizers only on the affected paths.
- Risk: timing/observability of `Seize`/`Release`, queue occupancy and biological state. Validate against authoritative kernel state at declared event boundaries.
- Risk: unavailable extension records. Fail closed for malformed/mismatched payload and preserve unrecognized data, with user-confirmed destructive action.
- Rollback by reverting the bounded feature PR, not altering kernel semantics or deleting unknown data; retain compatibility fixtures.

## 7. Deferred dynamic loading / unloading gate (NOT part of G0–G7)

Dynamic graphical packages and unload support require a *separate future decision, issue, test plan and PR series*, after static target consolidation concerns and ABI design are resolved. Record, but **do not implement now**, Qt/C++ bridge vs approved versioned C ABI, package discovery, symbol/SDK/toolchain compatibility, host/plugin allocator boundaries, lifecycle and active scene/QObject/callback pinning, dependency unload order, event queue drain, security/trust, state rehydration, error recovery and teardown test matrix. No current-phase `QPluginLoader`, `dlopen`, ref counting or hot reload.

The stable C ABI mandated for future core packages does not allow Qt/C++ objects across that boundary. The future GUI boundary requires explicit design: versioned C ABI adapter where appropriate and/or a separately controlled Qt-compatible host SDK, with no undocumented compatibility claims.

## 8. Plan execution and exit conditions

- **Planning stage:** maintainer decision and docs recorded; no source change; `AUTO-GUI-003` not executable yet.
- **After explicit activation:** start with G0 audit and one minimum G1/G2 vertical slice proving registered static animation insertion. Do not begin queue/resource/bacteria migrations until that contract and persistence strategy are tested.
- **Success:** adding a new optional GUI extension requires no changes to generic GUI host source, it can declare simulation dependencies without owning them, it receives safe GUI-routed simulation updates, and its in-scene animation survives absence/reinstallation and save/reopen with no silent data loss.
- **Stop/escalate:** an unresolved semantic kernel event question, persistence compatibility ambiguity, ownership/ABI boundary shift, broad target reorganization, or new requirement for actual dynamic loading. Do not assume decisions not recorded in the approved architecture.

**Documentation-only manual impact:** this plan records future architectural and UI changes but modifies no current runtime/user instructions. Developer/User Manual updates and generated PDFs belong to the implementation PRs as their actual scope is delivered.
