---
document_type: evidence
authority: informative
owner: project-maintainer
date: 2026-10-30
status: partial-validation
tracks: HUM-MODAL-002,AUTO-MODAL-003
---

# Modal/Network GUI checkpoint — G1.1/G1.2/G2 implementation

## State

- Repository: `/home/rafaelcancian/Repositories/TEMP_GENESYS_BRANCH_MODALMODEL/Genesys-Simulator`
- Branch: `WiP20261029/modalmodel-architecture`
- Published HEAD: `0dcb001597035b5cb49c2c2c907409f4310e37d1`
- PR: #551, draft, not merged.
- The accepted progress remains 3/8 (37.5%); the following milestones are not marked accepted by this checkpoint.

## Implemented in this checkpoint

- G1.1 projection: `EFSMNetworkDiagramProjection` reuses the shared
  `ModelGraphicsScene` and projects real `FSMState` graphics plus non-owning
  directed `EFSMTransitionGraphicsItem` paths. Parallel transitions and
  self-loops are separated visually; no process `Connection` is created.
- `GraphicalModelBuilder` and `GraphicalModelSerializer` synchronize the
  projection after rebuilding/restoring state positions.
- G1.2 context navigation: selecting a `ModalModelDefault` resolves its
  attached `DefaultNetwork` and opens the existing submodel tab at the Network
  ID. Two adapters therefore resolve to one Network pointer/context; an empty
  registered Network remains openable.
- G2 bounded GUI actions: create an EFSM Network through the plugin catalog,
  select an existing EFSM Network, attach it through `setNetwork()`, and create
  named/unnamed states and transitions in an active EFSM context.
- The prior G0.2 empty-transition-name fix remains published in
  `5f63fee85e21391efd4dbf6089ffb392dafe8e90`.

## Validation

- Focused EFSM/GUI tests: 30/30 passed, including 16 graphical persistence/
  projection tests and 14 EFSM kernel tests.
- `tests-unit`: 1,940 passed, 4 disabled, 0 failed (1,944 discovered).
- `tests-kernel-unit`: 1,940 passed, 4 disabled, 0 failed (1,944 discovered).
- `tests-smoke`: 3/3 passed.
- `gui-app`: configured and built successfully.
- Manual GUI interaction, sanitizers for the new mutation actions, and full
  end-to-end create/edit/save/reopen automation were not demonstrated here.

## Explicit remaining limits

The formal topology creation actions are not yet a complete editor. Property
editing, deletion of states/transitions, and safe undo/redo for owned EFSM
transitions require commands that preserve the `EFSMNetwork` ownership
contract; `removeTransition()` currently destroys the transition, so a GUI
command must not retain and reuse that pointer after undo. No unsafe pointer
workaround was introduced. This checkpoint must not be represented as G3/G4 or
G6 acceptance.
