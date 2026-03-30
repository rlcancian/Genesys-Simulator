/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/*
 * File:   HypothesisTesterDefaultImpl1.h
 * Author: rlcancian
 *
 * Created on 24 de novembro de 2021, 02:52
 */

#ifndef HYPOTHESISTESTERDEFAULTIMPL1_H
#define HYPOTHESISTESTERDEFAULTIMPL1_H

#include "HypothesisTester_if.h"

class HypothesisTesterDefaultImpl1 : public HypothesisTester_if {
public:
	HypothesisTesterDefaultImpl1();
	~HypothesisTesterDefaultImpl1() = default;
public:
	//
	// confidence intervals of parameters
	//
	HypothesisTester_if::ConfidenceInterval averageConfidenceInterval(double avg, double stddev, unsigned int n, double confidenceLevel) override;
	HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(double prop, unsigned int n, double confidenceLevel) override;
	HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(double prop, unsigned int n, int N, double confidenceLevel) override;
	HypothesisTester_if::ConfidenceInterval varianceConfidenceInterval(double var, unsigned int n, double confidenceLevel) override;
	// confidence intervals of differences of parameters
	HypothesisTester_if::ConfidenceInterval averageDifferenceConfidenceInterval(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel) override;
	HypothesisTester_if::ConfidenceInterval proportionDifferenceConfidenceInterval(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel) override;
	HypothesisTester_if::ConfidenceInterval varianceRatioConfidenceInterval(double var1, unsigned int n1, double var2, unsigned int n2, double confidenceLevel) override;
	// confidence intervals based on datafile
	HypothesisTester_if::ConfidenceInterval averageConfidenceInterval(std::string sampleDataFilename, double confidenceLevel) override;
	HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(std::string sampleDataFilename, checkProportionFunction function, double confidenceLevel) override;
	HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(std::string sampleDataFilename, checkProportionFunction function, double N, double confidenceLevel) override;
	HypothesisTester_if::ConfidenceInterval varianceConfidenceInterval(std::string sampleDataFilename, double confidenceLevel) override;
	// confidence intervals based on StatisticsCollector
	//virtual HypothesisTester_if::ConfidenceInterval averageConfidenceInterval(StatisticsDataFile_if* cstat, double confidenceLevel);
	//virtual HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(StatisticsDataFile_if* cstat, checkProportionFunction function, double confidenceLevel);
	//virtual HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(StatisticsDataFile_if* cstat, checkProportionFunction function, double N, double confidenceLevel);
	//virtual HypothesisTester_if::ConfidenceInterval varianceConfidenceInterval(StatisticsDataFile_if* cstat, double confidenceLevel);
	// determine sample size based on confidence interval
	unsigned int estimateSampleSize(double avg, double stddev, double desiredE0, double confidenceLevel) override;
	//
	// parametric tests
	//
	// one population
	HypothesisTester_if::TestResult testAverage(double avg, double stddev, unsigned int n, double avgSample, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;
	HypothesisTester_if::TestResult testProportion(double prop, unsigned int n, double proptest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;
	HypothesisTester_if::TestResult testVariance(double var, unsigned int n, double vartest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;
	// two populations
	HypothesisTester_if::TestResult testAverage(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;
	HypothesisTester_if::TestResult testProportion(double prop1, unsigned int n1, double prop2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;
	HypothesisTester_if::TestResult testVariance(double var1, unsigned int n1, double var2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;
	// one population based on datafile
	HypothesisTester_if::TestResult testAverage(std::string sampleDataFilename, double avgSample, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;
	HypothesisTester_if::TestResult testProportion(std::string sampleDataFilename, checkProportionFunction function, double proptest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;
	HypothesisTester_if::TestResult testVariance(std::string sampleDataFilename, double vartest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;
	// two populations based on datafile
	HypothesisTester_if::TestResult testAverage(std::string firstSampleDataFilename, std::string secondSampleDataFilename, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;
	HypothesisTester_if::TestResult testProportion(std::string firstSampleDataFilename, std::string secondSampleDataFilename, checkProportionFunction function, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;
	HypothesisTester_if::TestResult testVariance(std::string firstSampleDataFilename, std::string secondSampleDataFilename, double confidenceLevel, HypothesisTester_if::H1Comparition comp) override;
	//
	// non parametric tests
	//private:

};

#endif /* HYPOTHESISTESTERDEFAULTIMPL1_H */
