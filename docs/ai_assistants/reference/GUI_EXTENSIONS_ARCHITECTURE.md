---
document_type: technical-architecture
authority: technical-reference
owner: project-maintainer
last_reviewed: 2026-11-11
status: approved-concept-implementation-pending
tracks: HUM-ARCH-004
---

# GenESyS Main GUI — Graphical Extension Architecture

## 1. Authority, status and scope

This document records the **maintainer-approved architectural direction of 2026-11-11**, not a claim that the complete design is implemented. Apply [governance](../GOVERNANCE.md), the [canonical architecture](../ARCHITECTURE.md), [current status](../STATUS.md), [plugin reference](PLUGINS.md) and the [implementation plan](GUI_EXTENSIONS_IMPLEMENTATION_PLAN.md). `HUM-ARCH-004` records the decision; `AUTO-GUI-003` tracks future execution. A later interface/signature choice must be verified against source and tests rather than inferred from this proposal.

**Goal:** an optional graphical extension can add menus/actions/toolbar entries, dialogs/windows/docks, animation tools and scene-bound visualizations, component renderers or alternative GUI service providers *without editing or recompiling the generic GUI source for each new plugin type*. Simulation plugins must remain usable by the kernel and non-graphical applications without Qt or any GUI extension.

**Not in current scope:** implementation; widespread CMake or source moves; dynamic loading/unloading; hot reload; converting simulation plugins to shared libraries; a second plugin framework; changes to biological/modal/continuous simulation semantics. PR #436 and the two student DCS-5 deliveries are historical candidate material **only**, not binding design or a merge/cherry-pick target.

## 2. Current source evidence (WorkInProgress, inspected 2026-11-11)

- `source/applications/gui/genesys/extensions/GuiExtensionContracts.h` already defines abstract `GuiExtensionPlugin`, `GuiExtensionRegistry` and `GuiActionContribution`, `GuiWindowContribution`, `GuiDockContribution`; `GuiExtensionRuntimeContext` exposes raw pointers to GUI internals.
- `GuiExtensionManager` installs contributions and checks `requiredModelPlugins()`; `GuiExtensionPluginCatalog` registers statically and resolves configurable enabled/disabled extensions.
- The main GUI CMakeLists.txt recursively includes its `*.cpp` into `genesys_gui_application`. Existing extension files are not independently loadable packages.
- `ModelGraphicsScene` contains plugin-specific `DrawingMode` choices for queue/resource/station and constructs their placeholders; `SceneToolController` invokes those concrete modes. `AnimationQueue` is referenced directly by the scene.
- `SimulationEventController` connects the GUI to `OnEventManager` events. `OnEventManager` supports event handler registration; a generic removal/subscription lifetime contract was not established from its current public interface.
- `GraphicalModelSerializer` serializes current animation types and placeholder dimensions/targets, but does not expose a generic opaque payload round-trip contract for unknown future extension types.
- `PluginInformation` distinguishes dynamic-library filename dependencies and `SystemDependency`. These kernel metadata concepts may inform, but do not automatically define, graphical extension dependency behavior.
- `BacteriaColonyViewerGuiExtensionPlugin` currently exposes a Tools/Extensions viewer, periodically reads colony state, and documents that generic extension-owned widgets lack per-event callback support.

These are *code observations*, not executed test results. No dynamic-plugin functionality is claimed.

## 3. Mandatory architectural invariants

1. **Strict dependency direction:** kernel, parser and simulation plugin sources never include Qt/GUI application headers or depend on graphical extensions. The shell/worker/model-specific applications operate without GUI extension code.
2. **Generic GUI host knows no optional plugin classes:** generic `MainWindow`, `ModelGraphicsScene`, controllers, registry, dispatcher and serializer must not include headers or hardcode class names of `BacteriaColony`, `CellularAutomata`, `Queue`, `Resource`, `Station` or other optional plugins. Such knowledge belongs only to corresponding concrete graphical extension implementations. Shared kernel-facing abstract contracts are allowed.
3. **One graphical-extension contract, one registry:** evolve `GuiExtensionPlugin` and `GuiExtensionRegistry` instead of a competing plugin architecture. Plugins register only contribution categories they use; no mandatory empty overrides for unused categories.
4. **Dependency is not ownership:** optional graphical extensions declare required model/simulation plugins, other GUI extensions and system requirements. Activation does not construct a model component, transfer ownership of a `Plugin` or require a model already to contain that component.
5. **Static now, dynamic capable later:** extensions remain compiled/registered with the application for the present phase, but dispatch, rendering and persistence must be based on stable identifiers and registered factories, not host `#include`, `dynamic_cast` to optional plugin types, hardcoded enums or switch branches.
6. **Generic scene/editor reused:** do not create a parallel `QGraphicsScene` for each domain. Respect the existing submodel/Network editor and editing-context semantics.
7. **Preserve semantics and data:** graphical extension installation must not change simulation event order, results, model ownership or kernel persistence. The GUI file must preserve unknown graphical-extension records by default through reopen/save.
8. **No silent takeover:** competing service providers/renderers use deterministic explicit selection with safe default fallback, not menu mutation or last-registered-wins.
9. **No UI code on kernel event threads:** any crossing into Qt GUI objects requires thread-safe GUI delivery; event/snapshot meaning and chronological ordering must be explicit.

