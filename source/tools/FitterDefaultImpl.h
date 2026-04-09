#ifndef FITTERDEFAULTIMPL_H
#define FITTERDEFAULTIMPL_H

#include "Fitter_if.h"
#include "ProbabilityDistributionBase.h"
#include "SolverDefaultImpl1.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <limits>
#include <numeric>
#include <string>
#include <vector>

/**
 * @brief Functional implementation of Fitter_if consolidated through FITTER-2.
 *
 * Current scope:
 * - Supports real dataset loading from binary file with raw sequential doubles
 *   (compatible with CollectorDatafileDefaultImpl1 persistence format).
 * - Implements fitting for Uniform, Triangular, Normal, Exponential, Erlang,
 *   Beta (scaled) and Weibull.
 *
 * Error metric:
 * - Uses SSE between empirical CDF points p_i = (i+0.5)/n and fitted model CDF.
 * - The same criterion is used consistently by fit* methods and fitAll().
 *
 * Failure convention:
 * - sqrerror = +infinity.
 * - returned parameters = quiet_NaN().
 * - fitAll name = "no-valid-fit" or "invalid-dataset".
 * - isNormalDistributed() = false on invalid/insufficient data.
 */
class FitterDefaultImpl : public Fitter_if {
public:
	FitterDefaultImpl() = default;
	virtual ~FitterDefaultImpl() = default;

public:
	virtual bool isNormalDistributed(double confidencelevel) override {
		double sqrerror = std::numeric_limits<double>::infinity();
		double avg = _nan();
		double stddev = _nan();
		fitNormal(&sqrerror, &avg, &stddev);
		if (!std::isfinite(sqrerror)) {
			return false;
		}
		if (_count < 8) {
			return false;
		}
		double cl = confidencelevel;
		if (cl > 1.0) {
			cl /= 100.0;
		}
		cl = std::max(0.0, std::min(1.0, cl));
		const double normalizedSse = sqrerror / static_cast<double>(_count);
		const double threshold = 0.008 + (1.0 - cl) * 0.02;
		return normalizedSse <= threshold;
	}

	virtual void fitUniform(double *sqrerror, double *min, double *max) override {
		if (!_ensureDataLoaded() || _count < 2 || !(_sampleMax > _sampleMin)) {
			_setFailure3(sqrerror, min, max);
			return;
		}
		const double fittedMin = _sampleMin;
		const double fittedMax = _sampleMax;

		if (min != nullptr) {
			*min = fittedMin;
		}
		if (max != nullptr) {
			*max = fittedMax;
		}
		_setSse(sqrerror, [fittedMin, fittedMax](double x) {
			if (x <= fittedMin) {
				return 0.0;
			}
			if (x >= fittedMax) {
				return 1.0;
			}
			return (x - fittedMin) / (fittedMax - fittedMin);
		});
	}

	virtual void fitTriangular(double *sqrerror, double *min, double *mo, double *max) override {
		if (!_ensureDataLoaded() || _count < 2 || !(_sampleMax > _sampleMin)) {
			_setFailure4(sqrerror, min, mo, max);
			return;
		}

		const double fittedMin = _sampleMin;
		const double fittedMax = _sampleMax;
		const double range = fittedMax - fittedMin;
		double mode = 3.0 * _sampleMean - fittedMin - fittedMax;
		mode = std::max(fittedMin, std::min(fittedMax, mode));

		const double eps = range * 1e-9;
		if (mode <= fittedMin) {
			mode = fittedMin + eps;
		}
		if (mode >= fittedMax) {
			mode = fittedMax - eps;
		}
		if (!(mode > fittedMin) || !(mode < fittedMax)) {
			_setFailure4(sqrerror, min, mo, max);
			return;
		}

		if (min != nullptr) {
			*min = fittedMin;
		}
		if (mo != nullptr) {
			*mo = mode;
		}
		if (max != nullptr) {
			*max = fittedMax;
		}

		_setSse(sqrerror, [fittedMin, mode, fittedMax](double x) {
			if (x <= fittedMin) {
				return 0.0;
			}
			if (x >= fittedMax) {
				return 1.0;
			}
			if (x <= mode) {
				const double num = (x - fittedMin) * (x - fittedMin);
				const double den = (fittedMax - fittedMin) * (mode - fittedMin);
				return num / den;
			}
			const double num = (fittedMax - x) * (fittedMax - x);
			const double den = (fittedMax - fittedMin) * (fittedMax - mode);
			return 1.0 - num / den;
		});
	}

