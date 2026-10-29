---
document_type: implementation-plan
authority: maintainer-approved-plan
owner: project-maintainer
last_reviewed: 2026-10-29
review_cadence: on-modal-gui-contract-change
status: active
tracks: HUM-MODAL-002,AUTO-MODAL-003,AUTO-MODAL-004
---

# ModalModel / DefaultNetwork — Qt6 Network Diagram Implementation Plan

## 1. Authority, decision and scope

Maintainer approval: 2026-10-29, in the GenESyS ModalModel architecture discussion.
Development branch: `WiP20261029/modalmodel-architecture`, created from `WorkInProgress`.
Integration target: `WorkInProgress`. This plan is **not** evidence of a tested GUI implementation.

Read `../README.md`, `../GOVERNANCE.md`, `../ARCHITECTURE.md`, `../STATUS.md`,
`../BACKLOG_HUMAN.md`, `../BACKLOG_AUTONOMOUS.md`, and `../runbooks/LOCAL_AGENT.md`
or `../runbooks/GITHUB_AGENT.md` as appropriate. The mandatory entrypoint remains
`/docs/ai_assistants/README.md`.

This plan refines `MODAL_NETWORK_GUI_ARCHITECTURE.md` and must be read with
`GENESYS_MODAL_MODEL_NETWORK_ARCHITECTURE.md` and
`MODAL_NETWORK_COMPLETION_PLAN.md`.
The existing G0–G6 milestone names are retained.

### DEC-MODAL-GUI-20261029 — Approved architectural contract

1. The top-level process canvas represents `ModelComponent` instances connected by
   ordinary process `Connection` objects carrying `Entity` instances.
2. `ModalModelDefault` remains a `ModelComponent` **adapter** that activates an
   attached `DefaultNetwork : ModelDataDefinition`; it does not own a nested process
   model merely because the GUI opens a hierarchical view.
3. A Network opens in a **separate graphical editing context**, with the same
   tab/navigation experience used for submodels where safe. That context is a view
   of a mathematical network, **not a process submodel**.
4. Network nodes, states, places, transitions, graph edges and arcs retain their
   existing mathematical types and ownership. They do **not** become
   `ModelComponent` or ordinary process `Connection` objects.
5. The `DefaultNetwork` domain model is the authority for topology, simulation state,
   validation and activation. GUI items are non-owning representations of the
   domain model. Network GUI data stores only visual/layout information.
6. Network state is shared when multiple `ModalModelDefault` adapters point to
   the same Network. Opening two views must not clone Network objects or create
   separate mathematical states.
7. The `.gen` semantic model must remain executable without a GUI. The existing
   `.gui` format includes the kernel model followed by comment-prefixed visual
   overlay sections; the new diagram layout belongs in that GUI overlay.
8. The GUI must not invoke Network activation, dispatch `Entity` objects, or introduce
   an independent simulation/event scheduler.
9. G1 starts with a read-only **EFSM** diagram. Editing, Network creation, other
   formalisms, runtime overlays and optional architecture extensions follow only
   after their respective phase gates.

The approved decision **does not change** the backend hierarchy, object ownership,
process execution semantics or persistence of mathematical state.

## 2. G0.1 — Current-code inventory (GitHub inspection, 2026-10-29)

The following are **confirmed by direct file inspection on the new branch**, not
by compiling, running the GUI, or executing tests.

