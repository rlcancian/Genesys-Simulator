#include "ProbabilityDistribution.h"
#include <algorithm>
#include <cmath>
#include <cassert>
#include <functional>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include "TraitsAnalysis.h"

namespace {

double clampProbability(double value) {
	return std::clamp(value, 0.0, 1.0);
}

void validateCumulativeProbability(double cumulativeProbability) {
	if (!(cumulativeProbability > 0.0 && cumulativeProbability < 1.0) || !std::isfinite(cumulativeProbability)) {
		throw std::invalid_argument("cumulativeProbability must be in (0,1)");
	}
}

std::map<std::string, double>& quantileCache() {
	static std::map<std::string, double> cache;
	return cache;
}

double integrateMidpoint(double lower, double upper, const std::function<double(double)>& pdf) {
	if (!(upper > lower)) {
		return 0.0;
	}
	constexpr unsigned int intervals = TraitsAnalysis<ProbabilityDistribution>::IntegrationIntervals;
	const double width = (upper - lower) / static_cast<double>(intervals);
	double sum = 0.0;
	for (unsigned int i = 0; i < intervals; ++i) {
		const double x = lower + (static_cast<double>(i) + 0.5) * width;
		const double fx = pdf(x);
		if (std::isfinite(fx)) {
			sum += fx;
		}
	}
	return sum * width;
}

double normalCdf(double x, double mean, double stddev) {
	if (!(stddev > 0.0) || !std::isfinite(stddev)) {
		throw std::invalid_argument("normal stddev must be positive and finite");
	}
	return clampProbability(0.5 * std::erfc(-(x - mean) / (stddev * std::sqrt(2.0))));
}

double inverseStandardNormal(double p) {
	// Peter J. Acklam's rational approximation is accurate enough for the
	// confidence intervals and tests implemented in this package.
	static const double a[] = {
		-3.969683028665376e+01,
		 2.209460984245205e+02,
		-2.759285104469687e+02,
		 1.383577518672690e+02,
		-3.066479806614716e+01,
		 2.506628277459239e+00
	};
	static const double b[] = {
		-5.447609879822406e+01,
		 1.615858368580409e+02,
		-1.556989798598866e+02,
		 6.680131188771972e+01,
		-1.328068155288572e+01
	};
	static const double c[] = {
		-7.784894002430293e-03,
		-3.223964580411365e-01,
		-2.400758277161838e+00,
		-2.549732539343734e+00,
		 4.374664141464968e+00,
		 2.938163982698783e+00
	};
	static const double d[] = {
		 7.784695709041462e-03,
		 3.224671290700398e-01,
		 2.445134137142996e+00,
		 3.754408661907416e+00
	};

	const double plow = TraitsAnalysis<ProbabilityDistribution>::NormalTailBreakPoint;
	const double phigh = 1.0 - plow;

	if (p < plow) {
		const double q = std::sqrt(-2.0 * std::log(p));
		return (((((c[0] * q + c[1]) * q + c[2]) * q + c[3]) * q + c[4]) * q + c[5]) /
		       ((((d[0] * q + d[1]) * q + d[2]) * q + d[3]) * q + 1.0);
	}
	if (p <= phigh) {
		const double q = p - 0.5;
		const double r = q * q;
		return (((((a[0] * r + a[1]) * r + a[2]) * r + a[3]) * r + a[4]) * r + a[5]) * q /
		       (((((b[0] * r + b[1]) * r + b[2]) * r + b[3]) * r + b[4]) * r + 1.0);
	}

	const double q = std::sqrt(-2.0 * std::log(1.0 - p));
	return -(((((c[0] * q + c[1]) * q + c[2]) * q + c[3]) * q + c[4]) * q + c[5]) /
	        ((((d[0] * q + d[1]) * q + d[2]) * q + d[3]) * q + 1.0);
}

double chi2Cdf(double x, double degreeFreedom) {
	if (x <= 0.0) {
		return 0.0;
	}
	if (!(degreeFreedom > 0.0) || !std::isfinite(degreeFreedom)) {
		throw std::invalid_argument("chi-square degrees of freedom must be positive and finite");
	}
	if (std::fabs(degreeFreedom - 1.0) < 1e-12) {
		return clampProbability(std::erf(std::sqrt(x / 2.0)));
	}
	if (std::fabs(degreeFreedom - 2.0) < 1e-12) {
		return clampProbability(1.0 - std::exp(-x / 2.0));
	}
	return clampProbability(integrateMidpoint(0.0, x, [degreeFreedom](double value) {
		return ProbabilityDistributionBase::chi2(value, degreeFreedom);
	}));
}

double tStudentCdf(double x, double mean, double stddev, double degreeFreedom) {
	if (!(stddev > 0.0) || !(degreeFreedom > 0.0) || !std::isfinite(stddev) || !std::isfinite(degreeFreedom)) {
		throw std::invalid_argument("t-student parameters must be positive and finite");
	}
	if (degreeFreedom > TraitsAnalysis<ProbabilityDistribution>::TStudentNormalApproximationDegreesFreedom) {
		return normalCdf(x, mean, stddev);
	}
	if (x == mean) {
		return 0.5;
	}
	const double lower = std::min(mean, x);
	const double upper = std::max(mean, x);
	const double integral = integrateMidpoint(lower, upper, [mean, stddev, degreeFreedom](double value) {
		return ProbabilityDistributionBase::tStudent(value, mean, stddev, degreeFreedom);
	});
	return clampProbability((x > mean) ? (0.5 + integral) : (0.5 - integral));
}

double fisherSnedecorCdf(double x, double d1, double d2) {
	if (x <= 0.0) {
		return 0.0;
	}
	if (!(d1 > 0.0) || !(d2 > 0.0) || !std::isfinite(d1) || !std::isfinite(d2)) {
		throw std::invalid_argument("F degrees of freedom must be positive and finite");
	}
	return clampProbability(integrateMidpoint(0.0, x, [d1, d2](double value) {
		return ProbabilityDistributionBase::fisherSnedecor(value, d1, d2);
	}));
}

template<typename Cdf>
double inverseByBisection(double cumulativeProbability, double lower, double upper, Cdf cdf) {
	validateCumulativeProbability(cumulativeProbability);
	double lowerCdf = cdf(lower);
	double upperCdf = cdf(upper);
	unsigned int expansions = 0;
	while (lowerCdf > cumulativeProbability && expansions < TraitsAnalysis<ProbabilityDistribution>::MaxBracketExpansions) {
		upper = lower;
		upperCdf = lowerCdf;
		lower *= 2.0;
		lowerCdf = cdf(lower);
		++expansions;
	}
	while (upperCdf < cumulativeProbability && expansions < TraitsAnalysis<ProbabilityDistribution>::MaxBracketExpansions) {
		lower = upper;
		lowerCdf = upperCdf;
		upper = (upper == 0.0) ? 1.0 : upper * 2.0;
		upperCdf = cdf(upper);
		++expansions;
	}
	if (!(lowerCdf <= cumulativeProbability && upperCdf >= cumulativeProbability)) {
		throw std::runtime_error("Unable to bracket inverse distribution");
	}

	for (unsigned int i = 0; i < TraitsAnalysis<ProbabilityDistribution>::MaxBisectionIterations; ++i) {
		const double middle = 0.5 * (lower + upper);
		const double middleCdf = cdf(middle);
		if (std::fabs(middleCdf - cumulativeProbability) <= TraitsAnalysis<ProbabilityDistribution>::CdfTolerance ||
				std::fabs(upper - lower) <= TraitsAnalysis<ProbabilityDistribution>::QuantileTolerance) {
			return middle;
		}
		if (middleCdf < cumulativeProbability) {
			lower = middle;
			lowerCdf = middleCdf;
		} else {
			upper = middle;
			upperCdf = middleCdf;
		}
	}
	return 0.5 * (lower + upper);
}

} // namespace

