#ifndef HYPOTHESISTESTER_IF_H
#define HYPOTHESISTESTER_IF_H

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

typedef bool (*checkProportionFunction)(double value);
typedef std::function<double(double value)> distributionCdfFunction;

/**
 * @brief Interface for classical parametric inference.
 *
 * Purpose:
 * - Provide confidence intervals and hypothesis tests from parameters or
 *   dataset files.
 *
 * Mathematical meaning:
 * - Contract follows classical statistical inference for one-population and
 *   two-population analyses.
 *
 * Confidence level contract:
 * - All confidenceLevel parameters are expected to be probabilities in (0, 1).
 */
class HypothesisTester_if {
public:
	virtual ~HypothesisTester_if() = default;

	/**
	 * @brief Confidence interval [inferior, superior] and e0.
	 *
	 * e0 is the usual margin of error for symmetric intervals. For asymmetric
	 * intervals, such as variance and variance-ratio intervals, it is only half
	 * of the reported interval width.
	 */
	class ConfidenceInterval {
	public:

		/** @brief Creates an interval and stores bounds in increasing order. */
		ConfidenceInterval(double inferiorLimit, double superiorLimit, double e0) {
			if (inferiorLimit <= superiorLimit) {
				_infLim = inferiorLimit;
				_supLim = superiorLimit;
			} else {
				_infLim = superiorLimit;
				_supLim = inferiorLimit;
			}
			_e0 = e0;
		}

		/** @brief Returns the lower confidence-interval limit. */
		double inferiorLimit() const {
			return _infLim;
		}

		/** @brief Returns the upper confidence-interval limit. */
		double superiorLimit() const {
			return _supLim;
		}

		/** @brief Returns the margin or half interval width stored as e0. */
		double halfWidth() const {
			return _e0;
		}

	private:
		double _infLim, _supLim, _e0;
	};

	/**
	 * @brief Alternative-hypothesis comparison mode.
	 * DIFFERENT: two-sided; LESS_THAN: left-tailed; GREATER_THAN: right-tailed.
	 */
	enum H1Comparition {
		DIFFERENT = 1,
		LESS_THAN = 2,
		GREATER_THAN = 3
	};

	/**
	 * @brief Result of a parametric hypothesis test.
	 *
	 * Contains p-value, reject/accept decision for H0, acceptance bounds for the
	 * test statistic and observed statistic value.
	 */
	class TestResult {
	public:
		struct GoodnessOfFitDetails {
			/** @brief Creates empty goodness-of-fit diagnostic details. */
			GoodnessOfFitDetails()
					: available(false),
					initialClasses(0),
					effectiveClasses(0),
					estimatedParameters(0),
					degreesOfFreedom(0.0),
					observedTotal(0.0),
					expectedTotal(0.0) {
			}

			bool available;
			std::size_t initialClasses;
			std::size_t effectiveClasses;
			unsigned int estimatedParameters;
			double degreesOfFreedom;
			double observedTotal;
			double expectedTotal;
		};

		/** @brief Creates a hypothesis-test result without goodness-of-fit details. */
		TestResult(double pvalue, bool rejectH0, double acceptanceInferiorLimit, double acceptanceSuperiorLimit, double testStat)
				: TestResult(pvalue, rejectH0, acceptanceInferiorLimit, acceptanceSuperiorLimit, testStat, GoodnessOfFitDetails()) {
		}

		/** @brief Creates a hypothesis-test result with optional diagnostics. */
		TestResult(double pvalue, bool rejectH0, double acceptanceInferiorLimit, double acceptanceSuperiorLimit, double testStat, GoodnessOfFitDetails goodnessOfFitDetails) {
			_pvalue = pvalue;
			_rejectH0 = rejectH0;
			_acceptanceInferiorLimit = acceptanceInferiorLimit;
			_acceptanceSuperiorLimit = acceptanceSuperiorLimit;
			_testStat = testStat;
			_goodnessOfFitDetails = goodnessOfFitDetails;
		}

