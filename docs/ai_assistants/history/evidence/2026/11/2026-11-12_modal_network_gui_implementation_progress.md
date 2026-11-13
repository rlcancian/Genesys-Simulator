---
document_type: implementation-progress-evidence
authority: informative
owner: project-maintainer
tracks: HUM-MODAL-003,AUTO-MODAL-005,HUM-MODAL-004,AUTO-MODAL-006
branch: WiP20261112/modal_model_gui
base_commit: 6b0facf40e4225c80b4d77cc31f212f5b3a8844e
status: in-progress
---

# ModalModel / Network GUI implementation progress

This log records executed work on the dedicated development branch. The approved
acceptance criteria remain in `reference/MODAL_NETWORK_GUI_EVOLUTION_PLAN.md`.

## Milestone state

| Milestone | State | Evidence / remaining work |
|---|---|---|
| G0 | Pass (bounded) | Verified clean branch base, CMake/Ninja presets, GUI target, GUI regression target, Network hierarchy and current menu/property-editor code. Focused fixtures cover EFSM, shared adapters, Graph/directed/DAG, Markov, CPN, null references, duplicate names and damaged identities. Baseline and integrated GUI filters pass. |
| A1 | Pass (focused) | Removed EFSM-specific create/select menu actions and handlers; generic Open Network labeling and reference gating use the shared submodel tab/context path. Adapter-open, null-reference and direct graphical Network data-definition open tests pass; repeated opening reuses the shared tab. |
| A2 | Pass (focused) | `Network` choices enumerate registered `DefaultNetwork` subtypes and resolve by concrete class plus model ID. Focused test selects EFSM, Graph, Directed Graph, DAG, Markov and CPN objects with the same name. |
| A3 | Pass (focused) | Plugin metadata records direct data-definition base types; PluginManager discovers transitive compatible factories. Property Editor creates and attaches an explicitly selected subtype and offers no inert base type. |
| A4 | Pass (focused) | Property Editor now offers explicit reference removal and permits reattaching an existing Network. A detached Network remains model-managed. The focused property and EFSM GUI tests pass. |
| A5 | Pass (focused) | Property Editor choices expose concrete type/name/ID; association editing and `Network Details` provide separate mathematical properties/binding rows, while Open Network remains contextual. Graph and DAG disclose their structural/no-activation semantics, and the generic base Network is labeled inert. Focused GUI coverage exercises typed creation, selection across all six concrete families, binding editing/undo, and a Create→ModalModelDefault→Dispose connection fixture for each family. |
| B1 | Pass (focused) | Identity audit is covered by six concrete subtype choices with duplicate display names, exact identity selection, unique legacy name resolution and ambiguous legacy name rejection. |
| B2 | Pass (focused) | ModalModelDefault persists concrete Network type and model ID alongside the legacy name. Exact lookup validates type, ID and name; invalid, missing and ambiguous legacy references fail closed with explicit diagnostics. Exact typed `.gen` round-trip and a damaged typed-ID load/check regression pass; legacy name-only ambiguity is also covered. Broader compatibility coverage remains in final validation. |
| B3 | Pass (focused) | Bindings remap by stable port names; new ports receive defaults; port-name metadata persists while detached. Unsafe custom-binding/connection changes reject transactionally. Property Editor confirms schema-changing replacements before mutation; cancellation preserves the old association. Binding rows are exposed in the Property Editor; the EFSM bindings dialog/action is removed. Undo/redo covers binding edits and Network reference changes, including graphical port synchronization. |
| B4 | Pass (focused) | ModelDataManager remains the Network owner; adapter detach and replacement do not delete an unshared or shared Network. Existing GUI coverage also proves reference removal leaves the Network selectable for reattachment. Focused adapter suite: 14/14 PASS. Broader close/delete lifetime audit remains in final validation. |
| B5 | Pass | Reviewed ModalModel implementation/header placement and Network data-definition headers/implementations. The existing split keeps GUI-independent mathematical implementations in plugin translation units and data contracts under `data/ModalModel`; no source relocation is justified. GUI app build passes. |
| C1 | Pass (focused) | Responsibility map and generic ownership hooks are recorded. Context-menu dispatch and scene overlays are formalism-neutral; EFSM item classification is removed from generic scene/controller. Extension action/handler dispatch and projection registration are covered by focused tests. |
| C2 | Pass (focused) | EFSM state/transition handlers and selection predicates are in `extensions/plugins/efsm/EFSMNetworkEditorController`; GUI build and focused edit/undo/reopen tests pass. |
| C3 | Pass (focused) | `EFSMGuiExtensionPlugin` registers five actions through the existing registry with stable IDs. Manager refreshes current-context visibility/enabled state and dispatches against the live context. Root/Network visibility and handler tests pass. |
| C4 | Pass (focused) | Projection synchronization, layout capture/restore, and `.gui` codec parse/write dispatch through registered diagram contributions. Generic MainWindow/serializer no longer include EFSM layout/codec types or name EFSM for tab titles/persistence detection. The existing EFSM section format remains unchanged; legacy sections load safely, and unknown diagram sections survive load/save. Focused projection, persistence, shared-tab, and save/reopen execution tests pass. |
| C5 | Pass (focused) | The shared `GuiNetworkEditorContribution` supports formalism identity, allowed drops, action predicates, projection refresh, and generic persistence integration. EFSM and Graph register concrete editor policies/actions without MainWindow formalism branches. Focused static registration and dispatch coverage passes. |
| C6 | Pass (focused) | EFSM, Graph, Markov and CPN extensions are statically registered and the GUI/test targets link them. Scene projection refresh is marshalled onto the scene QObject thread; unknown `.gui` records round-trip. With the Markov GUI contribution disabled through the supported extension catalog, `.gui` geometry still round-trips and the same Markov model executes its real transition. No separate binary build excluding the statically linked source was needed. |
| D1 | Pass (focused) | Network scene insertion enforces formalism-owned allowed-drop policy at the mutation boundary. The plugin palette now switches to each active formalism's accepted mathematical items and restores process components in root/ordinary Submodel contexts. Invalid process-component drops in Network are rejected, while ordinary Submodel component drops remain functional. Tests cover Graph, Markov, CPN, EFSM palette contents and MainWindow tab switching. |
| D2 | Pass (focused) | Graph extension renders directed/undirected topology including parallel/self-loop edges, accepts GraphNode drops, creates edges with explicit endpoints, supports edge weight/removal undo/redo, rejects DAG cycles, and shows BFS/DFS structural-analysis overlays without implying simulation events. Selected GraphNode deletion removes incident edges; undo restores the same node/edge identities, edge order and node position. Generic `.gui` round-trip restores node positions by identity; Graph nodes/edges remain associated with the Network during model checks. The inherited GraphNode `Name` control is present in the Property Editor. GUI and mathematical Graph test filters pass. |
| D3 | Pass (focused) | Static Markov editor accepts MarkovState drops, projects initial/current-state markers and directed probability-labelled transitions, and supports transition create/edit/remove plus initial-state selection with undo/redo. Selected-state removal also removes incident transitions; undo restores transition identities/order, initial/current states and state geometry. Runtime exposes the sampled transition; scene-thread projection refresh highlights the actual sampled transition. References survive model checks; `.gui` round-trip verifies topology, initial state, probability and state positions. The editor exposes diagnostics for malformed stochastic rows. Focused GUI and kernel tests pass, including seeded reproducibility and replication reset. |
| D4 | Pass (focused) | Static CPN extension accepts place/transition drops; projects bipartite arcs, current colored marking and enabled transitions; supports arc create/edit/remove, initial marking editing, transition guard/priority editing, firing and node removal with undo/redo. Node removal restores incident arcs/order, initial marking, current tokens, last-fired transition and geometry. Runtime highlights the actual firing path; aggregate inscription validation and model associations are covered. `.gui` round-trip verifies topology/marking/positions. Validation action surfaces kernel guard/topology diagnostics. Focused GUI and kernel tests pass. |
| D5 | Pass (focused) | Runtime exposes real EFSM/Markov/CPN state changes and queues diagram refresh to the scene thread; EFSM current state/fired transition, Markov sampled state/transition, and CPN fired transition plus its incident arcs are visually highlighted. Graph BFS/DFS is explicitly structural analysis. MainWindow marshals off-thread projection refresh onto the GUI thread and guards teardown. Controller integration runs a real two-replication process model through start, pause, step, resume and end; queued refresh is safely discarded after scene close. Formalism runtime projections are covered by focused GUI fixtures. Final broad integration validation remains. |

