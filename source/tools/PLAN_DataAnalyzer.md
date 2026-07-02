# DataAnalyzer — Architecture & Implementation Plan

## 1. Scope reminder

Theme 11.3.1: unified data analysis facade for GenESyS, with emphasis on
**additional diagnostics** (chi-square, KS) and **curve fitting**.
No graphics. Numerical output only.

Guiding checklist from proposta section 1:
- receive data by file or memory structures
- calculate descriptive measures and dispersion
- provide numerical structures for histograms, boxplots and temporal analyses
- fit probability distributions to observed data
- execute hypothesis tests of one AND two populations
- provide test statistics, confidence intervals, p-values
- allow explicit configuration of confidence AND significance levels
- perform basic temporal analysis: moving averages, correlogram/autocorrelation
- serve as basis for analysis of simulation input AND output data

---

## 2. Existing infrastructure inventory

### 2.1 Data ingestion

| Class | Location | Format | Notes |
|---|---|---|---|
| `SimulationResultsDataset` + `SimulationResultsDatasetParser` | `source/tools/` | **Text** (RawNumeric, RecordLegacy, RecordEnriched, GuiTabular) | Produces `std::vector<double>` via `.values()` |
| `CollectorDatafileDefaultImpl1` | `source/kernel/statistics/` | **Binary** (raw `double` stream) | Used by `StatisticsDatafileDefaultImpl1` |

**Critical mismatch**: `FitterDefaultImpl` and `StatisticsDatafileDefaultImpl1` both read binary files.
`SimulationResultsDataset` reads text files. The new tool must bridge this gap
by working primarily from an in-memory `std::vector<double>`.

### 2.2 Descriptive statistics

`StatisticsDatafile_if` / `StatisticsDatafileDefaultImpl1` (kernel/statistics/):
- min, max, average, variance, stddeviation, variationCoef
- mode, mediane, quartil(1-3), decil(1-9), centil(1-99)
- histogramClassLowerLimit, histogramClassFrequency

These are **file-bound** (binary). Proposta section 6 recommends reusing
`StatisticsDataFile_if` internally. However, since the new tool's primary data
path is an in-memory `std::vector<double>` (loaded from text files via
`SimulationResultsDataset`, or provided directly), routing through
`StatisticsDatafileDefaultImpl1` would require writing a temporary binary file.

**Decision**: implement descriptive statistics inline directly from `_data`.
The logic (sorting + linear interpolation for quantiles) is the same as what
`StatisticsDatafileDefaultImpl1` does internally — we are not re-inventing new
math, just not going through the file-backed layer. This is documented as an
explicit architectural trade-off against the proposta's recommendation.

### 2.3 Curve fitting

`FitterDefaultImpl` (header-only, `source/tools/`):
- Uniform, Triangular, Normal, Exponential, Erlang, Beta (scaled), Weibull
- `fitAll()` — selects best by SSE vs empirical CDF
- Currently reads only binary files via `setDataFilename`
- **Must be extended** with `setDataValues(std::vector<double>)` so in-memory
  data can flow in without writing a temp file

`Fitter_if` must receive a matching `setDataValues` declaration.

### 2.4 Hypothesis testing

`HypothesisTesterDefaultImpl1` (source/tools/):
- Confidence intervals: average, proportion, variance (1-pop and 2-pop)
- Tests: testAverage, testProportion, testVariance (1-pop and 2-pop)
- Both parametric (from numbers) and file-based overloads
- Uses `ProbabilityDistribution::{inverseNormal, inverseTStudent, inverseChi2, inverseFFisherSnedecor}`
- Uses `SolverDefaultImpl1` for numerical integration
- Already has `chi2CdfByIntegration` in anonymous namespace — will be needed by GoF tests

### 2.5 Probabilistic math

`ProbabilityDistributionBase` (source/tools/):
- PDFs: beta, chi2, erlang, exponential, fisherSnedecor, gamma, logNormal,
  normal, poisson, triangular, tStudent, uniform, weibull

`ProbabilityDistribution` (source/tools/):
- Inverse/quantile: inverseChi2, inverseFFisherSnedecor, inverseNormal, inverseTStudent

