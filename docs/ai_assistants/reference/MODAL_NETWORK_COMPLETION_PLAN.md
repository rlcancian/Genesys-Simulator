---
document_type: implementation-plan
authority: maintainer-approved-plan
owner: project-maintainer
last_reviewed: 2026-09-18
review_cadence: on-modal-contract-or-status-change
status: active
tracks: AUTO-MODAL-001,AUTO-MODAL-002,HUM-MODAL-001,HUM-MODAL-002,HUM-MODAL-003,HUM-MODAL-004,HUM-MODAL-005
---

# ModalModel / DefaultNetwork Completion and Verification Plan

## 1. Purpose

This document records the current maintainer decisions and the recommended verification/completion path for the GenESyS `ModalModel` / `DefaultNetwork` architecture after the implementation integrated by PRs #528 and #529.

It is deliberately a **verification-led implementation guide**, not proof that the architecture is complete. The next local agent has access to Git, `gh`, CMake, Ninja, CTest, executables and diagnostic tools and must confirm the current code before accepting, modifying or discarding any recommendation below.

Canonical current state remains in [`../STATUS.md`](../STATUS.md). Executable task state remains in [`../BACKLOG_AUTONOMOUS.md`](../BACKLOG_AUTONOMOUS.md). Maintainer decisions remain in [`../BACKLOG_HUMAN.md`](../BACKLOG_HUMAN.md).

This plan refines the earlier design baseline in [`GENESYS_MODAL_MODEL_NETWORK_ARCHITECTURE.md`](GENESYS_MODAL_MODEL_NETWORK_ARCHITECTURE.md). Where that older reference leaves an item open or treats a larger CPN/legacy-compatibility scope as mandatory, the maintainer decisions dated 2026-09-18 recorded here take precedence for the continuation of this development.

## 2. Evidence boundary at this checkpoint

### Confirmed in current remote code/documentation

At `WorkInProgress` HEAD `4c745d7181c1584c7cf859a4db4ca93d46c95615` inspected on 2026-09-18:

- the network-centered implementation from PRs #528/#529 is present in the integration branch;
- `DefaultNetwork`, `DefaultNode`, `ModalModelDefault`, `EFSMNetwork`, the `GraphNetwork` family, `MarkovChainNetwork` and `ColoredPetriNetNetwork` are represented in the current source tree;
- legacy modal classes remain present beside the newer network-centered implementation;
- `source/plugins/components/CMakeLists.txt` currently obtains component sources with `file(GLOB_RECURSE ... "*.cpp")`;
- `source/plugins/components/ModalModel/CellularAutomata/` and `CellularAutomataComp.*` are present;
- the latest retained implementation evidence for the 2026-08-31 modal work reports focused and aggregate tests passing at that historical checkpoint.

### Historical executed evidence, not current-head proof

The 2026-08-31 records report successful focused Modal/Network tests and a `tests-kernel-unit` run with 1810/1810 executed tests passing and four pre-existing disabled tests. Those results are valuable historical evidence for the commits recorded at the time. They **do not prove** that the 2026-09-18 `WorkInProgress` HEAD has the same behavior.

### Verification pending

The current GitHub-only reconciliation did not locally execute:

- CMake configure;
- Ninja build;
- CTest;
- GUI startup or interaction;
- sanitizers;
- model execution;
- persistence round trips at the current HEAD.

The local continuation must re-establish these facts before declaring the architecture complete.

## 3. Maintainer decisions recorded on 2026-09-18

### 3.1 Historical `.gen` compatibility is not a requirement for this work

Old persisted `.gen` models are irrelevant to completion of the ModalModel/Network migration. They must not constrain architecture, implementation, cleanup or acceptance criteria for this effort.

This does **not** remove the requirement that the **current supported implementation** have coherent save/load behavior and round-trip tests for its own persisted data.

### 3.2 Completion state must be explicit

A branch, PR, issue or backlog item being `closed` means only that it is **not currently being developed**. It does not mean complete, correct or validated.

For backlog/task maturity, `done_confirmed` is the only status that certifies all currently approved required work for that bounded task/initiative has been implemented, tested, functioning and verified according to its acceptance criteria.