## Executed changes

### C1 responsibility map

| Area | Current EFSM responsibility | Intended boundary |
|---|---|---|
| `mainwindow.cpp` | Builds EFSM actions; implements state/transition CRUD and dialogs; gates actions by EFSM casts; restores/captures network layouts; requests EFSM synchronization; serializer layout cleanup. | EFSM GUI extension/controller owns actions and editing; generic host provides model, scene, undo, navigation and serializer lifecycle hooks. |
| `mainwindow.h` | EFSM action fields and handler declarations. | No EFSM-specific API or fields in the generic host. |
| `GraphicalContextMenuController` | Maps EFSM transition graphics to semantic items. | Generic controller handles scene selection/context; extension receives the clicked graphic and contributes semantics/actions. |
| `ModelGraphicsScene` | Owns a dedicated list of EFSM diagram items. | Scene owns generic extension graphics by lifetime or a formalism-neutral overlay registry. |
| `EFSMNetworkDiagramProjection` | Builds/synchronizes state and transition graphics; called directly by builder, serializer and MainWindow. | EFSM extension registers projection and invokes it via a generic projection lifecycle. |
| `EFSMNetworkDiagramCodec` / `GraphicalModelSerializer` | EFSM-specific `.gui` section parse/write and cached per-network layout; serializer has direct codec dispatch. | Keep file orchestration generic and register per-formalism section codecs; retain unknown records losslessly. |
| EFSM undo commands | Dedicated commands mutate EFSM state membership, initial state, transitions and properties. | Remain GUI-only/domain-aware in the EFSM extension module and share the scene undo stack. |

The generic context-menu extension hook is implemented and tested. This map records
the initial coupling and its current state; formalism editor-tool dispatch and
non-EFSM formalism work remain in C5 and D1–D5.

- Removed `Create EFSM Network` and `Select EFSM Network` from the Model menu and
  ModalModelDefault context menu, including their MainWindow action handlers.
- The shared open action is labeled `Open Network` for ModalModelDefault and
  DefaultNetwork graphical selections, and remains disabled for an unassigned
  adapter through the existing live-registration check.
- Updated the existing EFSM GUI test for the generic action label and absence of
  the obsolete actions.
- Added identity-based polymorphic selection for the ModalModelDefault
  `Network` reference. No generic `DefaultNetwork` creation option is exposed;
  creation requires a concrete type.
- Added direct data-definition base-type metadata and transitive plugin type
  discovery so GUI code does not maintain a hard-coded network factory list.
- Implemented concrete typed factory creation from the Network property. The
  selected factory result is assigned only after it passes the DefaultNetwork
  type check. Old context-dialog creation/selection regression tests were
  removed; Property Editor coverage now exercises those operations instead.
- Updated the existing EFSM save-close-reopen runtime regression to open its
  associated Network through the generic `Open Network` action.
- Added an explicit reference-removal choice for supported object references;
  Network removal detaches the adapter while retaining the Network as a
  model-managed root, allowing it to be selected again.
- Added concrete class and model ID fields to the ModalModelDefault `.gen`
  reference. Legacy name-only references are resolved only when the name is
  unique across all DefaultNetwork subtypes.
- Added a `.gen` round-trip regression asserting that the adapter resolves to
  the original concrete GraphNetwork by type and ID.
- Added legacy-reference regressions: a unique name resolves, while the same
  name shared by GraphNetwork and EFSMNetwork is rejected and reported as
  ambiguous during model validation.
- Network replacement now maps input/output bindings by port name, initializes
  added ports with defaults, and rejects replacements before mutation when a
  custom binding or connected port would be lost or change meaning. Binding
  port names persist across detach/reopen/reattach, including incomplete models.
- Added dynamic port-count queries to ModelComponent and graphical port
  synchronization so the Property Editor's select/create/remove actions update
  the ModalModel diagram's input and output handles. Existing connected handles
  remain represented.
- The Property Editor presents the kernel's incompatibility reason when a
  reference change is rejected.
