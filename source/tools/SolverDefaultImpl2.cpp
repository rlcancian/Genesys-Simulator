#include "SolverDefaultImpl2.h"
#include <math.h>
#include <map>
#include <string>

SolverDefaultImpl2::SolverDefaultImpl2(double precision, unsigned int steps) {
	_precision = precision;
	_numSteps = steps;
}

void SolverDefaultImpl2::setPrecision(double precision) {
	_precision = precision;
}

double SolverDefaultImpl2::getPrecision() {
	return _precision;
}

void SolverDefaultImpl2::setMaxSteps(double steps) {
	_numSteps = steps;
}

double SolverDefaultImpl2::getMaxSteps() {
	return _numSteps;
}

double SolverDefaultImpl2::randNumber() {
    _seed = (_seed * 1664525 + 1013904223) & 0xFFFFFFFF;
    return (double) ((1.0 * (_seed >> 16)) / (1.0 * ((1 << 16) - 1))); // any number [0, 1]
}

double SolverDefaultImpl2::integrate(double min, double max, double (*f)(double, double), double p2) {
	std::string key = std::to_string((int)f) + " " + std::to_string(p2);
    if (cachedResults.find(key) != cachedResults.end())
        return cachedResults[key];
    // Fast Monte Carlo integration
    int steps = _numSteps;
    double x;
    double sum = 0;
    double diff = max - min;
    for (int i = 0; i < steps; i++)
    {
        x = (randNumber() * diff) + min;
        sum += f(x, p2);
    }
    double res = diff * (sum / steps);
    cachedResults.insert(std::make_pair(key, res));
    return res;
}

double SolverDefaultImpl2::integrate(double min, double max, double (*f)(double, double, double), double p2, double p3) {
	std::string key = std::to_string((int)f) + " " + std::to_string(p2) + " " + std::to_string(p3);
    if (cachedResults.find(key) != cachedResults.end())
        return cachedResults[key];
    // Fast Monte Carlo integration
    int steps = _numSteps;
    double x;
    double sum = 0;
    double diff = max - min;
    for (int i = 0; i < steps; i++)
    {
        x = (randNumber() * diff) + min;
        sum += f(x, p2, p3);
    }
    double res = diff * (sum / steps);
    cachedResults.insert(std::make_pair(key, res));
    return res;
}

double SolverDefaultImpl2::integrate(double min, double max, double (*f)(double, double, double, double), double p2, double p3, double p4) {
	std::string key = std::to_string((int)f) + " " + std::to_string(p2) + " " + std::to_string(p3) + " " + std::to_string(p4);
    if (cachedResults.find(key) != cachedResults.end())
        return cachedResults[key];
    // Fast Monte Carlo integration
    int steps = _numSteps;
    double x;
    double sum = 0;
    double diff = max - min;
    for (int i = 0; i < steps; i++)
    {
        x = (randNumber() * diff) + min;
        sum += f(x, p2, p3, p4);
    }
    double res = diff * (sum / steps);
    cachedResults.insert(std::make_pair(key, res));
    return res;
}

double SolverDefaultImpl2::integrate(double min, double max, double (*f)(double, double, double, double, double), double p2, double p3, double p4, double p5) {
	std::string key = std::to_string((int)f) + " " + std::to_string(p2) + " " + std::to_string(p3) + " " + std::to_string(p4) + " " + std::to_string(p5);
    if (cachedResults.find(key) != cachedResults.end())
        return cachedResults[key];
    // Fast Monte Carlo integration
    int steps = _numSteps;
    double x;
    double sum = 0;
    double diff = max - min;
    for (int i = 0; i < steps; i++)
    {
        x = (randNumber() * diff) + min;
        sum += f(x, p2, p3, p4, p5);
    }
    double res = diff * (sum / steps);
    cachedResults.insert(std::make_pair(key, res));
    return res;
}

double SolverDefaultImpl2::derivate(double initPoint, double initValue, double (*f)(double, double), double p2) {
    /*  @TODO: +-: not implemented yet */
	return 0.0;
}

double SolverDefaultImpl2::derivate(double initPoint, double initValue, double (*f)(double, double, double), double p2, double p3) {
    /*  @TODO: +-: not implemented yet */
	return 0.0;
}

double SolverDefaultImpl2::derivate(double initPoint, double initValue, double (*f)(double, double, double, double), double p2, double p3, double p4) {
    /*  @TODO: +-: not implemented yet */
	return 0.0;
}

double SolverDefaultImpl2::derivate(double initPoint, double initValue, double (*f)(double, double, double, double, double), double p2, double p3, double p4, double p5) {
    /*  @TODO: +-: not implemented yet */
	return 0.0;
}