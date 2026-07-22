---
document_type: architecture
authority: normative-reference
owner: project-maintainer
last_reviewed: 2026-07-22
review_cadence: 90d
status: active
tracks: 511
---

# GenESyS Architecture and Development Boundaries

## 1. Purpose

This document records the durable architectural baseline and approved technical boundaries for GenESyS. It is intentionally separated from current CI counts, individual PRs, workflow run IDs, temporary failures, and task sequencing.

Use `STATUS.md` for current operational state and the backlogs for pending work.

## 2. Platform baseline

The supported primary development and CI baseline is:

- Ubuntu 24.04;
- CMake 3.24 or newer;
- Ninja;
- C++23, required, with compiler extensions disabled;
- Qt6 for graphical applications;
- Google Test through a compatible system package with bundled fallback where configured.

Canonical build entry points are:

```text
CMakeLists.txt
CMakePresets.json
```

Legacy qmake, Qt5, `project/`, `projects/`, `source/applications/terminal/`, and `source/applications/web/` references are historical or compatibility material unless current code explicitly retains a bounded alias.

## 3. Repository map

```text
Genesys-Simulator/
├── CMakeLists.txt
├── CMakePresets.json
├── debian/
├── docs/
│   ├── ai_assistants/
│   ├── developers/
│   ├── users/
│   └── ManualGenESyS.pdf
├── models/
├── packaging/
├── scripts/
└── source/
    ├── applications/
    │   ├── gui/
    │   ├── modelSpecific/
    │   ├── shell/
    │   └── worker/
    ├── kernel/
    │   ├── simulator/
    │   ├── statistics/
    │   └── util/
    ├── parser/
    ├── plugins/
    ├── tests/
    └── tools/
```

Generated build, package, Doxygen intermediate, temporary, cache, IDE, and diagnostic output directories are not source architecture and must not be versioned unless a specific final artifact is intentionally maintained.

## 4. Layer responsibilities

### 4.1 Kernel

The kernel provides generic simulation infrastructure:

- simulator and model lifecycle;
- managers;
- event scheduling;
- persistence;
- tracing;
- model components and data-definition foundations;
- plugin registration interfaces;
- statistics and shared utilities.

The kernel must not absorb GUI-specific, worker-route-specific, SBML-parser-specific, optimizer-GUI-specific, or domain-product behavior merely to satisfy one application.

Kernel changes require explicit ownership, lifetime, persistence, event-order, and compatibility analysis.

### 4.2 Parser

The parser owns expression and model-language interpretation. Parser grammar and generated/runtime parser behavior must remain synchronized.

Changes to expression semantics require:

- explicit language contract;
- compatibility analysis for persisted `.gen` models;
- nominal, invalid, and regression tests;
- no assumption that a GUI-side list of functions is the parser source of truth.

### 4.3 Plugins

Plugins provide model components and data definitions across discrete-event, modal, continuous, biochemical, whole-cell, external-integration, electronics, and other domains.

The current production architecture uses statically aggregated plugin libraries and static registration paths. It is not a production dynamic-plugin package architecture.

Current consolidation priorities are:

- one unambiguous source-to-target ownership model;
- consistent optional dependency behavior;
- explicit registration and metadata;
- preserved persistence and factories;
- testable lifecycle and ownership;
- no broad dynamic migration during baseline stabilization.

The approved future dynamic boundary is a stable C ABI with:

- `extern "C"` discovery/entry points;
- opaque handles;
- fixed-width scalar types;
- explicit create/destroy pairs;
- versioned function and capability tables;
- structured errors;
- explicit strings, arrays, buffers, callbacks, allocator, unload, and threading conventions;
- no STL, Qt, C++ implementation classes, or C++ exceptions crossing the boundary.

### 4.4 Tools

`source/tools/` contains reusable non-GUI backends and algorithms, including statistical, numerical, optimization, continuous-simulation, factorial-design, and AI-assistant support.

Rules:

- backend algorithms remain independent from Qt windows where practical;
- GUI prototypes do not define scientific correctness;
- numerical/statistical algorithms require declared references and oracles;
- incomplete scaffold behavior must not be exposed as mature functionality;
- tool target separation should follow real dependency boundaries, not directory changes alone.

### 4.5 Applications

Applications orchestrate user- or service-facing workflows without becoming substitutes for the kernel or reusable tool backends.

Current application areas are:

- `source/applications/shell/` — command-line shell;
- `source/applications/worker/` — HTTP/background worker runtime;
- `source/applications/gui/genesys/` — main Qt editor;
- independent Qt frontends under `source/applications/gui/`;
- `source/applications/modelSpecific/` — selected/generated model applications.

Application-specific needs must first be satisfied through existing kernel and tool contracts. A kernel change requires a demonstrated generic API limitation.

## 5. GUI architecture

GenESyS supports Qt6 only as the intended platform contract.

The GUI umbrella may add application subdirectories, but must not recursively aggregate sibling GUI application source trees into one executable.

Each independent GUI application must own:

- its `CMakeLists.txt`;
- its executable target;
- its source scope;
- its startup validation;
- its application-specific dependencies.

The main GUI should launch independent frontends through an explicit process/application-launching service instead of constructing their windows directly.

Process boundaries require explicit model/context transfer. Raw C++ pointers must not be shared across independent processes.

Qt parent-child ownership must be distinguished from non-`QObject` ownership. GUI refactoring must not introduce double deletion, retained callbacks, or widgets outliving their backing model objects.

## 6. Worker architecture and security boundary

The worker is an application runtime, not a GUI tool backend.

Its intended deployment profile is a controlled academic intranet. Direct public-Internet exposure is not an approved default.

A deployable worker architecture requires:

- explicit bind address/interface;
- authenticated clients;
- TLS or equivalent protected transport for credentials and simulation data;
- cryptographically secure credentials;
- rotation, expiry, revocation, and ownership semantics where applicable;
- payload, job, concurrency, CPU, memory, and time limits;
- restricted service identity and filesystem access;
- no unrestricted shell or arbitrary code execution;
- audit logging, request identifiers, provenance, and deny-by-default behavior.

Startup and public-health evidence do not establish security readiness.

## 7. Ownership and lifetime architecture

Raw pointers may represent observation, framework-managed lifetime, or ownership in legacy areas. Their meaning must be established from construction and destruction paths before refactoring.

Preferred principles for new or corrected code:

- Rule of Zero when possible;
- `std::unique_ptr` for exclusive ownership;
- deleted copy/move operations when a legacy owner cannot safely support them;
- explicit non-owning references/pointers;
- deterministic destruction order;
- callbacks disconnected or guarded before referenced state is destroyed;
- matching allocation/deallocation within one ABI boundary;
- no ownership transfer implied only by method naming.

Do not broadly migrate containers of raw pointers until all call sites, adoption paths, persistence behavior, and destruction order are mapped.

## 8. Event and runtime semantics

GenESyS is event-driven. Routing may enqueue future events rather than invoke a destination synchronously.

Tests and callers must distinguish:

- immediate state mutation;
- scheduled event insertion;
- event-calendar draining/processing;
- replication and simulation callbacks;
- public operations that may be called before model-wide initialization phases.

Statistics and accounting objects must be initialized safely for supported public operations and must remain absent when reporting is explicitly disabled.

## 9. Modal, continuous, and hybrid simulation

Modal models, EFSMs, Petri nets, cellular automata, and continuous systems are distinct semantic domains.

Discrete-event calendar time and continuous solver time must not be conflated. A component that advances a continuous state must define:

- independent variable and units;
- step/tolerance policy;
- relationship to elapsed simulated time;
- event-boundary behavior;
- synchronization and error semantics;
- reproducibility expectations.

Fixed-step advancement is not automatically equivalent to elapsed time between discrete events.

## 10. Numerical and statistical correctness

Correct compilation and green tests are necessary but insufficient for scientific correctness.

Each supported numerical/statistical method requires:

- exact formulation and parameterization;
- valid domain and preconditions;
- edge and degenerate behavior;
- units/scaling;
- stopping and error-control rules;
- justified tolerances;
- overflow, underflow, non-finite, and failure behavior;
- analytical invariants or a declared authoritative reference;
- reproducible fixtures and comparator provenance where used.

Independent software such as R, SciPy, Boost.Math, GSL, or another simulator may corroborate a result, but is not automatically the specification.

## 11. Optimization architecture

The optimizer must remain separated into backend contracts and GUI orchestration.

A mature design should distinguish at least:

- problem and decision-variable metadata;
- candidate solution and provenance;
- model mutation and simulation evaluation;
- objectives and constraints;
- archive and dominance behavior;
- quality indicators;
- algorithm lifecycle, cancellation, persistence, and diagnostics;
- repeated stochastic experiments and performance assessment.

The current scaffold must not be described as a functioning mature optimizer until a real reference-backed algorithm and complete Level 3 workflow exist.

## 12. Biochemical, whole-cell, and SBML architecture

Native GenESyS biochemical definitions remain the runtime model. SBML is an interoperability boundary, not a replacement for native kernel classes.

Requirements include:

- explicit supported SBML level/version/package subset;
- diagnostics for unsupported constructs;
- no silent semantic loss;
- identifier, unit, compartment, reaction, parameter, event, rule, and annotation preservation within the supported subset;
- round-trip fixtures;
- separation between bridge services, domain plugins, and generic kernel behavior.

Whole-cell modeling is a domain-specific orchestration layer above biochemical and simulation primitives. Mathematical, numerical, biochemical, biological, persistence, stochastic, and optional-solver claims must be validated separately.

## 13. AI virtual-cell research direction

The approved strategic direction is neuro-symbolic-mechanistic.

Mechanistic ODE, SSA, Petri-net, metabolic/FBA, regulatory, spatial, modal, and hybrid models remain explicit scientific tools. Learned representations and transition models may complement them when validation, uncertainty, provenance, and out-of-domain behavior are documented.

An agent layer may route tasks, generate hypotheses, select experiments, call typed/versioned tools, and check constraints. Unverified LLM output must not become scientific state without validated tool calls, consistency checks, provenance, and appropriate human approval.

Current GenESyS functionality must not be described as an implemented predictive AI virtual cell.

## 14. Python and external integration boundaries

Python-facing APIs must remain narrow and intentional.

Do not expose unstable C++ internals, raw ownership, manager internals, callbacks, or open abstract interfaces without dedicated wrappers and lifetime contracts.

Generated C++/Python code, compilers, external processes, dynamic loading, temporary files, and scripting require security, cleanup, path, error, unload, and package-dependency validation.

## 15. Persistence and compatibility

Persistence changes require:

- load/save symmetry;
- compatibility fixtures for supported historical models;
- explicit defaults for missing fields;
- diagnostics for unsupported or invalid data;
- no casual renaming of plugin types, field names, exported identifiers, targets, or installed binaries.

Directory and target refactors must preserve user-visible names unless a separately approved compatibility migration exists.

## 16. Maturity boundary

The minimum target for supported functionality is Level 3 — Beta. Level 3 requires the intended workflow, tests, error behavior, reproducibility, cancellation/persistence/reporting where applicable, realistic fixtures, and documented limitations.

Level 4 requires stronger stability, compatibility, performance, long-term regression, and user-facing support guarantees.

Software maturity never implies predictive scientific validity.
