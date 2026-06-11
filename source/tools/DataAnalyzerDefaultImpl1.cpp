#include "DataAnalyzerDefaultImpl1.h"
#include "SimulationResultsDataset.h"
#include "SolverDefaultImpl1.h"
#include "ProbabilityDistributionBase.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <numeric>
#include <vector>

// ============================================================
// Static helpers — chi-square CDF
// ============================================================

double DataAnalyzerDefaultImpl1::_chi2PdfIntegrand(double t, double halfDf, double logNorm) {
	if (t < 1e-15) {
		return 0.0;
	}
	return std::exp((halfDf - 1.0) * std::log(t) - 0.5 * t - logNorm);
}

double DataAnalyzerDefaultImpl1::_chi2Cdf(double x, unsigned int df) {
	if (x <= 0.0 || df == 0) {
		return 0.0;
	}
	const double halfDf = 0.5 * static_cast<double>(df);
	const double logNorm = halfDf * std::log(2.0) + std::lgamma(halfDf);
	// Avoid singularity at t=0 for df=1 by starting from small epsilon
	const double lo = (df == 1) ? 1e-8 : 0.0;
	SolverDefaultImpl1 solver(1e-6, 5000);
	const double cdf = solver.integrate(lo, x, _chi2PdfIntegrand, halfDf, logNorm);
	return std::max(0.0, std::min(1.0, cdf));
}

double DataAnalyzerDefaultImpl1::_chi2Quantile(unsigned int df, double p) {
	if (p <= 0.0) {
		return 0.0;
	}
	if (p >= 1.0) {
		return std::numeric_limits<double>::infinity();
	}
	// Bisection over the chi-square CDF
	double lo = 0.0;
	double hi = 2.0 * df + 10.0;
	// Expand upper bound until CDF exceeds p
	while (_chi2Cdf(hi, df) < p) {
		hi *= 2.0;
	}
	for (int i = 0; i < 100; ++i) {
		const double mid = 0.5 * (lo + hi);
		if (_chi2Cdf(mid, df) < p) {
			lo = mid;
		} else {
			hi = mid;
		}
		if (hi - lo < 1e-6) {
			break;
		}
	}
	return 0.5 * (lo + hi);
}

// ============================================================
// Static helpers — KS test
// ============================================================

double DataAnalyzerDefaultImpl1::_ksPValue(double Dn, std::size_t n) {
	if (Dn <= 0.0 || n == 0) {
		return 1.0;
	}
	const double t = std::sqrt(static_cast<double>(n)) * Dn;
	double pval = 0.0;
	for (int k = 1; k <= 200; ++k) {
		const double term = std::exp(-2.0 * k * k * t * t);
		if (k % 2 == 1) {
			pval += term;
		} else {
			pval -= term;
		}
		if (term < 1e-12) {
			break;
		}
	}
	pval *= 2.0;
	return std::max(0.0, std::min(1.0, pval));
}

double DataAnalyzerDefaultImpl1::_ksQuantile(std::size_t n, double alpha) {
	if (n == 0 || alpha <= 0.0 || alpha >= 1.0) {
		return std::numeric_limits<double>::quiet_NaN();
	}
	return std::sqrt(-0.5 * std::log(alpha / 2.0) / static_cast<double>(n));
}

// ============================================================
// Private helpers
// ============================================================

void DataAnalyzerDefaultImpl1::_rebuildCache() {
	_sortedData = _data;
	std::sort(_sortedData.begin(), _sortedData.end());
	_count = _data.size();
	if (_count == 0) {
		_sampleMean = _sampleVariance = _sampleStddev = 0.0;
		return;
	}
	const double sum = std::accumulate(_data.begin(), _data.end(), 0.0);
	_sampleMean = sum / static_cast<double>(_count);
	if (_count >= 2) {
		double sq = 0.0;
		for (double x : _data) {
			const double d = x - _sampleMean;
			sq += d * d;
		}
		_sampleVariance = sq / static_cast<double>(_count - 1U);
		_sampleStddev = std::sqrt(_sampleVariance);
	} else {
		_sampleVariance = 0.0;
		_sampleStddev = 0.0;
	}
}

