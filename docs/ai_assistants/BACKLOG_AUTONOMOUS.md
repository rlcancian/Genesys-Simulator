---
document_type: backlog
authority: executable-task-source
owner: project-maintainer
last_updated: 2026-11-13
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

### AUTO-PKG-002 — Portable Linux user-space installer

- Priority: `P1`
- Status: `blocked-review`
- Environment: `local`
- Authorization: `HUM-PKG-002`, explicit maintainer instruction dated
  2026-10-15; authorization is limited to this local implementation and
  validation task.
- Branch: `WiP20261015/portable-installer`
- Base: current `WorkInProgress` HEAD `19a0100519c7b7bfad809095da13f85bfc976627`.
- Dependencies: current Launcher/Dispatcher and Debian packaging contracts;
  no unresolved dependency blocks the approved local scope.
- Scope:
  - implement `packaging/installer/` as the reproducible build, staging,
    installation, uninstallation and test interface;
  - build the mandatory Launcher, Dispatcher, main GUI, Shell and Worker,
    plus additional independently implemented Linux GUIs when they compile
    and can be packaged without weakening the mandatory scope;
  - package required resources, Qt platform plugins and redistributable
    libraries after real ELF/dependency inspection;
  - generate versioned provenance metadata and a single offline `.tar.gz` plus
    SHA-256 checksum under the ignored `packaging/installer/dist/` directory;
  - validate user-space installation, relocation, path-with-spaces,
    reinstall, local upgrade/rollback fixtures, execution, uninstall and
    security failure cases in isolated temporary homes;
  - document supported baseline, limitations, MCP status, Worker behavior,
    remote-update policy, dependency inventory and UFSC laboratory procedure.
- Non-goals:
  - no Debian package replacement or modification of its public contract;
  - no AppImage, Flatpak, container or destination-side compilation;
  - no runtime signing key generation, remote update enablement or release
    publication;
  - no dynamic-plugin migration, `autoloadplugins.txt` architecture decision,
    Worker security redesign, MCP server implementation, scientific algorithm
    changes or broad Launcher rewrite;
  - no merge, stable-branch promotion, hosted release or external
    distribution.
- Acceptance criteria:
  - build scripts produce the complete mandatory application set or stop with
    an explicit evidence-backed exclusion and fail the completeness gate;
  - the archive is offline, relocatable, integrity-checkable and contains no
    build-tree absolute paths or secrets;
  - install and uninstall run as a non-root user, are idempotent, preserve
    user models/results, reject unsafe/corrupt/non-owned inputs and retain a
    functional prior installation on failure;
  - installed applications are tested from arbitrary working directories with
    inherited `LD_LIBRARY_PATH`/`QT_PLUGIN_PATH` cleared, and GUI platform
    plugin loading is evidenced rather than inferred from exit status;
  - T01–T16 from the approved mission are individually classified with
    commands, environment, logs and evidence; UNKNOWN is not converted to
    PASS;
  - affected user/developer documentation and governed evidence are
    reconciled; no `done_confirmed` claim is made before review/integration.
- Stop/escalation conditions: missing mandatory application, unresolved ELF
  or Qt redistribution/licensing issue, unsafe installer behavior, required
  security or architecture decision outside `HUM-PKG-002`, inability to
  validate a mandatory gate, or regression outside this bounded scope.
- Local handoff: implementation, package generation and local evidence are
  recorded in `packaging/installer/README.md` and
  `packaging/installer/VALIDATION.md`; review is required before any
  integration or `done_confirmed` state. The physical UFSC laboratory check,
  complete Debian build validation and the pre-existing BacteriaColony test
  failure remain explicitly unresolved.

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

### AUTO-GUIFIX-001 — Safe model save without ModelCheck

- Priority: `P0`
- Status: `done_confirmed`
- Environment: `local`
- Authorization: explicit maintainer determination dated 2026-11-05; execute the five GenESyS GUI/model-editor fixes, including PRs and merge to `WorkInProgress` after required checks.
- Branch/PR: `WiP202611/model-save-preparation` / PR #559; merge `3ae85774949ebc87e521a2409eecf8f086bcc43b`
- Scope: trace and correct save preparation for incomplete models while preserving independent DataDefinitions, `.gen`/`.gui` behavior and atomic file replacement. Never call `_clearOrphanedDataDefinitions()` from save.
- Non-goals: ModelCheck as save precondition, simulation/script execution, persistence format redesign, stable branch promotion.
- Dependencies: none.
- Acceptance: incomplete models save/load without check; valid declared dependencies are materialized idempotently; invalid references diagnose; independent/internal/attached data survive; failed writes preserve prior file.
- Validation: focused persistence regressions, `tests-unit`, `tests-kernel-unit`, `tests-smoke`; affected GUI preset and sanitizer where applicable.
- Stop gates: evidence that safe preparation requires a compatibility or architecture decision; any data loss; required validation red.
- Completion evidence: save materializes existing internal dependencies without ModelCheck or orphan cleanup; persistence regressions and final integration validation passed. See [`history/evidence/2026/11/VALIDATION_LEDGER.md`](history/evidence/2026/11/VALIDATION_LEDGER.md).

### AUTO-GUIFIX-002 — Positional Decide conditions and dynamic outputs