		/** @brief Returns true when the test rejects H0. */
		inline bool rejectH0() const {
			return _rejectH0;
		}

		/** @brief Returns true when the test does not reject H0. */
		inline bool acceptH0() const {
			return !_rejectH0;
		}

		/** @brief Returns the computed p-value. */
		inline double pValue() const {
			return _pvalue;
		}

		/** @brief Returns the observed test statistic. */
		inline double testStat() const {
			return _testStat;
		}

		/** @brief Returns the lower acceptance-region limit. */
		inline double acceptanceInferiorLimit() const {
			return _acceptanceInferiorLimit;
		}

		/** @brief Returns the upper acceptance-region limit. */
		inline double acceptanceSuperiorLimit() const {
			return _acceptanceSuperiorLimit;
		}

		/** @brief Returns whether goodness-of-fit diagnostics are available. */
		inline bool hasGoodnessOfFitDetails() const {
			return _goodnessOfFitDetails.available;
		}

		/** @brief Returns goodness-of-fit diagnostics when available. */
		inline GoodnessOfFitDetails goodnessOfFitDetails() const {
			return _goodnessOfFitDetails;
		}

	private:
		double _pvalue, _acceptanceInferiorLimit, _acceptanceSuperiorLimit, _testStat;
		bool _rejectH0;
		GoodnessOfFitDetails _goodnessOfFitDetails;
	};


public:
	// One-population confidence intervals.
	/** @brief Computes a Student-t confidence interval for one mean. */
	virtual HypothesisTester_if::ConfidenceInterval averageConfidenceInterval(double avg, double stddev, unsigned int n, double confidenceLevel) = 0;
	/** @brief Computes a normal-approximation confidence interval for one proportion. */
	virtual HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(double prop, unsigned int n, double confidenceLevel) = 0;
	/** @brief Computes a finite-population confidence interval for one proportion. */
	virtual HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(double prop, unsigned int n, int N, double confidenceLevel) = 0;
	/** @brief Computes a chi-square confidence interval for one variance. */
	virtual HypothesisTester_if::ConfidenceInterval varianceConfidenceInterval(double var, unsigned int n, double confidenceLevel) = 0;
	// Two-population confidence intervals.
	/** @brief Computes a confidence interval for the difference of two means. */
	virtual HypothesisTester_if::ConfidenceInterval averageDifferenceConfidenceInterval(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel) = 0;
	/** @brief Computes a confidence interval for the difference of two proportions. */
	virtual HypothesisTester_if::ConfidenceInterval proportionDifferenceConfidenceInterval(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel) = 0;
	/** @brief Computes an F confidence interval for the ratio of two variances. */
	virtual HypothesisTester_if::ConfidenceInterval varianceRatioConfidenceInterval(double var1, unsigned int n1, double var2, unsigned int n2, double confidenceLevel) = 0;
	// File-based confidence intervals.
	/** @brief Loads a sample file and computes a confidence interval for one mean. */
	virtual HypothesisTester_if::ConfidenceInterval averageConfidenceInterval(std::string sampleDataFilename, double confidenceLevel) = 0;
	/** @brief Loads a sample file and computes a confidence interval for one proportion. */
	virtual HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(std::string sampleDataFilename, checkProportionFunction function, double confidenceLevel) = 0;
	/** @brief Loads a sample file and computes a finite-population proportion interval. */
	virtual HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(std::string sampleDataFilename, checkProportionFunction function, double N, double confidenceLevel) = 0;
	/** @brief Loads a sample file and computes a confidence interval for variance. */
	virtual HypothesisTester_if::ConfidenceInterval varianceConfidenceInterval(std::string sampleDataFilename, double confidenceLevel) = 0;
	// Sample-size planning based on a desired confidence-interval half-width.
	/** @brief Estimates the sample size needed for a target mean half-width. */
	virtual unsigned int estimateSampleSize(double avg, double stddev, double desiredE0, double confidenceLevel) = 0;

