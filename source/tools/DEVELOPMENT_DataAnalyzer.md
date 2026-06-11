# DataAnalyzer Development Log

This document tracks the evolution of the unified data analysis tool in `source/tools/`. Each milestone section explains what was done, why, and what files changed.

---

## M1 — Extend Fitter interface to accept in-memory data

**Problem to solve:** `FitterDefaultImpl` only accepted data through a binary file (`setDataFilename`). The new analysis tool works with `std::vector<double>` loaded from text files via `SimulationResultsDatasetParser`. There was no way to give the fitter in-memory data.

**What changed:**

- `Fitter_if.h` — Added `#include <vector>` and declared `virtual void setDataValues(const std::vector<double>& values) = 0`.

- `FitterDefaultImpl.h` — Extracted `_computeStatsFromLoadedData()` from `_loadDataFromBinaryFile()`. This helper assumes `_data` and `_sortedData` are already populated and computes `_count`, `_sampleMin/Max/Mean/Variance/Stddev`, and `_hasNegativeData`. `_loadDataFromBinaryFile()` now calls it after reading the binary file. The new `setDataValues()` override validates the input vector (rejects non-finite values), populates `_data`/`_sortedData`, and calls `_computeStatsFromLoadedData()`. It then sets `_cacheLoaded = true` / `_cacheUsable = result`, so the existing `_ensureDataLoaded()` check in every `fit*` method will see the data as already loaded and skip the file I/O path entirely.

- `FitterDummyImpl.h` + `.cpp` — Added an empty `setDataValues` stub to satisfy the interface contract.

**Key design decision:** Reused the exact same cache flags (`_cacheLoaded`, `_cacheUsable`) and stats fields that the binary-file path already used. No structural changes to `FitterDefaultImpl` — just an additional entry point that bypasses I/O.

---

## M2 — Define the DataAnalyzer_if interface

**Problem to solve:** There was no unified interface for data analysis. The old `DataAnalyser_if` (British spelling) had a dependency on `ExperimentManager_if`, which belongs to the simulator layer — wrong level for a standalone tool.

**What changed:**

- `DataAnalyzer_if.h` (new file) — Defines five output structs:
  - `SummaryStatistics` — n, min, max, range, mean, median, mode, q1, q3, variance, stddev, cv, skewness, kurtosis.
  - `HistogramData` — number of classes, vector of lower limits, vector of frequencies, vector of relative frequencies.
  - `BoxplotData` — min, q1, median, q3, max, iqr, vector of outliers.
  - `FitResult` — distribution name, SSE, up to five parameters (semantics documented in the struct comment), valid flag.
  - `GoFResult` — distribution name, test statistic, p-value, critical value, significance level, reject/accept flag, conclusion string.

  The interface re-uses `HypothesisTester_if::ConfidenceInterval`, `TestResult`, and `H1Comparition` by including `HypothesisTester_if.h`. It also re-uses `checkProportionFunction` (a function pointer typedef for proportion tests).

**Key design decision:** American spelling (`DataAnalyzer`) to avoid confusion with the old facade. No inheritance from `DataAnalyser_if`. All output structs are plain data — no methods, no inheritance — so any future GUI layer can consume them without depending on this package.

---

## M3 — DataAnalyzerDefaultImpl1: data loading and descriptive statistics

**Problem to solve:** The interface needed a concrete implementation. The first working slice is data loading and descriptive statistics — the foundation everything else depends on.

**What changed:**

- `DataAnalyzerDefaultImpl1.h` (new) — Class declaration. Private state: `_data` + `_sortedData` (primary sample), same pair for second sample (`_data2`/`_sortedData2`), cached stats (`_count`, `_sampleMean`, `_sampleVariance`, `_sampleStddev`, same for second sample), `_confidenceLevel = 0.95`, a `FitterDefaultImpl _fitter` member, and a `HypothesisTesterDefaultImpl1 _tester` member.

