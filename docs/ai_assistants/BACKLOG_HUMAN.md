---
document_type: backlog
authority: human-decision-source
owner: project-maintainer
last_updated: 2026-09-18
review_cadence: on-decision-or-status-change
status: active
tracks: 511
---

# GenESyS Human Decision Backlog

## 1. Purpose

This file records decisions that an AI agent must not make by assumption. Each entry identifies the evidence, options, recommendation, and implementation work that becomes eligible after a maintainer decision.

A decision recorded here is not implemented automatically unless a corresponding autonomous task is created or activated in `BACKLOG_AUTONOMOUS.md`.

## 2. Decision status values

- `open` — human decision required;
- `research-needed` — more evidence must be collected before deciding;
- `decision-recorded` — choice made; implementation may still be pending;
- `implemented` — approved choice implemented and validated;
- `deferred` — intentionally postponed;
- `cancelled` — no longer part of the project plan.

## 3. Architectural decisions

### HUM-MODAL-001 — Legacy ModalModel cleanup and compatibility boundary

- Priority: `P1`
- Status: `decision-recorded`
- Decision date: 2026-09-18
- Decision:
  - historical `.gen` model compatibility is **not a requirement** for completing the ModalModel/DefaultNetwork migration and must not constrain the new architecture;
  - current-format save/load symmetry for the supported network-centered implementation remains required;
  - legacy classes such as `ModalModelFSM`, `ModalModelPetriNet` and other superseded modal scaffolding may be removed from the active build after a local dependency audit;
  - temporary quarantine under `source/plugins/components/ModalModel/deprecated/` is preferred when it helps a safe staged migration; permanent deletion is acceptable later when local evidence proves it is safer/clearer;
  - files below `deprecated/` are intended not to compile.
- Confirmed current-code constraint:
  - `source/plugins/components/CMakeLists.txt` currently uses recursive `*.cpp` globbing, so merely moving sources beneath `ModalModel/deprecated/` would still compile them; build exclusion or a safer source-selection adjustment must precede that move.
- Verification pending locally:
  - identify every current code/factory/registration/test/example/GUI reference to the legacy classes;
  - determine which types can be quarantined immediately and which current callers first require migration;
  - validate that active plugin registration and current persistence continue to work after removal from the build.
- Implementation guidance: see [`reference/MODAL_NETWORK_COMPLETION_PLAN.md`](reference/MODAL_NETWORK_COMPLETION_PLAN.md), especially Sections 3.3 and 5B.
- This decision supersedes the former requirement to select legacy `.gen` fixtures or design automatic migration of historical ModalModel files.
- Must not be combined with: broad plugin-architecture redesign, dynamic-plugin migration or unrelated CMake cleanup.

### HUM-MODAL-002 — GUI editor architecture for DataDefinition-based networks

- Priority: `P1`
- Status: `decision-recorded`
- Decision date: 2026-09-18
- Decision:
  - retain the dedicated network/data-definition editor direction in [`reference/MODAL_NETWORK_GUI_ARCHITECTURE.md`](reference/MODAL_NETWORK_GUI_ARCHITECTURE.md);
  - do not overload the process-flow `ModelComponent + Connection` canvas with mathematical/formal network topology;
  - broad GUI implementation is deferred until the backend ModalModel/DefaultNetwork architecture has been locally revalidated and satisfies its completion gate;
  - once backend-ready, the local agent must re-check the current Qt6 GUI before treating the previously proposed G0-G6 phases as executable literally.
- Confirmed evidence:
  - `DefaultNode` is a `ModelDataDefinition` in the network-centered design;
  - graph, EFSM, Markov and CPN topology elements belong to network data definitions;
  - `GraphEdge` and `CPNArc` are not process `Connection` objects;
  - `ModalModelDefault` mirrors/bridges the attached network interface according to the current design.
- Verification pending locally:
  - current GUI synchronization for network ports/bindings;
  - current serializer/property-editor/navigation contracts;
  - whether any GUI work landed after the original 2026-08-31 proposal.
- Decision unlocks after backend completion:
  - GUI creation/editing of network-owned nodes, places, transitions, arcs and graph edges;
  - synchronized `ModalModelDefault` input/output bindings;
  - visual distinction between process topology and mathematical/formal network topology.