void DataAnalyzerDefaultImpl1::_rebuildCache2() {
	_sortedData2 = _data2;
	std::sort(_sortedData2.begin(), _sortedData2.end());
	_count2 = _data2.size();
	if (_count2 == 0) {
		_sampleMean2 = _sampleVariance2 = _sampleStddev2 = 0.0;
		return;
	}
	const double sum = std::accumulate(_data2.begin(), _data2.end(), 0.0);
	_sampleMean2 = sum / static_cast<double>(_count2);
	if (_count2 >= 2) {
		double sq = 0.0;
		for (double x : _data2) {
			const double d = x - _sampleMean2;
			sq += d * d;
		}
		_sampleVariance2 = sq / static_cast<double>(_count2 - 1U);
		_sampleStddev2 = std::sqrt(_sampleVariance2);
	} else {
		_sampleVariance2 = 0.0;
		_sampleStddev2 = 0.0;
	}
}

double DataAnalyzerDefaultImpl1::_quantile(const std::vector<double>& sorted, double p) {
	if (sorted.empty()) {
		return std::numeric_limits<double>::quiet_NaN();
	}
	const std::size_t n = sorted.size();
	if (n == 1) {
		return sorted[0];
	}
	// Linear interpolation (R method R7)
	const double h = (static_cast<double>(n) - 1.0) * p;
	const std::size_t i = static_cast<std::size_t>(h);
	const double f = h - static_cast<double>(i);
	if (i + 1 >= n) {
		return sorted[n - 1];
	}
	return sorted[i] * (1.0 - f) + sorted[i + 1] * f;
}

double DataAnalyzerDefaultImpl1::_sampleMode() const {
	if (_sortedData.empty()) {
		return std::numeric_limits<double>::quiet_NaN();
	}
	double modeVal = _sortedData[0];
	unsigned int maxRun = 1;
	unsigned int curRun = 1;
	for (std::size_t i = 1; i < _sortedData.size(); ++i) {
		if (_sortedData[i] == _sortedData[i - 1]) {
			++curRun;
			if (curRun > maxRun) {
				maxRun = curRun;
				modeVal = _sortedData[i];
			}
		} else {
			curRun = 1;
		}
	}
	if (maxRun == 1) {
		return std::numeric_limits<double>::quiet_NaN();
	}
	return modeVal;
}

static double _betaPdfIntegrand(double t, double alpha, double beta) {
	const double eps = 1e-12;
	const double tc = std::max(eps, std::min(1.0 - eps, t));
	return ProbabilityDistributionBase::beta(tc, alpha, beta);
}

double DataAnalyzerDefaultImpl1::_cdfAt(double x, const FitResult& fit) const {
	const std::string& name = fit.distributionName;
	const double p1 = fit.param1, p2 = fit.param2, p3 = fit.param3, p4 = fit.param4;

	if (name == "uniform") {
		if (x <= p1) return 0.0;
		if (x >= p2) return 1.0;
		return (x - p1) / (p2 - p1);

	} else if (name == "triangular") {
		// p1=min, p2=mode, p3=max
		if (x <= p1) return 0.0;
		if (x >= p3) return 1.0;
		if (x <= p2) {
			return (x - p1) * (x - p1) / ((p3 - p1) * (p2 - p1));
		}
		return 1.0 - (p3 - x) * (p3 - x) / ((p3 - p1) * (p3 - p2));

	} else if (name == "normal") {
		// p1=mean, p2=stddev
		if (p2 <= 0.0) return std::numeric_limits<double>::quiet_NaN();
		const double z = (x - p1) / (p2 * std::sqrt(2.0));
		return 0.5 * std::erfc(-z);

	} else if (name == "exponential") {
		// p1=mean
		if (x < 0.0 || p1 <= 0.0) return 0.0;
		return 1.0 - std::exp(-x / p1);

	} else if (name == "erlang") {
		// p1=mean, p2=m (shape, integer)
		if (x < 0.0 || p1 <= 0.0) return 0.0;
		const long long int m = std::max(1LL, std::llround(p2));
		const double scale = p1 / static_cast<double>(m);
		if (scale <= 0.0) return std::numeric_limits<double>::quiet_NaN();
		const double z = x / scale;
		double sum = 1.0, term = 1.0;
		for (long long int k = 1; k < m; ++k) {
			term *= z / static_cast<double>(k);
			sum += term;
		}
		return std::max(0.0, std::min(1.0, 1.0 - std::exp(-z) * sum));

	} else if (name == "beta") {
		// p1=alpha, p2=beta, p3=infLimit, p4=supLimit
		if (x <= p3) return 0.0;
		if (x >= p4) return 1.0;
		if (p4 <= p3 || p1 <= 0.0 || p2 <= 0.0) return std::numeric_limits<double>::quiet_NaN();
		const double y = (x - p3) / (p4 - p3);
		if (y <= 0.0) return 0.0;
		if (y >= 1.0) return 1.0;
		SolverDefaultImpl1 solver(1e-5, 2000);
		return std::max(0.0, std::min(1.0, solver.integrate(0.0, y, _betaPdfIntegrand, p1, p2)));

	} else if (name == "weibull") {
		// p1=alpha (shape), p2=scale
		if (x < 0.0 || p1 <= 0.0 || p2 <= 0.0) return 0.0;
		return std::max(0.0, std::min(1.0, 1.0 - std::exp(-std::pow(x / p2, p1))));
	}

	return std::numeric_limits<double>::quiet_NaN();
}