A bounded historical PR can therefore have been merged successfully without the broader ModalModel/Network initiative being `done_confirmed`.

### 3.3 Legacy ModalModel implementation may be isolated from the build

Legacy classes that are no longer part of the intended architecture may be moved temporarily to:

`source/plugins/components/ModalModel/deprecated/`

The standard spelling `deprecated` is recommended.

Files under a `deprecated/` directory are intended **not to compile**.

Important current-code constraint: the existing component CMake uses recursive `*.cpp` globbing, so moving a `.cpp` file beneath `ModalModel/deprecated/` would still compile it today. Before using this quarantine strategy, the local implementation must either:

1. explicitly exclude `deprecated/` from the component source set; or
2. replace the broad recursive source discovery with a safer source-selection mechanism if that is justified by the surrounding CMake architecture.

The smallest safe change is preferred. Do not use the deprecation move as a hidden broad CMake refactor.

Physical deletion is also acceptable later if local dependency analysis proves the classes are unused and the deletion is safer/clearer than retained dead source. Permanent cleanup should occur only after the replacement architecture is `done_confirmed`.

### 3.4 Initial CPN objective is pragmatic GenESyS support

The near-term target is a **pragmatic CPN subset sufficient for GenESyS use cases**.

Typed token systems, general variable-binding enumeration, general expression-based arc inscriptions, maximal concurrent-step firing and other advanced CPN features are not automatically mandatory merely because they exist in the complete academic formalism.

They should be added only when a concrete GenESyS requirement, scientific use case or approved extension justifies them and the semantics can be reference-backed and tested.

### 3.5 EFSM multiple-enabled-transition policy is configurable

`EFSMNetwork` must expose a persisted configuration policy equivalent to an enum with exactly these semantic choices:

1. **model error** — more than one enabled transition is a model error;
2. **nondeterministic selection** — choose one enabled transition using the reproducible GenESyS RNG infrastructure;
3. **deterministic by priority** — select according to an explicit deterministic priority rule.

Exact C++ identifier spelling should follow the surrounding code style and is intentionally left to the local implementation.

The default policy must be selected explicitly during implementation and documented. It must not be inherited accidentally from container iteration order.

The model checker, runtime behavior, persistence and reset/reproducibility tests must agree with the selected policy.

Ptolemy II is an architectural/semantic reference for EFSM conflict behavior, but GenESyS does not need API compatibility with Ptolemy.

### 3.6 Cellular Automata will migrate later to DefaultNetwork

The maintainer's architectural objective is to integrate Cellular Automata into the `DefaultNetwork` architecture.

This work is **deferred until the current ModalModel/DefaultNetwork architecture is complete, functional, tested and `done_confirmed`**.

The later migration must preserve Cellular Automata-specific semantics rather than flattening them merely for nominal hierarchy uniformity, including at least:

- regular spatial/lattice structure;
- neighborhood definition;
- boundary conditions;
- synchronous/asynchronous update policy;
- deterministic/stochastic rules;
- state ownership and persistence;
- relation to simulation time/events.

Whether the final CA specialization uses generic graph internals, specialized lattice storage, or another structure remains an implementation question to resolve when that deferred work begins.

### 3.7 GUI follows backend stabilization

The architecture proposed in [`MODAL_NETWORK_GUI_ARCHITECTURE.md`](MODAL_NETWORK_GUI_ARCHITECTURE.md) remains the planning baseline.

Do not begin broad network editor implementation until the backend contracts, ownership, persistence, formalism semantics and unit tests have been revalidated and the backend completion gate has been satisfied.

## 4. Current architectural target

The intended boundary remains:

```text
ModelComponent
    |
    +-- ModalModel / ModalModelDefault process adapter
             |
             | attaches to / activates
             v
ModelDataDefinition
    |
    +-- DefaultNetwork
           |
           +-- EFSMNetwork
           +-- GraphNetwork
           |      +-- DirectedGraphNetwork
           |             +-- DirectedAcyclicGraphNetwork
           +-- MarkovChainNetwork
           +-- ColoredPetriNetNetwork
           +-- future CellularAutomataNetwork or equivalent CA specialization
```

