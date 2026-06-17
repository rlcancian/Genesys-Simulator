# tools/analysis

`tools/analysis` is the standalone statistical-analysis layer used by GenESyS tools and examples. It provides dataset loading, distribution fitting, parametric hypothesis testing and goodness-of-fit tests behind a small facade, `DataAnalyserDefaultImpl`.

The CMake target for this layer is `genesys_tools_analysis`. It must remain independent from `source/kernel`: analysis code should not include kernel headers directly. Consumers that need analysis should depend on this target.

## DataAnalyser

`DataAnalyser_if` is the facade contract. `DataAnalyserDefaultImpl` is the concrete implementation currently used by examples and tests.

Implemented behavior:

| Method                                                           | Current behavior                                                                                                                       |
| ---------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------- |
| `DataAnalyserDefaultImpl()`                                      | Builds a ready-to-use analyser with default fitter and hypothesis tester selected by `TraitsAnalysis`.                                 |
| `DataAnalyserDefaultImpl(fitter, sampler, experimenter, tester)` | Allows dependency injection. Null `fitter` and `tester` are replaced by defaults.                                                      |
| `loadDataSet(filename)`                                          | Validates that the dataset is loadable, stores one in-memory snapshot, preserves the source filename and forwards the same validated values to the fitter. Returns `false` when the file cannot be loaded as a usable numeric dataset. |
| `loadDataSet(data)`                                              | Validates an in-memory numeric dataset, stores one snapshot and forwards the same values to `fitter()->setData(data)`. Returns `false` for empty or non-finite data. |
| `data()`                                                         | Returns a read-only view of the current dataset snapshot in input order. Use this when calling tester methods that accept raw samples. |
| `sortedData()`                                                   | Returns a read-only sorted view of the current dataset snapshot.                                                                      |
| `summary()`                                                      | Returns a minimal descriptive summary for the currently loaded dataset.                                                                |
| `histogram(classCount)`                                          | Returns numeric histogram bins. `classCount = 0` uses Sturges' rule; bins are half-open except the last bin, which includes the upper bound. |
| `boxplot()`                                                      | Returns quartiles, fences, whiskers and outliers using the 1.5 IQR rule.                                                              |
| `fitter()`                                                       | Returns the active `Fitter_if`. By default this is `FitterDefaultImpl`.                                                                |
| `tester()`                                                       | Returns the active `HypothesisTester_if`. By default this is `HypothesisTesterDefaultImpl`.                                            |
| `sampler()`                                                      | Not supported in the current analysis-tool scope. Returns an injected sampler when provided, otherwise throws `std::runtime_error("TODO: implement ...")`. |
| `experimenter()`                                                 | Not supported in the current analysis-tool scope. Returns an injected experiment manager when provided, otherwise throws `std::runtime_error("TODO: implement ...")`. |
| `saveDataSet(...)`, `newDataSet(...)`                            | Outside the current scope and not supported. Both throw `std::runtime_error`.                                                          |

Default implementations are centralized in `TraitsAnalysis`:

| Interface             | Default implementation        |
| --------------------- | ----------------------------- |
| `Fitter_if`           | `FitterDefaultImpl`           |
| `HypothesisTester_if` | `HypothesisTesterDefaultImpl` |
| `Solver_if`           | `SolverDefaultImpl1`          |

`DataSetSummary` exposes `usable`, `count`, `min`, `max`, `mean`, `variance`, `stddev` and `hasNegativeData`.

`DataSetHistogram` exposes `usable`, `count`, `min`, `max`, `classWidth` and a vector of `HistogramBin` entries with `lowerLimit`, `upperLimit`, `frequency` and `relativeFrequency`.

`DataSetBoxPlot` exposes `usable`, `count`, `min`, `firstQuartile`, `median`, `thirdQuartile`, `max`, `interquartileRange`, `lowerFence`, `upperFence`, `lowerWhisker`, `upperWhisker` and `outliers`.

These descriptive structures are computed by `DatasetLoader` and local `tools/analysis` helpers, so the facade remains independent from `source/kernel`.

