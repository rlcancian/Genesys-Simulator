# tools package

## 1. Purpose of the tools package

The `source/tools` package hosts statistical and numerical support abstractions used by simulation and analysis flows. It provides interfaces and legacy implementations for fitting, hypothesis testing, probability utilities, and numerical solvers.

## 2. Current major abstractions

- `DataAnalyser_if`: high-level façade to orchestrate dataset-oriented analysis services.
- `SimulationResultsDatasetParser`: loader for numeric result files, including Genesys `Record` outputs split by replication and optional time/value columns.
- `Fitter_if`: fitting contract for distribution parameter inference from sample data.
- `HypothesisTester_if`: parametric inference API (confidence intervals and tests).
- `analysis/ProbabilityDistributionBase` / `analysis/ProbabilityDistribution`: static math façade for PDF/PMF and inverse/quantile routines used by the analysis tool.
- `Solver_if`: legacy numerical contract mixing integration and derivation/advancement.
- `TraitsAnalysis`: traits registry binding analysis abstractions to concrete implementations.

## 3. Current limitations

- Fitting baseline was expanded in FITTER-2: `FitterDefaultImpl` now also provides functional Beta (scaled) and Weibull fitting, still preserving controlled-failure behavior when constraints are not met.
- `FitterDummyImpl` is preserved as a legacy placeholder/documental implementation, but it is no longer the default trait binding after FITTER-3.
- Hypothesis-testing paths are implemented for the analysis-tool scope, including confidence intervals, parametric tests, chi-square goodness-of-fit and one-sample KS. Future work should focus on additional calibration policies, not on kernel/statistics coupling.
- The one-sample KS goodness-of-fit p-value is exact only when the tested CDF is fully specified before observing the sample. When parameters are fitted from the same sample, it is reported as an approximation/diagnostic value.
- `DataAnalyserDefaultImpl::newDataSet`, `saveDataSet`, default `sampler` and default `experimenter` are roadmap hooks and are not supported in the current analysis-tool scope.
- Distribution APIs are static utilities, not yet an OO hierarchy with reusable distribution objects.
- Solver abstraction conflates quadrature and ODE-like concerns.

## 4. Planned evolution

- Introduce cohesive interfaces for dataset, distributions, quadrature, root finding, and ODE solving.
- Continue hardening `FitterDefaultImpl` as the promoted default fitting implementation.
- Evolve traits coverage to include newly stabilized abstractions.
- Keep legacy interfaces during migration to avoid behavior breaks.

## 5. Relationship with kernel/statistics

`source/tools/analysis` is intended to be usable without kernel/statistics dependencies. The independent CMake target is `genesys_tools_analysis`; it contains dataset loading, fitting, hypothesis testing, probability utilities and the legacy numerical solver needed by those routines. The broader `genesys_tools` target may still aggregate legacy tools that interact with kernel-level concepts, but analysis code must not include kernel headers directly.

## 6. Current status by topic

- **Fitting**: interface defined; `FitterDefaultImpl` is functional for uniform/triangular/normal/exponential/erlang/beta/weibull with binary dataset loading and SSE-CDF comparison and is now the default `TraitsAnalysis<Fitter_if>` binding (FITTER-3). `FitterDummyImpl` remains available as legacy placeholder.
- **Hypothesis testing**: functional baseline exists in `HypothesisTesterDefaultImpl` for the current analysis-tool scope.
  - HYPTEST-1 alignment update: proportion-difference CI now follows the classical two-proportion formula, and one-population average/variance tests now compute p-values with Student-t/chi-square-coherent CDF paths.
  - HYPTEST-2 final alignment update: one-population proportion confidence intervals (with and without finite-population correction) now use the large-sample normal quantile formulation.
  - HYPTEST-3 consolidation update: `HypothesisTesterDefaultImpl` is now part of `tools/analysis`, file-based overloads use analysis dataset loaders instead of kernel/statistics collectors, and `genesys_tools_analysis` builds independently from `genesys_kernel_*`.
  - HYPTEST-4 goodness-of-fit update: chi-square and one-sample Kolmogorov-Smirnov adherence tests are exposed through `HypothesisTester_if` and implemented by `HypothesisTesterDefaultImpl`.
  - HYPTEST-5 auditability update: variance-ratio confidence intervals use the correct F degrees-of-freedom order, chi-square results expose class/df metadata, and examples/tests distinguish formal tests from EDF/CDF SSE heuristics.
- **Simulation result datasets**: `SimulationResultsDatasetParser` reads text datasets and Genesys `Record` files, preserving replication ids and optional time columns so GUI analysis can expose pooled and per-replication scopes.
- **Probability distributions**: mathematical static base and inverse façade available, with internal numeric dependencies.
- **Numerical solvers**: legacy `Solver_if` + `SolverDefaultImpl1` remain the compatible baseline.