The exact C++ inheritance and class names must always be confirmed from the current headers before modifying code. This diagram expresses architectural responsibility, not permission to invent missing classes.

`ModalModelDefault` should remain the process-world adapter. Formalism-specific runtime state and semantics belong in `DefaultNetwork` specializations.

## 5. Completion work — verification-led guidance

The items below are recommendations derived from the architecture, current source inspection and prior implementation record. The local agent must first verify whether each item is already satisfied at the current HEAD. If a requirement is already fully implemented and tested, record the evidence and do not reimplement it.

### A. Necessary to complete the approved backend architecture

#### A1. Re-establish the `DefaultNetwork` core contract

Verify from current code and tests:

- class relationships and ownership/non-ownership;
- input/output port schema;
- activation frame/result and presence/value distinction;
- activation counter semantics;
- `_check()` behavior;
- replication reset/lifecycle;
- current-format save/load round trip;
- shared activation from multiple ModalModel adapters;
- invalid/stale reference behavior.

If any contract is incomplete, implement only the smallest correction demonstrated by tests.

#### A2. Re-establish the `ModalModelDefault` adapter contract

Verify end-to-end:

- attachment/reference to `DefaultNetwork`;
- input bindings and expression validation;
- port/schema synchronization;
- fresh activation frame construction;
- correct input presence semantics;
- network activation;
- output presence/value handling;
- output bindings;
- zero-output entity consumption through the canonical lifecycle;
- one-output routing of the original entity;
- multi-output cloning using canonical entity cloning;
- absence of cross-contamination among clones;
- `_check()` diagnostics;
- reset/lifecycle;
- current-format persistence.

#### A3. Confirm ownership/lifetime explicitly

Trace construction, registration, attachment, destruction and model teardown for:

- `DefaultNetwork`;
- nodes/states;
- transitions/edges/arcs;
- ModalModel adapters;
- shared networks.

Run ASan/LSan/UBSan on focused representative tests if technically available. A clean focused run must be recorded as focused evidence only, not repository-wide proof.

#### A4. Confirm plugin/factory/registration paths

Verify each supported network and its required element types can be created through the current plugin/static registration mechanisms used by GenESyS, and that no deprecated class remains required solely because registration was not migrated.

### B. Compatibility and migration cleanup

Historical `.gen` compatibility is not a goal.

The cleanup question is therefore only whether legacy source types remain necessary for **current code, current factories, current tests, current examples, current GUI or current compilation paths**.

Recommended process:

1. inventory all references to `ModalModelFSM`, `ModalModelPetriNet` and other superseded modal scaffolding;
2. classify each reference as current-required, test-only, example-only, dead, or historical;
3. migrate current-required callers to the network-centered architecture where appropriate;
4. remove legacy files from the active build;
5. preferably quarantine temporarily under `source/plugins/components/ModalModel/deprecated/` if useful for one validation cycle;
6. ensure CMake excludes that directory before moving sources there;
7. after full validation, decide whether the quarantined files provide any continuing value; if not, delete them in a later bounded cleanup.

Do not preserve a wrapper merely to read an old `.gen` model.

### C. Formalism-specific completion

#### C1. EFSM

Verify and, where missing, complete:

- network-owned initial/current state;
- states and transitions;
- guards;
- state/internal variables if part of the supported contract;
- actions/output publication;
- input value and input presence access;
- final-state behavior if part of the supported contract;
- reset;
- current-format persistence;
- invalid guard/action diagnostics;
- shared-network activation;
- configurable multiple-enabled-transition policy with the three maintainer-approved modes.

No separate `FSMNetwork` should be introduced solely for naming symmetry. Add one only if current code evidence demonstrates a distinct semantic requirement not representable as a restricted EFSM.

#### C2. Graph family

Verify:

- graph node/edge ownership/reference rules;
- directed/undirected invariants;
- DAG cycle rejection;
- self-loop and parallel-edge behavior where supported;
- weight semantics;
- only the graph algorithms that the public/current contract actually claims;
- persistence and invalid-reference behavior.

