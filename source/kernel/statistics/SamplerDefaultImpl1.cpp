/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   SamplerDefaultImpl1.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 2 de Agosto de 2018, 01:10
 * 22/10/2019 old genesys code reinserted
 */

#include <cassert>
#include <cstdarg>
#include <cmath>
#include <complex>
#include <random>
#include <stdexcept>

#include "SamplerDefaultImpl1.h"

namespace {

constexpr double kTwoPi = 6.283185307179586476925286766559;

SamplerDefaultImpl1::DefaultImpl1RNG_Parameters* asDefaultParams(Sampler_if::RNG_Parameters* base) {
	return static_cast<SamplerDefaultImpl1::DefaultImpl1RNG_Parameters*> (base);
}

const SamplerDefaultImpl1::DefaultImpl1RNG_Parameters* asDefaultParams(const Sampler_if::RNG_Parameters* base) {
	return static_cast<const SamplerDefaultImpl1::DefaultImpl1RNG_Parameters*> (base);
}

// Generate values in (0, 1) to avoid singularities in log/Box-Muller transforms.
double randomOpen01(SamplerDefaultImpl1& sampler) {
	double u;
	do {
		u = sampler.random();
	} while (u <= 0.0 || u >= 1.0);
	return u;
}

}

//using namespace GenesysKernel;

SamplerDefaultImpl1::SamplerDefaultImpl1() {
	reset();
}

SamplerDefaultImpl1::~SamplerDefaultImpl1() {
	if (_ownsParam) {
		delete _param;
	}
	_param = nullptr;
}

void SamplerDefaultImpl1::reset() {
	const auto* params = asDefaultParams(_param);
	_xi = params->seed;
	_normalflag = false;
}

void SamplerDefaultImpl1::setRNGparameters(Sampler_if::RNG_Parameters * param) {
	if (_param == param || param == nullptr) {
		return;
	}
	if (_ownsParam) {
		delete _param;
	}
	_param = param; // there is a better solution for this...
	_ownsParam = false;
	reset();
}

Sampler_if::RNG_Parameters * SamplerDefaultImpl1::getRNGparameters() const {
	return _param;
}

double SamplerDefaultImpl1::random() {
	const auto* params = asDefaultParams(_param);
	const uint32_t a = params->a;
	const uint32_t m = params->m;
	assert(m > 0);
	_xi = static_cast<uint64_t> (_xi) * a % m;
	return static_cast<double> (_xi) / static_cast<double> (m);
}

double SamplerDefaultImpl1::sampleUniform(double min, double max) {
	if (min > max) {
		throw std::invalid_argument("unif requires min <= max");
	}
	return min + (max - min) * random();
}

double SamplerDefaultImpl1::sampleExponential(double mean, double offset) {
	if (mean < 0.0) {
		throw std::invalid_argument("expo requires mean >= 0");
	}
	return offset + mean * (-std::log(randomOpen01(*this)));
}

double SamplerDefaultImpl1::sampleErlang(double mean, int M, double offset) {
	if (mean < 0.0 || M <= 0) {
		throw std::invalid_argument("erla requires mean >= 0 and M > 0");
	}
	double P = 1.0;
	for (int i = 0; i < M; i++) {
		P *= randomOpen01(*this);
	}
	return offset + (mean / M) * (-std::log(P));
}

double SamplerDefaultImpl1::sampleNormal(double mean, double stddev) {
	if (stddev < 0.0) {
		throw std::invalid_argument("norm requires stddev >= 0");
	}
	double z;
	if (_normalflag) {
		z = _lastnormal;
	} else {
		const double u1 = randomOpen01(*this);
		const double u2 = randomOpen01(*this);
		const double r = std::sqrt(-2.0 * std::log(u1));
		const double theta = kTwoPi * u2;
		z = r * std::sin(theta);
		_lastnormal = r * std::cos(theta);
	}
	_normalflag = !_normalflag;
	return mean + stddev * z;
}

double SamplerDefaultImpl1::sampleLogNormal(double mean, double stddev, double offset) {
	if (mean <= 0.0) {
		throw std::invalid_argument("logn requires mean > 0");
	}
	if (stddev < 0.0) {
		throw std::invalid_argument("logn requires stddev >= 0");
	}
	const double dispersionNorm = std::log((stddev * stddev) / (mean * mean) + 1.0);
	const double meanNorm = std::log(mean) - 0.5 * dispersionNorm;
	return offset + std::exp(sampleNormal(meanNorm, std::sqrt(dispersionNorm)));
}

double SamplerDefaultImpl1::sampleTriangular(double min, double mode, double max) {
	if ((min > mode) || (max < mode) || (min > max)) {
		throw std::invalid_argument("tria requires min <= mode <= max");
	}
	const double part1 = mode - min;
	const double part2 = max - mode;
	const double full = max - min;
	if (full == 0.0) {
		return min;
	}
	const double r = random();
	if (r <= part1 / full)
		return min + std::sqrt(part1 * full * r);
	return max - std::sqrt(part2 * full * (1.0 - r));
}

double SamplerDefaultImpl1::sampleDiscrete(double prob, double value, ...) {
	if (prob <= 0.0) {
		return value;
	}
	if (prob > 1.0) {
		throw std::invalid_argument("disc requires cumulative probabilities in [0,1]");
	}

	const double x = random();
	if (x <= prob || prob >= 1.0) {
		return value;
	}

	double previousProb = prob;
	double selectedValue = value;
	va_list args;
	va_start(args, value);
	do {
		prob = va_arg(args, double);
		selectedValue = va_arg(args, double);
		if (prob < previousProb || prob > 1.0) {
			va_end(args);
			throw std::invalid_argument("disc requires nondecreasing cumulative probabilities ending at 1");
		}
		if (x <= prob) {
			va_end(args);
			return selectedValue;
		}
		previousProb = prob;
	} while (prob < 1.0);
	va_end(args);
	return selectedValue;
}

