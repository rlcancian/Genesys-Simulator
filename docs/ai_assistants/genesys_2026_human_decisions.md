# GenESyS 2026 Human Decisions and Clarifications

## 1. Purpose and scope

This document records human decisions, clarified terminology, available architectural options, and decisions that remain intentionally deferred in the GenESyS 2026 consolidation plan.

- Repository: `rlcancian/Genesys-Simulator`.
- Consolidation branch: `audit/genesys-2026-consolidation-plan-20260720`.
- Consolidation PR: #469.
- Decision date: 2026-07-20.
- Decision authority: Professor Rafael Luiz Cancian.
- Implementation status: documentation and policy only; no functional source changes are authorized by this document alone.

Status terms used here:

- `decided`: the architectural or product policy is approved;
- `deferred`: no implementation should begin until the missing decision inputs exist;
- `proposed`: a technically recommended default is documented but still needs explicit human approval;
- `needs-human-decision`: alternatives are understood, but one must still be selected;
- `needs-implementation`: policy is decided but code/build/documentation still contain legacy compatibility or behavior;
- `needs-validation`: the implementation or scientific claim still needs executable evidence.

## 2. Dynamic plugins: current decision and future ABI/toolchain options

### 2.1 Current decision

Status: `decided`.

For the current consolidation phase, GenESyS plugins will continue to be compiled together with the simulator through the existing static build graph. The consolidation plan must not start a broad dynamic-plugin migration now.

The immediate technical work is limited to:

- making the current static plugin target graph non-overlapping;
- identifying source files compiled into more than one target;
- documenting plugin registration, metadata, factories, persistence, dependencies, and lifecycle;
- preserving current behavior while tests and CI are stabilized.

### 2.2 Why an ABI/toolchain decision is required later

An in-process C++ plugin crosses binary boundaries. Compatibility can be affected by:

- compiler family and major version;
- C++ standard library and its ABI mode;
- build flags, exception model, RTTI, visibility, sanitizers, and debug/release mode;
- allocation and deallocation across module boundaries;
- STL, Qt, or other C++ types exposed in public plugin interfaces;
- Qt major/minor version when Qt types cross the boundary;
- operating system, architecture, glibc, and packaging baseline;
- plugin unload semantics, threads, callbacks, and object lifetime.

A source-compatible C++ interface is not automatically a stable binary interface.

### 2.3 Future architectural options

#### Option A — same-toolchain C++ ABI

Plugins expose C++ interfaces directly and are built with the same supported compiler, standard library, build mode, C++ standard, Qt version, and packaging baseline as the simulator.

Advantages:

- smallest migration from the current design;
- direct use of existing C++ classes;
- low call overhead;
- simpler initial implementation.

Costs and risks:

- plugins may need rebuilding for each GenESyS/toolchain release;
- STL and Qt types make ABI compatibility more fragile;
- allocator, exception, RTTI, and ownership mismatches can cause undefined behavior;
- third-party binary distribution is tightly coupled to the official build environment.

This is suitable when plugins are developed and built together with GenESyS, even if packaged as separate shared libraries.

#### Option B — stable C ABI with opaque handles and versioned function tables

The dynamic boundary exposes `extern "C"` entry points, fixed-width scalar types, opaque handles, explicit ownership functions, and versioned capability/function tables. C++ implementation details remain behind the boundary.

Advantages:

- substantially better long-term ABI control;
- less dependence on compiler-specific C++ ABI details;
- explicit ownership, error reporting, version negotiation, and lifecycle;
- suitable for independently distributed plugins.

Costs and risks:

- requires adapter layers around existing C++ plugin classes;
- API design must be deliberate and versioned;
- callbacks, strings, arrays, errors, and object identity need explicit conventions;
- more initial architecture and test work.

This is the preferred long-term option if GenESyS intends to support independently built third-party in-process plugins.

#### Option C — out-of-process plugin/service protocol

A plugin runs in a separate process and communicates with GenESyS through an IPC or network protocol.

Advantages:

- strongest crash and memory isolation;
- language-independent implementations;
- no shared C++ ABI;
- easier resource limits and sandboxing.

Costs and risks:

- serialization and IPC overhead;
- more complex deployment, process supervision, versioning, and diagnostics;
- inappropriate for very fine-grained, high-frequency component calls unless carefully designed.

This is suitable for coarse-grained services, external solvers, AI services, distributed workers, or untrusted extensions. It is not automatically suitable for every simulation component.

### 2.4 Information required before choosing

The future decision should answer:

1. Must third parties distribute binary-only plugins?
2. Must a plugin remain binary-compatible across multiple GenESyS releases?
3. Will plugins be built only for Ubuntu 24.04, or for multiple Linux distributions and Windows?
4. May STL or Qt types cross the plugin boundary?
5. Is plugin unload/hot reload required?
6. Can a plugin allocate an object that another module destroys?
7. Is process isolation more important than call latency?
8. Are non-C++ plugin implementations expected?
9. What version-negotiation and deprecation policy is acceptable?

### 2.5 Recorded status

- Current static aggregation: `decided`.
- Dynamic migration: `deferred`.
- Preferred long-term candidate for independent third-party plugins: stable C ABI with opaque handles and versioned function tables.
- Final ABI/toolchain policy: `needs-human-decision` after the requirements above are answered.

## 3. Qt compatibility policy

### 3.1 Decision

Status: `decided`, `needs-implementation`.

GenESyS will support Qt6 only. Qt5 compatibility is to be removed.

### 3.2 Consequences

Future changes should:

- use `find_package(Qt6 REQUIRED ...)` rather than falling back to Qt5;
- remove `GENESYS_QT_PACKAGE` branches that select Qt5;
- remove Qt5-only compatibility code and version-conditionals after confirming they are not required by Qt6;
- update CI, packaging, README, developer documentation, and installation instructions to require Qt6;
- keep Qt6 ownership and signal/slot semantics explicit;
- validate every GUI target and GUI unit test after the removal.

Confirmed current legacy references include GUI CMake fallback logic, unit-test Qt package selection, the README, and additional source/build-script references. Their removal must be a bounded implementation task with CMake/Ninja/CTest validation.

### 3.3 Acceptance criteria for the removal task

- repository search finds no active Qt5 build fallback outside retained historical documents;
- all GUI CMake targets require Qt6;
- `tests-unit` configures and runs with Qt6;
- `gui-app`, `gui-httpworker`, `gui-dataanalyser`, `gui-optimizer`, and `gui-ai-assistant` build with Qt6;
- Debian/other packaging declares Qt6 dependencies only;
- current README and developer documentation contain no Qt5 instructions.

## 4. Authoritative numerical and statistical references

### 4.1 Clarification

An authoritative reference is the declared oracle used to determine whether a numerical or statistical implementation is correct within a stated domain and tolerance. It is not merely another implementation that happens to produce a similar result.

The professor may and should provide specific bibliography when GenESyS behavior is intended to follow particular formulations, conventions, algorithms, approximations, or decision rules.

Useful reference material includes:

- bibliographic citation, edition, chapter, section, theorem, equation, or algorithm number;
- original or peer-reviewed paper defining the method;
- accepted standard or technical specification;
- authoritative reference dataset and expected outputs;
- thesis chapter, pseudocode, source code, tables, or benchmark results when the intended implementation comes from the professor's research;
- explicit convention choices where multiple valid formulations exist.

### 4.2 Validation hierarchy

For each numerical/statistical feature, prefer the following evidence hierarchy:

1. **Analytical invariants or closed-form solutions**, when available.
2. **Declared primary bibliographic or standards reference** for the exact formulation.
3. **Independent high-quality implementation** used as a cross-check, not as the sole specification unless explicitly adopted.
4. **Published or curated reference datasets/benchmark problems**.
5. **Property, convergence, conservation, metamorphic, and randomized tests**.
6. **Regression fixtures** preserving previously validated behavior.

R, SciPy, Boost.Math, GNU Scientific Library, or similar packages can be valuable independent comparators, but the project should document which package/version/function/convention was used and whether it is normative or only corroborating evidence.

### 4.3 Required validation record per algorithm

Each validated implementation should document:

- mathematical definition and parameterization;
- valid input domain and preconditions;
- units and scaling;
- small-sample and degenerate-case policy;
- numerical method and stopping criteria;
- absolute/relative tolerance and why it is appropriate;
- overflow, underflow, non-finite, and error behavior;
- reference citation and expected values;
- independent cross-check implementation, when used;
- deterministic seeds and reproducibility conditions for stochastic methods.

### 4.4 Recorded status

- Validation framework: `decided`.
- Specific authoritative bibliography for each method: `needs-human-decision`.
- Professor-provided references are explicitly supported and preferred when they define the intended GenESyS semantics.

## 5. Optimizer algorithms and user-visible maturity

### 5.1 Meaning of maturity

User-visible maturity describes what GenESyS claims the Optimizer can reliably do, how prominently it is exposed, and what guarantees accompany it. It is separate from whether a class compiles or a GUI opens.

Recommended maturity levels:

1. **Internal scaffold**
   - interfaces and UI may exist;
   - no claim that optimization is performed correctly;
   - not presented as a finished user capability.