- Removed the EFSM-specific bindings dialog, MainWindow action, and its dedicated
  undo command. Network input/output bindings are now editable child properties
  under `Network Details` in the Property Editor; association selection stays in
  the Property Editor.
- Added an opt-in reference-child editing contract, preserving existing selector
  behavior for unrelated ModelDataDefinition references.
- Added undo/redo commands for Network association transitions and input/output
  binding text edits. Association undo/redo re-runs the kernel's transactional
  reconciliation and synchronizes graphical ports; unsafe stale replay is marked
  obsolete. The focused UI regression now exercises both command families.
- Added a shared-reference lifetime regression: detaching one adapter and then
  replacing the other leaves both model-managed Network objects registered.
- Property Editor replacement prompts before changing an existing association
  when the input/output port schema differs. The confirmation explains
  name-based mapping and that unsafe loss of custom bindings or connected-port
  meaning is still rejected by the transactional kernel operation. Cancellation
  leaves the old reference intact and refreshes the Property Editor.
- Added read-only `Execution Capability` detail for GraphNetwork (including
  directed/DAG subclasses) and direct generic DefaultNetwork references. It
  describes the former as structural with no simulation-time activation events
  and the latter as inert; other formalisms receive no invented capability text.
- The A5 Property Editor regression selects all six network families, creates a
  typed network, removes/reattaches references, edits bindings, exercises
  reference/binding undo/redo and verifies Create→adapter→Dispose connection
  fixtures for each family.
- Added `GuiContextMenuContribution` to the existing extension contracts and
  registry, with visibility and population callbacks receiving the current
  runtime context and selected graphics item. `GuiExtensionManager` dispatches
  registered contributions; `GraphicalContextMenuController` provides a
  generic callback after its built-in menu population, and MainWindow connects
  that callback to the manager without formalism-specific branching. A focused
  fake-plugin regression verifies selected-item dispatch and visibility.
- Replaced the scene's EFSM-specific overlay list/API with a generic registry
  keyed by GUI extension ID. The EFSM projection now registers and clears its
  transition graphics through the `efsm` key; `ModelGraphicsScene` has no EFSM
  type, member, or API. Removed the EFSM graphics type check from the generic
  context-menu controller; unknown graphics items retain their identity for
  extensions to interpret.
- Extracted EFSM state creation/editing, transition creation/editing and initial
  state selection from MainWindow into the GUI-only
  `EFSMNetworkEditorController` under `extensions/plugins/efsm/`. Existing menu
  actions delegate to the controller; dialogs, undo commands, projection
  updates and dirty/action refresh behavior remain the same. MainWindow no
  longer declares or defines these five EFSM command handlers.
- Moved EFSM-specific action enablement and selection counting from MainWindow's
  `_actualizeActions()` into the EFSM controller. The composition root now
  calls one controller update hook; the existing EFSM edit/undo tests remain
  green after the move.
- Added a statically registered `EFSMGuiExtensionPlugin` under
  `extensions/plugins/efsm/`; it contributes create/edit/set-initial actions
  through the existing `GuiExtensionRegistry`, using stable IDs and the Model
  menu. MainWindow no longer constructs, stores or inserts EFSM QAction
  instances. EFSM context predicates live with the controller. The generic
  extension manager now updates contribution visibility/enabled state using
  the latest runtime context and invokes the latest context on trigger.
- Expanded the registered diagram-projection contribution to own synchronization,
  layout capture/restore, and `.gui` codec callbacks. MainWindow, builder, and
  serializer dispatch through the catalog; generic persistence stores opaque
  extension layouts and preserves unknown diagram sections. The EFSM extension
  retains the existing persisted section format.

## Validation

