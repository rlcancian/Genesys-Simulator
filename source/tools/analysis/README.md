# tools/analysis

`tools/analysis` is the standalone data-analysis layer of GenESyS. It provides dataset loading, descriptive summaries, distribution fitting, confidence intervals, hypothesis tests and goodness-of-fit tests through the `genesys_tools_analysis` CMake target.

This target must remain independent from `source/kernel`: analysis code must not include kernel headers or depend on kernel statistics collectors. Consumers that need analysis should link/import `genesys_tools_analysis`.

## UML Diagrams

The main architecture diagrams are available as rendered images in `diagrams/`.
They are intentionally high-level and should be read in this order.

Core class view centered on `DataAnalyserDefaultImpl`, its default fitter/tester, dataset loader and local probability helpers:

![Tools/Analysis class overview](diagrams/analysis_class_overview.png)

Essential user-facing workflows supported by the analysis tool:

![Tools/Analysis use cases](diagrams/analysis_use_cases.png)

Typical runtime flow for loading a dataset, computing summaries, fitting distributions and running tests:

![Tools/Analysis dataset flow](diagrams/analysis_dataset_flow_sequence.png)

Dependency direction between consumers, `tools/analysis` and `source/kernel`:

![Tools/Analysis dependency boundaries](diagrams/analysis_dependency_boundaries.png)

## Facade

`DataAnalyser_if` is the facade contract and `DataAnalyserDefaultImpl` is the default implementation. A default-constructed analyser is ready to use; it creates the default fitter and tester from `TraitsAnalysis`.

Supported facade operations:

| Operation | Behavior |
| --- | --- |
| `loadDataSet(filename)` | Loads and validates a numeric dataset from a file. Returns `false` when the file is not usable. |
| `loadDataSet(data)` | Loads and validates an in-memory numeric vector. Returns `false` for empty or non-finite data. |
| `data()` / `sortedData()` | Exposes the current dataset snapshot in input order or sorted order. |
| `summary()` | Returns count, min, max, mean, sample variance, sample standard deviation and negative-data flag. |
| `histogram(classCount)` | Returns numeric bins and frequencies. `classCount = 0` uses Sturges' rule. |
| `boxplot()` | Returns quartiles, IQR fences, whiskers and outliers using the 1.5 IQR rule. |
| `fitter()` | Returns the active `Fitter_if`; default is `FitterDefaultImpl`. |
| `tester()` | Returns the active `HypothesisTester_if`; default is `HypothesisTesterDefaultImpl`. |

The facade loads a dataset once into an internal snapshot and forwards the same validated values to the fitter. `HypothesisTesterDefaultImpl` is stateless; when a test needs raw observations, pass `analyser.data()` so the tester uses the same sample as `summary()`, `histogram()`, `boxplot()` and `fitter()`.

Out of scope for the current tool: `newDataSet(...)`, `saveDataSet(...)`, default `sampler()` and default `experimenter()`. These methods remain roadmap hooks and throw `std::runtime_error` unless an external collaborator was explicitly injected where supported.

## Fitting

`FitterDefaultImpl` fits these distributions: uniform, triangular, normal, exponential, erlang, scaled beta and weibull. It can also run all candidates through `fitAll(...)` or `fitAllSummary()`.

Parameter estimation is distribution-specific, then candidates are ranked by the EDF/CDF squared-error measure:

```text
SE = sum_i (F(x_i) - p_i)^2
```

where `F(x_i)` is the theoretical CDF at the sorted sample value and `p_i = (i + 0.5) / n` is Hazen's empirical plotting position.

`fitAllSummary()` returns a structured `FitSummary` with `bestFit` and the full ranked list of `FittingResult` entries. The legacy pointer-based fitting methods remain available for compatibility.

`isNormalDistributed(...)` is only an EDF/CDF SSE heuristic. It is not a formal normality test and does not return a p-value. Use `kolmogorovSmirnov(...)` or `chiSquareGoodnessOfFit(...)` when an explicit goodness-of-fit statistic is needed.

## Hypothesis Testing

`HypothesisTesterDefaultImpl` implements classical inference for one and two populations:

| Area | Supported methods |
| --- | --- |
| Confidence intervals | Mean, proportion, finite-population proportion, variance, difference of means, difference of proportions and variance ratio. |
| Parametric tests | Mean, proportion and variance tests for one or two populations. |
| Goodness of fit | Chi-square and one-sample Kolmogorov-Smirnov. |

All `confidenceLevel` arguments must be probabilities in `(0, 1)`, for example `0.95`.

