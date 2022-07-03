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

#include <cmath>
#include <complex>
#include <cassert>
#include <random>

#include "SamplerDefaultImpl1.h"

//using namespace GenesysKernel;

SamplerDefaultImpl1::SamplerDefaultImpl1() {
	reset();
}

void SamplerDefaultImpl1::reset() {
	_xi = static_cast<DefaultImpl1RNG_Parameters*> (_param)->seed;
	_normalflag = false;
}

void SamplerDefaultImpl1::setRNGparameters(Sampler_if::RNG_Parameters * param) {

	_param = param; // there is a better solution for this...
	reset();
}

Sampler_if::RNG_Parameters * SamplerDefaultImpl1::getRNGparameters() const {
	return _param;
}

double SamplerDefaultImpl1::random() {
	unsigned long a = static_cast<DefaultImpl1RNG_Parameters*> (_param)->a;
	unsigned long m = static_cast<DefaultImpl1RNG_Parameters*> (_param)->m;
	_xi = (uint64_t) _xi * a % m; //@TODO: Check the target-architecture and choose 64 or 32 bits
	//unsigned long c = static_cast<DefaultImpl1RNG_Parameters*> (_param)->c;
	// _xi = ((uint64_t)_xi * a + c) % m;
	return (double) (_xi) / (double) (m);
}

double SamplerDefaultImpl1::sampleUniform(double min, double max) {
	return min + (max - min) * random();
}

double SamplerDefaultImpl1::sampleExponential(double mean) {
	return mean * (-std::log(random()));
}

double SamplerDefaultImpl1::sampleErlang(double mean, int M) {
	int i;
	double P;
	assert((mean >= 0.0) && (M > 0));
	P = 1;
	for (i = 1; i <= M; i++) {
		P *= random();
	}
	return (mean / M) * (-log(P));
}

double SamplerDefaultImpl1::sampleNormal(double mean, double stddev) {
	double z;
	if (_normalflag) {
		z = _lastnormal;
	} else {
		double u1 = random(), u2 = random();
		z = std::sqrt(-2 * std::log(u1)) * std::sin(2 * M_PI * u2);
		_lastnormal = std::sqrt(-2 * std::log(u1)) * std::cos(2 * M_PI * u2);
	}
	_normalflag = !_normalflag;
	return mean + stddev*z;
}

double SamplerDefaultImpl1::sampleLogNormal(double mean, double stddev) {
	double meanNorm, DispNorm;
	//assert(!((mean <= 0.0) || (stddev <= 0.0)));
	DispNorm = log((stddev * stddev) / (mean * mean) + 1.0);
	meanNorm = log(mean) - 0.5 * DispNorm;
	return exp(sampleNormal(meanNorm, sqrt(DispNorm)));
}

double SamplerDefaultImpl1::sampleTriangular(double min, double mode, double max) {
	double Part1, Part2, Full, R;
	assert(!((min > mode) || (max < mode) || (min > max)));
	Part1 = mode - min;
	Part2 = max - mode;
	Full = max - min;
	R = random();
	if (R <= Part1 / Full)
		return min + sqrt(Part1 * Full * R);
	else
		return max - sqrt(Part2 * Full * (1.0 - R));
}

double SamplerDefaultImpl1::sampleDiscrete(double prob, double value, ...) {
	// @TODO: to implement
	return 0.0;
}

double SamplerDefaultImpl1::sampleDiscrete(double *prob, double *value, int size) {
	double cdf = 0;
	double x;
	x = random();
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

double SamplerDefaultImpl1::sampleGamma(double alpha, double beta) {
	double u, v, w, delta;
	double eps;
	double nt;
	int n;
	n = floor(alpha);
	delta = alpha - n;
	while (1) {
		u = random();
		v = random();
		w = random();
		if (u <= (M_E / (M_E + delta))) {
			eps = pow(v, 1 / delta);
			nt = w * pow(eps, delta - 1);
		} else {
			eps = 1 - log(v);
			nt = w * exp(-eps);
		}
		if (nt > (pow(eps, delta - 1) * exp(-eps))) {
			continue;
		}
		break;
	}
	double gamma_n = 0;
	for (int i = 0; i < n; i++) {
		gamma_n += log(random());
	}
	double gamma = beta * (eps - gamma_n);
	return gamma;
}

double SamplerDefaultImpl1::sampleBeta(double alpha, double beta, double infLimit, double supLimit) {
	double X = sampleBeta(alpha, beta);
	return infLimit + (supLimit - infLimit) * X;
}

double SamplerDefaultImpl1::sampleWeibull(double alpha, double scale) {
	assert(!((alpha <= 0.0) || (scale <= 0.0)));
	return exp(log(scale * (-log(random()))) / alpha);
}

double SamplerDefaultImpl1::sampleBinomial(int trials, double p) {
	double binomial = 0.0;
	double U;

	for (int i = 0; i < trials; i++) {
		U = random();
		if (U < p) {
			binomial += 1.0;
		}
	}

	return binomial;
}

double SamplerDefaultImpl1::sampleBernoulli(double p) {
	double U;

	U = random();
	if (U <= p) {
		return 1.0;
	}
	return 0.0;
}

double SamplerDefaultImpl1::sampleGeometric(double p) {
	assert(p > 0 && p <= 1);
	double rand = random();

	return ceil(log(1 - rand) / log(1 - p));
}

double SamplerDefaultImpl1::sampleGumbell(double mode, double scale) {
	double x;
	x = random();
	return mode - (scale * log(-log(x)));
}

double SamplerDefaultImpl1::sampleBeta(double alpha, double beta) {

	double x = sampleGamma(alpha, 1);
	double y = sampleGamma(beta, 1);
	return x / (x + y);

}