unsigned int DataAnalyzerDefaultImpl1::_numParams(const std::string& distName) {
	if (distName == "uniform") return 0;
	if (distName == "exponential") return 1;
	if (distName == "normal") return 2;
	if (distName == "triangular") return 1;
	if (distName == "erlang") return 2;
	if (distName == "beta") return 2;
	if (distName == "weibull") return 2;
	return 0;
}

// ============================================================
// Data loading
// ============================================================

void DataAnalyzerDefaultImpl1::setDataFilename(const std::string& filename) {
	SimulationResultsDataset dataset;
	std::string err;
	if (SimulationResultsDatasetParser::loadFromTextFile(filename, &dataset, &err)) {
		setDataValues(dataset.values());
	} else {
		clearData();
	}
}

void DataAnalyzerDefaultImpl1::setDataValues(const std::vector<double>& values) {
	_data = values;
	_rebuildCache();
	_fitter.setDataValues(_data);
}

void DataAnalyzerDefaultImpl1::clearData() {
	_data.clear();
	_sortedData.clear();
	_count = 0;
	_sampleMean = _sampleVariance = _sampleStddev = 0.0;
	_data2.clear();
	_sortedData2.clear();
	_count2 = 0;
	_sampleMean2 = _sampleVariance2 = _sampleStddev2 = 0.0;
}

bool DataAnalyzerDefaultImpl1::loadSecondSample(const std::vector<double>& values) {
	if (values.empty()) {
		return false;
	}
	_data2 = values;
	_rebuildCache2();
	return true;
}

bool DataAnalyzerDefaultImpl1::loadSecondSampleFromFile(const std::string& filename) {
	SimulationResultsDataset dataset;
	std::string err;
	if (!SimulationResultsDatasetParser::loadFromTextFile(filename, &dataset, &err)) {
		return false;
	}
	return loadSecondSample(dataset.values());
}

// ============================================================
// Configuration
// ============================================================

void DataAnalyzerDefaultImpl1::setConfidenceLevel(double cl) {
	_confidenceLevel = cl;
}

double DataAnalyzerDefaultImpl1::getConfidenceLevel() {
	return _confidenceLevel;
}

void DataAnalyzerDefaultImpl1::setSignificanceLevel(double alpha) {
	_confidenceLevel = 1.0 - alpha;
}

double DataAnalyzerDefaultImpl1::getSignificanceLevel() {
	return 1.0 - _confidenceLevel;
}

// ============================================================
// Descriptive statistics
// ============================================================

DataAnalyzer_if::SummaryStatistics DataAnalyzerDefaultImpl1::summaryStatistics() {
	SummaryStatistics s{};
	if (_data.empty()) {
		return s;
	}
	s.n = static_cast<unsigned int>(_count);
	s.min = _sortedData.front();
	s.max = _sortedData.back();
	s.range = s.max - s.min;
	s.mean = _sampleMean;
	s.median = _quantile(_sortedData, 0.5);
	s.mode = _sampleMode();
	s.q1 = _quantile(_sortedData, 0.25);
	s.q3 = _quantile(_sortedData, 0.75);
	s.variance = _sampleVariance;
	s.stddev = _sampleStddev;
	s.cv = (_sampleMean != 0.0) ? _sampleStddev / std::abs(_sampleMean) : std::numeric_limits<double>::quiet_NaN();

	if (_count >= 2 && _sampleStddev > 0.0) {
		double m3 = 0.0, m4 = 0.0;
		for (double x : _data) {
			const double d = (x - _sampleMean) / _sampleStddev;
			const double d2 = d * d;
			m3 += d2 * d;
			m4 += d2 * d2;
		}
		s.skewness = m3 / static_cast<double>(_count);
		s.kurtosis = m4 / static_cast<double>(_count) - 3.0;
	} else {
		s.skewness = std::numeric_limits<double>::quiet_NaN();
		s.kurtosis = std::numeric_limits<double>::quiet_NaN();
	}
	return s;
}

