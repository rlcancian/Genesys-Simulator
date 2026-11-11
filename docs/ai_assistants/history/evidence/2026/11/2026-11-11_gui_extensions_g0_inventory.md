---
document_type: evidence
authority: source-inspection
owner: project-maintainer
last_updated: 2026-11-11
status: pending-local-validation
tracks: AUTO-GUI-003
---

# Graphical extensions G0 — source-backed inventory and local-validation handoff (2026-11-11)

## 1. Provenance and scope

- Project: `rlcancian/Genesys-Simulator`.
- Source baseline **confirmed**: `WorkInProgress` HEAD `aeddc481d00ae17bfdfaa0506f361f03c9939cf9` (documentation PR #574 merge); implementation branch `WiP202611/gui-extensions-architecture-implementation` created directly from this commit.
- Evidence type: GitHub connector *read-only source inspection*, file-path/line examination and Git tree inventory. **No local checkout, generated CMake graph, compilation, GUI execution, CTest or sanitizer execution**. Source findings are not runtime PASS.
- Authority: [governance](../../../GOVERNANCE.md), [approved architecture](../../../reference/GUI_EXTENSIONS_ARCHITECTURE.md), [unchanged G0-G7 plan](../../../reference/GUI_EXTENSIONS_IMPLEMENTATION_PLAN.md), `HUM-ARCH-004`, `AUTO-GUI-003`.
- Scope is **G0 only**: coupling audit, ownership and event map, serialization grammar/fixtures, entry-point identification, test strategy. No C++/CMake/kernel modifications or architectural redesign. The previous student PR #436 remains historical reference only.

## 2. Existing interfaces and implementation inventory — confirmed in code

| File (all under `source/applications/gui/genesys/` unless specified) | Lines | Current responsibility and next boundary |
|---|---|---|
| `extensions/GuiExtensionContracts.h` | 18–24, 26–101 | `GuiExtensionRuntimeContext` contains `Simulator*`, `QMainWindow*`, private `Ui::MainWindow*`, `ModelGraphicsView*`, `ModelGraphicsScene*`. Existing action/dock/window descriptors and abstract `GuiExtensionPlugin::registerContributions`. No registered animation/renderer/service contribution type. |
| `extensions/GuiExtensionManager.cpp` | 35–76, 78–123, 126–240, 243–302 | Rebuilds and installs contributions, checks normalized model-plugin IDs, builds menus and binds actions/toolbars, deletes manager-created actions and docks. Does not provide generic animation factory/dispatch; no observed duplicate **contribution-ID** reject path. |
| `extensions/GuiExtensionPluginCatalog.h`, `.cpp` | header 15–33; cpp 21–23, 225–288 | Static auto-registration of plugin instances; `resolvedPlugins()` resolves config; `registerPlugin()` currently deduplicates pointer identity, not extension ID (cpp 277–285). |
| `extensions/BacteriaColonyViewerGuiExtensionPlugin.cpp` | 706–724, 1179–1257 | Polls live colony state using a Qt timer and registers Tools/Extensions biological viewer; real kernel plugin requirement at 1190–1192. This is an existing viewer, not an in-scene animation. |
| `extensions/CellularAutomataGuiExtensionPlugin.cpp` | 21–50 | Registers existing Tools/Extensions window, not in-scene animation. |
| `controllers/SceneToolController.cpp` | 46–59, 349–396 | Animation actions invoke specific `ModelGraphicsScene::drawing*` member functions; queue/resource/station hardcoded. |
| `graphicals/ModelGraphicsScene.h`, `.cpp` | header 116–120, 344, 479–481, 562–571; cpp 120–156, 2098–2117, 3161–3188, 3246–3307 | Enum + switch/factory for specific placeholder animation types; direct `AnimationQueue` inclusion and use; fixed insertion/undo processing. |
| `graphicals/GraphicalModelComponent.h`, `.cpp` | header 23, 131–170; cpp 38, 119–125, 370–550 | **Important residual coupling**: public header imports `plugins/data/DiscreteProcessing/Queue.h` and exposes `QMap<Queue*, ...>`, queue-specific images and helpers; cpp also includes `Decide.h` and specializes condition port count. Generic host cannot yet be independent of optional concrete plugin types. |
| `graphicals/GraphicalModelItemRenderStrategy.cpp` | 316–324, 584–607, 853–878, 1060–1085 | Existing Classic/Modern paint strategy selection; domain-specific GeneticCircuitPart and GeneticRegulation glyphs hardcoded by semantic class name. Renderer migration must preserve style and visual hit-shapes. |
| `controllers/SimulationEventController.cpp` | 399–416, 430–483, 487–501 | Routes process/movement callbacks and performs direct queue transition animation; registers `MainWindow` wrapper handlers in kernel `OnEventManager`. Current kernel callback semantics must be verified before event-specific migration. |
| `source/kernel/simulator/OnEventManager.h` | 179–242, 250–280, 298–377 | Registers replication/process/movement events and accepts member callbacks; no generic per-registration unsubscribe method identified in the inspected public surface. Do **not** add direct transient scene-item subscriptions to kernel in G1. |
| `services/GraphicalModelSerializer.cpp` | 609–1004, 758–878, 1723–1895, 1981–2050 | Saves known `.gui` graphical types (counter, variable, timer, placeholder) and restores by explicit recognized tags; no generic opaque unknown-extension record retention. In current animation load loop, unrecognized tag falls through without being retained (1727–1895). |
| `mainwindow.cpp`, `mainwindow.ui` | cpp 2283–2298; ui 930–934, 1333–1354, 1890–1914 | MainWindow owns `GuiExtensionManager`; `.ui` fixes toolbar and QAction entries for Queue, Resource, Station. The main host includes certain optional modal/EFSM plugin classes (mainwindow.cpp 25–29); **out of scope to remove them as unrelated cosmetic cleanup** before consumers are mapped. |
| `source/applications/gui/genesys/CMakeLists.txt` | 1–7, 15–23, 34–64 | Static aggregation via recursive `*.cpp` into one GUI executable; C++23. CMake file currently includes a Qt5 fallback, despite current Qt6-only governance — separate pre-existing issue, not a G0/G1 refactor. |
| `source/tests/unit/CMakeLists.txt` | 176–280 | Existing Qt-backed render and GUI GMDD/serializer suites include application sources; concrete targets `genesys_test_gui_render_strategy` and `genesys_test_gui_gmdd_layout`. Generated dependency graph remains **UNKNOWN** until CMake is configured. |

Source links: [contracts](https://github.com/rlcancian/Genesys-Simulator/blob/WorkInProgress/source/applications/gui/genesys/extensions/GuiExtensionContracts.h), [manager](https://github.com/rlcancian/Genesys-Simulator/blob/WorkInProgress/source/applications/gui/genesys/extensions/GuiExtensionManager.cpp), [scene](https://github.com/rlcancian/Genesys-Simulator/blob/WorkInProgress/source/applications/gui/genesys/graphicals/ModelGraphicsScene.cpp), [graphical component](https://github.com/rlcancian/Genesys-Simulator/blob/WorkInProgress/source/applications/gui/genesys/graphicals/GraphicalModelComponent.h), [serializer](https://github.com/rlcancian/Genesys-Simulator/blob/WorkInProgress/source/applications/gui/genesys/services/GraphicalModelSerializer.cpp).

## 3. Coupling classification and priority

**P1 — prevents true extension independence:**

1. `GraphicalModelComponent.h:23,131–170` and `.cpp:370–550`: `Queue` model class and queue-image ownership/lookup are exposed through generic graphical-item API. Moving only `AnimationQueue` will **not** decouple the scene. Model type identities, per-instance metadata and queue overlay semantics need a bounded migration after new factory/observer acceptance.
2. `ModelGraphicsScene.cpp:120–156,2098–2117` and `SceneToolController.cpp:383–392`: hardcoded `Queue/Resource/Station` animation tool choices, queue overlay update.
3. `mainwindow.ui:930–934,1333–1354`: static UI action/toolbar declarations for those animations. Registered tools must eventually replace the per-plugin UI entries rather than duplicate them.
4. `GraphicalModelSerializer.cpp:1723–1895`: unknown animation tags discarded on load; opaque extension payload must survive missing plugin and subsequent resave. This is a **data-loss risk for future formats**, not proof of a current-format bug.

**P2 — interface/lifecycle and migration constraints:**

5. `GuiExtensionRuntimeContext` exposes internal `Ui::MainWindow*`; future public SDK must not rely on private host layout.
6. Manager `registerPlugin()` rejects duplicate pointers, not duplicate `extensionId()`; contribution-ID collision handling is not demonstrated. Validate tests before adding rejection.
7. `GraphicalModelItemRenderStrategy.cpp` has genetic-glyph specialization and current style selection; do not migrate rendering before preserving those behaviors.
8. `GraphicalModelComponent.cpp:38,119–125` has direct `Decide` specialization for output ports; do not remove until semantic port contract identified, and separate it from G1 animation registration.
9. `BacteriaColonyViewerGuiExtensionPlugin.cpp:706–724` uses polling because no general extension event dispatcher; event delivery needs explicit post-process semantics and thread-affinity tests.
10. CMake source glob is **static**; future independent loading requires a separate later package/ABI decision and is not G0–G7 work.

This inventory identifies audited high-risk files; a complete **generated compiler/include/link dependency graph** is still pending. No statement of repository-wide absence of further concrete references is made.

## 4. Ownership and lifetime — current trace and open questions

```text
Simulator -> current Model -> ModelComponent / ModelDataDefinition (kernel-managed)
      |                               ^
      | observer/event callback          | non-owning model pointers
      v                               |
MainWindow --owns unique_ptr--> GuiExtensionManager
      |                            | stores non-owning static
      |                            | GuiExtensionPlugin*; owns QAction/QDockWidget
      v                            v
ModelGraphicsView -> ModelGraphicsScene (QGraphicsScene)
                                  | owns items while attached to scene
                                  +-- GraphicalModelComponent -> Queue* map (non-owning)
                                  +-- AnimationPlaceholder/Counter/Variable/Timer
                                  +-- Undo commands retain item references across detach/redo
```

- `ModelGraphicsScene.cpp:1440–1521` categorizes/detaches drawing items via `dynamic_cast`; `addItem` gives scene ownership while attached. `ModelGraphicsScene::clear()` calls `QGraphicsScene::clear()` (cpp 338–340); destructor clears animation collections and deletes transient drawing pointers (cpp 200–320).
- `clearAnimationsPlaceholder()` currently empties its observer/list container without itself deleting attached items (cpp 2237–2242); do not infer a leak from list clear alone.
- `AddUndoCommand.cpp:54–174` and `DeleteUndoCommand.cpp:97–330` detach/re-add graphical items; no extension factory may destroy an item that is still retained by undo stack. No lifetime correction proposed before targeted tests.
- `GuiExtensionManager.cpp:78–93` deletes created actions/docks; registered static plugin instances come from auto-registrar (`GuiExtensionPluginCatalog.h` 28–33). Future module unload is **not implemented and must not receive guards now**.
- `OnEventManager` callback registration (header 192–225) may hold receiver references; recommend a GUI-owned long-lived forwarding bridge, not direct per-item handlers. Exact threading/callback order **UNKNOWN** until local execution/targeted tests.

## 5. Before/after responsibility matrix (approved intent)

| Responsibility | Currently observed | Post-migration owner / invariant |
|---|---|---|
| GUI extension identity and registration | Existing `GuiExtensionPlugin` + catalog | Same abstractions, typed dependencies and deterministic IDs |
| Menus / windows / docks | `GuiExtensionRegistry` + manager | Same; extend contributions only where actually needed |
| Animation toolbar + creation | `mainwindow.ui`, `SceneToolController`, scene enum/switch | Registered animation descriptors/factories; generic scene/tool host |
| Queue/resource/station visual state | `GraphicalModelComponent`, `AnimationQueue`, scene | Domain graphical extension(s), not generic host or kernel |
| Biological/cellular viewer | Tools/Extensions window | In-scene registered animation + optional compatible tools; retire old window progressively |
| Event reception | Kernel `OnEventManager` -> GUI controller -> concrete scene | Kernel remains unchanged; generic GUI event forwarding to active extensions |
| GUI item persistence | Serializer knows fixed types | Host common format + type-specific payload, unknown records preserved |
| Component render style | Classic/Modern strategy with domain exceptions | Host style context + optional registered renderer, generic fallback |
| Report/chart/Expression Builder replacement | Existing host implementations | Explicitly selected registered service providers, deterministic fallback |
| CMake/source packaging | Static single main-GUI executable | Static for current scope; separate future dynamic package project |

## 6. Current test baselines and fixtures (not executed in this session)

**Tracked `.gui` files:** Git tree inventory found **zero** tracked files with a `.gui` suffix at baseline. Thus **no checked-in graphical animation round-trip fixture can be asserted**. Existing GUI tests create temporary `.gui` files:

- `source/tests/unit/test_gui_model_save_diagnostics.cpp:20–99` uses a serializer harness with `QTemporaryDir`; tests repeated save and failure preservation.
- `source/tests/unit/test_gui_efsm_diagram_persistence.cpp:69–180` uses `QTemporaryDir` and the real graphical save/load path; reuse this test fixture/harness pattern for animation records.
- `source/tests/unit/test_gui_gmdd_layout.cpp` covers scene, undo, property and model layout; `test_gui_submodel_level.cpp` covers hierarchical editing contexts.
- `source/tests/unit/test_gui_render_strategy.cpp:117–218` exercises Classic/Modern rendering and genetic specialized glyphs.
- `source/tests/unit/test_support_oneventmanager.cpp` covers kernel event manager; check exact cases before event bridge changes.
- Model-domain candidate fixtures, **kernel `.gen` only**: `models/Smart_SeizeDelayRelease.gen`, `models/Smart_RouteStation.gen`, `models/Smart_CellularAutomata.gen`, `models/Smart_CellularAutomata1.gen`, `models/BacteriaColony_TwoSignalDemo.gen` and `models/Smart_BacteriaColony_GRO.gen`. Their GUI animation content **not established**; build graphical test fixtures separately.

**Proposed negative/positive tests (not executed):**

1. G1: plugin registry accepts unique stable IDs; duplicate ID/missing mandatory dependencies reject deterministically; GUI-only extension registers without kernel plugin.
2. G2: standalone static sample contributes tool and factory without edits to generic host; one item and multiple items inserted, resized, deleted, undone/redone, grouped and restored; inherited `ModelGraphicsScene` context remains correct.
3. G3: event ordering/pre-vs-post state, GUI-thread delivery, model close/reopen and disabled animation; missing extension `.gui` raw opaque record survives `load -> save -> reinstall -> reload`; explicit discard is irreversible only after confirmation.
4. G4–G5: queue/resource/station state at known kernel checkpoints, biological/cellular visual snapshots, standalone kernel results unchanged with GUI extension disabled.
5. G6: two providers same service ID, explicit choice and stable fallback; optional render style and geometric hit shapes.

Do not interpret the model `.gen` fixtures as `.gui` graphical regression fixtures. Mark unexecuted acceptance **UNKNOWN**.

## 7. Validation and handoff

| Check | Result | Evidence |
|---|---|---|
| Previous docs PR #574 merged | PASS | Merge commit `aeddc481d00ae17bfdfaa0506f361f03c9939cf9` |
| Previous branch has exclusive unmerged commits | PASS (none) | GitHub `compare_commits(WorkInProgress, WiP202611/gui-extensions-architecture-docs)`: ahead 0, behind 1, files 0 |
| Previous branch physically deleted | BLOCKED | GitHub connector lacks delete-branch/delete-ref operation; branch retained safely |
| G0 source-backed coupling and ownership map | PASS for inspected source paths | Paths and line references above |
| Generated compiler/include/link graph and Qt6 compilation | UNKNOWN / PENDING LOCAL VALIDATION | No local checkout/CMake invocation |
| Existing GUI test baselines and new animation behavior | UNKNOWN / PENDING LOCAL VALIDATION | No GUI/CTest execution |
| Dynamic plugin package load/unload | NOT IN CURRENT SCOPE | Future documented gate |

**Local continuation (before G1 source changes):**

1. Read the mandatory assistant docs and this record; checkout `WiP202611/gui-extensions-architecture-implementation`.
2. Run `cmake --preset gui-app -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`, then `cmake --build --preset gui-app --parallel "$(nproc)"`; inspect the generated compile commands and actual GUI source ownership and its includes (do not assume build-dir paths).
3. Run `cmake --preset tests-unit`, `cmake --build --preset tests-unit --parallel "$(nproc)"`, `ctest --preset tests-unit --output-on-failure`; record exact head/versions/counts. If the branch has only G0 documentation, these are **baseline checks**, not proof of a new factory.
4. Execute focused `genesys_test_gui_render_strategy` and `genesys_test_gui_gmdd_layout`/CTest test names reported by configuration and record runtime/diagnostics. An interactive Qt6 scene check under local GUI/Xvfb is useful for later G2.
5. For G1 start, first verify baseline acceptance and proposed exact API against code; keep one bounded PR, avoid changing scene/serialization until registry behavior and focused tests are green. C++ changes must come with tests and manual-impact assessment.

**G0 assessment:** source inventory and handoff documented. The exact generated build graph and executable baseline remain **PENDING LOCAL VALIDATION**. No G1 source implementation is performed in the cloud until the relevant gate is satisfied.

**Next action:** a local assistant validates generated build/source linkage and baseline tests; after recorded PASS, an authorized executor can implement the minimum G1 registry/dependency slice and its targeted tests. The first G1 source-editing action is separately gated, not silently initiated as part of this inventory.
