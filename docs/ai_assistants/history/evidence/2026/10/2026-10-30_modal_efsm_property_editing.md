---
document_type: evidence
authority: informative
owner: project-maintainer
date: 2026-10-30
status: partial-validation
tracks: HUM-MODAL-002,AUTO-MODAL-003,AUTO-MODAL-004
---

# EFSM transition properties and initial state editing

## Source checkpoint

- Branch: `WiP20261029/modalmodel-architecture`
- Commits: `78a7757a8` and `cb2381b75`
- PR #551 remains a draft and is not merged.
- Accepted progress remains 3/8 (37.5%); G3/G4 is not complete.

## Implementation

- Selecting one EFSM transition enables **Edit EFSM Transition...** in the
  existing Model menu. The dialog edits its name, guard/output expressions,
  input/trigger events, priority and probability expression.
- `EFSMTransitionPropertiesUndoCommand` captures the prior values and applies
  both undo and redo to the same transition pointer while it is still present
  in its `EFSMNetwork`.
- Selecting one state enables **Set Selected EFSM State as Initial**. The
  undo command restores the previous initial-state pointer and the saved
  initial flags. It does not change kernel source or process connections.
- The menu actions require the active non-root Network context and validate
  selected items against the actual domain collections.

## Validation

- `cmake --build --preset tests-unit --parallel 4 --target genesys_test_gui_gmdd_layout`: passed.
- Offscreen `EFSMNetworkUndo.*`: 5/5 passed, including transition property and
  initial-state undo/redo regressions.
- `cmake --build --preset gui-app --parallel 4`: passed.
- No full unit/kernel/smoke run or interactive dialog automation was performed
  for this checkpoint. Syntax/semantic expression validation through a full
  model check is still pending.

## Remaining work

State name/entry/exit action/final-state editing, safe state deletion and its
ownership/lifetime contract, automated interaction with the dialogs, full
regression baselines, manual updates and the integrated G6 workflow remain
open. Do not treat this as G3/G4 acceptance.