	// One-population tests.
	/** @brief Tests one population mean against a hypothesized mean. */
	virtual HypothesisTester_if::TestResult testAverage(double avg, double stddev, unsigned int n, double avgSample, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	/** @brief Tests one population proportion against a hypothesized proportion. */
	virtual HypothesisTester_if::TestResult testProportion(double prop, unsigned int n, double proptest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	/** @brief Tests one population variance against a hypothesized variance. */
	virtual HypothesisTester_if::TestResult testVariance(double var, unsigned int n, double vartest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	// Two-population tests.
	/** @brief Tests the difference between two population means. */
	virtual HypothesisTester_if::TestResult testAverage(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	/** @brief Tests the difference between two population proportions. */
	virtual HypothesisTester_if::TestResult testProportion(double prop1, unsigned int n1, double prop2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	/** @brief Tests the ratio between two population variances. */
	virtual HypothesisTester_if::TestResult testVariance(double var1, unsigned int n1, double var2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	// One-population file-based tests.
	/** @brief Loads a sample file and tests one population mean. */
	virtual HypothesisTester_if::TestResult testAverage(std::string sampleDataFilename, double avgSample, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	/** @brief Loads a sample file and tests one population proportion. */
	virtual HypothesisTester_if::TestResult testProportion(std::string sampleDataFilename, checkProportionFunction function, double proptest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	/** @brief Loads a sample file and tests one population variance. */
	virtual HypothesisTester_if::TestResult testVariance(std::string sampleDataFilename, double vartest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	// Two-population file-based tests.
	/** @brief Loads two sample files and tests the difference of means. */
	virtual HypothesisTester_if::TestResult testAverage(std::string firstSampleDataFilename, std::string secondSampleDataFilename, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	/** @brief Loads two sample files and tests the difference of proportions. */
	virtual HypothesisTester_if::TestResult testProportion(std::string firstSampleDataFilename, std::string secondSampleDataFilename, checkProportionFunction function, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	/** @brief Loads two sample files and tests the ratio of variances. */
	virtual HypothesisTester_if::TestResult testVariance(std::string firstSampleDataFilename, std::string secondSampleDataFilename, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	// Goodness-of-fit tests.
	/** @brief Runs chi-square goodness-of-fit from observed and expected frequencies. */
	virtual HypothesisTester_if::TestResult chiSquareGoodnessOfFit(const std::vector<double>& observedFrequencies, const std::vector<double>& expectedFrequencies, unsigned int estimatedParameters, double confidenceLevel) = 0;
	/** @brief Runs chi-square goodness-of-fit from raw data and automatic classes. */
	virtual HypothesisTester_if::TestResult chiSquareGoodnessOfFit(const std::vector<double>& sample, distributionCdfFunction cdf, unsigned int estimatedParameters, double confidenceLevel, std::size_t classCount = 0, double minExpectedFrequency = 5.0) = 0;
	/** @brief Runs chi-square goodness-of-fit from raw data and explicit classes. */
	virtual HypothesisTester_if::TestResult chiSquareGoodnessOfFit(const std::vector<double>& sample, distributionCdfFunction cdf, const std::vector<double>& classBoundaries, unsigned int estimatedParameters, double confidenceLevel, double minExpectedFrequency = 5.0) = 0;
	/** @brief Runs a one-sample Kolmogorov-Smirnov test from raw data. */
	virtual HypothesisTester_if::TestResult kolmogorovSmirnov(const std::vector<double>& sample, distributionCdfFunction cdf, double confidenceLevel) = 0;
	/** @brief Loads a sample file and runs a one-sample Kolmogorov-Smirnov test. */
	virtual HypothesisTester_if::TestResult kolmogorovSmirnov(std::string sampleDataFilename, distributionCdfFunction cdf, double confidenceLevel) = 0;
};

#endif /* HYPOTHESISTESTER_IF_H */
