---
document_type: evidence
authority: executed-evidence
owner: project-maintainer
recorded_at: 2026-09-20
status: active
scope: modal-network-backend-gate-progress
---

# 2026-09-20 ModalModel / DefaultNetwork backend gate progress

## Environment

- Worktree: `Genesys-ModalNetwork`
- Ubuntu 24.04; g++ 13.3.0; CMake 3.28.3; Ninja 1.11.1; Qt 6.4.2
- Base HEAD at start: `de0fbc1f16030c25595e301623f83bb6dec24380` (PR #535)

## Integrated since start

| PR | Merge | Scope |
|---|---|---|
| #536 | `5494b237` | Canonical `LoadInstance` reuse by name; no re-apply of `_loadInstance` on reuse |
| #537 | `e0a6a73b` | Full Model round-trips for Graph/DTMC/CPN + Modal dispatch-after-load |

## Local validation (Gate C tip / post-#537)

- Focused Modal/Network regex: **75/75**
- `tests-unit`: **1832** passed, 0 failed, 4 disabled (1836 registered)
- `tests-kernel-unit`: **1832** passed, 0 failed, 4 disabled
- `tests-smoke`: **3/3**

## Confirmed findings

1. **EFSM identity bug (Gate B)**: Model file round-trip created 4 `FSMState` entries (expected 2). **Confirmed by execution**, fixed by canonical `LoadInstance` reuse.
2. **Association wipe on reuse**: re-applying `_loadInstance` cleared `GraphEdge`/`CPNArc` endpoints. **Confirmed by execution**, fixed by returning existing without re-load.
3. **ModalModelDefault List leak**: `_nodes`/`_transitions` not deleted in destructor. **Confirmed by ASan/LSan**, fix in PR #538.

## Remaining backend blockers (not `done_confirmed`)

1. Legacy `ModalModelDefault` node-list path + `ModalModelFSM`/`ModalModelPetriNet` still registered and used by model-specific smart apps.
2. Residual heap `EFSMTransition` ownership on load (pre-existing; not introduced by #534/#536).
3. Focused ASan suite still reports other List/container leaks for stack-constructed network probes / heap transitions — requires dedicated ownership PR beyond #538.
4. GUI explicitly out of scope; global Modal/Network `done_confirmed` still requires GUI decision/execution per completion plan.

## Classification note

`AUTO-MODAL-002` remains `running`. Backend persistence/identity/full-model round-trips are advanced; legacy cleanup and sanitizer closure are not complete.