- `DataAnalyzerDefaultImpl1.cpp` (new) — Data loading:
  - `setDataFilename` uses `SimulationResultsDatasetParser::loadFromTextFile` (text formats: RawNumeric, RecordLegacy, RecordEnriched, GuiTabular) then calls `setDataValues`.
  - `setDataValues` stores the vector, calls `_rebuildCache()` (sort + compute stats), and syncs the embedded fitter via `_fitter.setDataValues(_data)`.
  - `_rebuildCache()` sorts into `_sortedData` and computes `_sampleMean`, `_sampleVariance`, `_sampleStddev` from the raw data.

  Descriptive statistics:
  - `_quantile(sorted, p)` — linear interpolation (R method R7): `h = (n-1)*p; i = floor(h); result = sorted[i]*(1-f) + sorted[i+1]*f`. Returns NaN for empty input.
  - `_sampleMode()` — scans sorted data for runs of equal values; returns NaN if all values are distinct (appropriate for continuous data).
  - `summaryStatistics()` — assembles all fields from cached stats and `_quantile`. Skewness = (1/n) Σ((x-μ)/σ)³, excess kurtosis = (1/n) Σ((x-μ)/σ)⁴ - 3.
  - `quartile/decile/centile` — thin wrappers over `_quantile`.

- `source/tools/CMakeLists.txt` — Added `DataAnalyzerDefaultImpl1.cpp` to `genesys_tools` sources.
- `source/tools/TraitsTools.h` — Added `TraitsTools<DataAnalyzer_if>` specialization pointing to `DataAnalyzerDefaultImpl1`.

---

## M4 — Distribution fitting

**Problem to solve:** The interface has `fitDistribution(name)` and `fitAll()`, but `FitterDefaultImpl` uses a different output API (raw output pointers). Needed a bridge that packages the results into `FitResult`.

**What changed (within `DataAnalyzerDefaultImpl1.cpp`):**

- `fitDistribution(name)` — Dispatches to the correct `_fitter.fit*()` method based on the name string. Packages SSE and parameters into `FitResult`. Parameters are named consistently: uniform (p1=min, p2=max), triangular (p1=min, p2=mode, p3=max), normal (p1=mean, p2=stddev), exponential (p1=mean), erlang (p1=mean, p2=m), beta (p1=α, p2=β, p3=infLimit, p4=supLimit), weibull (p1=shape, p2=scale).

- `fitAll()` — Calls `_fitter.fitAll()` to get the best distribution name, then calls `fitDistribution(bestName)` to retrieve the parameters. The double-fit is intentional: `fitAll` only returns the name + SSE, not the parameters; re-fitting the winner is the only way to get them.

---

## M5 — Chi-square goodness-of-fit test

**Problem to solve:** Needed a formal test to determine whether a fitted distribution adequately describes the data.

**What changed (within `DataAnalyzerDefaultImpl1.cpp`):**

- `_chi2PdfIntegrand(t, halfDf, logNorm)` — Static helper: chi-square PDF at `t` for given degrees of freedom, parameterized for `SolverDefaultImpl1::integrate` (which takes function pointers with extra parameters). Returns 0 at t < 1e-15 to avoid log(0).

- `_chi2Cdf(x, df)` — Integrates `_chi2PdfIntegrand` from 0 (or 1e-8 for df=1 to avoid the singularity) to `x` using `SolverDefaultImpl1`.

- `_chi2Quantile(df, p)` — Bisection over `_chi2Cdf` to find the critical value.

- `chiSquareGoodnessOfFit(name, alpha)` workflow:
  1. Fit the distribution to get parameters.
  2. Compute Sturges bin count: k = max(5, 1 + ceil(log₂(n))).
  3. Assign observations to bins; compute expected frequencies using `_cdfAt(binHigh) - _cdfAt(binLow)`.
  4. Forward-merge bins where E < 5; merge the last bin back if it's still E < 5.
  5. Compute χ² = Σ(O-E)²/E.
  6. DoF = k_merged - 1 - p, where p = estimated parameters per distribution (uniform=0, exponential=1, normal/erlang/beta/weibull=2, triangular=1).
  7. p-value = 1 - chi2_cdf(χ², DoF); reject H0 if χ² > critical value.

