#include "FitterDefaultImpl.h"
#include "tools/ProbabilityDistributionBase.h"
#include "tools/SolverDefaultImpl1.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace {

double nanVal() {
	return std::numeric_limits<double>::quiet_NaN();
}

bool isFinitePositive(double v) {
	return std::isfinite(v) && v > 0.0;
}

double clampUnit(double v) {
	return std::max(0.0, std::min(1.0, v));
}

void setFailure2(double* v1, double* v2) {
	if (v1 != nullptr) { *v1 = std::numeric_limits<double>::infinity(); }
	if (v2 != nullptr) { *v2 = nanVal(); }
}

void setFailure3(double* v1, double* v2, double* v3) {
	setFailure2(v1, v2);
	if (v3 != nullptr) { *v3 = nanVal(); }
}

void setFailure4(double* v1, double* v2, double* v3, double* v4) {
	setFailure3(v1, v2, v3);
	if (v4 != nullptr) { *v4 = nanVal(); }
}

void setFailure5(double* v1, double* v2, double* v3, double* v4, double* v5) {
	setFailure4(v1, v2, v3, v4);
	if (v5 != nullptr) { *v5 = nanVal(); }
}

FittedParameter parameter(const std::string& name, double value) {
	return FittedParameter{name, value};
}

FittingResult fittingResult(const std::string& name, double error, std::vector<FittedParameter> parameters) {
	FittingResult result;
	result.distributionName = name;
	result.squaredError = error;
	result.parameters = std::move(parameters);
	result.success = std::isfinite(error);
	result.message = result.success ? "ok" : "fit failed";
	return result;
}

} // namespace

// --- Fitter_if interface ---

bool FitterDefaultImpl::isNormalDistributed(double confidencelevel) {
	double sqrerror = std::numeric_limits<double>::infinity();
	double avg = nanVal();
	double stddev = nanVal();
	fitNormal(&sqrerror, &avg, &stddev);
	if (!std::isfinite(sqrerror)) {
		return false;
	}
	if (_dataset.count() < 8) {
		return false;
	}
	double cl = confidencelevel;
	if (cl > 1.0) {
		cl /= 100.0;
	}
	cl = std::max(0.0, std::min(1.0, cl));
	const double normalizedSse = sqrerror / static_cast<double>(_dataset.count());
	const double threshold = 0.008 + (1.0 - cl) * 0.02;
	return normalizedSse <= threshold;
}

void FitterDefaultImpl::fitUniform(double* sqrerror, double* min, double* max) {
	if (!_hasUsableDataset() || _dataset.count() < 2 || !(_dataset.max() > _dataset.min())) {
		setFailure3(sqrerror, min, max);
		return;
	}
	const double fittedMin = _dataset.min();
	const double fittedMax = _dataset.max();
	if (min != nullptr) { *min = fittedMin; }
	if (max != nullptr) { *max = fittedMax; }
	_setSse(sqrerror, [fittedMin, fittedMax](double x) {
		if (x <= fittedMin) { return 0.0; }
		if (x >= fittedMax) { return 1.0; }
		return (x - fittedMin) / (fittedMax - fittedMin);
	});
}

