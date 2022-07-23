#ifndef HypothesisTesterDefaultImpl2_H
#define HypothesisTesterDefaultImpl2_H

#include "HypothesisTester_if.h"

class HypothesisTesterDefaultImpl2 : public HypothesisTester_if {
public:
	HypothesisTesterDefaultImpl2();
	virtual ~HypothesisTesterDefaultImpl2() = default;
public:
	//
	// confidence intervals of parameters
	//
	virtual HypothesisTester_if::ConfidenceInterval averageConfidenceInterval(double avg, double stddev, unsigned int n, double confidenceLevel);
	virtual HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(double prop, unsigned int n, double confidenceLevel);
	virtual HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(double prop, unsigned int n, int N, double confidenceLevel);
	virtual HypothesisTester_if::ConfidenceInterval varianceConfidenceInterval(double var, unsigned int n, double confidenceLevel);
	// confidence intervals of differences of parameters
	virtual HypothesisTester_if::ConfidenceInterval averageDifferenceConfidenceInterval(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel);
	virtual HypothesisTester_if::ConfidenceInterval proportionDifferenceConfidenceInterval(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel);
	virtual HypothesisTester_if::ConfidenceInterval varianceRatioConfidenceInterval(double var1, unsigned int n1, double var2, unsigned int n2, double confidenceLevel);
	// confidence intervals based on datafile
	virtual HypothesisTester_if::ConfidenceInterval averageConfidenceInterval(std::string sampleDataFilename, double confidenceLevel);
	virtual HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(std::string sampleDataFilename, checkProportionFunction function, double confidenceLevel);
	virtual HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(std::string sampleDataFilename, checkProportionFunction function, double N, double confidenceLevel);
	virtual HypothesisTester_if::ConfidenceInterval varianceConfidenceInterval(std::string sampleDataFilename, double confidenceLevel);
	// confidence intervals based on StatisticsCollector
	//virtual HypothesisTester_if::ConfidenceInterval averageConfidenceInterval(StatisticsDataFile_if* cstat, double confidenceLevel);
	//virtual HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(StatisticsDataFile_if* cstat, checkProportionFunction function, double confidenceLevel);
	//virtual HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(StatisticsDataFile_if* cstat, checkProportionFunction function, double N, double confidenceLevel);
	//virtual HypothesisTester_if::ConfidenceInterval varianceConfidenceInterval(StatisticsDataFile_if* cstat, double confidenceLevel);
	// determine sample size based on confidence interval
	virtual unsigned int estimateSampleSize(double avg, double stddev, double desiredE0, double confidenceLevel);
	//
	// parametric tests
	//
	// one population
	virtual HypothesisTester_if::TestResult testAverage(double avg, double stddev, unsigned int n, double avgSample, double confidenceLevel, HypothesisTester_if::H1Comparition comp);
	virtual HypothesisTester_if::TestResult testProportion(double prop, unsigned int n, double proptest, double confidenceLevel, HypothesisTester_if::H1Comparition comp);
	virtual HypothesisTester_if::TestResult testVariance(double var, unsigned int n, double vartest, double confidenceLevel, HypothesisTester_if::H1Comparition comp);
	// two populations
	virtual HypothesisTester_if::TestResult testAverage(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp);
	virtual HypothesisTester_if::TestResult testProportion(double prop1, unsigned int n1, double prop2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp);
	virtual HypothesisTester_if::TestResult testVariance(double var1, unsigned int n1, double var2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp);
	// one population based on datafile
	virtual HypothesisTester_if::TestResult testAverage(std::string sampleDataFilename, double avgSample, double confidenceLevel, HypothesisTester_if::H1Comparition comp);
	virtual HypothesisTester_if::TestResult testProportion(std::string sampleDataFilename, checkProportionFunction function, double proptest, double confidenceLevel, HypothesisTester_if::H1Comparition comp);
	virtual HypothesisTester_if::TestResult testVariance(std::string sampleDataFilename, double vartest, double confidenceLevel, HypothesisTester_if::H1Comparition comp);
	// two populations based on datafile
	virtual HypothesisTester_if::TestResult testAverage(std::string firstSampleDataFilename, std::string secondSampleDataFilename, double confidenceLevel, HypothesisTester_if::H1Comparition comp);
	virtual HypothesisTester_if::TestResult testProportion(std::string firstSampleDataFilename, std::string secondSampleDataFilename, checkProportionFunction function, double confidenceLevel, HypothesisTester_if::H1Comparition comp);
	virtual HypothesisTester_if::TestResult testVariance(std::string firstSampleDataFilename, std::string secondSampleDataFilename, double confidenceLevel, HypothesisTester_if::H1Comparition comp);
	//
	// non parametric tests
	//private:
private:
	HypothesisTester_if::TestResult testChiSquared(double chi, unsigned int n, double confidenceLevel, HypothesisTester_if::H1Comparition comp);
	HypothesisTester_if::TestResult testChiSquared(std::string observedSampleDataFilename, std::string expectedSampleDataFilename, double confidenceLevel, HypothesisTester_if::H1Comparition comp);
};

#endif /* HypothesisTesterDefaultImpl2_H */