- Priority: `P1`
- Status: `done_confirmed`
- Environment: `local`
- Authorization: explicit maintainer determination dated 2026-11-05.
- Branch/PR: PR #559 plus `WiP202611/decide-condition-undo` / PR #565; final merge `957c66503c8966de95b4d34b047ff03dc256efcb`
- Scope: ordered positional condition editing, N+1 kernel/GUI output ports, preserving unaffected connections and Else through edit, persistence and undo/redo.
- Non-goals: automatic connections, altered simulation routing semantics beyond the approved contract.
- Dependencies: none; implemented in the shared bounded editor-fixes PR.
- Acceptance: add/edit/remove by position, including duplicates, keeps logical/graphical ports, kernel connections, Else and save/load coherent.
- Validation: focused kernel and Qt tests, routing regression, presets required for kernel/GUI changes.
- Stop gates: connection ownership ambiguity or destination corruption; required validation red.
- Completion evidence: positional insert/replace/remove, N+1 outputs, Else, connection preservation and QUndoStack restore/remove behavior passed unit, kernel, smoke, focused GUI and ASan/UBSan tests. See [`history/evidence/2026/11/VALIDATION_LEDGER.md`](history/evidence/2026/11/VALIDATION_LEDGER.md).

### AUTO-GUIFIX-003 — Source-code property editor and persistence

- Priority: `P1`
- Status: `done_confirmed`
- Environment: `local`
- Authorization: explicit maintainer determination dated 2026-11-05.
- Branch/PR: PR #559 and #563; integrated by `957c66503c8966de95b4d34b047ff03dc256efcb`
- Scope: reuse CodeEditor contracts for justified C++, Python, R and Gro properties; fix CppForG field persistence and add round-trip/cancel tests.
- Non-goals: syntax highlighting, LSP, execution/compilation on save, changing structured command-list semantics.
- Dependencies: none; implemented in the shared bounded editor-fixes PR.
- Acceptance: selected code fields retain multiline/UTF-8/escaped content through edit/apply/cancel/save/load; no code executes on save.
- Validation: focused plugin/editor/persistence tests; unit/kernel/GUI presets as affected.
- Stop gates: serializer compatibility decision or script execution needed for textual validation; required validation red.
- Completion evidence: C++, Python, R, Gro and Octave source fields use the existing editor contract and round-trip tests; CppForG persistence is complete; save does not execute source. See the property matrix in [`history/evidence/2026/11/VALIDATION_LEDGER.md`](history/evidence/2026/11/VALIDATION_LEDGER.md).

### AUTO-GUIFIX-004 — Correct Remove and PickUp plugin identity

- Priority: `P1`
- Status: `done_confirmed`
- Environment: `local`
- Authorization: explicit maintainer determination dated 2026-11-05.
- Branch/PR: `WiP202611/model-save-preparation` / PR #559; merge `3ae85774949ebc87e521a2409eecf8f086bcc43b`
- Scope: identify and correct registration metadata/factories causing apparent Remove duplication while retaining model compatibility.
- Non-goals: generic plugin registry redesign or restricting multiple component instances.
- Dependencies: none; implemented in the shared bounded editor-fixes PR.
- Acceptance: one catalog entry per registered typename, correct PickUp identity where supported, idempotent registration/reload, Remove/PickUp create and round-trip.
- Validation: focused plugin/catalog tests and unit/GUI presets as affected.
- Stop gates: current model compatibility cannot be preserved; required validation red.
- Completion evidence: PickUp has distinct metadata and factories; catalog uniqueness, repeat registration/reload and component persistence regressions pass. See [`history/evidence/2026/11/VALIDATION_LEDGER.md`](history/evidence/2026/11/VALIDATION_LEDGER.md).

### AUTO-GUIFIX-005 — ModelComponent-only main plugin palette

- Priority: `P1`
- Status: `done_confirmed`
- Environment: `local`
- Authorization: explicit maintainer determination dated 2026-11-05.
- Branch/PR: `WiP202611/model-save-preparation` / PR #559; merge `3ae85774949ebc87e521a2409eecf8f086bcc43b`
- Scope: filter only main palette presentation to ModelComponents; keep all ModelDataDefinitions registered and usable in specialized contexts.
- Non-goals: registry removal, changes to plugin type metadata, breaking submodel/network editors.
- Dependencies: none; implemented in the shared bounded editor-fixes PR.
- Acceptance: main palette shows components only and refresh is idempotent; data definitions remain constructible, loadable and available to specialized editors.
- Validation: focused Qt tests, `gui-app`, unit/kernel presets as affected.
- Stop gates: specialized editor regression or required validation red.
- Completion evidence: the main palette filters presentation only; ModelDataDefinitions remain registered and available to model and specialized-editor flows. Catalog and GUI regressions pass. See [`history/evidence/2026/11/VALIDATION_LEDGER.md`](history/evidence/2026/11/VALIDATION_LEDGER.md).

### AUTO-TRANSFER-001 — Transfer and LabelDefinition/Label/Goto implementation

