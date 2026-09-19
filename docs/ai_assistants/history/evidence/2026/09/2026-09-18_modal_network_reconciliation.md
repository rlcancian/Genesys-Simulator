---
document_type: evidence
authority: historical-evidence
owner: project-maintainer
recorded_at: 2026-09-18
status: active
scope: modal-network-documentation-reconciliation
---

# 2026-09-18 ModalModel / DefaultNetwork reconciliation

## Scope analyzed

Remote GitHub-only reconciliation of the current `WorkInProgress` ModalModel / DefaultNetwork state, prior implementation records, canonical documentation, and maintainer decisions supplied on 2026-09-18.

Current remote `WorkInProgress` HEAD observed during this pass:

`4c745d7181c1584c7cf859a4db4ca93d46c95615`

Commit message: `Enabling gui bilds in clion`.

## Environment boundary

This pass used the GitHub connector/API only.

No claim is made that this environment locally configured CMake, built with Ninja, ran CTest, launched GenESyS, or ran sanitizers.

Current executable verification is therefore pending for the local continuation agent.

## Canonical documents consulted

- repository `/README.md`;
- `docs/ai_assistants/README.md`;
- `GOVERNANCE.md`;
- `STATUS.md`;
- `BACKLOG_AUTONOMOUS.md`;
- `BACKLOG_HUMAN.md`;
- `reference/GENESYS_MODAL_MODEL_NETWORK_ARCHITECTURE.md`;
- `reference/MODAL_NETWORK_GUI_ARCHITECTURE.md`;
- `runbooks/GITHUB_AGENT.md`;
- relevant PR/branch/history evidence from the prior Modal/Network cycle.

## Historical implementation evidence retained

The 2026-08-31 cycle associated with PRs #528/#529 records implementation and validation of:

- `DefaultNetwork` / `DefaultNode` core;
- `ModalModelDefault` bridge;
- `EFSMNetwork`;
- `GraphNetwork`, `DirectedGraphNetwork`, `DirectedAcyclicGraphNetwork`;
- `MarkovChainNetwork` as a finite time-homogeneous DTMC;
- `ColoredPetriNetNetwork` as a pragmatic fixed-inscription CPN subset;
- focused tests for the above;
- historical kernel-unit snapshot of 1810/1810 executed tests passing, with four preexisting disabled tests.

Classification: **historical evidence**, not proof for the 2026-09-18 current HEAD.

## Maintainer decisions recorded

The maintainer, Prof. Rafael Cancian, established the following on 2026-09-18:

1. Historical `.gen` models are irrelevant to the continuation and are not compatibility requirements or acceptance criteria. Current supported persistence still requires save/load round-trip validation.
2. `closed` means only that a task/development cycle is inactive. It does not imply completion.
3. `done_confirmed` is reserved for scope that is fully implemented according to plan, tested, functioning, verified, documented, and accepted with no known unmet in-scope criterion.
4. `AUTO-MODAL-001` therefore must not be interpreted as proof that the Modal/Network architecture is complete.
5. Obsolete Modal/Network implementation classes must be removed from the repository `source/` tree once current dependency analysis proves that their migration/removal is safe. They must not be retained in another source subdirectory merely for historical reference. A temporary local copy may be kept outside the repository (for example `/tmp`) during work, but Git history is the durable archive.
6. The source-removal decision is specifically supported by current CMake evidence: `source/plugins/components/CMakeLists.txt` uses recursive `*.cpp` discovery (`GLOB_RECURSE`), so relocating an obsolete `.cpp` below that tree would continue compiling it. The project should not complicate CMake merely to retain obsolete source.
7. The initial CPN target is a pragmatic subset sufficient for GenESyS, not a complete academic CPN platform.
8. Cellular Automata is intended eventually to migrate to the `DefaultNetwork` architecture, but that work is deferred until the current Modal/Network architecture is complete, functional, tested, and validated.
9. Multiple enabled EFSM transitions shall use a configurable EFSM-level policy with three semantic choices: model error; nondeterministic random choice using the GenESyS reproducible RNG/sampler infrastructure; deterministic priority.
10. Every supported network type shall have dedicated unit tests, with local build/test/runtime verification before claims of completion.
11. GUI/editor work should follow backend contract stabilization rather than precede it.

## Documentation changes in this branch

- Added `reference/MODAL_NETWORK_COMPLETION_PLAN.md` with the verification-first continuation roadmap.
- Updated `STATUS.md` to mark the current Modal/Network state as verification-pending and not `done_confirmed`, record the maintainer decisions, remove historical `.gen` compatibility as a requirement, and link the continuation guide.
- Updated `GOVERNANCE.md` with normative lifecycle semantics for `closed`, `done_confirmed`, and legacy `done` records.
- Reconciled the legacy-source cleanup guidance after inspecting `source/plugins/components/CMakeLists.txt`: obsolete classes are to leave `source/`, not be retained in a nested source directory.

## Backlog reconciliation boundary

`BACKLOG_AUTONOMOUS.md` and `BACKLOG_HUMAN.md` are large canonical files whose connector responses are truncated. `runbooks/GITHUB_AGENT.md` explicitly prohibits reconstructing a large file from partial/truncated connector output or performing unsafe full-file replacement for a small edit.

Therefore this GitHub-only pass intentionally did **not** overwrite either backlog.

Required first local documentation action before autonomous Modal implementation:

- change `AUTO-MODAL-001` from historical `done` to `closed` and preserve its PR/validation record as historical evidence;
- add/activate a bounded continuation task, recommended ID `AUTO-MODAL-002`, whose scope is to verify the current HEAD and complete only confirmed gaps described by `reference/MODAL_NETWORK_COMPLETION_PLAN.md`;
- reconcile `HUM-MODAL-001`: historical `.gen` compatibility is no longer a decision or requirement; remaining legacy-class handling is implementation/dependency analysis under the maintainer direction recorded above, with obsolete classes removed from `source/` when safe;
- reconcile `HUM-MODAL-002` against the existing GUI architecture reference: backend stabilization precedes broad editor implementation; only genuinely unresolved GUI product/architecture choices should remain human-decision items;
- use the new normative status vocabulary from `GOVERNANCE.md`.

This pending backlog edit is a documentation safety limitation of the GitHub-only connector environment, not an unresolved product/architecture decision.

## Current verification status

Current local CMake/Ninja build: **not executed in this environment**.

Current Modal/Network focused CTest suite: **not executed in this environment**.

Current aggregate regression: **not executed in this environment**.

Current sanitizers: **not executed in this environment**.

The next local agent must establish these facts before classifying implementation gaps.

## Next step

Use `reference/MODAL_NETWORK_COMPLETION_PLAN.md` as the technical guide. Begin with current-HEAD inventory and executable verification, then reconcile the two canonical backlogs before implementing confirmed gaps. Do not treat historical `closed`/`done` labels as completion evidence.