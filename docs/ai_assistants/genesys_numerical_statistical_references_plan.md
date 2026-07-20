# GenESyS Numerical and Statistical Reference Acquisition Plan

## 1. Purpose

Create a traceable reference package for validating GenESyS numerical, probabilistic, statistical, DOE, and optimization functionality.

Status: `deferred` until the professor can gather the required source material.

This plan is not a request to change algorithms immediately. It defines the evidence that must exist before a method is treated as scientifically validated.

## 2. Scope

Priority domains include:

- numerical integration and differentiation;
- ODE solvers and error control;
- PDE/Method of Lines and diffusion;
- random-number generation;
- probability distributions and inverse distributions;
- descriptive statistics;
- confidence intervals;
- parametric and non-parametric hypothesis tests;
- chi-square and Kolmogorov–Smirnov diagnostics;
- distribution fitting;
- simulation input and output analysis;
- DOE, ANOVA, RSM, desirability, and factorial designs;
- optimization and stochastic multiobjective performance assessment.

## 3. Required material per method

For each method, collect when available:

1. Full bibliographic citation.
2. Edition, chapter, section, theorem, equation, table, or algorithm number.
3. Original or peer-reviewed paper when it defines the method.
4. PDF or legally accessible source.
5. Converted Markdown/text for AI-assisted retrieval, preserving equations and tables.
6. Exact parameterization and notation used by GenESyS.
7. Valid input domain and preconditions.
8. Expected behavior for edge and degenerate cases.
9. Reference datasets or benchmark problems.
10. Expected numerical outputs and accepted tolerance.
11. Independent comparator and its version/function/configuration.
12. Reproducibility conditions, including random seeds.
13. Licensing and citation requirements for reused source code or datasets.

## 4. Evidence hierarchy

Use the following order of authority:

1. analytical solution or invariant;
2. declared primary bibliographic/standards reference;
3. published benchmark or curated dataset;
4. independent reference implementation;
5. property, convergence, conservation, and metamorphic tests;
6. regression fixtures from previously validated GenESyS behavior.

A comparator such as R, SciPy, Boost.Math, GSL, or another simulator is not automatically the specification. Record whether it is normative or corroborative.

## 5. Suggested repository structure

Do not add copyrighted PDFs without permission. Store only material that may legally be versioned.

Suggested future structure:

```text
docs/references/numerical-statistical/
  README.md
  bibliography.md
  methods/
  datasets/
  expected-results/
  provenance/
```

Large source PDFs and private working material may remain outside Git and be referenced by stable citation/provenance metadata.

## 6. Acquisition workflow

### Phase A — inventory

- list every exposed numerical/statistical operation;
- map each operation to implementation files and tests;
- assign priority according to impact and known risk;
- identify methods with no declared reference.

### Phase B — bibliography and source collection

- professor supplies or approves references;
- obtain legal copies;
- record DOI/ISBN/version and relevant sections;
- convert selected PDFs to searchable Markdown/text when useful.

### Phase C — datasets and expected values

- identify published or curated datasets;
- define compact repository fixtures;
- generate expected values with provenance;
- record parameterization and tolerances.

### Phase D — executable validation

- add focused tests before algorithm changes;
- compare with analytical/reference values;
- test edge cases and failure behavior;
- record toolchain and comparator versions;
- classify the method's maturity.

## 7. Priority order

1. `SolverDefaultImpl1` and statistical callers that depend on it.
2. Probability distribution CDF/inverse functions.
3. Hypothesis tests and confidence intervals.
4. Distribution fitting and goodness-of-fit tests.
5. ODE/PDE/diffusion implementations.
6. Random-number generation and stochastic simulation.
7. DOE/RSM/ANOVA.
8. Optimization performance assessment.

## 8. Acceptance criteria

A method may be marked scientifically validated only when:

- its formulation and parameterization are documented;
- at least one authoritative reference exists;
- analytical/reference fixtures cover nominal behavior;
- edge and invalid-input behavior is defined;
- tolerances are justified;
- independent comparison is recorded when appropriate;
- tests pass reproducibly in the supported toolchain;
- limitations and unsupported domains are explicit.

## 9. Human inputs pending

The professor will provide this material when available. The task should be raised again in a future consolidation round, but it must not block unrelated structural work that does not alter numerical semantics.