- Priority: `P1`
- Status: `done_confirmed`
- Environment: `github` and `local`
- Authorization: direct maintainer instruction (2026-11-05); explicit handoff to validate and integrate into `WorkInProgress` (2026-11-06).
- Branch/PR: `WiP202611/transfer-label-goto` / PR #564; merge commit `c1099e3e7eef781c90cece1110bc7b1b0b82c3fc`.
- Scope: E1–E9 as specified by the maintainer; rename plugin categories and data identity, implement transfer components, static registration, GUI/property/persistence contracts, tests and manuals.
- Non-goals: historic `.gen`/`.gui` type compatibility, dynamic plugin architecture, unrelated modernization, stable-branch promotion.
- Acceptance: category migration preserves existing behavior; LabelDefinition has one owning Label; Goto schedules one entity-preserving transfer; Route/Sequence references remain coherent; GUI metadata/persistence, `.gen`/`.gui` roundtrips, documentation and current-head CI pass.
- Validation: local Ubuntu 24.04 `gui-app`, `tests-unit` (2,004 passed; 4 disabled), `tests-kernel-unit` (2,004 passed; 4 disabled), `tests-smoke` (3/3), focused ASan/UBSan Transfer tests (16/16 with LSan disabled); all PR checks passed on head `020279a55db3f287bb900225d43f15e01add252b`.
- Evidence: [transfer_label_goto_remote_progress.md](history/evidence/2026/11/transfer_label_goto_remote_progress.md), including the sanitizer leak limitation and distinction between automated GUI persistence validation and manual interaction.

### AUTO-PERSIST-001 — Investigate recurrent model-save failures (diagnostic phase)