`SolverDefaultImpl1` (source/tools/):
- Simpson 1/3 quadrature — used by chi2 CDF integration and KS p-value support

### 2.6 Traits system

`TraitsTools<Fitter_if>::Implementation` → `FitterDefaultImpl`
`TraitsTools<HypothesisTester_if>::Implementation` → `HypothesisTesterDefaultImpl1`
`TraitsKernel<StatisticsDatafile_if>::Implementation` → `StatisticsDatafileDefaultImpl1`

The new `DataAnalyzer_if` will get its own `TraitsTools<DataAnalyzer_if>` specialisation.

---

## 3. Why we don't extend `DataAnalyser_if`

The existing `DataAnalyser_if` (British spelling, `source/tools/DataAnalyser_if.h`) has:
```cpp
virtual ExperimentManager_if* experimenter() = 0;
```
This pulls in `kernel/simulator/ExperimentManager_if.h`, a simulator-level concept
with no role in pure data analysis. Extending it would contaminate the tool with
simulator headers.

**Decision**: create a new `DataAnalyzer_if` (American spelling) as a clean,
independent interface. Leave `DataAnalyser_if` untouched for backward compatibility.

---

## 4. Architecture of the new tool

```
DataAnalyzer_if                    (new interface — source/tools/DataAnalyzer_if.h)
     |
     └── DataAnalyzerDefaultImpl1  (new implementation — source/tools/)
              |
              ├── std::vector<double> _data      (primary sample, in memory)
              ├── std::vector<double> _sortedData (sorted cache, rebuilt on data change)
              ├── std::vector<double> _data2     (optional second sample)
              ├── FitterDefaultImpl _fitter      (owns; uses setDataValues)
              ├── HypothesisTesterDefaultImpl1 _tester  (owns)
              └── double _confidenceLevel        (default 0.95)
```

Data flow:

```
Text file ──► SimulationResultsDatasetParser::loadFromTextFile()
                           │
                           ▼
              SimulationResultsDataset.values()
                           │
                           ▼
              DataAnalyzerDefaultImpl1._data  (std::vector<double>)
                           │
              ┌────────────┼───────────────────────────┐
              ▼            ▼                           ▼
         Fitter         Inline stats             Diagnostics
    (setDataValues)   (sort + scan)          (chi2, KS, MA, ACF)
```

---

## 5. Complete interface: `DataAnalyzer_if`

### 5.1 Output structs (defined in `DataAnalyzer_if.h`)

```cpp
struct SummaryStatistics {
    unsigned int n;
    double min, max, range;
    double mean, median, mode;
    double q1, q3;
    double variance, stddev, cv;
    double skewness, kurtosis;   // excess kurtosis (Fisher)
};

struct HistogramData {
    unsigned int numClasses;
    std::vector<double> lowerLimits;       // size = numClasses
    std::vector<unsigned int> frequencies; // size = numClasses
    std::vector<double> relativeFrequencies;
};

struct BoxplotData {
    double min, q1, median, q3, max;
    double iqr;
    std::vector<double> outliers;  // values outside [q1 - 1.5*iqr, q3 + 1.5*iqr]
};

struct FitResult {
    std::string distributionName;
    double sse;          // SSE vs empirical CDF
    double param1, param2, param3, param4, param5;  // distribution-specific (see §5.2)
    bool valid;
};

struct GoFResult {
    std::string distributionName;
    double testStatistic;
    double pValue;
    double criticalValue;
    double significanceLevel;
    bool rejectH0;
    std::string conclusion;
};
```

### 5.2 FitResult parameter semantics by distribution

| Distribution | param1 | param2 | param3 | param4 |
|---|---|---|---|---|
| uniform | min | max | — | — |
| triangular | min | mode | max | — |
| normal | mean | stddev | — | — |
| exponential | mean | — | — | — |
| erlang | mean | m (int shape) | — | — |
| beta | alpha | beta | infLimit | supLimit |
| weibull | alpha (shape) | scale | — | — |

### 5.3 Interface methods

