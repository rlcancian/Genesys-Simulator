/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   HypothesisTester_if.h
 * Author: rafael.luiz.cancian
 *
 * Created on 23 de Agosto de 2018, 19:04
 */

#ifndef HYPOTHESISTESTER_IF_H
#define HYPOTHESISTESTER_IF_H

#include <string>
#include "../kernel/statistics/StatisticsDataFile_if.h"

typedef bool (*checkProportionFunction)(double value);

/*!
 * Interface for parametric hypothesis tests based on a datafile or parameters.
 * All tests are suposed to be based on samples with unknown population parameters
 */
class HypothesisTester_if {
public:
	virtual ~HypothesisTester_if() = default;

	class ConfidenceInterval {
	public:

		ConfidenceInterval(double inferiorLimit, double superiorLimit, double e0) {
			if (inferiorLimit <= superiorLimit) {
				_infLim = inferiorLimit;
				_supLim = superiorLimit;
				_e0 = e0;
			} else {
				_infLim = superiorLimit;
				_supLim = inferiorLimit;
			}
		}

		double inferiorLimit() {
			return _infLim;
		}

		double superiorLimit() {
			return _supLim;
		}

		double halfWidth() {
			return _e0;
		}

	private:
		double _infLim, _supLim, _e0;
	};

	enum H1Comparition {
		DIFFERENT = 1,
		LESS_THAN = 2,
		GREATER_THAN = 3
	};

	class TestResult {
	public:

		TestResult(double pvalue, bool rejectH0, double acceptanceInferiorLimit, double acceptanceSuperiorLimit, double testStat) {
			_pvalue = pvalue;
			_rejectH0 = rejectH0;
			_acceptanceInferiorLimit = acceptanceInferiorLimit;
			_acceptanceSuperiorLimit = acceptanceSuperiorLimit;
			_testStat = testStat;
		}

		inline bool rejectH0() const {
			return _rejectH0;
		}

		inline bool acceptH0() const {
			return !_rejectH0;
		}

		inline double pValue() const {
			return _pvalue;
		}

		inline double testStat() const {
			return _testStat;
		}

		inline double acceptanceInferiorLimit() const {
			return _acceptanceInferiorLimit;
		}

		inline double acceptanceSuperiorLimit() const {
			return _acceptanceSuperiorLimit;
		}

	private:
		double _pvalue, _acceptanceInferiorLimit, _acceptanceSuperiorLimit, _testStat;
		bool _rejectH0;
	};


public:
	//
	// confidence intervals of parameters
	//
	virtual HypothesisTester_if::ConfidenceInterval averageConfidenceInterval(double avg, double stddev, unsigned int n, double confidenceLevel) = 0;
	virtual HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(double prop, unsigned int n, double confidenceLevel) = 0;
	virtual HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(double prop, unsigned int n, int N, double confidenceLevel) = 0;
	virtual HypothesisTester_if::ConfidenceInterval varianceConfidenceInterval(double var, unsigned int n, double confidenceLevel) = 0;
	// confidence intervals of differences of parameters
	virtual HypothesisTester_if::ConfidenceInterval averageDifferenceConfidenceInterval(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel) = 0;
	virtual HypothesisTester_if::ConfidenceInterval proportionDifferenceConfidenceInterval(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel) = 0;
	virtual HypothesisTester_if::ConfidenceInterval varianceRatioConfidenceInterval(double var1, unsigned int n1, double var2, unsigned int n2, double confidenceLevel) = 0;
	// confidence intervals based on datafile
	virtual HypothesisTester_if::ConfidenceInterval averageConfidenceInterval(std::string sampleDataFilename, double confidenceLevel) = 0;
	virtual HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(std::string sampleDataFilename, checkProportionFunction function, double confidenceLevel) = 0;
	virtual HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(std::string sampleDataFilename, checkProportionFunction function, double N, double confidenceLevel) = 0;
	virtual HypothesisTester_if::ConfidenceInterval varianceConfidenceInterval(std::string sampleDataFilename, double confidenceLevel) = 0;
	// confidence intervals based on StatisticsCollector
	//virtual HypothesisTester_if::ConfidenceInterval averageConfidenceInterval(StatisticsDataFile_if* cstat, double confidenceLevel) = 0;
	//virtual HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(StatisticsDataFile_if* cstat, checkProportionFunction function, double confidenceLevel) = 0;
	//virtual HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(StatisticsDataFile_if* cstat, checkProportionFunction function, double N, double confidenceLevel) = 0;
	//virtual HypothesisTester_if::ConfidenceInterval varianceConfidenceInterval(StatisticsDataFile_if* cstat, double confidenceLevel) = 0;
	// determine sample size based on confidence interval
	virtual unsigned int estimateSampleSize(double avg, double stddev, double desiredE0, double confidenceLevel) = 0;
	//
	// parametric tests
	//
	// one population
	virtual HypothesisTester_if::TestResult testAverage(double avg, double stddev, unsigned int n, double avgSample, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	virtual HypothesisTester_if::TestResult testProportion(double prop, unsigned int n, double proptest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	virtual HypothesisTester_if::TestResult testVariance(double var, unsigned int n, double vartest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	// two populations
	virtual HypothesisTester_if::TestResult testAverage(double avg1, double stddev1, unsigned int n1, double avg2, double stddev2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	virtual HypothesisTester_if::TestResult testProportion(double prop1, unsigned int n1, double prop2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	virtual HypothesisTester_if::TestResult testVariance(double var1, unsigned int n1, double var2, unsigned int n2, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	// one population based on datafile
	virtual HypothesisTester_if::TestResult testAverage(std::string sampleDataFilename, double avgSample, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	virtual HypothesisTester_if::TestResult testProportion(std::string sampleDataFilename, checkProportionFunction function, double proptest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	virtual HypothesisTester_if::TestResult testVariance(std::string sampleDataFilename, double vartest, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	// two populations based on datafile
	virtual HypothesisTester_if::TestResult testAverage(std::string firstSampleDataFilename, std::string secondSampleDataFilename, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	virtual HypothesisTester_if::TestResult testProportion(std::string firstSampleDataFilename, std::string secondSampleDataFilename, checkProportionFunction function, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	virtual HypothesisTester_if::TestResult testVariance(std::string firstSampleDataFilename, std::string secondSampleDataFilename, double confidenceLevel, HypothesisTester_if::H1Comparition comp) = 0;
	//
	// non parametric tests
	//
};

#endif /* HYPOTHESISTESTER_IF_H */