- `_cdfAt(x, fit)` — Computes the theoretical CDF at `x` for any of the seven supported distributions, using the parameters stored in `FitResult`. The beta CDF uses `SolverDefaultImpl1` integration over the beta PDF (same approach as `FitterDefaultImpl`).

---

## M6 — Kolmogorov-Smirnov test

**Problem to solve:** Chi-square requires binning and is sensitive to bin choice. KS is a distribution-free test on the raw order statistics.

**What changed (within `DataAnalyzerDefaultImpl1.cpp`):**

- `_ksPValue(Dn, n)` — Kolmogorov asymptotic series: p-value ≈ 2 Σ_{k=1}^{200} (-1)^{k+1} exp(-2k²(√n·Dn)²). Series terminates early when terms fall below 1e-12. This approximation is known to underestimate the p-value for small n (< 35); acceptable for a university-project context.

- `_ksQuantile(n, alpha)` — Approximate critical value: D_α ≈ sqrt(-ln(α/2) / (2n)). This is the inverse of the asymptotic formula.

- `kolmogorovSmirnov(name, alpha)` workflow:
  1. Fit the distribution.
  2. For each sorted observation x_i, compute both F_n(x_i⁻) = i/n and F_n(x_i) = (i+1)/n (left and right empirical CDF boundaries), and take the max absolute difference with the theoretical CDF F(x_i).
  3. D_n = maximum over all i.
  4. p-value from `_ksPValue`; critical value from `_ksQuantile`; reject if D_n > critical value.

---

## M7 — Time-series analysis

**Problem to solve:** Simulation output data often has temporal structure. Needed moving averages and autocorrelation to detect trends and serial correlation.

**What changed (within `DataAnalyzerDefaultImpl1.cpp`):**

- `movingAverage(window)` — Trailing window: output[i] = mean(data[i .. i+window-1]). Uses an O(n) sliding sum (add new element, subtract oldest). Output has size n - window + 1.

- `autocorrelation(maxLag)` — Biased estimator: r[k] = c[k]/c[0] where c[k] = (1/n) Σ_{i=0}^{n-k-1} (x_i - μ)(x_{i+k} - μ). Output is a vector of size maxLag + 1 (lags 0 through maxLag). r[0] = 1.0 by construction.

- `correlogram(maxLag)` — Returns the same computation as `autocorrelation`. The distinction is conceptual: `autocorrelation` returns raw ACF values, `correlogram` returns the same values intended for plotting as a bar chart by lag. Sharing the implementation avoids duplication.

---

## M8 — Parametric inference bridges

**Problem to solve:** `HypothesisTesterDefaultImpl1` is fully capable but requires all parameters to be passed explicitly (mean, stddev, n, confidence level). The new tool has those cached, so the bridge just forwards them.

**What changed (within `DataAnalyzerDefaultImpl1.cpp`):**

- One-population confidence intervals: `averageConfidenceInterval`, `proportionConfidenceInterval`, `varianceConfidenceInterval` — each delegates to `_tester` with `_sampleMean`/`_sampleStddev`/`_sampleVariance`/`_count`/`_confidenceLevel`.

- One-population hypothesis tests: `testAverage`, `testProportion`, `testVariance` — same delegation pattern. For proportion tests, the checker function is applied to `_data` inline to compute the sample proportion.

- Two-population tests: `testAverageTwoSamples`, `testProportionTwoSamples`, `testVarianceTwoSamples` — delegate to `_tester` with both samples' cached stats. Require `loadSecondSample` to have been called first.

---

## Build history

| Milestone | Files touched | Build result |
|-----------|---------------|--------------|
| M1 | Fitter_if.h, FitterDefaultImpl.h, FitterDummyImpl.h, FitterDummyImpl.cpp | Clean (pre-existing warnings in FitterDummyImpl only) |
| M2 | DataAnalyzer_if.h (new) | Clean |
| M3–M8 | DataAnalyzerDefaultImpl1.h (new), DataAnalyzerDefaultImpl1.cpp (new), CMakeLists.txt, TraitsTools.h | Clean after one fix (long int vs long long int in erlang CDF) |