2. **Research prototype / experimental**
   - one or more real algorithms execute end-to-end;
   - assumptions and limitations are explicit;
   - validated on selected benchmark problems;
   - API, results, and performance may still change.

3. **Beta**
   - intended workflows are complete;
   - error handling, reproducibility, cancellation, persistence, and reporting are tested;
   - broader benchmarks and realistic GenESyS models are covered;
   - still not guaranteed stable for all supported problem classes.

4. **Stable user feature**
   - supported problem classes and algorithms are documented;
   - reference-backed correctness and regression tests exist;
   - deterministic/reproducible behavior is controlled where applicable;
   - performance, failure behavior, versioning, and result semantics are documented;
   - GUI and backend behavior are consistent.

The current `OptimizerDefaultImpl1` is an internal scaffold. It must not be described as a functioning or stable optimizer.

### 5.2 Algorithm decision

Status: `needs-human-decision`.

Yes: implementing a real Optimizer requires selecting the algorithms or algorithm family that define the initial supported capability. The professor does not need to provide a generic algorithm chosen by the assistant. Techniques developed in the professor's doctoral research are appropriate and potentially preferable, especially for multiobjective optimization.

Useful inputs from the professor include:

- thesis citation and relevant chapters;
- algorithm names and intended variants;
- pseudocode or existing source code;
- ownership/licensing status of that source;
- supported decision-variable types;
- single-objective and/or multiobjective scope;
- constraint-handling method;
- stochastic simulation and replication policy;
- noise handling and statistical comparison of candidate solutions;
- archive, dominance, diversity, and tie-breaking rules;
- stopping criteria;
- deterministic seed/reproducibility policy;
- benchmark problems and published expected results.

### 5.3 Recommended implementation sequence

1. Define a backend-neutral optimization contract separating:
   - decision-variable encoding;
   - model mutation/application;
   - simulation evaluation;
   - objective/constraint evaluation;
   - algorithm state;
   - solution archive;
   - stopping/cancellation;
   - reporting and persistence.
2. Select one algorithm from the professor's research as the first reference implementation.
3. Implement deterministic unit tests for dominance, constraints, archives, variation/search operators, and stopping logic.
4. Validate against benchmark problems used in the thesis or other declared references.
5. Add simulation-noise and replication handling.
6. Only then expose the capability as a research prototype in the GUI.
7. Add further algorithms behind the same interface without coupling them to GUI classes.

### 5.4 Recorded status

- Current Optimizer maturity: `internal scaffold` / `partially-implemented`.
- Public stable claim: not authorized.
- Initial algorithm family: `needs-human-decision`.
- Professor's multiobjective research techniques: preferred candidate source, pending artifacts and exact scope.

## 6. Worker exposure and security profiles

### 6.1 Deployment profiles

#### Profile A — local-only

- bind only to loopback;
- used by the same machine or through an authenticated tunnel;
- smallest network attack surface;
- appropriate for development and single-computer execution.

#### Profile B — controlled academic intranet

- worker accepts requests from authorized computers in a laboratory/private network;
- no direct public-Internet exposure;
- network and client identities are controlled by the institution/lab;
- appropriate for parallel simulation jobs submitted by trusted intranet clients.

This is the current intended GenESyS use case described by the professor.

#### Profile C — Internet-facing service

- reachable from untrusted networks;
- requires a hardened gateway, stronger identity, TLS, isolation, monitoring, patching, abuse controls, and operational incident response;
- substantially higher security and maintenance cost;
- not an approved default GenESyS deployment profile.

#### Profile D — outbound/pull worker

- the worker initiates an outbound authenticated connection to a coordinator and pulls jobs;
- avoids opening an inbound laboratory port and works behind NAT/firewalls;
- useful when a central scheduler already exists;
- requires coordinator and queue infrastructure.

### 6.2 Recommended baseline for the controlled-intranet profile

Status: `proposed`, with the deployment profile itself recorded as intended.

Minimum controls:

- bind to an explicit private interface/address; avoid accidental public interfaces;
- host firewall allowlist for the laboratory CIDR or specific clients;
- TLS for credentials and simulation data in transit;
- per-client or per-user credentials rather than one permanent global token;
- preferably mutual TLS for managed laboratory machines; alternatively short-lived signed tokens over TLS;
- token expiration, rotation, revocation, constant-time comparison, and replay resistance;
- OS-backed cryptographically secure random generation;
- rate limits, concurrent-job limits, queue quotas, payload-size limits, and timeouts;
- allowlisted simulation operations; no arbitrary shell command or unrestricted code execution;
- process isolation for jobs, dedicated service account, restricted filesystem access, controlled temporary directories, CPU/RAM/time limits, and cleanup;
- authenticated job ownership, audit logs, request IDs, and result provenance;
- explicit API versioning and compatibility checks;
- secrets absent from command-line arguments, logs, model files, and client-visible configuration;
- deny-by-default behavior when authentication or configuration is missing.

