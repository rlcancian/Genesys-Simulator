---
document_type: evidence
authority: executed-evidence
owner: project-maintainer
recorded_at: 2026-09-20
status: active
scope: modal-network-backend-gate-closure
---

# 2026-09-20 ModalModel / DefaultNetwork backend gate closure

## Environment

- Worktree: `Genesys-ModalNetwork`
- Ubuntu 24.04; g++ 13.3.0; CMake 3.28.3; Ninja 1.11.1; Qt 6.4.2
- Integration HEAD: `5bd10bb5d04e9acfe95d6277415dbb98b9a33b40` (`refactor(modal): remove legacy wrappers and ModalModelDefault node-list path (#541)`)

## Integrated in this continuation (after #535)

| PR | Merge | Scope |
|---|---|---|
| #536 | `5494b237` | Canonical `LoadInstance` reuse (identity) |
| #537 | `e0a6a73b` | Full Model round-trips Graph/DTMC/CPN + Modal execute-after-load |
| #538 | `811c2525` | `ModalModelDefault` owned List container delete |
| #539 | `4229c00f` | STATUS/backlog sync after #536/#537 |
| #540 | `a913a07a` | Own heap EFSM/Markov transitions; `DefaultNode` List dtor |
| #541 | `5bd10bb5` | Remove `ModalModelFSM`/`ModalModelPetriNet`/node-list/`PetriTransition`; migrate smart apps |

## Local validation on HEAD `5bd10bb5`

- Focused Modal/Network regex: **73/73** (2 legacy-only tests removed with wrappers)
- `tests-unit`: **1830** passed, 0 failed, 4 disabled (1834 registered) — confirmed on pre-merge tip and CI #541
- `tests-kernel-unit`: **1830** passed, 0 failed, 4 disabled (1834 registered)
- `tests-smoke`: **3/3**
- ASan/LSan (EFSM excl. ModelFile autoInsert Buffer noise): **11/11** exit 0 after #540
- ASan/LSan (Markov excl. ModelFile): **10/10** exit 0 after #540

## Backend gate classification

| Area | Status |
|---|---|
| `DefaultNetwork` / `ModalModelDefault` adapter | confirmed by execution |
| EFSM (3 conflict policies, identity, persistence, reset) | confirmed by execution |
| Graph / Directed / DAG | confirmed by execution |
| DTMC | confirmed by execution |
| pragmatic CPN | confirmed by execution |
| plugin/factory registration | confirmed by execution |
| ownership of heap transitions + DefaultNode List | confirmed by ASan + code |
| legacy wrappers / node-list path | removed from `source/` (#541) |
| GUI | intentionally deferred (next phase) |
| Cellular Automata migration | intentionally deferred |

## Explicit non-claims

- Global Modal/Network architecture is **not** `done_confirmed` while GUI remains an approved completion-plan phase.
- `AUTO-MODAL-002` backend scope may be `done_confirmed`; GUI is tracked under `HUM-MODAL-002`.
- Process-lifetime `autoInsertPlugins` Buffer/PluginInformation leaks remain outside Modal ownership scope.