A pure mathematical graph has no implicit Entity movement, current vertex, routing or activation traversal. Such dynamics require an explicitly approved later specialization.

#### C3. MarkovChainNetwork

Treat the current target as finite time-homogeneous DTMC.

Verify:

- network-owned current/initial state;
- nonnegative transition probabilities;
- row sum normalization/validation within declared tolerance;
- absorbing-state representation;
- one activation = one DTMC step;
- reproducible GenESyS RNG use;
- deterministic seeded behavior tests;
- empirical frequency sanity test;
- reset;
- current-format persistence;
- shared activation.

CTMC, controlled chains and MDPs are future extensions, not completion requirements for this DTMC task.

#### C4. ColoredPetriNetNetwork

First document the exact currently supported pragmatic subset from code and tests, then verify it fully.

At minimum, investigate the current implementation of:

- places;
- explicit transition nodes;
- bipartite arcs;
- marking;
- symbolic colors/token counts;
- fixed inscriptions;
- guards;
- enabling;
- atomic firing;
- conflict selection;
- initial marking/reset;
- persistence;
- shared activation.

Advanced typed tokens, general variable binding and concurrent/multi-firing are optional later extensions unless a concrete approved GenESyS use case makes one necessary.

### D. Unit and integration test matrix

There must be focused unit tests for **every supported network type**.

The local agent should inventory current tests first and fill only real gaps. The final backend suite should demonstrate, as applicable:

- construction/defaults;
- `_check()` nominal and invalid cases;
- topology/invariants;
- activation;
- no-op/inert activation where that is the intended Graph behavior;
- reset/replication lifecycle;
- deterministic seed/reproducibility for stochastic semantics;
- current-format persistence round trip;
- shared networks;
- invalid references;
- input/output binding and presence;
- zero/one/multiple output adapter behavior;
- entity cloning/consumption;
- formalism-specific edge/error behavior.

Run at least the project-required focused tests plus ordinary `tests-unit`, `tests-kernel-unit`, and `tests-smoke` when runtime behavior is affected. Use sanitizers for ownership/lifetime changes.

### E. GUI/editor — planned but backend-dependent

After backend completion is confirmed, revisit [`MODAL_NETWORK_GUI_ARCHITECTURE.md`](MODAL_NETWORK_GUI_ARCHITECTURE.md) against the current GUI before implementation.

Expected product direction remains:

- open/create/attach a network from a `ModalModelDefault`;
- dedicated network/data-definition editor, not process `Connection` reuse;
- formalism-specific palettes/properties;
- adapter input/output binding editing;
- independent semantic `.gen` data and graphical `.gui` layout;
- kernel-authoritative validation;
- read-only runtime overlays after editing is stable.

The local agent may adjust the previously proposed G0-G6 sequence if current GUI architecture or tests demonstrate a safer smaller path.

### F. Deferred future extensions

These do not block `done_confirmed` for the current ModalModel/DefaultNetwork scope unless separately activated by the maintainer:

- CTMC;
- MDP/controlled Markov chains;
- time-inhomogeneous Markov processes;
- dynamic graph traversal/random walk/routing/entity movement;
- advanced/full CPN type and binding system;
- CPN maximal concurrent-step semantics;
- EFSM state refinements/full Ptolemy semantics;
- generalized Connection/token payloads;
- Cellular Automata migration (explicitly planned, but deferred until current architecture is complete).

## 6. Recommended local sequence

This sequence is guidance. Reorder only when current-code evidence demonstrates a dependency reason.