## 4. Conceptual structure and responsibilities

`GuiExtensionPlugin` (existing pure abstract class):
- identity (stable string ID), display name, version, dependencies, contribution registration;
- its existing `registerContributions(GuiExtensionRegistry*)` remains the single entry point; do not create virtual methods for every menu, window or animation category;
- consider versioned capability metadata only when required by a concrete extension or compatibility test.

`GuiExtensionRegistry` (existing):
- retain `addAction`, `addDock`, `addWindow` and their current contribution structures;
- proposed categories: `addAnimation` (tool descriptor + creation/restore factory), `addComponentRenderer` (type/style-specific representation), `addServiceProvider` (replaceable GUI functionality such as output reports, result plots, Expression Builder);
- each contribution gets a stable ID, metadata, optional context predicates and an explicit handler/factory. GUI host owns placement and orchestration; extension owns domain rendering/behavior.

`GuiExtensionManager` / `GuiExtensionPluginCatalog` (existing):
- resolve activation/dependencies and installation; map actions/menus/animation toolbar and View/Animations items; keep static registration deterministic;
- for future dynamic loading, discovery is an adapter and not part of a plugin-specific code path in the GUI host;
- diagnose missing dependencies and duplicate/ambiguous IDs; avoid auto-installs and shell commands.

`GUI animation tool and scene factory` (new contribution contract, exact C++ names TBD):
- represents insertable graphical animation tools, not model-component factories;
- descriptor provides stable extension ID, animation type ID, display text/icon, tool placement, factory and configuration/restore support;
- scene uses the active registered tool/factory instead of an enum entry or branch for each plugin; generic drag/drop, selection, resize, grouping, redraw, undo, property editing and lifetime remain host responsibilities where already supported;
- extension-created visual items can represent many simulation objects or none; animations in a model must be able to identify the appropriate target instance, not just an ambiguous display name;
- no need to mandate `AnimationPlaceholder` as the base of every extension renderer until its suitability and actual ownership contract are tested.

`GUI event observation bridge` (new or bounded extension of `SimulationEventController`, name TBD):
- GUI subscribes through existing kernel `OnEventManager`/controller mechanisms and forwards relevant notifications or consistent snapshots to interested active graphical instances;
- extension-specific interpretation of `Seize`, `Release`, `Enter`, `Leave`, biological transitions or cellular states never goes in generic GUI dispatcher code;
- document pre/post-event semantics, order, simulation time, GUI-thread transfer, throttle/coalescing policy, cancellation/model close and the effect of skipped intermediate events;
- an animation requiring each transition cannot silently be replaced by sampled latest-state polling; a snapshot-oriented visualization may deliberately throttle refresh.

`Graphical persistence bridge` (extend existing `GraphicalModelSerializer`):
- generic record identifies `extensionId`, `animationTypeId`, schema/version, instance/item identity, target identity (type + stable model object ID or suitable context key), geometry, appearance and extension-owned data;
- verify exact current `.gui` grammar and support existing current-format animation records rather than silently changing/deleting them;
- known extension: factory reconstructs item + payload + target binding; unknown extension/type/schema: keep opaque raw payload and common placement/configuration metadata, display a non-executing placeholder if useful;
- **default on resave: preserve missing-extension records**, including unrecognized fields, without lossy parsing/re-encoding; user may explicitly choose to discard these records before save, with clear confirmation and list of affected items;
- absence of a graphical extension is distinct from absence of a model plugin required to deserialize/execute the underlying scientific model; do not claim the latter can always load.
- a save/reload/save/reinstall/reload round-trip is the acceptance oracle for unknown extension preservation.

`Component renderer` (contribution category):
- register optional visual representation for a simulation component type without forcing the host to include concrete plugin headers; account for graphical `Interface Style`, theme and selection via an explicit render context, with a generic fallback;
- Qt's `QStyle` does not by itself define arbitrary `QGraphicsItem` rendering; style mapping is a GUI concern.
- distinguish domain diagram projection from process-flow connections, especially ModalModel/DefaultNetwork contexts.

`GUI service provider` (contribution category):
- extension may supply alternative output report, results chart, Expression Builder or another named host capability;
- a stable service ID, defined contract, default provider, explicit user choice/priority and deterministic fallback; no arbitrary replacement of existing `QAction` or direct rewiring of `MainWindow`.

## 5. Dependency declaration and identification

Support declarative dependencies on:
- simulation/model plugin identifiers (already exposed by `requiredModelPlugins()`);
- GUI extension IDs (future typed metadata);
- system capability/library/Qt requirement (future typed metadata, diagnostics only).