```cpp
class DataAnalyzer_if {
public:
    // --- Data loading ---
    virtual void setDataFilename(std::string filename) = 0;
    virtual void setDataValues(const std::vector<double>& values) = 0;
    virtual void clearData() = 0;
    virtual bool loadSecondSample(const std::vector<double>& values) = 0;
    virtual bool loadSecondSampleFromFile(std::string filename) = 0;

    // --- Configuration ---
    // confidenceLevel and significanceLevel are linked: alpha = 1 - cl.
    // Both are exposed explicitly as the proposta requires each to be a named,
    // configurable entry point. Setting one updates the other.
    virtual void setConfidenceLevel(double confidenceLevel) = 0;
    virtual double getConfidenceLevel() = 0;
    virtual void setSignificanceLevel(double significanceLevel) = 0;  // sets _cl = 1 - alpha
    virtual double getSignificanceLevel() = 0;                        // returns 1 - _cl

    // --- Descriptive statistics ---
    virtual SummaryStatistics summaryStatistics() = 0;
    virtual double quartile(unsigned short num) = 0;   // 1-3
    virtual double decile(unsigned short num) = 0;     // 1-9
    virtual double centile(unsigned short num) = 0;    // 1-99

    // --- Structures for future visual tools ---
    virtual HistogramData histogramStructure(unsigned int numClasses) = 0;
    virtual BoxplotData boxplotStatistics() = 0;

    // --- Curve fitting ---
    virtual FitResult fitDistribution(const std::string& name) = 0;
    virtual FitResult fitAll() = 0;

    // --- Goodness-of-fit diagnostics ---
    virtual GoFResult chiSquareGoodnessOfFit(const std::string& distributionName,
                                              double significanceLevel) = 0;
    virtual GoFResult kolmogorovSmirnov(const std::string& distributionName,
                                         double significanceLevel) = 0;

    // --- Temporal / time-series analysis ---
    virtual std::vector<double> movingAverage(unsigned int window) = 0;
    virtual std::vector<double> autocorrelation(unsigned int maxLag) = 0;
    virtual std::vector<double> correlogram(unsigned int maxLag) = 0;  // same as autocorrelation

    // --- Inference bridges (delegate to HypothesisTesterDefaultImpl1) ---
    // One-population confidence intervals
    virtual HypothesisTester_if::ConfidenceInterval averageConfidenceInterval() = 0;
    virtual HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(
        checkProportionFunction checker) = 0;
    virtual HypothesisTester_if::ConfidenceInterval varianceConfidenceInterval() = 0;
    // One-population hypothesis tests
    virtual HypothesisTester_if::TestResult testAverage(
        double hypothesizedMean, HypothesisTester_if::H1Comparition comp) = 0;
    virtual HypothesisTester_if::TestResult testProportion(
        double hypothesizedProp, checkProportionFunction checker,
        HypothesisTester_if::H1Comparition comp) = 0;
    virtual HypothesisTester_if::TestResult testVariance(
        double hypothesizedVariance, HypothesisTester_if::H1Comparition comp) = 0;
    // Two-population hypothesis tests
    virtual HypothesisTester_if::TestResult testAverageTwoSamples(
        HypothesisTester_if::H1Comparition comp) = 0;
    virtual HypothesisTester_if::TestResult testProportionTwoSamples(
        checkProportionFunction checker, HypothesisTester_if::H1Comparition comp) = 0;
    virtual HypothesisTester_if::TestResult testVarianceTwoSamples(
        HypothesisTester_if::H1Comparition comp) = 0;
};
```

---

## 6. Implementation: `DataAnalyzerDefaultImpl1`

### 6.1 Private members

```cpp
std::vector<double> _data;
std::vector<double> _sortedData;    // rebuilt via _rebuildFromData()
std::vector<double> _data2;
double _confidenceLevel = 0.95;     // alpha = 1 - _confidenceLevel
FitterDefaultImpl _fitter;
HypothesisTesterDefaultImpl1 _tester;
```

### 6.2 `setDataFilename` flow

```
SimulationResultsDatasetParser::loadFromTextFile(filename, &dataset)
_data = dataset.values()
_rebuildFromData()        // sort + fitter.setDataValues
```

### 6.3 Descriptive statistics (inline implementation)

All computed from `_data` / `_sortedData` without file I/O:

