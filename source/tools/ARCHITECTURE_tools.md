# Architecture notes for `source/tools`

## Overview

`source/tools` groups support services that are useful outside the simulation kernel: data analysis, probability helpers and numerical utilities. The package contains both the consolidated analysis tool and legacy support APIs kept for compatibility.

## Target Boundaries

### `genesys_tools_analysis`

Standalone analysis target. It contains:

- `DataAnalyser_if` / `DataAnalyserDefaultImpl`
- Dataset loaders and simulation-result parsers
- `Fitter_if` / `FitterDefaultImpl`
- `HypothesisTester_if` / `HypothesisTesterDefaultImpl`
- Probability helpers
- The legacy solver implementation needed internally by the analysis routines

This target must build without linking `genesys_kernel_*` and without including kernel headers.

### `genesys_tools`

Aggregate/legacy target. It may link `genesys_tools_analysis` and broader GenESyS dependencies where existing consumers still require them.

## Dependency Direction

The intended direction is:

```text
applications / GUI / tests / kernel consumers
    -> genesys_tools_analysis
    -> local analysis helpers
```

`tools/analysis` must not depend on kernel statistics collectors, model objects or simulator runtime objects. If a kernel or GUI workflow needs analysis, that workflow imports the analysis target.

## Stable Analysis Contracts

- `DataAnalyserDefaultImpl` owns one validated dataset snapshot and keeps summaries, histogram, boxplot and fitter input aligned.
- `FitterDefaultImpl` is the default fitter selected by `TraitsAnalysis<Fitter_if>`.
- `HypothesisTesterDefaultImpl` is the default tester selected by `TraitsAnalysis<HypothesisTester_if>`.
- `ProbabilityDistributionBase` and `ProbabilityDistribution` expose static mathematical helpers only.
- File-based tester overloads use analysis loaders/parsers, not kernel statistics APIs.

## Compatibility

- Public legacy signatures are preserved where practical.
- `FitterDummyImpl` remains as a placeholder/documental implementation.
- `Solver_if` and `SolverDefaultImpl1` remain available while newer numerical interfaces mature.
- Roadmap hooks in `DataAnalyser_if` (`newDataSet`, `saveDataSet`, default `sampler`, default `experimenter`) stay explicit and unsupported rather than partially implemented.

## Future Work

- Introduce reusable distribution objects over the current static probability helpers.
- Split the legacy solver responsibilities into focused quadrature, root-finding and ODE components.
- Add calibrated alternatives for goodness-of-fit p-values when parameters are estimated from the tested sample.
