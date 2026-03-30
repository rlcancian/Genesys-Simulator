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
#include "../kernel/statistics/StatisticsDataFile_if.h"
#include "../kernel/TraitsKernel.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string_view>

namespace {

[[noreturn]] void throwUnimplemented(std::string_view methodName) {
	throw std::logic_error("Method not implemented yet: " + std::string(methodName));
}

double clampProbability(double value) {
	return std::clamp(value, 0.0, 1.0);
}

}

HypothesisTesterDefaultImpl1::HypothesisTesterDefaultImpl1() {
}

// confidence intervals

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl1::averageConfidenceInterval(double avg, double stddev, unsigned int n, double confidenceLevel) {
	double correctConf = (1.0 - confidenceLevel) / 2.0;
	double critic = -ProbabilityDistribution::inverseTStudent(correctConf, 0.0, 1.0, n - 1);
	double e0 = critic * stddev / sqrt(n);
	return HypothesisTester_if::ConfidenceInterval(avg - e0, avg + e0, e0);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl1::proportionConfidenceInterval(double prop, unsigned int n, double confidenceLevel) {
	double correctConf = (1.0 - confidenceLevel) / 2.0;
	double critic = -ProbabilityDistribution::inverseTStudent(correctConf, 0.0, 1.0, n - 1);
	double e0 = critic * sqrt(prop * (1 - prop) / n);
	return HypothesisTester_if::ConfidenceInterval(prop - e0, prop + e0, e0);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl1::proportionConfidenceInterval(double prop, unsigned int n, int N, double confidenceLevel) {
	double correctConf = (1.0 - confidenceLevel) / 2.0;
	double critic = -ProbabilityDistribution::inverseTStudent(correctConf, 0.0, 1.0, n - 1);
	double e0 = critic * sqrt(prop * (1 - prop) / n) * sqrt((N - n) / (N - 1));
	return HypothesisTester_if::ConfidenceInterval(prop - e0, prop + e0, e0);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl1::varianceConfidenceInterval(double var, unsigned int n, double confidenceLevel) {
	const double alpha = 1.0 - confidenceLevel;
	double il = (n - 1) * var / ProbabilityDistribution::inverseChi2(1.0 - alpha / 2.0, n - 1);
	double sl = (n - 1) * var / ProbabilityDistribution::inverseChi2(alpha / 2.0, n - 1);
	double e0 = (sl - il) / 2.0;
	return HypothesisTester_if::ConfidenceInterval(il, sl, e0);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl1::averageDifferenceConfidenceInterval(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel) {
	double correctConf = (1.0 - confidenceLevel) / 2.0;
	double e0;
	HypothesisTester_if::ConfidenceInterval varIC = varianceRatioConfidenceInterval(pow(stddev1, 2), n1, pow(stddev2, 2), n2, confidenceLevel);
	if ((varIC.inferiorLimit() <= 1.0 && varIC.superiorLimit() >= 1.0) || (varIC.inferiorLimit() >= 1.0 && varIC.superiorLimit() <= 1.0)) { // test variances ratio
		// equal variances
		e0 = -ProbabilityDistribution::tStudent(correctConf, 0.0, 1.0, n1 + n2 - 2) * sqrt(pow(stddev1, 2) * pow(stddev2, 2) * (1 / n1 + 1 / n2));
	} else { // different variances
		double degreeFreedom = pow(pow(stddev1, 2) / n1 + pow(stddev2, 2) / n2, 2) / (pow(pow(stddev1, 2) / n1, 2) / (n1 + 1) + pow(pow(stddev2, 2) / n2, 2) / (n2 + 1)) - 2;
		e0 = -ProbabilityDistribution::tStudent(correctConf, 0.0, 1.0, degreeFreedom) * sqrt(pow(stddev1, 2) / n1 + pow(stddev2, 2) / n2);
	}
	return HypothesisTester_if::ConfidenceInterval(avg1 - avg2 - e0, avg1 - avg2 + e0, e0);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl1::proportionDifferenceConfidenceInterval(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel) {
	constexpr auto epsilon = std::numeric_limits<double>::epsilon();
	const double denominator = std::sqrt((stddev1 * stddev1) / n1 + (stddev2 * stddev2) / n2);
	if (denominator <= epsilon) {
		return HypothesisTester_if::ConfidenceInterval(avg1 - avg2, avg1 - avg2, 0.0);
	}
	double correctConf = (1.0 - confidenceLevel) / 2.0;
	double critic = -ProbabilityDistribution::inverseTStudent(correctConf, 0.0, 1.0, std::min(n1, n2) - 1);
	double e0 = critic * denominator;
	return HypothesisTester_if::ConfidenceInterval(avg1 - avg2 - e0, avg1 - avg2 + e0, e0);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl1::varianceRatioConfidenceInterval(double var1, unsigned int n1, double var2, unsigned int n2, double confidenceLevel) {
	double ratio = var1 / var2;
	double il = 1 / ProbabilityDistribution::inverseFFisherSnedecor((1.0 - confidenceLevel) / 2.0, n2 - 1, n1 - 1);
	il *= ratio;
	double sl = ProbabilityDistribution::inverseFFisherSnedecor(confidenceLevel / 2.0, n1 - 1, n2 - 1);
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
	if (desiredE0 <= 0.0) {
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
	double significanceLevel = (1.0 - confidenceLevel);
	double acceptInfLimit = ProbabilityDistribution::inverseTStudent((comp == HypothesisTester_if::H1Comparition::GREATER_THAN) ? significanceLevel : significanceLevel / 2.0, 0.0, 1.0, n - 1);
	double acceptSupLimit = ProbabilityDistribution::inverseTStudent((comp == HypothesisTester_if::H1Comparition::LESS_THAN) ? confidenceLevel : 1.0 - significanceLevel / 2.0, 0.0, 1.0, n - 1);
	double testStat = (avgSample - avg) / (stddev / sqrt(n));
	const double oneTail = 0.5 * std::erfc(std::abs(testStat) / std::sqrt(2.0)); // normal approximation
	double pvalue = (comp == HypothesisTester_if::H1Comparition::DIFFERENT) ? 2.0 * oneTail : oneTail;
	return HypothesisTester_if::TestResult(clampProbability(pvalue), pvalue < significanceLevel, acceptInfLimit, acceptSupLimit, testStat);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl1::testProportion(double prop, unsigned int n, double proptest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	(void) prop;
	(void) n;
	(void) proptest;
	(void) confidenceLevel;
	(void) comp;
	throwUnimplemented(__func__);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl1::testVariance(double var, unsigned int n, double vartest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	(void) var;
	(void) n;
	(void) vartest;
	(void) confidenceLevel;
	(void) comp;
	throwUnimplemented(__func__);
}
// two populations

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl1::testAverage(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	(void) avg1;
	(void) stddev1;
	(void) n1;
	(void) avg2;
	(void) stddev2;
	(void) n2;
	(void) confidenceLevel;
	(void) comp;
	throwUnimplemented(__func__);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl1::testProportion(double prop1, unsigned int n1, double prop2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	(void) prop1;
	(void) n1;
	(void) prop2;
	(void) n2;
	(void) confidenceLevel;
	(void) comp;
	throwUnimplemented(__func__);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl1::testVariance(double var1, unsigned int n1, double var2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	(void) var1;
	(void) n1;
	(void) var2;
	(void) n2;
	(void) confidenceLevel;
	(void) comp;
	throwUnimplemented(__func__);
}
// one population based on datafile

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl1::testAverage(std::string sampleDataFilename, double avgSample, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	(void) sampleDataFilename;
	(void) avgSample;
	(void) confidenceLevel;
	(void) comp;
	throwUnimplemented(__func__);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl1::testProportion(std::string sampleDataFilename, checkProportionFunction function, double proptest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	(void) sampleDataFilename;
	(void) function;
	(void) proptest;
	(void) confidenceLevel;
	(void) comp;
	throwUnimplemented(__func__);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl1::testVariance(std::string sampleDataFilename, double vartest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	(void) sampleDataFilename;
	(void) vartest;
	(void) confidenceLevel;
	(void) comp;
	throwUnimplemented(__func__);
}
// two populations based on datafile

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl1::testAverage(std::string firstSampleDataFilename, std::string secondSampleDataFilename, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	(void) firstSampleDataFilename;
	(void) secondSampleDataFilename;
	(void) confidenceLevel;
	(void) comp;
	throwUnimplemented(__func__);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl1::testProportion(std::string firstSampleDataFilename, std::string secondSampleDataFilename, checkProportionFunction function, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	(void) firstSampleDataFilename;
	(void) secondSampleDataFilename;
	(void) function;
	(void) confidenceLevel;
	(void) comp;
	throwUnimplemented(__func__);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl1::testVariance(std::string firstSampleDataFilename, std::string secondSampleDataFilename, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	(void) firstSampleDataFilename;
	(void) secondSampleDataFilename;
	(void) confidenceLevel;
	(void) comp;
	throwUnimplemented(__func__);
}
// @TODO: Add interface for non-parametrical tests, such as chi-square (based on values and on datafile)