- `cmake --build --preset gui-app --target genesys_gui -j 4` — PASS.
- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 4` — PASS.
- `QT_QPA_PLATFORM=offscreen ./build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='EFSMNetworkEditor.SharedNetworkOpensOneSubmodelTabFromMultipleAdapters:EFSMNetworkEditor.AdapterWithoutNetworkCannotOpenASubmodelContext'` — PASS, 2 tests.
- `cmake --build --preset gui-app --target genesys_gui -j 4` — PASS after the Property Editor change.
- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 4` — PASS after the Property Editor change.
- `QT_QPA_PLATFORM=offscreen ./build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='PropertyEditorModelDataReference.ModalModelSelectsPolymorphicNetworkByTypeAndIdentity'` — PASS, selects all six concrete network families from duplicate-name options.
- `cmake --build --preset gui-app --target genesys_gui -j 4` — PASS with plugin type metadata and typed creation.
- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 8` — PASS.
- `QT_QPA_PLATFORM=offscreen ./build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='PropertyEditorModelDataReference.*:EFSMNetworkEditor.*'` — PASS, 28 tests, including association, typed creation, shared opening and EFSM save/reopen/activation.
- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 8` — PASS after A4/B2.
- `QT_QPA_PLATFORM=offscreen ./build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='PropertyEditorModelDataReference.*:ModalNetworkReferencePersistence.*:EFSMNetworkEditor.*'` — PASS, 29 tests.
- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 8` — PASS after legacy identity diagnostics.
- `QT_QPA_PLATFORM=offscreen ./build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='ModalNetworkReferencePersistence.*'` — PASS, 2 tests.
- `cmake --build build/tests-unit --target genesys_test_modal_model_default_network -j 8` — PASS; 13 adapter tests after B3.
- `./build/tests-unit/source/tests/genesys_test_modal_model_default_network` — PASS, 13 tests, including name remapping, detached reattachment and transactional rejection for lost custom bindings or connected output meaning.
- `QT_QPA_PLATFORM=offscreen ./build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='PropertyEditorModelDataReference.ModalModelSelectsPolymorphicNetworkByTypeAndIdentity:ModalNetworkReferencePersistence.*:EFSMNetworkEditor.*'` — PASS, 11 tests; includes GUI port counts 0/1/2 input and 0/1/3 output across network choices, creation, removal and reattachment.
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS after dynamic port synchronization and replacement safeguards.
- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 8` — PASS after Property Editor Network child properties and legacy dialog removal.
- `QT_QPA_PLATFORM=offscreen ./build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='PropertyEditorModelDataReference.ModalModelSelectsPolymorphicNetworkByTypeAndIdentity:EFSMNetworkEditor.AdapterBindingsHaveNoLegacyDialogAction'` — PASS, 2 tests; includes direct Property Editor binding edits and absence of the old action.
- `./build/tests-unit/source/tests/genesys_test_modal_model_default_network` — PASS, 13 adapter tests.
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS after the reference-child capability.
- `git diff --check` — PASS.
- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 8` — PASS after undo/redo integration.
- `QT_QPA_PLATFORM=offscreen ./build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='PropertyEditorModelDataReference.ModalModelSelectsPolymorphicNetworkByTypeAndIdentity:EFSMNetworkEditor.AdapterBindingsHaveNoLegacyDialogAction'` — PASS, 2 tests; verifies reference and binding undo/redo, port synchronization, Property Editor edits, and removal of the legacy action.
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS after undo/redo integration.
- `git diff --check` — PASS after undo/redo integration.
- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 8` — PASS after replacement confirmation.
- `QT_QPA_PLATFORM=offscreen ./build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='PropertyEditorModelDataReference.ModalModelSelectsPolymorphicNetworkByTypeAndIdentity'` — PASS; exercises schema-change cancellation, acceptance, reference undo/redo, binding edits and binding undo/redo.
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS after replacement confirmation.
- `git diff --check` — PASS after replacement confirmation.
- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 8` — PASS after A5 capability display and per-family process-flow coverage.
- `QT_QPA_PLATFORM=offscreen ./build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='PropertyEditorModelDataReference.ModalModelSelectsPolymorphicNetworkByTypeAndIdentity'` — PASS; verifies capability display, Property Editor operations, undo/redo, and all six subtype flow fixtures.
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS after A5 capability display.
- `git diff --check` — PASS after A5.
- B5 bounded review: `rg` found no GUI/Qt includes in the ModalModel component/data directories; cross-directory includes follow the existing data-contract/component-implementation split. No physical moves made.
- `cmake --build build/tests-unit --target genesys_test_modal_model_default_network -j 8` — PASS after shared Network lifetime coverage.
- `./build/tests-unit/source/tests/genesys_test_modal_model_default_network` — PASS, 14 tests, including shared-reference detach/replacement retention.
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS after A4/B2.
- Full GUI regression suite — UNKNOWN/not run. Existing tests for the removed
  create/select actions must be migrated to the Property Editor in A2/A3.

## Commits

- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 8` — PASS for extension-menu dispatch.
- `QT_QPA_PLATFORM=offscreen ./build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='GuiExtensionManager.ContextMenuContributionsAreDispatchedWithoutFormalismBranches'` — PASS, 1 test.
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS after extension-menu dispatch.
- `git diff --check` — PASS after extension-menu dispatch.
- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 8` — PASS after generic overlay tracking.
- `QT_QPA_PLATFORM=offscreen ./build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='EFSMNetworkDiagramProjection.*:EFSMNetworkEditor.*:GuiExtensionManager.ContextMenuContributionsAreDispatchedWithoutFormalismBranches'` — PASS, 10 tests.
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS after generic overlay/context dispatch.
- `git diff --check` and `rg -n 'EFSM'` over generic scene/context-menu controller sources — PASS (no EFSM references).
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS after handler extraction.
- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 8` — PASS after handler extraction.
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS after moving action enablement.
- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 8` — PASS after moving action enablement.
- `QT_QPA_PLATFORM=offscreen ./build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='EFSMNetworkEditor.SelectedStateCanBeEditedAndUndoneThroughTheMainWindowAction:EFSMNetworkEditor.SelectedTransitionCanBeEditedAndUndoneThroughTheMainWindowAction:EFSMNetworkEditor.MainWindowSaveCloseAndReopenRestoresAndExecutesEFSM'` — PASS, 3 tests after moving action enablement.
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS after static action contribution integration.
- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 8` — PASS after action contribution integration.
- `QT_QPA_PLATFORM=offscreen ./build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='GuiExtensionManager.*:EFSMNetworkEditor.*'` — PASS, 9 tests; includes dynamic contribution state/dispatch, stable EFSM action ID, hidden root context, enabled Network context, editing, undo and persistence/reopen execution.
- `git diff --check` — PASS after action contribution integration.
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS after generic diagram layout/codec dispatch.
- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 8` — PASS after generic diagram layout/codec dispatch.
- `QT_QPA_PLATFORM=offscreen ./build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='EFSMNetworkDiagramPersistence.*:EFSMNetworkDiagramProjection.*:EFSMNetworkEditor.SharedNetworkOpensOneSubmodelTabFromMultipleAdapters:EFSMNetworkEditor.MainWindowSaveCloseAndReopenRestoresAndExecutesEFSM'` — PASS, 22 tests; includes EFSM layout round-trip/determinism, legacy loading, unknown-section preservation, projection geometry, shared tab reuse, and saved/reopened EFSM execution.
- `rg -n 'EFSMNetworkDiagramProjection'` on `GraphicalModelBuilder.cpp` and `GraphicalModelSerializer.cpp` — PASS, no direct type references.
- `git diff --check` — PASS after projection contribution dispatch.
- `QT_QPA_PLATFORM=offscreen ./build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='EFSMNetworkEditor.SelectedStateCanBeEditedAndUndoneThroughTheMainWindowAction:EFSMNetworkEditor.SelectedTransitionCanBeEditedAndUndoneThroughTheMainWindowAction:EFSMNetworkEditor.MainWindowSaveCloseAndReopenRestoresAndExecutesEFSM'` — PASS, 3 tests; confirms state/transition editing, undo and saved/reopened execution behavior remain.

- `c3f97a9e30866e5ae3b8cc670cce867c96a0d823` — `modal-gui(A1): replace EFSM menu actions with Open Network` (pushed to `origin/WiP20261112/modal_model_gui`).
- `67ee70abfc066da4916db55953616ada985aa15f` — `modal-gui(A2): select typed network references by identity` (pushed to `origin/WiP20261112/modal_model_gui`).
- `e9feaae618581bb032d6f91251f1191897f99500` — `modal-gui(A3): create explicitly typed network references` (pushed to `origin/WiP20261112/modal_model_gui`).
- `f08fe0e0f` — `modal-gui(A4,B2): preserve typed network references` (pushed to `origin/WiP20261112/modal_model_gui`).
- `31296e77f` — `modal-gui(B2): diagnose ambiguous legacy network references` (pushed to `origin/WiP20261112/modal_model_gui`).
- B3 replacement, binding-schema persistence, graphical port synchronization, Property Editor binding edits, cancellation/acceptance confirmation, and undo/redo are validated for the cases listed above and pending commit.
- B4 focused ownership evidence: detach/replacement does not remove Network objects from their model manager, including when a second adapter still references the shared object. Full destruction/scene-close audit remains a final-validation item.
- `2a1c27852` — `modal-gui(C1): add extension context menu contributions` (pushed).
- `f104f4141` — `docs(modal-gui): map EFSM host responsibilities` (pushed).
- `da9858712` — `modal-gui(C1): generalize extension scene overlays` (pushed).
- `16b43904b` — `modal-gui(C2): extract EFSM editing handlers` (pushed).
- `55e15fbba` — `modal-gui(C2): isolate EFSM action enablement` (pushed).
- `5ca1431aa` — progress-log commit recording the C2 enablement extraction (pushed).
- `5158f456c` — `modal-gui(C3): register EFSM actions as extension contributions` (pushed).
- C4 generic capture/restore/codec dispatch, unknown-section round-trip regression, and serializer test adaptation are validated and pending commit.

- Added a minimal `GuiNetworkEditorContribution` to the existing extension registry/catalog. Network scenes retain their non-owning formalism owner; the actual drop insertion boundary asks the registered editor policy before constructing any model object. Root process canvases keep their existing unrestricted behavior.
- EFSM registers FSMState as its current allowed palette drop; process components and unrelated data definitions are rejected for that Network.
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS after the drop policy integration.
- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 8` — PASS after the drop policy integration.
- `QT_QPA_PLATFORM=offscreen ./build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='GuiExtensionManager.NetworkCanvas*'` — PASS, 2 tests; accepted EFSM mathematical data definition, rejected component/unrelated type, and actual scene drop rejection before model mutation.
- `git diff --check` — PASS after the drop policy integration.

