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

**Note on StatisticsDataFile_if:** The proposta (section 6) recommends routing descriptive statistics through `StatisticsDatafile_if`. This was studied and deliberately not used: that interface extends `CollectorDatafile_if` and is structurally bound to a binary file collector — there is no constructor or setter that accepts a `vector<double>`. Routing through it would require writing data to a binary file and reading it back, which defeats the purpose. The statistics are instead computed inline from the in-memory sorted vector, producing the same results. This decision is documented here rather than hidden.

**What changed:**

- `DataAnalyzerDefaultImpl1.h` (new) — Class declaration. Private state: `_data` + `_sortedData` (primary sample), same pair for second sample (`_data2`/`_sortedData2`), cached stats (`_count`, `_sampleMean`, `_sampleVariance`, `_sampleStddev`, same for second sample with `_count2`/`_sampleMean2`/`_sampleVariance2`/`_sampleStddev2`), `_confidenceLevel = 0.95`, a `FitterDefaultImpl _fitter` member, and a `HypothesisTesterDefaultImpl1 _tester` member.

- `DataAnalyzerDefaultImpl1.cpp` (new) — Data loading:
  - `setDataFilename` uses `SimulationResultsDatasetParser::loadFromTextFile` (text formats: RawNumeric, RecordLegacy, RecordEnriched, GuiTabular) then calls `setDataValues`.
  - `setDataValues` stores the vector, calls `_rebuildCache()` (sort + compute stats), and syncs the embedded fitter via `_fitter.setDataValues(_data)`.
  - `loadSecondSampleFromFile` parses a text file and calls `loadSecondSample`, which stores into `_data2` and calls `_rebuildCache2()`.
  - `_rebuildCache()` / `_rebuildCache2()` sort their respective vectors and compute `_sampleMean`, `_sampleVariance`, `_sampleStddev`, `_count` from the raw data.

  Descriptive statistics:
  - `_quantile(sorted, p)` — linear interpolation (R method R7): `h = (n-1)*p; i = floor(h); result = sorted[i]*(1-f) + sorted[i+1]*f`. Returns NaN for empty input.
  - `_sampleMode()` — scans sorted data for runs of equal values; returns NaN if all values are distinct (appropriate for continuous data).
  - `summaryStatistics()` — assembles all fields from cached stats and `_quantile`. Skewness = (1/n) Σ((x-μ)/σ)³, excess kurtosis = (1/n) Σ((x-μ)/σ)⁴ - 3.
  - `quartile/decile/centile` — thin wrappers over `_quantile`.

- Configuration methods:
  - `setConfidenceLevel(cl)` / `getConfidenceLevel()` — stores `_confidenceLevel` directly.
  - `setSignificanceLevel(alpha)` / `getSignificanceLevel()` — linked to confidence level: `setSignificanceLevel` sets `_confidenceLevel = 1 - alpha`; `getSignificanceLevel` returns `1 - _confidenceLevel`. The two are always consistent; there is no separate alpha field.

- `clearData()` clears both samples — it resets `_data`, `_sortedData`, and all cached stats for the primary sample, and also clears `_data2`, `_sortedData2`, and all second-sample stats. The second sample is therefore implicitly invalidated whenever the primary sample is cleared.

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

- `_cdfAt(x, fit)` — Computes the theoretical CDF at `x` for any of the seven supported distributions, using the parameters stored in `FitResult`. The beta CDF uses `SolverDefaultImpl1` integration over a file-scope static `_betaPdfIntegrand(t, alpha, beta)` helper (required because `SolverDefaultImpl1::integrate` takes function pointers, not lambdas).

- `_numParams(distName)` — Returns the number of parameters estimated from data for each distribution (uniform=0, exponential=1, triangular=1, normal/erlang/beta/weibull=2). Used to compute degrees of freedom after bin merging.

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