	virtual void fitNormal(double *sqrerror, double *avg, double *stddev) override {
		if (!_ensureDataLoaded() || _count < 2 || !(_sampleStddev > 0.0) || !std::isfinite(_sampleStddev)) {
			_setFailure3(sqrerror, avg, stddev);
			return;
		}
		if (avg != nullptr) {
			*avg = _sampleMean;
		}
		if (stddev != nullptr) {
			*stddev = _sampleStddev;
		}
		_setSse(sqrerror, [this](double x) {
			const double z = (x - _sampleMean) / (_sampleStddev * std::sqrt(2.0));
			return 0.5 * std::erfc(-z);
		});
	}

	virtual void fitExpo(double *sqrerror, double *avg1) override {
		if (!_ensureDataLoaded() || !(_sampleMean > 0.0) || _hasNegativeData) {
			_setFailure2(sqrerror, avg1);
			return;
		}
		if (avg1 != nullptr) {
			*avg1 = _sampleMean;
		}
		_setSse(sqrerror, [this](double x) {
			if (x < 0.0) {
				return 0.0;
			}
			return 1.0 - std::exp(-x / _sampleMean);
		});
	}

	virtual void fitErlang(double *sqrerror, double *avg, double *m) override {
		if (!_ensureDataLoaded() || !(_sampleMean > 0.0) || !(_sampleVariance > 0.0) || _hasNegativeData) {
			_setFailure3(sqrerror, avg, m);
			return;
		}
		long int mInt = static_cast<long int>(std::llround((_sampleMean * _sampleMean) / _sampleVariance));
		if (mInt < 1) {
			mInt = 1;
		}
		const double scale = _sampleMean / static_cast<double>(mInt);
		if (!(scale > 0.0) || !std::isfinite(scale)) {
			_setFailure3(sqrerror, avg, m);
			return;
		}
		if (avg != nullptr) {
			*avg = _sampleMean;
		}
		if (m != nullptr) {
			*m = static_cast<double>(mInt);
		}
		_setSse(sqrerror, [mInt, scale](double x) {
			if (x < 0.0) {
				return 0.0;
			}
			const double z = x / scale;
			double sum = 1.0;
			double term = 1.0;
			for (long int k = 1; k < mInt; ++k) {
				term *= z / static_cast<double>(k);
				sum += term;
			}
			return 1.0 - std::exp(-z) * sum;
		});
	}

	virtual void fitBeta(double *sqrerror, double *alpha, double *beta, double *infLimit, double *supLimit) override {
		double fittedAlpha = _nan();
		double fittedBeta = _nan();
		double fittedInf = _nan();
		double fittedSup = _nan();
		if (!_estimateScaledBetaMoments(&fittedAlpha, &fittedBeta, &fittedInf, &fittedSup)) {
			_setFailure5(sqrerror, alpha, beta, infLimit, supLimit);
			return;
		}

		if (alpha != nullptr) {
			*alpha = fittedAlpha;
		}
		if (beta != nullptr) {
			*beta = fittedBeta;
		}
		if (infLimit != nullptr) {
			*infLimit = fittedInf;
		}
		if (supLimit != nullptr) {
			*supLimit = fittedSup;
		}

		_setSse(sqrerror, [this, fittedAlpha, fittedBeta, fittedInf, fittedSup](double x) {
			return _betaCdfScaled(x, fittedAlpha, fittedBeta, fittedInf, fittedSup);
		});
	}

