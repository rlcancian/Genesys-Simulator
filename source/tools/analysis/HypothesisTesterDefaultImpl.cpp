#include "HypothesisTesterDefaultImpl.h"
#include "DatasetLoader.h"
#include "SimulationResultsDataset.h"
#include "tools/analysis/ProbabilityDistribution.h"
#include "tools/SolverDefaultImpl1.h"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {

struct ChiSquareClass {
	double observed = 0.0;
	double expected = 0.0;
};

double clampProbability(double value) {
	return std::clamp(value, 0.0, 1.0);
}

double normalCdf(double z) {
	return 0.5 * std::erfc(-z / std::sqrt(2.0));
}

double chi2CdfByIntegration(double chi2, double degreesOfFreedom) {
	if (chi2 <= 0.0) {
		return 0.0;
	}
	if (degreesOfFreedom <= 0.0 || !std::isfinite(degreesOfFreedom)) {
		throw std::invalid_argument("chi2CdfByIntegration requires positive finite degreesOfFreedom");
	}
	if (std::fabs(degreesOfFreedom - 1.0) < 1e-12) {
		return clampProbability(std::erf(std::sqrt(chi2 / 2.0)));
	}
	if (std::fabs(degreesOfFreedom - 2.0) < 1e-12) {
		return clampProbability(1.0 - std::exp(-chi2 / 2.0));
	}
	// Numerical integration keeps chi-square p-values aligned with the same PDF
	// family used by the quantile helper.
	SolverDefaultImpl1 integrator(1e-6, 10000);
	const double integral = integrator.integrate(0.0, chi2, ProbabilityDistributionBase::chi2, degreesOfFreedom);
	return clampProbability(integral);
}

void validateConfidenceLevel(double confidenceLevel) {
	if (!(confidenceLevel > 0.0 && confidenceLevel < 1.0)) {
		throw std::invalid_argument("confidenceLevel must be in (0,1)");
	}
}

double studentTCdf(double t, double degreesOfFreedom) {
	if (degreesOfFreedom <= 0.0 || !std::isfinite(degreesOfFreedom)) {
		throw std::invalid_argument("studentTCdf requires positive finite degreesOfFreedom");
	}
	if (t == 0.0) {
		return 0.5;
	}
	SolverDefaultImpl1 integrator(1e-6, 10000);
	const double absT = std::fabs(t);
	const double integral = integrator.integrate(0.0, absT, ProbabilityDistributionBase::tStudent, 0.0, 1.0, degreesOfFreedom);
	const double cdf = (t > 0.0) ? (0.5 + integral) : (0.5 - integral);
	return clampProbability(cdf);
}

double fisherSnedecorCdf(double f, double d1, double d2) {
	if (f <= 0.0) {
		return 0.0;
	}
	if (d1 <= 0.0 || d2 <= 0.0 || !std::isfinite(d1) || !std::isfinite(d2)) {
		throw std::invalid_argument("fisherSnedecorCdf requires positive finite degrees of freedom");
	}
	SolverDefaultImpl1 integrator(1e-6, 10000);
	const double integral = integrator.integrate(0.0, f, ProbabilityDistributionBase::fisherSnedecor, d1, d2);
	return clampProbability(integral);
}

double kolmogorovSmirnovPValue(double d, unsigned int n) {
	if (d <= 0.0) {
		return 1.0;
	}
	if (n == 0) {
		throw std::invalid_argument("kolmogorovSmirnovPValue requires n > 0");
	}
	const double sqrtN = std::sqrt(static_cast<double>(n));
	const double lambda = (sqrtN + 0.12 + 0.11 / sqrtN) * d;
	double sum = 0.0;
	// Classical one-sample asymptotic series. The implementation intentionally
	// does not apply Lilliefors/bootstrap correction when parameters are fitted
	// from the same sample; callers/documentation treat that p-value as diagnostic.
	for (unsigned int k = 1; k <= 100; ++k) {
		const double term = std::exp(-2.0 * static_cast<double>(k * k) * lambda * lambda);
		sum += (k % 2 == 1) ? term : -term;
		if (term < 1e-12) {
			break;
		}
	}
	return clampProbability(2.0 * sum);
}

double pValueFromCdf(double cdf, HypothesisTester_if::H1Comparition comp) {
	if (comp == HypothesisTester_if::H1Comparition::LESS_THAN) {
		return clampProbability(cdf);
	}
	if (comp == HypothesisTester_if::H1Comparition::GREATER_THAN) {
		return clampProbability(1.0 - cdf);
	}
	return clampProbability(2.0 * std::min(cdf, 1.0 - cdf));
}

DatasetLoader loadSampleData(const std::string& sampleDataFilename) {
	SimulationResultsDataset dataset;
	std::string parserError;
	// Prefer the simulation-results parser so Record/GUI metadata formats are
	// reduced to their observation values before falling back to raw numeric text.
	if (SimulationResultsParser::loadFromTextFile(sampleDataFilename, &dataset, &parserError) && dataset.hasNumericData()) {
		DatasetLoader loader;
		if (loader.loadFromVector(dataset.values())) {
			return loader;
		}
	}

	DatasetLoader loader;
	if (loader.loadFromFile(sampleDataFilename, ',') || loader.loadFromFile(sampleDataFilename, ' ')) {
		return loader;
	}
	throw std::invalid_argument("Could not load a usable numeric sample dataset: " + sampleDataFilename);
}