double DataAnalyzerDefaultImpl1::quartile(unsigned short num) {
	if (_sortedData.empty() || num < 1 || num > 3) {
		return std::numeric_limits<double>::quiet_NaN();
	}
	return _quantile(_sortedData, num * 0.25);
}

double DataAnalyzerDefaultImpl1::decile(unsigned short num) {
	if (_sortedData.empty() || num < 1 || num > 9) {
		return std::numeric_limits<double>::quiet_NaN();
	}
	return _quantile(_sortedData, num * 0.10);
}

double DataAnalyzerDefaultImpl1::centile(unsigned short num) {
	if (_sortedData.empty() || num < 1 || num > 99) {
		return std::numeric_limits<double>::quiet_NaN();
	}
	return _quantile(_sortedData, num * 0.01);
}

// ============================================================
// Exploratory structures
// ============================================================

DataAnalyzer_if::HistogramData DataAnalyzerDefaultImpl1::histogramStructure(unsigned int numClasses) {
	HistogramData h{};
	if (_data.empty() || numClasses == 0) {
		return h;
	}
	h.numClasses = numClasses;
	const double lo = _sortedData.front();
	const double hi = _sortedData.back();
	const double width = (hi > lo) ? (hi - lo) / static_cast<double>(numClasses) : 1.0;

	h.lowerLimits.resize(numClasses);
	h.frequencies.resize(numClasses, 0U);
	h.relativeFrequencies.resize(numClasses, 0.0);

	for (unsigned int k = 0; k < numClasses; ++k) {
		h.lowerLimits[k] = lo + static_cast<double>(k) * width;
	}
	for (double x : _data) {
		int k = static_cast<int>((x - lo) / width);
		if (k < 0) {
			k = 0;
		}
		if (k >= static_cast<int>(numClasses)) {
			k = static_cast<int>(numClasses) - 1;
		}
		h.frequencies[static_cast<std::size_t>(k)]++;
	}
	const double n = static_cast<double>(_data.size());
	for (unsigned int k = 0; k < numClasses; ++k) {
		h.relativeFrequencies[k] = static_cast<double>(h.frequencies[k]) / n;
	}
	return h;
}

DataAnalyzer_if::BoxplotData DataAnalyzerDefaultImpl1::boxplotStatistics() {
	BoxplotData b{};
	if (_sortedData.empty()) {
		return b;
	}
	b.min = _sortedData.front();
	b.max = _sortedData.back();
	b.q1 = _quantile(_sortedData, 0.25);
	b.median = _quantile(_sortedData, 0.50);
	b.q3 = _quantile(_sortedData, 0.75);
	b.iqr = b.q3 - b.q1;
	const double fenceLow = b.q1 - 1.5 * b.iqr;
	const double fenceHigh = b.q3 + 1.5 * b.iqr;
	for (double x : _sortedData) {
		if (x < fenceLow || x > fenceHigh) {
			b.outliers.push_back(x);
		}
	}
	return b;
}

// ============================================================
// Distribution fitting
// ============================================================

DataAnalyzer_if::FitResult DataAnalyzerDefaultImpl1::fitDistribution(const std::string& name) {
	FitResult r{};
	r.distributionName = name;
	r.valid = false;
	r.param1 = r.param2 = r.param3 = r.param4 = r.param5 = std::numeric_limits<double>::quiet_NaN();
	r.sse = std::numeric_limits<double>::infinity();

	double sse = std::numeric_limits<double>::infinity();
	double p1 = r.param1, p2 = r.param2, p3 = r.param3, p4 = r.param4;

	if (name == "uniform") {
		_fitter.fitUniform(&sse, &p1, &p2);
	} else if (name == "triangular") {
		_fitter.fitTriangular(&sse, &p1, &p2, &p3);
	} else if (name == "normal") {
		_fitter.fitNormal(&sse, &p1, &p2);
	} else if (name == "exponential") {
		_fitter.fitExpo(&sse, &p1);
	} else if (name == "erlang") {
		_fitter.fitErlang(&sse, &p1, &p2);
	} else if (name == "beta") {
		_fitter.fitBeta(&sse, &p1, &p2, &p3, &p4);
	} else if (name == "weibull") {
		_fitter.fitWeibull(&sse, &p1, &p2);
	} else {
		return r;
	}

	r.sse = sse;
	r.param1 = p1;
	r.param2 = p2;
	r.param3 = p3;
	r.param4 = p4;
	r.valid = std::isfinite(sse);
	return r;
}