### HUM-MODAL-003 — Initial Colored Petri Net supported scope

- Priority: `P1`
- Status: `decision-recorded`
- Decision date: 2026-09-18
- Decision: the initial supported objective is a **pragmatic CPN subset sufficient for GenESyS**, not automatic implementation of every feature of the complete academic CPN formalism.
- Consequences:
  - the local continuation must first establish exactly what the current `ColoredPetriNetNetwork` implements and tests;
  - places, explicit transition nodes, bipartite arcs, marking, symbolic colors/token counts, inscriptions, guards, enabling, atomic firing, reset and persistence should be evaluated against the current GenESyS use cases;
  - typed token payloads, general variable-binding enumeration, general expression-based arc inscriptions, maximal concurrent-step firing and stochastic conflict policies are future extensions unless a concrete approved GenESyS/scientific requirement makes one necessary.
- Verification pending locally: construct a feature/test matrix for the current CPN subset and close only demonstrated gaps.
- Implementation guidance: [`reference/MODAL_NETWORK_COMPLETION_PLAN.md`](reference/MODAL_NETWORK_COMPLETION_PLAN.md), Section 5C4.

### HUM-MODAL-004 — EFSM multiple-enabled-transition policy

- Priority: `P1`
- Status: `decision-recorded`
- Decision date: 2026-09-18
- Decision: `EFSMNetwork` shall have a persisted configurable policy, represented by an enum or equivalent strongly typed configuration, with exactly these semantic choices:
  1. **model error**;
  2. **nondeterministic selection**;
  3. **deterministic by priority**.
- Semantic requirements:
  - model-error mode reports an invalid/ambiguous model when more than one transition is enabled;
  - nondeterministic mode selects among enabled transitions using the reproducible GenESyS RNG infrastructure, never `std::rand()`;
  - deterministic-priority mode uses an explicit documented priority rule and must not depend accidentally on container iteration order.
- Exact C++ enum/type/member names: implementation detail to be selected from the current code style.
- Default policy: **verification/implementation decision still required locally**; it must be chosen explicitly and documented rather than inherited accidentally from current behavior.
- Required local validation: `_check()`/runtime agreement, all three modes, seed reproducibility, priority/tie behavior, reset and persistence round trip.
- Reference direction: Ptolemy II may be used for semantic comparison, without requiring API compatibility.

### HUM-MODAL-005 — Cellular Automata integration into DefaultNetwork

- Priority: `P2`
- Status: `decision-recorded`
- Decision date: 2026-09-18
- Decision:
  - Cellular Automata is intended to migrate into the `DefaultNetwork` architecture;
  - implementation is intentionally deferred until the current ModalModel/DefaultNetwork architecture is fully functional, tested, validated and eligible for `done_confirmed`;
  - the later migration must preserve CA-specific semantics, including regular spatial/lattice structure and update semantics, rather than forcing an artificial generic-graph representation merely for hierarchy uniformity.
- Required later design evidence must cover at least:
  - lattice/mesh representation;
  - neighborhood definition;
  - boundary conditions;
  - synchronous/asynchronous update policy;
  - deterministic/stochastic rules;
  - state ownership/persistence;
  - simulation-time/event interaction.
- Current implementation evidence: `source/plugins/components/ModalModel/CellularAutomata/` and `CellularAutomataComp.*` are present in the current source tree.
- Activation condition: create/activate a separate bounded autonomous task only after the current ModalModel/DefaultNetwork initiative reaches `done_confirmed`.

### HUM-ARCH-001 — Static component target consolidation

- Priority: `P1`
- Status: `open`
- Tracking issue: #492
- Decision required: choose the architecture that replaces the current duplicate `genesys_plugins_components` and `genesys_plugins_components_minimal` source aggregation.
- Confirmed evidence:
  - both targets compile the same 84 component `.cpp` files;
  - GLPK compile behavior differs when GLPK is present;
  - runtime consumers primarily use the target named `minimal`;
  - the continuous-diffusion test reaches both archives;
  - duplicate compilation/storage is active;
  - no active duplicate-symbol linker failure was demonstrated.
