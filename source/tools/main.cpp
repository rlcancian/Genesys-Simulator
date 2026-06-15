#include "ContinuousDistribution_if.h"
#include "DataAnalyser_if.h"
#include "DataAnalyzer_if.h"
#include "DataAnalyzerDefaultImpl1.h"
#include "DataSet_if.h"
#include "DiscreteDistribution_if.h"
#include "Distribution_if.h"
#include "FactorialDesign/FactorialDesign.h"
#include "Fitter_if.h"
#include "HypothesisTester_if.h"
#include "OdeSolver_if.h"
#include "OdeSystem_if.h"
#include "Optimizer_if.h"
#include "Quadrature_if.h"
#include "RootFinder_if.h"
#include "Solver_if.h"
#include "TraitsTools.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string_view>
#include <vector>

namespace {

void printToolsOverview() {
	std::cout << "GenESyS tools interfaces\n"
	          << "  analysis: DataSet_if, DataAnalyser_if\n"
	          << "  fitting: Fitter_if -> TraitsTools<Fitter_if>::Implementation\n"
	          << "  hypothesis: HypothesisTester_if -> TraitsTools<HypothesisTester_if>::Implementation\n"
	          << "  distributions: Distribution_if, ContinuousDistribution_if, DiscreteDistribution_if\n"
	          << "  numerics: Solver_if, Quadrature_if, RootFinder_if, OdeSystem_if, OdeSolver_if\n"
	          << "  optimization: Optimizer_if\n"
	          << "  design: FactorialDesign\n"
	          << "  data analysis: DataAnalyzer_if -> TraitsTools<DataAnalyzer_if>::Implementation\n";
}

// ---- helpers -------------------------------------------------------

static void printSep(char c = '-', int w = 60) {
	std::cout << std::string(w, c) << "\n";
}

static void check(const char* label, bool passed) {
	std::cout << (passed ? "  [PASS] " : "  [FAIL] ") << label << "\n";
}

// ---- demo data -----------------------------------------------------

// 50 values sampled from N(5, 1): realistic normal-looking data
static const std::vector<double> kNormalData = {
	4.21, 5.83, 4.78, 6.11, 5.02, 4.55, 5.44, 6.32, 4.90, 5.17,
	3.98, 5.61, 4.42, 5.78, 6.05, 4.73, 5.29, 4.87, 5.51, 6.18,
	4.64, 5.33, 4.19, 5.96, 5.08, 4.47, 6.25, 5.15, 4.82, 5.70,
	3.91, 6.09, 5.37, 4.61, 5.88, 4.30, 5.52, 6.40, 4.75, 5.23,
	4.58, 5.67, 4.94, 5.11, 6.03, 4.36, 5.79, 5.04, 4.48, 5.90
};

// 20 values from Exp(2) (mean=2)
static const std::vector<double> kExpoData = {
	0.42, 1.83, 0.21, 3.15, 2.64, 0.78, 4.33, 1.12, 0.58, 2.97,
	0.34, 1.56, 3.82, 0.91, 2.15, 1.37, 0.67, 5.10, 0.83, 1.74
};

// Temporal data with a trend for time-series tests
static const std::vector<double> kTemporalData = {
	1.1, 1.3, 1.2, 1.5, 1.4, 1.6, 1.8, 1.7, 1.9, 2.0,
	2.2, 2.1, 2.3, 2.5, 2.4, 2.6, 2.7, 2.9, 2.8, 3.0
};

// ---- demo sections -------------------------------------------------

void demoDataLoading(DataAnalyzerDefaultImpl1& da) {
	printSep('=');
	std::cout << "1. DATA LOADING\n";
	printSep();

	// Test in-memory loading
	da.setDataValues(kNormalData);
	check("setDataValues accepted 50 values (count check via summaryStatistics)",
	      da.summaryStatistics().n == 50);

	// Write a temp RawNumeric file and test file loading
	const std::string tmpFile = "/tmp/da_test_raw.txt";
	{
		std::ofstream f(tmpFile);
		for (double v : kNormalData) {
			f << v << "\n";
		}
	}
	da.setDataFilename(tmpFile);
	check("setDataFilename loaded same 50 values from RawNumeric text file",
	      da.summaryStatistics().n == 50);

	// Test that a missing file is detected and reported
	const bool failedLoad = !da.setDataFilename("/nonexistent/path/file.txt");
	check("setDataFilename returns false for a missing file", failedLoad);
	check("getLastError is non-empty after a failed load", !da.getLastError().empty());
	da.setDataValues(kNormalData);  // restore


	// Restore in-memory data for subsequent tests
	da.setDataValues(kNormalData);

	check("clearData empties the dataset",
	      [&]() { da.clearData(); bool ok = da.summaryStatistics().n == 0; da.setDataValues(kNormalData); return ok; }());

	check("loadSecondSample stores a second vector",
	      da.loadSecondSample(kExpoData));
}

void demoDescriptiveStats(DataAnalyzerDefaultImpl1& da) {
	printSep('=');
	std::cout << "2. DESCRIPTIVE STATISTICS\n";
	printSep();

	const auto s = da.summaryStatistics();
	std::cout << std::fixed << std::setprecision(4);
	std::cout << "  n=" << s.n << "  min=" << s.min << "  max=" << s.max
	          << "  range=" << s.range << "\n"
	          << "  mean=" << s.mean << "  median=" << s.median
	          << "  mode=" << s.mode << "\n"
	          << "  q1=" << s.q1 << "  q3=" << s.q3 << "\n"
	          << "  var=" << s.variance << "  stddev=" << s.stddev
	          << "  cv=" << s.cv << "\n"
	          << "  skewness=" << s.skewness << "  kurtosis=" << s.kurtosis << "\n";

	// Mean should be close to 5.0 (our data is N(5,1))
	check("mean is in [4.5, 5.5] for N(5,1) data", s.mean >= 4.5 && s.mean <= 5.5);
	check("min < q1 < median < q3 < max (ordering)", s.min < s.q1 && s.q1 < s.median && s.median < s.q3 && s.q3 < s.max);
	check("stddev > 0", s.stddev > 0.0);

	// Quantile accessors
	const double q2 = da.quartile(2);
	check("quartile(2) == summaryStatistics().median", std::abs(q2 - s.median) < 1e-10);
	const double d5 = da.decile(5);
	check("decile(5) == quartile(2) (both are median)", std::abs(d5 - q2) < 1e-10);
	const double c50 = da.centile(50);
	check("centile(50) == decile(5)", std::abs(c50 - d5) < 1e-10);
}

void demoExploratoryStructures(DataAnalyzerDefaultImpl1& da) {
	printSep('=');
	std::cout << "3. HISTOGRAM AND BOXPLOT\n";
	printSep();

	const auto h = da.histogramStructure(8);
	std::cout << "  Histogram (8 classes):\n";
	unsigned int total = 0;
	for (unsigned int k = 0; k < h.numClasses; ++k) {
		std::cout << "    [" << std::setw(6) << std::fixed << std::setprecision(2)
		          << h.lowerLimits[k] << ", ...)  freq=" << h.frequencies[k]
		          << "  rel=" << std::setprecision(3) << h.relativeFrequencies[k] << "\n";
		total += h.frequencies[k];
	}
	check("histogram frequency sum == n", total == 50);
	check("histogram has 8 classes", h.numClasses == 8);

	double relSum = 0.0;
	for (double r : h.relativeFrequencies) relSum += r;
	check("relative frequencies sum to 1.0", std::abs(relSum - 1.0) < 1e-10);

	const auto b = da.boxplotStatistics();
	std::cout << "  Boxplot: min=" << b.min << "  q1=" << b.q1
	          << "  med=" << b.median << "  q3=" << b.q3
	          << "  max=" << b.max << "  iqr=" << b.iqr
	          << "  outliers=" << b.outliers.size() << "\n";
	check("iqr == q3 - q1", std::abs(b.iqr - (b.q3 - b.q1)) < 1e-10);
}

void demoFitting(DataAnalyzerDefaultImpl1& da) {
	printSep('=');
	std::cout << "4. DISTRIBUTION FITTING\n";
	printSep();

	const auto rNorm = da.fitDistribution("normal");
	std::cout << "  normal:  valid=" << rNorm.valid << "  sse=" << rNorm.sse
	          << "  mean=" << rNorm.param1 << "  stddev=" << rNorm.param2 << "\n";
	check("normal fit is valid", rNorm.valid);
	check("normal mean param close to 5.0", rNorm.param1 >= 4.5 && rNorm.param1 <= 5.5);

	const auto rBest = da.fitAll();
	std::cout << "  fitAll:  best=" << rBest.distributionName
	          << "  sse=" << rBest.sse << "\n";
	check("fitAll returned a valid result", rBest.valid);
	check("fitAll SSE <= normal SSE (it's the best)", rBest.sse <= rNorm.sse + 1e-9);

	// Exponential data fitting
	da.setDataValues(kExpoData);
	const auto rExpo = da.fitDistribution("exponential");
	std::cout << "  expo (on expo data): valid=" << rExpo.valid
	          << "  mean=" << rExpo.param1 << " (expected ~2)\n";
	check("expo mean param close to 2.0", rExpo.param1 >= 1.0 && rExpo.param1 <= 3.5);

	da.setDataValues(kNormalData);  // restore
}

void demoGoF(DataAnalyzerDefaultImpl1& da) {
	printSep('=');
	std::cout << "5. GOODNESS-OF-FIT TESTS\n";
	printSep();

	// Normal data should not be rejected as normal
	const auto chi2 = da.chiSquareGoodnessOfFit("normal", 0.05);
	std::cout << "  chi2 normal: stat=" << chi2.testStatistic
	          << "  pval=" << chi2.pValue
	          << "  critical=" << chi2.criticalValue
	          << "  reject=" << chi2.rejectH0 << "\n"
	          << "  -> " << chi2.conclusion << "\n";
	check("chi2 does not reject normal fit at alpha=0.05", !chi2.rejectH0);

	const auto ks = da.kolmogorovSmirnov("normal", 0.05);
	std::cout << "  KS normal:   Dn=" << ks.testStatistic
	          << "  pval=" << ks.pValue
	          << "  critical=" << ks.criticalValue
	          << "  reject=" << ks.rejectH0 << "\n"
	          << "  -> " << ks.conclusion << "\n";
	check("KS does not reject normal fit at alpha=0.05", !ks.rejectH0);
}

void demoTimeSeries(DataAnalyzerDefaultImpl1& da) {
	printSep('=');
	std::cout << "6. TIME-SERIES ANALYSIS\n";
	printSep();

	da.setDataValues(kTemporalData);

	const auto ma3 = da.movingAverage(3);
	std::cout << "  Moving average (window=3), first 5 values:";
	for (std::size_t i = 0; i < std::min<std::size_t>(5, ma3.size()); ++i) {
		std::cout << " " << std::setprecision(3) << ma3[i];
	}
	std::cout << "\n";
	check("movingAverage(3) produces n-w+1 = 18 values", ma3.size() == 18);
	// First MA value = mean of first 3 = (1.1+1.3+1.2)/3
	check("first MA value = mean of first 3 elements",
	      std::abs(ma3[0] - (1.1 + 1.3 + 1.2) / 3.0) < 1e-9);

	const auto acf = da.autocorrelation(5);
	std::cout << "  Autocorrelation lags 0-5:";
	for (double r : acf) { std::cout << " " << std::setprecision(3) << r; }
	std::cout << "\n";
	check("autocorrelation returns maxLag+1 values", acf.size() == 6);
	check("acf[0] == 1.0", std::abs(acf[0] - 1.0) < 1e-9);
	// Trending data should have positive correlation at lag 1
	check("acf[1] > 0 for trending data", acf.size() >= 2 && acf[1] > 0.5);

	const auto cgram = da.correlogram(5);
	check("correlogram returns confidence bounds", cgram.confidenceBound > 0.0);
	check("correlogram ACF matches autocorrelation", cgram.acf == acf);

	da.setDataValues(kNormalData);  // restore
}

void demoInference(DataAnalyzerDefaultImpl1& da) {
	printSep('=');
	std::cout << "7. PARAMETRIC INFERENCE\n";
	printSep();

	da.setConfidenceLevel(0.95);

	// CI for mean — should contain 5.0
	auto ci = da.averageConfidenceInterval();
	std::cout << "  95% CI for mean: [" << ci.inferiorLimit() << ", " << ci.superiorLimit() << "]\n";
	check("CI for mean contains true mean 5.0",
	      ci.inferiorLimit() <= 5.0 && ci.superiorLimit() >= 5.0);

	// H0: mu = 5.2 — close to actual sample mean (5.206), should not reject
	const auto tAvg = da.testAverage(5.2, HypothesisTester_if::DIFFERENT);
	std::cout << "  testAverage(mu0=5.2, two-sided): stat=" << tAvg.testStat()
	          << "  pval=" << tAvg.pValue()
	          << "  reject=" << tAvg.rejectH0() << "\n";
	check("testAverage does not reject mu=5.2 (close to sample mean 5.206)", !tAvg.rejectH0());

	// Hypothesis test: H0: mu = 10.0 (should reject — far from true mean)
	const auto tAvg2 = da.testAverage(10.0, HypothesisTester_if::DIFFERENT);
	check("testAverage rejects mu=10.0 (clearly wrong)", tAvg2.rejectH0());

	// Variance CI
	auto vci = da.varianceConfidenceInterval();
	std::cout << "  95% CI for variance: [" << vci.inferiorLimit() << ", " << vci.superiorLimit() << "]\n";
	check("variance CI upper > lower", vci.superiorLimit() > vci.inferiorLimit());

	// Two-sample average test (normal vs expo data — different means, should reject H0: mu1=mu2)
	da.loadSecondSample(kExpoData);
	const auto t2 = da.testAverageTwoSamples(HypothesisTester_if::DIFFERENT);
	std::cout << "  testAverageTwoSamples(normal vs expo): stat=" << t2.testStat()
	          << "  pval=" << t2.pValue() << "  reject=" << t2.rejectH0() << "\n";
	check("testAverageTwoSamples rejects H0 for normal vs expo data", t2.rejectH0());
}

void runDemo() {
	DataAnalyzerDefaultImpl1 da;

	std::cout << "\n";
	printSep('=');
	std::cout << "DataAnalyzer Demo — GenESyS tools\n";
	printSep('=');

	demoDataLoading(da);
	demoDescriptiveStats(da);
	demoExploratoryStructures(da);
	demoFitting(da);
	demoGoF(da);
	demoTimeSeries(da);
	demoInference(da);

	printSep('=');
	std::cout << "Demo complete.\n";
	printSep('=');
}

}  // namespace

int main(int argc, char* argv[]) {
	const std::string_view command = argc > 1 ? argv[1] : "--list";
	if (command == "--list" || command == "--help") {
		printToolsOverview();
		return 0;
	}
	if (command == "--demo") {
		runDemo();
		return 0;
	}

	std::cerr << "Unknown tools command: " << command << "\n";
	std::cerr << "Use --list to print the available tools interfaces.\n";
	std::cerr << "Use --demo to run the DataAnalyzer demo and tests.\n";
	return 1;
}
