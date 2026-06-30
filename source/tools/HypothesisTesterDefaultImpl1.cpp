/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/*
 * File:   HypothesisTesterDefaultImpl1.cpp
 * Author: rlcancian
 *
 * Created on 24 de novembro de 2021, 02:52
 */

#include "HypothesisTesterDefaultImpl1.h"
#include "ProbabilityDistribution.h"
#include "SolverDefaultImpl1.h"
#include "kernel/statistics/StatisticsDataFile_if.h"
#include "kernel/TraitsKernel.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>

namespace {


double clampProbability(double value) {
	return std::clamp(value, 0.0, 1.0);
}

double normalCdf(double z) {
	return 0.5 * std::erfc(-z / std::sqrt(2.0));
}

double chi2CdfApproximation(double chi2, double degreesOfFreedom) {
	// Wilson-Hilferty approximation
	if (chi2 <= 0.0 || degreesOfFreedom <= 0.0) {
		return 0.0;
	}
	const double k = degreesOfFreedom;
	const double transformed = (std::cbrt(chi2 / k) - (1.0 - 2.0 / (9.0 * k))) / std::sqrt(2.0 / (9.0 * k));
	return clampProbability(normalCdf(transformed));
}

double chi2CdfByIntegration(double chi2, double degreesOfFreedom) {
	if (chi2 <= 0.0) {
		return 0.0;
	}
	if (degreesOfFreedom <= 0.0 || !std::isfinite(degreesOfFreedom)) {
		throw std::invalid_argument("chi2CdfByIntegration requires positive finite degreesOfFreedom");
	}
	// Use numerical integration of the chi-square PDF to keep p-values coherent with chi-square quantiles.
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

double pValueFromCdf(double cdf, HypothesisTester_if::H1Comparition comp) {
	if (comp == HypothesisTester_if::H1Comparition::LESS_THAN) {
		return clampProbability(cdf);
	}
	if (comp == HypothesisTester_if::H1Comparition::GREATER_THAN) {
		return clampProbability(1.0 - cdf);
	}
	return clampProbability(2.0 * std::min(cdf, 1.0 - cdf));
}

}

HypothesisTesterDefaultImpl1::HypothesisTesterDefaultImpl1() {
}

// confidence intervals

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl1::averageConfidenceInterval(double avg, double stddev, unsigned int n, double confidenceLevel) {
	if (n < 2 || stddev < 0.0) {
		throw std::invalid_argument("averageConfidenceInterval requires n >= 2 and stddev >= 0");
	}
	double correctConf = (1.0 - confidenceLevel) / 2.0;
	double critic = -ProbabilityDistribution::inverseTStudent(correctConf, 0.0, 1.0, n - 1);
	double e0 = critic * stddev / sqrt(n);
	return HypothesisTester_if::ConfidenceInterval(avg - e0, avg + e0, e0);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl1::proportionConfidenceInterval(double prop, unsigned int n, double confidenceLevel) {
	if (n < 2 || prop < 0.0 || prop > 1.0) {
		throw std::invalid_argument("proportionConfidenceInterval requires n >= 2 and 0 <= prop <= 1");
	}
	validateConfidenceLevel(confidenceLevel);
	// Use the large-sample normal approximation for one-population proportion CI;
	// keep the previous t-Student quantile expression commented for historical/technical traceability.
	const double alpha = 1.0 - confidenceLevel;
	const double critic = ProbabilityDistribution::inverseNormal(1.0 - alpha / 2.0, 0.0, 1.0);
	// const double critic = -ProbabilityDistribution::inverseTStudent((1.0 - confidenceLevel) / 2.0, 0.0, 1.0, n - 1);
	double e0 = critic * sqrt(prop * (1 - prop) / n);
	return HypothesisTester_if::ConfidenceInterval(prop - e0, prop + e0, e0);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl1::proportionConfidenceInterval(double prop, unsigned int n, int N, double confidenceLevel) {
	if (N <= 1 || n < 2 || n > static_cast<unsigned int> (N) || prop < 0.0 || prop > 1.0) {
		throw std::invalid_argument("proportionConfidenceInterval(population) requires N > 1, 2 <= n <= N and 0 <= prop <= 1");
	}
	validateConfidenceLevel(confidenceLevel);
	// Use the finite-population proportion CI with normal approximation + finite-population correction;
	// keep the previous t-Student quantile expression commented for historical/technical traceability.
	const double alpha = 1.0 - confidenceLevel;
	const double critic = ProbabilityDistribution::inverseNormal(1.0 - alpha / 2.0, 0.0, 1.0);
	// const double critic = -ProbabilityDistribution::inverseTStudent((1.0 - confidenceLevel) / 2.0, 0.0, 1.0, n - 1);
	double e0 = critic * sqrt(prop * (1 - prop) / n) * sqrt((static_cast<double> (N) - n) / (static_cast<double> (N) - 1.0));
	return HypothesisTester_if::ConfidenceInterval(prop - e0, prop + e0, e0);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl1::varianceConfidenceInterval(double var, unsigned int n, double confidenceLevel) {
	if (n < 2 || var < 0.0) {
		throw std::invalid_argument("varianceConfidenceInterval requires n >= 2 and var >= 0");
	}
	const double alpha = 1.0 - confidenceLevel;
	double il = (n - 1) * var / ProbabilityDistribution::inverseChi2(1.0 - alpha / 2.0, n - 1);
	double sl = (n - 1) * var / ProbabilityDistribution::inverseChi2(alpha / 2.0, n - 1);
	double e0 = (sl - il) / 2.0;
	return HypothesisTester_if::ConfidenceInterval(il, sl, e0);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl1::averageDifferenceConfidenceInterval(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel) {
	if (n1 < 2 || n2 < 2 || stddev1 < 0.0 || stddev2 < 0.0) {
		throw std::invalid_argument("averageDifferenceConfidenceInterval requires n1,n2 >= 2 and stddev1,stddev2 >= 0");
	}
	double correctConf = (1.0 - confidenceLevel) / 2.0;
	double e0;
	HypothesisTester_if::ConfidenceInterval varIC = varianceRatioConfidenceInterval(pow(stddev1, 2), n1, pow(stddev2, 2), n2, confidenceLevel);
	// @TODO: Equal-variance selection is currently indirect, based on whether the variance-ratio CI contains 1.0; revisit with an explicit pooled-vs-Welch policy.
	if ((varIC.inferiorLimit() <= 1.0 && varIC.superiorLimit() >= 1.0) || (varIC.inferiorLimit() >= 1.0 && varIC.superiorLimit() <= 1.0)) { // test variances ratio
		// equal variances
		const double pooledVariance = (((n1 - 1) * stddev1 * stddev1) + ((n2 - 1) * stddev2 * stddev2)) / (n1 + n2 - 2);
		const double critic = -ProbabilityDistribution::inverseTStudent(correctConf, 0.0, 1.0, n1 + n2 - 2);
		e0 = critic * sqrt(pooledVariance * (1.0 / n1 + 1.0 / n2));
	} else { // different variances
		const double varianceTerm = (stddev1 * stddev1) / n1 + (stddev2 * stddev2) / n2;
		const double degreeFreedom = (varianceTerm * varianceTerm) /
				((pow((stddev1 * stddev1) / n1, 2) / (n1 - 1)) + (pow((stddev2 * stddev2) / n2, 2) / (n2 - 1)));
		const double critic = -ProbabilityDistribution::inverseTStudent(correctConf, 0.0, 1.0, degreeFreedom);
		e0 = critic * sqrt(varianceTerm);
	}
	return HypothesisTester_if::ConfidenceInterval(avg1 - avg2 - e0, avg1 - avg2 + e0, e0);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl1::proportionDifferenceConfidenceInterval(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel) {
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

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl1::varianceRatioConfidenceInterval(double var1, unsigned int n1, double var2, unsigned int n2, double confidenceLevel) {
	if (n1 < 2 || n2 < 2 || var1 < 0.0 || var2 <= 0.0) {
		throw std::invalid_argument("varianceRatioConfidenceInterval requires n1,n2 >= 2, var1 >= 0 and var2 > 0");
	}
	double ratio = var1 / var2;
	const double alpha = 1.0 - confidenceLevel;
	double il = 1 / ProbabilityDistribution::inverseFFisherSnedecor(1.0 - alpha / 2.0, n2 - 1, n1 - 1);
	il *= ratio;
	double sl = ProbabilityDistribution::inverseFFisherSnedecor(1.0 - alpha / 2.0, n1 - 1, n2 - 1);
	sl *= ratio;
	double e0 = (sl - il) / 2.0;
	return HypothesisTester_if::ConfidenceInterval(il, sl, e0);
}


// confidence intervals based on datafile

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl1::averageConfidenceInterval(std::string sampleDataFilename, double confidenceLevel) {
	auto stat = std::make_unique<TraitsKernel<StatisticsDatafile_if>::Implementation>();
	static_cast<CollectorDatafile_if*> (stat->getCollector())->setDataFilename(sampleDataFilename);
	return averageConfidenceInterval(stat->average(), stat->stddeviation(), stat->numElements(), confidenceLevel);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl1::proportionConfidenceInterval(std::string sampleDataFilename, checkProportionFunction function, double confidenceLevel) {
	auto stat = std::make_unique<TraitsKernel<StatisticsDatafile_if>::Implementation>();
	static_cast<CollectorDatafile_if*> (stat->getCollector())->setDataFilename(sampleDataFilename);
	unsigned long count = 0;
	double value;
	for (unsigned long i = 0; i < stat->numElements(); i++) {
		value = static_cast<CollectorDatafile_if*> (stat->getCollector())->getValue(i);
		if (function(value))
			count++;
	}
	double prop = (double) count / stat->numElements();
	return proportionConfidenceInterval(prop, stat->numElements(), confidenceLevel);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl1::proportionConfidenceInterval(std::string sampleDataFilename, checkProportionFunction function, double N, double confidenceLevel) {
	auto stat = std::make_unique<TraitsKernel<StatisticsDatafile_if>::Implementation>();
	static_cast<CollectorDatafile_if*> (stat->getCollector())->setDataFilename(sampleDataFilename);
	unsigned long count = 0;
	for (unsigned long i = 0; i < stat->numElements(); i++) {
		const double value = static_cast<CollectorDatafile_if*> (stat->getCollector())->getValue(i);
		if (function(value)) {
			++count;
		}
	}
	double prop = static_cast<double> (count) / stat->numElements();
	return proportionConfidenceInterval(prop, stat->numElements(), static_cast<int> (N), confidenceLevel);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl1::varianceConfidenceInterval(std::string sampleDataFilename, double confidenceLevel) {
	auto stat = std::make_unique<TraitsKernel<StatisticsDatafile_if>::Implementation>();
	static_cast<CollectorDatafile_if*> (stat->getCollector())->setDataFilename(sampleDataFilename);
	return varianceConfidenceInterval(stat->variance(), stat->numElements(), confidenceLevel);
}

// estimate sample size

unsigned int HypothesisTesterDefaultImpl1::estimateSampleSize(double avg, double stddev, double desiredE0, double confidenceLevel) {
	(void) avg;
	if (desiredE0 <= 0.0 || stddev < 0.0) {
		return 0;
	}
	const double alphaHalf = (1.0 - confidenceLevel) / 2.0;
	const double z = -ProbabilityDistribution::inverseNormal(alphaHalf, 0.0, 1.0);
	const double n = std::pow((z * stddev) / desiredE0, 2.0);
	return static_cast<unsigned int> (std::ceil(std::max(n, 1.0)));
}


// parametric tests
// one population

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl1::testAverage(double avg, double stddev, unsigned int n, double avgSample, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	if (n < 2 || stddev <= 0.0) {
		throw std::invalid_argument("testAverage requires n >= 2 and stddev > 0");
	}
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
	// Use Student-t CDF for p-value consistency with the t-based critical limits above.
	const double cdf = studentTCdf(testStat, n - 1);
	// Historical reference: the previous implementation used a normal approximation for p-value.
	// const double cdf = normalCdf(testStat); // normal approximation
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

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl1::testProportion(double prop, unsigned int n, double proptest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	if (n < 2 || prop <= 0.0 || prop >= 1.0 || proptest < 0.0 || proptest > 1.0) {
		throw std::invalid_argument("testProportion requires n >= 2, 0 < prop < 1 and 0 <= proptest <= 1");
	}
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

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl1::testVariance(double var, unsigned int n, double vartest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	if (n < 2 || var < 0.0 || vartest <= 0.0) {
		throw std::invalid_argument("testVariance requires n >= 2, var >= 0 and vartest > 0");
	}
	const double significanceLevel = 1.0 - confidenceLevel;
	const double dof = n - 1;
	const double testStat = (dof * var) / vartest;
	// Use integrated chi-square CDF for p-value coherence with inverseChi2-based acceptance limits.
	const double cdf = chi2CdfByIntegration(testStat, dof);
	// Historical reference: Wilson-Hilferty approximation kept commented for technical traceability.
	// const double cdf = chi2CdfApproximation(testStat, dof);
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
// two populations

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl1::testAverage(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	if (n1 < 2 || n2 < 2 || stddev1 < 0.0 || stddev2 < 0.0) {
		throw std::invalid_argument("testAverage(2pop) requires n1,n2 >= 2 and stddev1,stddev2 >= 0");
	}
	validateConfidenceLevel(confidenceLevel);
	const double alpha = 1.0 - confidenceLevel;
	const double var1 = stddev1 * stddev1;
	const double var2 = stddev2 * stddev2;
	HypothesisTester_if::ConfidenceInterval varIC = varianceRatioConfidenceInterval(var1, n1, var2, n2, confidenceLevel);
	// @TODO: Equal-variance selection is currently indirect, based on whether the variance-ratio CI contains 1.0; revisit with an explicit pooled-vs-Welch policy.
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

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl1::testProportion(double prop1, unsigned int n1, double prop2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
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

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl1::testVariance(double var1, unsigned int n1, double var2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
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
// one population based on datafile

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl1::testAverage(std::string sampleDataFilename, double avgSample, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	auto stat = std::make_unique<TraitsKernel<StatisticsDatafile_if>::Implementation>();
	static_cast<CollectorDatafile_if*> (stat->getCollector())->setDataFilename(sampleDataFilename);
	return testAverage(avgSample, stat->stddeviation(), stat->numElements(), stat->average(), confidenceLevel, comp);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl1::testProportion(std::string sampleDataFilename, checkProportionFunction function, double proptest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	auto stat = std::make_unique<TraitsKernel<StatisticsDatafile_if>::Implementation>();
	static_cast<CollectorDatafile_if*> (stat->getCollector())->setDataFilename(sampleDataFilename);
	unsigned long count = 0;
	for (unsigned long i = 0; i < stat->numElements(); i++) {
		const double value = static_cast<CollectorDatafile_if*> (stat->getCollector())->getValue(i);
		if (function(value)) {
			++count;
		}
	}
	const double sampleProp = static_cast<double> (count) / stat->numElements();
	return testProportion(proptest, stat->numElements(), sampleProp, confidenceLevel, comp);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl1::testVariance(std::string sampleDataFilename, double vartest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	auto stat = std::make_unique<TraitsKernel<StatisticsDatafile_if>::Implementation>();
	static_cast<CollectorDatafile_if*> (stat->getCollector())->setDataFilename(sampleDataFilename);
	return testVariance(stat->variance(), stat->numElements(), vartest, confidenceLevel, comp);
}
// two populations based on datafile

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl1::testAverage(std::string firstSampleDataFilename, std::string secondSampleDataFilename, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	auto first = std::make_unique<TraitsKernel<StatisticsDatafile_if>::Implementation>();
	auto second = std::make_unique<TraitsKernel<StatisticsDatafile_if>::Implementation>();
	static_cast<CollectorDatafile_if*> (first->getCollector())->setDataFilename(firstSampleDataFilename);
	static_cast<CollectorDatafile_if*> (second->getCollector())->setDataFilename(secondSampleDataFilename);
	return testAverage(first->average(), first->stddeviation(), first->numElements(), second->average(), second->stddeviation(), second->numElements(), confidenceLevel, comp);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl1::testProportion(std::string firstSampleDataFilename, std::string secondSampleDataFilename, checkProportionFunction function, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	auto first = std::make_unique<TraitsKernel<StatisticsDatafile_if>::Implementation>();
	auto second = std::make_unique<TraitsKernel<StatisticsDatafile_if>::Implementation>();
	static_cast<CollectorDatafile_if*> (first->getCollector())->setDataFilename(firstSampleDataFilename);
	static_cast<CollectorDatafile_if*> (second->getCollector())->setDataFilename(secondSampleDataFilename);
	unsigned long firstCount = 0;
	for (unsigned long i = 0; i < first->numElements(); ++i) {
		const double value = static_cast<CollectorDatafile_if*> (first->getCollector())->getValue(i);
		if (function(value)) {
			++firstCount;
		}
	}
	unsigned long secondCount = 0;
	for (unsigned long i = 0; i < second->numElements(); ++i) {
		const double value = static_cast<CollectorDatafile_if*> (second->getCollector())->getValue(i);
		if (function(value)) {
			++secondCount;
		}
	}
	const double firstProp = static_cast<double> (firstCount) / first->numElements();
	const double secondProp = static_cast<double> (secondCount) / second->numElements();
	return testProportion(firstProp, first->numElements(), secondProp, second->numElements(), confidenceLevel, comp);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl1::testVariance(std::string firstSampleDataFilename, std::string secondSampleDataFilename, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	auto first = std::make_unique<TraitsKernel<StatisticsDatafile_if>::Implementation>();
	auto second = std::make_unique<TraitsKernel<StatisticsDatafile_if>::Implementation>();
	static_cast<CollectorDatafile_if*> (first->getCollector())->setDataFilename(firstSampleDataFilename);
	static_cast<CollectorDatafile_if*> (second->getCollector())->setDataFilename(secondSampleDataFilename);
	return testVariance(first->variance(), first->numElements(), second->variance(), second->numElements(), confidenceLevel, comp);
}
// @TODO: Add interface for non-parametrical tests, such as chi-square (based on values and on datafile)