	virtual void fitWeibull(double *sqrerror, double *alpha, double *scale) override {
		if (!_ensureDataLoaded() || _count < 2 || _hasNegativeData || !(_sampleMean > 0.0) || !(_sampleVariance > 0.0)) {
			_setFailure3(sqrerror, alpha, scale);
			return;
		}

		const double cv = _sampleStddev / _sampleMean;
		double fittedShape = _nan();
		if (!_estimateWeibullShapeFromCv(cv, &fittedShape)) {
			_setFailure3(sqrerror, alpha, scale);
			return;
		}

		const double gammaTerm = std::tgamma(1.0 + 1.0 / fittedShape);
		const double fittedScale = _sampleMean / gammaTerm;
		if (!_isFinitePositive(fittedShape) || !_isFinitePositive(fittedScale)) {
			_setFailure3(sqrerror, alpha, scale);
			return;
		}

		if (alpha != nullptr) {
			*alpha = fittedShape;
		}
		if (scale != nullptr) {
			*scale = fittedScale;
		}

		_setSse(sqrerror, [this, fittedShape, fittedScale](double x) {
			return _weibullCdf(x, fittedShape, fittedScale);
		});
	}

	virtual void fitAll(double *sqrerror, std::string *name) override {
		if (!_ensureDataLoaded()) {
			if (sqrerror != nullptr) {
				*sqrerror = std::numeric_limits<double>::infinity();
			}
			if (name != nullptr) {
				*name = "invalid-dataset";
			}
			return;
		}

		double bestError = std::numeric_limits<double>::infinity();
		std::string bestName = "no-valid-fit";

		auto consider = [&bestError, &bestName](double error, const std::string& candidate) {
			if (std::isfinite(error) && error < bestError) {
				bestError = error;
				bestName = candidate;
			}
		};

		double error = std::numeric_limits<double>::infinity();
		double p1, p2, p3, p4;

		fitUniform(&error, &p1, &p2);
		consider(error, "uniform");

		fitTriangular(&error, &p1, &p2, &p3);
		consider(error, "triangular");

		fitNormal(&error, &p1, &p2);
		consider(error, "normal");

		fitExpo(&error, &p1);
		consider(error, "exponential");

		fitErlang(&error, &p1, &p2);
		consider(error, "erlang");

		fitBeta(&error, &p1, &p2, &p3, &p4);
		consider(error, "beta");

		fitWeibull(&error, &p1, &p2);
		consider(error, "weibull");

		if (sqrerror != nullptr) {
			*sqrerror = bestError;
		}
		if (name != nullptr) {
			*name = bestName;
		}
	}

	virtual void setDataFilename(std::string dataFilename) override {
		if (_dataFilename != dataFilename) {
			_dataFilename = dataFilename;
			_invalidateCache();
		}
	}

	virtual std::string getDataFilename() override {
		return _dataFilename;
	}

private:
	void _invalidateCache() {
		_cacheLoaded = false;
		_cacheUsable = false;
		_data.clear();
		_sortedData.clear();
		_count = 0;
		_sampleMin = _nan();
		_sampleMax = _nan();
		_sampleMean = _nan();
		_sampleVariance = _nan();
		_sampleStddev = _nan();
		_hasNegativeData = false;
	}

	bool _ensureDataLoaded() {
		if (!_cacheLoaded) {
			_cacheUsable = _loadDataFromBinaryFile();
			_cacheLoaded = true;
		}
		return _cacheUsable;
	}