- Priority: P0.
- Status: running.
- Environment: github (instrumentation, CI evidence) then local (root-cause reproduction).
- Authorization: direct maintainer instruction dated 2026-11-06 for diagnostic implementation; explicit maintainer instruction dated 2026-11-09 authorizes integration into WorkInProgress after required validation.
- Branch/PR: `WiP202611/save-failure-diagnostics`, [PR #568](https://github.com/rlcancian/Genesys-Simulator/pull/568), target WorkInProgress.
- Scope: map kernel/GUI persistence, classify failures of temporary open/write/dump/flush/close/rename, observe plugin/name/registration issues, characterize filesystem/serializer/model/overlay scenarios, preserve previous saves and record evidence.
- Non-goals: persistence redesign, changing ModelCheck, deleting independent data, comprehensive plugin refactoring, or promoting stable branches.
- Acceptance: complete evidence record with baseline SHA and CI provenance; separate logical/I/O/GUI diagnostics; repeatable isolated kernel and GUI regression fixtures; unresolved hypotheses explicitly labelled; current-head focused tests and required checks pass before merge.
- Stop gates: unsafe large-file change, data-loss risk, current-head CI failure that cannot be diagnosed remotely, local-only interaction or filesystem evidence, or any human architectural decision.
- Evidence: `history/evidence/2026/11/model_save_failure_diagnostics.md`.

### AUTO-GUI-002 — Consolidate standalone GUI tools and installed launch paths

- Priority: `P1`
- Status: `done_confirmed`.
- Environment: local + GitHub Actions.
- Authorization: explicit maintainer instructions dated 2026-11-05 and 2026-11-06, including merge authorization after required checks.
- Issue/branch/PR: [#561](https://github.com/rlcancian/Genesys-Simulator/issues/561); `WiP202611/standalone-tools-gui`; [PR #562](https://github.com/rlcancian/Genesys-Simulator/pull/562).
- Base: `WorkInProgress` `82cb4bdb28d698daea14dc01914797cb59f6d1ed`.
- Merge: `46bdedfc94ac9ae6144675fe6b2a64c0c5e98460` (2026-11-07).
- Scope: independent Data Analyser, Optimizer and AI Assistant Qt applications; Tools process launch; installed executable lookup; Debian inclusion; tests and manuals.
- Non-goals: scientific/optimizer/AI algorithm changes, IPC/model transfer, broad source relocation, dynamic-plugin migration, stable-branch promotion. PR #524 remains separate.
- Validation: tests-unit 2,004 passed + 4 disabled; tests-kernel-unit 2,004 passed + 4 disabled; smoke 3/3; six ToolLauncher CTests; real Tools menu launches with PID/window/close/cleanup checks; Debian package and install/reinstall/remove/purge lifecycle; all final PR checks passed.
- Acceptance: three independent targets and processes; no in-process Tools windows; deterministic installed lookup; safe missing/non-executable behavior; real menu launches; package, documentation and final-head CI verified.
- Completion evidence: [`history/evidence/2026/11/2026-11-07_november_consolidation.md`](history/evidence/2026/11/2026-11-07_november_consolidation.md). Post-merge Debian workflow is separately tracked in that evidence; pre-merge lifecycle passed.
- Remaining boundaries: no live optional issue-relay test; no full portable archive lifecycle in this run; optimizer and AI scientific/provider behavior were outside scope.

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
- Acceptance: all approved **backend** in-scope behavior is implemented, compiled, tested, runtime-verified, documented and integrated in `WorkInProgress`; no known backend-gate criterion remains pending. GUI was outside this backend task and blocked global Modal/Network `done_confirmed` at the time; the EFSM GUI phase has since been completed and accepted under `HUM-MODAL-002` (PRs #551/#552).
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

### AUTO-MODAL-003 — Qt6 Network diagram G0: inventory and persistence contract

- Priority: `P1`
- Status: `done_confirmed` — G0.1, G0.1b and G0.2 were accepted and integrated as part of the completed eight-milestone EFSM GUI. The versioned `.gui` codec, compatibility behavior, empty transition-name persistence and identity checks are covered by the final implementation and evidence. The complete GUI delivery and post-merge activation regression are indexed in [`history/evidence/2026/10/2026-10-30_modal_efsm_postmerge_closeout.md`](history/evidence/2026/10/2026-10-30_modal_efsm_postmerge_closeout.md).
- Environment: `github` for inspection/planning; `local` for serializer code, CMake/Ninja, headless Qt6 tests and full validation.
- Branch: `WiP20261029/modalmodel-architecture`, based on `WorkInProgress`.
- Authorization: maintainer's explicit 2026-10-29 approval in `HUM-MODAL-002`; decision `DEC-MODAL-GUI-20261029`.
- Primary plan: [`reference/MODAL_NETWORK_GUI_IMPLEMENTATION_PLAN.md`](reference/MODAL_NETWORK_GUI_IMPLEMENTATION_PLAN.md), G0.1/G0.2; conceptual baseline `reference/MODAL_NETWORK_GUI_ARCHITECTURE.md`.
- Scope: **Alternative A**: reuse the existing submodel `ModelGraphicsScene`/`ModelGraphicsView`, tabs and `level` filter; map scene, builder, editor, serializer, catalog, context menus and ownership. Define `level == 0` as root, else owning editing-context `ModelDataDefinition::getId()` (which includes ModelComponents). Normalize accepted, programmatically inserted Network child data definitions to the Network ID, matching load-time behavior; add focused tests. Then define a versioned GUI-only Network diagram identity/layout record, prove backward-compatible `.gui` serialization and add headless round-trip, malformed, unknown-version, orphan and parallel-edge tests.
- Non-goals: `DefaultNetwork` execution changes, process `Connection` generalization, graph/node conversion to `ModelComponent`, runtime GUI activation, CTMC, Cellular Automata migration, dynamic plugins, broad GUI rewrite, Network creation/interactive topology editing.
- Acceptance: reviewed source-backed inventory; stable identity/duplicate-arc strategy; versioned overlay codec; tests executed on real Qt6 CMake/Ninja toolchain; old `.gui` files load unchanged; malformed/orphan GUI records cannot alter `.gen` semantics; mandatory regression green; manual impact assessed; evidence published.
- Stop gates: unstable relation identity across reload, incompatible GUI file format, unexpected domain ownership, incomplete/failed mandatory tests, or new architectural choices beyond `HUM-MODAL-002`. Return to maintainer before changing kernel identity or data ownership.
- Progress 2026-10-29: maintainer approved **Alternative A** and the owner-ID level contract. Branch code now assigns the Network level on accepted EFSM/Markov/Graph/CPN insertions (four domain tests). **G0.1b code implemented and tested at source SHA `5a37c7ae`:** `GraphicalModelBuilder` discovers direct, registered data definitions at non-root `level` even if there are no process components or attached/internal associations; root visibility remains unchanged. Unanchored new items receive a deterministic fallback placement while existing positions remain untouched. `source/tests/unit/test_gui_submodel_level.cpp` adds six offscreen GUI tests in the existing `genesys_test_gui_gmdd_layout` target. The Developer Manual LaTeX chapter is updated. **CI `40961067698`: 1920 executed/passed (four legacy disabled), all ten new tests passed, GUI GMDD diagnostics green.**
- Progress 2026-10-29 (local agent, same day, final-head `4965eb889`): all three previously remaining milestone-2 gates executed locally — (1) `cmake --preset gui-app` build succeeded and a bounded Xvfb interaction (clean startup/shutdown, no residual processes, conventional `Smart_Process.gen` model loaded and rendered, selection/Property Editor verified for two component types, existing `Open Submodel` context-menu/tab path exercised without regression) was recorded with screenshots; (2) `validate_figure_specs.sh` and `make.sh` both succeeded (`rg` dependency worked around via an already-installed user-local binary — apt install blocked by lack of interactive sudo; see evidence for the exact limitation), regenerating a 388-page `docs/ManualGenESyS.pdf` with zero undefined references in the final `xelatex` pass; section 26.7 was visually inspected and matches current code/tests; (3) local `tests-unit`/`tests-kernel-unit` reproduced the exact CI inventory (1920/1920 passed, 4 disabled) under a `C` locale — a `pt_BR.UTF-8`-only, pre-existing, out-of-scope `PropertyEditorDoubleCommit` locale sensitivity was found and documented, not fixed (unrelated to G0.1b); `tests-smoke` 3/3. Evidence: `history/evidence/2026/10/2026-10-29_modal_g0_1b_local.md`. **Coordinator decision, 2026-10-29:** approved milestone 2 after reviewing source-range commits, final-head CI `40970233832` and `40970233761`, locally regenerated manual PDF, evidence and screenshots. The frozen MVP advances to **2/8 = 25%**. `AUTO-MODAL-003` remains `running`, not `done_confirmed`, because the complete GUI program is not delivered. G0.2 has since been implemented and accepted with an explicitly deferred `.gen` empty-name regression; there is still no EFSM read-only viewer.
- Historical handoff (superseded 2026-10-30): G0.2 had an explicit unnamed-name `.gen` deviation and G1.1 was the sole authorized next milestone. The empty-name persistence correction is published in `5f63fee85`. Earlier checkpoint evidence: [`history/evidence/2026/10/2026-10-30_modal_g1_gui_checkpoint.md`](history/evidence/2026/10/2026-10-30_modal_g1_gui_checkpoint.md) and [`history/evidence/2026/10/2026-10-30_modal_efsm_transition_undo_foundation.md`](history/evidence/2026/10/2026-10-30_modal_efsm_transition_undo_foundation.md). **Latest continuation:** HEAD `cecf2ef1a` adds state/transition editing, bindings, Network selection and move projection refresh; 18 focused tests pass. Full baselines, manuals and lifecycle/E2E gates remain, so accepted progress is still 3/8.

### AUTO-MODAL-004 — Qt6 Network diagram G1: read-only EFSM viewer

- Priority: `P1`
- Status: `done_confirmed` (all eight approved EFSM GUI milestones are implemented and accepted through PR #551, merged at `17338f8e566f87fc3b06eb422a5dd354e6d57b02`. All mandatory local and hosted checks passed. Post-merge PR #552 adds a single regression that executes the GUI-created, saved and reloaded EFSM using the kernel API and reconciles status/evidence records; it does not reopen feature scope).
- Environment: `local` (Qt6 GUI build and interactive verification).
- Branch family: `WiP20261029/modalmodel-architecture`, with bounded commits/PR scopes into `WorkInProgress`.
- Authorization: same maintainer decision `DEC-MODAL-GUI-20261029`; **read-only EFSM first**.
- Scope: a read-only EFSM **projection within the existing level-filtered `ModelGraphicsScene` and submodel tab infrastructure**. Resolve an attached EFSMNetwork from the `ModalModelDefault`, then use `network.getId()` as the level owner. Render states/transitions through domain topology APIs, not process Connections or attached/internal association lines; deterministic initial layout and graceful null/missing Network diagnostics.
- Non-goals: editing/creating Network topology, process `Connection` creation, mathematical state duplication, new simulation activation rules, new formalisms beyond the accepted EFSM first slice.
- Acceptance: build GUI with the supported preset; focused GUI tests and ordinary regression green; verify nested-tab opening, switch/close/reopen, two adapters sharing one Network, no-Network case, root model teardown, and unchanged simulation semantics; record manual UI interaction evidence.
- Latest local validation: commits `5d4ae61eb`, `64644b2c7`, `fc815a597`, `ffafc5ade`, `2bffa8a87`, `72dcbaf51`, and `9590b31ed` cover the MainWindow workflow, self-loop/parallel actions, process-flow preservation, Network creation/association failures, deletion, and mixed history undo/redo. Commit `94724d519` adds semantic labels for unnamed transitions and updates the Developer Manual. The 37 focused EFSM diagram/editor/undo tests pass; targeted ASan/UBSan for deletion and mixed history passes (3/3, then 1/1; leak detection disabled); `gui-app` builds. Unit/kernel 1,963 each (+4 disabled) and smoke 3/3 were validated at production code before the label-only change. `validate_figure_specs.sh`/`make.sh` passed and regenerated the 390-page PDF with page 176 visually inspected. PR #551 final-head hosted CI passed. Evidence: [`history/evidence/2026/10/2026-10-30_modal_efsm_mainwindow_e2e.md`](history/evidence/2026/10/2026-10-30_modal_efsm_mainwindow_e2e.md).
- Original continuation validation (2026-10-30) and PR #551 integration are recorded in [`history/evidence/2026/10/2026-10-30_modal_efsm_g6_final.md`](history/evidence/2026/10/2026-10-30_modal_efsm_g6_final.md). Post-merge regression at the clean `WorkInProgress` merge base validates the complete GUI create/edit/save/reopen path and directly activates the reloaded EFSM through `EFSMNetwork::activate()`: expected output 91 and current state `PersistedDone`. The regression passed in the focused 37-test GUI suite; post-merge full local tests-unit/kernel-unit each passed 1,963 with four disabled, smoke 3/3, and `gui-app` built. Full evidence and any follow-up PR checks are indexed in [`history/evidence/2026/10/2026-10-30_modal_efsm_postmerge_closeout.md`](history/evidence/2026/10/2026-10-30_modal_efsm_postmerge_closeout.md). No separate manual EFSM mouse/keyboard session is claimed.
- Stop gates: dangling GUI-to-Network references, duplicate domain objects, unexpectedly mutated `.gen` state, incorrect model switching, or regressions in existing process/property-editor workflows.


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
- Status: `closed` (maintainer-authorized bounded end of development on 2026-10-13; not `done_confirmed`)
- Environment: `local` (Ubuntu/CMake/Ninja/Qt6 toolchain)
- Branch: `WiP20261009/BacteriaColony-clean-candidate` (review candidate; original branch retained for audit)
- Base: current `origin/WorkInProgress` HEAD `50a340d460876c657d8a3be66554bb29cfe8519a`
- Authorization: explicit maintainer mission dated 2026-10-09. This task replaces the pending four-milestone proposal in §22 of `reference/BACTERIA_COLONY_GRO_INTEGRATION_PLAN.md` for this scope only.
- Scope: execute the nine approved cycles in that mission: (1) finite `[0,1]` signal-coefficient validation and phenomenological-operator documentation; (2) continuous coordinates and consistent grid mapping; (3) zero-growth and zero-step invariants; (4) optional persisted automatic division plus Gro division through one routine; (5) simple deterministic positional correction, kinematic movement and kernel-RNG-backed `run`/`tumble`; (6) exactly-once signal/event-step behavior; (7) Qt6 channel selection/heatmap and minimum communication demonstration; (8) supported-model persistence checks and causal investigation of `Smart_BacteriaColony_GRO.gen`; (9) final regressions, GUI build, manual impact and evidence closeout.
- Approved decisions: retain the current dimensionless per-update signal relaxation as phenomenological, constrain finite coefficients to `[0,1]`, defer conservative/physical alternatives, use simple 2D kinematics and optional automatic division disabled by default, and preserve the existing Gro subset, `.gen` format and no-external-physics-engine boundary.
- Non-goals: full original-Gro compatibility; new Gro grammar; external physics dependency; conservative/PDE signal solver; incompatible persistence migration; changes to `WorkInProgress` or stable branches; PR or merge.
- Acceptance: all nine cycles have explicit results; supported bacteria can run the selected Gro subset, grow/divide/die/move with zero invariants and reproducible RNG, use independent validated signals, display runtime state in the existing viewer, and pass supported persistence checks; required local presets/tests/builds are recorded at final HEAD; historical/unrelated failures and limitations are reported separately.
- Stop: escalate only for a material architectural/scientific/security/persistence decision not approved here, incompatible branch work, a regression that cannot be safely fixed within scope, or required validation unavailable. Preserve pre-existing `models/gro_examples/` and `source/tests/unit/generated/` contents and never stage them.
- Review handoff: original feature branch is preserved at `dcc8bb89910e3b0f8d8c15f29f89dcb36846f230`. The clean candidate is based on `origin/WorkInProgress` `50a340d460876c657d8a3be66554bb29cfe8519a` and deliberately excludes commit `0f64fa8e07`; only the legitimate coordinate tests from that commit were reapplied. Handoff `BC-HANDOFF-001` execution, current Qt interaction limitation, coordinate correction, and final local preset results are recorded in `reference/BACTERIA_COLONY_AGENT_REPORT.md` and plan §22.9. No hosted CI run is claimed. Maintainer instructed integration into `WorkInProgress` and closure of development for now on 2026-10-13. The scope is frozen as a demonstrative first version, with known GUI interaction gaps (`GUI_RESET` not executed, running/paused manual-step interaction partial, viewer lifecycle partial) and hosted CI not yet established at the time of this note. These gaps remain recorded as limitations; task `closed` means no active development, **not** that every criterion was validated (`done_confirmed`). Integration is through a guarded PR with mandatory CI checks; no additional autonomous cycles are authorized.

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


## 9. Active bounded GUI work authorized by maintainer

### AUTO-GUI-001 — Simplify and correct GUI Report Issue

- Priority: P1
- Status: done_confirmed
- Environment: local + github
- Authorization: direct maintainer instruction dated 2026-11-06 authorizes local audit, required validation, PR completion, and merge into WorkInProgress after all gates pass; this supersedes the 2026-11-05 no-merge limit.
- Branch/PR: `WiP202611/report-issue-gui` / PR #560; merge commit `82cb4bdb28d698daea14dc01914797cb59f6d1ed`.
- Scope: three-field issue report dialog, browser URL prefill without relay, optional secure asynchronous relay, strict issue confirmation, privacy, focused QtTest and CI, user/developer documentation, regenerated canonical manual PDF, and local/hosted validation.
- Non-goals: no production relay deployment, GitHub secret/token provisioning, production test issue, new backend service, broad controller refactor, or promotion to stable branches.
- Acceptance: title/description validation; bug as default; no automatic sensitive diagnostics; encoded and bounded GitHub URL; clipboard path for oversize content; explicit browser handoff; error preservation; verified HTTPS issue identity for relay; bounded asynchronous response handling/cancellation; mock HTTP status/timeout/security tests; GUI build and QtTest evidence; successful manual PDF regeneration and review; final-head required checks green.
- Validation: 21 focused QtTests; local GUI build; unit/kernel (1,998 passed, 4 disabled each), smoke (3/3), canonical PDF build and page inspection; final PR-head required checks passed.
- Evidence: `history/evidence/2026/11/2026-11-06_report_issue_pr560_validation.md`. A live relay-to-GitHub transaction remains unverified because no operational relay was available.

### AUTO-SIGNAL-001 — Expose shared SignalData on Wait and Signal

- Priority: `P1`
- Status: `running`
- Environment: `local` (C++23, CMake/Ninja, Qt6)
- Authorization: explicit maintainer task dated 2026-11-08; maintainer explicitly authorized integration on 2026-11-09, subject to required checks.
- Branch/PR: `WiP202611/wait-signal-data-property`, [PR #570](https://github.com/rlcancian/Genesys-Simulator/pull/570) (Draft; reconciling latest WorkInProgress and rerunning required validation).
- Base: current `origin/WorkInProgress` at `82acab46238b1fea689202b24185bf2e2e8b3c00`.
- Scope: expose the existing `SignalData` reference as an editable property on only `Wait` and `Signal`, following current `SimulationControlGenericClass` patterns; add getters if required; verify shared-instance selection, `.gen`/`.gui` persistence, invalid/null reference behavior and signal/wait regressions.
- Non-goals: changes to other ModelComponents, SignalData ownership semantics, property-editor architecture, plugin registration, or persistence format.
- Acceptance: both controls appear in the Property Editor and can reference the same existing SignalData without duplicate definitions; references round-trip through model and graphical save/load; invalid/null handling matches existing contracts; focused signal behavior tests and affected unit/kernel/GUI builds pass.
- Validation: historical branch evidence: GCC 13.3/C++23; 16 signal runtime tests and 28 focused tests passed, including Property Editor selection and `.gen`/`.gui` shared-reference round trips. Full `gui-app` build was interrupted and is not counted as PASS. Current merged-base validation is pending.
- Stop gates: a required ownership/persistence contract change or invalid-reference policy decision beyond the current architecture; preserve the branch and report evidence if encountered.

### AUTO-PROPERTY-001 — ModelComponent DataDefinition property contracts

- Priority: `P1`
- Status: `running`
- Environment: `local` (C++23, CMake/Ninja, Qt6, Google Test)
- Authorization: explicit maintainer request dated 2026-11-08; on 2026-11-09 the maintainer explicitly directed merge of the validated slice after required checks passed. The overall audit remains open.
- Branch/PR: `WiP202611/datadef-component-properties`, [PR #571](https://github.com/rlcancian/Genesys-Simulator/pull/571), merged as `3eafcc14544c4b91c6b17d35b3827581d9dbcb76`.
- Base/integration: started at `origin/WorkInProgress` `82acab46238b1fea689202b24185bf2e2e8b3c00`; current integrated HEAD `3eafcc14544c4b91c6b17d35b3827581d9dbcb76`.
- Scope: inventory ModelComponent-to-ModelDataDefinition associations and correct confirmed Property Editor gaps across AI, continuous, modal, template, external integration, polymorphic Search/Remove, discrete/transfer and scientific components; implement Match queue configuration in a separate bounded commit after auditing routing and persistence contracts; add focused GUI/kernel/persistence/lifetime regressions and manual-impact documentation.
- Non-goals: making `Counter` or `StatisticsCollector` editable; broad property framework/plugin architecture refactoring; changing simulation algorithms except where a confirmed property contract requires it; introducing unsafe sharing/ownership changes; inventing migration behavior for legacy Match output connections; reimplementing Wait/Signal changes identified by PR #570 commits `b33c9c634` and `b10adbf59`.
- Dependencies: reconcile Wait/Signal from PR #570 only after its commits are integrated into the working base or explicitly incorporated by an auditable dependency; current implementation is not part of `origin/WorkInProgress` at task start.
- Acceptance: each configurable association is represented by a compatible component-owned SimulationControl; Property Editor selection/editing demonstrably changes the component state; ownership, internal/external and shared-reference semantics remain safe; supported `.gen`/`.gui` references round-trip without duplicate definitions; `Counter`/`StatisticsCollector` remain excluded; Match queue count/rules/attributes stay coherent and persist; no unresolved in-scope component association is omitted from the final traceability matrix; required focused, ordinary, kernel, smoke and GUI validation passes, or explicit blockers remain documented.
- Validation: red/green focused tests by vertical slice; current CMake/Ninja presets and CTest; GUI interaction/Qt tests for editor behavior; sanitizer diagnostics for ownership changes; manual-impact review and PDF regeneration if LaTeX changes.
- Stop gates: stop any ownership design requiring a materially different public/API contract; stop Match multi-output migration if legacy connection routing is ambiguous (contain it and record exact blocker); do not add controls for Counter/StatisticsCollector; preserve the Wait/Signal feature branch and avoid duplicate changes.
- Audit/integration update (2026-11-09): the inventory and local validation are recorded in [`history/evidence/2026/11/2026-11-08_datadef_component_property_audit.md`](history/evidence/2026/11/2026-11-08_datadef_component_property_audit.md); PR #571 at head `44e36ee5a6117101b4bf96f429da015349aae197` passed all PR checks and was merged as `3eafcc14544c4b91c6b17d35b3827581d9dbcb76`. The local `WorkInProgress` checkout was fast-forwarded to that SHA and was clean/synchronized. The post-merge Debian package workflow was still running when recorded. The item remains `running`: Wait/Signal is still owned by open Draft PR #570, and Match output-port count/routing migration remains intentionally contained because legacy single-output connections have no specified mapping. `Match::NumberOfQueues` remains editable during model editing without a runtime guard. No LaTeX sources changed; PDF regeneration is not applicable.

## 10. Planned graphical extensions (not executable until activated)

### AUTO-GUI-003 — Implement static-first main GUI graphical extensions

- Priority: `P1`
- Status: `blocked-dependency`
- Environment: `local` for C++23/Qt6 GUI tests + `github` for PR/CI integration.
- Authorization: maintainer approved **architecture and documentation only** on 2026-11-11 (`HUM-ARCH-004`). The current request explicitly authorizes the documentation PR/merge, **not implementation**. Separate explicit activation required before starting G0/G1 or any source changes.
- Decision and plan: [GUI extension architecture](reference/GUI_EXTENSIONS_ARCHITECTURE.md), [G0–G7 implementation plan](reference/GUI_EXTENSIONS_IMPLEMENTATION_PLAN.md).
- Scope after activation: audit current coupling, extend existing registry/dependency contracts, add registered scene animation/factory and GUI event/persistence bridge, migrate domain animations incrementally, integrate BacteriaColony/CellularAutomata scene visualizations and alternative renderer/service contributions.
- Non-goals: compile/load/unload dynamic packages; C ABI/Qt adapter implementation; hot-reload guards; kernel algorithm/model semantics changes; full GUI rewrite; broad static plugin target consolidation (#492); indiscriminate application of student PR #436.
- Dependencies/blockers: maintainer must activate the first bounded slice; G0 current-head audit and accepted baseline precede G1/G2; per-slice API/lifecycle/persistence decisions must be evidenced before later migrations. Future dynamic work needs a separate human decision and follows the established C ABI/target gates.
- First ready candidate **after authorization**: bounded G0 evidence-backed coupling, ownership and persistence inventory, then a minimal G1/G2 registered-animation vertical slice (separate PRs as appropriate). Do not begin G4/G5 before factory + event/persistence acceptance.
- Acceptance: a new optional GUI extension adds its animation/tool without modifying generic GUI, kernel or parser; dependency absence preserves simulation; event delivery is consistent; graphical state survives save/reload and missing-extension round-trip; alternate providers are deterministic; targeted GUI tests, unit/kernel/smoke as affected, and safety diagnostics on ownership paths pass.
- Stop gates: uncertain event semantics, model binding, unknown-extension data retention, ownership, concurrent GUI modifications, unstable C++/Qt ABI or any request to implement dynamic loading without explicit scope approval.
- Rollback: per-phase reviewable merge commits and current-format compatibility fixtures. No release-branch promotion.

## 11. Authorized full ModalNetwork GUI implementation

### AUTO-MODAL-005 — Execute complete ModalModel/DefaultNetwork GUI evolution

- Priority: P1.
- Status: **done_confirmed** (integrated by PR #577 on 2026-11-13).
- Environment: local Ubuntu plus GitHub Actions; merge commit `1811a7ae6834af5d26f4c9c4de0cec8252bd88eb` on `WorkInProgress`.
- Source and full acceptance: [MODAL_NETWORK_GUI_EVOLUTION_PLAN.md](reference/MODAL_NETWORK_GUI_EVOLUTION_PLAN.md), decision HUM-MODAL-003.
- Integration base: synchronize the latest origin/WorkInProgress; branch example WiP20261112/modal_model_gui. Never develop directly on WorkInProgress.
- Authorization: execute ALL milestones G0, A1–A5, B1–B5, C1–C6 and D1–D5 sequentially and autonomously, including fully functional contextual Graph/Markov/CPN GUI editors and faithful graphical animation of actual supported Network behavior. No intermediate approval for plan-covered decisions.
- Scope/acceptance: all gates and end-to-end scenarios in the plan, small traceable commits, short persistent progress log, essential targeted validation throughout, final appropriate CMake/CTest/Qt verification. Focus on complete functional delivery, avoid excessive or repetitive audits/tests.
- Non-goals: new mathematical semantics, broad plugin-system rewrite, dynamic GUI packaging/unload, and unrelated AUTO-GUI-003 migrations.
- Blockers: only genuine unforeseen semantic, irreversible persistence, ownership, ABI or security decision, or indispensable failing validation. Report precise blocker; do not mark partial implementation complete.
- Completion: all milestones and acceptance evidence complete; merge into WorkInProgress is explicitly authorized by the maintainer on 2026-11-13, subject to the required PR review and validation gates.
- Completion checkpoint: all milestones and acceptance evidence are complete; PR #577 merged at `1811a7ae6834af5d26f4c9c4de0cec8252bd88eb`. All 19 final-head hosted checks passed, including unit (2,109 executed; four disabled), kernel-unit (2,111 executed; four disabled), smoke (3/3 runnable), GUI builds, documentation, Debian packaging/Lintian and package lifecycle. The branch `WiP20261112/modal_model_gui` remains available for continued work. See `history/evidence/2026/11/2026-11-12_modal_network_gui_implementation_progress.md`.

### AUTO-MODAL-006 — Universal Network node/relation editing

- Priority: P1.
- Status: **blocked-review** (implementation and local validation complete; awaiting maintainer review).
- Environment: local Ubuntu; branch `WiP20261112/modal_model_gui`, based on the current `origin/WorkInProgress` integration line.
- Authorization: HUM-MODAL-004 and the maintainer's explicit implementation mandate dated 2026-11-13.
- Scope: extend the shared extension and Property Editor contracts to mathematical nodes and relations; complete EFSM (state/transition editing, mouse relation creation, undo/redo, `.gen`/`.gui` persistence, removal of EFSM edit dialogs) as the first testable checkpoint; then apply the same mechanisms to Graph, DirectedGraph, DAG, Markov and CPN while preserving each domain's topology and ownership rules.
- Acceptance: selected elements expose their actual domain properties in the existing Property Editor; relation creation requires no dialog and validates before mutation; invalid graphs remain unchanged; operations update the projection and undo/redo; persisted models reopen with topology and properties intact. Focused tests and affected CMake targets pass at each checkpoint.
- Non-goals: kernel Qt dependencies, proxy ModelDataDefinitions, invented formalism semantics, and automatic merge to WorkInProgress.
- Progress evidence: `history/evidence/2026/11/2026-11-12_modal_network_gui_implementation_progress.md`.
- Completion evidence (2026-11-13): EFSM, Graph/DirectedGraph/DAG, Markov and CPN implementations are committed on the development branch; GUI test target passed 155/155 on rerun, formalism kernel/model-file tests passed 45/45, GUI app build and documentation governance passed. Branch HEAD `fe73fb845`; no merge to WorkInProgress. Interactive desktop smoke remains unperformed.
