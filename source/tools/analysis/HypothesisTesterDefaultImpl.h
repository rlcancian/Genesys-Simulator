#ifndef HypothesisTesterDefaultImpl_H
#define HypothesisTesterDefaultImpl_H

#include "HypothesisTester_if.h"

/**
 * @brief Current default implementation of HypothesisTester_if.
 *
 * Architectural role:
 * - Default hypothesis-testing implementation used by DataAnalyserDefaultImpl.
 *
 * Implementation status:
 * - Consolidated for confidence intervals, parametric tests and goodness-of-fit
 *   tests required by the analysis tool.
 */
class HypothesisTesterDefaultImpl : public HypothesisTester_if {
public:
	/** @brief Creates the stateless default tester. */
	HypothesisTesterDefaultImpl();
	/** @brief Destroys the tester without owning external resources. */
	~HypothesisTesterDefaultImpl() = default;
public:
	// One-population confidence intervals.
	/** @brief Computes a Student-t confidence interval for one mean. */
	HypothesisTester_if::ConfidenceInterval averageConfidenceInterval(double avg, double stddev, unsigned int n, double confidenceLevel) override;
	/** @brief Computes a normal-approximation confidence interval for one proportion. */
	HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(double prop, unsigned int n, double confidenceLevel) override;
	/** @brief Computes a finite-population confidence interval for one proportion. */
	HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(double prop, unsigned int n, int N, double confidenceLevel) override;
	/** @brief Computes a chi-square confidence interval for one variance. */
	HypothesisTester_if::ConfidenceInterval varianceConfidenceInterval(double var, unsigned int n, double confidenceLevel) override;

	// Two-population confidence intervals.
	/** @brief Computes a confidence interval for the difference of two means. */
	HypothesisTester_if::ConfidenceInterval averageDifferenceConfidenceInterval(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel) override;
	/** @brief Computes a confidence interval for the difference of two proportions. */
	HypothesisTester_if::ConfidenceInterval proportionDifferenceConfidenceInterval(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel) override;
	/** @brief Computes an F confidence interval for the ratio of two variances. */
	HypothesisTester_if::ConfidenceInterval varianceRatioConfidenceInterval(double var1, unsigned int n1, double var2, unsigned int n2, double confidenceLevel) override;

	// File-based confidence intervals. GenESyS Record/GUI result files are
	// parsed through tools/analysis before falling back to raw numeric files.
	/** @brief Loads a sample file and computes a confidence interval for one mean. */
	HypothesisTester_if::ConfidenceInterval averageConfidenceInterval(std::string sampleDataFilename, double confidenceLevel) override;
	/** @brief Loads a sample file and computes a confidence interval for one proportion. */
	HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(std::string sampleDataFilename, checkProportionFunction function, double confidenceLevel) override;
	/** @brief Loads a sample file and computes a finite-population proportion interval. */
	HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(std::string sampleDataFilename, checkProportionFunction function, double N, double confidenceLevel) override;
	/** @brief Loads a sample file and computes a confidence interval for variance. */
	HypothesisTester_if::ConfidenceInterval varianceConfidenceInterval(std::string sampleDataFilename, double confidenceLevel) override;

	// Sample-size planning.
	/** @brief Estimates the sample size needed for a target mean half-width. */
	unsigned int estimateSampleSize(double avg, double stddev, double desiredE0, double confidenceLevel) override;

	// One-population tests.
	/** @brief Tests one population mean against a hypothesized mean. */
	HypothesisTester_if::TestResult testAverage(double avg, double stddev, unsigned int n, double avgSample, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;
	/** @brief Tests one population proportion against a hypothesized proportion. */
	HypothesisTester_if::TestResult testProportion(double prop, unsigned int n, double proptest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;
	/** @brief Tests one population variance against a hypothesized variance. */
	HypothesisTester_if::TestResult testVariance(double var, unsigned int n, double vartest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;

	// Two-population tests.
	/** @brief Tests the difference between two population means. */
	HypothesisTester_if::TestResult testAverage(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;
	/** @brief Tests the difference between two population proportions. */
	HypothesisTester_if::TestResult testProportion(double prop1, unsigned int n1, double prop2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;
	/** @brief Tests the ratio between two population variances. */
	HypothesisTester_if::TestResult testVariance(double var1, unsigned int n1, double var2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;

	// File-based one-population tests.
	/** @brief Loads a sample file and tests one population mean. */
	HypothesisTester_if::TestResult testAverage(std::string sampleDataFilename, double avgSample, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;
	/** @brief Loads a sample file and tests one population proportion. */
	HypothesisTester_if::TestResult testProportion(std::string sampleDataFilename, checkProportionFunction function, double proptest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;
	/** @brief Loads a sample file and tests one population variance. */
	HypothesisTester_if::TestResult testVariance(std::string sampleDataFilename, double vartest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;

	// File-based two-population tests.
	/** @brief Loads two sample files and tests the difference of means. */
	HypothesisTester_if::TestResult testAverage(std::string firstSampleDataFilename, std::string secondSampleDataFilename, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;
	/** @brief Loads two sample files and tests the difference of proportions. */
	HypothesisTester_if::TestResult testProportion(std::string firstSampleDataFilename, std::string secondSampleDataFilename, checkProportionFunction function, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;
	/** @brief Loads two sample files and tests the ratio of variances. */
	HypothesisTester_if::TestResult testVariance(std::string firstSampleDataFilename, std::string secondSampleDataFilename, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;

	// Goodness-of-fit tests.
	/** @brief Runs chi-square goodness-of-fit from observed and expected frequencies. */
	HypothesisTester_if::TestResult chiSquareGoodnessOfFit(const std::vector<double>& observedFrequencies, const std::vector<double>& expectedFrequencies, unsigned int estimatedParameters, double confidenceLevel) override;
	/** @brief Runs chi-square goodness-of-fit from raw data and automatic classes. */
	HypothesisTester_if::TestResult chiSquareGoodnessOfFit(const std::vector<double>& sample, distributionCdfFunction cdf, unsigned int estimatedParameters, double confidenceLevel, std::size_t classCount = 0, double minExpectedFrequency = 5.0) override;
	/** @brief Runs chi-square goodness-of-fit from raw data and explicit classes. */
	HypothesisTester_if::TestResult chiSquareGoodnessOfFit(const std::vector<double>& sample, distributionCdfFunction cdf, const std::vector<double>& classBoundaries, unsigned int estimatedParameters, double confidenceLevel, double minExpectedFrequency = 5.0) override;
	/** @brief Runs a one-sample Kolmogorov-Smirnov test from raw data. */
	HypothesisTester_if::TestResult kolmogorovSmirnov(const std::vector<double>& sample, distributionCdfFunction cdf, double confidenceLevel) override;
	/** @brief Loads a sample file and runs a one-sample Kolmogorov-Smirnov test. */
	HypothesisTester_if::TestResult kolmogorovSmirnov(std::string sampleDataFilename, distributionCdfFunction cdf, double confidenceLevel) override;
};

#endif /* HypothesisTesterDefaultImpl_H */