- `autocorrelation(maxLag)` — Biased estimator: r[k] = c[k]/c[0] where c[k] = (1/n) Σ_{i=0}^{n-k-1} (x_i - μ)(x_{i+k} - μ). Output is a vector of size maxLag + 1 (lags 0 through maxLag). r[0] = 1.0 by construction. If `maxLag >= n`, it is silently capped at `n - 1` to avoid reading out of bounds.

- `correlogram(maxLag)` — Returns the same computation as `autocorrelation`. The distinction is conceptual: `autocorrelation` returns raw ACF values, `correlogram` returns the same values intended for plotting as a bar chart by lag. Sharing the implementation avoids duplication.

---

## M8 — Parametric inference bridges

**Problem to solve:** `HypothesisTesterDefaultImpl1` is fully capable but requires all parameters to be passed explicitly (mean, stddev, n, confidence level). The new tool has those cached, so the bridge just forwards them.

**What changed (within `DataAnalyzerDefaultImpl1.cpp`):**

- One-population confidence intervals: `averageConfidenceInterval`, `proportionConfidenceInterval`, `varianceConfidenceInterval` — each delegates to `_tester` with `_sampleMean`/`_sampleStddev`/`_sampleVariance`/`_count`/`_confidenceLevel`.

- One-population hypothesis tests: `testAverage`, `testProportion`, `testVariance` — same delegation pattern. For proportion tests, the checker function is applied to `_data` inline to compute the sample proportion before delegating.

- Two-population tests: `testAverageTwoSamples`, `testProportionTwoSamples`, `testVarianceTwoSamples` — delegate to `_tester` using both samples' cached stats. Require `loadSecondSample` to have been called first.

**Bug found and fixed post-implementation:** The initial versions of `testProportionTwoSamples` and `testVarianceTwoSamples` accidentally called the one-sample overloads of `_tester.testProportion` and `_tester.testVariance`, treating the second sample's statistic as the null-hypothesis value. `HypothesisTesterDefaultImpl1` already had correct two-population overloads — `testProportion(prop1, n1, prop2, n2, cl, comp)` and `testVariance(var1, n1, var2, n2, cl, comp)`. The fix was passing `_count2` as the fourth argument to both calls, selecting the proper overloads. A stale comment left over from the wrong implementation (`// Delegate: use proportion difference test`) was also removed from `testProportionTwoSamples`.

---

---

## M9 — Demo and test harness

**Problem to solve:** The implementation needed to be exercisable without the GUI. The existing `main.cpp` only printed an interface overview.

**What changed:**

- `source/tools/main.cpp` — Added `--demo` command that instantiates `DataAnalyzerDefaultImpl1` directly and runs 26 checks across 7 sections. Each check prints `[PASS]` or `[FAIL]`. Run with: `./build/gui-app/source/tools/genesys_tools_application --demo` (requires `GENESYS_BUILD_TOOLS=ON` at configure time).

  Sections: (1) data loading from memory and text file, (2) descriptive statistics correctness and quantile consistency, (3) histogram frequency sum and boxplot IQR formula, (4) fitting validity and `fitAll` ordering, (5) chi-square and KS not rejecting a correct fit at α=0.05, (6) moving-average size and first value, ACF[0]=1 and positive lag-1 for trending data, correlogram equality, (7) CI coverage, single-sample t-test accept/reject, two-sample average test.

**Build fix during M9:** `HypothesisTester_if::ConfidenceInterval` accessor methods (`inferiorLimit`, `superiorLimit`, `halfWidth`) are not declared `const`. Local variables holding CI objects in `main.cpp` must be non-`const` `auto`, otherwise calling these methods fails with a qualifier error.

---

## M10 — Five targeted extensions

**What changed:**

