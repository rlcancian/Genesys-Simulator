#include "DataAnalyzerDefaultImpl1.h"
#include "SimulationResultsDataset.h"
#include "SolverDefaultImpl1.h"
#include "ProbabilityDistributionBase.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <string>
#include <vector>

namespace {
constexpr double kEps = 1e-12;

static double normalCdf(double x, double mean, double stddev) {
	if (stddev <= 0.0) {
		return std::numeric_limits<double>::quiet_NaN();
	}
	const double z = (x - mean) / (stddev * std::sqrt(2.0));
	return 0.5 * std::erfc(-z);
}

static double betaPdfIntegrand(double t, double alpha, double beta) {
	const double tc = std::max(kEps, std::min(1.0 - kEps, t));
	return ProbabilityDistributionBase::beta(tc, alpha, beta);
}

static double finiteOrInf(double value) {
	return std::isfinite(value) ? value : std::numeric_limits<double>::infinity();
}
} // namespace

// ============================================================
// Static helpers — chi-square CDF
// ============================================================

double DataAnalyzerDefaultImpl1::_chi2PdfIntegrand(double t, double halfDf, double logNorm) {
	if (t < kEps) {
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
	const double lo = (df == 1) ? 1e-8 : 0.0;
	SolverDefaultImpl1 solver(1e-6, 5000);
	const double cdf = solver.integrate(lo, x, _chi2PdfIntegrand, halfDf, logNorm);
	return std::max(0.0, std::min(1.0, cdf));
}

double DataAnalyzerDefaultImpl1::_chi2Quantile(unsigned int df, double p) {
	if (p <= 0.0 || df == 0) {
		return 0.0;
	}
	if (p >= 1.0) {
		return std::numeric_limits<double>::infinity();
	}
	double lo = 0.0;
	double hi = 2.0 * static_cast<double>(df) + 10.0;
	while (_chi2Cdf(hi, df) < p && std::isfinite(hi)) {
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
// Static helpers — KS and Anderson-Darling
// ============================================================

double DataAnalyzerDefaultImpl1::_ksPValue(double Dn, std::size_t n) {
	if (Dn <= 0.0 || n == 0) {
		return 1.0;
	}
	const double t = std::sqrt(static_cast<double>(n)) * Dn;
	double pval = 0.0;
	for (int k = 1; k <= 200; ++k) {
		const double term = std::exp(-2.0 * static_cast<double>(k * k) * t * t);
		pval += (k % 2 == 1) ? term : -term;
		if (term < 1e-12) {
			break;
		}
	}
	return std::max(0.0, std::min(1.0, 2.0 * pval));
}

double DataAnalyzerDefaultImpl1::_ksQuantile(std::size_t n, double alpha) {
	if (n == 0 || alpha <= 0.0 || alpha >= 1.0) {
		return std::numeric_limits<double>::quiet_NaN();
	}
	return std::sqrt(-0.5 * std::log(alpha / 2.0) / static_cast<double>(n));
}

double DataAnalyzerDefaultImpl1::_adPValue(double A2) {
	if (A2 <= 0.0) return 1.0;
	double p;
	if (A2 < 0.2) {
		p = 1.0 - std::exp(-13.436 + 101.14 * A2 - 223.73 * A2 * A2);
	} else if (A2 < 0.34) {
		p = 1.0 - std::exp(-8.318 + 42.796 * A2 - 59.938 * A2 * A2);
	} else if (A2 < 0.6) {
		p = std::exp(0.9177 - 4.279 * A2 - 1.38 * A2 * A2);
	} else {
		p = std::exp(1.2937 - 5.709 * A2 + 0.0186 * A2 * A2);
	}
	return std::max(0.0, std::min(1.0, p));
}

double DataAnalyzerDefaultImpl1::_adQuantile(double alpha) {
	if (alpha <= 0.0) return std::numeric_limits<double>::infinity();
	if (alpha >= 1.0) return 0.0;
	double lo = 0.0;
	double hi = 5.0;
	while (_adPValue(hi) > alpha) {
		hi *= 2.0;
	}
	for (int i = 0; i < 100; ++i) {
		const double mid = 0.5 * (lo + hi);
		if (_adPValue(mid) > alpha) {
			lo = mid;
		} else {
			hi = mid;
		}
		if (hi - lo < 1e-7) {
			break;
		}
	}
	return 0.5 * (lo + hi);
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

double DataAnalyzerDefaultImpl1::_cdfAt(double x, const FitResult& fit) const {
	const std::string& name = fit.distributionName;
	const double p1 = fit.param1;
	const double p2 = fit.param2;
	const double p3 = fit.param3;
	const double p4 = fit.param4;

	if (name == "uniform") {
		if (p2 <= p1) return std::numeric_limits<double>::quiet_NaN();
		if (x <= p1) return 0.0;
		if (x >= p2) return 1.0;
		return (x - p1) / (p2 - p1);
	}
	if (name == "triangular") {
		if (p3 <= p1 || p2 < p1 || p2 > p3) return std::numeric_limits<double>::quiet_NaN();
		if (x <= p1) return 0.0;
		if (x >= p3) return 1.0;
		if (x <= p2) return (x - p1) * (x - p1) / ((p3 - p1) * std::max(kEps, p2 - p1));
		return 1.0 - (p3 - x) * (p3 - x) / ((p3 - p1) * std::max(kEps, p3 - p2));
	}
	if (name == "normal") {
		return normalCdf(x, p1, p2);
	}
	if (name == "exponential") {
		if (x < 0.0 || p1 <= 0.0) return 0.0;
		return 1.0 - std::exp(-x / p1);
	}
	if (name == "erlang") {
		if (x < 0.0 || p1 <= 0.0) return 0.0;
		const long long int m = std::max(1LL, std::llround(p2));
		const double scale = p1 / static_cast<double>(m);
		const double z = x / std::max(kEps, scale);
		double sum = 1.0;
		double term = 1.0;
		for (long long int k = 1; k < m; ++k) {
			term *= z / static_cast<double>(k);
			sum += term;
		}
		return std::max(0.0, std::min(1.0, 1.0 - std::exp(-z) * sum));
	}
	if (name == "beta") {
		if (x <= p3) return 0.0;
		if (x >= p4) return 1.0;
		if (p1 <= 0.0 || p2 <= 0.0 || p4 <= p3) return std::numeric_limits<double>::quiet_NaN();
		const double y = (x - p3) / (p4 - p3);
		SolverDefaultImpl1 solver(1e-5, 2000);
		return std::max(0.0, std::min(1.0, solver.integrate(0.0, y, betaPdfIntegrand, p1, p2)));
	}
	if (name == "weibull") {
		if (x < 0.0 || p1 <= 0.0 || p2 <= 0.0) return 0.0;
		return std::max(0.0, std::min(1.0, 1.0 - std::exp(-std::pow(x / p2, p1))));
	}
	return std::numeric_limits<double>::quiet_NaN();
}

unsigned int DataAnalyzerDefaultImpl1::_numParams(const std::string& distName) {
	if (distName == "uniform")     return 2;
	if (distName == "triangular")  return 3;
	if (distName == "normal")      return 2;
	if (distName == "exponential") return 1;
	if (distName == "erlang")      return 2;
	if (distName == "beta")        return 4;
	if (distName == "weibull")     return 2;
	return 0;
}

// ============================================================
// Data loading
// ============================================================

bool DataAnalyzerDefaultImpl1::setDataFilename(const std::string& filename) {
	SimulationResultsDataset dataset;
	std::string err;
	if (!SimulationResultsDatasetParser::loadFromTextFile(filename, &dataset, &err)) {
		_lastError = "setDataFilename(\"" + filename + "\"): " + err;
		clearData();
		return false;
	}
	_lastError.clear();
	setDataValues(dataset.values());
	return true;
}

void DataAnalyzerDefaultImpl1::setDataValues(const std::vector<double>& values) {
	_data.clear();
	for (double v : values) {
		if (std::isfinite(v)) {
			_data.push_back(v);
		}
	}
	_rebuildCache();
}

void DataAnalyzerDefaultImpl1::clearData() {
	_data.clear();
	_sortedData.clear();
	_data2.clear();
	_sortedData2.clear();
	_count = _count2 = 0;
	_sampleMean = _sampleVariance = _sampleStddev = 0.0;
	_sampleMean2 = _sampleVariance2 = _sampleStddev2 = 0.0;
}

bool DataAnalyzerDefaultImpl1::loadSecondSample(const std::vector<double>& values) {
	_data2.clear();
	for (double v : values) {
		if (std::isfinite(v)) {
			_data2.push_back(v);
		}
	}
	_rebuildCache2();
	return !_data2.empty();
}

bool DataAnalyzerDefaultImpl1::loadSecondSampleFromFile(const std::string& filename) {
	SimulationResultsDataset dataset;
	std::string err;
	if (!SimulationResultsDatasetParser::loadFromTextFile(filename, &dataset, &err)) {
		_lastError = "loadSecondSampleFromFile(\"" + filename + "\"): " + err;
		return false;
	}
	_lastError.clear();
	return loadSecondSample(dataset.values());
}

std::string DataAnalyzerDefaultImpl1::getLastError() const {
	return _lastError;
}

// ============================================================
// Configuration
// ============================================================

void DataAnalyzerDefaultImpl1::setConfidenceLevel(double confidenceLevel) {
	_confidenceLevel = std::max(0.0, std::min(1.0, confidenceLevel));
}

double DataAnalyzerDefaultImpl1::getConfidenceLevel() {
	return _confidenceLevel;
}

void DataAnalyzerDefaultImpl1::setSignificanceLevel(double significanceLevel) {
	_confidenceLevel = std::max(0.0, std::min(1.0, 1.0 - significanceLevel));
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
	if (_count >= 3 && _sampleStddev > 0.0) {
		const double n = static_cast<double>(_count);
		double m3 = 0.0;
		double m4 = 0.0;
		for (double x : _data) {
			const double z = (x - _sampleMean) / _sampleStddev;
			m3 += z * z * z;
			m4 += z * z * z * z;
		}
		s.skewness = (n / ((n - 1.0) * (n - 2.0))) * m3;
		if (_count >= 4) {
			s.kurtosis = ((n * (n + 1.0)) / ((n - 1.0) * (n - 2.0) * (n - 3.0))) * m4
			           - (3.0 * (n - 1.0) * (n - 1.0)) / ((n - 2.0) * (n - 3.0));
		} else {
			s.kurtosis = std::numeric_limits<double>::quiet_NaN();
		}
	} else {
		s.skewness = std::numeric_limits<double>::quiet_NaN();
		s.kurtosis = std::numeric_limits<double>::quiet_NaN();
	}
	return s;
}

double DataAnalyzerDefaultImpl1::quartile(unsigned short num) {
	return (_sortedData.empty() || num < 1 || num > 3) ? std::numeric_limits<double>::quiet_NaN() : _quantile(_sortedData, num * 0.25);
}

double DataAnalyzerDefaultImpl1::decile(unsigned short num) {
	return (_sortedData.empty() || num < 1 || num > 9) ? std::numeric_limits<double>::quiet_NaN() : _quantile(_sortedData, num * 0.10);
}

double DataAnalyzerDefaultImpl1::centile(unsigned short num) {
	return (_sortedData.empty() || num < 1 || num > 99) ? std::numeric_limits<double>::quiet_NaN() : _quantile(_sortedData, num * 0.01);
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
	h.frequencies.assign(numClasses, 0U);
	h.relativeFrequencies.assign(numClasses, 0.0);
	for (unsigned int k = 0; k < numClasses; ++k) {
		h.lowerLimits[k] = lo + static_cast<double>(k) * width;
	}
	for (double x : _data) {
		int k = static_cast<int>((x - lo) / width);
		k = std::max(0, std::min(static_cast<int>(numClasses) - 1, k));
		h.frequencies[static_cast<std::size_t>(k)]++;
	}
	for (unsigned int k = 0; k < numClasses; ++k) {
		h.relativeFrequencies[k] = static_cast<double>(h.frequencies[k]) / static_cast<double>(_data.size());
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
	b.median = _quantile(_sortedData, 0.5);
	b.q3 = _quantile(_sortedData, 0.75);
	b.iqr = b.q3 - b.q1;
	const double low = b.q1 - 1.5 * b.iqr;
	const double high = b.q3 + 1.5 * b.iqr;
	for (double x : _sortedData) {
		if (x < low || x > high) {
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
	r.rmse = r.r2 = std::numeric_limits<double>::quiet_NaN();
	if (_data.empty()) {
		return r;
	}
	const double minv = _sortedData.front();
	const double maxv = _sortedData.back();
	const bool nonNegative = minv >= 0.0;
	if (name == "uniform" && maxv > minv) {
		r.param1 = minv; r.param2 = maxv; r.valid = true;
	} else if (name == "triangular" && maxv > minv) {
		double mode = _sampleMode();
		if (!std::isfinite(mode)) mode = std::max(minv, std::min(maxv, _sampleMean));
		r.param1 = minv; r.param2 = mode; r.param3 = maxv; r.valid = true;
	} else if (name == "normal" && _sampleStddev > 0.0) {
		r.param1 = _sampleMean; r.param2 = _sampleStddev; r.valid = true;
	} else if (name == "exponential" && nonNegative && _sampleMean > 0.0) {
		r.param1 = _sampleMean; r.valid = true;
	} else if (name == "erlang" && nonNegative && _sampleMean > 0.0 && _sampleVariance > 0.0) {
		const double shape = std::max(1.0, std::round((_sampleMean * _sampleMean) / _sampleVariance));
		r.param1 = _sampleMean; r.param2 = shape; r.valid = true;
	} else if (name == "beta" && maxv > minv && _sampleVariance > 0.0) {
		const double range = maxv - minv;
		const double m = (_sampleMean - minv) / range;
		const double v = _sampleVariance / (range * range);
		const double common = m * (1.0 - m) / std::max(kEps, v) - 1.0;
		if (m > 0.0 && m < 1.0 && common > 0.0) {
			r.param1 = m * common; r.param2 = (1.0 - m) * common; r.param3 = minv; r.param4 = maxv; r.valid = true;
		}
	} else if (name == "weibull" && nonNegative && _sampleMean > 0.0 && _sampleStddev > 0.0) {
		const double cv = _sampleStddev / _sampleMean;
		const double shape = std::max(0.1, std::pow(std::max(kEps, cv), -1.086));
		const double scale = _sampleMean / std::tgamma(1.0 + 1.0 / shape);
		r.param1 = shape; r.param2 = scale; r.valid = std::isfinite(scale) && scale > 0.0;
	}
	if (!r.valid) {
		return r;
	}
	double sse = 0.0;
	const double n = static_cast<double>(_sortedData.size());
	for (std::size_t i = 0; i < _sortedData.size(); ++i) {
		const double theoretical = _cdfAt(_sortedData[i], r);
		if (!std::isfinite(theoretical)) {
			r.valid = false;
			return r;
		}
		const double empirical = (static_cast<double>(i) + 0.5) / n;
		const double d = theoretical - empirical;
		sse += d * d;
	}
	r.sse = sse;
	r.rmse = std::sqrt(sse / n);
	double sst = 0.0;
	for (std::size_t i = 0; i < _sortedData.size(); ++i) {
		const double centered = (static_cast<double>(i) + 0.5) / n - 0.5;
		sst += centered * centered;
	}
	r.r2 = (sst > 0.0) ? 1.0 - sse / sst : std::numeric_limits<double>::quiet_NaN();
	return r;
}

DataAnalyzer_if::FitResult DataAnalyzerDefaultImpl1::fitAll() {
	auto ranked = fitAllRanked();
	return ranked.empty() ? FitResult{} : ranked.front();
}

std::vector<DataAnalyzer_if::FitResult> DataAnalyzerDefaultImpl1::fitAllRanked() {
	static const std::vector<std::string> names = {"uniform", "triangular", "normal", "exponential", "erlang", "beta", "weibull"};
	std::vector<FitResult> results;
	for (const auto& name : names) {
		results.push_back(fitDistribution(name));
	}
	std::stable_sort(results.begin(), results.end(), [](const FitResult& a, const FitResult& b) {
		if (a.valid != b.valid) return a.valid > b.valid;
		return finiteOrInf(a.sse) < finiteOrInf(b.sse);
	});
	return results;
}

// ============================================================
// Goodness-of-fit tests
// ============================================================

DataAnalyzer_if::GoFResult DataAnalyzerDefaultImpl1::chiSquareGoodnessOfFit(const std::string& distributionName, double alpha) {
	GoFResult g{};
	g.distributionName = distributionName;
	g.significanceLevel = alpha;
	g.testStatistic = g.pValue = g.criticalValue = std::numeric_limits<double>::quiet_NaN();
	g.rejectH0 = false;
	const auto fit = fitDistribution(distributionName);
	if (!fit.valid || _data.size() < 2) {
		g.conclusion = "Invalid dataset or distribution fit.";
		return g;
	}
	const unsigned int classes = std::max(3U, static_cast<unsigned int>(std::sqrt(static_cast<double>(_data.size()))));
	const double lo = _sortedData.front();
	const double hi = _sortedData.back();
	const double width = (hi > lo) ? (hi - lo) / static_cast<double>(classes) : 1.0;
	double chi2 = 0.0;
	for (unsigned int k = 0; k < classes; ++k) {
		const double a = lo + static_cast<double>(k) * width;
		const double b = (k + 1 == classes) ? hi : a + width;
		unsigned int observed = 0;
		for (double x : _data) {
			if ((k + 1 == classes && x >= a && x <= b) || (x >= a && x < b)) {
				++observed;
			}
		}
		const double expected = static_cast<double>(_data.size()) * std::max(0.0, _cdfAt(b, fit) - _cdfAt(a, fit));
		if (expected > kEps) {
			const double d = static_cast<double>(observed) - expected;
			chi2 += d * d / expected;
		}
	}
	const unsigned int params = _numParams(distributionName);
	const unsigned int df = (classes > params + 1) ? classes - params - 1 : 1;
	g.testStatistic = chi2;
	g.pValue = 1.0 - _chi2Cdf(chi2, df);
	g.criticalValue = _chi2Quantile(df, 1.0 - alpha);
	g.rejectH0 = g.testStatistic > g.criticalValue;
	g.conclusion = g.rejectH0 ? "Reject H0." : "Do not reject H0.";
	return g;
}

DataAnalyzer_if::GoFResult DataAnalyzerDefaultImpl1::kolmogorovSmirnov(const std::string& distributionName, double alpha) {
	GoFResult g{};
	g.distributionName = distributionName;
	g.significanceLevel = alpha;
	g.testStatistic = g.pValue = g.criticalValue = std::numeric_limits<double>::quiet_NaN();
	const auto fit = fitDistribution(distributionName);
	if (!fit.valid || _sortedData.empty()) {
		g.conclusion = "Invalid dataset or distribution fit.";
		return g;
	}
	const double n = static_cast<double>(_sortedData.size());
	double d = 0.0;
	for (std::size_t i = 0; i < _sortedData.size(); ++i) {
		const double f = _cdfAt(_sortedData[i], fit);
		d = std::max(d, std::abs(f - static_cast<double>(i) / n));
		d = std::max(d, std::abs((static_cast<double>(i) + 1.0) / n - f));
	}
	g.testStatistic = d;
	g.pValue = _ksPValue(d, _sortedData.size());
	g.criticalValue = _ksQuantile(_sortedData.size(), alpha);
	g.rejectH0 = g.testStatistic > g.criticalValue;
	g.conclusion = g.rejectH0 ? "Reject H0." : "Do not reject H0.";
	return g;
}

DataAnalyzer_if::GoFResult DataAnalyzerDefaultImpl1::andersonDarling(const std::string& distributionName, double alpha) {
	GoFResult g{};
	g.distributionName = distributionName;
	g.significanceLevel = alpha;
	g.testStatistic = g.pValue = g.criticalValue = std::numeric_limits<double>::quiet_NaN();
	const auto fit = fitDistribution(distributionName);
	if (!fit.valid || _sortedData.empty()) {
		g.conclusion = "Invalid dataset or distribution fit.";
		return g;
	}
	const double n = static_cast<double>(_sortedData.size());
	double sum = 0.0;
	for (std::size_t i = 0; i < _sortedData.size(); ++i) {
		const double fi = std::max(kEps, std::min(1.0 - kEps, _cdfAt(_sortedData[i], fit)));
		const double fj = std::max(kEps, std::min(1.0 - kEps, _cdfAt(_sortedData[_sortedData.size() - 1 - i], fit)));
		sum += (2.0 * static_cast<double>(i + 1) - 1.0) * (std::log(fi) + std::log(1.0 - fj));
	}
	const double a2 = -n - sum / n;
	g.testStatistic = a2;
	g.pValue = _adPValue(a2);
	g.criticalValue = _adQuantile(alpha);
	g.rejectH0 = g.testStatistic > g.criticalValue;
	g.conclusion = g.rejectH0 ? "Reject H0." : "Do not reject H0.";
	return g;
}

DataAnalyzer_if::FitReport DataAnalyzerDefaultImpl1::analyzeFit(const std::string& distributionName, double significanceLevel) {
	FitReport report{};
	report.fit = fitDistribution(distributionName);
	report.chiSquare = chiSquareGoodnessOfFit(distributionName, significanceLevel);
	report.ks = kolmogorovSmirnov(distributionName, significanceLevel);
	report.ad = andersonDarling(distributionName, significanceLevel);
	return report;
}

// ============================================================
// Time-series analysis
// ============================================================

std::vector<double> DataAnalyzerDefaultImpl1::movingAverage(unsigned int window) {
	std::vector<double> result;
	if (window == 0 || _data.size() < window) return result;
	double sum = std::accumulate(_data.begin(), _data.begin() + window, 0.0);
	result.push_back(sum / static_cast<double>(window));
	for (std::size_t i = window; i < _data.size(); ++i) {
		sum += _data[i] - _data[i - window];
		result.push_back(sum / static_cast<double>(window));
	}
	return result;
}

std::vector<double> DataAnalyzerDefaultImpl1::autocorrelation(unsigned int maxLag) {
	std::vector<double> acf;
	if (_data.empty()) return acf;
	const unsigned int capped = std::min<unsigned int>(maxLag, static_cast<unsigned int>(_data.size() - 1));
	acf.resize(capped + 1, 0.0);
	double denom = 0.0;
	for (double x : _data) {
		const double d = x - _sampleMean;
		denom += d * d;
	}
	if (denom <= 0.0) {
		acf[0] = 1.0;
		return acf;
	}
	for (unsigned int lag = 0; lag <= capped; ++lag) {
		double num = 0.0;
		for (std::size_t i = lag; i < _data.size(); ++i) {
			num += (_data[i] - _sampleMean) * (_data[i - lag] - _sampleMean);
		}
		acf[lag] = num / denom;
	}
	return acf;
}

DataAnalyzer_if::CorrelogramData DataAnalyzerDefaultImpl1::correlogram(unsigned int maxLag) {
	CorrelogramData c{};
	c.acf = autocorrelation(maxLag);
	c.n = static_cast<unsigned int>(_data.size());
	c.confidenceBound = _data.empty() ? std::numeric_limits<double>::quiet_NaN() : 1.96 / std::sqrt(static_cast<double>(_data.size()));
	return c;
}

DataAnalyzer_if::TrendDiagnostic DataAnalyzerDefaultImpl1::detectTrend() {
	TrendDiagnostic t{};
	if (_data.size() < 4) return t;
	const double n = static_cast<double>(_data.size());
	const double meanX = (n - 1.0) / 2.0;
	double sxx = 0.0;
	double sxy = 0.0;
	for (std::size_t i = 0; i < _data.size(); ++i) {
		const double dx = static_cast<double>(i) - meanX;
		sxx += dx * dx;
		sxy += dx * (_data[i] - _sampleMean);
	}
	t.trendSlope = (sxx > 0.0) ? sxy / sxx : 0.0;
	t.trendIntercept = _sampleMean - t.trendSlope * meanX;
	const auto cg = correlogram(std::min<unsigned int>(10, static_cast<unsigned int>(_data.size() - 1)));
	t.hasTrend = cg.acf.size() > 1 && std::abs(cg.acf[1]) > cg.confidenceBound;
	t.hasSeasonality = false;
	for (std::size_t i = 2; i < cg.acf.size(); ++i) {
		if (std::abs(cg.acf[i]) > cg.confidenceBound) {
			t.hasSeasonality = true;
			break;
		}
	}
	return t;
}

double DataAnalyzerDefaultImpl1::exceedanceProbability(double x, const std::string& distributionName) {
	const auto fit = fitDistribution(distributionName);
	if (!fit.valid) return std::numeric_limits<double>::quiet_NaN();
	return 1.0 - _cdfAt(x, fit);
}

std::vector<double> DataAnalyzerDefaultImpl1::exceedanceCurve(double xMin, double xMax, unsigned int points, const std::string& distributionName) {
	std::vector<double> curve;
	if (points == 0 || xMax < xMin) return curve;
	curve.reserve(points);
	if (points == 1) {
		curve.push_back(exceedanceProbability(xMin, distributionName));
		return curve;
	}
	for (unsigned int i = 0; i < points; ++i) {
		const double x = xMin + static_cast<double>(i) * (xMax - xMin) / static_cast<double>(points - 1);
		curve.push_back(exceedanceProbability(x, distributionName));
	}
	return curve;
}

// ============================================================
// Inference wrappers
// ============================================================

HypothesisTester_if::ConfidenceInterval DataAnalyzerDefaultImpl1::averageConfidenceInterval() {
	if (_count == 0) return {0.0, 0.0, 0.0};
	return _tester.averageConfidenceInterval(_sampleMean, _sampleStddev, static_cast<unsigned int>(_count), _confidenceLevel);
}

HypothesisTester_if::ConfidenceInterval DataAnalyzerDefaultImpl1::proportionConfidenceInterval(checkProportionFunction checker) {
	if (_count == 0 || checker == nullptr) return {0.0, 0.0, 0.0};
	unsigned int hits = 0;
	for (double x : _data) if (checker(x)) ++hits;
	return _tester.proportionConfidenceInterval(static_cast<double>(hits) / static_cast<double>(_count), static_cast<unsigned int>(_count), _confidenceLevel);
}

HypothesisTester_if::ConfidenceInterval DataAnalyzerDefaultImpl1::varianceConfidenceInterval() {
	if (_count < 2) return {0.0, 0.0, 0.0};
	return _tester.varianceConfidenceInterval(_sampleVariance, static_cast<unsigned int>(_count), _confidenceLevel);
}

HypothesisTester_if::TestResult DataAnalyzerDefaultImpl1::testAverage(double hypothesizedMean, HypothesisTester_if::H1Comparition comp) {
	if (_count < 2) return {1.0, false, 0.0, 0.0, 0.0};
	return _tester.testAverage(_sampleMean, _sampleStddev, static_cast<unsigned int>(_count), hypothesizedMean, _confidenceLevel, comp);
}

HypothesisTester_if::TestResult DataAnalyzerDefaultImpl1::testProportion(double hypothesizedProp, checkProportionFunction checker, HypothesisTester_if::H1Comparition comp) {
	if (_count == 0 || checker == nullptr) return {1.0, false, 0.0, 0.0, 0.0};
	unsigned int hits = 0;
	for (double x : _data) if (checker(x)) ++hits;
	return _tester.testProportion(static_cast<double>(hits) / static_cast<double>(_count), static_cast<unsigned int>(_count), hypothesizedProp, _confidenceLevel, comp);
}

HypothesisTester_if::TestResult DataAnalyzerDefaultImpl1::testVariance(double hypothesizedVariance, HypothesisTester_if::H1Comparition comp) {
	if (_count < 2) return {1.0, false, 0.0, 0.0, 0.0};
	return _tester.testVariance(_sampleVariance, static_cast<unsigned int>(_count), hypothesizedVariance, _confidenceLevel, comp);
}

HypothesisTester_if::TestResult DataAnalyzerDefaultImpl1::testAverageTwoSamples(HypothesisTester_if::H1Comparition comp) {
	if (_count < 2 || _count2 < 2) return {1.0, false, 0.0, 0.0, 0.0};
	return _tester.testAverage(_sampleMean, _sampleStddev, static_cast<unsigned int>(_count), _sampleMean2, _sampleStddev2, static_cast<unsigned int>(_count2), _confidenceLevel, comp);
}

HypothesisTester_if::TestResult DataAnalyzerDefaultImpl1::testProportionTwoSamples(checkProportionFunction checker, HypothesisTester_if::H1Comparition comp) {
	if (_count == 0 || _count2 == 0 || checker == nullptr) return {1.0, false, 0.0, 0.0, 0.0};
	unsigned int hits1 = 0;
	unsigned int hits2 = 0;
	for (double x : _data) if (checker(x)) ++hits1;
	for (double x : _data2) if (checker(x)) ++hits2;
	return _tester.testProportion(static_cast<double>(hits1) / static_cast<double>(_count), static_cast<unsigned int>(_count), static_cast<double>(hits2) / static_cast<double>(_count2), static_cast<unsigned int>(_count2), _confidenceLevel, comp);
}

HypothesisTester_if::TestResult DataAnalyzerDefaultImpl1::testVarianceTwoSamples(HypothesisTester_if::H1Comparition comp) {
	if (_count < 2 || _count2 < 2) return {1.0, false, 0.0, 0.0, 0.0};
	return _tester.testVariance(_sampleVariance, static_cast<unsigned int>(_count), _sampleVariance2, static_cast<unsigned int>(_count2), _confidenceLevel, comp);
}