- Added a statically registered Graph GUI extension. It projects existing graph topology into scene-owned edge overlays, including directed arrows, parallel-edge separation, self-loops, and weight tooltips. Node position remains in the generic graphical data-definition layer; domain membership and topology remain in GraphNetwork/GraphEdge.
- GraphNetwork accepts GraphNode drops only; the drop callback registers the new node with the Network before adding its graphical item. A shared editor action connects exactly two selected GraphNodes. Graph-edge add/remove commands use the scene undo stack and refresh the projection.
- The edge action is disabled when its scene has no undo stack. The initial test fixture omitted the stack and crashed inside Qt `QUndoStack::push`; configuring a real stack and guarding the action fixed the repro.
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS with the Graph extension linked statically.
- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 8` — PASS.
- `QT_QPA_PLATFORM=offscreen ./build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='GuiExtensionManager.*:GraphNetworkGuiExtension.*:EFSMNetworkDiagramPersistence.*:EFSMNetworkDiagramProjection.*:EFSMNetworkEditor.SharedNetworkOpensOneSubmodelTabFromMultipleAdapters:EFSMNetworkEditor.MainWindowSaveCloseAndReopenRestoresAndExecutesEFSM'` — PASS, 29 tests. Covers EFSM persistence/editor regressions, contextual drop acceptance/rejection, Graph node membership, Graph edge rendering, and edge add/remove undo/redo.
- `git diff --check` — PASS for the current D2 increment.

- Graph edge creation now asks explicitly for source and destination because Qt selection order is not an orientation contract. DAG self-loops and cycle-forming edges are rejected before allocating/registering a GraphEdge. Graph edge weight editing is undoable and refreshes the edge projection/tooltip.
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS after source/destination selection, weight editing, and DAG validation.
- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 8` — PASS after the same Graph editor changes.
- `QT_QPA_PLATFORM=offscreen ./build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='GuiExtensionManager.*:GraphNetworkGuiExtension.*:EFSMNetworkDiagramPersistence.*:EFSMNetworkDiagramProjection.*:EFSMNetworkEditor.SharedNetworkOpensOneSubmodelTabFromMultipleAdapters:EFSMNetworkEditor.MainWindowSaveCloseAndReopenRestoresAndExecutesEFSM'` — PASS, 30 tests, including DAG cycle rejection without orphan GraphEdge creation and graph edge add/remove/weight undo/redo.
- `git diff --check` — PASS for this Graph editor increment.

- Source/destination selection is now explicit in the edge creation dialog; the orientation no longer depends on Qt's scene selection ordering. DAG cycle validation is exercised through the real editor action and confirms no extra GraphEdge is allocated/registered on rejection.
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS (up to date after the final D2 orientation/cycle checks).
- Focused integration filter covering `GuiExtensionManager`, `GraphNetworkGuiExtension`, EFSM diagram persistence/projection and shared/reopened EFSM execution — PASS, 30 tests.
- `git diff --check` — PASS after explicit orientation and DAG cycle regression.

- Began D3 with a statically registered MarkovChain GUI extension. MarkovState palette drops become network members; the projection renders initial/current-state markers and probability-labelled directed transition overlays. The create-transition action requires explicit endpoints/probability and uses an owning detach/restore command so undo/redo preserves transition identity and ordinal.
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS with the Markov editor linked into the application.
- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 8` — PASS with Markov GUI coverage.
- `QT_QPA_PLATFORM=offscreen ./build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='MarkovChainNetworkGuiExtension.*:GraphNetworkGuiExtension.*:GuiExtensionManager.*'` — PASS, 10 tests; covers Graph extension regressions, Markov state drop, Markov projection and transition create/undo/redo.
- `cmake --build build/tests-unit --target genesys_test_markov_chain_network -j 8` and `./build/tests-unit/source/tests/genesys_test_markov_chain_network` — PASS, 13 tests, including detach/restore ownership and ordinal.
- `git diff --check` — PASS for the Markov editor increment.
- Extended the Markov editor with selectable probability-labelled transitions, probability edit and removal commands, plus an undoable initial-state command. Updating the initial state now keeps exactly one MarkovState flagged as initial.
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS after the expanded Markov actions and initial-state invariant.
- `QT_QPA_PLATFORM=offscreen ./build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='MarkovChainNetworkGuiExtension.*:GraphNetworkGuiExtension.*:GuiExtensionManager.*'` — PASS, 10 tests; Markov coverage includes add/edit/remove transition, initial-state selection, drops, and undo/redo.
- `./build/tests-unit/source/tests/genesys_test_markov_chain_network` — PASS, 14 tests; includes transition detach/restore ordering and exactly-one-initial-state invariant.
- `git diff --check` — PASS after Markov edit/removal controls.

- Began D4 with a statically registered CPN editor. Place and CPNTransition drops become formal network members; the projection draws the bipartite arcs, place marking counts/colors, and enabled/disabled transition status from the actual CPN domain.
- CPN GUI operations now create bipartite arcs with explicit direction and fixed inscription, edit/remove arcs, edit initial colored marking and transition guard/priority, and fire an enabled transition through `ColoredPetriNetNetwork::fire()`. Firing undo/redo restores captured place markings and reprojects the actual state.
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS with the CPN extension linked statically.
- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 8` — PASS with CPN editor coverage.
- `QT_QPA_PLATFORM=offscreen ./build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='ColoredPetriNetGuiExtension.*'` — PASS, 3 tests; covers allowed place/transition drops, arc authoring/edit/remove undo/redo, transition guard/priority editing, initial marking edits, and firing/marking undo/redo.
- `git diff --check` — PASS for the current CPN GUI increment.

