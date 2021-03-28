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

#include "SamplerDefaultImpl1.h"

//using namespace GenesysKernel;

SamplerDefaultImpl1::SamplerDefaultImpl1() {
	reset();
}

void SamplerDefaultImpl1::reset() {
	_xi = static_cast<DefaultImpl1RNG_Parameters*> (_param)->seed;
	//_normalflag = true;
}

double SamplerDefaultImpl1::random() {
	double module = (double) static_cast<DefaultImpl1RNG_Parameters*> (_param)->module;
	_xi *= static_cast<DefaultImpl1RNG_Parameters*> (_param)->multiplier;
	_xi -= std::trunc((double) _xi / module) * module;
	return (double) _xi / (double) static_cast<DefaultImpl1RNG_Parameters*> (_param)->module;
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
	double z = std::sqrt(-2 * std::log(random())) * std::cos(2 * M_PI * random());
	return mean + stddev*z;
}

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

double SamplerDefaultImpl1::sampleBeta(double alpha, double beta, double infLimit, double supLimit) {
	double X, Y1, Y2;
	assert(!((alpha <= 0.0) || (beta <= 0.0) || (infLimit > supLimit) || (infLimit < 0) || (supLimit < 0)));
	do {
		Y1 = sampleGamma(alpha, alpha);
		Y2 = sampleGamma(beta, beta);
		X = Y1 / (Y1 + Y2);
	} while (!((X >= 0) && (X <= 1.0)));
	return infLimit + (supLimit - infLimit) * X;
}

double SamplerDefaultImpl1::sampleWeibull(double alpha, double scale) {
	assert(!((alpha <= 0.0) || (scale <= 0.0)));
	return exp(log(scale * (-log(random()))) / alpha);
}

double SamplerDefaultImpl1::sampleLogNormal(double mean, double stddev) {
	double meanNorm, DispNorm;
	assert(!((mean <= 0.0) || (stddev <= 0.0)));
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

double SamplerDefaultImpl1::sampleDiscrete(double acumProb, double value, ...) {
	// \todo: to implement
	return 0.0;
}

/* Discrete Distributions: Binomal and Geometric */

double SamplerDefaultImpl1::sampleBinomial(int trials){
	double binomial = 0.0;
	double U;

	for(int i = 0; i < trials; i++){
		U = sampleUniform(0.0, 1.0);
		if(U < 0.5){
			binomial += 1.0;
		}
	}

	return binomial;
}

double SamplerDefaultImpl1::sampleGeometric(double k, double p){
	assert(p > 0 && p <= 1);

	return pow((1-p),(k-1))*p;
}


double SamplerDefaultImpl1::sampleGumbellInv(double mode, double scale) {
	double x;
	x = random();
	return mode - (scale * log(-log(x)));
}

double SamplerDefaultImpl1::sampleGumbell(double mode, double scale) {
	double x;
	x = random();
	return exp(-exp(-(x-mode)/scale));
}

double SamplerDefaultImpl1::sampleChiSqrt(double degrees) {
	double chi2, z;
	chi2 = 0;
	for (int i=0; i < degrees; i++) {
		z = sampleNormal(0,1);
		chi2 = chi2 + (z*z);
	}
	return chi2;
}

double SamplerDefaultImpl1::gammaFunction(int n) {
    int fact = n - 1;
    for (int i = (fact - 1); n > 1; n--) {
        fact *= i;
    }

    return fact;
}

double SamplerDefaultImpl1::sampleGammaPDF(int alpha, int beta) {
    double x;
    x = random();

    return (pow(beta,alpha)*pow(x,alpha-1)*exp(-beta*x))/gammaFunction(alpha);
}

double SamplerDefaultImpl1::betaFunction(int y, int x) {
    return gammaFunction(x)*gammaFunction(y)/gammaFunction(x+y);
}

double SamplerDefaultImpl1::sampleBetaPDF(int alpha, int beta) {
    double x;
    x = random();

    return (pow(x,alpha-1)*pow(1-x, beta-1))/betaFunction(alpha, beta);
}

void SamplerDefaultImpl1::setRNGparameters(Sampler_if::RNG_Parameters * param) {

	_param = param; // there is a better solution for this...
}

Sampler_if::RNG_Parameters * SamplerDefaultImpl1::getRNGparameters() const {
	return _param;
}
