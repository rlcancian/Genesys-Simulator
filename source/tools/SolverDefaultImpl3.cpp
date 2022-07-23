#include "SolverDefaultImpl3.h"
#include <math.h>
#include <map>
#include <string>

SolverDefaultImpl3::SolverDefaultImpl3(double precision, unsigned int steps) {
	_precision = precision;
	_numSteps = steps;
}

void SolverDefaultImpl3::setPrecision(double precision) {
	_precision = precision;
}

double SolverDefaultImpl3::getPrecision() {
	return _precision;
}

void SolverDefaultImpl3::setMaxSteps(double steps) {
	_numSteps = steps;
}

double SolverDefaultImpl3::getMaxSteps() {
	return _numSteps;
}

double SolverDefaultImpl3::integrate(double min, double max, double (*f)(double, double), double p2) {
	std::string key = std::to_string((int)f) + " " + std::to_string(p2);
    if (cachedResults.find(key) != cachedResults.end())
        return cachedResults[key];
    // Simpson's 1/3 rule
	double steps = _numSteps;
	double h = (max - min) / steps; // distance between points
	if (h < _precision) {
		steps = (max - min) / _precision;
		h = (max - min) / steps;
	}
	double x, c; // x is the point being evaluated, c is the weight
	double sum = 0.0;
	for (int i = 0; i <= steps; i++) {
		if (i == 0 || i == steps)
			c = 1;
		else if (i % 2 == 0)
			c = 2;
		else
			c = 4;
		x = min + i*h;
		sum += c * f(x, p2);
	}
    double res = (h / 3)*sum;
    cachedResults.insert(std::make_pair(key, res));
    return res;
}

double SolverDefaultImpl3::integrate(double min, double max, double (*f)(double, double, double), double p2, double p3) {
	std::string key = std::to_string((int)f) + " " + std::to_string(p2) + " " + std::to_string(p3);
    if (cachedResults.find(key) != cachedResults.end())
        return cachedResults[key];
    // Simpson's 1/3 rule
	unsigned int steps = _numSteps;
	double h = (max - min) / steps; // distance between points
	if (h < _precision) {
		steps = ceil((max - min) / _precision);
		h = (max - min) / steps;
	}
	double x, c; // x is the point being evaluated, c is the weight
	double sum = 0.0;
	for (unsigned int i = 0; i <= steps; i++) {
		if (i == 0 || i == steps)
			c = 1;
		else if (i % 2 == 0)
			c = 2;
		else
			c = 4;
		x = min + i*h;
		sum += c * f(x, p2, p3);
	}
	double res = (h / 3)*sum;
    cachedResults.insert(std::make_pair(key, res));
    return res;
}

double SolverDefaultImpl3::integrate(double min, double max, double (*f)(double, double, double, double), double p2, double p3, double p4) {
	std::string key = std::to_string((int)f) + " " + std::to_string(p2) + " " + std::to_string(p3) + " " + std::to_string(p4);
    if (cachedResults.find(key) != cachedResults.end())
        return cachedResults[key];
    // Simpson's 1/3 rule
	double steps = _numSteps;
	double h = (max - min) / steps; // distance between points
	if (h < _precision) {
		steps = (max - min) / _precision;
		h = (max - min) / steps;
	}
	double x, c; // x is the point being evaluated, c is the weight
	double sum = 0.0;
	for (int i = 0; i <= steps; i++) {
		if (i == 0 || i == steps)
			c = 1;
		else if (i % 2 == 0)
			c = 2;
		else
			c = 4;
		x = min + i*h;
		sum += c * f(x, p2, p3, p4);
	}
	double res = (h / 3)*sum;
    cachedResults.insert(std::make_pair(key, res));
    return res;
}

double SolverDefaultImpl3::integrate(double min, double max, double (*f)(double, double, double, double, double), double p2, double p3, double p4, double p5) {
	std::string key = std::to_string((int)f) + " " + std::to_string(p2) + " " + std::to_string(p3) + " " + std::to_string(p4) + " " + std::to_string(p5);
    if (cachedResults.find(key) != cachedResults.end())
        return cachedResults[key];
    // Simpson's 1/3 rule
	double steps = _numSteps;
	double h = (max - min) / steps; // distance between points
	if (h < _precision) {
		steps = (max - min) / _precision;
		h = (max - min) / steps;
	}
	double x, c; // x is the point being evaluated, c is the weight
	double sum = 0.0;
	for (int i = 0; i <= steps; i++) {
		if (i == 0 || i == steps)
			c = 1;
		else if (i % 2 == 0)
			c = 2;
		else
			c = 4;
		x = min + i*h;
		sum += c * f(x, p2, p3, p4, p5);
	}
	double res = (h / 3)*sum;
    cachedResults.insert(std::make_pair(key, res));
    return res;
}

double SolverDefaultImpl3::derivate(double initPoint, double initValue, double (*f)(double, double), double p2) {
	double time, halfStep;
	unsigned int i, numEqs = 1;
	double k1[numEqs], k2[numEqs], k3[numEqs], k4[numEqs], valVar[numEqs], result[numEqs];
	time = initPoint;
	halfStep = _stepSize * 0.5;
	for (i = 0; i < numEqs; i++) {
		k1[i] = f(time, p2);
	}
	time += halfStep;
	for (i = 0; i < numEqs; i++) {
		k2[i] = f(time, p2 + k1[i] * halfStep);
	}
	for (i = 0; i < numEqs; i++) {
		k3[i] = f(time, p2 + k2[i] * halfStep);
	}
	for (i = 0; i < numEqs; i++) {
		k4[i] = f(time, p2 + k3[i] * halfStep);
	}
	for (i = 0; i < numEqs; i++) {
		result[i] = p2 + (_stepSize / 6) * (k1[i] + 2 * (k2[i] + k3[i]) + k4[i]);
	}
	time = initPoint + _stepSize; // OR  time += halfStep;
	return result[0];

}

double SolverDefaultImpl3::derivate(double initPoint, double initValue, double (*f)(double, double, double), double p2, double p3) {
	/*  @TODO: +-: not implemented yet */
	return 0.0;
}

double SolverDefaultImpl3::derivate(double initPoint, double initValue, double (*f)(double, double, double, double), double p2, double p3, double p4) {
	/*  @TODO: +-: not implemented yet */
	return 0.0;
}

double SolverDefaultImpl3::derivate(double initPoint, double initValue, double (*f)(double, double, double, double, double), double p2, double p3, double p4, double p5) {
	/*  @TODO: +-: not implemented yet */
	return 0.0;
}