- Fixed `.gui` Graph reopen: Network-owned GraphNode/GraphEdge references are now registered as attached data definitions, preventing `Model::check()` from pruning them. The round-trip regression verifies topology, canonical data-manager registration and stable node positions after reload.
- Applied the same model-lifetime association to Markov states and CPN places, transitions and arcs; existing EFSM state association remains in place.
- Added runtime event projection refresh through `GuiExtensionPluginCatalog`; updates are queued to the live scene's QObject thread, with a regression covering off-thread invocation and scene lifetime guarding.
- Added runtime highlights for EFSM's current state/last transition, Markov's sampled transition, and the CPN's last fired transition. CPN undo/redo restores both marking and last-fired state.
- Added Graph BFS/DFS structural analysis as a separate visual overlay, using the domain traversal results and leaving the Network activation count unchanged.
- Kernel targeted CTest: `GraphNetworkTest|MarkovChainNetworkTest|ColoredPetriNetNetworkTest|EFSMNetworkTest` — PASS, 61/61 tests, including model checking after Markov/CPN load.
- GUI focused tests: Graph BFS/DFS, generic `.gui` Graph geometry round-trip, projection thread dispatch — PASS, 3/3. Earlier focused extension tests — PASS, 8/8 before the added associations.
- Integrated GUI regression filter `GuiExtensionManager.*:GraphNetworkGuiExtension.*:MarkovChainNetworkGuiExtension.*:ColoredPetriNetGuiExtension.*:EFSMNetworkDiagramProjection.*:EFSMNetworkDiagramPersistence.*:EFSMNetworkEditor.*` — PASS, 43/43.
- `cmake --build --preset gui-app --target genesys_gui -j8` — PASS.
- Rebuilt the GUI regression target against the latest Network lifetime changes — PASS.
- `git diff --check` — PASS for the current runtime/ownership increment.
- Markov GUI round-trip regression — PASS: `.gui` save/reopen restores the two-state transition topology, initial state, transition probability, canonical model objects and both node positions.
- CPN GUI round-trip regression — PASS: `.gui` save/reopen restores places, transition, both bipartite arcs, initial marking, canonical data objects and node positions.
- Focused current GUI round-trip filter for Graph, Markov and CPN — PASS, 3/3.
- Repeated integrated GUI regression filter after adding Markov/CPN round-trip coverage — PASS, 45/45 across 7 suites.
- Graph node deletion test — PASS: connected topology is removed and undo/redo preserves GraphNode identity, incident GraphEdge ordinals and graphical position.
- CPN runtime projection now highlights all arcs incident to the last actually fired transition; its GUI test asserts the two firing arcs and transition highlight.
- `cmake --build --preset gui-app --target genesys_gui -j8` — PASS after Graph deletion and CPN firing-path projection.
- Focused Graph, Markov and CPN extension tests — PASS, 14/14.
- Markov state removal regression — PASS: undo restores both incident transition identities/order, initial/current state, and node geometry across redo.
- CPN transition removal regression — PASS: undo restores both arc identities/order, initial marking, live marking and last-fired transition.
- Updated Graph/Markov/CPN GUI extension filter — PASS, 16/16.
- `GraphNetworkTest|MarkovChainNetworkTest|ColoredPetriNetNetworkTest|EFSMNetworkTest` kernel filter — PASS, 61/61 after adding ordinal restore APIs.
- Integrated GUI filter across extensions, persistence and EFSM host/editor — PASS, 48/48 across 7 suites.
- Network palette drops now enter the scene undo stack through a formalism-neutral command. Undo removes the extension membership and detaches the existing graphical object; redo restores the same domain and graphical identities. Graph drop undo/redo regression — PASS, 2/2 with the existing drop test; `git diff --check` — PASS. The command is generic and dispatches membership through the registered editor contribution.
- Markov editor now offers `Validate Markov Network Probabilities`; its diagnostic is produced by the existing mathematical `ModelDataDefinition::Check` contract. Headless GUI regression creates an incomplete stochastic row and confirms the UI reports that outgoing probabilities must sum to 1.0 — PASS, 1/1.
- CPN editor now offers `Validate Colored Petri Net`, surfacing diagnostics from its existing mathematical guard, topology and inscription validation. A headless regression confirms malformed transition guards are reported — PASS, 1/1; Markov validation regression remains PASS, 1/1.
- Tightened network drop undo/redo to unregister/re-register the created data definition through `Model`, matching existing deletion semantics and preventing undone nodes from remaining as model-managed orphans. The drop regression now asserts data-manager identity is absent after undo and restored after redo — PASS, 1/1.
- Extended the shared adapter navigation regression to select a graphical EFSM Network data definition directly and open/reuse the same Network tab — PASS, 1/1.
- Added a persistence-level damaged typed-reference regression: after changing the saved model ID while retaining the concrete type/name, load does not bind to another object and validation reports `missing or mismatched Network` — PASS, 1/1.
- Integrated GUI filter `GuiExtensionManager.*:GraphNetworkGuiExtension.*:MarkovChainNetworkGuiExtension.*:ColoredPetriNetGuiExtension.*:EFSMNetworkDiagramProjection.*:EFSMNetworkDiagramPersistence.*:EFSMNetworkEditor.*:ModalNetworkReferencePersistence.*` — PASS, 55/55 across 8 suites at HEAD `5a05fcf2e` (includes the A1 graphical Network opening change).
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS at current source state, including the generic network-drop undo and Markov/CPN validation actions.
- C6 disabled-extension integration: config disables `modal.markov.gui`; the static catalog omits its contribution, `.gui` node geometry survives load/save/reload, and loaded Markov DTMC activation still takes its actual self-transition — PASS, 1/1.
- D1 contextual palette and submodel regression filter `GuiExtensionManager.*:EFSMNetworkEditor.SharedNetworkOpensOneSubmodelTabFromMultipleAdapters` — PASS, 8/8; covers per-formalism palette contents, model-root palette restoration, process component acceptance in ordinary Submodel, process component rejection in Network, and shared Network tab switching.
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS after contextual palette integration.

