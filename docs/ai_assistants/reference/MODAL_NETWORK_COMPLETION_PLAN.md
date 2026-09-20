---
document_type: reference
authority: technical-plan
owner: project-maintainer
last_reviewed: 2026-09-19
review_cadence: on-modal-contract-change
status: active
tracks: AUTO-MODAL-001,AUTO-MODAL-002
---

# ModalModel / DefaultNetwork Completion and Verification Plan

## 1. Purpose

This document records the maintainer-approved continuation direction for the GenESyS ModalModel / DefaultNetwork development after the implementation cycle represented by PRs #528 and #529.

It is a technical guide for the next local development agent. It is **not** evidence that the items below are currently missing, broken, or incomplete. The next agent has access to the local checkout, Git, `gh`, CMake, Ninja, CTest, sanitizers, executables, and runtime inspection and must verify each item against the current `WorkInProgress` branch before changing code.

The companion architectural baseline remains:

- `reference/GENESYS_MODAL_MODEL_NETWORK_ARCHITECTURE.md`;
- `reference/MODAL_NETWORK_GUI_ARCHITECTURE.md`.

This document narrows the continuation work, records maintainer decisions made on 2026-09-18, and distinguishes verified historical evidence from **verification pending on the current branch**.

## 1.1 Current integration checkpoint (2026-09-20)

Confirmed by executed local evidence on `WorkInProgress` HEAD `e0a6a73b`:

- public presets restored by PR #533;
- EFSM three-mode conflict policy merged by PR #534;
- Model-file EFSM/Modal + DTMC reproducibility by PR #535;
- canonical LoadInstance reuse (identity) by PR #536;
- full Model round-trips Graph/DTMC/CPN + Modal execute-after-load by PR #537;
- `tests-unit` / `tests-kernel-unit`: 1,836 registered, 1,832 passed, 0 failed, 4 disabled;
- `tests-smoke`: 3/3;
- focused Modal/Network: 75/75.

This checkpoint advances the verification matrix for persistence/identity but does **not** close legacy cleanup, sanitizer residual ownership, GUI, or authorize `done_confirmed`.

Evidence: `history/evidence/2026/09/2026-09-20_modal_backend_gate_progress.md`.

## 2. Evidence discipline for this plan

Use the governance classifications exactly:

- **confirmed in current code/build** — directly inspected in the current branch or generated build graph;
- **confirmed by executed evidence** — demonstrated by current local execution or an identified current workflow/artifact;
- **strong indication** — supported by multiple observations but not yet executed or exhaustively proven;
- **hypothesis to validate** — plausible and explicitly unconfirmed;
- **historical evidence** — demonstrated only for an older commit/PR/environment.

Historical validation from PRs #528/#529 is useful but does not automatically prove the same result for the current `WorkInProgress` HEAD.

Every proposed gap in Sections 7–13 below must therefore be reclassified by the local agent after inspection as one of:

- already implemented and currently verified;
- implemented but current verification pending;
- partially implemented;
- required and missing;
- intentionally deferred;
- superseded/not applicable.

Do not implement an item merely because it appears in this plan.

## 3. Maintainer decisions recorded on 2026-09-18

### 3.1 Historical `.gen` compatibility is not a requirement

Maintainer decision:

> Historical `.gen` model compatibility is irrelevant to the ModalModel / DefaultNetwork completion work and must not be used as a requirement, acceptance criterion, blocker, or reason to preserve obsolete classes.

Consequences:

- do not spend development effort preserving old `.gen` files solely because they existed before the migration;
- persistence testing remains required for the **current supported representation**;
- save/load round-trip for current objects remains required;
- factories, registration, GUI, examples, tests, and current code references must still be checked before obsolete classes are removed.

### 3.2 Meaning of `closed` and `done_confirmed`

Maintainer decision:

- `closed` means only that a task/development cycle is no longer actively being developed;
- `closed` **does not mean complete**;
- `done_confirmed` is the terminal state reserved for work that is fully developed according to its approved plan, built, tested, functioning, verified, documented, and accepted with no known unmet acceptance criterion in that scope.

Accordingly, `AUTO-MODAL-001` must be interpreted as **closed**, not as proof that ModalModel / DefaultNetwork development is complete.

Historical uses of `done` elsewhere in the backlog are legacy records and must not be silently reinterpreted as `done_confirmed` without explicit re-verification.