double SamplerDefaultImpl1::sampleDiscrete(double *prob, double *value, int size) {
	assert(prob != nullptr);
	assert(value != nullptr);
	assert(size > 0);
	double cdf = 0.0;
	const double x = random();
	for (int i = 0; i < size; i++) {
		cdf += prob[i];
		if (x <= cdf) {
			return value[i];
		}
	}
	return value[size - 1];
}

/*
double SamplerDefaultImpl1::_gammaJonk(double alpha) {
	double R;
	double R1, R2, X, Y;
	do {
		do {
			R1 = random();
			R2 = random();
		} while (!((R1 > 1e-30) and (R2 > 1e-30)));
		if (log(R2) / alpha < -1e3)
			X = 0;
		else
			X = exp(log(R2) / alpha);
		if ((log(R1) / (1 - alpha) < -1e3))
			Y = 0;
		else
			Y = exp(log(R1) / (1 - alpha));
	} while (!(X + Y <= 1));
	do {
		R = random();
	} while (!(R > 1e-20));
	return -log(R) * X / (Y + X);
}

double SamplerDefaultImpl1::sampleGamma(double mean, double alpha) {
	int i;
	double P;
	int IntAlpha;
	double OstAlpha;
	assert(!((mean <= 0.0) || (alpha <= 0.0)));
	if (alpha < 1.0)
		return (mean / alpha) * _gammaJonk(alpha);
	else {
		if (alpha == 1.0)
			return mean * (-log(random()));
		else {
			IntAlpha = round(alpha);
			OstAlpha = alpha - IntAlpha;
			do {
				P = 1;
				for (i = 1; i <= IntAlpha; i++)
					P *= random();
			} while (!(P > 0));
			if (OstAlpha > 0)
				return (mean / alpha)*((-log(P)) + _gammaJonk(OstAlpha));
			else
				return (mean / alpha)*(-log(P));
		};
	};
}
 */

double SamplerDefaultImpl1::sampleGamma(double alpha, double beta, double offset) {
	if (alpha <= 0.0 || beta <= 0.0) {
		throw std::invalid_argument("gamm requires alpha > 0 and beta > 0");
	}

	if (alpha < 1.0) {
		const double g = sampleGamma(alpha + 1.0, beta);
		const double u = randomOpen01(*this);
		return offset + g * std::pow(u, 1.0 / alpha);
	}

	const double d = alpha - 1.0 / 3.0;
	const double c = 1.0 / std::sqrt(9.0 * d);

	while (true) {
		double x;
		double v;
		do {
			x = sampleNormal(0.0, 1.0);
			v = 1.0 + c * x;
		} while (v <= 0.0);

		v = v * v * v;
		const double u = randomOpen01(*this);
		const double x2 = x * x;
		if (u < 1.0 - 0.0331 * x2 * x2) {
			return offset + beta * d * v;
		}
		if (std::log(u) < 0.5 * x2 + d * (1.0 - v + std::log(v))) {
			return offset + beta * d * v;
		}
	}
}

double SamplerDefaultImpl1::sampleBeta(double alpha, double beta, double infLimit, double supLimit) {
	if (alpha <= 0.0 || beta <= 0.0) {
		throw std::invalid_argument("beta requires alpha > 0 and beta > 0");
	}
	if (infLimit > supLimit) {
		throw std::invalid_argument("beta requires infLimit <= supLimit");
	}
	double X = sampleBeta(alpha, beta);
	return infLimit + (supLimit - infLimit) * X;
}

double SamplerDefaultImpl1::sampleWeibull(double alpha, double scale) {
	if ((alpha <= 0.0) || (scale <= 0.0)) {
		throw std::invalid_argument("weib requires alpha > 0 and scale > 0");
	}
	return std::exp(std::log(scale * (-std::log(randomOpen01(*this)))) / alpha);
}

double SamplerDefaultImpl1::sampleBinomial(int trials, double p) {
	assert(trials >= 0);
	assert(p >= 0.0 && p <= 1.0);
	double binomial = 0.0;

	for (int i = 0; i < trials; i++) {
		if (random() < p) {
			binomial += 1.0;
		}
	}

	return binomial;
}

double SamplerDefaultImpl1::sampleBernoulli(double p) {
	assert(p >= 0.0 && p <= 1.0);
	if (random() <= p) {
		return 1.0;
	}
	return 0.0;
}

double SamplerDefaultImpl1::sampleGeometric(double p) {
	assert(p > 0.0 && p <= 1.0);
	if (p == 1.0) {
		return 1.0;
	}
	const double u = randomOpen01(*this);
	return std::ceil(std::log(1.0 - u) / std::log(1.0 - p));
}

double SamplerDefaultImpl1::sampleGumbell(double mode, double scale) {
	assert(scale > 0.0);
	const double x = randomOpen01(*this);
	return mode - (scale * std::log(-std::log(x)));
}

double SamplerDefaultImpl1::sampleBeta(double alpha, double beta) {
	if (alpha <= 0.0 || beta <= 0.0) {
		throw std::invalid_argument("beta requires alpha > 0 and beta > 0");
	}
	const double x = sampleGamma(alpha, 1.0);
	const double y = sampleGamma(beta, 1.0);
	return x / (x + y);
}