### D5 GUI-thread refresh safety increment

- `_actualizeGraphicalModel` marshals off-thread event projection refresh onto
  the MainWindow Qt thread. The queued callback guards the MainWindow and looks
  up the active scene only when it runs, so a closed window/scene cannot be
  dereferenced during shutdown. GUI-thread event handling retains component
  selection behavior.
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS.
- `build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='GuiExtensionManager.NetworkProjectionRefreshIsQueuedToTheSceneThread:GraphNetworkGuiExtension.*:ColoredPetriNetGuiExtension.*'` — PASS, 15/15.
- `build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='MarkovChainNetworkGuiExtension.*'` — PASS, 5/5. Complete simulation lifecycle and integrated GUI validation remain pending.

### D2 Graph editor completion evidence

- Added a GUI regression that compares GraphNetwork and DirectedGraphNetwork
  projections and confirms only the directed family renders an arrow. Existing
  DAG editor coverage rejects a cycle before allocating a GraphEdge.
- Added a Property Editor regression confirming GraphNode exposes its inherited
  `Name` control. The existing edge weight action is
  undoable; node position remains draggable and survives `.gui` round-trip.
- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 8` — PASS.
- `QT_QPA_PLATFORM=offscreen build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='GraphNetworkGuiExtension.*:GraphNodePropertyEditor.*'` — PASS, 10/10.
- `cmake --build build/tests-unit --target genesys_test_graph_network -j 8` and `build/tests-unit/source/tests/genesys_test_graph_network` — PASS, 18/18.
- Structural graph semantics remain unchanged; entity routing is not introduced.

### D3/D4 focused completion gate

- `ctest --test-dir build/tests-unit --output-on-failure -R 'MarkovChainNetworkTest|ColoredPetriNetNetworkTest'` — PASS, 27/27. Includes stochastic-row checks, seeded Markov replication reproducibility, CPN bipartite topology, firing, reset and persistence.
- `QT_QPA_PLATFORM=offscreen build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='GuiExtensionManager.NetworkProjectionRefreshIsQueuedToTheSceneThread:GraphNetworkGuiExtension.*:MarkovChainNetworkGuiExtension.*:ColoredPetriNetGuiExtension.*:GraphNodePropertyEditor.*'` — PASS, 22/22. Includes Markov probability/initial-state editing and CPN marking/arc/guard editing, firing, undo/redo and graphical persistence.
- Both formalism editors pass focused GUI and kernel gates; whole-application lifecycle validation remains in D5/final integration.

### D5 simulation lifecycle and close-path increment

- Added observable read-only animation lifecycle state and a controller regression
  covering simulation start, pause, resume, end, and a second start/end cycle.
  The test confirms start clears stale pause state, enables animation, and end
  stops animation, clears pause, and resets the model-check latch.
- Added a worker-thread projection test that destroys the Network scene before
  the queued refresh runs; the guarded scene expires and the pending refresh is
  safely discarded.
- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 8` — PASS.
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS after the animation lifecycle API update.
- `QT_QPA_PLATFORM=offscreen build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='SimulationEventController.*:GuiExtensionManager.NetworkProjectionRefreshIsQueuedToTheSceneThread:GuiExtensionManager.QueuedNetworkProjectionIsDiscardedWhenSceneCloses'` — PASS, 3/3.
- Combined focused Graph/Markov/CPN, Property Editor, lifecycle and close-path filter — PASS, 24/24.
- A real process simulation now runs through pause, one-event step, resume and two replications while the GUI controller handlers record event refreshes and replication rows.
- `QT_QPA_PLATFORM=offscreen build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='SimulationEventController.RealSimulationCanPauseStepResumeAndRefreshReplications'` — PASS, 1/1.
- Combined focused formalism, lifecycle, property-editor and scene-close filter — PASS, 25/25.
- D5 focused lifecycle validation is complete; the integration checks immediately below close out the full build and test validation for this branch.

### Integration validation checkpoint

- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS after the `.gui` header parser fix.
- `ctest --test-dir build/tests-unit --output-on-failure -j 8` — PASS, 2,109/2,109 executed; four configured-disabled tests did not run. A prior run exposed the `.gui` view-state regression below; the full suite passed after its fix.
- `cmake --build --preset tests-kernel-unit -j 8` — PASS on final source.
- `ctest --test-dir build/tests-kernel-unit --output-on-failure -j 8` — PASS, 2,111/2,111 executed; four configured-disabled tests did not run.
- `cmake --build --preset tests-smoke -j 8` — PASS. `ctest --test-dir build/tests-smoke --output-on-failure -j 8 -E '_NOT_BUILT'` — PASS, 3/3 runnable smoke tests. The preset's other 48 registered entries are `_NOT_BUILT` placeholders and were excluded after building its declared smoke targets.
- `GuiGmddLayout.SerializerRoundTripRestoresViewStateGeometriesAndGroups` failed in both the first full unit run and isolated reproduction: the `.gui` reader treated `0 GUI`/`0 Show` header directives as an unknown section, leaving visibility flags and zoom unchanged. Header directives are now retained as header data; isolated regression and the subsequent full `tests-unit` suite pass.
- `git diff --check` — PASS before final commit. Final branch commit/push verification is recorded at closeout.

### EFSM transition context-menu follow-up

