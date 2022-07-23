#include "HypothesisTesterDefaultImpl2.h"
#include "ProbabilityDistribution.h"
#include "../kernel/statistics/StatisticsDataFile_if.h"
#include "../kernel/TraitsKernel.h"
#include <math.h>
#include <fstream>
#include "TraitsTools.h"

Solver_if* ProbabilityDistribution::integrator = new TraitsTools<Solver_if>::Implementation(TraitsTools<Solver_if>::Precision, TraitsTools<Solver_if>::MaxSteps);

HypothesisTesterDefaultImpl2::HypothesisTesterDefaultImpl2() {
}

// confidence intervals

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl2::averageConfidenceInterval(double avg, double stddev, unsigned int n, double confidenceLevel) {
	double correctConf = (1.0 - confidenceLevel) / 2.0;
	double critic =  -ProbabilityDistribution::inverseTStudent(correctConf,  0.0, 1.0, n-1);
	double e0 = critic * stddev / sqrt(n);
	return HypothesisTester_if::ConfidenceInterval(avg - e0, avg + e0, e0);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl2::proportionConfidenceInterval(double prop, unsigned int n, double confidenceLevel) {
	double correctConf = (1.0 - confidenceLevel) / 2.0;
	double critic =  -ProbabilityDistribution::inverseTStudent(correctConf,  0.0, 1.0, n-1);
	double e0 = critic * sqrt(prop*(1-prop) / n);
	return HypothesisTester_if::ConfidenceInterval(prop - e0, prop + e0, e0);
}


HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl2::proportionConfidenceInterval(double prop, unsigned int n, int N, double confidenceLevel) {
	double correctConf = (1.0 - confidenceLevel) / 2.0;
	double critic =  -ProbabilityDistribution::inverseTStudent(correctConf,  0.0, 1.0, n-1);
	double e0 = critic * sqrt(prop*(1-prop) / n) * sqrt((N-n) / (N-1));
	return HypothesisTester_if::ConfidenceInterval(prop - e0, prop + e0, e0);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl2::varianceConfidenceInterval(double var, unsigned int n, double confidenceLevel) {
	double il = (n - 1) * var / ProbabilityDistribution::inverseChi2((1.0 - confidenceLevel) / 2.0, n - 1);
	double sl = (n - 1) * var / ProbabilityDistribution::inverseChi2(confidenceLevel / 2.0, n - 1);
	double e0 = (sl - il) / 2.0;
	return HypothesisTester_if::ConfidenceInterval(il, sl, e0);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl2::averageDifferenceConfidenceInterval(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel) {
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

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl2::proportionDifferenceConfidenceInterval(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel) {

}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl2::varianceRatioConfidenceInterval(double var1, unsigned int n1, double var2, unsigned int n2, double confidenceLevel) {
	double ratio = var1 / var2;
	double il = 1 / ProbabilityDistribution::inverseFFisherSnedecor((1.0 - confidenceLevel) / 2.0, n2 - 1, n1 - 1);
	il *= ratio;
	double sl = ProbabilityDistribution::inverseFFisherSnedecor(confidenceLevel / 2.0, n1 - 1, n2 - 1);
	sl *= ratio;
	double e0 = (sl - il) / 2.0;
	return HypothesisTester_if::ConfidenceInterval(il, sl, e0);
}


// confidence intervals based on datafile

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl2::averageConfidenceInterval(std::string sampleDataFilename, double confidenceLevel) {
	StatisticsDatafile_if* stat = new TraitsKernel<StatisticsDatafile_if>::Implementation();
	static_cast<CollectorDatafile_if*> (stat->getCollector())->setDataFilename(sampleDataFilename);
	return averageConfidenceInterval(stat->average(), stat->stddeviation(), stat->numElements(), confidenceLevel);
}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl2::proportionConfidenceInterval(std::string sampleDataFilename, checkProportionFunction function, double confidenceLevel) {
	StatisticsDatafile_if* stat = new TraitsKernel<StatisticsDatafile_if>::Implementation();
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

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl2::proportionConfidenceInterval(std::string sampleDataFilename, checkProportionFunction function, double N, double confidenceLevel) {

}

HypothesisTester_if::ConfidenceInterval HypothesisTesterDefaultImpl2::varianceConfidenceInterval(std::string sampleDataFilename, double confidenceLevel) {
	StatisticsDatafile_if* stat = new TraitsKernel<StatisticsDatafile_if>::Implementation();
	static_cast<CollectorDatafile_if*> (stat->getCollector())->setDataFilename(sampleDataFilename);
	return varianceConfidenceInterval(stat->variance(), stat->numElements(), confidenceLevel);
}

// estimate sample size

unsigned int HypothesisTesterDefaultImpl2::estimateSampleSize(double avg, double stddev, double desiredE0, double confidenceLevel) {

}


// parametric tests
// one population

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl2::testAverage(double avg, double stddev, unsigned int n, double avgSample, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	double significanceLevel = (1.0 - confidenceLevel);
	double cumulative;
	bool rejectH0;
	double testStat = (avgSample - avg) / (stddev / sqrt(n));
	double pvalue = integrator->integrate(-100, testStat, &ProbabilityDistribution::tStudent, 0.0, 1.0, n - 1);
	if (comp == HypothesisTester_if::H1Comparition::LESS_THAN) {
		cumulative = significanceLevel;
		rejectH0 = pvalue > significanceLevel;
	} else if (comp == HypothesisTester_if::H1Comparition::GREATER_THAN) {
		cumulative = confidenceLevel;
		rejectH0 = pvalue < significanceLevel;
	} else {
		cumulative = confidenceLevel + significanceLevel / 2.0;
		rejectH0 = pvalue < (significanceLevel / 2.0);
	}
	double acceptInfLimit = ProbabilityDistribution::inverseTStudent(cumulative, 0.0, 1.0, n - 1);
	double acceptSupLimit = ProbabilityDistribution::inverseTStudent(cumulative, 0.0, 1.0, n - 1);
	return HypothesisTester_if::TestResult(pvalue, rejectH0, acceptInfLimit, acceptSupLimit, testStat);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl2::testProportion(double prop, unsigned int n, double proptest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
    double significanceLevel = (1.0 - confidenceLevel);
	double cumulative;
	bool rejectH0;
    double stddev = sqrt(prop * (1 - prop) / n);
	double testStat = (proptest - prop) / stddev;
	double pvalue = integrator->integrate(-100, testStat, &ProbabilityDistribution::normal, 0.0, 1.0);
	if (comp == HypothesisTester_if::H1Comparition::LESS_THAN) {
		cumulative = significanceLevel;
		rejectH0 = pvalue > significanceLevel;
	} else if (comp == HypothesisTester_if::H1Comparition::GREATER_THAN) {
		cumulative = confidenceLevel;
		rejectH0 = pvalue < significanceLevel;
	} else {
		cumulative = confidenceLevel + significanceLevel / 2.0;
		rejectH0 = pvalue < (significanceLevel / 2.0);
	}
	double acceptInfLimit = ProbabilityDistribution::inverseNormal(cumulative, 0.0, 1.0);
	double acceptSupLimit = ProbabilityDistribution::inverseNormal(cumulative, 0.0, 1.0);
	return HypothesisTester_if::TestResult(pvalue, rejectH0, acceptInfLimit, acceptSupLimit, testStat);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl2::testVariance(double var, unsigned int n, double vartest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	double significanceLevel = (1.0 - confidenceLevel);
	double cumulative;
	bool rejectH0;
	double testStat = ((n - 1) * vartest) / var;
	double pvalue = integrator->integrate(-100, testStat, &ProbabilityDistribution::chi2, n - 1);
	if (comp == HypothesisTester_if::H1Comparition::LESS_THAN) {
		cumulative = significanceLevel;
		rejectH0 = pvalue > significanceLevel;
	} else if (comp == HypothesisTester_if::H1Comparition::GREATER_THAN) {
		cumulative = confidenceLevel;
		rejectH0 = pvalue < significanceLevel;
	} else {
		cumulative = confidenceLevel + significanceLevel / 2.0;
		rejectH0 = pvalue < (significanceLevel / 2.0);
	}
	double acceptInfLimit = ProbabilityDistribution::inverseChi2(cumulative, n - 1);
	double acceptSupLimit = ProbabilityDistribution::inverseChi2(cumulative, n - 1);
	return HypothesisTester_if::TestResult(pvalue, rejectH0, acceptInfLimit, acceptSupLimit, testStat);
}

// two populations

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl2::testAverage(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	double significanceLevel = (1.0 - confidenceLevel);
	double cumulative;
	bool rejectH0;
    double sn1 = (stddev1 * stddev1) / n1;
    double sn2 = (stddev2 * stddev2) / n2;
    int df = (int) (((sn1 + sn2) * (sn1 + sn2)) / (((sn1 * sn1) / (n1 - 1)) + ((sn2 * sn2) / (n2 - 1))));
	double testStat = (avg1 - avg2) / sqrt(sn1 + sn2);
	double pvalue = integrator->integrate(-100, testStat, &ProbabilityDistribution::tStudent, 0.0, 1.0, df);
	if (comp == HypothesisTester_if::H1Comparition::LESS_THAN) {
		cumulative = significanceLevel;
		rejectH0 = pvalue > significanceLevel;
	} else if (comp == HypothesisTester_if::H1Comparition::GREATER_THAN) {
		cumulative = confidenceLevel;
		rejectH0 = pvalue < significanceLevel;
	} else {
		cumulative = confidenceLevel + significanceLevel / 2.0;
		rejectH0 = pvalue < (significanceLevel / 2.0);
	}
	double acceptInfLimit = ProbabilityDistribution::inverseTStudent(cumulative, 0.0, 1.0, df);
	double acceptSupLimit = ProbabilityDistribution::inverseTStudent(cumulative, 0.0, 1.0, df);
	return HypothesisTester_if::TestResult(pvalue, rejectH0, acceptInfLimit, acceptSupLimit, testStat);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl2::testProportion(double prop1, unsigned int n1, double prop2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	double significanceLevel = (1.0 - confidenceLevel);
	double cumulative;
	bool rejectH0;
    double p = ((prop1 * n1) + (prop2 * n2)) / (n1 + n2);
	double e = ((1 / (n1 * 1.0)) + (1 / (n2 * 1.0)));
	double se = sqrt(p * (1 - p) * e);
	double testStat = (prop1 - prop2) / se;
	double pvalue = integrator->integrate(-100, testStat, &ProbabilityDistribution::normal, 0.0, 1.0);
	if (comp == HypothesisTester_if::H1Comparition::LESS_THAN) {
		cumulative = significanceLevel;
		rejectH0 = pvalue > significanceLevel;
	} else if (comp == HypothesisTester_if::H1Comparition::GREATER_THAN) {
		cumulative = confidenceLevel;
		rejectH0 = pvalue < significanceLevel;
	} else {
		cumulative = confidenceLevel + significanceLevel / 2.0;
		rejectH0 = pvalue < (significanceLevel / 2.0);
	}
	double acceptInfLimit = ProbabilityDistribution::inverseNormal(cumulative, 0.0, 1.0);
	double acceptSupLimit = ProbabilityDistribution::inverseNormal(cumulative, 0.0, 1.0);
	return HypothesisTester_if::TestResult(pvalue, rejectH0, acceptInfLimit, acceptSupLimit, testStat);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl2::testVariance(double var1, unsigned int n1, double var2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	double significanceLevel = (1.0 - confidenceLevel);
	double cumulative;
	bool rejectH0;
	double testStat = var1 / var2;
	double pvalue = integrator->integrate(-100, testStat, &ProbabilityDistribution::fisherSnedecor, n1, n2);
	if (comp == HypothesisTester_if::H1Comparition::LESS_THAN) {
		cumulative = significanceLevel;
		rejectH0 = pvalue > significanceLevel;
	} else if (comp == HypothesisTester_if::H1Comparition::GREATER_THAN) {
		cumulative = confidenceLevel;
		rejectH0 = pvalue < significanceLevel;
	} else {
		cumulative = confidenceLevel + significanceLevel / 2.0;
		rejectH0 = pvalue < (significanceLevel / 2.0);
	}
	double acceptInfLimit = ProbabilityDistribution::inverseFFisherSnedecor(cumulative, n1, n2);
	double acceptSupLimit = ProbabilityDistribution::inverseFFisherSnedecor(cumulative, n1, n2);
	return HypothesisTester_if::TestResult(pvalue, rejectH0, acceptInfLimit, acceptSupLimit, testStat);
}

// one population based on datafile

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl2::testAverage(std::string sampleDataFilename, double avgSample, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	StatisticsDatafile_if* stat = new TraitsKernel<StatisticsDatafile_if>::Implementation();
	static_cast<CollectorDatafile_if*> (stat->getCollector())->setDataFilename(sampleDataFilename);
	return HypothesisTesterDefaultImpl2::testAverage(stat->average(), stat->stddeviation(), stat->newSampleSize(false), avgSample, confidenceLevel, comp);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl2::testProportion(std::string sampleDataFilename, checkProportionFunction function, double proptest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	StatisticsDatafile_if* stat = new TraitsKernel<StatisticsDatafile_if>::Implementation();
	static_cast<CollectorDatafile_if*> (stat->getCollector())->setDataFilename(sampleDataFilename);
	unsigned long count = 0;
	double value;
	for (unsigned long i = 0; i < stat->numElements(); i++) {
		value = static_cast<CollectorDatafile_if*> (stat->getCollector())->getValue(i);
		if (function(value))
			count++;
	}
	double prop = (double) count / stat->numElements();
	return HypothesisTesterDefaultImpl2::testProportion(prop, stat->newSampleSize(false), proptest, confidenceLevel, comp);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl2::testVariance(std::string sampleDataFilename, double vartest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	StatisticsDatafile_if* stat = new TraitsKernel<StatisticsDatafile_if>::Implementation();
	static_cast<CollectorDatafile_if*> (stat->getCollector())->setDataFilename(sampleDataFilename);
	return HypothesisTesterDefaultImpl2::testVariance(stat->variance(), stat->newSampleSize(false), vartest, confidenceLevel, comp);
}

// two populations based on datafile

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl2::testAverage(std::string firstSampleDataFilename, std::string secondSampleDataFilename, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	StatisticsDatafile_if* stat = new TraitsKernel<StatisticsDatafile_if>::Implementation();
	static_cast<CollectorDatafile_if*> (stat->getCollector())->setDataFilename(firstSampleDataFilename);
	StatisticsDatafile_if* stat2 = new TraitsKernel<StatisticsDatafile_if>::Implementation();
	static_cast<CollectorDatafile_if*> (stat2->getCollector())->setDataFilename(secondSampleDataFilename);
	return HypothesisTesterDefaultImpl2::testAverage(stat->average(), stat->stddeviation(), stat->newSampleSize(false), stat2->average(), stat2->stddeviation(), stat2->newSampleSize(false), confidenceLevel, comp);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl2::testProportion(std::string firstSampleDataFilename, std::string secondSampleDataFilename, checkProportionFunction function, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	StatisticsDatafile_if* stat = new TraitsKernel<StatisticsDatafile_if>::Implementation();
	static_cast<CollectorDatafile_if*> (stat->getCollector())->setDataFilename(firstSampleDataFilename);
	StatisticsDatafile_if* stat2 = new TraitsKernel<StatisticsDatafile_if>::Implementation();
	static_cast<CollectorDatafile_if*> (stat2->getCollector())->setDataFilename(secondSampleDataFilename);
	unsigned long count = 0;
	double value;
	for (unsigned long i = 0; i < stat->numElements(); i++) {
		value = static_cast<CollectorDatafile_if*> (stat->getCollector())->getValue(i);
		if (function(value))
			count++;
	}
	double prop1 = (double) count / stat->numElements();
	count = 0;
	for (unsigned long i = 0; i < stat2->numElements(); i++) {
		value = static_cast<CollectorDatafile_if*> (stat2->getCollector())->getValue(i);
		if (function(value))
			count++;
	}
	double prop2 = (double) count / stat->numElements();
	return HypothesisTesterDefaultImpl2::testProportion(prop1, stat->newSampleSize(false), prop2, stat2->newSampleSize(false), confidenceLevel, comp);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl2::testVariance(std::string firstSampleDataFilename, std::string secondSampleDataFilename, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	StatisticsDatafile_if* stat = new TraitsKernel<StatisticsDatafile_if>::Implementation();
	static_cast<CollectorDatafile_if*> (stat->getCollector())->setDataFilename(firstSampleDataFilename);
	StatisticsDatafile_if* stat2 = new TraitsKernel<StatisticsDatafile_if>::Implementation();
	static_cast<CollectorDatafile_if*> (stat2->getCollector())->setDataFilename(secondSampleDataFilename);
	return HypothesisTesterDefaultImpl2::testVariance(stat->variance(), stat->newSampleSize(false), stat2->variance(), stat2->newSampleSize(false), confidenceLevel, comp);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl2::testChiSquared(double chi, unsigned int n, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	double significanceLevel = (1.0 - confidenceLevel);
	double cumulative;
	bool rejectH0;
	double testStat = chi;
	double pvalue = integrator->integrate(-100, testStat, &ProbabilityDistribution::chi2, n - 1);
	if (comp == HypothesisTester_if::H1Comparition::LESS_THAN) {
		cumulative = significanceLevel;
		rejectH0 = pvalue > significanceLevel;
	} else if (comp == HypothesisTester_if::H1Comparition::GREATER_THAN) {
		cumulative = confidenceLevel;
		rejectH0 = pvalue < significanceLevel;
	} else {
		cumulative = confidenceLevel + significanceLevel / 2.0;
		rejectH0 = pvalue < (significanceLevel / 2.0);
	}
	double acceptInfLimit = ProbabilityDistribution::inverseChi2(cumulative, n - 1);
	double acceptSupLimit = ProbabilityDistribution::inverseChi2(cumulative, n - 1);
	return HypothesisTester_if::TestResult(pvalue, rejectH0, acceptInfLimit, acceptSupLimit, testStat);
}

HypothesisTester_if::TestResult HypothesisTesterDefaultImpl2::testChiSquared(std::string observedSampleDataFilename, std::string expectedSampleDataFilename, double confidenceLevel, HypothesisTester_if::H1Comparition comp) {
	StatisticsDatafile_if* stat = new TraitsKernel<StatisticsDatafile_if>::Implementation();
	static_cast<CollectorDatafile_if*> (stat->getCollector())->setDataFilename(observedSampleDataFilename);
	StatisticsDatafile_if* stat2 = new TraitsKernel<StatisticsDatafile_if>::Implementation();
	static_cast<CollectorDatafile_if*> (stat2->getCollector())->setDataFilename(expectedSampleDataFilename);
	double chi = 0;
	double observedVal;
	double expectedVal;
	for (unsigned long i = 0; i < stat->numElements(); i++) {
		observedVal = static_cast<CollectorDatafile_if*> (stat->getCollector())->getValue(i);
		expectedVal = static_cast<CollectorDatafile_if*> (stat2->getCollector())->getValue(i);
		chi += ((observedVal - expectedVal) * (observedVal - expectedVal)) / expectedVal;
	}
	return HypothesisTesterDefaultImpl2::testChiSquared(chi, stat->newSampleSize(false), confidenceLevel, comp);
}