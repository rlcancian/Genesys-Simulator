#ifndef DATAANALYZERDEFAULTIMPL1_H
#define DATAANALYZERDEFAULTIMPL1_H

#include "DataAnalyzer_if.h"
#include "FitterDefaultImpl.h"
#include "HypothesisTesterDefaultImpl1.h"

#include <cstddef>
#include <vector>

class DataAnalyzerDefaultImpl1 : public DataAnalyzer_if {
public:
	DataAnalyzerDefaultImpl1() = default;
	virtual ~DataAnalyzerDefaultImpl1() = default;

public:
	// data loading
	virtual bool setDataFilename(const std::string& filename) override;
	virtual void setDataValues(const std::vector<double>& values) override;
	virtual void clearData() override;
	virtual bool loadSecondSample(const std::vector<double>& values) override;
	virtual bool loadSecondSampleFromFile(const std::string& filename) override;
	virtual std::string getLastError() const override;

	// configuration
	virtual void setConfidenceLevel(double confidenceLevel) override;
	virtual double getConfidenceLevel() override;
	virtual void setSignificanceLevel(double significanceLevel) override;
	virtual double getSignificanceLevel() override;

	// descriptive statistics
	virtual SummaryStatistics summaryStatistics() override;
	virtual double quartile(unsigned short num) override;
	virtual double decile(unsigned short num) override;
	virtual double centile(unsigned short num) override;

	// exploratory structures
	virtual HistogramData histogramStructure(unsigned int numClasses) override;
	virtual BoxplotData boxplotStatistics() override;

	// distribution fitting
	virtual FitResult fitDistribution(const std::string& name) override;
	virtual FitResult fitAll() override;
	virtual std::vector<FitResult> fitAllRanked() override;

	// goodness-of-fit tests
	virtual GoFResult chiSquareGoodnessOfFit(const std::string& distributionName, double significanceLevel) override;
	virtual GoFResult kolmogorovSmirnov(const std::string& distributionName, double significanceLevel) override;
	virtual GoFResult andersonDarling(const std::string& distributionName, double significanceLevel) override;
	virtual FitReport analyzeFit(const std::string& distributionName, double significanceLevel) override;

	// time-series analysis
	virtual std::vector<double> movingAverage(unsigned int window) override;
	virtual std::vector<double> autocorrelation(unsigned int maxLag) override;
	virtual CorrelogramData correlogram(unsigned int maxLag) override;
	virtual TrendDiagnostic detectTrend() override;
	virtual double exceedanceProbability(double x, const std::string& distributionName) override;
	virtual std::vector<double> exceedanceCurve(double xMin, double xMax,
	                                             unsigned int points,
	                                             const std::string& distributionName) override;

	// inference: one-population confidence intervals
	virtual HypothesisTester_if::ConfidenceInterval averageConfidenceInterval() override;
	virtual HypothesisTester_if::ConfidenceInterval proportionConfidenceInterval(checkProportionFunction checker) override;
	virtual HypothesisTester_if::ConfidenceInterval varianceConfidenceInterval() override;

	// inference: one-population hypothesis tests
	virtual HypothesisTester_if::TestResult testAverage(double hypothesizedMean, HypothesisTester_if::H1Comparition comp) override;
	virtual HypothesisTester_if::TestResult testProportion(double hypothesizedProp, checkProportionFunction checker, HypothesisTester_if::H1Comparition comp) override;
	virtual HypothesisTester_if::TestResult testVariance(double hypothesizedVariance, HypothesisTester_if::H1Comparition comp) override;

	// inference: two-population hypothesis tests
	virtual HypothesisTester_if::TestResult testAverageTwoSamples(HypothesisTester_if::H1Comparition comp) override;
	virtual HypothesisTester_if::TestResult testProportionTwoSamples(checkProportionFunction checker, HypothesisTester_if::H1Comparition comp) override;
	virtual HypothesisTester_if::TestResult testVarianceTwoSamples(HypothesisTester_if::H1Comparition comp) override;

private:
	void _rebuildCache();
	void _rebuildCache2();
	static double _quantile(const std::vector<double>& sorted, double p);
	double _sampleMode() const;
	double _cdfAt(double x, const FitResult& fit) const;
	static unsigned int _numParams(const std::string& distName);

	// chi-square CDF via numerical integration
	static double _chi2PdfIntegrand(double t, double halfDf, double logNorm);
	static double _chi2Cdf(double x, unsigned int df);
	static double _chi2Quantile(unsigned int df, double p);

	// KS test helpers
	static double _ksPValue(double Dn, std::size_t n);
	static double _ksQuantile(std::size_t n, double alpha);

	// Anderson-Darling test helpers
	static double _adPValue(double A2);
	static double _adQuantile(double alpha);

private:
	// primary sample cached stats
	std::vector<double> _data;
	std::vector<double> _sortedData;
	std::size_t _count = 0;
	double _sampleMean = 0.0;
	double _sampleVariance = 0.0;
	double _sampleStddev = 0.0;

	// second sample cached stats
	std::vector<double> _data2;
	std::vector<double> _sortedData2;
	std::size_t _count2 = 0;
	double _sampleMean2 = 0.0;
	double _sampleVariance2 = 0.0;
	double _sampleStddev2 = 0.0;

	double _confidenceLevel = 0.95;
	std::string _lastError;
	FitterDefaultImpl _fitter;
	HypothesisTesterDefaultImpl1 _tester;
};

#endif /* DATAANALYZERDEFAULTIMPL1_H */