- User testing found `Create EFSM Transition` only in the Model menu, not the graphical context menu described in the interaction flow.
- Registered a formalism-specific context-menu contribution that is visible when one or two EFSM states are selected and invokes the existing undoable transition creation handler.
- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 8` — PASS.
- `QT_QPA_PLATFORM=offscreen build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='EFSMNetworkEditor.*'` — PASS, 8/8, including the new context-menu visibility regression.

### EFSM transition selection crash follow-up

- A user-reported SIGSEGV on transition click mapped to `ModelGraphicsScene::mouseReleaseEvent`: path bounds were compared to item position, incorrectly enqueuing a Move command for a click. The command refreshed the EFSM projection, deleting the selected overlay while release handling still referenced it.
- Movement detection now compares actual item positions with the press snapshot; legitimate FSMState movement still refreshes transition geometry and remains undoable.
- `cmake --build build/tests-unit --target genesys_test_gui_gmdd_layout -j 8` — PASS.
- EFSM transition click regression plus existing state-move/undo regression — PASS, 2/2.
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS.
- `QT_QPA_PLATFORM=offscreen build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='EFSMNetworkEditor.*:EFSMNetworkDiagramProjection.*'` — PASS, 11 tests.

### EFSM transition contextual editing follow-up

- Added `Edit EFSM Transition...` to the graphical context menu when exactly one transition is selected; it invokes the existing properties dialog and undoable property command.
- Regression test opens that context action's dialog, changes the guard expression and verifies the model value. The existing MainWindow action test continues to verify undo.
- `QT_QPA_PLATFORM=offscreen build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='EFSMNetworkEditor.CreateTransitionAppearsInContextMenuForSelectedStates:EFSMNetworkEditor.SelectedTransitionCanBeEditedAndUndoneThroughTheMainWindowAction'` — PASS, 2/2.

### Integration authorization and manual closeout

- On 2026-11-13 the maintainer explicitly authorized a PR and merge of this branch into `WorkInProgress`, superseding the plan's earlier no-automatic-merge boundary for this candidate only. Merge remains subject to final-head PR checks and repository gates.
- Updated the user EFSM guide for Property Editor association, generic `Open Network`, state/transition creation, and contextual transition editing. Updated the Developer Manual's Modal Network architecture section to remove stale branch-status claims and describe current extension/editor boundaries.
- `cd docs/developers/ManualGenESyS_subrepo && ./make.sh` — PASS; regenerated `docs/ManualGenESyS.pdf` (398 pages).
- `./validate_figure_specs.sh` — PASS. No undefined-reference/fatal LaTeX diagnostics were found; Developer Manual pages 250–251 were visually inspected for the updated section.


### PR #577 integration closeout (2026-11-13)

- PR #577 merged into `WorkInProgress` with merge commit `1811a7ae6834af5d26f4c9c4de0cec8252bd88eb`; final candidate head was `b88984d7fd6d957ba87cff6f9382f9b1df73d427`.
- All 19 final-head GitHub checks passed. This includes `tests-unit` (2,109 executed, four configured-disabled), `tests-kernel-unit` (2,111 executed, four configured-disabled), runnable smoke (3/3), GUI application build, focused GUI tests, manual build/governance, Debian package build with Lintian, and install/reinstall/remove/purge lifecycle validation.
- `origin/WorkInProgress` verified at merge commit `1811a7ae6834af5d26f4c9c4de0cec8252bd88eb`; `origin/WiP20261112/modal_model_gui` remains at candidate `b88984d7fd6d957ba87cff6f9382f9b1df73d427` for continued development.
- Merge mode: merge commit; no squash/rebase; no source-branch deletion.

## AUTO-MODAL-006 — Universal Network element editing (2026-11-13)

### EFSM checkpoint implemented on the development branch

- `FSMState` now exposes entry/exit action expressions and final/initial flags as Property Editor controls. Extension membership is checked by object identity within the active EFSM; base MDD controls and state-specific edits use undoable commands.
- The extension contract now supports Network-owned mathematical element properties and relation creation without adding formalism classes to generic GUI code. Double-clicking a valid EFSM state starts a crosshair connection mode; clicking a destination creates the actual owned `EFSMTransition` via the extension command. Self-loops and parallel relations work. Esc, empty-canvas click and context change cancel the mode.
- Selecting an EFSM relation presents Name, read-only source/destination, guard, output, input event, priority, trigger event and probability expression in the Property Editor. Changes target the real transition and use undo/redo. The redundant EFSM state/transition edit commands and their dialog implementations were removed.
- `.gen` persistence omitted `Trigger Event`; the field is now read/written. Graphical projection refresh is linked to the undo stack so relation creation/edit/undo/redo stays synchronized with the diagram.
- Gesture regression uses three states and verifies A→B, B→C and C→C, identity/topology, undo/redo projection updates, Esc and canvas cancellation. Additional tests cover state editability by Network membership identity, transition Property Editor fields, no legacy actions, context menu behavior, and GUI/model round-trip.
- `cmake --build --preset tests-unit --target genesys_test_gui_gmdd_layout -j 4` — PASS.
- Focused filter covering state selection/editing, EFSM gesture, transition Property Editor, context/action integration, main-window save/reopen and parallel-transition layout — PASS, 9/9.
- `git diff --check` — PASS before checkpoint commit.

### Remaining work

- EFSM user-visible checkpoint: commit `762747215`, published on `origin/WiP20261112/modal_model_gui`.
- GraphNetwork/DirectedGraph/DAG now use the shared source-selection and relation-creation API. GraphNodes expose base properties through the active network editor; selected GraphEdges expose Name and Weight in the Property Editor, with endpoints read-only. The old edge weight dialog action was removed.
- DAG rejects self-loops and cycle-forming edges before allocating a GraphEdge. Directedness remains assigned by the concrete GraphNetwork specialization.
- Graph-focused build — PASS. `GraphNetworkGuiExtension.*` — PASS, 9/9, including gesture-driven directed edge creation, Property Editor descriptors/weight edit undo-redo, DAG cycle/self-loop rejection without allocation, existing projection, persistence geometry and node-removal regressions.
- Next: complete MarkovChainNetwork and ColoredPetriNetNetwork integration, remove redundant relation edit dialogs when equivalent Property Editor coverage is in place, and validate persistence/undo-redo for those formalism-owned relations. AUTO-MODAL-006 remains running.