unsigned int countMatches(const std::vector<double>& data, checkProportionFunction function) {
	if (function == nullptr) {
		throw std::invalid_argument("checkProportionFunction must not be null");
	}
	unsigned int count = 0;
	for (double value : data) {
		if (function(value)) {
			++count;
		}
	}
	return count;
}

std::size_t defaultChiSquareClassCount(std::size_t count) {
	if (count == 0) {
		return 0;
	}
	return static_cast<std::size_t>(std::ceil(1.0 + 3.322 * std::log10(static_cast<double>(count))));
}

void validateSampleAndCdf(const std::vector<double>& sample, const distributionCdfFunction& cdf) {
	if (sample.empty()) {
		throw std::invalid_argument("chiSquareGoodnessOfFit requires a non-empty sample");
	}
	if (!cdf) {
		throw std::invalid_argument("chiSquareGoodnessOfFit requires a valid CDF function");
	}
	for (double value : sample) {
		if (!std::isfinite(value)) {
			throw std::invalid_argument("chiSquareGoodnessOfFit requires finite sample values");
		}
	}
}

double checkedCdf(const distributionCdfFunction& cdf, double value) {
	const double probability = cdf(value);
	if (probability < 0.0 || probability > 1.0 || !std::isfinite(probability)) {
		throw std::invalid_argument("chiSquareGoodnessOfFit CDF must return finite probabilities in [0,1]");
	}
	return probability;
}

std::vector<double> automaticClassBoundaries(const std::vector<double>& sample, std::size_t classCount) {
	auto minmax = std::minmax_element(sample.begin(), sample.end());
	const double min = *minmax.first;
	const double max = *minmax.second;
	if (!(max > min)) {
		throw std::invalid_argument("chiSquareGoodnessOfFit requires sample range > 0 for automatic classes");
	}
	if (classCount == 0) {
		classCount = defaultChiSquareClassCount(sample.size());
	}
	if (classCount < 2) {
		throw std::invalid_argument("chiSquareGoodnessOfFit requires at least two initial classes");
	}

	std::vector<double> boundaries;
	boundaries.reserve(classCount + 1U);
	const double width = (max - min) / static_cast<double>(classCount);
	for (std::size_t i = 0; i <= classCount; ++i) {
		boundaries.push_back((i == classCount) ? max : min + static_cast<double>(i) * width);
	}
	return boundaries;
}

void validateClassBoundaries(const std::vector<double>& classBoundaries) {
	if (classBoundaries.size() < 3) {
		throw std::invalid_argument("chiSquareGoodnessOfFit requires at least two classes");
	}
	for (double boundary : classBoundaries) {
		if (!std::isfinite(boundary)) {
			throw std::invalid_argument("chiSquareGoodnessOfFit requires finite class boundaries");
		}
	}
	for (std::size_t i = 1; i < classBoundaries.size(); ++i) {
		if (!(classBoundaries[i] > classBoundaries[i - 1U])) {
			throw std::invalid_argument("chiSquareGoodnessOfFit requires strictly increasing class boundaries");
		}
	}
}

std::vector<ChiSquareClass> buildChiSquareClasses(const std::vector<double>& sample, const distributionCdfFunction& cdf, const std::vector<double>& classBoundaries) {
	validateClassBoundaries(classBoundaries);
	std::vector<ChiSquareClass> classes(classBoundaries.size() - 1U);
	const double lowerLimit = classBoundaries.front();
	const double upperLimit = classBoundaries.back();
	const double totalProbability = checkedCdf(cdf, upperLimit) - checkedCdf(cdf, lowerLimit);
	if (!(totalProbability > 0.0) || !std::isfinite(totalProbability)) {
		throw std::invalid_argument("chiSquareGoodnessOfFit requires positive CDF probability over class boundaries");
	}

	for (double value : sample) {
		if (value < lowerLimit || value > upperLimit) {
			throw std::invalid_argument("chiSquareGoodnessOfFit sample values must be covered by class boundaries");
		}
		auto upper = std::upper_bound(classBoundaries.begin(), classBoundaries.end(), value);
		std::size_t index = static_cast<std::size_t>(std::distance(classBoundaries.begin(), upper));
		if (index == 0) {
			index = 1;
		}
		if (index >= classBoundaries.size()) {
			index = classBoundaries.size() - 1U;
		}
		++classes[index - 1U].observed;
	}

	const double n = static_cast<double>(sample.size());
	for (std::size_t i = 0; i < classes.size(); ++i) {
		const double lower = classBoundaries[i];
		const double upper = classBoundaries[i + 1U];
		const double probability = checkedCdf(cdf, upper) - checkedCdf(cdf, lower);
		if (probability < 0.0 || !std::isfinite(probability)) {
			throw std::invalid_argument("chiSquareGoodnessOfFit requires nondecreasing CDF over class boundaries");
		}
		classes[i].expected = n * probability / totalProbability;
	}
	return classes;
}