- Options:
  - **A — one canonical static component archive**;
  - **B — define a real explicit minimal subset plus full archive**;
  - **C — common object/core composition with explicitly defined facades**;
  - **D — defer until dynamic migration**, not recommended because current ambiguity remains.
- Recommendation: Option A unless a concrete supported lightweight runtime requirement is identified.
- Decision unlocks:
  - a bounded CMake target correction;
  - consistent GLPK configuration;
  - removal/compatibility handling for the duplicate target;
  - present/absent GLPK validation.
- Must not be combined with: dynamic ABI implementation, GUI changes, worker security, or plugin source reorganization by domain.

### HUM-ARCH-002 — `autoloadplugins.txt` deployment and fallback contract

- Priority: `P1`
- Status: `open`
- Tracking issue: #496
- Decision required: define where static/dynamic plugin autoload configuration belongs and how applications search for it.
- Confirmed evidence:
  - shell startup succeeds through static plugin registration;
  - runtime attempts to find `autoloadplugins.txt` beside the executable;
  - the file was absent in the validated build tree;
  - missing-file diagnostics do not currently prevent static fallback.
- Options:
  - copy/install beside executable;
  - application data/config directory;
  - repository/build-tree development fallback;
  - static builds skip file lookup unless explicitly configured;
  - documented ordered multi-location search.
- Recommendation: define an ordered platform-aware configuration search with explicit static fallback and diagnostics, avoiding repository-relative assumptions in installed builds.
- Decision unlocks: focused shell/application deployment tests and packaging integration.

## 4. Security and deployment decisions

### HUM-SEC-001 — Worker bind-address contract

- Priority: `P1`
- Status: `open`
- Tracking issue: #500
- Decision required: choose safe default and configuration semantics for worker listener binding.
- Confirmed evidence:
  - validated worker listens on `0.0.0.0`;
  - the health request was sent through loopback;
  - the approved deployment profile is controlled academic intranet;
  - explicit private-interface selection and deny-by-default behavior are required.
- Options:
  - loopback default with explicit private address for lab deployment;
  - mandatory configured address with startup refusal when absent;
  - enumerate/allowlist private interfaces;
  - retain wildcard only behind an explicit unsafe/development opt-in.
- Recommendation: loopback default plus explicit configured private address; reject wildcard unless explicitly enabled for a controlled environment.
- Decision unlocks: worker CLI/config changes, focused listener tests, packaging/service configuration.

### HUM-SEC-002 — Worker authentication mechanism

- Priority: `P1`
- Status: `open`
- Decision required: select the primary authentication mechanism for the controlled academic intranet profile.
- Principal candidates:
  - mutual TLS for managed laboratory machines;
  - short-lived signed tokens over TLS;
  - institutional identity integration;
  - hybrid mTLS machine identity plus user/job authorization.
- Required inputs:
  - laboratory device management capability;
  - certificate/secret provisioning and rotation process;
  - user versus machine identity requirements;
  - offline/private-network constraints;
  - revocation and audit requirements.
- Recommendation: use mTLS for managed machines when operationally feasible, with short-lived signed authorization tokens for user/job scope where needed.
- Decision unlocks: threat model, credential lifecycle, protected endpoint tests, TLS configuration, audit schema.

### HUM-SEC-003 — AI provider secret storage and invocation policy

- Priority: `P1`
- Status: `research-needed`
- Decision required: approve the supported credential backends and fallback behavior for desktop/server environments.
- Known concern: secret handling must keep values out of argv, logs, repository files, browser-visible configuration, and generated model artifacts.
- Required evidence:
  - current `AISecretStore` behavior on Ubuntu;
  - Secret Service integration availability;
  - headless/server fallback requirements;
  - redaction/error-path tests;
  - packaging dependencies.
- Decision unlocks: secret-store hardening and provider integration tests.

### HUM-SEC-004 — Runtime signing public key provisioning

- Priority: `P1`
- Status: `open`
- Decision required: provision an approved OpenPGP keypair for signing per-user
  GenESyS runtime release bundles, and install the corresponding public key at
  the path the Launcher already expects
  (`/usr/share/genesys/keys/update.gpg`, `GENESYS_UPDATE_KEYRING_PATH`).