| Area | Observed implementation | Implication for the GUI work |
|---|---|---|
| Network bridge | `source/plugins/components/ModalModel/ModalModelDefault.h` has `getNetwork()`, `setNetwork()`, `getNetworkName()`, `setNetworkName()` and binding APIs | G1 resolves the attached Network by the existing adapter contract; do not create a new adapter |
| EFSM domain | `source/plugins/data/ModalModel/EFSMNetwork.h` exposes states, transitions, initial/current state and conflict policy | First read-only rendering reads existing EFSM topology; no semantic update |
| DTMC domain | `source/plugins/data/ModalModel/MarkovChainNetwork.h` exposes Markov states, named/probabilistic transitions and network-owned state | A later renderer must not interpret a probability arc as a process connection |
| Process scene | `source/applications/gui/genesys/graphicals/ModelGraphicsScene.h` subclasses `QGraphicsScene` and supplies process-component/connection creation and diagram-definition helpers | Reuse Qt Graphics View patterns, not the process connection-creation path |
| Existing data-definition item | `graphicals/GraphicalModelDataDefinition.h` subclasses `QGraphicsObject`, binds a non-owning `ModelDataDefinition*` | Rendering/mouse handling may be reused selectively; diagram item ownership and lifecycle require review |
| Existing diagram link | `graphicals/GraphicalDiagramConnection.h` represents `ATTACHED`/`INTERNAL` data-definition visual associations | **Not** a formal-network edge; do not confuse with `EFSMTransition` or `GraphEdge` |
| Existing hierarchical tabs | `mainwindow.cpp` already implements `_ensureSubmodelViewContext()`, `_closeSubmodelViewContext()`, `QTabWidget`, and “Open Submodel” | Reuse tab lifecycle and navigation, **not** the lower process-level rendering |
| Tab context | `mainwindow.h` has `GraphicalModelViewContextKind::{RootModel, Submodel}`; the submodel context stores `modelLevel`, owner and graphics view | Introduce a separate Network context/kind after inspecting all tab switching, save, close and teardown paths |
| Current submodel filter | `MainWindow::_canOpenSubmodelFor()` checks components/data definitions by `getLevel() == owner->getId()`, while `_ensureSubmodelViewContext()` calls `setModelLevelFilter()` | This filter cannot define mathematical Network membership; never fake model levels to make the diagram appear |
| Context menu | `controllers/GraphicalContextMenuController.cpp` builds the item menu and currently appends the existing Open Submodel action for component/data-definition selections | G1 may add a distinct, type-checked `Open Network...` action without replacing general Submodel behavior |
| Property editor | `controllers/PropertyEditorController.cpp` and `propertyeditor/` select/edit process controls and data definitions | G1 is read-only; G2/G4 must inspect editability, notification and commit pipelines before exposing Network properties |
| Catalog | `controllers/PluginCatalogController.h` exposes categorized plugin-tree insertion/selection | G2 can reuse discovery where appropriate; do not display network edges as process component plugins |
| GUI persistence | `services/GraphicalModelSerializer.cpp` uses `saveGraphicalModel()` / `loadGraphicalModel()`, emits comment-prefixed `Graphical Model Data Definitions` and `Graphical Model Components` sections, parses `GuiSection` | Add a dedicated, versioned **Network diagram overlay section**; preserve existing sections and legacy load |
| Diagram reconstruction | `services/GraphicalModelBuilder.cpp` reconstructs model graphics, including data-definition associations | Keep process diagram reconstruction unchanged; build Network views from a resolved `DefaultNetwork` |
| Undo/redo | `graphicals/ModelGraphicsScene.h` exposes `getUndoStack()`; existing actions reside in `actions/` | Read-only G1 must create no undo actions; G3 tests command ownership and atomicity |
| Qt6 build | `source/applications/gui/genesys/CMakeLists.txt` provides `genesys_gui_application` / `genesys-gui`, with C++23 | Use the existing `gui-app` preset and retain Qt6-only target semantics |
| Unit harness | `source/tests/unit/CMakeLists.txt` has `genesys_add_unit_test()`, `gtest_discover_tests()`, and conditional Qt GUI tests | G0.2 may add headless record-codec tests following actual target/link patterns |

**Evidence boundary:** this is a source inspection, not a statement that any new
Modal Network GUI behavior exists or that current HEAD passes build/tests.

### G0.1 risk discovered: submodel navigation is not a Network editor

A regular submodel tab filters existing process elements by a model level.
Network membership is instead established by its data-definition ownership and
formalism-specific topology. Simply opening the existing submodel tab for a
`ModalModelDefault` would therefore give incorrect/incomplete rendering.
The safe reuse boundary is tab lifecycle and navigation, not scene semantics.

### G0.1 risk discovered: the current `.gui` file is a combined artifact

Current `GraphicalModelSerializer::saveGraphicalModel()` writes the canonical
kernel model first and then appends GUI visual data. The logical distinction
between `.gen` semantics and `.gui` layout **does not imply** two physically
independent files must always be present. New records must integrate with this
actual format and existing loaders.

## 3. G0.2 — Network diagram persistence contract (proposed; not implemented)

A `NetworkDiagramRecord` is a **planned GUI value object**, not an existing
C++ class. Its exact implementation API, overlay syntax and integration point
must be confirmed against the current serializer and tests before coding.

The **logical** version-1 record is:

| Field | Required contract |
|---|---|
| `schemaVersion` | Explicit `1`, reject/skip unknown future major versions safely |
| `networkRef` | Stable reference scoped to the parent model: Network class/type + name; include persistent ID only after validating ID stability across `.gen` load |
| `nodes[]` | Logical element reference (class/type + name or validated stable key), position and optional presentation metadata |
| `relations[]` | Formalism-specific relation reference, optional bend/label coordinates; must distinguish parallel relations and unnamed arcs |
| `viewport` | Optional zoom/center and other GUI-only view metadata |
| `extensions` | Optional forward-compatible presentation fields only |

Rules for the on-disk format:

- Put the record into a **distinct, comment-prefixed, versioned GUI section**
  in `.gui`; keep existing `GuiSection` parsing behavior and loading of
  previously saved models unchanged.
- Do not include mathematical topology, initial/current states, transition
  priorities/guards/actions, transition probabilities, markings or `Entity` data.
- Use a deterministic serialization order and the existing GUI text-encoding
  conventions where compatible; malformed records must not corrupt the model.
- A missing or orphaned visual reference is ignored or reported as a layout
  warning. It must never synthesize domain objects.
