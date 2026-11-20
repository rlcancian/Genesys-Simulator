---
document_type: evidence
authority: informative
owner: project-maintainer
date: 2026-10-30
status: partial-validation
tracks: HUM-MODAL-002,AUTO-MODAL-003,AUTO-MODAL-004
---

# EFSM transition editing — create/delete undo and redo

## Source checkpoint

- Branch: `WiP20261029/modalmodel-architecture`
- Commit: `c986e868b5998694419bff6c7a11f6b8d59c900f`
- PR #551 remains a draft and is not merged.
- Accepted milestone progress remains 3/8 (37.5%). This is a G3/G4 partial
  implementation checkpoint, not milestone acceptance.

## Implementation

- `EFSMTransitionGraphicsItem` is selectable with a wider invisible hit target.
- The existing Edit/Delete action recognizes selected EFSM relation items and
  pushes undo commands against the EFSM model; no process `Connection` is
  created.
- Transition creation from the existing Model menu now uses the same undo
  command. Undo/redo synchronizes the shared scene projection.
- `EFSMTransitionUndoCommand` owns a transition with `unique_ptr` only while it
  is detached. Attached transitions remain owned by `EFSMNetwork`; restoration
  validates endpoints and preserves the sequence ordinal.
- Tests cover GUI controller deletion of parallel transitions, projection
  refresh, repeated undo/redo, semantic property preservation, and creation
  undo/redo.

## Executed validation

- Focused EFSM, adapter, persistence and GUI scene suite: 51/51 passed before
  the final multi-selection assertion was added. The three final
  `EFSMNetworkUndo.*` tests passed afterward.
- `cmake --build --preset gui-app --parallel 4`: passed after source changes.
- ASan/UBSan target was configured in `build/efsm-asan` using GCC 13.3.0 and
  `-fsanitize=address,undefined -fno-omit-frame-pointer`. All three
  `EFSMNetworkUndo.*` tests passed with `ASAN_OPTIONS=detect_leaks=0` and
  `UBSAN_OPTIONS=halt_on_error=1`; no invalid access or UB was reported.
- With LeakSanitizer enabled, the same tests ran but process exit was 23:
  4,934 bytes in 162 allocations traced to generic simulator/plugin
  initialization (`PluginManager::autoInsertPlugins()` and
  `Buffer::LoadInstance`), not to `EFSMTransitionUndoCommand` or
  `EFSMNetwork::detachTransition()`. This LSan result is not a pass and does
  not establish leak freedom.
- The regular CMake GUI executable rebuilt successfully. No full unit,
  kernel-unit or smoke baseline was run for this checkpoint.

## Remaining scope

State creation/deletion and properties, transition guard/action/name editing,
editing lifecycle edge cases, the full user workflow, complete baselines and
manual updates remain pending. Do not describe the EFSM editor as complete in
the User Manual until those workflows are validated.
