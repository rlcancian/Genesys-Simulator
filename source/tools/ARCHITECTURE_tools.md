# Architecture Notes for `source/tools`

This document records architectural boundaries for `source/tools`. User-facing capabilities, examples and test execution notes are documented in `README_tools.md` and `analysis/README.md`.

## Scope

`source/tools` contains support services that are useful outside the simulation kernel. The consolidated data-analysis package is the main current deliverable; legacy numerical and optimization helpers remain for compatibility.

## Target Boundaries

| Target | Boundary |
| --- | --- |
| `genesys_tools_analysis` | Standalone analysis library. It contains the analysis facade, dataset loaders/parsers, fitter, hypothesis tester, probability helpers and the pre-existing solver implementation integrated for analysis routines. |
| `genesys_tools` | Aggregate/legacy target. It may link `genesys_tools_analysis` and older tools such as optimizer/factorial-design code. |

`genesys_tools_analysis` must build without linking `genesys_kernel_*` and without including kernel headers.

## Dependency Direction

The intended direction is:

```text
applications / GUI / tests / kernel consumers
    -> genesys_tools_analysis
    -> local analysis helpers
```

If a kernel, GUI or application workflow needs analysis behavior, that workflow should import/link the analysis target. The analysis package must not depend on kernel statistics collectors, simulator runtime objects or model objects.

## Stable Analysis Contracts

- `DataAnalyserDefaultImpl` owns one validated dataset snapshot and keeps summary, histogram, boxplot and default fitter input aligned.
- `FitterDefaultImpl` is the default fitter selected by `TraitsAnalysis<Fitter_if>`.
- `HypothesisTesterDefaultImpl` is the default tester selected by `TraitsAnalysis<HypothesisTester_if>`.
- File-based tester overloads use analysis loaders/parsers, not kernel statistics APIs.
- `ProbabilityDistributionBase` and `ProbabilityDistribution` expose static mathematical helpers only.

## Compatibility and Roadmap

- Public legacy signatures are preserved where practical.
- `FitterDummyImpl` remains as a legacy placeholder, but it is not the default fitter binding.
- `analysis/Solver_if` and `analysis/SolverDefaultImpl` provide the numerical
  quadrature used by fitting and inference. They are a relocated, renamed
  legacy implementation rather than a new analysis-tool capability; derivation
  overloads remain only for compatibility with older external consumers.
- `DataAnalyser_if::newDataSet`, `saveDataSet`, default `sampler()` and default `experimenter()` are explicit roadmap hooks and are unsupported in the current analysis-tool scope.
- Future work may introduce reusable distribution objects and calibrated goodness-of-fit p-values when parameters are estimated from the tested sample.