### 3.3 Legacy ModalModel classes and source-tree removal

Maintainer decision:

- obsolete Modal/Network implementation classes must **not** be retained anywhere under `source/` merely for historical reference;
- once current dependency analysis proves that an old class is no longer needed, remove its source/header files from the `source` tree;
- do **not** modify the component CMake source-selection policy merely to keep obsolete source files present but uncompiled;
- if a short-lived safety copy is useful during local work, it may be placed outside the repository source tree (for example under `/tmp`) and must not be committed;
- Git history is the durable archival mechanism for removed source;
- do not preserve a class solely for historical `.gen` compatibility;
- before deleting anything, map current includes, inheritance, CMake/build discovery, plugin registration, factories/connectors, GUI references, tests, model-specific examples, and current persistence references.

Confirmed CMake fact motivating this decision:

`source/plugins/components/CMakeLists.txt` currently uses:

```cmake
file(GLOB_RECURSE GENESYS_PLUGINS_COMPONENTS_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)
```

Therefore, moving an obsolete `.cpp` to another subdirectory below `source/plugins/components/` would still include it in `genesys_plugins_components`. The cleanup strategy is removal from `source/`, not creation of a special source subdirectory or a CMake exclusion solely for obsolete code retention.

Candidate legacy types to reassess include at least:

- `ModalModelFSM`;
- `ModalModelPetriNet`;
- legacy `State` / `Transition` artifacts and any predecessor node/transition structures shown by the current dependency graph.

The list above is a verification target, not a deletion order. Remove a class only after current-code evidence establishes that its functionality has been superseded and all required callers/registrations/tests have been migrated.

This is a migration-cleanup activity, not the first implementation step.

### 3.4 CPN target depth

Maintainer decision:

> The first target is a **pragmatic CPN subset sufficient for GenESyS**, not a complete academic CPN platform.

The current fixed-inscription implementation may already satisfy part of this target. The local agent must first establish precisely what the current code supports before extending it.

Advanced features such as a general type system, unrestricted variable binding, expression-rich inscriptions, maximal concurrent-step firing, or additional stochastic semantics are not automatically required. They become required only if they are necessary for the agreed pragmatic GenESyS subset or are separately approved later.

### 3.5 Cellular Automata

Maintainer decision:

> Cellular Automata shall eventually be integrated into the DefaultNetwork architecture.

However, this migration is **deferred** until the current ModalModel / DefaultNetwork architecture is complete, functional, tested, and validated.

The future migration must respect Cellular Automata-specific characteristics, including regular spatial structure, neighborhood topology, and synchronous/asynchronous update semantics. No CA refactor should be started as part of the immediate completion work.

### 3.6 EFSM multiple-enabled-transition policy

Maintainer decision:

When multiple EFSM transitions are enabled, the policy shall be configurable by an EFSM-level enumeration with the following semantic choices:

1. **model error** — multiple enabled normal transitions are treated as an invalid/ambiguous model state;
2. **nondeterministic choice** — choose one enabled transition randomly using the GenESyS reproducible RNG/sampler infrastructure;
3. **deterministic priority** — choose according to explicit transition priority, with deterministic handling of ties defined and tested.

The exact C++ enum/type/member names must be chosen only after inspecting the current EFSM API and persistence conventions. Do not invent an incompatible API merely to match these prose labels.

The local implementation must define and test:

- the default policy;
- persistence of the selected policy;
- reset behavior;
- model-error diagnostics;
- reproducibility for the random policy under controlled seed/reset;
- priority ordering and tie behavior.

Ptolemy II may be consulted as a semantic reference for nondeterministic FSM behavior, but GenESyS must preserve its own architecture and reproducibility requirements.

## 4. Current historical baseline — not current-run proof

Historical evidence from the 2026-08-31 implementation cycle records that the following existed and passed focused/regression tests at that checkpoint:

- `DefaultNetwork` / `DefaultNode` core;
- `ModalModelDefault` network bridge;
- `EFSMNetwork`;
- `GraphNetwork`, `DirectedGraphNetwork`, `DirectedAcyclicGraphNetwork`;
- `MarkovChainNetwork` as a finite time-homogeneous DTMC;
- `ColoredPetriNetNetwork` as a fixed-inscription, single-firing pragmatic CPN subset;
- focused unit tests for those families;
- a kernel-unit regression snapshot with 1810 executed tests passing and four preexisting disabled tests.