std::vector<ChiSquareClass> groupChiSquareClasses(const std::vector<ChiSquareClass>& initialClasses, double minExpectedFrequency) {
	if (!(minExpectedFrequency >= 0.0) || !std::isfinite(minExpectedFrequency)) {
		throw std::invalid_argument("chiSquareGoodnessOfFit requires finite minExpectedFrequency >= 0");
	}

	std::vector<ChiSquareClass> grouped;
	ChiSquareClass current;
	for (std::size_t i = 0; i < initialClasses.size(); ++i) {
		current.observed += initialClasses[i].observed;
		current.expected += initialClasses[i].expected;
		const bool last = (i + 1U == initialClasses.size());
		if (current.expected >= minExpectedFrequency || last) {
			grouped.push_back(current);
			current = ChiSquareClass{};
		}
	}

	if (grouped.size() > 1U && grouped.back().expected < minExpectedFrequency) {
		grouped[grouped.size() - 2U].observed += grouped.back().observed;
		grouped[grouped.size() - 2U].expected += grouped.back().expected;
		grouped.pop_back();
	}
	return grouped;
}

void splitChiSquareClasses(const std::vector<ChiSquareClass>& classes, std::vector<double>* observed, std::vector<double>* expected) {
	observed->clear();
	expected->clear();
	observed->reserve(classes.size());
	expected->reserve(classes.size());
	for (const ChiSquareClass& cls : classes) {
		observed->push_back(cls.observed);
		expected->push_back(cls.expected);
	}
}

HypothesisTester_if::TestResult makeChiSquareResult(const std::vector<double>& observedFrequencies,
		const std::vector<double>& expectedFrequencies,
		unsigned int estimatedParameters,
		double confidenceLevel,
		std::size_t initialClasses) {
	validateConfidenceLevel(confidenceLevel);
	if (observedFrequencies.size() != expectedFrequencies.size() || observedFrequencies.size() < 2) {
		throw std::invalid_argument("chiSquareGoodnessOfFit requires observed and expected vectors with equal size >= 2");
	}
	if (observedFrequencies.size() <= static_cast<std::size_t>(estimatedParameters + 1U)) {
		throw std::invalid_argument("chiSquareGoodnessOfFit requires positive degrees of freedom");
	}

	double testStat = 0.0;
	double observedTotal = 0.0;
	double expectedTotal = 0.0;
	for (std::size_t i = 0; i < observedFrequencies.size(); ++i) {
		const double observed = observedFrequencies[i];
		const double expected = expectedFrequencies[i];
		if (observed < 0.0 || expected <= 0.0 || !std::isfinite(observed) || !std::isfinite(expected)) {
			throw std::invalid_argument("chiSquareGoodnessOfFit requires finite observed >= 0 and expected > 0 frequencies");
		}
		const double diff = observed - expected;
		testStat += (diff * diff) / expected;
		observedTotal += observed;
		expectedTotal += expected;
	}

	const double alpha = 1.0 - confidenceLevel;
	const double dof = static_cast<double>(observedFrequencies.size() - 1U - estimatedParameters);
	const double cdf = chi2CdfByIntegration(testStat, dof);
	const double pValue = clampProbability(1.0 - cdf);
	const double acceptSupLim = ProbabilityDistribution::inverseChi2(1.0 - alpha, dof);

	HypothesisTester_if::TestResult::GoodnessOfFitDetails details;
	details.available = true;
	details.initialClasses = initialClasses;
	details.effectiveClasses = observedFrequencies.size();
	details.estimatedParameters = estimatedParameters;
	details.degreesOfFreedom = dof;
	details.observedTotal = observedTotal;
	details.expectedTotal = expectedTotal;

	return HypothesisTester_if::TestResult(pValue, pValue < alpha, 0.0, acceptSupLim, testStat, details);
}

}