1. **Baseline and inventory** — update local `WorkInProgress`, record HEAD/toolchain, run current build/test baseline, map classes/tests/registrations/references.
2. **Core contract audit** — `DefaultNetwork` + `DefaultNode` + ownership/persistence/reset.
3. **Adapter audit** — `ModalModelDefault` bindings/presence/activation/output routing/cloning/consumption.
4. **EFSM completion** — especially the new three-mode conflict policy and its persistence/tests.
5. **Graph validation** — structural invariants/algorithms/persistence without inventing movement semantics.
6. **DTMC validation** — mathematical invariants, RNG, reset, persistence and stochastic tests.
7. **CPN pragmatic-subset validation** — explicitly define supported subset and close real gaps only.
8. **Legacy dependency audit and build isolation** — migrate current callers, exclude `deprecated/`, quarantine or remove superseded implementation.
9. **Full regression + sanitizers** — rerun all required focused/aggregate paths and ownership diagnostics.
10. **Backend completion review** — only now assess whether backend can be marked `done_confirmed`.
11. **GUI revalidation and implementation** — revisit G0-G6 against current GUI and execute as small PRs.
12. **Final end-to-end verification/documentation** — examples/workflows, GUI where in scope, documentation, manuals, evidence and final completion gate.
13. **Only after the current architecture is done_confirmed:** activate a separate Cellular Automata -> `DefaultNetwork` migration task.

## 7. Recommended PR boundaries

Prefer independent, reversible branches/PRs rather than one large “finish ModalModel” change. A likely decomposition is:

- baseline/audit and missing contract tests;
- core/adapter corrections if proven necessary;
- EFSM conflict policy;
- Graph corrections if proven necessary;
- DTMC corrections if proven necessary;
- CPN pragmatic-subset corrections if proven necessary;
- legacy build isolation/deprecation;
- GUI viewer/core;
- GUI editing slices;
- final verification/documentation.

If an audit finds no code change is necessary for one area, record evidence instead of creating cosmetic changes.

## 8. Completion gate for `done_confirmed`

The current ModalModel/DefaultNetwork initiative may be considered `done_confirmed` only when all **required, non-deferred** scope is demonstrated at the current final commit.

Minimum evidence expected:

- architecture and ownership match the approved network-centered design;
- no required runtime semantics remain in deprecated legacy paths;
- supported `DefaultNetwork` specializations have focused unit tests;
- `ModalModelDefault` adapter semantics are tested end-to-end;
- EFSM three-mode conflict policy is implemented, persisted and tested;
- Graph supported invariants/algorithms are tested;
- DTMC invariants/RNG/reset/persistence are tested;
- the declared pragmatic CPN subset is implemented and tested;
- current-format save/load round trips are green;
- required plugin/factory registrations are green;
- required CMake/Ninja builds are green;
- required `tests-unit`, `tests-kernel-unit` and `tests-smoke` regressions are green or any unrelated baseline failures are explicitly characterized according to governance;
- focused sanitizer evidence is green for ownership/lifetime paths materially changed during the work;
- backend/GUI integration required by the currently approved scope is functional and validated;
- documentation, status, backlog, history/evidence and manuals are synchronized;
- no remaining item classified as **necessary for the approved current scope** is merely assumed complete.

Historical `.gen` compatibility is explicitly excluded from this gate.

Future optional extensions listed in Section 5F do not prevent `done_confirmed` unless separately promoted into the approved current scope.

## 9. Stop and reassess conditions for the local agent

Pause the affected subtask and update the plan rather than forcing the recommendation when local evidence shows any of the following:

- the actual class hierarchy differs materially from this checkpoint;
- another merged change since 2026-09-02 already implements the proposed work;
- moving a legacy class breaks a current registration/factory/API that still has a valid non-legacy role;
- current GUI architecture invalidates a proposed GUI layer;
- a scientific/formalism decision beyond those recorded here is required;
- sanitizer/test failure indicates a broader ownership defect;
- the proposed change would require a broad unrelated refactor.

The correct outcome of a verification step may be “already satisfied”, “recommendation superseded” or “new maintainer decision required”.

## 10. Evidence recording for continuation

For every local phase, record:

- exact branch and commit;
- files/classes inspected;
- observed facts versus hypotheses;
- commands executed;
- toolchain versions when relevant;
- focused test results;
- aggregate regression results;
- sanitizer results when relevant;
- discovered deviations from this plan;
- remaining required work;
- whether the evidence changes task status.

Only evidence from the final relevant commit can support a later `done_confirmed` classification.