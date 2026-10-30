---
document_type: evidence
authority: informative
owner: project-maintainer
date: 2026-10-30
status: partial-validation
tracks: HUM-MODAL-002,AUTO-MODAL-003,AUTO-MODAL-004
---

# Modal/Network EFSM GUI continuation evidence — 2026-10-30

## Scope and Git state

- Repository: `/home/rafaelcancian/Repositories/TEMP_GENESYS_BRANCH_MODALMODEL/Genesys-Simulator`
- Branch: `WiP20261029/modalmodel-architecture`
- Starting published checkpoint for this continuation: `0ad3feb7d5e6515cc3f86231140e63e6694018ae`.
- Current code/test HEAD: `cecf2ef1a77a0814ce9eef3bbd042ad1c09d7699`.
- Commits published in this continuation: `690f56d2b`, `728bc3b96`, `570110c1e`, `98e193533`, `677a5a108`, `f39729978`, `cecf2ef1a`.
- PR #551 remains open and draft. No merge or push to `WorkInProgress` occurred.
- Worktree was clean after each code checkpoint; this evidence/document update is a separate documentation unit.

## Implemented behavior

- EFSM states and formal transitions project into the existing `ModelGraphicsScene`; no process `Connection` or duplicate mathematical object is created.
- The projection handles empty and single-state networks, multiple states, unnamed parallel transitions, self-loops, initial/final visual distinction, deterministic fallback positions, and persisted state positions.
- Opening the same Network from two `ModalModelDefault` adapters reuses one submodel tab and one Network object. The tested path closes/reopens the tab and opens a newly created empty Network.
- GUI actions create and select Networks. Same-name candidates are labeled with their registered ID to avoid arbitrary selection. Input/output adapter bindings have a dialog and `QUndoCommand`.
- State creation/deletion, state properties and initial-state changes, plus transition creation/deletion/properties, are integrated with undo/redo. Transition creation explicitly lets the user choose source and destination because scene selection order is not a direction contract.
- Intermediate Network construction uses local registration/membership checks; it does not require the entire process model to pass `Model::check()` before editing is complete.
- State movement refreshes EFSM edge geometry through `MoveUndoCommand` on redo and undo.

## Executed validation

- `cmake --build --preset tests-unit --parallel 4 --target genesys_test_gui_gmdd_layout`: passed at current code/test HEAD.
- `QT_QPA_PLATFORM=offscreen build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout --gtest_filter='EFSMNetworkEditor.*:EFSMNetworkUndo.*:EFSMNetworkDiagramProjection.*'`: **18/18 passed** at current code/test HEAD.
- `cmake --build --preset gui-app --parallel 4`: passed at code-equivalent HEAD `f39729978`; no production-code changes followed that build (the subsequent change adds a projection regression test).
- ASan/UBSan focused command on `EFSMNetworkUndo.*`, `EFSMNetworkDiagramProjection.MovingAStateRefreshesItsFormalTransitionGeometry`, and `EFSMNetworkEditor.AdapterBindingsCanBeEditedAndUndoneFromTheProcessCanvas`: **11/11 passed**, with `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` and `UBSAN_OPTIONS=halt_on_error=1`, at code-equivalent HEAD `f39729978`.
- A prior focused LSan run reported 4,934 bytes in 162 allocations rooted in generic plugin initialization (`PluginManager`/`Buffer`). This is recorded as an existing process-level limitation; it does not establish global leak freedom.
- `git diff --check` passed before each published code commit.
- GitHub checks on the current HEAD were pending when inspected; PR #551 was `OPEN`, `isDraft=true`, `mergeable=MERGEABLE`.
- `python3 scripts/validate-ai-docs.py`: passed after updating the AI status, plan, backlog and evidence index.
- In `docs/developers/ManualGenESyS_subrepo`, `./validate_figure_specs.sh` passed and `./make.sh` completed with exit code 0. The regenerated `docs/ManualGenESyS.pdf` has 388 pages (8,075,072 bytes); affected Developer Manual section 26.7, pages 242–243, was extracted and inspected. `xdvipdfmx` emitted annotation-out-of-page-boundary warnings during the full build; they were non-fatal and the affected section rendered.

## Milestone status and pending gates

- Accepted baseline remains G0.1, G0.1b and G0.2: **3/8 = 37.5%**.
- G1.1 projection is implemented and focused-validated. Full baseline/regression validation and final manual UI interaction evidence remain.
- G1.2 has main-window tab reuse, sharing, close/reopen, and empty-network tests. Root-model teardown, model switching and the no-Network interaction still need direct lifecycle coverage.
- G2 has real GUI create/select/associate and binding-edit flows. A complete GUI save/close/reopen plus runtime scenario remains.
- G3/G4 editing and undo foundations are active. Full mixed-command GUI sequences, final expression-validation policy, and complete save/reload scenarios remain.
- G6 has not started. Full unit, kernel-unit, smoke, GUI, sanitizer, manual/PDF, governance and final CI checks remain before delivery/merge.
- Known constraints: transition presentation identity is the persisted ordinal, valid for the same `.gen` transition sequence; topology reordering can invalidate layout identity. `.gen` remains mathematical authority. This implementation adds no mathematical transition IDs.

## Next actions

1. Add direct tests for no-Network open behavior, root-model closure/model switching, and state/transition GUI deletion/undo in the actual tab context.
2. Verify edited bindings and topology in an integrated `.gen`/`.gui` save/reopen flow.
3. Continue G3/G4 interaction and mixed undo/redo coverage; run focused ownership sanitizers.
4. Run milestone checkpoints and final CMake/Ninja/CTest baselines; update User/Developer Manual and regenerate the official PDF.
5. Recheck the final SHA's CI, document limitations and leave PR #551 draft until all acceptance gates are satisfied.