Classification: **historical evidence** until re-executed or otherwise confirmed against the current `WorkInProgress` HEAD.

## 5. First action for the next local agent

Before coding:

1. read `/README.md` and `docs/ai_assistants/README.md` and follow their routing;
2. read `GOVERNANCE.md`, `ARCHITECTURE.md`, `STATUS.md`, both canonical backlogs, `runbooks/LOCAL_AGENT.md`, and the two Modal/Network architecture references;
3. fetch/prune remotes and confirm the exact `origin/WorkInProgress` HEAD;
4. inspect current Git status and do not overwrite unrelated local work;
5. inspect PRs #525, #526, #528, #529, #530 and relevant historical branches only as needed to reconcile current state;
6. configure/build the current supported local baseline before changing source;
7. run the existing Modal/Network focused tests and the required aggregate regression suites;
8. create an evidence record for the current HEAD before concluding what remains.

The result of this first pass should be a **verification matrix**, not an immediate refactor.

## 6. Required verification matrix

At minimum, classify each item below using current-code/current-run evidence.

| Area | Current implementation | Current focused tests | Current aggregate regression | Status/gap |
|---|---|---|---|---|
| `DefaultNetwork` core | verify | verify | verify | pending |
| `DefaultNode` / `DefaultNodeTransition` | verify | verify | verify | pending |
| `ModalModelDefault` adapter | verify | verify | verify | pending |
| EFSM | verify | verify | verify | pending |
| Graph / Directed / DAG | verify | verify | verify | pending |
| DTMC / Markov | verify | verify | verify | pending |
| pragmatic CPN subset | verify | verify | verify | pending |
| persistence/current round-trip | verify | verify | verify | pending |
| plugin/factory registration | verify | verify where possible | verify | pending |
| legacy classes/current references | verify | N/A | build/test | pending |
| GUI integration | verify | verify existing GUI tests | verify | pending |
| Cellular Automata migration prerequisites | inspect only | N/A | N/A | deferred |

## 7. Candidate work A — complete the already-approved backend architecture

These are candidate completion items. The local agent must first prove whether each remains necessary.

### A1. `DefaultNetwork` contract and ownership

Verify:

- actual C++ inheritance and composition relationships;
- ownership versus observation for nodes, transitions/edges, and runtime state;
- port schema ownership;
- activation-frame/result lifecycle;
- activation counter semantics;
- `_check()` behavior and invariants;
- reset/replication lifecycle;
- current persistence contract.

If the contract is already complete and covered, record evidence and do not refactor it for style.

If a real ownership/lifetime gap exists, add a focused regression before changing ownership.

### A2. `ModalModelDefault` adapter contract

Verify end-to-end:

- association with `DefaultNetwork`;
- input bindings;
- output bindings;
- synchronization/mirroring of logical network ports;
- activation-frame construction;
- network activation;
- input presence semantics;
- output presence semantics;
- zero-output entity consumption through the correct GenESyS lifecycle;
- one-output reuse of the original entity;
- multiple-output cloning and independent routing;
- `_check()` diagnostics;
- reset/lifecycle;
- current persistence round-trip.

Do not add functionality that tests prove already exists correctly.

### A3. Current persistence

Historical `.gen` compatibility is explicitly out of scope, but current persistence is not.

For each supported current formalism, verify a sequence equivalent to:

```text
construct -> check -> save -> destroy/recreate -> load -> check -> execute -> compare expected semantics
```

The exact fixture mechanism should follow current GenESyS test patterns.

## 8. Candidate work B — compatibility and migration cleanup

Only after Section 7 is stable and green:

1. map all current dependencies of candidate legacy modal classes;
2. determine whether each class is:
   - still part of the current runtime/API;
   - a temporary bridge whose callers still require migration;
   - redundant with `DefaultNetwork` formalism classes;
   - unused/dead;
3. migrate any legitimate current caller, factory/connector registration, GUI reference, test, or example to the approved architecture before removing the obsolete class;
4. once dependency analysis proves the old class is unnecessary, delete its implementation/header from `source/` rather than retaining obsolete code inside the source tree;
5. do not add CMake exclusions solely to retain obsolete classes; the current recursive component source discovery is a reason to remove the files, not to complicate the build;
6. if a temporary safety copy is useful during the local edit, keep it outside the repository (for example `/tmp`) and do not commit it;
7. update tests, documentation, examples, factories/connectors, and current persistence references consistently;
8. rebuild and rerun focused plus aggregate tests after each bounded removal.