double ProbabilityDistribution::inverseFFisherSnedecor(double cumulativeProbability, double d1, double d2) {
	std::string key = "fisher(" + std::to_string(d1) + "," + std::to_string(d2) + ")" + std::to_string(cumulativeProbability);
	auto& cache = quantileCache();
	auto search = cache.find(key);
	if (search != cache.end()) { //found
		return search->second;
	} else {
		validateCumulativeProbability(cumulativeProbability);
		double inv = inverseByBisection(cumulativeProbability, 0.0, 1.0, [d1, d2](double x) {
			return fisherSnedecorCdf(x, d1, d2);
		});
		std::pair<std::string, double> pair = std::pair<std::string, double>(key, inv);
		cache.insert(pair);
		return inv;
	}
}

double ProbabilityDistribution::inverseChi2(double cumulativeProbability, double m) {
	std::string key = "chi2(" + std::to_string(m) + ")" + std::to_string(cumulativeProbability);
	auto& cache = quantileCache();
	auto search = cache.find(key);
	if (search != cache.end()) { //found
		return search->second;
	} else {
		validateCumulativeProbability(cumulativeProbability);
		double inv = inverseByBisection(cumulativeProbability, 0.0, std::max(1.0, m), [m](double x) {
			return chi2Cdf(x, m);
		});
		std::pair<std::string, double> pair = std::pair<std::string, double>(key, inv);
		cache.insert(pair);
		return inv;
	}
}

double ProbabilityDistribution::inverseNormal(double cumulativeProbability, double mean, double stddev) {
	std::string key = "normal(" + std::to_string(mean) + "," + std::to_string(stddev) + ")" + std::to_string(cumulativeProbability);
	auto& cache = quantileCache();
	auto search = cache.find(key);
	if (search != cache.end()) { //found
		return search->second;
	} else {
		validateCumulativeProbability(cumulativeProbability);
		if (!(stddev > 0.0) || !std::isfinite(stddev)) {
			throw std::invalid_argument("normal stddev must be positive and finite");
		}
		double inv = mean + stddev * inverseStandardNormal(cumulativeProbability);
		std::pair<std::string, double> pair = std::pair<std::string, double>(key, inv);
		cache.insert(pair);
		return inv;
	}
}

double ProbabilityDistribution::inverseTStudent(double cumulativeProbability, double mean, double stddev, double degreeFreedom) {
	std::string key = "tstudent(" + std::to_string(mean) + "," + std::to_string(stddev) + "," + std::to_string(degreeFreedom) + ")" + std::to_string(cumulativeProbability);
	auto& cache = quantileCache();
	auto search = cache.find(key);
	if (search != cache.end()) { //found
		return search->second;
	} else {
		validateCumulativeProbability(cumulativeProbability);
		if (!(stddev > 0.0) || !(degreeFreedom > 0.0) || !std::isfinite(stddev) || !std::isfinite(degreeFreedom)) {
			throw std::invalid_argument("t-student parameters must be positive and finite");
		}
		double inv = inverseByBisection(
			cumulativeProbability,
			mean - stddev,
			mean + stddev,
			[mean, stddev, degreeFreedom](double x) {
				return tStudentCdf(x, mean, stddev, degreeFreedom);
			}
		);
		std::pair<std::string, double> pair = std::pair<std::string, double>(key, inv);
		cache.insert(pair);
		return inv;
	}
}