- `DataAnalyzer_if.h` — `FitResult` gained two new fields: `rmse` (sqrt(SSE/n)) and `r2`
  (1 - SSE/SST, coefficient of determination vs empirical CDF using Hazen plotting positions
  p_i = (i+0.5)/n). New structs: `FitReport` (aggregates `FitResult` + three `GoFResult`),
  `TrendDiagnostic` (hasTrend, hasSeasonality, trendSlope, trendIntercept). New pure
  virtuals: `andersonDarling`, `analyzeFit`, `detectTrend`, `exceedanceProbability`,
  `exceedanceCurve`.

- `DataAnalyzerDefaultImpl1.h` — Added matching `override` declarations and two new static
  helpers: `_adPValue(A2)` and `_adQuantile(alpha)`.

- `DataAnalyzerDefaultImpl1.cpp`:
  - `_adPValue` — asymptotic approximation from Marsaglia & Marsaglia (2004); four piecewise
    branches cover the range of realistic A² values.
  - `_adQuantile` — bisection on `_adPValue` (monotone decreasing) to find the critical value.
  - `fitDistribution` — now computes `rmse` and `r2` after calling the fitter; SST is
    computed in O(n) from the Hazen order statistics.
  - `andersonDarling` — computes A² over sorted order statistics, applies the Stephens (1974)
    finite-sample correction (1 + 4/n - 25/n²), then delegates p-value and critical value to
    the helpers.
  - `analyzeFit` — thin facade: calls `fitDistribution` + `chiSquareGoodnessOfFit` +
    `kolmogorovSmirnov` + `andersonDarling` and packages results into `FitReport`.
  - `detectTrend` — computes ACF up to min(10, n-1) lags via the existing `autocorrelation`
    method; classifies lag-1 significance (trend) and any higher-lag significance (seasonality)
    against the Bartlett band ±1.96/√n; computes OLS slope and intercept in O(n).
  - `exceedanceProbability` — returns `max(0, 1 - _cdfAt(x, fit))`.
  - `exceedanceCurve` — evaluates exceedance at `points` equally-spaced x values in
    [xMin, xMax]; both methods reuse `_cdfAt` without duplication.

- `main.cpp` — Added `demoExtensions()` (section 8) with 13 new checks covering all five
  extensions. All 49 checks (26 original + 10 from M9 extensions + 13 new) pass.

**Key design decisions:**

- Anderson-Darling p-value uses the asymptotic Marsaglia formula (no lookup tables) with the
  Stephens finite-sample correction. Acceptable approximation for a university context;
  documented as such.
- `FitReport` is a pure data struct (no methods) consistent with the pattern established by
  the existing output structs.
- `detectTrend` deliberately reuses `autocorrelation()` rather than recomputing ACF inline —
  single-responsibility principle; no duplicated logic.
- `exceedanceCurve` returns only the probability values (not pairs), consistent with how
  `autocorrelation` and `movingAverage` return plain `vector<double>`. The caller controls x
  sampling.

---

## Build history

| Milestone | Files touched | Build result |
|-----------|---------------|--------------|
| M1 | Fitter_if.h, FitterDefaultImpl.h, FitterDummyImpl.h, FitterDummyImpl.cpp | Clean (pre-existing warnings in FitterDummyImpl only) |
| M2 | DataAnalyzer_if.h (new) | Clean |
| M3–M8 | DataAnalyzerDefaultImpl1.h (new), DataAnalyzerDefaultImpl1.cpp (new), CMakeLists.txt, TraitsTools.h | One fix: `long long int` vs `long int` mismatch in erlang CDF (`std::llround` returns `long long int`) |
| M9 | main.cpp | One fix: `const auto ci` → `auto ci` for ConfidenceInterval locals (non-const accessors) |
| M8 fix | DataAnalyzerDefaultImpl1.cpp | Two-sample proportion and variance tests corrected to use two-population overloads; `_count2` added as fourth argument |
| M10 | DataAnalyzer_if.h, DataAnalyzerDefaultImpl1.h, DataAnalyzerDefaultImpl1.cpp, main.cpp | Clean — all 49 demo checks pass |
