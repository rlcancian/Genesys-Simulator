---
document_type: backlog
authority: human-decision-source
owner: project-maintainer
last_updated: 2026-08-31
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

### HUM-MODAL-001 — Legacy ModalModel wrapper and persistence compatibility

- Priority: `P1`
- Status: `open`
- Decision required: define how existing persisted models and public plugin type names that use `ModalModelFSM` and `ModalModelPetriNet` are migrated to the network-centric architecture.
- Confirmed evidence:
  - `ModalModelDefault` now supports a `DefaultNetwork` bridge with explicit input/output bindings;
  - `EFSMNetwork`, `MarkovChainNetwork`, `GraphNetwork` and `ColoredPetriNetNetwork` are registered network data definitions;
  - `ModalModelFSM` and `ModalModelPetriNet` remain thin subclasses of `ModalModelDefault`;
  - the legacy `ModalModelDefault` node-list execution path still exists for compatibility when no `DefaultNetwork` is attached;
  - the legacy probabilistic path now uses a resettable GenESyS kernel sampler instead of `std::rand()`.
- Options:
  - keep wrappers indefinitely as compatibility aliases over `ModalModelDefault`;
  - convert wrappers into explicit shims that auto-create or bind `EFSMNetwork` / `ColoredPetriNetNetwork` for new instances while still loading old serialized fields;
  - migrate saved legacy node/transition fields into standalone network data definitions during load;
  - deprecate wrappers and require explicit model-file migration tooling before removal.
- Recommendation: keep wrappers as compatibility shims in this PR; select representative legacy `.gen` fixtures before implementing automatic persistence migration.
- Decision unlocks:
  - bounded wrapper rewrite or deprecation path;
  - persisted-model migration tests;
  - documentation of supported legacy model-file compatibility.
- Must not be combined with: full CPN variable binding, GUI network editor implementation, broad ModelDataManager ownership redesign or dynamic-plugin migration.

### HUM-MODAL-002 — GUI editor architecture for DataDefinition-based networks

- Priority: `P1`
- Status: `open`
- Decision required: choose how the GUI creates, edits and visualizes `DefaultNetwork` data definitions and their formalism-owned elements without treating them as process-flow `ModelComponent` + `Connection` topologies.
- Confirmed evidence:
  - `DefaultNode` is now a `ModelDataDefinition`;
  - graph, EFSM, Markov and CPN topology elements belong to network data definitions;
  - `GraphEdge` and `CPNArc` are not process `Connection` objects;
  - `ModalModelDefault` mirrors the attached network's logical port schema, but the current GUI synchronization for network ports and bindings remains unfinished.
- Options:
  - add a dedicated network/data-definition editor independent from the process-flow canvas;
  - add modal-network editing panels inside the existing component property editor;
  - provide model-file/API construction first and defer GUI editing;
  - temporarily expose only read-only network visualization until editing contracts are stable.
- Recommendation: implement a dedicated network/data-definition editor after the compatibility and persistence strategy is settled; do not overload the existing process `Connection` canvas with graph/CPN arcs.
- Decision unlocks:
  - GUI creation/editing of network-owned nodes, places, transitions, arcs and graph edges;
  - synchronized `ModalModelDefault` input/output bindings;
  - visual distinction between process topology and mathematical/formal network topology.
- Must not be combined with: process `Connection` payload generalization or release/promotion gates.

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