The facade keeps `summary()`, `histogram()`, `boxplot()`, `fitter()` and direct `tester()` calls aligned by loading the dataset once into an internal snapshot. File-based loads preserve the source filename for diagnostics and compatibility, but the default fitter receives the already validated vector instead of re-reading the file. `HypothesisTesterDefaultImpl` is stateless; when a test needs raw observations, pass `analyser.data()` or `analyser.sortedData()` so it uses the same snapshot as the fitter and descriptive statistics.

Dataset creation/persistence and simulation workflow orchestration are intentionally outside this consolidation. `newDataSet(...)`, `saveDataSet(...)`, default `sampler()` and default `experimenter()` are placeholders for future work, not partial implementations of supported features.

## Fitter

`Fitter_if` defines the distribution-fitting API. `FitterDefaultImpl` reads the dataset configured by `setDataFilename(...)`, `setData(...)` or `setData(data, sourceFilename)`, estimates parameters and reports a squared-error-like adherence measure.

Implemented fitting methods:

| Method                | Distribution / purpose  | Main returned parameters                |
| --------------------- | ----------------------- | --------------------------------------- |
| `fitUniform`          | Uniform                 | `min`, `max`                            |
| `fitTriangular`       | Triangular              | `min`, `mo`, `max`                      |
| `fitNormal`           | Normal                  | `avg`, `stddev`                         |
| `fitExpo`             | Exponential             | `avg`                                   |
| `fitErlang`           | Erlang                  | `avg`, `m`                              |
| `fitBeta`             | Scaled Beta             | `alpha`, `beta`, `infLimit`, `supLimit` |
| `fitWeibull`          | Weibull                 | `alpha`, `scale`                        |
| `fitAll`              | Compares all candidates | best error and distribution name        |
| `fitAllSummary`       | Compares all candidates | structured best fit and ranked results  |
| `isNormalDistributed` | Normality check         | boolean decision                        |

Parameter estimation uses the Method of Moments (MOM), based on sample mean, sample variance and observed extremes.

`FittingResult` exposes `distributionName`, `success`, `squaredError`, named fitted `parameters` and a short `message`. `FitSummary` exposes `success`, `bestFit` and the full `ranking` returned by `fitAllSummary()`. The legacy pointer-based `fitAll(...)` remains available and delegates to the structured best fit.

## Probability Distribution Helpers

`ProbabilityDistributionBase` and `ProbabilityDistribution` live inside `tools/analysis` and are part of the `genesys_tools_analysis` target. They provide static mathematical helpers used by `FitterDefaultImpl`, `HypothesisTesterDefaultImpl` and callers that need distribution PDFs or reference quantiles.

Public API:

| Class                         | Methods / purpose                                                                                                 |
| ----------------------------- | ----------------------------------------------------------------------------------------------------------------- |
| `ProbabilityDistributionBase` | Static PDF/PMF helpers: `normal`, `tStudent`, `chi2`, `fisherSnedecor`, `beta`, `gamma`, `erlang`, `exponential`, `logNormal`, `poisson`, `triangular`, `uniform`, `weibull`. |
| `ProbabilityDistribution`     | Static quantile helpers: `inverseNormal`, `inverseTStudent`, `inverseChi2`, `inverseFFisherSnedecor`.             |

The public headers expose only this mathematical API. They do not include kernel headers and do not expose solver, parser, simulator or statistics types. Numerical integration and quantile caching are private implementation details in `ProbabilityDistribution.cpp`.

## Hypothesis Tester

`HypothesisTester_if` defines classical parametric inference methods. `HypothesisTesterDefaultImpl` implements confidence intervals and hypothesis tests for mean, proportion and variance, for one and two populations.

Implemented confidence intervals:

| Method group                             | Statistical model                                                                 |
| ---------------------------------------- | --------------------------------------------------------------------------------- |
| `averageConfidenceInterval`              | Mean, using Student-t.                                                            |
| `proportionConfidenceInterval`           | Proportion, using normal approximation; supports finite-population correction.    |
| `varianceConfidenceInterval`             | Variance, using chi-square.                                                       |
| `averageDifferenceConfidenceInterval`    | Difference of means; pooled t or Welch depending on variance-ratio compatibility. |
| `proportionDifferenceConfidenceInterval` | Difference of proportions, using normal approximation.                            |
| `varianceRatioConfidenceInterval`        | Ratio of variances, using F distribution.                                         |

