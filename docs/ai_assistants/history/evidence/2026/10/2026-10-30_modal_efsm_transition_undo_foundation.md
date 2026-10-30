---
document_type: evidence
authority: informative
owner: project-maintainer
date: 2026-10-30
status: partial-validation
tracks: HUM-MODAL-002,AUTO-MODAL-003,AUTO-MODAL-004
---

# EFSM editor continuation checkpoint — safe transition detach/restore

## Scope and source

- Repository: `rlcancian/Genesys-Simulator`
- Branch: `WiP20261029/modalmodel-architecture`
- Source commit: `cfea4abc52189dc5bd482c509df33a55d33f9074`
- PR: #551, draft, not merged.
- Accepted progress remains 3/8 (37.5%). This checkpoint does not claim G1.1,
  G1.2, G2, G3/G4 or G6 acceptance.

## Current evidence

- The existing scene projection test builds real state and transition items in
  the shared `ModelGraphicsScene`; it passed with the diagram persistence and
  adapter/network regressions.
- Focused test command:
  `LC_ALL=C LANG=C QT_QPA_PLATFORM=offscreen ctest --preset tests-unit -R 'EFSMNetworkTest|GuiSubmodelLevel|EFSMNetworkDiagramPersistence|EFSMNetworkDiagramProjection|ModalModelDefaultNetworkTest' --output-on-failure`
- Result: 48/48 passed.
- `cmake --build --preset gui-app --parallel 4` completed successfully with
  no work pending at this source checkpoint.
- A bounded Xvfb launch kept `genesys-gui` alive for four seconds and printed
  its normal startup banner. This is startup-only evidence: no user interaction
  or live Network-tab workflow was demonstrated in this command.
- GitHub Actions for source SHA `cfea4abc` were in progress when recorded;
  no result is claimed here.

## Ownership finding and change

`EFSMNetwork::removeTransition()` previously removed a transition from the
network and its source state's outgoing list, then destroyed it. A GUI undo
command could not safely keep and reuse that pointer. This commit adds
`detachTransition()`, returning a `unique_ptr` and the former sequence ordinal,
plus `restoreTransition()`, which validates both endpoints belong to the
Network and restores the transition at that ordinal. Ownership transfers back
only after successful restoration. The existing destructive
`removeTransition()` retains its behavior through the detach operation.

Two kernel tests demonstrate ordinal-preserving detach/restore and rejection of
an endpoint outside the Network without taking ownership. No EFSM firing,
guard, output, priority, or persistence semantics were changed.

## Remaining work

- Integrate the safe API into GUI undo/redo commands and test mixed sequences.
- Complete property editing, deletion, lifecycle safety and end-to-end GUI
  validation.
- Continue the remaining approved milestones without changing the fixed
  eight-milestone plan.