- A missing GUI record uses a deterministic auto-layout fallback in the viewer;
  `.gen` loading and simulation work independently.
- Multiple views of the same Network share **one Network identity** and its
  domain state. Layout can be per-Network; per-view viewport metadata must not
  create a second mathematical Network.
- **Identity gate:** prove stable references for duplicate names, unnamed EFSM
  transitions, parallel graph/Markov edges and CPN arcs. If current persistence
  cannot distinguish them, stop and record the limitation; do not silently
  select the wrong edge or change kernel ownership solely for GUI convenience.
- The initial EFSM viewer does not require persisted layout to display a diagram:
  building the view from the current domain topology with deterministic
  positions is a valid G1 fallback.

G0.2 must add headless tests for: empty Network record, 1–N diagrams,
Unicode/escaping, deterministic round-trip, unknown-version and malformed
records, missing domain elements, duplicate/parallel relation identity,
and loading an old `.gui` without Network sections.

**G0 gate:** approved record format + unit tests executed and green; no
mandatory regressions. G0.1 documentation alone does **not** close G0.

## 4. Incremental implementation sequence

| Phase | Minimal deliverable | Acceptance / stop gate |
|---|---|---|
| **G0.1** | Source-backed GUI and ownership inventory, architectural decision and handoff plan | Documentation review; **prepared in this branch** |
| **G0.2** | Confirmed record schema, `.gui` overlay codec, headless tests | Qt6/CMake/Ninja tests; existing `.gui` compatibility; unknown schema safe; no math mutations |
| **G1.1** | Separate read-only EFSM scene/render mapping with non-owning domain references | EFSM states/transitions correct, empty/null safe, no mutation or simulation side effects |
| **G1.2** | `Open Network...` action on ModalModel + hierarchical tab/view context | Open, switch, close and reopen safely; shared Network does not duplicate data; GUI app builds and interaction tested |
| **G2** | Create/choose/attach Networks; persist `.gen` plus `.gui` | Current-format save/reload/execute unchanged; binding synchronization verified |
| **G3** | Formalism-specific topology editing, commands and undo/redo | Invalid relations rejected, delete/undo ownership safe, focused sanitizers |
| **G4** | Property editing and semantic diagnostics | Backend remains authoritative; EFSM/DTMC/CPN validation and persistence tests |
| **G5** | Read-only runtime overlays | Pausing/closing scene does not alter simulation; no GUI dependency in kernel |
| **G6** | Accessibility, layout, usability and supported formalism coverage | GUI startup + interactive tests + regression + manual updates |

G1 starts with EFSM. Expand to DTMC, Graph/DAG and pragmatic CPN only after
their visual identity rules and drawing semantics are checked. CTMC, MDP,
Cellular Automata migration, full typed CPN, dynamic plugins and broad GUI
refactoring remain separate scopes.

## 5. Handoff to a future local coding agent

The local agent must start by reading
`/docs/ai_assistants/README.md`, then the repository root README,
governance, architecture, status, backlogs, local runbook and the three
Modal/Network documents referenced in Section 1.

1. Confirm clean Git status, fetch and inspect `origin/WorkInProgress`, and
   check that `WiP20261029/modalmodel-architecture` has not drifted.
2. Inspect implementation (not only headers) of tab/switch/close workflows,
   `GraphicalModelSerializer`, model identity and formalism relations.
3. Validate G0.2 identity and storage contract before creating C++ files or
   changing `source/tests/unit/CMakeLists.txt`.
4. Implement one bounded concern per commit; use tests first for the codec,
   then add read-only EFSM presentation as an independently reviewable slice.
5. Run `cmake --preset gui-app`, `cmake --build --preset gui-app`,
   focused GUI tests, `cmake --preset tests-unit`,
   `cmake --build --preset tests-unit` and
   `ctest --preset tests-unit --output-on-failure`; run smoke and sanitizers
   for any runtime/ownership change. Adapt only to actually configured targets.
6. For G1, manually or in a supported UI harness verify opening EFSM, switching
   tabs/models, closing a model with an open Network view, reopening the same
   Network from two adapters, and opening with no Network attached.
7. Write dated executed evidence in `history/evidence/`; update the appropriate
   task state, STATUS, applicable manuals and AI changelog before requesting
   integration.
8. Open a **draft PR** into `WorkInProgress` and do not merge without green
   mandatory validation and maintainer review.

## 6. Current delivery and remaining risks

This 2026-10-29 GitHub-only delivery records **approval, source inventory,
architecture and a staged plan**, without claiming GUI code, record codec,
Qt6 compilation, tests, sanitizer results, or a completed G0 gate.

Risks to resolve locally: stable identity of relation objects across reload,
process-versus-Network scene lifetime, switching and restoring the active tab,
Qt QObject/GraphicsItem versus kernel object ownership, `.gui` compatibility,
editing command rollback, and regression against existing GUI/property-editor
behavior. Do not start G2+ until those prerequisites are demonstrated.
