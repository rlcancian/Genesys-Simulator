# tools/analysis

`tools/analysis` is the standalone statistical-analysis layer used by GenESyS tools and examples. It provides dataset loading, distribution fitting and parametric hypothesis testing behind a small facade, `DataAnalyserDefaultImpl`.

The CMake target for this layer is `genesys_tools_analysis`. It must remain independent from `source/kernel`: analysis code should not include kernel headers directly. Consumers that need analysis should depend on this target.

## DataAnalyser

`DataAnalyser_if` is the facade contract. `DataAnalyserDefaultImpl` is the concrete implementation currently used by examples and tests.

Implemented behavior:

| Method | Current behavior |
|---|---|
| `DataAnalyserDefaultImpl()` | Builds a ready-to-use analyser with default fitter and hypothesis tester selected by `TraitsAnalysis`. |
| `DataAnalyserDefaultImpl(fitter, sampler, experimenter, tester)` | Allows dependency injection. Null `fitter` and `tester` are replaced by defaults. |
| `loadDataSet(filename)` | Stores the dataset filename and forwards it to `fitter()->setDataFilename(filename)`. |
| `fitter()` | Returns the active `Fitter_if`. By default this is `FitterDefaultImpl`. |
| `tester()` | Returns the active `HypothesisTester_if`. By default this is `HypothesisTesterDefaultImpl`. |
| `sampler()` | Future roadmap hook. Returns injected sampler, or throws `std::runtime_error("TODO: implement ...")` when none is injected. |
| `experimenter()` | Future roadmap hook. Returns injected experiment manager, or throws `std::runtime_error("TODO: implement ...")` when none is injected. |
| `saveDataSet(...)`, `newDataSet(...)` | Not implemented yet; both throw `std::runtime_error`. |

Default implementations are centralized in `TraitsAnalysis`:

| Interface | Default implementation |
|---|---|
| `Fitter_if` | `FitterDefaultImpl` |
| `HypothesisTester_if` | `HypothesisTesterDefaultImpl` |
| `Solver_if` | `SolverDefaultImpl1` |

## Fitter

`Fitter_if` defines the distribution-fitting API. `FitterDefaultImpl` reads the dataset configured by `setDataFilename(...)`, estimates parameters and reports a squared-error-like adherence measure.

Implemented fitting methods:

| Method | Distribution / purpose | Main returned parameters |
|---|---|---|
| `fitUniform` | Uniform | `min`, `max` |
| `fitTriangular` | Triangular | `min`, `mo`, `max` |
| `fitNormal` | Normal | `avg`, `stddev` |
| `fitExpo` | Exponential | `avg` |
| `fitErlang` | Erlang | `avg`, `m` |
| `fitBeta` | Scaled Beta | `alpha`, `beta`, `infLimit`, `supLimit` |
| `fitWeibull` | Weibull | `alpha`, `scale` |
| `fitAll` | Compares all candidates | best error and distribution name |
| `isNormalDistributed` | Normality check | boolean decision |

Parameter estimation uses the Method of Moments (MOM), based on sample mean, sample variance and observed extremes.

## Hypothesis Tester

`HypothesisTester_if` defines classical parametric inference methods. `HypothesisTesterDefaultImpl` implements confidence intervals and hypothesis tests for mean, proportion and variance, for one and two populations.

Implemented confidence intervals:

| Method group | Statistical model |
|---|---|
| `averageConfidenceInterval` | Mean, using Student-t. |
| `proportionConfidenceInterval` | Proportion, using normal approximation; supports finite-population correction. |
| `varianceConfidenceInterval` | Variance, using chi-square. |
| `averageDifferenceConfidenceInterval` | Difference of means; pooled t or Welch depending on variance-ratio compatibility. |
| `proportionDifferenceConfidenceInterval` | Difference of proportions, using normal approximation. |
| `varianceRatioConfidenceInterval` | Ratio of variances, using F distribution. |