### 6.3 Practical authentication options

1. **Static bearer token over TLS**
   - simplest;
   - acceptable only for small controlled deployments if tokens are unique, rotatable, revocable, protected, and never sent without TLS;
   - weaker attribution and lifecycle management.

2. **Short-lived signed access tokens over TLS**
   - better expiration and scope;
   - requires a token issuer or lab authentication integration.

3. **Mutual TLS**
   - strong machine identity for managed laboratory computers;
   - good fit for a controlled intranet;
   - certificate provisioning/revocation requires administration.

4. **Institutional identity/OIDC behind a reverse proxy**
   - strong user identity and centralized policy;
   - more infrastructure;
   - useful when the institution already operates compatible identity services.

### 6.4 Recorded status

- Intended deployment profile: controlled academic intranet.
- Public Internet exposure by default: not approved.
- Required security hardening before intranet use: `needs-implementation`.
- Exact authentication mechanism: `needs-human-decision`; mutual TLS is the strongest natural fit for managed lab machines, while short-lived tokens over TLS are operationally simpler.

## 7. Scientific criteria for biochemical and whole-cell functionality

### 7.1 Meaning of scientific criteria

Scientific criteria define what evidence is required before claiming that a model, algorithm, import/export path, or simulation result is semantically and scientifically valid. A successful build or unit test is necessary but not sufficient.

### 7.2 Validation dimensions

1. **Software correctness**
   - lifecycle, persistence, error handling, deterministic behavior, tests, and absence of known UB/leaks.

2. **Mathematical correctness**
   - equations, stoichiometry, propensity functions, constraints, conservation laws, units, dimensions, and initial/boundary conditions match the declared formulation.

3. **Numerical correctness**
   - solver order, tolerances, stability, convergence, step-size policy, non-negativity policy, stiffness handling, stochastic sampling, and failure behavior are validated.

4. **Biochemical/biological semantic correctness**
   - species, compartments, reactions, gene-expression events, cell-cycle rules, resource budgets, and biological assumptions have explicit meanings and valid domains.

5. **Interoperability correctness**
   - supported SBML constructs are declared;
   - unsupported constructs are rejected or diagnosed explicitly;
   - import/export preserves units, identifiers, compartments, reactions, parameters, events, and annotations within the supported subset;
   - no silent semantic loss is allowed.

6. **Empirical or benchmark validation**
   - outputs are compared with analytical cases, published models, curated datasets, established simulators, or laboratory data appropriate to the intended claim.

7. **Reproducibility and provenance**
   - model version, parameter source, units, random seeds, solver configuration, dependency versions, and result provenance are recorded.

### 7.3 Claim levels

1. **Educational/demonstrative**
   - illustrates mechanisms or architecture;
   - not claimed to reproduce a biological system quantitatively.

2. **Mechanistic research prototype**
   - equations and interactions are meaningful and reference-backed;
   - selected benchmark behavior is validated;
   - quantitative biological generalization is limited.

3. **Quantitatively validated model**
   - calibrated/validated against declared datasets and protocols;
   - uncertainty, sensitivity, parameter identifiability, and prediction intervals are addressed where relevant.

4. **Predictive biological model**
   - prospective predictive claims require strong independent validation and are a much higher bar than software correctness.

### 7.4 Current default claim

Until a domain-specific validation package is approved, GenESyS whole-cell and biochemical functionality must be described as experimental/research-oriented. It must not make a general predictive-validity claim.

### 7.5 Decisions still required

- intended initial claim level: educational or mechanistic research prototype;
- supported biochemical/whole-cell scope;
- canonical units and conservation invariants;
- accepted solver/tolerance policy;
- stochastic validation methodology;
- supported SBML level/version/packages and construct matrix;
- reference models, datasets, publications, and comparator simulators;
- criteria for accepting GLPK and fallback solver equivalence or intentionally different capability.

Recorded status: validation dimensions and no-overclaim policy are `decided`; exact domain benchmark suite and claim scope are `needs-human-decision`.

## 8. Promotion gate for 2026-2

### 8.1 Meaning of promotion gate

A promotion gate is a documented go/no-go checklist that must be satisfied before code is promoted from an active development branch to a semester-stable branch such as `2026-2`.