DataAnalyzer_if::FitResult DataAnalyzerDefaultImpl1::fitAll() {
	double sse = std::numeric_limits<double>::infinity();
	std::string bestName = "no-valid-fit";
	_fitter.fitAll(&sse, &bestName);
	if (bestName == "no-valid-fit" || bestName == "invalid-dataset") {
		FitResult r{};
		r.distributionName = bestName;
		r.sse = sse;
		r.valid = false;
		r.param1 = r.param2 = r.param3 = r.param4 = r.param5 = std::numeric_limits<double>::quiet_NaN();
		return r;
	}
	return fitDistribution(bestName);
}

// ============================================================
// Goodness-of-fit tests
// ============================================================

DataAnalyzer_if::GoFResult DataAnalyzerDefaultImpl1::chiSquareGoodnessOfFit(
		const std::string& distributionName, double significanceLevel) {
	GoFResult result{};
	result.distributionName = distributionName;
	result.significanceLevel = significanceLevel;
	result.rejectH0 = false;
	result.conclusion = "insufficient data";

	if (_count < 5) {
		return result;
	}

	// Fit the distribution to get parameters
	FitResult fit = fitDistribution(distributionName);
	if (!fit.valid) {
		result.conclusion = "fit failed for " + distributionName;
		return result;
	}

	// Sturges bins
	const unsigned int kInitial = static_cast<unsigned int>(
		std::max(5.0, 1.0 + std::ceil(std::log2(static_cast<double>(_count)))));

	const double lo = _sortedData.front();
	const double hi = _sortedData.back();
	const double width = (hi > lo) ? (hi - lo) / static_cast<double>(kInitial) : 1.0;

	// Compute observed and expected frequencies
	std::vector<double> obs(kInitial, 0.0);
	std::vector<double> exp(kInitial, 0.0);

	for (double x : _data) {
		int k = static_cast<int>((x - lo) / width);
		if (k < 0) k = 0;
		if (k >= static_cast<int>(kInitial)) k = static_cast<int>(kInitial) - 1;
		obs[static_cast<std::size_t>(k)] += 1.0;
	}

	const double n = static_cast<double>(_count);
	for (unsigned int k = 0; k < kInitial; ++k) {
		const double binLo = lo + static_cast<double>(k) * width;
		const double binHi = (k + 1 < kInitial) ? binLo + width : hi + 1e-10;
		const double pBin = _cdfAt(binHi, fit) - _cdfAt(binLo, fit);
		exp[k] = n * std::max(0.0, pBin);
	}

	// Merge bins where expected < 5 (merge forward)
	std::vector<double> mObs, mExp;
	double accObs = 0.0, accExp = 0.0;
	for (unsigned int k = 0; k < kInitial; ++k) {
		accObs += obs[k];
		accExp += exp[k];
		if (accExp >= 5.0 || k == kInitial - 1) {
			mObs.push_back(accObs);
			mExp.push_back(accExp);
			accObs = accExp = 0.0;
		}
	}
	// If the last merged bin has E < 5, merge it back into the previous one
	if (mObs.size() >= 2 && mExp.back() < 5.0) {
		const std::size_t last = mObs.size() - 1;
		mObs[last - 1] += mObs[last];
		mExp[last - 1] += mExp[last];
		mObs.pop_back();
		mExp.pop_back();
	}

	const unsigned int kMerged = static_cast<unsigned int>(mObs.size());
	if (kMerged < 2) {
		result.conclusion = "not enough bins after merging";
		return result;
	}

	// Chi-square statistic
	double chi2 = 0.0;
	for (std::size_t i = 0; i < kMerged; ++i) {
		if (mExp[i] > 0.0) {
			const double d = mObs[i] - mExp[i];
			chi2 += d * d / mExp[i];
		}
	}

	const unsigned int p = _numParams(distributionName);
	const unsigned int df = (kMerged >= 1 + p) ? kMerged - 1 - p : 1;

	const double pValue = 1.0 - _chi2Cdf(chi2, df);
	const double critVal = _chi2Quantile(df, 1.0 - significanceLevel);

	result.testStatistic = chi2;
	result.pValue = pValue;
	result.criticalValue = critVal;
	result.rejectH0 = (chi2 > critVal);
	result.conclusion = result.rejectH0
		? "reject H0: data does not follow " + distributionName
		: "fail to reject H0: data is consistent with " + distributionName;
	return result;
}

