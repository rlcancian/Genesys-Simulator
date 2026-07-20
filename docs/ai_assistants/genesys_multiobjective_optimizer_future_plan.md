# GenESyS Multiobjective Optimizer Future Plan

## 1. Purpose

Define the research and implementation path for turning the current Optimizer scaffold into at least a Level 3 — Beta GenESyS capability.

Status: `deferred` for algorithm implementation; research baseline recorded.

The current `OptimizerDefaultImpl1` remains below Level 3. It must not be presented as a functioning optimizer until the acceptance criteria in this plan are satisfied.

## 2. Project maturity policy

The project-wide policy is:

1. all supported functionality must reach at least Level 3 — Beta;
2. after the supported set reaches Level 3, prioritized capabilities are promoted to Level 4 — Stable user feature.

For the Optimizer, Level 3 requires more than a running algorithm. It requires a complete supported workflow, reproducibility, error handling, cancellation, persistence/reporting where applicable, realistic GenESyS model evaluation, and benchmark evidence.

## 3. Research identification

The professor's recollection of a Zürich group/framework is most plausibly associated with:

- ETH Zürich;
- Eckart Zitzler and collaborators;
- the Systems Optimization / TIK research lineage;
- PISA — Platform and Programming Language Independent Interface for Search Algorithms.

PISA separates problem-specific variation/evaluation from selection algorithms and includes performance-assessment support. This separation is relevant to GenESyS because simulation evaluation should be decoupled from optimization algorithm logic.

## 4. Initial research baseline

### 4.1 PISA

PISA is a modular framework/interface for evolutionary multi-criterion optimization. Relevant design ideas:

- separation between selector and variator/problem modules;
- standardized communication contract;
- interchangeable algorithms and benchmark problems;
- explicit performance assessment;
- algorithm-independent experiment orchestration.

GenESyS should study PISA as an architectural reference, not copy its historical implementation blindly.

### 4.2 SPEA and SPEA2

Relevant concepts:

- elitist archive;
- strength-based fitness assignment;
- density estimation;
- archive truncation preserving diversity;
- clear separation between current population and external archive.

### 4.3 IBEA

Relevant concepts:

- indicator-based fitness assignment;
- pairwise comparison using a quality indicator;
- diversity pressure arising from the indicator rather than a separate niching mechanism.

### 4.4 HypE

HypE is a hypervolume-estimation evolutionary algorithm proposed by Johannes Bader and Eckart Zitzler for many-objective optimization.

Relevant concepts:

- hypervolume contribution as selection pressure;
- Monte Carlo estimation when exact hypervolume becomes expensive;
- explicit trade-off between estimation accuracy and computational resources;
- suitability for many-objective problems.

### 4.5 Hypervolume indicator

The research baseline should cover:

- dominated hypervolume definition;
- minimization/maximization normalization;
- reference-point selection;
- exact versus approximate computation;
- individual hypervolume contribution;
- sensitivity to objective scaling;
- weighted hypervolume and preference articulation;
- computational complexity by objective count and archive size;
- deterministic reproducibility of approximate estimators.

### 4.6 Benchmark and assessment baseline

Initial benchmark candidates:

- ZDT suite;
- DTLZ suite where appropriate;
- constrained multiobjective benchmarks;
- discrete/mixed decision-variable problems;
- simulation-based noisy benchmarks;
- benchmark problems from the professor's doctoral work.

Assessment should include:

- hypervolume;
- inverted generational distance or another explicitly justified distance metric;
- epsilon indicator where appropriate;
- feasibility and constraint violation;
- archive size and diversity;
- runtime and simulation-call count;
- statistical comparison over repeated stochastic runs.

## 5. Relationship to the professor's doctoral research

The professor's thesis, algorithms, source code, datasets, and benchmark results are preferred inputs for the first GenESyS implementation.

Future collection should identify:

- thesis citation and chapters;
- algorithm names and variants;
- pseudocode and source code;
- licensing/ownership status;
- objective and constraint models;
- decision-variable types;
- dominance and archive rules;
- diversity mechanism;
- hypervolume or other indicator usage;
- stochastic evaluation and replication policy;
- termination rules;
- benchmark problems and expected outcomes.

External ETH Zürich/PISA algorithms should be used as references, comparators, architectural inputs, or additional implementations according to licensing and project needs. They should not erase the project's own research lineage.

## 6. Proposed GenESyS architecture

Separate the following interfaces/concepts:

1. `OptimizationProblem`
   - decision-variable metadata;
   - bounds/types;
   - objectives;
   - constraints;
   - direction and scaling.

2. `CandidateSolution`
   - encoded decisions;
   - evaluated objectives;
   - constraint violations;
   - provenance and seed/replication metadata.

3. `SimulationEvaluator`
   - applies decisions to a model;
   - controls replications and random seeds;
   - executes simulations;
   - extracts responses;
   - returns diagnostics and cost.

4. `MultiobjectiveArchive`
   - dominance;
   - duplicate/equivalence policy;
   - diversity/truncation;
   - serialization.

5. `OptimizationAlgorithm_if`
   - initialization;
   - iteration/step;
   - pause/resume/cancel;
   - state persistence;
   - deterministic seed configuration;
   - progress and diagnostics.

6. `QualityIndicator_if`
   - hypervolume;
   - epsilon/other indicators;
   - reference-point and normalization policy.

7. `OptimizationExperiment`
   - repeated runs;
   - benchmark design;
   - statistical comparison;
   - result export and provenance.

GUI code must remain outside the algorithm backend.

## 7. Phased implementation

### Phase O0 — source-material acquisition

- obtain professor's thesis material and existing code;
- catalogue ETH Zürich/PISA references;
- review licenses;
- select initial problem classes.

Acceptance: documented algorithm candidates and legal/reproducible source package.

### Phase O1 — contracts and deterministic primitives

- implement decision/objective/constraint contracts;
- implement dominance and constraint comparison;
- implement deterministic archive behavior;
- implement exact hypervolume for small low-dimensional fixtures or adopt a justified implementation.

Acceptance: exhaustive focused unit tests.

### Phase O2 — first real algorithm

- implement one algorithm from the professor's research or an explicitly selected reference algorithm;
- keep algorithm independent from GUI and simulator internals;
- validate on analytical benchmark problems.

Acceptance: end-to-end deterministic optimization on declared benchmarks.

### Phase O3 — simulation integration

- evaluate GenESyS models;
- apply decision variables safely;
- extract objectives/constraints;
- control seeds and replications;
- support failures/timeouts.

Acceptance: realistic GenESyS optimization fixtures with reproducible results.

### Phase O4 — stochastic and multiobjective assessment

- repeated runs;
- uncertainty/noise handling;
- hypervolume and comparator metrics;
- statistical performance assessment;
- provenance and export.

Acceptance: benchmark report reproducible from a clean checkout.

### Phase O5 — Level 3 GUI/workflow

- complete configuration workflow;
- pause/resume/cancel;
- persistence;
- progress, diagnostics, result plots/tables, export;
- safe handling of invalid models and failed simulations.

Acceptance: all Level 3 criteria pass in backend, GUI, tests, and documentation.

### Phase O6 — Level 4 priorities

After the supported GenESyS feature set reaches Level 3, prioritize selected optimizer algorithms/workflows for stable API, broader benchmarks, performance guarantees, compatibility, and long-term regression support.

## 8. Level 3 acceptance criteria

The Optimizer may be labelled Beta only when:

- at least one genuine multiobjective algorithm executes end-to-end;
- algorithm, version, assumptions, and supported decision-variable types are documented;
- dominance, constraints, archive, indicators, and stopping behavior have unit tests;
- benchmark results are reproducible;
- model mutation/evaluation is safe and validated;
- stochastic replications and seeds are controlled;
- failure, timeout, pause, resume, cancellation, and restart/persistence policies are tested where supported;
- GUI and backend produce consistent results;
- limitations and unsupported cases are explicit;
- no scaffold-only behavior is exposed as optimization.

## 9. References to collect

Initial authoritative research targets include:

- Bleuler, Laumanns, Thiele, and Zitzler — PISA interface/framework;
- Zitzler and Thiele — Strength Pareto approach;
- Zitzler, Laumanns, and Thiele — SPEA2;
- Zitzler and Künzli — IBEA/indicator-based evolutionary algorithm;
- Bader and Zitzler — HypE;
- Zitzler, Deb, and Thiele — comparative benchmark work and ZDT problems;
- Zitzler, Thiele, Laumanns, and collaborators — performance assessment and hypervolume theory;
- the professor's doctoral thesis and related publications.

Exact editions, PDFs, source packages, benchmark datasets, and licensing must be collected in a later research round.

## 10. Current next action

Do not implement an algorithm in the current documentation PR.

The next optimizer-specific action is a research/material handoff from the professor followed by a bounded architecture-and-test PR.