	bool _loadDataFromBinaryFile() {
		_invalidateCache();
		if (_dataFilename.empty()) {
			return false;
		}

		std::ifstream file(_dataFilename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
		if (!file.good()) {
			return false;
		}

		const std::streamsize bytes = file.tellg();
		if (bytes <= 0 || (bytes % static_cast<std::streamsize>(sizeof(double))) != 0) {
			return false;
		}

		file.seekg(0, std::ios::beg);
		const std::size_t n = static_cast<std::size_t>(bytes / static_cast<std::streamsize>(sizeof(double)));
		std::vector<double> values(n, 0.0);
		file.read(reinterpret_cast<char*>(values.data()), bytes);
		if (!file || file.gcount() != bytes) {
			return false;
		}

		for (double value : values) {
			if (!std::isfinite(value)) {
				return false;
			}
		}

		_data = values;
		_sortedData = _data;
		std::sort(_sortedData.begin(), _sortedData.end());
		_count = _data.size();
		if (_count == 0) {
			return false;
		}

		_sampleMin = _sortedData.front();
		_sampleMax = _sortedData.back();
		const double sum = std::accumulate(_data.begin(), _data.end(), 0.0);
		_sampleMean = sum / static_cast<double>(_count);
		_hasNegativeData = (_sampleMin < 0.0);

		if (_count >= 2) {
			double sq = 0.0;
			for (double x : _data) {
				const double d = x - _sampleMean;
				sq += d * d;
			}
			_sampleVariance = sq / static_cast<double>(_count - 1U);
			_sampleStddev = std::sqrt(_sampleVariance);
		} else {
			_sampleVariance = _nan();
			_sampleStddev = _nan();
		}
		return true;
	}

	void _setSse(double* sqrerror, const std::function<double(double)>& cdf) {
		if (sqrerror == nullptr) {
			return;
		}
		if (_sortedData.empty()) {
			*sqrerror = std::numeric_limits<double>::infinity();
			return;
		}
		double sse = 0.0;
		const std::size_t n = _sortedData.size();
		for (std::size_t i = 0; i < n; ++i) {
			const double pi = (static_cast<double>(i) + 0.5) / static_cast<double>(n);
			double model = cdf(_sortedData[i]);
			if (!std::isfinite(model)) {
				*sqrerror = std::numeric_limits<double>::infinity();
				return;
			}
			model = std::max(0.0, std::min(1.0, model));
			const double d = model - pi;
			sse += d * d;
		}
		*sqrerror = sse;
	}

	static double _clampUnitInterval(double value) {
		return std::max(0.0, std::min(1.0, value));
	}

	static bool _isFinitePositive(double value) {
		return std::isfinite(value) && value > 0.0;
	}

	static double _betaPdfSafe(double x, double alpha, double beta) {
		if (!(_isFinitePositive(alpha) && _isFinitePositive(beta))) {
			return std::numeric_limits<double>::quiet_NaN();
		}
		const double eps = 1e-12;
		const double xc = std::max(eps, std::min(1.0 - eps, x));
		const double pdf = ProbabilityDistributionBase::beta(xc, alpha, beta);
		if (!std::isfinite(pdf) || pdf < 0.0) {
			return std::numeric_limits<double>::quiet_NaN();
		}
		return pdf;
	}

	double _betaCdfScaled(double x, double alpha, double beta, double infLimit, double supLimit) const {
		if (!(_isFinitePositive(alpha) && _isFinitePositive(beta))) {
			return _nan();
		}
		if (!std::isfinite(infLimit) || !std::isfinite(supLimit) || !(supLimit > infLimit)) {
			return _nan();
		}
		if (x <= infLimit) {
			return 0.0;
		}
		if (x >= supLimit) {
			return 1.0;
		}
		const double y = (x - infLimit) / (supLimit - infLimit);
		if (y <= 0.0) {
			return 0.0;
		}
		if (y >= 1.0) {
			return 1.0;
		}
		SolverDefaultImpl1 solver(1e-5, 2000);
		const double cdf = solver.integrate(0.0, y, &_betaPdfSafe, alpha, beta);
		if (!std::isfinite(cdf)) {
			return _nan();
		}
		return _clampUnitInterval(cdf);
	}

	double _weibullCdf(double x, double alpha, double scale) const {
		if (!(_isFinitePositive(alpha) && _isFinitePositive(scale))) {
			return _nan();
		}
		if (x < 0.0) {
			return 0.0;
		}
		const double ratio = x / scale;
		const double p = std::pow(ratio, alpha);
		if (!std::isfinite(p)) {
			return _nan();
		}
		return _clampUnitInterval(1.0 - std::exp(-p));
	}

	bool _estimateScaledBetaMoments(double* alpha, double* beta, double* infLimit, double* supLimit) {
		if (!_ensureDataLoaded() || _count < 2 || !(_sampleMax > _sampleMin) || !(_sampleVariance > 0.0)) {
			return false;
		}

		const double a = _sampleMin;
		const double b = _sampleMax;
		const double range = b - a;
		if (!(range > 0.0) || !std::isfinite(range)) {
			return false;
		}

		const double eps = 1e-12;
		double sum = 0.0;
		for (double x : _data) {
			const double y = (x - a) / range;
			const double yc = std::max(eps, std::min(1.0 - eps, y));
			sum += yc;
		}
		const double n = static_cast<double>(_data.size());
		const double m = sum / n;
		if (!(m > 0.0 && m < 1.0) || !std::isfinite(m)) {
			return false;
		}

		double varSum = 0.0;
		for (double x : _data) {
			const double y = (x - a) / range;
			const double yc = std::max(eps, std::min(1.0 - eps, y));
			const double d = yc - m;
			varSum += d * d;
		}
		const double v = varSum / static_cast<double>(_data.size() - 1U);
		if (!(v > 0.0) || !std::isfinite(v)) {
			return false;
		}

		const double common = (m * (1.0 - m) / v) - 1.0;
		const double aShape = m * common;
		const double bShape = (1.0 - m) * common;
		if (!(common > 0.0) || !_isFinitePositive(aShape) || !_isFinitePositive(bShape)) {
			return false;
		}

		if (alpha != nullptr) {
			*alpha = aShape;
		}
		if (beta != nullptr) {
			*beta = bShape;
		}
		if (infLimit != nullptr) {
			*infLimit = a;
		}
		if (supLimit != nullptr) {
			*supLimit = b;
		}
		return true;
	}

	bool _estimateWeibullShapeFromCv(double cv, double* shape) const {
		if (!(cv > 0.0) || !std::isfinite(cv) || shape == nullptr) {
			return false;
		}
		const double cv2 = cv * cv;
		auto g = [cv2](double k) {
			const double g1 = std::tgamma(1.0 + 1.0 / k);
			const double g2 = std::tgamma(1.0 + 2.0 / k);
			if (!(std::isfinite(g1) && std::isfinite(g2) && g1 > 0.0)) {
				return std::numeric_limits<double>::quiet_NaN();
			}
			return (g2 / (g1 * g1)) - 1.0 - cv2;
		};

		double lo = 0.1;
		double hi = 100.0;
		double flo = g(lo);
		double fhi = g(hi);
		if (!(std::isfinite(flo) && std::isfinite(fhi))) {
			return false;
		}
		if (flo * fhi > 0.0) {
			return false;
		}

		for (int i = 0; i < 100; ++i) {
			const double mid = 0.5 * (lo + hi);
			const double fmid = g(mid);
			if (!std::isfinite(fmid)) {
				return false;
			}
			if (std::abs(fmid) < 1e-10 || (hi - lo) < 1e-8) {
				*shape = mid;
				return true;
			}
			if (flo * fmid > 0.0) {
				lo = mid;
				flo = fmid;
			} else {
				hi = mid;
				fhi = fmid;
			}
		}

		*shape = 0.5 * (lo + hi);
		return _isFinitePositive(*shape);
	}

	static double _nan() {
		return std::numeric_limits<double>::quiet_NaN();
	}

	void _setFailure2(double* v1, double* v2) {
		if (v1 != nullptr) {
			*v1 = std::numeric_limits<double>::infinity();
		}
		if (v2 != nullptr) {
			*v2 = _nan();
		}
	}

	void _setFailure3(double* v1, double* v2, double* v3) {
		_setFailure2(v1, v2);
		if (v3 != nullptr) {
			*v3 = _nan();
		}
	}

	void _setFailure4(double* v1, double* v2, double* v3, double* v4) {
		_setFailure3(v1, v2, v3);
		if (v4 != nullptr) {
			*v4 = _nan();
		}
	}

	void _setFailure5(double* v1, double* v2, double* v3, double* v4, double* v5) {
		_setFailure4(v1, v2, v3, v4);
		if (v5 != nullptr) {
			*v5 = _nan();
		}
	}

private:
	std::string _dataFilename = "";

	bool _cacheLoaded = false;
	bool _cacheUsable = false;
	std::vector<double> _data;
	std::vector<double> _sortedData;

	std::size_t _count = 0;
	double _sampleMin = _nan();
	double _sampleMax = _nan();
	double _sampleMean = _nan();
	double _sampleVariance = _nan();
	double _sampleStddev = _nan();
	bool _hasNegativeData = false;
};

#endif /* FITTERDEFAULTIMPL_H */