**Quantile helper** (linear interpolation, R method R7):
```
h = (n - 1) * p
i = floor(h),  f = h - i
result = sorted[i] * (1 - f) + sorted[i+1] * f
```
Used for: quartile, decile, centile, median, q1, q3 in summaryStatistics.

**Mode**: histogram-based — Sturges bins, return midpoint of most-frequent bin.

**Skewness**: `sum((xi - mean)^3 / stddev^3) / n`

**Excess kurtosis**: `sum((xi - mean)^4 / stddev^4) / n - 3`

### 6.4 Chi-square goodness-of-fit

1. Fit the named distribution → get CDF function
2. Create k = ceil(1 + log2(n)) bins spanning [min - ε, max + ε]
3. For each bin [a,b]: `Ei = n * (CDF(b) - CDF(a))`
4. Merge bins left-to-right until Ei ≥ 5 (required condition)
5. Merge the last bin backward if still Ei < 5
6. `χ² = sum((Oi - Ei)² / Ei)`
7. `dof = k_merged - 1 - p` (p = free params per distribution, see §6.5)
8. p-value = `1 - chi2CDF(stat, dof)` — integrate `ProbabilityDistributionBase::chi2` via `SolverDefaultImpl1`
9. critical value = `ProbabilityDistribution::inverseChi2(1 - alpha, dof)`

### 6.5 Number of estimated parameters (for chi-square dof)

| Distribution | p |
|---|---|
| uniform | 2 |
| triangular | 3 |
| normal | 2 |
| exponential | 1 |
| erlang | 2 |
| beta | 4 |
| weibull | 2 |

### 6.6 CDF helper

A private `_cdfForDistribution(name, fit)` returning `std::function<double(double)>`.
Uses the same formulas as `FitterDefaultImpl`'s lambdas — implemented inline in .cpp:
- uniform, triangular, normal, exponential, erlang, weibull: closed-form
- beta: integrate `ProbabilityDistributionBase::beta` via `SolverDefaultImpl1`

### 6.7 Kolmogorov-Smirnov

1. Sort data, compute `Dn = max over i of max(|F(x_i) - (i-1)/n|, |F(x_i) - i/n|)`
2. `T = Dn * sqrt(n)`
3. p-value via Kolmogorov asymptotic series (valid for n ≥ 35; noted as approximate for smaller n):
   `p = 2 * sum_{k=1}^{100} (-1)^{k+1} * exp(-2 k² T²)` — converges in < 15 terms
4. Critical value: bisection on the same series to find `t` such that `p(t) = alpha`, then `D_crit = t / sqrt(n)`

### 6.8 Moving average

`MA[i] = mean(_data[i .. i+window-1])` for `i in [0, n-window]`.
Returns vector of length `n - window + 1`. Returns empty if `window > n`.

### 6.9 Autocorrelation / correlogram

```
xbar = mean
c0   = sum((xi - xbar)²)                            // denominator (unnormalized)
r(k) = sum_{i=0}^{n-k-1} (xi - xbar)(x_{i+k} - xbar) / c0
```
Returns `r[0..maxLag]`, always `r[0] = 1.0`.
`correlogram()` delegates to `autocorrelation()` — they are the same structure.

### 6.10 Inference bridges

| Method | Delegates to |
|---|---|
| `averageConfidenceInterval()` | `_tester.averageConfidenceInterval(mean, stddev, n, _cl)` |
| `proportionConfidenceInterval(checker)` | count via `checker`, then `_tester.proportionConfidenceInterval(prop, n, _cl)` |
| `varianceConfidenceInterval()` | `_tester.varianceConfidenceInterval(variance, n, _cl)` |
| `testAverage(mu0, comp)` | `_tester.testAverage(mu0, stddev, n, mean, _cl, comp)` |
| `testProportion(p0, checker, comp)` | count via `checker`, then `_tester.testProportion(p0, n, sampleP, _cl, comp)` |
| `testVariance(var0, comp)` | `_tester.testVariance(variance, n, var0, _cl, comp)` |
| `testAverageTwoSamples(comp)` | `_tester.testAverage(mean1, sd1, n1, mean2, sd2, n2, _cl, comp)` |
| `testProportionTwoSamples(checker, comp)` | count in `_data`/`_data2` via `checker`, then `_tester.testProportion(p1, n1, p2, n2, _cl, comp)` |
| `testVarianceTwoSamples(comp)` | `_tester.testVariance(var1, n1, var2, n2, _cl, comp)` |

