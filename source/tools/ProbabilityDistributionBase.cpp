/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/* 
 * File:   BaseProbabilityDistributionBase.cpp
 * Author: rlcancian
 * 
 * Created on 4 de agosto de 2022, 11:03
 */

#include "ProbabilityDistributionBase.h"
#include <cmath>
#include <cassert>

double ProbabilityDistributionBase::beta(double x, double alpha, double beta) {
	double x1 = _gammaFunction(alpha + beta) / (_gammaFunction(alpha) * _gammaFunction(beta));
	//double x1 = 1/_betaFunction(alpha,beta);
	double x2 = pow(x, alpha - 1) * pow(1 - x, beta - 1);
	return x1*x2;
}

double ProbabilityDistributionBase::chi2(double x, double degreeFreedom) {
	if (x >= 0) {
		double x1 = pow(x, degreeFreedom / 2 - 1) * pow(M_E, -x / 2);
		double x2 = pow(2, degreeFreedom / 2) * _gammaFunction(degreeFreedom / 2);
		return x1 / x2;
	} else
		return 0.0;
}

double ProbabilityDistributionBase::erlang(double x, double shape, double scale) {
	assert(x >= 0.0 && scale > 0.0 && shape > 0.0);
	double x1 = pow(x, shape - 1) * pow(M_E, -x / scale);
	double x2 = pow(scale, shape) * _gammaFunction(shape);
	return x1 / x2;
} // int M

double ProbabilityDistributionBase::exponential(double x, double mean) {
	if (x >= 0) {
		return mean * pow(M_E, -mean * x);
	} else
		return 0.0;
}

double ProbabilityDistributionBase::fisherSnedecor(double x, double d1, double d2) {
	assert(x >= 0);
	double x1 = 1 / _betaFunction(d1 / 2, d2 / 2); //@TODO: Reimplement using _gammaFunction, but NOT _betaFunction
	double x2 = pow(d1 / d2, d1 / 2) * pow(x, d1 / 2 - 1);
	double x3 = pow(1 + (d1 / d2) * x, -(d1 + d2) / 2);
	return x1 * x2 * x3;
}

double ProbabilityDistributionBase::gamma(double x, double shape, double scale) {
	if (x < 0.0 || shape <= 0.0 || scale <= 0.0) {
		return 0.0;
	}
	double x1 = pow(x, shape - 1) * pow(M_E, -x / scale);
	double x2 = pow(scale, shape) * _gammaFunction(shape);
	return x1 / x2;
}

double ProbabilityDistributionBase::logNormal(double x, double mean, double stddev) {
	if (x <= 0.0 || stddev <= 0.0) {
		return 0.0;
	}
	double x1 = 1 / (x * stddev * sqrt(2 * M_PI));
	double x2 = exp(-pow(log(x) - mean, 2) / (2 * stddev * stddev));
	return x1 * x2;
}

double ProbabilityDistributionBase::normal(double x, double mean, double stddev) {
	if (stddev <= 0.0) {
		return 0.0;
	}
	double x1 = 1 / (stddev * sqrt(2 * M_PI));
	double x2 = -0.5 * pow((x - mean) / stddev, 2);
	return x1 * pow(M_E, x2);
}

double ProbabilityDistributionBase::poisson(double x, double mean) {
	assert((x - trunc(x)) == 0);
	double x1 = pow(mean, x) * pow(M_E, -mean);
	double x2 = _factorial(x);
	return x1 / x2;
}

double ProbabilityDistributionBase::triangular(double x, double min, double mode, double max) {
	if (x < min || x > max)
		return 0.0;
	else if (x < mode)
		return 2 * (x - min) / ((max - min)*(mode - min));
	else if (x == mode)
		return 2 / (max - min);
	else
		return 2 * (max - x) / ((max - min)*(max - mode));
}

double ProbabilityDistributionBase::tStudent(double x, double mean, double stddev, double degreeFreedom) {
	if (stddev <= 0.0 || degreeFreedom <= 0.0) {
		return 0.0;
	}
	const double z = (x - mean) / stddev;
	if (degreeFreedom <= 341) { // threshold based on numeric tests for the gamma function to provide valid (not NaN) results
		double x1 = _gammaFunction((degreeFreedom + 1) / 2) / (sqrt(degreeFreedom * M_PI) * _gammaFunction(degreeFreedom / 2));
		double x2 = pow(1 + (z * z) / degreeFreedom, -(degreeFreedom + 1) / 2);
		return (x1 * x2) / stddev;
	} else {
		return normal(x, mean, stddev);
	}
}

double ProbabilityDistributionBase::uniform(double x, double min, double max) {
	if (x < min || x > max || min >= max) {
		return 0.0;
	}
	return 1.0 / (max - min);
}

double ProbabilityDistributionBase::weibull(double x, double shape, double scale) {
	assert(shape >= 0 && scale >= 0);
	if (x > 0) {
		double x1 = (shape / scale) * pow(x / scale, shape - 1);
		double x2 = pow(M_E, -pow(x / scale, shape));
		return x1 * x2;
	} else
		return 0.0;
}


//************************

double ProbabilityDistributionBase::_gammaFunction(double x) {
	return tgamma(x); ///@TODO: Implement by myself
}

double ProbabilityDistributionBase::_betaFunction(double x, double y) {
	//return std::beta(x, y);
	double x1, x2;
	x1 = _gammaFunction(x) * _gammaFunction(y);
	x2 = _gammaFunction(x + y);
	return x1 / x2;
}

double ProbabilityDistributionBase::_factorial(double x) {
	if (x > 1)
		return x * _factorial(x - 1);
	else
		return 1;
}