void FitterDefaultImpl::fitTriangular(double* sqrerror, double* min, double* mo, double* max) {
	if (!_hasUsableDataset() || _dataset.count() < 2 || !(_dataset.max() > _dataset.min())) {
		setFailure4(sqrerror, min, mo, max);
		return;
	}
	const double fittedMin = _dataset.min();
	const double fittedMax = _dataset.max();
	const double range = fittedMax - fittedMin;
	double mode = 3.0 * _dataset.mean() - fittedMin - fittedMax;
	mode = std::max(fittedMin, std::min(fittedMax, mode));
	const double eps = range * 1e-9;
	if (mode <= fittedMin) { mode = fittedMin + eps; }
	if (mode >= fittedMax) { mode = fittedMax - eps; }
	if (!(mode > fittedMin) || !(mode < fittedMax)) {
		setFailure4(sqrerror, min, mo, max);
		return;
	}
	if (min != nullptr) { *min = fittedMin; }
	if (mo  != nullptr) { *mo  = mode;      }
	if (max != nullptr) { *max = fittedMax; }
	_setSse(sqrerror, [fittedMin, mode, fittedMax](double x) {
		if (x <= fittedMin) { return 0.0; }
		if (x >= fittedMax) { return 1.0; }
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

void FitterDefaultImpl::fitNormal(double* sqrerror, double* avg, double* stddev) {
	const double sampleStddev = _dataset.stddev();
	if (!_hasUsableDataset() || _dataset.count() < 2 || !(sampleStddev > 0.0) || !std::isfinite(sampleStddev)) {
		setFailure3(sqrerror, avg, stddev);
		return;
	}
	const double sampleMean = _dataset.mean();
	if (avg    != nullptr) { *avg    = sampleMean;   }
	if (stddev != nullptr) { *stddev = sampleStddev; }
	_setSse(sqrerror, [sampleMean, sampleStddev](double x) {
		const double z = (x - sampleMean) / (sampleStddev * std::sqrt(2.0));
		return 0.5 * std::erfc(-z);
	});
}

void FitterDefaultImpl::fitExpo(double* sqrerror, double* avg1) {
	const double sampleMean = _dataset.mean();
	if (!_hasUsableDataset() || !(sampleMean > 0.0) || _dataset.hasNegativeData()) {
		setFailure2(sqrerror, avg1);
		return;
	}
	if (avg1 != nullptr) { *avg1 = sampleMean; }
	_setSse(sqrerror, [sampleMean](double x) {
		if (x < 0.0) { return 0.0; }
		return 1.0 - std::exp(-x / sampleMean);
	});
}

void FitterDefaultImpl::fitErlang(double* sqrerror, double* avg, double* m) {
	const double sampleMean     = _dataset.mean();
	const double sampleVariance = _dataset.variance();
	if (!_hasUsableDataset() || !(sampleMean > 0.0) || !(sampleVariance > 0.0) || _dataset.hasNegativeData()) {
		setFailure3(sqrerror, avg, m);
		return;
	}
	long int mInt = static_cast<long int>(std::llround((sampleMean * sampleMean) / sampleVariance));
	if (mInt < 1) { mInt = 1; }
	const double scale = sampleMean / static_cast<double>(mInt);
	if (!(scale > 0.0) || !std::isfinite(scale)) {
		setFailure3(sqrerror, avg, m);
		return;
	}
	if (avg != nullptr) { *avg = sampleMean;             }
	if (m   != nullptr) { *m   = static_cast<double>(mInt); }
	_setSse(sqrerror, [mInt, scale](double x) {
		if (x < 0.0) { return 0.0; }
		const double z = x / scale;
		double sum  = 1.0;
		double term = 1.0;
		for (long int k = 1; k < mInt; ++k) {
			term *= z / static_cast<double>(k);
			sum  += term;
		}
		return 1.0 - std::exp(-z) * sum;
	});
}

void FitterDefaultImpl::fitBeta(double* sqrerror, double* alpha, double* beta, double* infLimit, double* supLimit) {
	double fittedAlpha = nanVal();
	double fittedBeta  = nanVal();
	double fittedInf   = nanVal();
	double fittedSup   = nanVal();
	if (!_estimateScaledBetaMoments(&fittedAlpha, &fittedBeta, &fittedInf, &fittedSup)) {
		setFailure5(sqrerror, alpha, beta, infLimit, supLimit);
		return;
	}
	if (alpha    != nullptr) { *alpha    = fittedAlpha; }
	if (beta     != nullptr) { *beta     = fittedBeta;  }
	if (infLimit != nullptr) { *infLimit = fittedInf;   }
	if (supLimit != nullptr) { *supLimit = fittedSup;   }
	_setSse(sqrerror, [this, fittedAlpha, fittedBeta, fittedInf, fittedSup](double x) {
		return _betaCdfScaled(x, fittedAlpha, fittedBeta, fittedInf, fittedSup);
	});
}

void FitterDefaultImpl::fitWeibull(double* sqrerror, double* alpha, double* scale) {
	const double sampleMean     = _dataset.mean();
	const double sampleVariance = _dataset.variance();
	if (!_hasUsableDataset() || _dataset.count() < 2 || _dataset.hasNegativeData() || !(sampleMean > 0.0) || !(sampleVariance > 0.0)) {
		setFailure3(sqrerror, alpha, scale);
		return;
	}
	const double cv = _dataset.stddev() / sampleMean;
	double fittedShape = nanVal();
	if (!_estimateWeibullShapeFromCv(cv, &fittedShape)) {
		setFailure3(sqrerror, alpha, scale);
		return;
	}
	const double fittedScale = sampleMean / std::tgamma(1.0 + 1.0 / fittedShape);
	if (!isFinitePositive(fittedShape) || !isFinitePositive(fittedScale)) {
		setFailure3(sqrerror, alpha, scale);
		return;
	}
	if (alpha != nullptr) { *alpha = fittedShape; }
	if (scale != nullptr) { *scale = fittedScale; }
	_setSse(sqrerror, [this, fittedShape, fittedScale](double x) {
		return _weibullCdf(x, fittedShape, fittedScale);
	});
}

void FitterDefaultImpl::fitAll(double* sqrerror, std::string* name) {
	const FitSummary summary = fitAllSummary();
	if (sqrerror != nullptr) { *sqrerror = summary.bestFit.squaredError; }
	if (name     != nullptr) { *name     = summary.bestFit.distributionName; }
}

FitSummary FitterDefaultImpl::fitAllSummary() {
	FitSummary summary;
	summary.bestFit.distributionName = "invalid-dataset";
	summary.bestFit.squaredError = std::numeric_limits<double>::infinity();
	summary.bestFit.message = "invalid dataset";
	if (!_hasUsableDataset()) {
		return summary;
	}

	double error = std::numeric_limits<double>::infinity();
	double p1 = nanVal();
	double p2 = nanVal();
	double p3 = nanVal();
	double p4 = nanVal();

	fitUniform(&error, &p1, &p2);
	summary.ranking.push_back(fittingResult("uniform", error, {
		parameter("min", p1),
		parameter("max", p2)
	}));

	fitTriangular(&error, &p1, &p2, &p3);
	summary.ranking.push_back(fittingResult("triangular", error, {
		parameter("min", p1),
		parameter("mode", p2),
		parameter("max", p3)
	}));

	fitNormal(&error, &p1, &p2);
	summary.ranking.push_back(fittingResult("normal", error, {
		parameter("mean", p1),
		parameter("stddev", p2)
	}));

	fitExpo(&error, &p1);
	summary.ranking.push_back(fittingResult("exponential", error, {
		parameter("mean", p1)
	}));

	fitErlang(&error, &p1, &p2);
	summary.ranking.push_back(fittingResult("erlang", error, {
		parameter("mean", p1),
		parameter("m", p2)
	}));

	fitBeta(&error, &p1, &p2, &p3, &p4);
	summary.ranking.push_back(fittingResult("beta", error, {
		parameter("alpha", p1),
		parameter("beta", p2),
		parameter("infLimit", p3),
		parameter("supLimit", p4)
	}));

	fitWeibull(&error, &p1, &p2);
	summary.ranking.push_back(fittingResult("weibull", error, {
		parameter("alpha", p1),
		parameter("scale", p2)
	}));

	std::stable_sort(summary.ranking.begin(), summary.ranking.end(), [](const FittingResult& lhs, const FittingResult& rhs) {
		if (lhs.success != rhs.success) {
			return lhs.success;
		}
		return lhs.squaredError < rhs.squaredError;
	});

	if (!summary.ranking.empty() && summary.ranking.front().success) {
		summary.success = true;
		summary.bestFit = summary.ranking.front();
	} else {
		summary.bestFit.distributionName = "no-valid-fit";
		summary.bestFit.squaredError = std::numeric_limits<double>::infinity();
		summary.bestFit.message = "no valid fit";
	}
	return summary;
}

void FitterDefaultImpl::setDataFilename(std::string dataFilename) {
	_dataFilename = dataFilename;
	_dataset.loadFromFile(_dataFilename);
}

bool FitterDefaultImpl::setData(const std::vector<double>& data) {
	return setData(data, "");
}

bool FitterDefaultImpl::setData(const std::vector<double>& data, std::string dataFilename) {
	if (!_dataset.loadFromVector(data)) {
		return false;
	}
	_dataFilename = dataFilename;
	return true;
}

std::string FitterDefaultImpl::getDataFilename() {
	return _dataFilename;
}

// --- Private helpers ---

bool FitterDefaultImpl::_hasUsableDataset() const {
	return _dataset.isUsable();
}

void FitterDefaultImpl::_setSse(double* sqrerror, const std::function<double(double)>& cdf) {
	if (sqrerror == nullptr) { return; }
	const std::vector<double>& sortedData = _dataset.sortedData();
	if (sortedData.empty()) {
		*sqrerror = std::numeric_limits<double>::infinity();
		return;
	}
	double sse = 0.0;
	const std::size_t n = sortedData.size();
	for (std::size_t i = 0; i < n; ++i) {
		const double pi    = (static_cast<double>(i) + 0.5) / static_cast<double>(n);
		double model = cdf(sortedData[i]);
		if (!std::isfinite(model)) {
			*sqrerror = std::numeric_limits<double>::infinity();
			return;
		}
		model = clampUnit(model);
		const double d = model - pi;
		sse += d * d;
	}
	*sqrerror = sse;
}

double FitterDefaultImpl::_betaPdfSafe(double x, double alpha, double beta) {
	if (!(isFinitePositive(alpha) && isFinitePositive(beta))) {
		return std::numeric_limits<double>::quiet_NaN();
	}
	constexpr double eps = 1e-12;
	const double xc  = std::max(eps, std::min(1.0 - eps, x));
	const double pdf = ProbabilityDistributionBase::beta(xc, alpha, beta);
	if (!std::isfinite(pdf) || pdf < 0.0) {
		return std::numeric_limits<double>::quiet_NaN();
	}
	return pdf;
}

double FitterDefaultImpl::_betaCdfScaled(double x, double alpha, double beta, double infLimit, double supLimit) const {
	if (!(isFinitePositive(alpha) && isFinitePositive(beta))) { return nanVal(); }
	if (!std::isfinite(infLimit) || !std::isfinite(supLimit) || !(supLimit > infLimit)) { return nanVal(); }
	if (x <= infLimit) { return 0.0; }
	if (x >= supLimit) { return 1.0; }
	const double y = (x - infLimit) / (supLimit - infLimit);
	if (y <= 0.0) { return 0.0; }
	if (y >= 1.0) { return 1.0; }
	SolverDefaultImpl1 solver(1e-8, 10000);
	const double cdf = solver.integrate(0.0, y, &_betaPdfSafe, alpha, beta);
	if (!std::isfinite(cdf)) { return nanVal(); }
	return clampUnit(cdf);
}

double FitterDefaultImpl::_weibullCdf(double x, double alpha, double scale) const {
	if (!(isFinitePositive(alpha) && isFinitePositive(scale))) { return nanVal(); }
	if (x < 0.0) { return 0.0; }
	const double p = std::pow(x / scale, alpha);
	if (!std::isfinite(p)) { return nanVal(); }
	return clampUnit(1.0 - std::exp(-p));
}

bool FitterDefaultImpl::_estimateScaledBetaMoments(double* alpha, double* beta, double* infLimit, double* supLimit) {
	if (!_hasUsableDataset() || _dataset.count() < 2 || !(_dataset.max() > _dataset.min()) || !(_dataset.variance() > 0.0)) {
		return false;
	}
	const std::vector<double>& data = _dataset.data();
	const double a     = _dataset.min();
	const double b     = _dataset.max();
	const double range = b - a;
	if (!(range > 0.0) || !std::isfinite(range)) { return false; }

	constexpr double eps = 1e-12;
	const double n = static_cast<double>(data.size());

	double sum = 0.0;
	for (double x : data) {
		sum += std::max(eps, std::min(1.0 - eps, (x - a) / range));
	}
	const double m = sum / n;
	if (!(m > 0.0 && m < 1.0) || !std::isfinite(m)) { return false; }

	double varSum = 0.0;
	for (double x : data) {
		const double yc = std::max(eps, std::min(1.0 - eps, (x - a) / range));
		const double d  = yc - m;
		varSum += d * d;
	}
	const double v = varSum / static_cast<double>(data.size() - 1U);
	if (!(v > 0.0) || !std::isfinite(v)) { return false; }

	const double common = (m * (1.0 - m) / v) - 1.0;
	const double aShape = m * common;
	const double bShape = (1.0 - m) * common;
	if (!(common > 0.0) || !isFinitePositive(aShape) || !isFinitePositive(bShape)) { return false; }

	if (alpha    != nullptr) { *alpha    = aShape; }
	if (beta     != nullptr) { *beta     = bShape; }
	if (infLimit != nullptr) { *infLimit = a;      }
	if (supLimit != nullptr) { *supLimit = b;      }
	return true;
}

bool FitterDefaultImpl::_estimateWeibullShapeFromCv(double cv, double* shape) const {
	if (!(cv > 0.0) || !std::isfinite(cv) || shape == nullptr) { return false; }
	const double cv2 = cv * cv;
	auto g = [cv2](double k) {
		const double g1 = std::tgamma(1.0 + 1.0 / k);
		const double g2 = std::tgamma(1.0 + 2.0 / k);
		if (!(std::isfinite(g1) && std::isfinite(g2) && g1 > 0.0)) {
			return std::numeric_limits<double>::quiet_NaN();
		}
		return (g2 / (g1 * g1)) - 1.0 - cv2;
	};

	double lo  = 0.1;
	double hi  = 100.0;
	double flo = g(lo);
	double fhi = g(hi);
	if (!(std::isfinite(flo) && std::isfinite(fhi))) { return false; }
	if (flo * fhi > 0.0) { return false; }

	for (int i = 0; i < 100; ++i) {
		const double mid  = 0.5 * (lo + hi);
		const double fmid = g(mid);
		if (!std::isfinite(fmid)) { return false; }
		if (std::abs(fmid) < 1e-10 || (hi - lo) < 1e-8) {
			*shape = mid;
			return true;
		}
		if (flo * fmid > 0.0) { lo = mid; flo = fmid; }
		else                  { hi = mid; fhi = fmid; }
	}
	*shape = 0.5 * (lo + hi);
	return isFinitePositive(*shape);
}
