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
- Current code/test HEAD: `960b666c9c601d129a55a58682f163e917288a8c`.
- Commits published in this continuation: `690f56d2b`, `728bc3b96`, `570110c1e`, `98e193533`, `677a5a108`, `f39729978`, `cecf2ef1a`, `8ea18a10f`, `712ba9d3b`, `350040aa4`, `9549ac226`, `1c80d6aaa`, `85aad6801`, `960b666c9`.
- PR #551 remains open and draft. No merge or push to `WorkInProgress` occurred.
- Worktree was clean after each code checkpoint; this evidence/document update is a separate documentation unit.

## Implemented behavior

- EFSM states and formal transitions project into the existing `ModelGraphicsScene`; no process `Connection` or duplicate mathematical object is created.
- The projection handles empty and single-state networks, multiple states, unnamed parallel transitions, self-loops, initial/final visual distinction, deterministic fallback positions, and persisted state positions.
- Opening the same Network from two `ModalModelDefault` adapters reuses one submodel tab and one Network object. The tested path closes/reopens the tab and opens a newly created empty Network.
- An adapter without a Network cannot enable or open the `Open Submodel` action; the regression passed at `8ea18a10f`.
- Switching to a second root model and back to the first model's open Network tab preserves the original Network object and scene context; the regression passed at `350040aa4`.
- Closing the original root model while its Network tab is open removes both views and that model from `ModelManager`, while preserving the other open model; this regression passed at `1c80d6aaa`.
- GUI actions create and select Networks. Same-name candidates are labeled with their registered ID to avoid arbitrary selection. Input/output adapter bindings have a dialog and `QUndoCommand`.
- State creation/deletion, state properties and initial-state changes, plus transition creation/deletion/properties, are integrated with undo/redo. Transition creation explicitly lets the user choose source and destination because scene selection order is not a direction contract.
- Intermediate Network construction uses local registration/membership checks; it does not require the entire process model to pass `Model::check()` before editing is complete.
- State movement refreshes EFSM edge geometry through `MoveUndoCommand` on redo and undo.
- The production save path captures positions from all open Network tabs belonging to the root model; tab closure captures before destroying the scene. Cached layouts are scoped to the root model and Network ID. Opening a Network after load applies only records with matching Network/state identities and finite coordinates, then recomputes transition paths.
- The audit found that the previous serializer cache was never populated from real GUI scenes and that cached positions were not applied on opening a Network. Both production-path gaps are fixed in `960b666c9`.

## Executed validation

- Focused `EFSMNetworkDiagramPersistence.*:EFSMNetworkEditor.*:EFSMNetworkUndo.*:EFSMNetworkDiagramProjection.*`: **36/36 passed** at `960b666c9`.
- At `960b666c9`, `tests-unit` configure/build/CTest passed: **1,962 passed, 4 disabled, 0 failed** (1,966 registered).
- At `960b666c9`, `tests-kernel-unit` configure/build/CTest passed with the same **1,962 passed, 4 disabled, 0 failed**.
- At `960b666c9`, `tests-smoke` configure/build/CTest passed **3/3**; `gui-app` configure/build passed.
- ASan/UBSan with `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` and `UBSAN_OPTIONS=halt_on_error=1`, covering layout save/load/apply, root-model cache separation, root-model closure with an open Network context, and state deletion ownership: **4/4 passed** at `960b666c9`. Leak detection was disabled because the prior LSan run reported allocations from generic plugin initialization.
- ASan/UBSan focused command on `EFSMNetworkUndo.*`, `EFSMNetworkDiagramProjection.MovingAStateRefreshesItsFormalTransitionGeometry`, and `EFSMNetworkEditor.AdapterBindingsCanBeEditedAndUndoneFromTheProcessCanvas`: **11/11 passed**, with `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` and `UBSAN_OPTIONS=halt_on_error=1`, at code-equivalent HEAD `f39729978`.
- A prior focused LSan run reported 4,934 bytes in 162 allocations rooted in generic plugin initialization (`PluginManager`/`Buffer`). This is recorded as an existing process-level limitation; it does not establish global leak freedom.
- `git diff --check` passed before each published code commit.
- GitHub checks for `960b666c9` remain to be checked; its workflow was triggered after publication.
- `python3 scripts/validate-ai-docs.py`: passed after updating the AI status, plan, backlog and evidence index.
- In `docs/developers/ManualGenESyS_subrepo`, `./validate_figure_specs.sh` passed and `./make.sh` completed with exit code 0. The regenerated `docs/ManualGenESyS.pdf` has 388 pages (8,075,072 bytes); affected Developer Manual section 26.7, pages 242–243, was extracted and inspected. `xdvipdfmx` emitted annotation-out-of-page-boundary warnings during the full build; they were non-fatal and the affected section rendered.

## Milestone status and pending gates

- Accepted baseline remains G0.1, G0.1b and G0.2: **3/8 = 37.5%**.
- G1.1 projection and layout capture/restoration are implemented and focused-validated. Manual GUI interaction evidence remains.
- G1.2 has main-window tab reuse, sharing, close/reopen, empty-network, no-Network action, switching between root models, and closing a root model while its Network tab is open.
- G2 has real GUI create/select/associate and binding-edit flows. A complete GUI save/close/reopen plus runtime scenario remains.
- G3/G4 editing and undo foundations are active. Full mixed-command GUI sequences, final expression-validation policy, and complete save/reload scenarios remain.
- G6 has not started. A full manual GUI interaction scenario and final CI on the latest SHA remain before delivery/merge.
- Known constraints: transition presentation identity is the persisted ordinal, valid for the same `.gen` transition sequence; topology reordering can invalidate layout identity. `.gen` remains mathematical authority. This implementation adds no mathematical transition IDs.

## Next actions

1. Add GUI deletion/undo coverage in the actual Network tab context beyond the existing command-level regressions.
2. Verify edited bindings and topology through a complete MainWindow `.gen`/`.gui` save/close/reopen interaction.
3. Continue G3/G4 mixed-command undo/redo scenarios and confirm expression validation behavior.
4. Regenerate the official PDF after the current Developer Manual update and recheck governance.
5. Check CI for the latest published SHA and complete the E2E scenario before marking milestones accepted.
