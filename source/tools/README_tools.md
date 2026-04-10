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

- Fitting baseline was expanded in FITTER-2: `FitterDefaultImpl` now also provides functional Beta (scaled) and Weibull fitting, still preserving controlled-failure behavior when constraints are not met.
- `FitterDummyImpl` is preserved as a legacy placeholder/documental implementation, but it is no longer the default trait binding after FITTER-3.
- Some hypothesis-testing paths, especially two-population paths, remain partially consolidated.
- Distribution APIs are static utilities, not yet an OO hierarchy with reusable distribution objects.
- Solver abstraction conflates quadrature and ODE-like concerns.

## 4. Planned evolution

- Introduce cohesive interfaces for dataset, distributions, quadrature, root finding, and ODE solving.
- Continue hardening `FitterDefaultImpl` as the promoted default fitting implementation.
- Evolve traits coverage to include newly stabilized abstractions.
- Keep legacy interfaces during migration to avoid behavior breaks.

## 5. Relationship with kernel/statistics

`source/tools` consumes kernel/statistics contracts (collectors, samplers and data files) but this phase does not modify kernel or statistics code. The package remains a consumer and adapter layer over those existing contracts.

## 6. Current status by topic

- **Fitting**: interface defined; `FitterDefaultImpl` is functional for uniform/triangular/normal/exponential/erlang/beta/weibull with binary dataset loading and SSE-CDF comparison and is now the default `TraitsTools<Fitter_if>` binding (FITTER-3). `FitterDummyImpl` remains available as legacy placeholder.
- **Hypothesis testing**: functional baseline exists in `HypothesisTesterDefaultImpl1`, with known partial areas.
  - HYPTEST-1 alignment update: proportion-difference CI now follows the classical two-proportion formula, and one-population average/variance tests now compute p-values with Student-t/chi-square-coherent CDF paths.
- **Probability distributions**: mathematical static base and inverse façade available, with internal numeric dependencies.
- **Numerical solvers**: legacy `Solver_if` + `SolverDefaultImpl1` remain the compatible baseline.