It is not a single test. It combines technical evidence, known-risk review, packaging/documentation readiness, and explicit approval. A failed mandatory criterion blocks promotion unless an authorized waiver is documented.

### 8.2 Possible gate levels

#### Minimal gate

- canonical configure/build succeeds;
- primary unit tests pass;
- no known P0 blocker;
- known limitations documented.

Appropriate only for an internal snapshot, not a strong stable-release claim.

#### Standard gate

- clean CMake/Ninja build on the supported Ubuntu/Qt6 baseline;
- required unit and smoke tests pass;
- every supported application target builds and has a startup/smoke check;
- no open P0 issue;
- P1 issues resolved or explicitly waived with rationale and containment;
- representative model compatibility set passes;
- Debian/package artifacts build and basic install/run checks pass;
- security-sensitive worker/AI behavior matches the approved deployment profile;
- documentation, release notes, known limitations, and rollback procedure are current;
- branch/commit and CI evidence are recorded;
- human promotion approval is explicit.

This is the recommended default for `2026-2`.

#### Strict gate

Includes the standard gate plus:

- sanitizer/Valgrind or equivalent diagnostic matrix;
- broader GUI and platform matrix;
- numerical/scientific benchmark suites;
- performance/regression thresholds;
- release-candidate observation period;
- all P1 issues resolved rather than waived.

Appropriate before promotion toward `currentStable` or `master`, depending on release objectives and available infrastructure.

### 8.3 Proposed standard 2026-2 checklist

1. Branch ancestry and intended source branch confirmed.
2. Clean working tree and reproducible toolchain recorded.
3. `tests-unit`, `tests-kernel-unit`, and `tests-smoke` configure, build, and pass.
4. Required GUI, shell, worker, and model-specific smoke checks pass.
5. No unresolved P0.
6. Every unresolved P1 has an explicit owner, rationale, containment, and approval.
7. Numerical/statistical P0 paths are fixed, disabled, or clearly excluded from supported functionality.
8. Worker exposure matches the controlled-intranet security policy before network deployment.
9. Qt5 compatibility removal is complete and Qt6-only CI/build documentation is aligned.
10. Representative `.gen` models load and execute with expected behavior.
11. Debian artifacts build, install, launch, and uninstall cleanly in a controlled environment.
12. Documentation and known limitations are current.
13. Release notes and rollback/reversion plan exist.
14. Professor explicitly approves promotion.

### 8.4 Recorded status

- Meaning and structure of a promotion gate: `decided`.
- Recommended `2026-2` level: standard gate.
- Exact mandatory checklist and waiver authority: `proposed`, pending explicit human approval.

## 9. Decision summary

| Topic | Recorded position | Status |
|---|---|---|
| Current plugin build | keep plugins compiled together during current consolidation | `decided` |
| Dynamic plugin architecture | do not implement yet; gather requirements first | `deferred` |
| Future independent in-process plugins | stable C ABI is the preferred long-term candidate | `needs-human-decision` |
| Qt support | Qt6 only; remove Qt5 fallback | `decided`, `needs-implementation` |
| Numerical/statistical validation framework | analytical/reference/cross-check/property hierarchy | `decided` |
| Specific bibliography | professor may provide authoritative references per method | `needs-human-decision` |
| Optimizer current maturity | internal scaffold, not a functioning public optimizer | `decided` |
| Optimizer algorithms | select initial algorithm from professor's research or another declared source | `needs-human-decision` |
| Worker intended exposure | controlled academic intranet; public Internet not default | intended profile recorded |
| Worker authentication | mTLS or short-lived tokens over TLS; exact choice pending | `needs-human-decision` |
| Whole-cell/biochemical claim | experimental/research-oriented; no general predictive claim | `decided` |
| Scientific benchmark suite | domain scope, references, datasets, and claim level pending | `needs-human-decision` |
| Promotion gate | documented go/no-go checklist | `decided` |
| Recommended 2026-2 gate | standard gate | `proposed` |

## 10. Next human inputs that would close the remaining decisions

1. Plugin ecosystem expectations: binary-only third-party distribution, supported OS/toolchains, ABI lifetime, and hot-unload needs.
2. Numerical/statistical bibliography: references, equations/algorithms, datasets, and accepted parameterizations.
3. Optimizer source material: thesis chapters, algorithms, pseudocode/source, benchmark problems, and licensing status.
4. Worker authentication preference for the laboratory: mutual TLS, short-lived tokens, institutional identity, or another mechanism.
5. Whole-cell/biochemical intended claim level and initial reference models/datasets.
6. Explicit approval or modification of the proposed standard `2026-2` promotion gate.