Implemented hypothesis tests:

| Method group     | Statistical model                           |
| ---------------- | ------------------------------------------- |
| `testAverage`    | Mean test for one or two populations.       |
| `testProportion` | Proportion test for one or two populations. |
| `testVariance`   | Variance test for one or two populations.   |

Each `TestResult` exposes:

| Field / method                                            | Meaning                                          |
| --------------------------------------------------------- | ------------------------------------------------ |
| `pValue()`                                                | Test p-value.                                    |
| `testStat()`                                              | Observed test statistic.                         |
| `rejectH0()` / `acceptH0()`                               | Decision at the requested confidence level.      |
| `acceptanceInferiorLimit()` / `acceptanceSuperiorLimit()` | Acceptance region limits for the test statistic. |

File-based overloads load numeric data through `DatasetLoader` and `SimulationResultsParser`, not through kernel statistics collectors.

All inference methods expect `confidenceLevel` in `(0, 1)`, for example `0.95`.

Implemented goodness-of-fit tests:

| Method                   | Statistical model                                                | Inputs                                                                                           |
| ------------------------ | ---------------------------------------------------------------- | ------------------------------------------------------------------------------------------------ |
| `chiSquareGoodnessOfFit` | Chi-square adherence test.                                       | Observed/expected frequencies, or raw sample + theoretical CDF + class policy.                  |
| `kolmogorovSmirnov`      | One-sample Kolmogorov-Smirnov test for continuous distributions. | Sample or sample file, theoretical CDF function and confidence level.                            |

Both methods return `TestResult`: `testStat()` contains chi-square or KS `D`, `pValue()` contains the adherence-test p-value, and `rejectH0()` indicates whether the fitted/theoretical distribution should be rejected at the requested confidence level.

KS p-value limitation: `kolmogorovSmirnov(...)` computes the classical one-sample KS p-value assuming the theoretical CDF is fully specified before observing the sample. When distribution parameters are estimated from the same sample being tested, that assumption is not valid and the returned p-value should be treated as an approximation/diagnostic value, not as an exact calibrated p-value. No Lilliefors correction, bootstrap calibration or Monte Carlo calibration is applied.

For chi-square tests from raw samples, the class policy is:

- If `classBoundaries` are provided, they must be finite, strictly increasing and cover all sample values.
- If only `classCount` is provided, equal-width classes are built from the sample min/max. `classCount = 0` uses Sturges' rule.
- Expected frequencies are computed from the provided CDF and normalized over the covered class range so the expected total matches the sample size.
- Adjacent classes are merged until the grouped expected frequency reaches `minExpectedFrequency` (default `5.0`). A final low-expected class is merged backward.
- Degrees of freedom are computed after grouping as `groupedClasses - 1 - estimatedParameters`.

## Error Measure

The fitter reports an error computed as a Cramer-von Mises-style squared difference between the theoretical CDF and the empirical CDF over the sorted sample:

```text
SE = sum_i (F(x_i) - p_i)^2
```

where `F(x_i)` is the theoretical CDF at the sorted sample value and `p_i = (i + 0.5) / n` is Hazen's empirical plotting position. The `+ 0.5` avoids exact 0 and 1 endpoints, which are numerically problematic for distributions with infinite support such as the Normal.

## Example