`TestResult` exposes `pValue()`, `testStat()`, `rejectH0()`, `acceptH0()` and acceptance-region limits. For chi-square tests, `goodnessOfFitDetails()` also exposes initial class count, effective class count after grouping, estimated parameters, degrees of freedom, observed total and expected total.

`ConfidenceInterval::halfWidth()` is a true margin of error only for symmetric intervals such as means and proportions. For asymmetric intervals such as variance and variance ratio, it is only half of the displayed interval width.

Two-population mean inference keeps the current policy: pooled t-test when the variance-ratio confidence interval is compatible with 1.0; Welch otherwise.

### Goodness-of-fit Notes

Chi-square from raw samples accepts either explicit class boundaries or an automatic equal-width class count. Expected frequencies are computed from the supplied CDF and adjacent classes are grouped until they satisfy `minExpectedFrequency`. Degrees of freedom are computed after grouping:

```text
df = effectiveClasses - 1 - estimatedParameters
```

If `df <= 0`, the method throws `std::invalid_argument`.

The KS p-value is the classical one-sample approximation for a fully specified CDF. When distribution parameters are estimated from the same sample, treat the p-value as diagnostic; no Lilliefors, bootstrap or Monte Carlo calibration is applied.

## Probability Helpers

`ProbabilityDistributionBase` and `ProbabilityDistribution` are static mathematical helpers used by the fitter, tester and examples.

| Class | Purpose |
| --- | --- |
| `ProbabilityDistributionBase` | PDF/PMF helpers such as normal, Student-t, chi-square, Fisher-Snedecor, beta, gamma, erlang, exponential, lognormal, poisson, triangular, uniform and weibull. |
| `ProbabilityDistribution` | Quantile helpers such as `inverseNormal`, `inverseTStudent`, `inverseChi2` and `inverseFFisherSnedecor`. |

The public headers expose only mathematical APIs. Numerical integration and quantile caching are implementation details.

## Dataset Parsing

`DatasetLoader`, `SimulationResultsDataset` and `SimulationResultsParser` support plain numeric files and GenESyS result files with replication/time/value metadata. They are local to `tools/analysis` and do not depend on kernel collectors.

## Tests

Automated tests for this tool are documented in `source/tests/README.md`.

The tests are CMake/CTest targets and can be built or executed from QtCreator. The commands below are Makefile shortcuts for focused terminal runs.

| Test level | Location | Makefile shortcut | Latest recorded result |
| --- | --- | --- | --- |
| Unit | `source/tests/unit/tools/analysis` | `make run-unit-tests PACKAGE=tools` | 95/95 passed on 2026-06-22 |
| Integration | `source/tests/integration/tools/analysis` | `make run-integration-tests PACKAGE=tools` | 2/2 passed on 2026-06-22 |
| Example regression | `examples/analysis_tools_example.cpp` | `make run-examples` | All checks passed on 2026-06-22 |

The integration tests exercise the public facade with bundled datasets and verify that dataset loading, descriptive structures, fitting, confidence intervals, hypothesis tests, goodness-of-fit tests and file/memory ingestion work together.

## Example

```cpp
#include "tools/analysis/DataAnalyserDefaultImpl.h"

#include <iostream>

int main() {
    DataAnalyserDefaultImpl analyser;
    if (!analyser.loadDataSet("examples/data/sample_data.csv")) {
        return 1;
    }

    auto summary = analyser.summary();

    double error = 0.0;
    double mean = 0.0;
    double stddev = 0.0;
    analyser.fitter()->fitNormal(&error, &mean, &stddev);

    auto meanCi = analyser.tester()->averageConfidenceInterval(
        summary.mean,
        summary.stddev,
        static_cast<unsigned int>(summary.count),
        0.95
    );

    std::cout << "n=" << summary.count
              << " mean=" << summary.mean
              << " normal_error=" << error
              << " mean_ci=[" << meanCi.inferiorLimit()
              << ", " << meanCi.superiorLimit() << "]\n";
}
```

A complete runnable example is available in `examples/analysis_tools_example.cpp`. It can be built from the CMake examples target or from QtCreator. When using the repository Makefile, run it from the repository root with:

```sh
make run-examples
```

## Arena Input Analyzer Differences

Results can differ from Arena Input Analyzer even for the same apparent distribution. The main reasons are:

- Arena may report MLE standard deviation (`n`) while this module reports sample standard deviation (`n - 1`).
- Fitted parameters and intermediate values may use different rounding policies.
- The fitter ranking uses EDF/CDF squared error; Arena commonly reports a histogram-based error.
- Chi-square statistics depend on class boundaries, grouping policy and degrees of freedom.
- KS statistics and p-values depend on the exact CDF and on whether parameters were estimated from the same sample.