HypothesisTesterDefaultImpl::HypothesisTesterDefaultImpl() {
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl::averageConfidenceInterval(double avg, double stddev, unsigned int n, double confidenceLevel) {
	if (n < 2 || stddev < 0.0) {
		throw std::invalid_argument("averageConfidenceInterval requires n >= 2 and stddev >= 0");
	}
	validateConfidenceLevel(confidenceLevel);
	double correctConf = (1.0 - confidenceLevel) / 2.0;
	double critic = -ProbabilityDistribution::inverseTStudent(correctConf, 0.0, 1.0, n - 1);
	double e0 = critic * stddev / sqrt(n);
	return HypothesisTester_if::ConfidenceInterval(avg - e0, avg + e0, e0);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl::proportionConfidenceInterval(double prop, unsigned int n, double confidenceLevel) {
	if (n < 2 || prop < 0.0 || prop > 1.0) {
		throw std::invalid_argument("proportionConfidenceInterval requires n >= 2 and 0 <= prop <= 1");
	}
	validateConfidenceLevel(confidenceLevel);
	// Proportion intervals use the large-sample normal approximation adopted by
	// the requirements for this analysis tool.
	const double alpha = 1.0 - confidenceLevel;
	const double critic = ProbabilityDistribution::inverseNormal(1.0 - alpha / 2.0, 0.0, 1.0);
	double e0 = critic * sqrt(prop * (1 - prop) / n);
	return HypothesisTester_if::ConfidenceInterval(prop - e0, prop + e0, e0);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl::proportionConfidenceInterval(double prop, unsigned int n, int N, double confidenceLevel) {
	if (N <= 1 || n < 2 || n > static_cast<unsigned int> (N) || prop < 0.0 || prop > 1.0) {
		throw std::invalid_argument("proportionConfidenceInterval(population) requires N > 1, 2 <= n <= N and 0 <= prop <= 1");
	}
	validateConfidenceLevel(confidenceLevel);
	// Finite-population correction scales the same large-sample proportion
	// interval when the sampled population size is known.
	const double alpha = 1.0 - confidenceLevel;
	const double critic = ProbabilityDistribution::inverseNormal(1.0 - alpha / 2.0, 0.0, 1.0);
	double e0 = critic * sqrt(prop * (1 - prop) / n) * sqrt((static_cast<double> (N) - n) / (static_cast<double> (N) - 1.0));
	return HypothesisTester_if::ConfidenceInterval(prop - e0, prop + e0, e0);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl::varianceConfidenceInterval(double var, unsigned int n, double confidenceLevel) {
	if (n < 2 || var < 0.0) {
		throw std::invalid_argument("varianceConfidenceInterval requires n >= 2 and var >= 0");
	}
	validateConfidenceLevel(confidenceLevel);
	const double alpha = 1.0 - confidenceLevel;
	double il = (n - 1) * var / ProbabilityDistribution::inverseChi2(1.0 - alpha / 2.0, n - 1);
	double sl = (n - 1) * var / ProbabilityDistribution::inverseChi2(alpha / 2.0, n - 1);
	double e0 = (sl - il) / 2.0;
	return HypothesisTester_if::ConfidenceInterval(il, sl, e0);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl::averageDifferenceConfidenceInterval(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel) {
	if (n1 < 2 || n2 < 2 || stddev1 < 0.0 || stddev2 < 0.0) {
		throw std::invalid_argument("averageDifferenceConfidenceInterval requires n1,n2 >= 2 and stddev1,stddev2 >= 0");
	}
	validateConfidenceLevel(confidenceLevel);
	double correctConf = (1.0 - confidenceLevel) / 2.0;
	double e0;
	HypothesisTester_if::ConfidenceInterval varIC = varianceRatioConfidenceInterval(pow(stddev1, 2), n1, pow(stddev2, 2), n2, confidenceLevel);
	// Policy retained from the consolidated requirements: use pooled t when
	// the variance-ratio interval is compatible with 1.0, otherwise use Welch.
	if (varIC.inferiorLimit() <= 1.0 && varIC.superiorLimit() >= 1.0) {
		const double pooledVariance = (((n1 - 1) * stddev1 * stddev1) + ((n2 - 1) * stddev2 * stddev2)) / (n1 + n2 - 2);
		const double critic = -ProbabilityDistribution::inverseTStudent(correctConf, 0.0, 1.0, n1 + n2 - 2);
		e0 = critic * sqrt(pooledVariance * (1.0 / n1 + 1.0 / n2));
	} else {
		const double varianceTerm = (stddev1 * stddev1) / n1 + (stddev2 * stddev2) / n2;
		const double degreeFreedom = (varianceTerm * varianceTerm) /
				((pow((stddev1 * stddev1) / n1, 2) / (n1 - 1)) + (pow((stddev2 * stddev2) / n2, 2) / (n2 - 1)));
		const double critic = -ProbabilityDistribution::inverseTStudent(correctConf, 0.0, 1.0, degreeFreedom);
		e0 = critic * sqrt(varianceTerm);
	}
	return HypothesisTester_if::ConfidenceInterval(avg1 - avg2 - e0, avg1 - avg2 + e0, e0);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl::proportionDifferenceConfidenceInterval(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel) {
	// Legacy signature compatibility: avg1/avg2 are interpreted as p1/p2 and stddev1/stddev2 are intentionally unused.
	(void) stddev1;
	(void) stddev2;
	const double prop1 = avg1;
	const double prop2 = avg2;
	if (n1 < 2 || n2 < 2 || prop1 < 0.0 || prop1 > 1.0 || prop2 < 0.0 || prop2 > 1.0) {
		throw std::invalid_argument("proportionDifferenceConfidenceInterval requires n1,n2 >= 2 and 0 <= p1,p2 <= 1");
	}
	validateConfidenceLevel(confidenceLevel);
	// Use the classical CI for p1 - p2 with normal quantile z_(1-alpha/2), as required by the adopted formalism.
	const double alpha = 1.0 - confidenceLevel;
	const double critic = ProbabilityDistribution::inverseNormal(1.0 - alpha / 2.0, 0.0, 1.0);
	const double standardError = std::sqrt(prop1 * (1.0 - prop1) / n1 + prop2 * (1.0 - prop2) / n2);
	const double e0 = critic * standardError;
	return HypothesisTester_if::ConfidenceInterval((prop1 - prop2) - e0, (prop1 - prop2) + e0, e0);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl::varianceRatioConfidenceInterval(double var1, unsigned int n1, double var2, unsigned int n2, double confidenceLevel) {
	if (n1 < 2 || n2 < 2 || var1 < 0.0 || var2 <= 0.0) {
		throw std::invalid_argument("varianceRatioConfidenceInterval requires n1,n2 >= 2, var1 >= 0 and var2 > 0");
	}
	validateConfidenceLevel(confidenceLevel);
	double ratio = var1 / var2;
	const double alpha = 1.0 - confidenceLevel;
	const double d1 = n1 - 1;
	const double d2 = n2 - 1;
	double il = 1.0 / ProbabilityDistribution::inverseFFisherSnedecor(1.0 - alpha / 2.0, d1, d2);
	il *= ratio;
	double sl = ProbabilityDistribution::inverseFFisherSnedecor(1.0 - alpha / 2.0, d2, d1);
	sl *= ratio;
	double e0 = (sl - il) / 2.0;
	return HypothesisTester_if::ConfidenceInterval(il, sl, e0);
}


HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl::averageConfidenceInterval(std::string sampleDataFilename, double confidenceLevel) {
	const DatasetLoader data = loadSampleData(sampleDataFilename);
	return averageConfidenceInterval(data.mean(), data.stddev(), static_cast<unsigned int>(data.count()), confidenceLevel);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl::proportionConfidenceInterval(std::string sampleDataFilename, checkProportionFunction function, double confidenceLevel) {
	const DatasetLoader data = loadSampleData(sampleDataFilename);
	const unsigned int count = countMatches(data.data(), function);
	const double prop = static_cast<double>(count) / static_cast<double>(data.count());
	return proportionConfidenceInterval(prop, static_cast<unsigned int>(data.count()), confidenceLevel);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl::proportionConfidenceInterval(std::string sampleDataFilename, checkProportionFunction function, double N, double confidenceLevel) {
	const DatasetLoader data = loadSampleData(sampleDataFilename);
	const unsigned int count = countMatches(data.data(), function);
	const double prop = static_cast<double>(count) / static_cast<double>(data.count());
	return proportionConfidenceInterval(prop, static_cast<unsigned int>(data.count()), static_cast<int>(N), confidenceLevel);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl::varianceConfidenceInterval(std::string sampleDataFilename, double confidenceLevel) {
	const DatasetLoader data = loadSampleData(sampleDataFilename);
	return varianceConfidenceInterval(data.variance(), static_cast<unsigned int>(data.count()), confidenceLevel);
}

unsigned int HypothesisTesterDefaultImpl::estimateSampleSize(double avg, double stddev, double desiredE0, double confidenceLevel) {
	(void) avg;
	if (desiredE0 <= 0.0 || stddev < 0.0) {
		return 0;
	}
	validateConfidenceLevel(confidenceLevel);
	const double alphaHalf = (1.0 - confidenceLevel) / 2.0;
	const double z = -ProbabilityDistribution::inverseNormal(alphaHalf, 0.0, 1.0);
	const double n = std::pow((z * stddev) / desiredE0, 2.0);
	return static_cast<unsigned int> (std::ceil(std::max(n, 1.0)));
}


HypothesisTester_if::TestResult HypothesisTesterDefaultImpl::testAverage(double avg, double stddev, unsigned int n, double avgSample, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	if (n < 2 || stddev <= 0.0) {
		throw std::invalid_argument("testAverage requires n >= 2 and stddev > 0");
	}
	validateConfidenceLevel(confidenceLevel);
	double significanceLevel = (1.0 - confidenceLevel);
	double acceptInfLimit = -std::numeric_limits<double>::infinity();
	double acceptSupLimit = std::numeric_limits<double>::infinity();
	if (comp == HypothesisTester_if::H1Comparition::DIFFERENT) {
		acceptInfLimit = ProbabilityDistribution::inverseTStudent(significanceLevel / 2.0, 0.0, 1.0, n - 1);
		acceptSupLimit = ProbabilityDistribution::inverseTStudent(1.0 - significanceLevel / 2.0, 0.0, 1.0, n - 1);
	} else if (comp == HypothesisTester_if::H1Comparition::LESS_THAN) {
		acceptInfLimit = ProbabilityDistribution::inverseTStudent(significanceLevel, 0.0, 1.0, n - 1);
	} else {
		acceptSupLimit = ProbabilityDistribution::inverseTStudent(1.0 - significanceLevel, 0.0, 1.0, n - 1);
	}
	double testStat = (avgSample - avg) / (stddev / sqrt(n));
	// Use Student-t CDF for p-value consistency with the t-based acceptance
	// limits above.
	const double cdf = studentTCdf(testStat, n - 1);
	double pvalue;
	if (comp == HypothesisTester_if::H1Comparition::LESS_THAN) {
		pvalue = cdf;
	} else if (comp == HypothesisTester_if::H1Comparition::GREATER_THAN) {
		pvalue = 1.0 - cdf;
	} else {
		pvalue = 2.0 * std::min(cdf, 1.0 - cdf);
	}
	return HypothesisTester_if::TestResult(clampProbability(pvalue), pvalue < significanceLevel, acceptInfLimit, acceptSupLimit, testStat);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl::testProportion(double prop, unsigned int n, double proptest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	if (n < 2 || prop <= 0.0 || prop >= 1.0 || proptest < 0.0 || proptest > 1.0) {
		throw std::invalid_argument("testProportion requires n >= 2, 0 < prop < 1 and 0 <= proptest <= 1");
	}
	validateConfidenceLevel(confidenceLevel);
	const double significanceLevel = 1.0 - confidenceLevel;
	const double standardError = std::sqrt(prop * (1.0 - prop) / n);
	const double testStat = (proptest - prop) / standardError;
	const double cdf = normalCdf(testStat);
	double pValue;
	double acceptInfLim = -std::numeric_limits<double>::infinity();
	double acceptSupLim = std::numeric_limits<double>::infinity();
	if (comp == HypothesisTester_if::H1Comparition::LESS_THAN) {
		pValue = cdf;
		acceptInfLim = ProbabilityDistribution::inverseNormal(significanceLevel, 0.0, 1.0);
	} else if (comp == HypothesisTester_if::H1Comparition::GREATER_THAN) {
		pValue = 1.0 - cdf;
		acceptSupLim = ProbabilityDistribution::inverseNormal(1.0 - significanceLevel, 0.0, 1.0);
	} else {
		pValue = 2.0 * std::min(cdf, 1.0 - cdf);
		acceptInfLim = ProbabilityDistribution::inverseNormal(significanceLevel / 2.0, 0.0, 1.0);
		acceptSupLim = ProbabilityDistribution::inverseNormal(1.0 - significanceLevel / 2.0, 0.0, 1.0);
	}
	return HypothesisTester_if::TestResult(clampProbability(pValue), pValue < significanceLevel, acceptInfLim, acceptSupLim, testStat);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl::testVariance(double var, unsigned int n, double vartest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	if (n < 2 || var < 0.0 || vartest <= 0.0) {
		throw std::invalid_argument("testVariance requires n >= 2, var >= 0 and vartest > 0");
	}
	validateConfidenceLevel(confidenceLevel);
	const double significanceLevel = 1.0 - confidenceLevel;
	const double dof = n - 1;
	const double testStat = (dof * var) / vartest;
	// Use integrated chi-square CDF for p-value coherence with inverseChi2-based
	// acceptance limits.
	const double cdf = chi2CdfByIntegration(testStat, dof);
	double pValue;
	double acceptInfLim = -std::numeric_limits<double>::infinity();
	double acceptSupLim = std::numeric_limits<double>::infinity();
	if (comp == HypothesisTester_if::H1Comparition::LESS_THAN) {
		pValue = cdf;
		acceptInfLim = ProbabilityDistribution::inverseChi2(significanceLevel, dof);
	} else if (comp == HypothesisTester_if::H1Comparition::GREATER_THAN) {
		pValue = 1.0 - cdf;
		acceptSupLim = ProbabilityDistribution::inverseChi2(1.0 - significanceLevel, dof);
	} else {
		pValue = 2.0 * std::min(cdf, 1.0 - cdf);
		acceptInfLim = ProbabilityDistribution::inverseChi2(significanceLevel / 2.0, dof);
		acceptSupLim = ProbabilityDistribution::inverseChi2(1.0 - significanceLevel / 2.0, dof);
	}
	return HypothesisTester_if::TestResult(clampProbability(pValue), pValue < significanceLevel, acceptInfLim, acceptSupLim, testStat);
}
HypothesisTester_if::TestResult HypothesisTesterDefaultImpl::testAverage(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	if (n1 < 2 || n2 < 2 || stddev1 < 0.0 || stddev2 < 0.0) {
		throw std::invalid_argument("testAverage(2pop) requires n1,n2 >= 2 and stddev1,stddev2 >= 0");
	}
	validateConfidenceLevel(confidenceLevel);
	const double alpha = 1.0 - confidenceLevel;
	const double var1 = stddev1 * stddev1;
	const double var2 = stddev2 * stddev2;
	HypothesisTester_if::ConfidenceInterval varIC = varianceRatioConfidenceInterval(var1, n1, var2, n2, confidenceLevel);
	// Keep the same pooled-vs-Welch policy used by the corresponding confidence
	// interval to avoid contradictory two-population mean decisions.
	const bool equalVariances = (varIC.inferiorLimit() <= 1.0 && varIC.superiorLimit() >= 1.0);
	const double diff = avg1 - avg2;
	double testStat;
	double degreeFreedom;
	if (equalVariances) {
		const double pooledVariance = (((n1 - 1) * var1) + ((n2 - 1) * var2)) / (n1 + n2 - 2);
		const double denominator = std::sqrt(pooledVariance * (1.0 / n1 + 1.0 / n2));
		if (!(denominator > 0.0) || !std::isfinite(denominator)) {
			throw std::invalid_argument("testAverage(2pop) pooled denominator must be positive and finite");
		}
		testStat = diff / denominator;
		degreeFreedom = n1 + n2 - 2;
	} else {
		const double varianceTerm = var1 / n1 + var2 / n2;
		const double denominator = std::sqrt(varianceTerm);
		if (!(denominator > 0.0) || !std::isfinite(denominator)) {
			throw std::invalid_argument("testAverage(2pop) Welch denominator must be positive and finite");
		}
		testStat = diff / denominator;
		degreeFreedom = (varianceTerm * varianceTerm) /
				((std::pow(var1 / n1, 2) / (n1 - 1)) + (std::pow(var2 / n2, 2) / (n2 - 1)));
	}
	if (!(degreeFreedom > 0.0) || !std::isfinite(degreeFreedom)) {
		throw std::invalid_argument("testAverage(2pop) requires positive finite degrees of freedom");
	}

	double acceptInfLimit = -std::numeric_limits<double>::infinity();
	double acceptSupLimit = std::numeric_limits<double>::infinity();
	if (comp == HypothesisTester_if::H1Comparition::DIFFERENT) {
		acceptInfLimit = ProbabilityDistribution::inverseTStudent(alpha / 2.0, 0.0, 1.0, degreeFreedom);
		acceptSupLimit = ProbabilityDistribution::inverseTStudent(1.0 - alpha / 2.0, 0.0, 1.0, degreeFreedom);
	} else if (comp == HypothesisTester_if::H1Comparition::LESS_THAN) {
		acceptInfLimit = ProbabilityDistribution::inverseTStudent(alpha, 0.0, 1.0, degreeFreedom);
	} else {
		acceptSupLimit = ProbabilityDistribution::inverseTStudent(1.0 - alpha, 0.0, 1.0, degreeFreedom);
	}

	const double cdf = studentTCdf(testStat, degreeFreedom);
	const double pvalue = pValueFromCdf(cdf, comp);
	return HypothesisTester_if::TestResult(pvalue, pvalue < alpha, acceptInfLimit, acceptSupLimit, testStat);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl::testProportion(double prop1, unsigned int n1, double prop2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	if (n1 < 2 || n2 < 2 || prop1 < 0.0 || prop1 > 1.0 || prop2 < 0.0 || prop2 > 1.0) {
		throw std::invalid_argument("testProportion(2pop) requires n1,n2 >= 2 and 0 <= prop1,prop2 <= 1");
	}
	validateConfidenceLevel(confidenceLevel);
	const double alpha = 1.0 - confidenceLevel;
	const double x = prop1 * n1;
	const double y = prop2 * n2;
	const double pbar = (x + y) / (n1 + n2);
	const double denominator = std::sqrt(pbar * (1.0 - pbar) * (1.0 / n1 + 1.0 / n2));
	if (!(denominator > 0.0) || !std::isfinite(denominator)) {
		throw std::invalid_argument("testProportion(2pop) denominator must be positive and finite");
	}
	const double testStat = (prop1 - prop2) / denominator;
	const double cdf = normalCdf(testStat);
	const double pValue = pValueFromCdf(cdf, comp);
	double acceptInfLim = -std::numeric_limits<double>::infinity();
	double acceptSupLim = std::numeric_limits<double>::infinity();
	if (comp == HypothesisTester_if::H1Comparition::LESS_THAN) {
		acceptInfLim = ProbabilityDistribution::inverseNormal(alpha, 0.0, 1.0);
	} else if (comp == HypothesisTester_if::H1Comparition::GREATER_THAN) {
		acceptSupLim = ProbabilityDistribution::inverseNormal(1.0 - alpha, 0.0, 1.0);
	} else {
		acceptInfLim = ProbabilityDistribution::inverseNormal(alpha / 2.0, 0.0, 1.0);
		acceptSupLim = ProbabilityDistribution::inverseNormal(1.0 - alpha / 2.0, 0.0, 1.0);
	}
	return HypothesisTester_if::TestResult(pValue, pValue < alpha, acceptInfLim, acceptSupLim, testStat);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl::testVariance(double var1, unsigned int n1, double var2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	if (n1 < 2 || n2 < 2 || !(var1 > 0.0) || !(var2 > 0.0)) {
		throw std::invalid_argument("testVariance(2pop) requires n1,n2 >= 2 and var1,var2 > 0");
	}
	validateConfidenceLevel(confidenceLevel);
	const double alpha = 1.0 - confidenceLevel;
	const double d1 = n1 - 1;
	const double d2 = n2 - 1;
	const double testStat = var1 / var2;
	const double cdf = fisherSnedecorCdf(testStat, d1, d2);
	const double pValue = pValueFromCdf(cdf, comp);
	double acceptInfLim = -std::numeric_limits<double>::infinity();
	double acceptSupLim = std::numeric_limits<double>::infinity();
	if (comp == HypothesisTester_if::H1Comparition::LESS_THAN) {
		acceptInfLim = ProbabilityDistribution::inverseFFisherSnedecor(alpha, d1, d2);
	} else if (comp == HypothesisTester_if::H1Comparition::GREATER_THAN) {
		acceptSupLim = ProbabilityDistribution::inverseFFisherSnedecor(1.0 - alpha, d1, d2);
	} else {
		acceptInfLim = ProbabilityDistribution::inverseFFisherSnedecor(alpha / 2.0, d1, d2);
		acceptSupLim = ProbabilityDistribution::inverseFFisherSnedecor(1.0 - alpha / 2.0, d1, d2);
	}
	return HypothesisTester_if::TestResult(pValue, pValue < alpha, acceptInfLim, acceptSupLim, testStat);
}
HypothesisTester_if::TestResult HypothesisTesterDefaultImpl::testAverage(std::string sampleDataFilename, double avgSample, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	const DatasetLoader data = loadSampleData(sampleDataFilename);
	return testAverage(avgSample, data.stddev(), static_cast<unsigned int>(data.count()), data.mean(), confidenceLevel, comp);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl::testProportion(std::string sampleDataFilename, checkProportionFunction function, double proptest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	const DatasetLoader data = loadSampleData(sampleDataFilename);
	const unsigned int count = countMatches(data.data(), function);
	const double sampleProp = static_cast<double>(count) / static_cast<double>(data.count());
	return testProportion(proptest, static_cast<unsigned int>(data.count()), sampleProp, confidenceLevel, comp);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl::testVariance(std::string sampleDataFilename, double vartest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	const DatasetLoader data = loadSampleData(sampleDataFilename);
	return testVariance(data.variance(), static_cast<unsigned int>(data.count()), vartest, confidenceLevel, comp);
}
HypothesisTester_if::TestResult HypothesisTesterDefaultImpl::testAverage(std::string firstSampleDataFilename, std::string secondSampleDataFilename, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	const DatasetLoader first = loadSampleData(firstSampleDataFilename);
	const DatasetLoader second = loadSampleData(secondSampleDataFilename);
	return testAverage(first.mean(), first.stddev(), static_cast<unsigned int>(first.count()), second.mean(), second.stddev(), static_cast<unsigned int>(second.count()), confidenceLevel, comp);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl::testProportion(std::string firstSampleDataFilename, std::string secondSampleDataFilename, checkProportionFunction function, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	const DatasetLoader first = loadSampleData(firstSampleDataFilename);
	const DatasetLoader second = loadSampleData(secondSampleDataFilename);
	const unsigned int firstCount = countMatches(first.data(), function);
	const unsigned int secondCount = countMatches(second.data(), function);
	const double firstProp = static_cast<double>(firstCount) / static_cast<double>(first.count());
	const double secondProp = static_cast<double>(secondCount) / static_cast<double>(second.count());
	return testProportion(firstProp, static_cast<unsigned int>(first.count()), secondProp, static_cast<unsigned int>(second.count()), confidenceLevel, comp);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl::testVariance(std::string firstSampleDataFilename, std::string secondSampleDataFilename, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	const DatasetLoader first = loadSampleData(firstSampleDataFilename);
	const DatasetLoader second = loadSampleData(secondSampleDataFilename);
	return testVariance(first.variance(), static_cast<unsigned int>(first.count()), second.variance(), static_cast<unsigned int>(second.count()), confidenceLevel, comp);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl::chiSquareGoodnessOfFit(const std::vector<double>& observedFrequencies, const std::vector<double>& expectedFrequencies, unsigned int estimatedParameters, double confidenceLevel) {
	return makeChiSquareResult(observedFrequencies, expectedFrequencies, estimatedParameters, confidenceLevel, observedFrequencies.size());
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl::chiSquareGoodnessOfFit(const std::vector<double>& sample, distributionCdfFunction cdf, unsigned int estimatedParameters, double confidenceLevel, std::size_t classCount, double minExpectedFrequency) {
	validateSampleAndCdf(sample, cdf);
	const std::vector<double> boundaries = automaticClassBoundaries(sample, classCount);
	return chiSquareGoodnessOfFit(sample, cdf, boundaries, estimatedParameters, confidenceLevel, minExpectedFrequency);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl::chiSquareGoodnessOfFit(const std::vector<double>& sample, distributionCdfFunction cdf, const std::vector<double>& classBoundaries, unsigned int estimatedParameters, double confidenceLevel, double minExpectedFrequency) {
	validateConfidenceLevel(confidenceLevel);
	validateSampleAndCdf(sample, cdf);
	const std::vector<ChiSquareClass> initialClasses = buildChiSquareClasses(sample, cdf, classBoundaries);
	const std::vector<ChiSquareClass> groupedClasses = groupChiSquareClasses(initialClasses, minExpectedFrequency);

	std::vector<double> observed;
	std::vector<double> expected;
	splitChiSquareClasses(groupedClasses, &observed, &expected);
	return makeChiSquareResult(observed, expected, estimatedParameters, confidenceLevel, initialClasses.size());
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl::kolmogorovSmirnov(const std::vector<double>& sample, distributionCdfFunction cdf, double confidenceLevel) {
	validateConfidenceLevel(confidenceLevel);
	if (sample.empty()) {
		throw std::invalid_argument("kolmogorovSmirnov requires a non-empty sample");
	}
	if (!cdf) {
		throw std::invalid_argument("kolmogorovSmirnov requires a valid CDF function");
	}

	std::vector<double> sorted = sample;
	for (double value : sorted) {
		if (!std::isfinite(value)) {
			throw std::invalid_argument("kolmogorovSmirnov requires finite sample values");
		}
	}
	std::sort(sorted.begin(), sorted.end());

	const double n = static_cast<double>(sorted.size());
	double d = 0.0;
	for (std::size_t i = 0; i < sorted.size(); ++i) {
		const double theoretical = cdf(sorted[i]);
		if (theoretical < 0.0 || theoretical > 1.0 || !std::isfinite(theoretical)) {
			throw std::invalid_argument("kolmogorovSmirnov CDF must return finite probabilities in [0,1]");
		}
		const double empiricalUpper = static_cast<double>(i + 1U) / n;
		const double empiricalLower = static_cast<double>(i) / n;
		d = std::max(d, std::max(std::fabs(empiricalUpper - theoretical), std::fabs(theoretical - empiricalLower)));
	}

	const double alpha = 1.0 - confidenceLevel;
	const double pValue = kolmogorovSmirnovPValue(d, static_cast<unsigned int>(sorted.size()));
	const double acceptSupLim = std::sqrt(-0.5 * std::log(alpha / 2.0) / n);
	return HypothesisTester_if::TestResult(pValue, pValue < alpha, 0.0, acceptSupLim, d);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl::kolmogorovSmirnov(std::string sampleDataFilename, distributionCdfFunction cdf, double confidenceLevel) {
	const DatasetLoader data = loadSampleData(sampleDataFilename);
	return kolmogorovSmirnov(data.data(), cdf, confidenceLevel);
}
