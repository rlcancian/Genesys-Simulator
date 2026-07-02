#ifndef DATAANALYZER_IF_H
#define DATAANALYZER_IF_H

#include "HypothesisTester_if.h"

#include <string>
#include <vector>

/**
 * @brief Unified data analysis facade for the GenESyS tools package.
 *
 * Provides descriptive statistics, histogram/boxplot structures, distribution
 * fitting, goodness-of-fit tests, time-series analysis, and parametric
 * inference through a single coherent API. No graphical output is produced —
 * all methods return numerical structures suitable for future visualization.
 *
 * Data loading:
 * - setDataFilename / setDataValues load the primary sample.
 * - loadSecondSample / loadSecondSampleFromFile load an optional second sample
 *   required by two-population inference methods.
 *
 * Confidence / significance:
 * - confidenceLevel and significanceLevel are linked: alpha = 1 - cl.
 *   Setting one updates the other automatically.
 */
class DataAnalyzer_if {
public:
	virtual ~DataAnalyzer_if() = default;

	// ---- output structures ------------------------------------------------

	struct SummaryStatistics {
		unsigned int n;
		double min, max, range;
		double mean, median, mode;
		double q1, q3;
		double variance, stddev, cv;
		double skewness, kurtosis;
	};

	struct HistogramData {
		unsigned int numClasses;
		std::vector<double> lowerLimits;
		std::vector<unsigned int> frequencies;
		std::vector<double> relativeFrequencies;
	};

	struct BoxplotData {
		double min, q1, median, q3, max, iqr;
		std::vector<double> outliers;
	};

	/**
	 * FitResult param semantics:
	 * uniform:      p1=min, p2=max
	 * triangular:   p1=min, p2=mode, p3=max
	 * normal:       p1=mean, p2=stddev
	 * exponential:  p1=mean
	 * erlang:       p1=mean, p2=m (integer shape)
	 * beta:         p1=alpha, p2=beta, p3=infLimit, p4=supLimit
	 * weibull:      p1=alpha (shape), p2=scale
	 *
	 * rmse = sqrt(sse / n): average CDF error per observation.
	 * r2   = 1 - SSE/SST: proportion of empirical CDF variance explained by the fit.
	 *        SST = sum((p_i - 0.5)^2), p_i = (i+0.5)/n  (uniform order-statistic baseline).
	 *        r2 close to 1 means the fitted CDF tracks the empirical CDF well.
	 */
	struct FitResult {
		std::string distributionName;
		double sse;                                     ///< sum of squared CDF errors
		double rmse;                                    ///< sqrt(sse / n) — per-point error
		double r2;                                      ///< coefficient of determination vs empirical CDF
		double param1, param2, param3, param4, param5;
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

	/**
	 * @brief Consolidated result of fitting one distribution and running all three
	 *        goodness-of-fit tests (chi-square, KS, Anderson-Darling) in one call.
	 */
	struct FitReport {
		FitResult   fit;        ///< parameters + SSE/RMSE/R²
		GoFResult   chiSquare;  ///< chi-square goodness-of-fit
		GoFResult   ks;         ///< Kolmogorov-Smirnov
		GoFResult   ad;         ///< Anderson-Darling
	};

	/**
	 * @brief Diagnostics for detecting trend and seasonality in a time series.
	 *
	 * hasTrend is true when |ACF[1]| exceeds the Bartlett 95% confidence band
	 * (1.96/sqrt(n)), which is the standard criterion for declaring lag-1
	 * autocorrelation significant under the white-noise null hypothesis.
	 *
	 * hasSeasonality is true when any |ACF[k]|, k > 1, exceeds the band.
	 *
	 * trendSlope and trendIntercept come from ordinary least-squares regression
	 * of the observations on their sequential index (0, 1, …, n-1).
	 */
	struct TrendDiagnostic {
		bool   hasTrend;        ///< |ACF[1]| > 1.96/sqrt(n)
		bool   hasSeasonality;  ///< any |ACF[k]|, k>1, > 1.96/sqrt(n)
		double trendSlope;      ///< OLS slope (units per observation)
		double trendIntercept;  ///< OLS intercept
	};

	/**
	 * @brief Output of correlogram(): ACF values plus the 95% confidence band.
	 *
	 * confidenceBound is the Bartlett approximation ±1.96/sqrt(n), which is
	 * the standard threshold for declaring an autocorrelation lag significant
	 * at the 95% level under the white-noise null hypothesis.
	 * acf[0] is always 1.0 (lag 0).
	 */
	struct CorrelogramData {
		std::vector<double> acf;   // acf[k] = autocorrelation at lag k, k in [0, maxLag]
		double confidenceBound;    // +/- 1.96 / sqrt(n)
		unsigned int n;            // sample size used
	};

	// ---- data loading ----------------------------------------------------

	/**
	 * @brief Loads data from a text file (RawNumeric, RecordLegacy, RecordEnriched,
	 *        or GuiTabular). Returns true on success, false on failure.
	 *        On failure the dataset is cleared and getLastError() contains
	 *        a human-readable description.
	 */
	virtual bool setDataFilename(const std::string& filename) = 0;
	virtual void setDataValues(const std::vector<double>& values) = 0;
	virtual void clearData() = 0;
	virtual bool loadSecondSample(const std::vector<double>& values) = 0;
	virtual bool loadSecondSampleFromFile(const std::string& filename) = 0;

	/**
	 * @brief Returns the error message from the most recent failed operation
	 *        (setDataFilename or loadSecondSampleFromFile). Empty string when
	 *        no error has occurred.
	 */
	virtual std::string getLastError() const = 0;

	// ---- configuration ---------------------------------------------------