---

## 7. Files to create / modify

| File | Action | What changes |
|---|---|---|
| `source/tools/Fitter_if.h` | Modify | Add `#include <vector>` and `setDataValues` pure virtual |
| `source/tools/FitterDefaultImpl.h` | Modify | Extract `_computeStatsFromLoadedData()`; add `setDataValues` that fills data directly and calls it |
| `source/tools/FitterDummyImpl.h` | Modify | Add `setDataValues` stub declaration |
| `source/tools/FitterDummyImpl.cpp` | Modify | Add `setDataValues` stub body (empty) |
| `source/tools/DataAnalyzer_if.h` | **CREATE** | Interface + all output structs |
| `source/tools/DataAnalyzerDefaultImpl1.h` | **CREATE** | Class declaration |
| `source/tools/DataAnalyzerDefaultImpl1.cpp` | **CREATE** | Full implementation (~500 lines) |
| `source/tools/TraitsTools.h` | Modify | Add `TraitsTools<DataAnalyzer_if>` specialisation |
| `source/tools/CMakeLists.txt` | Modify | Add `DataAnalyzerDefaultImpl1.cpp` to library |
| `source/tools/main.cpp` | Modify | Add `--demo` command exercising all features |

---

## 8. Interaction and testing strategy

### 8.1 Primary deliverable: `main.cpp --demo`

The demo is what gets shown to the professor. Build and run it with:

```
cmake --preset gui-app -DGENESYS_BUILD_TOOLS=ON
cmake --build build/gui-app --target genesys_tools_application
./build/gui-app/source/tools/genesys_tools_application --demo
```

`GENESYS_BUILD_TOOLS=ON` is needed to build the standalone executable — this is
expected project structure. The `genesys_tools` library itself is always compiled
as part of `gui-app` regardless.

The demo runs two passes to satisfy section 12 of the proposta, which requires
demonstrating **both** file and memory loading paths.

**Pass 1 — in-memory path:**
Generates 200 samples from Exponential(mean=2) via `std::mt19937` +
`std::exponential_distribution`, calls `setDataValues(data)`, then exercises
every feature in sequence: summaryStatistics, quartile/decile/centile,
histogramStructure(8), boxplotStatistics, fitAll, fitDistribution("exponential"),
chiSquareGoodnessOfFit("exponential", 0.05), kolmogorovSmirnov("exponential", 0.05),
movingAverage(5), autocorrelation(10), averageConfidenceInterval,
proportionConfidenceInterval, varianceConfidenceInterval,
testAverage(2.0, DIFFERENT), testVariance(4.0, DIFFERENT).

**Pass 2 — file path:**
Writes the same 200 values to a temporary text file in RawNumeric format
(one value per line), then creates a fresh `DataAnalyzerDefaultImpl1` and calls
`setDataFilename(tmpPath)`. Repeats summaryStatistics and fitAll to confirm
that results match pass 1 exactly, demonstrating that the file ingestion path
(via `SimulationResultsDatasetParser`) produces the same data as the in-memory path.

### 8.2 Integration check: `gui-app` must still compile cleanly

After implementation, run `cmake --preset gui-app && cmake --build build/gui-app`
to confirm the new library code doesn't break anything. This is the handoff point
for future GUI developers who will eventually wire `DataAnalyzerDefaultImpl1`
into the GUI layer.

### 8.3 Text file path

Any `.dat` file readable by `SimulationResultsDatasetParser` can be used:
```cpp
DataAnalyzerDefaultImpl1 analyzer;
analyzer.setDataFilename("path/to/output.dat");
auto fit = analyzer.fitAll();
auto gof = analyzer.chiSquareGoodnessOfFit(fit.distributionName, 0.05);
```

### 8.4 In-memory path

```cpp
std::vector<double> data = { 1.2, 0.8, 2.3, 1.1, ... };
DataAnalyzerDefaultImpl1 analyzer;
analyzer.setDataValues(data);
// all methods work identically
```

---

## 9. Coding milestones

Each milestone ends in a clean build. Never move to the next until the current one compiles.

