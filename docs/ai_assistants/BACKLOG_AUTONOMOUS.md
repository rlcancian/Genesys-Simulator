---
document_type: backlog
authority: executable-task-source
owner: project-maintainer
last_updated: 2026-10-08
review_cadence: on-status-change
status: active
tracks: 511
---

# GenESyS Autonomous Agent Backlog

## 1. Purpose

This is the only approved source for work an AI agent may execute without a new material human decision. A task may run only when its status is `ready`, its environment is available, dependencies are resolved and no maintainer freeze excludes it.

## 2. Status values

- `ready` — fully specified and eligible;
- `running` — owned by one active branch/PR;
- `blocked-review` — prepared but waiting for review;
- `blocked-dependency` — waits for another task or decision;
- `paused` — executable but disabled by maintainer instruction;
- `closed` — development inactive; this does not assert completeness;
- `done_confirmed` — approved scope implemented, tested, functioning, verified, documented and accepted with no known unmet criterion;
- `done` — historical legacy state; do not reinterpret it as `done_confirmed` without revalidation;
- `cancelled` — intentionally removed.

## 3. Completed documentation migration

### AUTO-DOC-001 — Establish canonical governance layer

- Priority: `P0`
- Status: `done`
- Environment: `github`
- Issue/PR: #511 / #512
- Merge: `958cdc6f63c02d004f1ffdf55e104b58a245bb88`
- Validation: run `29929616027`, ordinary tests and GUI GMDD green.

### AUTO-DOC-002 — Consolidate normative governance and architecture

- Priority: `P0`
- Status: `done`
- Environment: `github`
- Issue/PR: #511 / #513
- Merge: `b48697e77d39b25cafc19271ce574bdead60f94d`
- Validation: run `29931594603`, ordinary tests and GUI GMDD green.

### AUTO-DOC-003 — Consolidate current state and plans

- Priority: `P0`
- Status: `done`
- Environment: `github`
- Issue/PR: #511 / #514
- Merge: `53b49f7518509823fe2265a3f017b5aa76f09d2f`
- Validation: run `29933330431`, ordinary tests and GUI GMDD green.

### AUTO-DOC-004 — Consolidate executed evidence

- Priority: `P1`
- Status: `done`
- Environment: `github`
- Issue/PR: #511 / #515
- Merge: `ca910a2fbe4504ef8520ef48b8b377da7e9e02ca`
- Validation: run `29934227250`, ordinary tests and GUI GMDD green.

### AUTO-DOC-005 — Consolidate technical references and active root

- Priority: `P0`
- Status: `done`
- Environment: `github`
- Issue/PR: #511 / #516
- Merge: `d375d9e68e5c1dc84e214a772fb15cb05944f0d8`
- Validation: run `29936506990`, ordinary tests and GUI GMDD green.
- Result: six technical references, canonical routing and removal of superseded active guides, plans and redirects.

### AUTO-DOC-007 — Consolidate retained oldies governance

- Priority: `P0`
- Status: `done`
- Environment: `github`
- Issue/PR: #511 / #517
- Merge: `c9c76c3d62633b69a7d18d899aa764b7ebdf69a5`
- Validation: run `29938004455`, ordinary tests and GUI GMDD green.
- Result: one tracker for 25 retained files; every file remains review-pending and not deletion-ready.

### AUTO-DOC-006 — Enforce documentation governance

- Priority: `P0`
- Status: `done`
- Environment: `github` or `local`
- Issue/PR: #511 / #518
- Merge: `610d8ab21c87cfd11663af78370b39262cf4da81`
- Validation:
  - documentation-governance run `29938886903`: passed;
  - ordinary CI run `29938886807`: configure, build, CTest and GUI GMDD passed.
- Result:
  - `scripts/validate-ai-docs.py` validates the final structure locally;
  - `.github/workflows/genesys-docs-governance.yml` enforces it on relevant PRs;
  - exact root allowlist, links, front matter, backlog IDs, evidence placement and oldies retention are checked;
  - source branch removed automatically.

## 4. Completed application, packaging and Arena-compatibility work

### AUTO-APP-003 — Implement per-user runtime launcher and dispatcher

- Priority: `P0`
- Status: `done`
- Environment: `github`
- Issue/PR: #521
- Merge: `d88b4b20b5163891e47c9e63bb04eca68a9e40d0`
- Validation: Launcher CI green (33 focused tests, dispatcher isolation, install contract), ordinary regression CI green on final PR head.
- Scope delivered:
  - `source/applications/launcher/` with `genesys-launcher` and `genesys-dispatch` (Qt6/C++23);
  - XDG configuration/runtime paths, deterministic user/system runtime selection, update manifest/version/platform validation, bounded HTTPS transport, SHA-256 verification, fail-closed signature verification, safe archive extraction, partial install, atomic activation, retention, logging and non-shell (`execv`) process launch;
  - `genesys-gui`, `genesys-shell`, canonical `genesys-worker`, legacy `genesys-web` compatibility, and existing Python `genesys-mcp` entry point.