	virtual void setConfidenceLevel(double confidenceLevel) = 0;
	virtual double getConfidenceLevel() = 0;
	virtual void setSignificanceLevel(double significanceLevel) = 0;
	virtual double getSignificanceLevel() = 0;

	// ---- descriptive statistics ------------------------------------------

	virtual SummaryStatistics summaryStatistics() = 0;
	virtual double quartile(unsigned short num) = 0;  // 1-3
	virtual double decile(unsigned short num) = 0;    // 1-9
	virtual double centile(unsigned short num) = 0;   // 1-99

	// ---- exploratory structures ------------------------------------------

	virtual HistogramData histogramStructure(unsigned int numClasses) = 0;
	virtual BoxplotData boxplotStatistics() = 0;

	// ---- distribution fitting -------------------------------------------

	virtual FitResult fitDistribution(const std::string& name) = 0;
	virtual FitResult fitAll() = 0;

	/**
	 * @brief Fits all supported distributions and returns a ranked list.
	 *
	 * Returns one FitResult per distribution (uniform, triangular, normal,
	 * exponential, erlang, beta, weibull), sorted by SSE ascending so the
	 * best-fitting candidate comes first. Distributions that failed to fit
	 * (valid == false) are placed at the end of the list.
	 */
	virtual std::vector<FitResult> fitAllRanked() = 0;

	// ---- goodness-of-fit tests ------------------------------------------

	virtual GoFResult chiSquareGoodnessOfFit(const std::string& distributionName, double significanceLevel) = 0;
	virtual GoFResult kolmogorovSmirnov(const std::string& distributionName, double significanceLevel) = 0;

	/**
	 * @brief Anderson-Darling goodness-of-fit test.
	 *
	 * More powerful than KS for detecting deviations in the distribution tails,
	 * which is particularly relevant for simulation output data (e.g. long-tailed
	 * service times). The p-value uses the asymptotic Marsaglia approximation;
	 * for n < 25 results are approximate.
	 */
	virtual GoFResult andersonDarling(const std::string& distributionName, double significanceLevel) = 0;

	/**
	 * @brief Fits a distribution and runs all three GoF tests in one call.
	 *
	 * Equivalent to calling fitDistribution + chiSquareGoodnessOfFit +
	 * kolmogorovSmirnov + andersonDarling individually, but more convenient
	 * for interactive analysis and reporting.
	 */
	virtual FitReport analyzeFit(const std::string& distributionName, double significanceLevel) = 0;

	// ---- time-series analysis -------------------------------------------

	virtual std::vector<double> movingAverage(unsigned int window) = 0;
	virtual std::vector<double> autocorrelation(unsigned int maxLag) = 0;

	/**
	 * @brief Detects linear trend and seasonality from the ACF and OLS regression.
	 *
	 * Uses the Bartlett 95% band (±1.96/sqrt(n)) to classify lags as significant.
	 * Linear trend parameters come from ordinary least-squares on the index sequence.
	 * Returns a zeroed-out TrendDiagnostic if fewer than 4 observations are loaded.
	 */
	virtual TrendDiagnostic detectTrend() = 0;

	/**
	 * @brief Returns the autocorrelation function together with its 95%
	 *        confidence bounds (±1.96/sqrt(n)).
	 *
	 * Unlike autocorrelation(), which returns only the ACF values,
	 * correlogram() provides the information needed to render a proper
	 * correlogram chart: the confidence band that separates significant
	 * from non-significant lags.
	 */
	virtual CorrelogramData correlogram(unsigned int maxLag) = 0;

	/**
	 * @brief Probability that a future observation exceeds x, given the fitted distribution.
	 *
	 * Returns 1 - F(x) where F is the theoretical CDF of the named distribution
	 * fitted to the current dataset. Returns NaN if the fit fails.
	 */
	virtual double exceedanceProbability(double x, const std::string& distributionName) = 0;

	/**
	 * @brief Exceedance curve: P(X > x) evaluated at `points` equally-spaced x values
	 *        in [xMin, xMax].
	 *
	 * Returns a vector of size `points` where element i corresponds to
	 * x_i = xMin + i*(xMax-xMin)/(points-1). Returns an empty vector if the
	 * fit fails or the inputs are invalid.
	 */
	virtual std::vector<double> exceedanceCurve(double xMin, double xMax,
	                                             unsigned int points,
	                                             const std::string& distributionName) = 0;

	// ---- inference: one-population confidence intervals -----------------

	virtual HypothesisTester_if::ConfidenceInterval averageConfidenceInterval() = 0;
	virtual HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(checkProportionFunction checker) = 0;
	virtual HypothesisTester_if::ConfidenceInterval varianceConfidenceInterval() = 0;

	// ---- inference: one-population hypothesis tests ---------------------

	virtual HypothesisTester_if::TestResult testAverage(double hypothesizedMean, HypothesisTester_if::H1Comparition comp) = 0;
	virtual HypothesisTester_if::TestResult testProportion(double hypothesizedProp, checkProportionFunction checker, HypothesisTester_if::H1Comparition comp) = 0;
	virtual HypothesisTester_if::TestResult testVariance(double hypothesizedVariance, HypothesisTester_if::H1Comparition comp) = 0;

	// ---- inference: two-population hypothesis tests ---------------------

	virtual HypothesisTester_if::TestResult testAverageTwoSamples(HypothesisTester_if::H1Comparition comp) = 0;
	virtual HypothesisTester_if::TestResult testProportionTwoSamples(checkProportionFunction checker, HypothesisTester_if::H1Comparition comp) = 0;
	virtual HypothesisTester_if::TestResult testVarianceTwoSamples(HypothesisTester_if::H1Comparition comp) = 0;
};

#endif /* DATAANALYZER_IF_H */
