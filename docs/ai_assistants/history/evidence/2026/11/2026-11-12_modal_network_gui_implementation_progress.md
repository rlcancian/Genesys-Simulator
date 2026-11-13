---
document_type: implementation-progress-evidence
tracks: HUM-MODAL-003,AUTO-MODAL-005
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
| G0 | In progress | Verified clean branch base, CMake/Ninja presets, GUI target, GUI regression target, Network hierarchy and current menu/property-editor code. A complete fixture matrix and baseline suite are still pending. |
| A1 | In progress | Removed EFSM-specific create/select menu actions and handlers; generic Open Network labeling and reference gating use the shared submodel tab/context path. Adapter-open and null-reference tests pass. A direct graphical Network data-definition open test remains. |
| A2 | Pass (focused) | `Network` choices enumerate registered `DefaultNetwork` subtypes and resolve by concrete class plus model ID. Focused test selects EFSM, Graph, Directed Graph, DAG, Markov and CPN objects with the same name. |
| A3 | Pass (focused) | Plugin metadata records direct data-definition base types; PluginManager discovers transitive compatible factories. Property Editor creates and attaches an explicitly selected subtype and offers no inert base type. |
| A4 | Pass (focused) | Property Editor now offers explicit reference removal and permits reattaching an existing Network. A detached Network remains model-managed. The focused property and EFSM GUI tests pass. |
| A5 | Pass (focused) | Property Editor choices expose concrete type/name/ID; association editing and `Network Details` provide separate mathematical properties/binding rows, while Open Network remains contextual. Graph and DAG disclose their structural/no-activation semantics, and the generic base Network is labeled inert. Focused GUI coverage exercises typed creation, selection across all six concrete families, binding editing/undo, and a Create→ModalModelDefault→Dispose connection fixture for each family. |
| B1 | Pass (focused) | Identity audit is covered by six concrete subtype choices with duplicate display names, exact identity selection, unique legacy name resolution and ambiguous legacy name rejection. |
| B2 | In progress | ModalModelDefault persists concrete Network type and model ID alongside the legacy name. Exact lookup validates type, ID and name; invalid, missing and ambiguous legacy references fail closed with explicit diagnostics. Exact typed `.gen` round-trip passes; damaged-reference and broader compatibility coverage remain. |
| B3 | Pass (focused) | Bindings remap by stable port names; new ports receive defaults; port-name metadata persists while detached. Unsafe custom-binding/connection changes reject transactionally. Property Editor confirms schema-changing replacements before mutation; cancellation preserves the old association. Binding rows are exposed in the Property Editor; the EFSM bindings dialog/action is removed. Undo/redo covers binding edits and Network reference changes, including graphical port synchronization. |
| B4 | Pass (focused) | ModelDataManager remains the Network owner; adapter detach and replacement do not delete an unshared or shared Network. Existing GUI coverage also proves reference removal leaves the Network selectable for reattachment. Focused adapter suite: 14/14 PASS. Broader close/delete lifetime audit remains in final validation. |
| B5 | Pass | Reviewed ModalModel implementation/header placement and Network data-definition headers/implementations. The existing split keeps GUI-independent mathematical implementations in plugin translation units and data contracts under `data/ModalModel`; no source relocation is justified. GUI app build passes. |
| C1–C6 | Not started | EFSM GUI isolation and extension registration. |
| D1–D5 | Not started | Context-specific editors, formalism topology, persistence, undo/redo and animation. |

## Executed changes

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

- `c3f97a9e30866e5ae3b8cc670cce867c96a0d823` — `modal-gui(A1): replace EFSM menu actions with Open Network` (pushed to `origin/WiP20261112/modal_model_gui`).
- `67ee70abfc066da4916db55953616ada985aa15f` — `modal-gui(A2): select typed network references by identity` (pushed to `origin/WiP20261112/modal_model_gui`).
- `e9feaae618581bb032d6f91251f1191897f99500` — `modal-gui(A3): create explicitly typed network references` (pushed to `origin/WiP20261112/modal_model_gui`).
- `f08fe0e0f` — `modal-gui(A4,B2): preserve typed network references` (pushed to `origin/WiP20261112/modal_model_gui`).
- `31296e77f` — `modal-gui(B2): diagnose ambiguous legacy network references` (pushed to `origin/WiP20261112/modal_model_gui`).
- B3 replacement, binding-schema persistence, graphical port synchronization, Property Editor binding edits, cancellation/acceptance confirmation, and undo/redo are validated for the cases listed above and pending commit.
- B4 focused ownership evidence: detach/replacement does not remove Network objects from their model manager, including when a second adapter still references the shared object. Full destruction/scene-close audit remains a final-validation item.