DataAnalyzer_if::GoFResult DataAnalyzerDefaultImpl1::kolmogorovSmirnov(
		const std::string& distributionName, double significanceLevel) {
	GoFResult result{};
	result.distributionName = distributionName;
	result.significanceLevel = significanceLevel;
	result.rejectH0 = false;
	result.conclusion = "insufficient data";

	if (_count < 2) {
		return result;
	}

	FitResult fit = fitDistribution(distributionName);
	if (!fit.valid) {
		result.conclusion = "fit failed for " + distributionName;
		return result;
	}

	const double n = static_cast<double>(_count);
	double Dn = 0.0;
	for (std::size_t i = 0; i < _count; ++i) {
		const double x = _sortedData[i];
		const double Fx = _cdfAt(x, fit);
		if (!std::isfinite(Fx)) {
			continue;
		}
		// Two-sided: check both empirical CDF boundaries
		const double empiricalAfter = static_cast<double>(i + 1) / n;
		const double empiricalBefore = static_cast<double>(i) / n;
		Dn = std::max(Dn, std::abs(empiricalAfter - Fx));
		Dn = std::max(Dn, std::abs(empiricalBefore - Fx));
	}

	const double pValue = _ksPValue(Dn, _count);
	const double critVal = _ksQuantile(_count, significanceLevel);

	result.testStatistic = Dn;
	result.pValue = pValue;
	result.criticalValue = critVal;
	result.rejectH0 = (Dn > critVal);
	result.conclusion = result.rejectH0
		? "reject H0: data does not follow " + distributionName
		: "fail to reject H0: data is consistent with " + distributionName;
	return result;
}

// ============================================================
// Time-series analysis
// ============================================================

std::vector<double> DataAnalyzerDefaultImpl1::movingAverage(unsigned int window) {
	if (_data.empty() || window == 0 || window > _count) {
		return {};
	}
	const std::size_t outSize = _count - window + 1;
	std::vector<double> result(outSize);
	double sum = 0.0;
	for (unsigned int i = 0; i < window; ++i) {
		sum += _data[i];
	}
	result[0] = sum / static_cast<double>(window);
	for (std::size_t i = 1; i < outSize; ++i) {
		sum += _data[i + window - 1] - _data[i - 1];
		result[i] = sum / static_cast<double>(window);
	}
	return result;
}

std::vector<double> DataAnalyzerDefaultImpl1::autocorrelation(unsigned int maxLag) {
	if (_count < 2 || maxLag == 0) {
		return {};
	}
	const unsigned int actualMax = static_cast<unsigned int>(
		std::min(static_cast<std::size_t>(maxLag), _count - 1));

	// Biased estimator: c[k] = (1/n) * sum_{i=0}^{n-k-1} (x_i - mean)(x_{i+k} - mean)
	double c0 = 0.0;
	for (double x : _data) {
		const double d = x - _sampleMean;
		c0 += d * d;
	}
	c0 /= static_cast<double>(_count);

	std::vector<double> acf(actualMax + 1);
	for (unsigned int lag = 0; lag <= actualMax; ++lag) {
		double ck = 0.0;
		for (std::size_t i = 0; i + lag < _count; ++i) {
			ck += (_data[i] - _sampleMean) * (_data[i + lag] - _sampleMean);
		}
		ck /= static_cast<double>(_count);
		acf[lag] = (c0 > 0.0) ? ck / c0 : 0.0;
	}
	return acf;
}

std::vector<double> DataAnalyzerDefaultImpl1::correlogram(unsigned int maxLag) {
	// Correlogram is the ACF by lag — same computation
	return autocorrelation(maxLag);
}

// ============================================================
// Inference: one-population confidence intervals
// ============================================================