- **M1 — Fitter extension**
  - Add `#include <vector>` and `setDataValues` pure virtual to `Fitter_if.h`
  - Extract `_computeStatsFromLoadedData()` in `FitterDefaultImpl.h`; implement `setDataValues` using it
  - Add empty `setDataValues` stub to `FitterDummyImpl.h` and `FitterDummyImpl.cpp`
  - Build `gui-app`. Zero new errors.

- **M2 — Interface + structs**
  - Create `DataAnalyzer_if.h`: all five output structs + full interface with every pure virtual
  - Build `gui-app`. Zero new errors.

- **M3 — Skeleton: data loading + descriptive stats**
  - Create `DataAnalyzerDefaultImpl1.h` and `DataAnalyzerDefaultImpl1.cpp`
  - Add `.cpp` to `source/tools/CMakeLists.txt`
  - Implement: constructor, `setDataFilename`, `setDataValues`, `clearData`, `loadSecondSample`, `loadSecondSampleFromFile`, `setConfidenceLevel`, `getConfidenceLevel`, `setSignificanceLevel`, `getSignificanceLevel`
  - Implement: `summaryStatistics`, `quartile`, `decile`, `centile`, `histogramStructure`, `boxplotStatistics`
  - All other interface methods: stub bodies that return empty/default values
  - Wire `TraitsTools<DataAnalyzer_if>` in `TraitsTools.h`
  - Build `gui-app`. Zero new errors.

- **M4 — Curve fitting**
  - Implement `fitDistribution` and `fitAll` (delegate to `_fitter`)
  - Implement `_cdfForDistribution` private helper (all seven distributions)
  - Build + run `--demo` pass 1 through fitting section. Verify best fit is "exponential".

- **M5 — Chi-square goodness-of-fit**
  - Implement `chiSquareGoodnessOfFit`: Sturges bins, expected frequencies, bin merging, χ² stat, p-value via `SolverDefaultImpl1` + `ProbabilityDistribution::inverseChi2`
  - Build + run `--demo`. Verify chi-square result prints with sensible stat and p-value.

- **M6 — Kolmogorov-Smirnov**
  - Implement `kolmogorovSmirnov`: Dn statistic, Kolmogorov asymptotic series for p-value, bisection for critical value
  - Build + run `--demo`. Verify KS result prints; for n=200 exponential data should fail to reject.

- **M7 — Time-series**
  - Implement `movingAverage`, `autocorrelation`, `correlogram`
  - Build + run `--demo`. Verify MA and ACF vectors print with r(0)=1.0.

- **M8 — Inference bridges**
  - Implement all confidence interval methods and all test methods (one- and two-population), delegating to `_tester`
  - Build + run `--demo`. Verify 95% CI for mean contains 2.0.

- **M9 — Complete demo + integration check**
  - Flesh out `main.cpp --demo`: both passes (in-memory and file), all features printed
  - Run `cmake --preset gui-app && cmake --build build/gui-app`. Must compile clean with no errors or new warnings.

---

## 10. Key design decisions

| Decision | Rationale |
|---|---|
| In-memory `std::vector<double>` as primary store | Removes binary-file dependency; identical for file-loaded and memory-provided data |
| Inline descriptive stats instead of routing through `StatisticsDataFile_if` | Avoids writing temp binary files; same math; architectural clarity |
| New `DataAnalyzer_if` instead of extending `DataAnalyser_if` | Existing interface has `ExperimentManager_if*` — wrong level of abstraction |
| `FitterDefaultImpl` extended with `setDataValues` | Existing fitting math is correct; only the data-source path needs extending |
| Chi-square bins merged if Ei < 5 | Standard statistical requirement for valid chi-square approximation |
| KS p-value via Kolmogorov asymptotic series | No lookup table needed; converges in < 15 iterations |
| `correlogram` delegates to `autocorrelation` | They are the same mathematical structure |
| `testProportion` uses `checkProportionFunction` | Reuses existing contract from `HypothesisTester_if.h` without inventing a new pattern |
| Parameter count for chi-square DoF | Uniform=2, Exponential=1, Normal=2, Triangular=3, Erlang=2, Beta=4, Weibull=2 |