Distinguish required from optional dependencies only when real use cases establish the activation semantics; do not eagerly implement a dependency solver or installer. Normalize IDs consistently and detect cycles, missing/duplicate IDs and incompatible versions as appropriate to the stage. Never run an arbitrary dependency `installCommand` from plugin metadata.

Recommended identity levels: a namespaced stable extension ID, version/schema of the contract, namespaced contribution IDs, and a distinct persistent graphical-instance ID. Do not require random GUIDs for every kind; use them only when stable identities cannot otherwise be guaranteed.

## 6. Repository placement proposal (not a source move yet)

Current real repository root is `source/`, not `src/`. Retain reusable extension contracts, manager and catalog under:

`source/applications/gui/genesys/extensions/`

**Proposed home for concrete GUI plugins:**

`source/applications/gui/genesys/extensions/plugins/`

Example subtrees: `plugins/BacteriaColony/`, `plugins/CellularAutomata/`, `plugins/Queue/`, `plugins/Resource/`, `plugins/Station/`. These are proposals, not current files or mandated one-plugin-one-directory targets. Initially register and compile concrete plugins statically under the existing GUI build. Move existing implementations into this tree only in scoped, independently reviewed changes after auditing includes, resources and build/test paths.

Future compilation/distribution packaging can use separate targets/packages and a loader adapter, without rewriting the GUI host. The structure of a future installable SDK is undecided.

## 7. Example behavior and migration priorities

**Bacteria Colony:** currently a Tools/Extensions viewer; replace this experience *progressively* with a scene-insertable/resizable animation registered through View/Animations and animation toolbar. Depend on the BacteriaColony simulation plugin; bind to a particular model colony; draw colony state in-scene and update through the GUI event bridge. Do not automatically create colony model objects on plugin registration. Retire the old Tools/Extensions window only after feature parity and verified persistence.

**Cellular Automata:** scene-insertable grid and configurable rendering; depend on the existing CA simulation plugin as applicable. Do not combine graphical migration with the separately deferred Cellular Automata -> DefaultNetwork backend migration.

**Queue/Resource/Station:** migrate host-side special drawing modes, placeholders, event interpretations and direct animation invocations into registered graphical contributions, keeping generic model rendering and lifecycle intact.

**GUI-only extension:** a custom dialog, window, toolbar action or menu can register without *any* kernel plugin dependency.

**Alternative renderer/service:** register type/style-specific component renderer or replaceable host service; user selects an alternative; absence restores the default provider.

## 8. Future dynamic loading/unloading — design constraints ONLY

The current approved **simulation plugin** future package boundary is a *stable versioned C ABI with opaque handles*, explicit create/destroy, function/capability tables, structured errors and no STL/Qt/C++ objects/exceptions across that stable ABI. Do not contradict `ARCHITECTURE.md` / `PLUGINS.md`. A Qt C++ interface (`QPluginLoader` or similar) is suitable as an in-process mechanism among tightly compatible builds but is **not automatically** a cross-compiler/version-stable binary SDK. The future relationship between a graphical Qt SDK/adapter and the mandated stable C ABI requires an explicit design decision.

When dynamic loading is activated **in a separate future project**, evaluate and test:
- library metadata, exports/visibility, semantic/ABI/package version compatibility, dependency resolution and load order;
- factory/handler code lifetime, Qt parent-child vs non-QObject ownership, outstanding callbacks and event queues, shutdown and unload order;
- no unloading while QObjects, `QGraphicsItem` instances, providers, callbacks, resources or threads still reference library code;
- failure/rollback on load, initialize, unload, missing dependency and incompatible version; persistence across absence/reactivation;
- trust/security model for foreign native code, filesystem search paths, signing/trust if required and sandbox assumptions;
- Qt runtime and toolchain binary compatibility, UI thread contract, package relocatability.

**Do not implement unload guards, reference counting, pinning, hot reload, dynamic package compilation or additional fallback frameworks in the present static phase.** Record only these requirements and keep code dependency directions compatible.

## 9. Evidence, risks and acceptance boundary

Architecture is accepted for **planning**, not claimed implemented or dynamically loadable. Main risks: persistence compatibility and exact opaque retention; event timing vs observed state, render performance; pointer/Qt object lifetimes; conflicting contributions/services; editing-context/undo ownership; unstable C++/Qt ABI; optional dependency resolution. The [implementation plan](GUI_EXTENSIONS_IMPLEMENTATION_PLAN.md) defines milestone tests and stop gates.

A key end-to-end target is that a newly written sample GUI extension adds a graphical animation to the model editor without modifying generic `MainWindow`, `ModelGraphicsScene`, generic controllers or kernel source. Subsequent tests must demonstrate static disabled/absent behavior, round-trip with missing extension, and host fallback.

No changes to C++ sources, CMake, kernel or user-visible runtime behavior are authorized by this document alone.