Acceptance is based on the current supported architecture, not historical `.gen` files. Git history remains available if removed source ever needs to be inspected again.

## 9. Candidate work C — EFSM completion

Verify first whether the existing `EFSMNetwork` already supports:

- network-owned current state;
- initial state;
- transitions;
- guards;
- actions/updates;
- persistence;
- reset;
- shared-network activation;
- deterministic priority behavior.

Then implement only the missing parts needed for the maintainer-approved multiple-enabled-transition configuration.

The required semantic policies are fixed by Section 3.6. The exact API shape is not fixed and must fit the current code.

A separate `FSMNetwork` is **not** required merely for naming symmetry. Introduce one only if current-code evidence demonstrates a technical need not satisfied cleanly by EFSM.

Required tests should include nominal, boundary, invalid-model and reproducibility cases for all configured conflict policies.

## 10. Candidate work D — Graph family completion

Treat Graph networks as mathematical structures unless a separately approved model-of-computation adds activation semantics.

Verify current implementation and tests for the operations actually promised by the current API/documentation, including where present:

- undirected/directed topology;
- parallel edges;
- self-loops;
- weights;
- BFS;
- DFS;
- reachability;
- shortest paths;
- connected components;
- strongly connected components;
- cycle detection;
- DAG cycle rejection;
- topological sorting;
- current persistence round-trip.

Do **not** add entity movement, random walk, graph routing or traversal simulation merely because they are possible graph applications.

Do not implement Bellman-Ford or negative-weight support unless the current supported contract requires it.

## 11. Candidate work E — DTMC / Markov completion

Treat the current target as finite time-homogeneous DTMC unless current approved architecture has changed.

Verify:

- current and initial network-owned state;
- one-step activation semantics;
- transition representation;
- nonnegative probability validation;
- row-stochastic validation and tolerance;
- deterministic and stochastic transitions;
- absorbing-state representation;
- GenESyS RNG/sampler use;
- reproducibility under controlled reset/seed;
- persistence and reset;
- current output semantics.

CTMC, controlled Markov chains, MDPs and time-inhomogeneous processes remain future extensions unless separately activated.

## 12. Candidate work F — pragmatic CPN completion

The target is a pragmatic GenESyS subset.

First create an implementation/test matrix for:

- places;
- transitions;
- directed bipartite arcs;
- marking;
- symbolic colors/token representation;
- arc inscriptions;
- guards;
- enabling;
- conflict resolution;
- firing;
- reset/initial marking;
- persistence;
- shared-network activation.

For each capability classify it as:

- supported and verified;
- supported but verification pending;
- partially supported;
- required for the pragmatic subset and missing;
- deferred/not required.

Do not automatically require:

- a general CPN type system;
- general variable-binding enumeration;
- arbitrary typed token payloads;
- maximal concurrent-step firing;
- stochastic conflict resolution;
- a full CPN language.

If a missing advanced feature is judged necessary for the pragmatic GenESyS subset, document the concrete GenESyS use case and obtain/record approval before broadening the scope.

## 13. Candidate work G — GUI/editor after backend stabilization

The backend contracts above take precedence over broad GUI implementation.

After the backend reaches a verified stable contract, compare the current GUI with `MODAL_NETWORK_GUI_ARCHITECTURE.md` and classify what is already implemented.

Candidate GUI capabilities include:

- creating/selecting a network for `ModalModelDefault`;
- displaying the attached network;
- presenting logical network input/output ports;
- editing input/output bindings;
- creating/editing formalism-owned elements;
- formalism-specific property editors;
- current-state visualization during execution;
- separation between process-flow `Connection` editing and formal network topology.

Do not duplicate network semantics inside GUI classes. The backend remains the source of truth.

## 14. Deferred future work

The following are explicitly **not part of the immediate completion gate**:

- Cellular Automata migration to `DefaultNetwork` — approved direction, deferred until the current architecture is `done_confirmed`;
- CTMC;
- MDP;
- controlled/time-inhomogeneous Markov models;
- graph traversal/movement simulation unless separately approved;
- full academic CPN semantics beyond the pragmatic subset;
- unrelated plugin-architecture modernization.

