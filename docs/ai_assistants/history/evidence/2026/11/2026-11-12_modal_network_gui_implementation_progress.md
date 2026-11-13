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
| A5 | Not started | Remaining acceptance items in the plan. |
| B1 | Pass (focused) | Identity audit is covered by six concrete subtype choices with duplicate display names, exact identity selection, unique legacy name resolution and ambiguous legacy name rejection. |
| B2 | In progress | ModalModelDefault persists concrete Network type and model ID alongside the legacy name. Exact lookup validates type, ID and name; invalid, missing and ambiguous legacy references fail closed with explicit diagnostics. Exact typed `.gen` round-trip passes; damaged-reference and broader compatibility coverage remain. |
| B3–B5 | Not started | Safe replacement reconciliation, lifetime audit and structural placement gates. |
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
- `cmake --build --preset gui-app --target genesys_gui -j 8` — PASS after A4/B2.
- Full GUI regression suite — UNKNOWN/not run. Existing tests for the removed
  create/select actions must be migrated to the Property Editor in A2/A3.

## Commits

- `c3f97a9e30866e5ae3b8cc670cce867c96a0d823` — `modal-gui(A1): replace EFSM menu actions with Open Network` (pushed to `origin/WiP20261112/modal_model_gui`).
- `67ee70abfc066da4916db55953616ada985aa15f` — `modal-gui(A2): select typed network references by identity` (pushed to `origin/WiP20261112/modal_model_gui`).
- `e9feaae618581bb032d6f91251f1191897f99500` — `modal-gui(A3): create explicitly typed network references` (pushed to `origin/WiP20261112/modal_model_gui`).
- `f08fe0e0f` — `modal-gui(A4,B2): preserve typed network references` (pushed to `origin/WiP20261112/modal_model_gui`).
- Legacy identity diagnostics and tests are validated and pending commit.