```cpp
#include "tools/analysis/DataAnalyserDefaultImpl.h"
#include "tools/analysis/ProbabilityDistribution.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

bool greaterThan50(double value) {
    return value > 50.0;
}

int main() {
    const std::string dataFile = "examples/data/sample_data.csv";

    DataAnalyserDefaultImpl analyser;
    if (!analyser.loadDataSet(dataFile)) {
        return 1;
    }
    auto summary = analyser.summary();
    auto histogram = analyser.histogram(6);
    auto boxplot = analyser.boxplot();
    const auto& data = analyser.data();

    // In-memory data is also supported:
    // analyser.loadDataSet(std::vector<double>{27.6, 33.4, 50.0, 66.6, 72.4});

    double normalError = 0.0;
    double mean = 0.0;
    double stddev = 0.0;
    analyser.fitter()->fitNormal(&normalError, &mean, &stddev);

    auto fitSummary = analyser.fitter()->fitAllSummary();
    auto normalCritical = ProbabilityDistribution::inverseNormal(0.975, 0.0, 1.0);
    auto fittedPdfAtMean = ProbabilityDistribution::normal(mean, mean, stddev);

    auto meanCi = analyser.tester()->averageConfidenceInterval(
        summary.mean,
        summary.stddev,
        static_cast<unsigned int>(summary.count),
        0.95
    );
    auto meanTest = analyser.tester()->testAverage(
        50.0,
        summary.stddev,
        static_cast<unsigned int>(summary.count),
        summary.mean,
        0.95,
        HypothesisTester_if::DIFFERENT
    );

    const double sampleProportion = static_cast<double>(
        std::count_if(data.begin(), data.end(), greaterThan50)
    ) / static_cast<double>(summary.count);
    auto proportionCi = analyser.tester()->proportionConfidenceInterval(
        sampleProportion,
        static_cast<unsigned int>(summary.count),
        0.95
    );

	auto normalCdf = [mean, stddev](double value) {
	    return 0.5 * std::erfc(-(value - mean) / (stddev * std::sqrt(2.0)));
	};
	auto chiSquare = analyser.tester()->chiSquareGoodnessOfFit(
	    data,
	    normalCdf,
	    2,
	    0.95,
	    4,
	    1.0
	);
	auto ks = analyser.tester()->kolmogorovSmirnov(data, normalCdf, 0.95);

    std::cout << "Normal fit: mean=" << mean
              << " stddev=" << stddev
              << " error=" << normalError << "\n";
    std::cout << "Sample count: " << summary.count
              << " sample mean=" << summary.mean << "\n";
    std::cout << "Histogram bins: " << histogram.bins.size()
              << " median=" << boxplot.median << "\n";
    std::cout << "Best fit: " << fitSummary.bestFit.distributionName
              << " error=" << fitSummary.bestFit.squaredError
              << " candidates=" << fitSummary.ranking.size() << "\n";
    std::cout << "Normal q(0.975): " << normalCritical
              << " fitted PDF at mean=" << fittedPdfAtMean << "\n";
    std::cout << "Mean CI: [" << meanCi.inferiorLimit()
              << ", " << meanCi.superiorLimit() << "]\n";
    std::cout << "Mean test p-value: " << meanTest.pValue()
              << " rejectH0=" << meanTest.rejectH0() << "\n";
	std::cout << "P(value > 50) CI: [" << proportionCi.inferiorLimit()
	          << ", " << proportionCi.superiorLimit() << "]\n";
	std::cout << "Chi-square p-value: " << chiSquare.pValue()
	          << " rejectH0=" << chiSquare.rejectH0() << "\n";
	std::cout << "KS p-value: " << ks.pValue()
	          << " rejectH0=" << ks.rejectH0() << "\n";
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
- **Chi-square goodness-of-fit:** the chi-square statistic depends directly on the class intervals, grouping policy and number of fitted parameters removed from the degrees of freedom. Arena may merge or choose histogram intervals internally. The example uses equal-width classes plus the documented minimum-expected-frequency grouping policy, so its chi-square statistic is not expected to match Arena unless the exact same class bounds, expected frequencies, grouping policy and degrees of freedom are used.
- **Kolmogorov-Smirnov:** the KS statistic depends on the exact theoretical CDF used for comparison, including parameter-estimation conventions such as MLE versus unbiased sample estimates. Small differences in fitted parameters change the CDF and therefore the KS `D` statistic. Tools may also differ in how they approximate or report p-values; for example, some report threshold ranges while this module prints its numeric approximation.