## 15. Test and validation expectations for the local continuation

Every supported network type must have dedicated unit tests.

At minimum, the continuation should verify or add coverage for:

### Common network layer

- creation;
- invalid construction/invariants;
- `_check()`;
- activation where applicable;
- reset;
- current persistence round-trip;
- plugin/factory registration when applicable.

### Modal adapter

- input/output bindings;
- port synchronization;
- presence/absence;
- zero/one/multiple outputs;
- entity consumption;
- entity cloning;
- invalid/missing network reference;
- reset;
- persistence.

### EFSM

- initial/current state;
- guards/actions;
- each multiple-enabled-transition policy;
- random-policy reproducibility;
- priority/tie behavior;
- model-error diagnostics;
- reset;
- persistence.

### Graph family

- structural invariants;
- each advertised algorithm;
- directed/undirected differences;
- DAG cycle rejection;
- persistence.

### DTMC

- probability constraints;
- row validation;
- sampling;
- deterministic/absorbing cases;
- reproducibility;
- reset;
- persistence.

### CPN pragmatic subset

- topology;
- marking;
- enabling;
- guards/inscriptions actually supported;
- conflict/firing policy actually supported;
- atomic consume/produce;
- reset;
- persistence.

After focused tests, run the aggregate regression required by `GOVERNANCE.md`. Use ASan/LSan/UBSan when ownership/lifetime/resource behavior is changed or uncertain.

## 16. Suggested incremental sequence

This is a recommended order, not a mandate. Reorder only when current-code evidence gives a concrete reason.

1. current-HEAD inventory and executable baseline;
2. `DefaultNetwork` contract verification;
3. `ModalModelDefault` adapter verification/completion;
4. current persistence round-trip verification;
5. EFSM verification and configurable conflict policy;
6. Graph family verification;
7. DTMC verification;
8. pragmatic CPN support matrix and completion of only confirmed required gaps;
9. legacy dependency audit and physical removal from `source/` of obsolete classes whose migration is proven complete;
10. complete focused + aggregate + sanitizer validation as appropriate;
11. reconcile canonical status/backlog/reference/evidence;
12. only then begin GUI/editor work;
13. after GUI/backend completion, assess whether the entire approved current architecture satisfies `done_confirmed`;
14. only after that consider the deferred Cellular Automata migration as a new bounded development task.

Prefer one bounded concern per branch/PR. Do not combine legacy cleanup, EFSM semantic changes, CPN expansion and GUI implementation in one PR.

## 17. Criteria for `done_confirmed` for the current Modal/Network architecture

The current development may be marked `done_confirmed` only when all approved in-scope requirements are demonstrably satisfied on the current integration baseline, including:

- backend architecture matches the approved `ModalModel` / `DefaultNetwork` separation;
- `DefaultNetwork` common contracts and ownership/lifecycle are explicit and verified;
- `ModalModelDefault` adapter semantics are complete and verified;
- EFSM, Graph, DTMC and the approved pragmatic CPN subset are implemented according to their declared contracts;
- each supported network type has focused unit tests;
- current persistence round-trips are verified;
- required plugin/factory registrations are verified;
- obsolete legacy source paths have been removed from `source/` after all required current callers/registrations/tests are migrated;
- required regression suites pass;
- sanitizer/ownership diagnostics required by changes pass;
- GUI work that is part of the approved current architecture is complete and verified, or explicitly reclassified by the maintainer as outside the completion gate;
- documentation/manual impact is reconciled;
- no known in-scope acceptance criterion remains unverified or unmet.

Historical `.gen` compatibility is not part of this criterion.

Cellular Automata migration, CTMC, MDP and other explicitly deferred extensions are not required for the current Modal/Network `done_confirmed` state.

## 18. Evidence and documentation closure

For every material local step:

- record branch and base SHA;
- record exact commands and results;
- distinguish focused test evidence from aggregate regression evidence;
- update `history/evidence/` with dated immutable execution records where appropriate;
- update `STATUS.md` only with current state;
- update `BACKLOG_AUTONOMOUS.md` when task state materially changes;
- use `BACKLOG_HUMAN.md` only for unresolved maintainer decisions;
- update this reference or the architecture reference only when the approved contract changes;
- assess Developer/User Manual impact under `GOVERNANCE.md`.

Do not mark the work `done_confirmed` from code inspection alone.