- Non-goals honored: no Debian package layout/wrapper/control/install-file redesign in this PR (delivered separately by `AUTO-PKG-001`/PR #522); no release bundle publication/signing workflow; no Worker authentication/network exposure redesign; no MCP rewrite or Python package installation.
- Reconciled 2026-08-20 while executing `AUTO-PKG-001`: this task was recorded `running` after its PR had already merged; re-verified against current `source/applications/launcher/` and a fresh green Launcher CI run before marking `done`.

### AUTO-PKG-001 — Execute Debian package lifecycle validation

- Priority: `P1`
- Status: `done`
- Environment: `github` and `local`
- Issue/PR: #522
- Merge: `d8fce9562617525657e8cbf9870b32323364773f`
- Authorization: this task was recorded `paused` (see historical Section 6 baseline); explicit maintainer instruction dated 2026-08-20 authorized executing it, including integrating the Launcher (`AUTO-APP-003`) into the Debian package, fixing the discovered Lintian and lifecycle-script defects, and merging when all objective criteria were satisfied.
- Scope delivered:
  - integrated the stable Launcher/Dispatcher into the Debian install tree (`/usr/libexec/genesys/`), added the `genesys-common` package (launcher, dispatcher, `/etc/genesys/update.conf`, public `genesys-mcp` entry) and split the GUI/Shell/Worker packages into public wrapper + Debian-provided system fallback, keeping `genesys-web` as a payload-free transitional package;
  - fixed real Lintian findings on GitHub Actions run `32407760833`: `depends-on-essential-package-without-using-version` (dropped an unneeded explicit `tar` dependency on an Essential package), `copyright-without-copyright-notice` (added copyright years), `no-manual-page` (added five section-1 manual pages); confirmed by re-inspection that `hardening-no-pie` and `possible-gpl-code-linked-with-openssl` do not occur on this branch (`readelf` confirms PIE binaries with no OpenSSL linkage);
  - found and fixed four real, previously unexercised defects in `packaging/linux/validate-debian-lifecycle.sh` (a `set -e`/`pgrep && fail` function-return bug, missing `sudo` on five per-user-home path checks, an insufficiently bounded GUI-startup poll, and `apt-get purge` failing to resolve locally-installed no-conffile packages by name) — the lifecycle job had never previously run to completion because the build job was always blocked earlier by Lintian;
  - validated locally (disposable Ubuntu 24.04 Docker container with `CAP_SYS_PTRACE`, matching GitHub Actions runner fidelity) and on GitHub Actions run `32421072334`: `dpkg-buildpackage` produces exactly 5 packages, `lintian --fail-on error` and `lintian` (no filter) report zero findings, `appstreamcli validate --no-net` passes, and the full lifecycle script (install, ownership, non-root user, system fallback, per-user runtime, admin-policy override, six invalid-runtime cases, file-integrity, reinstall/conffile, remove, purge) completes with exit 0;
  - updated `packaging/linux/README.md` and the Developer/User manual chapters (`chapter_packaging_and_ci.tex`, `chapter_user_installation.tex`) to document the current five-package split and the Launcher/system-fallback/per-user-runtime contract; regenerated `docs/ManualGenESyS.pdf`.
- Non-goals honored: no GitHub Release, PPA/APT repository, package signing, runtime signing key, or promotion beyond `WorkInProgress`.

### AUTO-ARENA-001 — Close Arena compatibility audit for Advanced Transfer and parser-variable scope

- Priority: `P1`
- Status: `done`
- Environment: `local`
- Branch: `WiP202608/arena-advanced-transfer-closeout` (merged, deleted)
- Base: `origin/WorkInProgress` at `b4fe16289b1aa4d0924e979cf795c8cc4d562e88`
- Merge: PR #527, merged 2026-08-30 into `WorkInProgress` at
  `ebd6f30e8c662ae74da6f5745472235090a830ca` (merge commit), current
  `WorkInProgress` head after this and one follow-up documentation commit:
  `883046e22`.
- Authorization: explicit maintainer continuation instruction dated 2026-08-30 authorized resuming the advanced Arena ↔ GenESyS front from the existing checkpoint without restarting Phase A or the already-closed Basic/Advanced Process audit.
- Scope:
  - reconcile `Distance` and `Segment` with the current code, registration, tests and the compatibility matrix;
  - finish the remaining Advanced Transfer audit (`Enter`, `Leave`, `PickStation`, `Route`, `Station` as flowchart concept, conveyor stubs, transporter/network gaps) and classify Flow Process without broad re-audits;
  - execute Phase C for the real parser/Arena Variables Guide overlap only, documenting implemented, partial, unsupported and future items;
  - implement only the smallest justified fixes/additions needed to close proven in-scope gaps with tests.
- Non-goals:
  - no restart of the already completed Phase A / Basic Process / Advanced Process analysis;
  - no broad plugin-architecture refactor, dynamic-plugin migration, incidental modernization, or premature ModalModel work;
  - no implementation of unsupported Arena features merely to reach nominal 100% parity.
- Acceptance:
  - `docs/ai_assistants/reference/ARENA_GENESYS_COMPATIBILITY.md` reconciled with the current code and final area statuses;
  - new/changed in-scope data definitions or components registered, persistible and covered by focused tests;
  - parser/Arena variables mapping documented from the real current pipeline;
  - focused validation green, plus required aggregate regression levels for any code changes performed;
  - remaining human decisions and future features isolated explicitly.
- Delivered:
  - `Distance`/`Segment` reconciliation, `Route` persistence/check fixes, minimum executable `Storage`/`Store`/`Unstore`/`DropOff`, minimum executable `Conveyor`/`Transporter` plus `Access`/`Exit`/`Start`/`Stop`/`Move`, and Phase C parser/Arena variables mapping;
  - documentation updated: compatibility matrix closed through Advanced Transfer, Flow Process classification and parser-variable scope, plus dedicated maintainer decision note for `Process::AllocationType` versus internal `Delay`
    ([`reference/PROCESS_ALLOCATIONTYPE_DELAY_DECISION.md`](reference/PROCESS_ALLOCATIONTYPE_DELAY_DECISION.md));
  - `Network`, `NetworkLink`, `ActivityArea` and CTMC-class features were
    intentionally left out of scope and remain documented as future/not-applicable
    in the compatibility matrix.
- Validation at merge time: focused Arena/MaterialHandling tests green (`28/28`), `tests-smoke` green (`3/3`), `tests-kernel-unit` green except the preexisting `genesys_test_optimizer_ownership_contract_NOT_BUILT`, and `tests-unit` still shows 13 unrelated WholeCell/Bio failures tied to a plugin-loading path involving `attribute.so`, outside this scope — these remain untriaged and are not yet a dedicated backlog entry; a maintainer should decide whether to open one before relying on `tests-unit` as a green gate.

### AUTO-MODAL-001 — Migrate ModalModel onto the Network bridge

- Priority: `P1`
- Status: `closed`
- Environment: `local`
- Branch: `WiP202608/modal-network-implementation`
- Merge: PR #528, merged 2026-08-31 into `WorkInProgress` at `e0185163` (docs sync + architecture consolidation), and PR #529, merged 2026-08-31 into `WorkInProgress` at `fdae135b` (ModalModel/network architecture, Phases 1-8), current `WorkInProgress` head after both merges: `fdae135b3`.
- Base: `origin/WorkInProgress` at `f40d067fd15c3fb57275dedd9852a7fe4100b2ce`
- Latest validated implementation commit: `4e5f4201` (Phase 8 compatibility-cleanup validation checkpoint); `d55a8fba` (proposed GUI architecture, docs-only, added and pushed 2026-08-31 after a session handoff -- re-verified: full build green, `ctest --preset tests-kernel-unit` 1810/1810 executed tests passed, 0 failed, 4 preexisting disabled)
- Push state: all commits through `d55a8fba` are pushed to `origin/WiP202608/modal-network-implementation` (branch was previously 23 commits ahead of `origin` only locally; pushed 2026-08-31 to avoid loss across a session/provider handoff)
- Authorization: explicit maintainer continuation instruction on 2026-08-30 authorized resuming the ModalModel/network architecture from the current `DefaultNetwork` checkpoint without restarting the earlier phase-0 inventory.
- Scope:
  - migrate `DefaultNode` from `ModelComponent` to `ModelDataDefinition`;
  - remove process-connection/dispatch semantics from the node layer;
  - preserve persistence and plugin registration compatibility for existing modal-model models;
  - keep `ModalModelDefault`/`ModalModelFSM`/`ModalModelPetriNet` behavior intact until the generic `ModalModel` adapter phase;
  - introduce the first formalism-owned network specialization (`EFSMNetwork`) with deterministic one-step activation semantics;
  - introduce structural mathematical graph networks as `DefaultNetwork` specializations without process-flow `Connection` semantics;
  - introduce finite time-homogeneous DTMC support through `MarkovChainNetwork`;
  - introduce a pragmatic fixed-inscription CPN subset through `ColoredPetriNetNetwork`;
  - add focused tests for the new node contract and any persistence/check regressions touched by the migration.
- Non-goals:
  - no full CPN variable-binding/type-system implementation yet;
  - no CPN maximal-concurrent-step firing implementation yet;
  - no CTMC, controlled Markov chain, Markov decision process or time-inhomogeneous Markov process implementation yet;
  - no graph traversal simulation, random walk, graph routing, spatial-network behavior or agent movement implementation yet;
  - no full generic `ModalModel` adapter rewrite beyond the current `ModalModelDefault` bridge;
  - no broad plugin-architecture refactor outside the modal-model path;
  - no restart of the architecture phase-1/phase-2 `DefaultNetwork` work.
- Acceptance:
  - `DefaultNode` compiles as a data-definition-based modal element;
  - `EFSMNetwork` compiles as a data-definition-based network specialization and is registered in the dummy/static plugin connector;
  - `GraphNetwork`, `DirectedGraphNetwork`, `DirectedAcyclicGraphNetwork`, `GraphNode` and `GraphEdge` compile as data-definition-based graph abstractions and are registered in the dummy/static plugin connector;
  - `MarkovChainNetwork` and `MarkovState` compile as data-definition-based DTMC abstractions and are registered in the dummy/static plugin connector;
  - `ColoredPetriNetNetwork`, `CPNTransition` and `CPNArc` compile as data-definition-based fixed-inscription CPN abstractions and are registered in the dummy/static plugin connector;
  - plugin loading/persistence remains backward-compatible for the current branch scope;
  - focused tests cover the migrated node contract, network bridge, EFSM activation/persistence contracts, graph topology/algorithm/persistence contracts, DTMC activation/persistence/validation contracts and CPN fixed-inscription activation/persistence/validation contracts and continue to pass;
  - remaining architecture follow-ups are left isolated for the next phase.
- Progress snapshot:
  - `DefaultNetwork` is already in place with network activation frame/result, port schema and activation counter;
  - `DefaultNode` has now moved to `ModelDataDefinition`, its old dispatch hook was removed, and node persistence now uses the data-definition serialization path;
  - `ModalModelDefault` now has an optional `DefaultNetwork` bridge with network reference persistence, input/output bindings, activation, zero-output consume semantics, one-output routing, and multi-output cloning;
  - `EFSMNetwork` now owns FSM states/transitions, default `input`/`output` ports, current/initial state, deterministic priority selection, output publication, replication reset and persistence round-trip;
  - `GraphNetwork` now represents structural mathematical graphs with `GraphNode`/`GraphEdge`, undirected and directed semantics, optional weights, self-loops, parallel edges, BFS, DFS, reachability, unweighted shortest path, Dijkstra for nonnegative weights, connected components, Tarjan SCC, cycle detection, DAG cycle rejection and topological order;
  - `MarkovChainNetwork` now represents finite time-homogeneous DTMCs with network-owned current/initial state, fixed transition probabilities, row-stochastic validation, one-step activation, kernel sampler usage, output of selected state index, replication reset and persistence round-trip;
  - `ColoredPetriNetNetwork` now represents a fixed-inscription CPN subset with bipartite `PetriPlace`/`CPNTransition`/`CPNArc` topology, symbolic-color token multisets, parser-based guards, deterministic single firing, atomic consume/produce semantics, initial marking reset, persistence round-trip and shared-network activation;
  - `DefaultNetwork::_loadInstance()` now replaces an existing port schema instead of accumulating stale/default ports during load, which keeps subclass persistence round-trips stable;
  - the legacy `ModalModelDefault` node-list execution path now uses a resettable GenESyS kernel sampler instead of `std::rand()` for proportional probabilistic transition selection, preserving the temporary compatibility path while aligning stochastic choices with the new network formalism policy;
  - `ModalModelFSM` no longer allocates an unused hidden `FSMState`, and both `ModalModelFSM` and `ModalModelPetriNet` now validate through the attached `DefaultNetwork` bridge without requiring obsolete legacy nodes when a network is configured;
  - `tests/unit/test_default_network.cpp`, `tests/unit/test_default_node.cpp`, `tests/unit/test_modal_model_default_network.cpp`, `tests/unit/test_efsm_network.cpp`, `tests/unit/test_graph_network.cpp`, `tests/unit/test_markov_chain_network.cpp`, and `tests/unit/test_colored_petri_net_network.cpp` cover the base network abstraction, migrated node contract, generic adapter bridge, shared-network activation, invalid network references, EFSM activation, EFSM priority, EFSM reset, EFSM persistence, GraphNetwork topology/algorithms/persistence, MarkovChainNetwork DTMC behavior, ColoredPetriNetNetwork fixed-inscription CPN behavior and plugin metadata;
  - validation snapshot on 2026-08-30 after the GraphNetwork documentation update: `cmake --build build/tests-unit -j2 --target genesys_kernel_unit_tests` passed; `ctest --test-dir build/tests-unit -R 'GraphNetwork|EFSMNetwork|ModalModelDefaultNetwork|DefaultNode|DefaultNetwork' --output-on-failure` passed with 44/44 focused tests; `ctest --test-dir build/tests-unit --output-on-failure` passed with 1789/1789 executed tests and 4 preexisting disabled tests;
  - validation snapshot on 2026-08-31 after `MarkovChainNetwork`: `cmake --build build/tests-unit -j2 --target genesys_test_markov_chain_network` passed; `ctest --test-dir build/tests-unit -R 'MarkovChainNetwork' --output-on-failure` passed with 9/9 focused tests; `ctest --test-dir build/tests-unit -R 'MarkovChainNetwork|GraphNetwork|EFSMNetwork|ModalModelDefaultNetwork|DefaultNode|DefaultNetwork' --output-on-failure` passed with 53/53 modal/network tests; `cmake --build build/tests-unit -j2 --target genesys_kernel_unit_tests` passed; `ctest --test-dir build/tests-unit --output-on-failure` passed with 1798/1798 executed tests and 4 preexisting disabled tests;
  - validation snapshot on 2026-08-31 after `ColoredPetriNetNetwork`: `cmake --build build/tests-unit -j2 --target genesys_test_colored_petri_net_network` passed; `ctest --test-dir build/tests-unit -R 'ColoredPetriNetNetwork' --output-on-failure` passed with 10/10 focused tests; `ctest --test-dir build/tests-unit -R 'ColoredPetriNetNetwork|MarkovChainNetwork|GraphNetwork|EFSMNetwork|ModalModelDefaultNetwork|DefaultNode|DefaultNetwork' --output-on-failure` passed with 63/63 modal/network tests; `cmake --build build/tests-unit -j2 --target genesys_kernel_unit_tests` passed; `ctest --test-dir build/tests-unit --output-on-failure` passed with 1808/1808 executed tests and 4 preexisting disabled tests;
  - validation snapshot on 2026-08-31 after the legacy sampler cleanup: `cmake --build build/tests-unit -j2 --target genesys_test_modal_model_default_network` passed; `ctest --test-dir build/tests-unit -R 'LegacyProbabilisticSelectionUsesResettableKernelSampler' --output-on-failure` passed with 1/1 focused test; `ctest --test-dir build/tests-unit -R 'ColoredPetriNetNetwork|MarkovChainNetwork|GraphNetwork|EFSMNetwork|ModalModelDefaultNetwork|DefaultNode|DefaultNetwork' --output-on-failure` passed with 64/64 modal/network tests;
  - validation snapshot on 2026-08-31 after wrapper shim cleanup: `cmake --build build/tests-unit -j2 --target genesys_test_modal_model_default_network` passed; `ctest --test-dir build/tests-unit -R 'ModalModelDefaultNetwork' --output-on-failure` passed with 8/8 bridge/shim tests;
  - full regression snapshot on 2026-08-31 after Phase 8 safe compatibility cleanup: `cmake --build build/tests-unit -j2 --target genesys_kernel_unit_tests` passed; `ctest --test-dir build/tests-unit --output-on-failure` passed with 1810/1810 executed tests and 4 preexisting disabled tests;
  - remaining work: GUI/editor synchronization for network ports and bindings, cleanup/migration strategy for legacy `ModalModelFSM`/`ModalModelPetriNet` wrappers, and future full CPN variable-binding/type-system/multi-firing semantics. This historical record is not a completion claim.

#### GraphNetwork extension

##### Architecture

- GraphNetwork: structural mathematical graph specialization of `DefaultNetwork`; inherited `activate()` remains inert and does not imply traversal, firing, movement or current vertex.
- GraphNode: `DefaultNode` subclass used as a persistable graph vertex and not as a process-flow `ModelComponent`.
- GraphEdge: separate `ModelDataDefinition` for graph incidence, endpoints, direction and optional numeric weight; it is not a GenESyS `Connection`.
- reuse of DefaultNode/DefaultTransition: `GraphNode` reuses `DefaultNode`; `GraphEdge` deliberately does not reuse `DefaultNodeTransition` because transition guards/actions/probability semantics are not mathematical edge semantics.
- directed/undirected representation: `GraphNetwork` is undirected; `DirectedGraphNetwork` overrides orientation-sensitive adjacency; `DirectedAcyclicGraphNetwork` extends directed graphs with a no-cycle invariant.

##### Features

- self loops: supported; directed self-loop contributes one in-edge and one out-edge and is detected as a directed cycle.
- parallel edges: supported; edge identity is independent from `(source,destination)` and weights remain independent.
- weights: optional numeric edge weight; absent weight is treated as cost `1.0` by shortest-path algorithms.
- adjacency: deterministic insertion-order neighbors, predecessors, successors, incoming/outgoing and incident-edge queries.
- degree: undirected degree counts self-loop as two; directed in-degree/out-degree count incoming/outgoing edge multiplicity.

##### Algorithms

- BFS: implemented with deterministic insertion-order traversal and predecessor/distance records.
- DFS: implemented with deterministic insertion-order traversal.
- reachability: implemented via BFS and respects directed orientation.
- unweighted shortest path: implemented via BFS with node and edge path reconstruction.
- Dijkstra: implemented for nonnegative weights and rejects negative weights with a diagnostic.
- Bellman-Ford: not implemented; negative-weight shortest paths are explicitly rejected for now.
- connected components: implemented for undirected `GraphNetwork`.
- strongly connected components: implemented for `DirectedGraphNetwork` using Tarjan DFS.
- cycle detection: implemented for undirected and directed graphs.
- topological sorting: implemented for directed graphs with Kahn-style order; DAG subclass rejects cycle-closing insertion.

##### Persistence

- fields: `graphDirected`, `graphNodesSize`, `graphNodeN.*`, `graphEdgesSize`, `graphEdgeN.*`, plus `GraphEdge` endpoint/direction/weight fields.
- round-trip validation: focused tests cover directed weighted multigraph persistence with isolated node, self-loop and parallel edges.

##### Tests

- focused: `genesys_test_graph_network` and CTest regex `GraphNetwork`.
- regression: focused ModalModel/Network regex `GraphNetwork|EFSMNetwork|ModalModelDefaultNetwork|DefaultNode|DefaultNetwork` passed after implementation.
- sanitizers if applicable: not run for this structural graph pass; no dedicated sanitizer target was added.

##### Limitations

- `GraphNetwork` stores non-owning topology references to model-managed `GraphNode` and `GraphEdge` data definitions, matching the current ModelDataManager ownership style; external deletion must still be coordinated through graph APIs to avoid stale topology references.
- Bellman-Ford and negative-cycle shortest-path analysis are deferred; Dijkstra rejects negative weights.

##### Future extensions

- GraphTraversalNetwork;
- RandomWalkNetwork;
- RoutingNetwork;
- agent movement;
- spatial networks;
- generalized Connection/token integration.

#### MarkovChainNetwork extension

##### Architecture

- MarkovChainNetwork: finite time-homogeneous DTMC specialization of `DefaultNetwork`; one activation performs exactly one Markov step.
- MarkovState: `DefaultNode` subclass used as a finite DTMC state and not as a process-flow `ModelComponent`.
- MarkovTransition: internal transition relation from source state to destination state with fixed numeric probability.
- legacy component boundary: `AnalyticalModeling/MarkovChain` remains a separate process component that reads/writes entity-associated data; it is not the new network-owned DTMC implementation.

##### Features

- current state: owned by `MarkovChainNetwork`, not by `Entity` attributes.
- initial state: persisted and restored between replications.
- probabilities: fixed numeric transition probabilities for the initial strict DTMC implementation.
- row validation: every state's outgoing probabilities must sum to 1.0 within configured tolerance.
- output: default `state` output publishes the selected state index after activation.

##### Algorithms

- transition selection: uses the GenESyS sampler infrastructure, not `std::rand()`.
- deterministic transitions: probability-one rows are supported.
- absorbing states: represented by a self-transition with probability `1.0`.
- empirical sanity: focused tests verify approximate frequencies for a two-branch stochastic row.

##### Persistence

- fields: `probabilityTolerance`, `initialState`, `currentState`, `markovStatesSize`, `markovStateN.*`, `markovTransitionsSize`, `markovTransitionSourceN`, `markovTransitionDestinationN`, `markovTransitionNameN`, and `markovTransitionProbabilityN`.
- precision: small probability/tolerance values are saved with explicit string precision to avoid `std::to_string` rounding to zero.
- round-trip validation: focused tests cover states, transitions, probabilities, ports, tolerance, current state and initial state.

##### Tests

- focused: `genesys_test_markov_chain_network` and CTest regex `MarkovChainNetwork`.
- regression: focused ModalModel/Network regex `MarkovChainNetwork|GraphNetwork|EFSMNetwork|ModalModelDefaultNetwork|DefaultNode|DefaultNetwork` passed after implementation.
- shared activation: focused tests activate one chain through two `ModalModelDefault` adapters and verify network-owned state progression.

##### Limitations

- no CTMC support.
- no controlled Markov chain, Markov decision process or time-inhomogeneous transition update semantics.
- no state-specific output mapping beyond the default selected-state-index output.
- `MarkovTransition` is currently an internal network relation rather than a standalone plugin data definition.

##### Future extensions

- initial distribution sampling rather than only a single initial state;
- controlled Markov chains;
- Markov decision processes;
- CTMC with event-calendar integration;
- richer output mappings per state or transition.

#### ColoredPetriNetNetwork extension

##### Architecture

- ColoredPetriNetNetwork: fixed-inscription CPN subset specialization of `DefaultNetwork`; one activation fires at most one enabled transition.
- PetriPlace: reused as the place data definition with symbolic-color token multiset storage.
- CPNTransition: explicit transition node with guard expression and priority.
- CPNArc: explicit directed bipartite arc between one `PetriPlace` and one `CPNTransition`; it is not a place-to-place edge.
- firing mode: currently `SingleDeterministic` only.

##### Features

- color sets: symbolic color names only in this subset.
- token values: represented as multiset counts per symbolic color; typed token payloads are deferred.
- inscriptions: fixed `color -> quantity` inscriptions on arcs.
- guards: optional parser expressions on transitions without CPN variable bindings.
- marking: observable marking belongs to `ColoredPetriNetNetwork` and is stored in its places for compatibility with the existing data-definition lifecycle.

##### Algorithms

- enabling: checks all fixed input inscriptions and transition guard.
- firing: atomically consumes input multisets and produces output multisets.
- conflict selection: deterministic by transition priority, preserving insertion order for equal priorities.
- multi-firing: not implemented; maximal concurrent-step semantics remain future work.

##### Persistence

- fields: default network ports, firing mode, `cpnPlaceN.*`, `cpnTransitionN.*`, `cpnArcN.*`, current place markings and explicit initial marking entries.
- round-trip validation: focused tests cover bipartite topology, arc inscriptions, guards/priorities, current marking and initial marking.

##### Tests

- focused: `genesys_test_colored_petri_net_network` and CTest regex `ColoredPetriNetNetwork`.
- regression: focused ModalModel/Network regex `ColoredPetriNetNetwork|MarkovChainNetwork|GraphNetwork|EFSMNetwork|ModalModelDefaultNetwork|DefaultNode|DefaultNetwork` passed after implementation.
- shared activation: focused tests activate one CPN through two `ModalModelDefault` adapters and verify network-owned marking.

##### Limitations

- no typed token payloads.
- no CPN variable binding search.
- no evaluated arc inscriptions beyond fixed symbolic-color multiplicities.
- no maximal concurrent-step or stochastic conflict resolution.
- `PetriTransition` legacy binary source/destination helper remains separate and is not the new CPN transition node.

##### Future extensions

- typed color sets and token values;
- binding enumeration;
- expression-based arc inscriptions;
- maximal concurrent-step firing;
- stochastic conflict resolution through the GenESyS sampler;
- CPN reference fixtures from literature.

## 5. Active bounded work

### AUTO-MODAL-002 — Verify and complete the current ModalModel/DefaultNetwork architecture

- Priority: `P1`
- Status: `done_confirmed`
- Environment: `local`
- Base: `origin/WorkInProgress` at `5bd10bb5` (after #541)
- Branch/worktree: continuation through #536–#541; docs close-out on `WiP202609/modal-backend-gate-docs`
- Authorization: explicit maintainer continuation instruction dated 2026-09-19; backend-gate continuation 2026-09-20.
- Dependency: `AUTO-MODAL-001` is closed; current-HEAD verification completed before and after #540/#541.
- Scope: verify the current `DefaultNetwork` contract and `ModalModelDefault` adapter; validate current persistence and plugin registration; audit EFSM, Graph, finite homogeneous DTMC and pragmatic CPN behavior; implement only confirmed gaps; remove obsolete Modal/Network source classes only after dependency proof; defer Cellular Automata migration until this task is genuinely complete.
- Non-goals: historical `.gen` compatibility, full academic CPN semantics, CTMC/MDP/time-inhomogeneous Markov models, graph movement/routing simulation, broad plugin redesign, Cellular Automata migration, and GUI (explicit backend-gate stop).
- Acceptance: all approved **backend** in-scope behavior is implemented, compiled, tested, runtime-verified, documented and integrated in `WorkInProgress`; no known backend-gate criterion remains pending. GUI remains outside this task (`HUM-MODAL-002`) and still blocks **global** Modal/Network architecture `done_confirmed` per the completion plan.
- Progress (2026-09-20 close-out):
  - #536/#537: identity + full Model round-trips Graph/DTMC/CPN/Modal;
  - #538/#540: List/transition ownership (ASan/LSan focused paths exit 0 excluding process-lifetime Buffer noise);
  - #541: physical removal of `ModalModelFSM`/`ModalModelPetriNet`/node-list/`PetriTransition`; smart apps migrated;
  - focused Modal/Network: 73/73; unit 1830/1830 passed (+4 disabled); smoke 3/3; CI #541 green.
- Current evidence (executed 2026-09-20 on HEAD `5bd10bb5`, Ubuntu 24.04, g++ 13.3.0, CMake 3.28.3, Ninja 1.11.1):
  - `tests-unit`: 1,834 registered; 1,830 executed/passed; 0 failed; 4 disabled;
  - `tests-kernel-unit`: 1,834 registered; 1,830 executed/passed; 0 failed; 4 disabled;
  - `tests-smoke`: 3/3 passed;
  - focused Modal/Network regex `ColoredPetriNetNetwork|MarkovChainNetwork|GraphNetwork|EFSMNetwork|ModalModelDefaultNetwork|DefaultNode|DefaultNetwork`: 73/73 passed;
  - evidence: `history/evidence/2026/09/2026-09-20_modal_backend_gate_closure.md`.

`AUTO-MODAL-001` remains in Section 4 as a closed historical development cycle, not as `done_confirmed`.

## 5.1. Scientific rigor program — biochemical and WholeCell models (S0–S13)

Maintainer authorization: 2026-10-08. The program is sequential: execute exactly one bounded microtask per hourly iteration, continue the same microtask until its PR is integrated and verified, and only begin its successor in a later iteration. A stage may be subdivided into smaller PRs, but never run concurrently with its unfinished predecessor. S0 is the documentation-only PR #543, already merged in WorkInProgress at 065c5adb58b1435efae9c31c37f3b92d6a0f714c; this record does not reclassify any unverified scientific behavior as validated.

Common execution contract for AUTO-SCI-001 through AUTO-SCI-013:
- Integration: start from the current real WorkInProgress HEAD, use a small WiPYYYYMM/<scope> branch, open a draft PR targeting WorkInProgress, inspect final-head CI, mandatory checks, review threads, mergeability, and required artifacts; merge only after every gate passes; verify post-merge HEAD; remove the source branch when safe.
- Evidence: distinguish current-code inspection, executed validation, canonical documentation, strong indication, hypothesis, and maintainer decision. Scientific references and analytical/numerical oracles precede behavioral implementation. Build/test green never implies predictive validity.
- Documentation and closure: reconcile the applicable canonical backlog, STATUS, AI changelog, scientific evidence, and affected manuals; mark done_confirmed only when all in-scope acceptance criteria are demonstrated, reviewed, and integrated.
- Shared stop gate: stop on red/unknown mandatory CI, material review, conflicting branch, missing scientific reference/oracle, unsafe persistence or ownership change, unresolved human scientific/architectural decision, or environment incapable of required validation. Record the exact blocker; do not start another stage.
- Scope boundary: backend before GUI; preserve existing DTMC; do not initiate Cellular Automata, MDP, RDME, Chemical Langevin, advanced tau-leaping, broad SBML redesign, or cosmetic refactoring.

### AUTO-SCI-001 (S1) — Consolidated scientific model inventory

- Priority: P1
- Status: ready
- Environment: github (documentation inventory; current source inspection and CI evidence), or local.
- Dependency: S0 / PR #543 integrated; no scientific runtime decision required for inventory.
- Scope: inspect current WholeCellModeling, BiochemicalSimulation, source/tools/Biochemical, Modal/Network, Continuous, tests and models. Produce a canonical per-mechanism matrix of mathematical formulation, state, variables/units, parameters, time, RNG, solver, persistence, references, tests/oracles, scientific claim level and gaps. Cover MolecularSpecies/WholeCellState/BioCompartment, SSA/reaction rules, stochastic gene expression, ODE/kinetics, steady state/FBA, growth/division/cycle/compartments/projections, DTMC and planned CTMC.
- Acceptance: source-linked matrix and explicit uncertainties are committed; plan/status/evidence are reconciled; documentation governance and required CI pass; PR is reviewed, merged and post-merge HEAD verified.
- Non-goals: implement or silently repair algorithms; choose CTMC clock policy; promote scientific claims.
- Stop: missing source or unsafe/incomplete repository read must be reported, not guessed.

### AUTO-SCI-002 (S2) — Stochastic reaction scientific contract

- Priority: P1
- Status: blocked-dependency
- Environment: local or github with approved executable CI.
- Dependency: AUTO-SCI-001 done_confirmed.
- Scope: audit StochasticReactionRule, species and stoichiometry; analytically test zero-, uni- and bimolecular reactions, repeated reactants (2A), insufficient reagents, non-negativity, conservation where applicable and microscopic rate units.
- Acceptance: explicit reference/units contract, focused oracles and passing relevant regression; production corrections only for demonstrated defects, integrated through gated PR.
- Non-goals: introduce new CTMC engine.
- Stop: unresolved stochastic parameterization or failed oracle requires escalation.

### AUTO-SCI-003 (S3) — CTMC event-time and RNG contract

- Priority: P1
- Status: blocked-dependency
- Environment: local or github with approved executable CI.
- Dependency: AUTO-SCI-002 done_confirmed.
- Scope: audit GenESyS event calendar, ModalModelDefault, DefaultNetwork, WholeCell clocks and RNG; specify CTMC holding-time scheduling, reset/seed reproducibility and explicit legacy windowed-SSA coexistence without hidden clock drift.
- Acceptance: evidence-backed temporal/RNG contract and reproducibility tests precede any runtime change; document resolved boundaries.
- Non-goals: silently choose between materially different event-calendar architectures.
- Stop: if HUM-SCI-002 requires a material maintainer decision, record exact options/tradeoffs, STOP, and notify the maintainer. Resume only after the decision is recorded; do not block S1/S2 preemptively.

### AUTO-SCI-004 (S4) — CTMC analytical oracle package

- Priority: P1
- Status: blocked-dependency
- Environment: local or github with approved executable CI.
- Dependency: AUTO-SCI-003 done_confirmed.
- Scope: reference fixtures for first-order A->B, N-molecule binomial decay, competing A->B/A->C, reversible two-state CTMC, combinatorics, absorbing states, generator Q invariants, seed/reset and persistence.
- Acceptance: distinguish exact analytical assertions from ensemble Monte Carlo tests, justify tolerances, and integrate passing oracle suite before runtime implementation.
- Non-goals: implement CTMC network.
- Stop: unreferenced expected results or unjustified statistical tolerances.

### AUTO-SCI-005 (S5) — CTMC network backend

- Priority: P1
- Status: blocked-dependency
- Environment: local or github with approved executable CI.
- Dependency: AUTO-SCI-004 done_confirmed.
- Scope: implement smallest distinct CTMC specialization compatible with DefaultNetwork, preserving DTMC semantics; discrete state, channels/propensities, exponential holding times, channel choice, reset, persistence, factory registration and diagnostics. Split S5a/S5b if required.
- Acceptance: S4 oracles and focused/regression/persistence tests pass; required runtime evidence and gated PR integration.
- Non-goals: reinterpret MarkovChainNetwork probabilities as rates.
- Stop: unresolved time integration, public API or persistence semantics.

### AUTO-SCI-006 (S6) — SSA/CTMC/WholeCell consolidation

- Priority: P1
- Status: blocked-dependency
- Environment: local or github with approved executable CI.
- Dependency: AUTO-SCI-005 done_confirmed.
- Scope: reconcile StochasticReactionComponent with CTMC reaction/propensity kernel, RNG/reset and shared scientific semantics while retaining the legacy windowed path until equivalence is demonstrated.
- Acceptance: reference-equivalent supported cases, reproducibility and compatibility regressions, gated PR integration.
- Non-goals: remove legacy path without migration evidence.
- Stop: behavior divergence without explained contract or safe migration.

### AUTO-SCI-007 (S7) — Deterministic ODE and kinetic-law rigor

- Priority: P1
- Status: blocked-dependency
- Environment: local or github with approved executable CI.
- Dependency: AUTO-SCI-006 done_confirmed.
- Scope: audit MassActionOdeSystem, BioKineticLawExpression, BioSimulate and solvers; test first-order decay, reversible reactions, conservation, coupled systems, amount/concentration/volume units, convergence and invalid/nonfinite behavior.
- Acceptance: mathematical reference and passing numerical/regression evidence; corrections only for demonstrated gaps; gated PR integration.
- Non-goals: unrelated solver redesign.
- Stop: unresolved dimensional conventions or scientific formulation.

### AUTO-SCI-008 (S8) — Steady-state and FBA rigor

- Priority: P1
- Status: blocked-dependency
- Environment: local or github with approved executable CI.
- Dependency: AUTO-SCI-007 done_confirmed.
- Scope: audit MetabolicFluxBalanceSolver, GLPK and BioSteadyState, Sv=0, bounds, objective, flux units, infeasible/unbounded handling, small analytical LP fixtures and optional-dependency/persistence behavior.
- Acceptance: reference-backed focused tests and regression, limitations and claim level documented, gated PR integration.
- Non-goals: equate LP correctness with biological validation.
- Stop: unresolved objective/unit semantics or unsupported solver behavior.

### AUTO-SCI-009 (S9) — Stochastic gene expression rigor

- Priority: P1
- Status: blocked-dependency
- Environment: local or github with approved executable CI.
- Dependency: AUTO-SCI-008 done_confirmed.
- Scope: audit StochasticTranscription, StochasticTranslation, GeneticExpressionStep/Simulate and related processes; state Poisson/tau-leaping approximations, step/parameter/validity assumptions; test analytically tractable birth-death mean/variance/distributions.
- Acceptance: referenced oracles, justified statistical tolerances, passing tests and gated PR integration.
- Non-goals: label tau-leaping as exact SSA or add advanced tau-leaping.
- Stop: unresolved biological rate/parameter meaning.

### AUTO-SCI-010 (S10) — Growth, division and compartment invariants

- Priority: P1
- Status: blocked-dependency
- Environment: local or github with approved executable CI.
- Dependency: AUTO-SCI-009 done_confirmed.
- Scope: audit CellGrowth, CellDivisionEvent, CellCycleCheckpoint, CompartmentExchange, BioStateProjection and related mechanisms for non-negativity, applicable conservation, partitioning, volume/units, time/order and stochasticity.
- Acceptance: minimal reproducible fixtures, documented educational versus quantitative limits, passing regression and gated PR integration.
- Non-goals: unsupported predictive cell physiology.
- Stop: missing biological invariants or unresolved mass-balance assumptions.

### AUTO-SCI-011 (S11) — WholeCell hybrid integration

- Priority: P1
- Status: blocked-dependency
- Environment: local or github with approved executable CI.
- Dependency: AUTO-SCI-010 done_confirmed.
- Scope: small end-to-end fixtures combining at least two validated mechanisms to detect double time advancement, semantic race/order, state drift, unit inconsistencies and non-reproducibility; revisit HUM-SCI-002 with evidence.
- Acceptance: passing integrated reference fixtures, explicit remaining decisions and gated PR integration.
- Non-goals: hide unresolved time-contract choices.
- Stop: material scientific synchronization decision goes to maintainer.

### AUTO-SCI-012 (S12) — Parameter provenance and interoperability

- Priority: P1
- Status: blocked-dependency
- Environment: local or github with approved executable CI.
- Dependency: AUTO-SCI-011 done_confirmed.
- Scope: audit WholeCellParameterReader, .gen models, annotations and actual SBML bridge subset; record parameter/dataset units, source, transformations, version and diagnostics against silent semantic loss.
- Acceptance: provenance/round-trip or diagnostic fixtures for actual supported subset, passing regression and gated PR integration.
- Non-goals: expand SBML scope without HUM-VC-002.
- Stop: subset extension or material interoperability policy requires maintainer decision.

### AUTO-SCI-013 (S13) — Scientific validation and maturity gate

- Priority: P1
- Status: blocked-dependency
- Environment: local or github with approved executable CI.
- Dependency: AUTO-SCI-012 done_confirmed.
- Scope: final per-subsystem reference/oracle/test matrix including nominal, edge, error, reproducibility, persistence, limitations, scientific claim level and independent software maturity; aggregate regression, appropriate sanitizers and manual impact.
- Acceptance: final-head executed evidence and documentation support every scoped claim; mark each work package done_confirmed only with complete demonstrated acceptance and integrated PR.
- Non-goals: infer predictive validity from build/test green.
- Stop: any unknown/failed mandatory scientific gate remains open, never silently waived.

### AUTO-BACTERIA-001 — Complete the approved Gro/BacteriaColony first-version scope

- Priority: `P1`
- Status: `blocked-review`
- Environment: `local` (Ubuntu/CMake/Ninja/Qt6 toolchain)
- Branch: `WiP20261008/BacteriaColony` (existing authorized work branch)
- Base: feature-branch HEAD `8cadfb651071c17acf7fd0f65fe47685b66e8829`
- Authorization: explicit maintainer mission dated 2026-10-09. This task replaces the pending four-milestone proposal in §22 of `reference/BACTERIA_COLONY_GRO_INTEGRATION_PLAN.md` for this scope only.
- Scope: execute the nine approved cycles in that mission: (1) finite `[0,1]` signal-coefficient validation and phenomenological-operator documentation; (2) continuous coordinates and consistent grid mapping; (3) zero-growth and zero-step invariants; (4) optional persisted automatic division plus Gro division through one routine; (5) simple deterministic positional correction, kinematic movement and kernel-RNG-backed `run`/`tumble`; (6) exactly-once signal/event-step behavior; (7) Qt6 channel selection/heatmap and minimum communication demonstration; (8) supported-model persistence checks and causal investigation of `Smart_BacteriaColony_GRO.gen`; (9) final regressions, GUI build, manual impact and evidence closeout.
- Approved decisions: retain the current dimensionless per-update signal relaxation as phenomenological, constrain finite coefficients to `[0,1]`, defer conservative/physical alternatives, use simple 2D kinematics and optional automatic division disabled by default, and preserve the existing Gro subset, `.gen` format and no-external-physics-engine boundary.
- Non-goals: full original-Gro compatibility; new Gro grammar; external physics dependency; conservative/PDE signal solver; incompatible persistence migration; changes to `WorkInProgress` or stable branches; PR or merge.
- Acceptance: all nine cycles have explicit results; supported bacteria can run the selected Gro subset, grow/divide/die/move with zero invariants and reproducible RNG, use independent validated signals, display runtime state in the existing viewer, and pass supported persistence checks; required local presets/tests/builds are recorded at final HEAD; historical/unrelated failures and limitations are reported separately.
- Stop: escalate only for a material architectural/scientific/security/persistence decision not approved here, incompatible branch work, a regression that cannot be safely fixed within scope, or required validation unavailable. Preserve pre-existing `models/gro_examples/` and `source/tests/unit/generated/` contents and never stage them.
- Review handoff: all nine cycles have local implementation records; final local `tests-unit`, `tests-kernel-unit`, `tests-smoke`, `gui-app`, persistence checks and manual build are recorded in `reference/BACTERIA_COLONY_GRO_INTEGRATION_PLAN.md` §22.7. The main GUI window was visually inspected through XWayland capture, but the two-signal demo view/selector was not interacted with; no hosted CI run is present. Await independent review and do not mark `done_confirmed` until the remaining evidence boundary is accepted.

## 6. Paused technical tasks

These tasks remain paused until the maintainer explicitly activates one. Completion of the documentation migration does not resume them automatically.

### AUTO-APP-001 — Validate standalone HTTP Worker GUI startup

- Priority: `P1`
- Status: `paused`
- Environment: `github`
- Acceptance: preset/build, PID-associated window, bounded liveness, controlled teardown and ordinary CI green.
- Non-goal: no worker security redesign.

### AUTO-APP-002 — Validate standalone main GUI startup

- Priority: `P1`
- Status: `paused`
- Environment: `github`
- Acceptance: `gui-app` preset/build/Xvfb startup evidence before minimal interaction work.

### AUTO-TEST-001 — Remove four historical duplicate Search/Remove test blocks

- Priority: `P2`
- Status: `paused`
- Environment: `local`
- Acceptance: active focused tests remain green, exact inventory updated and ordinary/kernel/smoke paths green.
- Stop: connector-only environments must not replace the large source file blindly.

### AUTO-QT-001 — Remove active Qt5 fallback

- Priority: `P1`
- Status: `paused`
- Environment: `local` preferred
- Acceptance: active references mapped, Qt6 presets/tests green and no GUI redesign.


## 7. Completed technical baseline

Do not reopen without new evidence:

- CI trigger corrections and AI test aggregation;
- Phase 0 kernel/smoke workflow;
- solver contract stabilization;
- active Search/Remove coverage;
- Queue/Station/Delay/Resource lifecycle corrections;
- focused plugin-completion ownership sanitizer;
- optimizer copy/move barrier;
- plugin target/codemodel/link evidence;
- shell, worker, Data Analyser, Optimizer and AI Assistant startup validations;
- per-user runtime launcher/dispatcher (`AUTO-APP-003`, PR #521) and its integration into the Debian package with lifecycle validation (`AUTO-PKG-001`, PR #522).

## 8. Activation and completion rules

A paused task becomes eligible only after the maintainer changes it to `ready` and confirms scope, validation and stop conditions.

A task moves to `done` only after required validation is green, evidence is reviewed, the PR is merged, source-branch deletion is confirmed, issue/status/backlog/changelog are updated and remaining boundaries are explicit.