Implemented hypothesis tests:

| Method group | Statistical model |
|---|---|
| `testAverage` | Mean test for one or two populations. |
| `testProportion` | Proportion test for one or two populations. |
| `testVariance` | Variance test for one or two populations. |

Each `TestResult` exposes:

| Field / method | Meaning |
|---|---|
| `pValue()` | Test p-value. |
| `testStat()` | Observed test statistic. |
| `rejectH0()` / `acceptH0()` | Decision at the requested confidence level. |
| `acceptanceInferiorLimit()` / `acceptanceSuperiorLimit()` | Acceptance region limits for the test statistic. |

File-based overloads load numeric data through `DatasetLoader` and `SimulationResultsParser`, not through kernel statistics collectors.

All inference methods expect `confidenceLevel` in `(0, 1)`, for example `0.95`.

## Error Measure

The fitter reports an error computed as a Cramer-von Mises-style squared difference between the theoretical CDF and the empirical CDF over the sorted sample:

```text
SE = sum_i (F(x_i) - p_i)^2
```

where `F(x_i)` is the theoretical CDF at the sorted sample value and `p_i = (i + 0.5) / n` is Hazen's empirical plotting position. The `+ 0.5` avoids exact 0 and 1 endpoints, which are numerically problematic for distributions with infinite support such as the Normal.

## Example

```cpp
#include "tools/analysis/DataAnalyserDefaultImpl.h"

#include <iostream>
#include <string>

bool greaterThan50(double value) {
    return value > 50.0;
}

int main() {
    const std::string dataFile = "examples/data/sample_data.csv";

    DataAnalyserDefaultImpl analyser;
    analyser.loadDataSet(dataFile);

    double normalError = 0.0;
    double mean = 0.0;
    double stddev = 0.0;
    analyser.fitter()->fitNormal(&normalError, &mean, &stddev);

    std::string bestDistribution;
    double bestError = 0.0;
    analyser.fitter()->fitAll(&bestError, &bestDistribution);

    auto meanCi = analyser.tester()->averageConfidenceInterval(dataFile, 0.95);
    auto meanTest = analyser.tester()->testAverage(
        dataFile,
        50.0,
        0.95,
        HypothesisTester_if::DIFFERENT
    );

    auto proportionCi = analyser.tester()->proportionConfidenceInterval(
        dataFile,
        greaterThan50,
        0.95
    );

    std::cout << "Normal fit: mean=" << mean
              << " stddev=" << stddev
              << " error=" << normalError << "\n";
    std::cout << "Best fit: " << bestDistribution
              << " error=" << bestError << "\n";
    std::cout << "Mean CI: [" << meanCi.inferiorLimit()
              << ", " << meanCi.superiorLimit() << "]\n";
    std::cout << "Mean test p-value: " << meanTest.pValue()
              << " rejectH0=" << meanTest.rejectH0() << "\n";
    std::cout << "P(value > 50) CI: [" << proportionCi.inferiorLimit()
              << ", " << proportionCi.superiorLimit() << "]\n";
}
```

A more complete runnable example is available in `examples/examples_analysis_tools.cpp` and can be executed from the repository root with:

```sh
make run-examples
```

## Arena Input Analyzer Differences

Some results may differ from Arena Input Analyzer even when the fitted distribution is the same. These differences are expected:

- **Standard deviation:** Arena displays the value computed with denominator `n` (MLE estimator), while this module uses denominator `n - 1` (unbiased sample estimator). For large samples the difference is small; for smaller samples, such as `n = 40`, it can be visible.
- **Rounding:** Arena performs intermediate display rounding. Means, standard deviations and fitted parameters may look different even when the underlying numerical difference is insignificant.
- **Squared error formula:** this module uses a Cramer-von Mises-style pointwise empirical CDF comparison. Arena Input Analyzer commonly reports a histogram-based `chi-square / n` measure. These metrics are not numerically comparable.