HypothesisTester_if::ConfidenceInterval DataAnalyzerDefaultImpl1::averageConfidenceInterval() {
	return _tester.averageConfidenceInterval(
		_sampleMean, _sampleStddev,
		static_cast<unsigned int>(_count), _confidenceLevel);
}

HypothesisTester_if::ConfidenceInterval DataAnalyzerDefaultImpl1::proportionConfidenceInterval(
		checkProportionFunction checker) {
	if (_data.empty() || checker == nullptr) {
		return HypothesisTester_if::ConfidenceInterval(0.0, 0.0, 0.0);
	}
	unsigned int successes = 0;
	for (double x : _data) {
		if (checker(x)) ++successes;
	}
	const double prop = static_cast<double>(successes) / static_cast<double>(_count);
	return _tester.proportionConfidenceInterval(
		prop, static_cast<unsigned int>(_count), _confidenceLevel);
}

HypothesisTester_if::ConfidenceInterval DataAnalyzerDefaultImpl1::varianceConfidenceInterval() {
	return _tester.varianceConfidenceInterval(
		_sampleVariance, static_cast<unsigned int>(_count), _confidenceLevel);
}

// ============================================================
// Inference: one-population hypothesis tests
// ============================================================

HypothesisTester_if::TestResult DataAnalyzerDefaultImpl1::testAverage(
		double hypothesizedMean, HypothesisTester_if::H1Comparition comp) {
	return _tester.testAverage(
		hypothesizedMean, _sampleStddev,
		static_cast<unsigned int>(_count), _sampleMean, _confidenceLevel, comp);
}

HypothesisTester_if::TestResult DataAnalyzerDefaultImpl1::testProportion(
		double hypothesizedProp, checkProportionFunction checker, HypothesisTester_if::H1Comparition comp) {
	if (_data.empty() || checker == nullptr) {
		return HypothesisTester_if::TestResult(0.0, false, 0.0, 0.0, 0.0);
	}
	unsigned int successes = 0;
	for (double x : _data) {
		if (checker(x)) ++successes;
	}
	const double sampleProp = static_cast<double>(successes) / static_cast<double>(_count);
	return _tester.testProportion(
		hypothesizedProp, static_cast<unsigned int>(_count), sampleProp, _confidenceLevel, comp);
}

HypothesisTester_if::TestResult DataAnalyzerDefaultImpl1::testVariance(
		double hypothesizedVariance, HypothesisTester_if::H1Comparition comp) {
	return _tester.testVariance(
		hypothesizedVariance, static_cast<unsigned int>(_count), _sampleVariance, _confidenceLevel, comp);
}

// ============================================================
// Inference: two-population hypothesis tests
// ============================================================

HypothesisTester_if::TestResult DataAnalyzerDefaultImpl1::testAverageTwoSamples(
		HypothesisTester_if::H1Comparition comp) {
	return _tester.testAverage(
		_sampleMean, _sampleStddev, static_cast<unsigned int>(_count),
		_sampleMean2, _sampleStddev2, static_cast<unsigned int>(_count2),
		_confidenceLevel, comp);
}

HypothesisTester_if::TestResult DataAnalyzerDefaultImpl1::testProportionTwoSamples(
		checkProportionFunction checker, HypothesisTester_if::H1Comparition comp) {
	if (_data.empty() || _data2.empty() || checker == nullptr) {
		return HypothesisTester_if::TestResult(0.0, false, 0.0, 0.0, 0.0);
	}
	unsigned int s1 = 0, s2 = 0;
	for (double x : _data) { if (checker(x)) ++s1; }
	for (double x : _data2) { if (checker(x)) ++s2; }
	const double p1 = static_cast<double>(s1) / static_cast<double>(_count);
	const double p2 = static_cast<double>(s2) / static_cast<double>(_count2);
	// Delegate: use proportion difference test (stddev args hold proportion estimates)
	return _tester.testProportion(
		p1, static_cast<unsigned int>(_count),
		p2, static_cast<unsigned int>(_count2),
		_confidenceLevel, comp);
}

HypothesisTester_if::TestResult DataAnalyzerDefaultImpl1::testVarianceTwoSamples(
		HypothesisTester_if::H1Comparition comp) {
	return _tester.testVariance(
		_sampleVariance, static_cast<unsigned int>(_count),
		_sampleVariance2, static_cast<unsigned int>(_count2),
		_confidenceLevel, comp);
}
