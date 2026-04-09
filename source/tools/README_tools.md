# tools package

## 1. Purpose of the tools package

The `source/tools` package hosts statistical and numerical support abstractions used by simulation and analysis flows. It provides interfaces and legacy implementations for fitting, hypothesis testing, probability utilities, and numerical solvers.

## 2. Current major abstractions

- `DataAnalyser_if`: high-level façade to orchestrate dataset-oriented analysis services.
- `Fitter_if`: fitting contract for distribution parameter inference from sample data.
- `HypothesisTester_if`: parametric inference API (confidence intervals and tests).
- `ProbabilityDistributionBase` / `ProbabilityDistribution`: static math façade for PDF/PMF and inverse/quantile routines.
- `Solver_if`: legacy numerical contract mixing integration and derivation/advancement.
- `TraitsTools`: traits registry binding abstractions to concrete implementations.

## 3. Current limitations

- Fitting now has a functional baseline implementation in `FitterDefaultImpl` (FITTER-1), while traits still keep `FitterDummyImpl` as the default binding in this phase.
- Some hypothesis-testing paths, especially two-population paths, remain partially consolidated.
- Distribution APIs are static utilities, not yet an OO hierarchy with reusable distribution objects.
- Solver abstraction conflates quadrature and ODE-like concerns.

## 4. Planned evolution

- Introduce cohesive interfaces for dataset, distributions, quadrature, root finding, and ODE solving.
- Promote `FitterDefaultImpl` from structural placeholder to complete fitting implementation.
- Evolve traits coverage to include newly stabilized abstractions.
- Keep legacy interfaces during migration to avoid behavior breaks.

## 5. Relationship with kernel/statistics

`source/tools` consumes kernel/statistics contracts (collectors, samplers and data files) but this phase does not modify kernel or statistics code. The package remains a consumer and adapter layer over those existing contracts.

## 6. Current status by topic

- **Fitting**: interface defined; `FitterDefaultImpl` is functional for baseline families (uniform/triangular/normal/exponential/erlang) with binary dataset loading and SSE-CDF comparison, but production trait binding remains on `FitterDummyImpl`.
- **Hypothesis testing**: functional baseline exists in `HypothesisTesterDefaultImpl1`, with known partial areas.
- **Probability distributions**: mathematical static base and inverse façade available, with internal numeric dependencies.
- **Numerical solvers**: legacy `Solver_if` + `SolverDefaultImpl1` remain the compatible baseline.
