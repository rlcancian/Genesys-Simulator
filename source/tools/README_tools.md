# tools package

`source/tools` contains analysis and numerical support code used by GenESyS applications, examples and tests. The main current deliverable is the standalone data-analysis package in `source/tools/analysis`.

## Main Targets

| Target | Purpose |
| --- | --- |
| `genesys_tools_analysis` | Standalone data-analysis layer: dataset loading, descriptive summaries, fitting, hypothesis testing, goodness-of-fit tests and probability helpers. It must not depend on `source/kernel`. |
| `genesys_tools` | Aggregating/legacy tools target. It links `genesys_tools_analysis` and legacy helpers such as optimizer/factorial-design code. |

## Main Abstractions

| Area | Main files |
| --- | --- |
| Analysis facade | `analysis/DataAnalyser_if.h`, `analysis/DataAnalyserDefaultImpl.h` |
| Dataset loading | `analysis/DatasetLoader.h`, `analysis/SimulationResultsDataset.h`, `analysis/SimulationResultsParser.cpp` |
| Fitting | `analysis/Fitter_if.h`, `analysis/FitterDefaultImpl.h` |
| Hypothesis testing | `analysis/HypothesisTester_if.h`, `analysis/HypothesisTesterDefaultImpl.h` |
| Probability helpers | `analysis/ProbabilityDistributionBase.h`, `analysis/ProbabilityDistribution.h` |
| Defaults/traits | `analysis/TraitsAnalysis.h` |
| Legacy numerical solver | `Solver_if.h`, `SolverDefaultImpl1.h` |
| Other numerical interfaces | `Quadrature_if.h`, `RootFinder_if.h`, `OdeSystem_if.h`, `OdeSolver_if.h` |

## Analysis Package

The analysis package is documented in `source/tools/analysis/README.md`. In short, it provides:

- File and in-memory dataset loading.
- Summary, histogram and boxplot structures.
- Distribution fitting for uniform, triangular, normal, exponential, erlang, beta and weibull.
- Structured fit ranking through `fitAllSummary()`.
- Parametric confidence intervals and hypothesis tests for one and two populations.
- Chi-square and one-sample Kolmogorov-Smirnov goodness-of-fit tests.
- Probability density/mass and quantile helpers.

`DataAnalyserDefaultImpl` is the facade intended for examples and direct use. A default-constructed analyser creates the default fitter and tester from `TraitsAnalysis`.

## Tests

The data-analysis package has dedicated unit and integration tests documented in `source/tests/README.md`. These tests are CMake/CTest targets and can be executed from QtCreator. When using the repository Makefile, the focused shortcuts are:

| Test level | Makefile shortcut |
| --- | --- |
| Unit tests | `make run-unit-tests PACKAGE=tools` |
| Integration tests | `make run-integration-tests PACKAGE=tools` |
| Runnable examples/regression checks | `make run-examples` |

The examples include a standalone analysis workflow over CSV files and a small GenESyS simulation workflow that writes a `Record` dataset and analyzes that simulation output through `DataAnalyserDefaultImpl`.

## Dependency Rule

`source/tools/analysis` must remain independent from `source/kernel`. If GUI, kernel tests or other modules need analysis functionality, those consumers should depend on `genesys_tools_analysis`; analysis code should not include kernel headers or use kernel statistics collectors.

## Current Limitations

- `DataAnalyserDefaultImpl::newDataSet`, `saveDataSet`, default `sampler()` and default `experimenter()` are roadmap hooks and are not supported in the current analysis-tool scope.
- `FitterDummyImpl` remains available as a legacy placeholder but is not the default fitter binding.
- `isNormalDistributed(...)` is an EDF/CDF SSE heuristic, not a formal normality test.
- KS p-values are classical one-sample approximations. When parameters are estimated from the tested sample, treat them as diagnostic values.
- Distribution APIs are static helpers; reusable distribution objects are future work.
- The legacy solver abstraction still mixes integration and derivation/advancement concerns.