- Confirmed evidence:
  - `source/applications/launcher/` already implements `gpgv`-based,
    fail-closed signature verification (`GpgvSignatureVerifier`) and the
    Debian package's `/etc/genesys/update.conf` ships with
    `require_signature=true` and remote updates disabled
    (`enabled=false`, no `manifest_url`);
  - no keypair, private key, or plausible-looking placeholder fingerprint
    exists anywhere in the repository or CI configuration, and none was
    invented while integrating the Launcher into the Debian package
    (PR #522);
  - without a provisioned public key, per-user runtime updates remain
    fail-closed by construction — this is the current, intentional, safe
    state, not a defect.
- Decision required from the maintainer:
  - who holds the private signing key and how it is protected (HSM,
    offline key, CI secret, etc.);
  - the key generation/rotation/revocation process;
  - whether a single key or a key hierarchy (e.g. release + emergency
    revocation) is used;
  - where the corresponding public key is published/pinned for
    installation by the Debian package.
- Decision unlocks: installing the public key file, enabling
  `require_signature=true` against a real signature, and eventually the
  deferred `Runtime Release Publishing` phase (signed
  `genesys-runtime-<version>-ubuntu24.04-x86_64.tar.zst` bundles). This
  decision must not be made by an AI agent, and no CI workflow should
  generate or embed a production private key.

### HUM-DOC-001 — Manual figure automation stack

- Priority: `P2`
- Status: `research-needed`
- Decision required: choose the approved automation path for manual screenshots
  and developer diagrams.
- Candidate areas to evaluate:
  - Qt test automation;
  - Xvfb or a comparable virtual desktop setup;
  - screenshot tooling available on the target platform;
  - TikZ, PlantUML, Graphviz, or Doxygen-based diagram generation;
  - repository-local scripts for capture, normalization, and validation.
- Recommendation: prefer repository-local, reproducible generation first; add
  external tooling only after explicit review and compatibility evidence.
- Decision unlocks: a bounded automation implementation and updated manual
  figure workflow.

## 5. Numerical and statistical decisions

### HUM-SCI-001 — Authoritative numerical/statistical reference package

- Priority: `P0/P1`
- Status: `research-needed`
- Source plan: `genesys_numerical_statistical_references_plan.md`
- Decision/input required from maintainer:
  - bibliography and exact formulations;
  - parameterization conventions;
  - relevant thesis/book/paper sections;
  - legal reference datasets;
  - expected outputs and tolerances;
  - comparator implementations and versions.
- Priority domains:
  - numerical integration/differentiation;
  - probability distributions;
  - confidence intervals and hypothesis tests;
  - chi-square and Kolmogorov–Smirnov diagnostics;
  - distribution fitting;
  - input/output analysis;
  - ODE/PDE/diffusion;
  - DOE/RSM/ANOVA;
  - optimization performance assessment.
- Recommendation: begin with methods that directly affect user-visible p-values, CDFs, fitting, and current solver callers.
- Decision unlocks: reference-backed regression tests and scientific maturity classification.

### HUM-SCI-002 — Modal/hybrid time synchronization contract

- Priority: `P1`
- Status: `research-needed`
- Decision required: define the semantic relationship among discrete-event time, solver internal time, fixed/adaptive steps, event boundaries, and state publication.
- Required evidence:
  - current `ModelSimulation` event flow;
  - continuous and biochemical plugin behavior;
  - representative deterministic models;
  - unit and time-dimension conventions;
  - expected handling of events inside a continuous step.
- Decision unlocks: formal interface contract and regression fixtures.

## 6. Optimization decisions

### HUM-OPT-001 — Initial real Optimizer algorithm and research package

- Priority: `P1`
- Status: `research-needed`
- Source plan: `genesys_multiobjective_optimizer_future_plan.md`
- Decision/input required:
  - professor's thesis citation and relevant chapters;
  - algorithm names/variants;
  - pseudocode or source code;
  - licensing/ownership status;
  - benchmark problems and expected results;
  - decision-variable, objective, constraint, archive, indicator, and stopping semantics.
- Recorded research direction:
  - evolutionary multiobjective optimization;
  - Pareto dominance and external archives;
  - hypervolume and contribution;
  - ETH Zürich/Eckart Zitzler/PISA lineage;
  - SPEA/SPEA2, IBEA, HypE, ZDT/DTLZ and related benchmarks;
  - statistical comparison of stochastic runs.
- Recommendation: implement deterministic contracts and primitives first, then one explicitly selected reference or thesis-derived algorithm.
- Decision unlocks: architecture tasks O0–O5 and eventual Level 3 workflow.

### HUM-OPT-002 — Supported optimizer problem scope

- Priority: `P1`
- Status: `open`
- Decision required: define the first supported problem classes.
- Questions:
  - continuous, integer, categorical, or mixed decisions;
  - deterministic versus stochastic simulation responses;
  - constrained versus unconstrained;
  - number of objectives;
  - model mutation and response extraction contract;
  - replication/noise policy.
- Decision unlocks: stable `OptimizationProblem`, candidate, evaluator, and GUI configuration contracts.

## 7. Whole-cell, biochemical, and AI virtual-cell decisions

### HUM-VC-001 — First bounded AI virtual-cell use case

- Priority: `P1/P2`
- Status: `research-needed`
- Source direction: `genesys_ai_virtual_cell_research_direction.md`
- Decision required:
  - organism, preferably a bounded yeast or microbial system;
  - exact biological question;
  - mechanistic modules included;
  - datasets and licenses;
  - calibration/validation protocol;
  - scientific claim level;
  - external tools and laboratory integration boundary.
- Recommendation: choose a narrow mechanistic perturbation-response or metabolic/regulatory use case with public curated data and explicit invariants before attempting whole-organism breadth.
- Decision unlocks: work packages VC0–VC6 and a concrete benchmark repository structure.

### HUM-VC-002 — SBML supported subset and compatibility target

- Priority: `P1`
- Status: `open`
- Decision required:
  - SBML level/version;
  - supported packages;
  - required constructs and annotations;
  - unsupported-construct behavior;
  - round-trip guarantees;
  - comparator/reference models.
- Decision unlocks: import/export support matrix, diagnostics, round-trip fixtures, and compatibility claims.

## 8. Product and release decisions

### HUM-PROD-001 — Supported feature set for semester stability

- Priority: `P1`, near release window
- Status: `deferred`
- Decision required: define which features are included in the supported set for `20262` and which are experimental, disabled, removed, or deferred.
- Constraint: every supported feature must reach at least Level 3 — Beta.
- Decision unlocks: final maturity matrix, release notes, package scope, and promotion gate.

### HUM-REL-001 — Final `20262` promotion gate and waiver authority

- Priority: `P1`, end of semester
- Status: `deferred`
- Decision required near the promotion window:
  - mandatory build/test/application/model/package/security/documentation checks;
  - P0/P1 definitions;
  - allowed waivers and approving authority;
  - rollback procedure;
  - exact supported platform/package set.
- Recorded timing: promotion only at the end of the second semester of 2026.
- No ordinary PR should attempt this promotion.

## 9. Historical documentation decision

### HUM-DOC-002 — Final deletion approval for `oldies/`

- Priority: `P2`
- Status: `deferred`
- Earliest gate: after 2026-11-01
- Preconditions:
  - every historical file reviewed;
  - useful content consolidated;
  - rejected content explicitly classified;
  - active links removed or updated;
  - preservation tag/branch or Git-history reference prepared;
  - dedicated deletion PR reviewed.
- Decision unlocks: removal of `oldies/` and temporary migration trackers.

## 10. Recording a decision

When the maintainer decides an entry:

1. record the chosen option and date in this file or a dedicated ADR/reference document;
2. update the entry to `decision-recorded`;
3. create or activate a bounded task in `BACKLOG_AUTONOMOUS.md`;
4. identify acceptance criteria and rollback;
5. do not modify source in the same documentation-only decision commit unless explicitly requested;
6. after implementation and validation, mark the decision `implemented` and update `STATUS.md`.