---
document_type: reference
authority: technical-reference
owner: project-maintainer
last_reviewed: 2026-07-22
review_cadence: on-scientific-contract-change
status: active
tracks: 511
---

# Scientific and Hybrid Simulation Domains Reference

## 1. Scope

This reference consolidates durable guidance for continuous, hybrid, modal, Petri-net, cellular-automata, biochemical, SBML, whole-cell, statistical, numerical and AI virtual-cell work.

It does not declare these features scientifically validated. Current evidence and blockers are recorded in [`../STATUS.md`](../STATUS.md) and [`../BACKLOG_HUMAN.md`](../BACKLOG_HUMAN.md).

## 2. Universal scientific requirements

Every supported method or model must declare:

- mathematical formulation and parameterization;
- variables, domains and units;
- preconditions and invalid/degenerate behavior;
- numerical method, tolerances and stopping rules;
- deterministic seeds/reproducibility controls where stochastic;
- authoritative reference or analytical invariant;
- fixtures, expected values and comparator provenance;
- uncertainty, limitations and out-of-domain behavior;
- persistence and interoperability assumptions;
- exact software maturity and scientific claim level.

Compilation or plausible output is not validation.

## 3. Continuous and hybrid simulation

Discrete-event calendar time and continuous solver time are separate concepts.

A continuous/hybrid component must define:

- independent variable and units;
- initial conditions and state vector;
- step-size/tolerance policy;
- relationship between solver advancement and elapsed simulated time;
- event-boundary synchronization;
- discontinuity/reset behavior;
- failure and non-finite-value handling;
- reproducibility and reporting.

Fixed-step progression is not automatically equivalent to time elapsed between discrete events.

## 4. Modal models, EFSMs and Petri nets

Keep these concepts explicit:

- state/place and transition identity;
- guard/enabling semantics;
- action/firing semantics;
- token/state ownership;
- deterministic versus stochastic timing;
- conflict/resolution priority;
- nested/submodel boundary;
- persistence and visualization;
- synchronization with discrete-event and continuous domains.

Do not collapse distinct formalisms into one generic transition class without preserving their invariants.

## 5. Cellular automata and spatial models

Specify:

- lattice/mesh topology;
- neighborhood definition;
- boundary conditions;
- synchronous/asynchronous update policy;
- deterministic/stochastic rule behavior;
- state type and persistence;
- conservation/invariant expectations;
- interaction with simulation time and events.

Visualization and mouse editing are application concerns; state transition semantics belong in reusable model/tool layers.

## 6. Numerical tools

For integration, ODE/PDE, root finding, interpolation, fitting and linear algebra:

- use reference problems with known behavior;
- test convergence order where applicable;
- cover stiffness, long-time behavior and event discontinuities when supported;
- define overflow/underflow/NaN/Inf handling;
- do not expose an unsupported derivative or solver mode that silently returns a plausible value;
- separate software correctness from model appropriateness.

Independent software may corroborate results but is not automatically the specification.

## 7. Statistical tools and input/output analysis

For distributions, fitting, tests, intervals, output analysis and DOE:

- declare distribution parameterization;
- distinguish sample versus population estimators;
- define tie/discrete-data treatment;
- define degrees of freedom and estimated-parameter corrections;
- validate p-values/critical values against authoritative references;
- use deterministic random seeds for tests;
- record assumptions such as independence, stationarity and normality;
- report confidence level, effect size and practical limitations;
- preserve raw data provenance and transformations.

Chi-square and Kolmogorov–Smirnov support require explicit binning/parameter-estimation rules and reference-backed expected results.

## 8. Experimental design and optimization

DOE/RSM functionality should separate:

- factor/level/coding design;
- randomization, replication and blocking;
- response collection;
- model fitting and diagnostics;
- ANOVA/effect interpretation;
- optimization/recommendation;
- experiment persistence and reproducibility.

Optimizer functionality should separate:

- decision variables and bounds;
- candidate/provenance;
- model mutation/evaluation;
- stochastic replications;
- objectives/constraints;
- archive/dominance;
- quality indicators;
- cancellation/checkpointing;
- benchmark suite.

The current optimizer remains a scaffold until a real algorithm and Level 3 workflow are selected and validated.

## 9. Biochemical models

Native GenESyS biochemical definitions remain the runtime model.

Model at least:

- compartments/volumes;
- species and amounts/concentrations;
- parameters and units;
- reactions/stoichiometry;
- kinetic laws;
- events/rules where supported;
- deterministic or stochastic solver choice;
- mass/charge/conservation constraints where applicable;
- provenance and annotation.

Do not infer biological validity from syntax or solver completion.

## 10. SBML interoperability

SBML is an interoperability boundary, not a replacement for native kernel/domain classes.

A supported bridge must define:

- SBML level/version/packages;
- supported and unsupported constructs;
- identifier/unit/compartment/species/reaction/parameter mapping;
- events, rules and annotations;
- no silent semantic loss;
- diagnostics with enough context;
- import/export/round-trip fixtures;
- default and extension behavior;
- separation between bridge services and generic kernel behavior.

## 11. Whole-cell modeling

Whole-cell is a domain orchestration layer above reusable mechanisms. Validate separately:

- metabolic/FBA;
- signaling/regulatory networks;
- gene expression/protein turnover;
- replication/division/cell-cycle events;
- spatial/compartment state;
- stochastic coupling;
- optional dependencies such as GLPK;
- persistence and reproducibility;
- data provenance and calibration.

A successful WholeCell test does not automatically validate all constituent biological mechanisms.

## 12. AI virtual-cell direction

The approved direction is neuro-symbolic-mechanistic:

- mechanistic models remain explicit tools;
- learned models may complement calibration, reduction, emulation, transition prediction or experiment selection;
- typed/versioned tools execute transformations and simulations;
- agents may plan, route, compare and check constraints;
- invariants, uncertainty, provenance and human review remain mandatory;
- unverified LLM text must never become scientific state directly.

Current GenESyS must not be described as an implemented predictive AI virtual cell.

## 13. Electronics and external simulators

Circuit/electronics integration must define:

- model/netlist representation;
- units and component conventions;
- external simulator/version contract;
- file/process lifecycle;
- error/cancellation behavior;
- result mapping and provenance;
- synchronization with GenESyS time/state;
- security and resource restrictions.

SPICE or another external engine remains an integration dependency, not an invisible implementation detail.

## 14. Claim levels

Use explicit language:

- **educational/demonstrative**;
- **mechanistic research prototype**;
- **quantitatively validated for a declared domain**;
- **predictive**, only with strong independent evidence.

Software Level 3/4 and scientific claim level are independent.
