/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/*
 * File:   ProbabilityDistribution.cpp
 * Author: rlcancian
 *
 * Created on 22 de novembro de 2021, 17:24
 */

#include "ProbabilityDistribution.h"
#include <cmath>
#include <cassert>
#include "TraitsTools.h"

std::map<std::string, double>* ProbabilityDistribution::memory = new std::map<std::string, double>();
Solver_if* ProbabilityDistribution::integrator = new TraitsTools<Solver_if>::Implementation(TraitsTools<Solver_if>::Precision, TraitsTools<Solver_if>::MaxSteps);

double ProbabilityDistribution::inverseFFisherSnedecor(double cumulativeProbability, double d1, double d2) {
	std::string key = "fisher(" + std::to_string(d1) + "," + std::to_string(d2) + ")" + std::to_string(cumulativeProbability);
	auto search = ProbabilityDistribution::memory->find(key);
	if (search != ProbabilityDistribution::memory->end()) { //found
		return search->second;
	} else {
		double a, fa;
		double b, fb;
		a = 0.0;
		fa = 0.0;
		b = 10000.0; ///@TODO: Is there a way to better determine the upper limit?
		unsigned int savedSteps = integrator->getMaxSteps();
		integrator->setMaxSteps(1e4);
		fb = integrator->integrate(a, b, ProbabilityDistributionBase::fisherSnedecor, d1, d2);
		//integrator->setMaxSteps(1e3);
		double inv = ProbabilityDistribution::_findInverseFFisherSnedecor(a, fa, b, fb, 0, cumulativeProbability, d1, d2);
		integrator->setMaxSteps(savedSteps);
		std::pair<std::string, double> pair = std::pair<std::string, double>(key, inv);
		ProbabilityDistribution::memory->insert(pair);
		return inv;
	}
}

double ProbabilityDistribution::inverseChi2(double cumulativeProbability, double m) {
	std::string key = "chi2(" + std::to_string(m) + ")" + std::to_string(cumulativeProbability);
	auto search = ProbabilityDistribution::memory->find(key);
	if (search != ProbabilityDistribution::memory->end()) { //found
		return search->second;
	} else {
		double a, fa;
		double b, fb;
		a = 0.0;
		fa = 0.0;
		b = 1000.0; ///@TODO: Is there a way to better determine the upper limit?
		unsigned int savedSteps = integrator->getMaxSteps();
		integrator->setMaxSteps(1e3);
		fb = integrator->integrate(a, b, ProbabilityDistributionBase::chi2, m);
		//integrator->setMaxSteps(1e3);
		double inv = ProbabilityDistribution::_findInverseChi2(a, fa, b, fb, 0, cumulativeProbability, m);
		integrator->setMaxSteps(savedSteps);
		std::pair<std::string, double> pair = std::pair<std::string, double>(key, inv);
		ProbabilityDistribution::memory->insert(pair);
		return inv;
	}
}

double ProbabilityDistribution::inverseNormal(double cumulativeProbability, double mean, double stddev) {
	std::string key = "normal(" + std::to_string(mean) + "," + std::to_string(stddev) + ")" + std::to_string(cumulativeProbability);
	auto search = ProbabilityDistribution::memory->find(key);
	if (search != ProbabilityDistribution::memory->end()) { //found
		return search->second;
	} else {
		double a, fa;
		double b, fb;
		if (cumulativeProbability <= 0.5) { ///@TODO: Could be better (separate into 1,2,3xstddev)
			a = mean - stddev * 5.0;
			fa = 0.0;
			b = mean;
			fb = 0.5;
		} else {
			a = mean;
			fa = 0.5;
			b = mean + stddev * 5.0;
			fb = 1.0;
		}
		double inv = ProbabilityDistribution::_findInverseNormal(a, fa, b, fb, 0, cumulativeProbability, mean, stddev);
		std::pair<std::string, double> pair = std::pair<std::string, double>(key, inv);
		ProbabilityDistribution::memory->insert(pair);
		return inv;
	}
}

double ProbabilityDistribution::inverseTStudent(double cumulativeProbability, double mean, double stddev, double degreeFreedom) {
	std::string key = "tstudent(" + std::to_string(mean) + "," + std::to_string(stddev) + "," + std::to_string(degreeFreedom) + ")" + std::to_string(cumulativeProbability);
	auto search = ProbabilityDistribution::memory->find(key);
	if (search != ProbabilityDistribution::memory->end()) { //found
		return search->second;
	} else {
		double a, fa;
		double b, fb;
		double inv;
		if (cumulativeProbability <= 0.5) { ///@TODO: Could be better
			a = mean - stddev * 5.0;
			fa = 0.0;
			b = mean;
			fb = 0.5;
		} else {
			a = mean;
			fa = 0.5;
			b = mean + stddev * 5.0;
			fb = 1.0;
		}
		if (degreeFreedom <= 341) {// threshold based on numeric tests for the gamma function to provide valid (not NaN) results
			inv = ProbabilityDistribution::_findInverseTStudent(a, fa, b, fb, 0, cumulativeProbability, mean, stddev, degreeFreedom);
			if (inv != inv) { // is NaN
				inv = ProbabilityDistribution::_findInverseNormal(a, fa, b, fb, 0, cumulativeProbability, mean, stddev);
			}
		} else {
			inv = ProbabilityDistribution::_findInverseNormal(a, fa, b, fb, 0, cumulativeProbability, mean, stddev);
		}
		std::pair<std::string, double> pair = std::pair<std::string, double>(key, inv);
		ProbabilityDistribution::memory->insert(pair);
		return inv;
	}
}

//************************

double ProbabilityDistribution::_findInverseFFisherSnedecor(double a, double fa, double b, double fb, unsigned int recursions, double cumulativeProbability, double d1, double d2) {
	double x = ((cumulativeProbability - fa) / (fb - fa)) * (b - a) + a;
	double fx = fa + integrator->integrate(a, x, ProbabilityDistributionBase::fisherSnedecor, d1, d2);
	++recursions;
	if ((recursions == integrator->getMaxSteps()) || (abs(fx - cumulativeProbability) <= integrator->getPrecision())) {
		return x;
	} else if (fx < cumulativeProbability) {
		return ProbabilityDistribution::_findInverseFFisherSnedecor(x, fx, b, fb, recursions, cumulativeProbability, d1, d2);
	} else {
		return ProbabilityDistribution::_findInverseFFisherSnedecor(a, fa, x, fx, recursions, cumulativeProbability, d1, d2);
	}
}

double ProbabilityDistribution::_findInverseNormal(double a, double fa, double b, double fb, unsigned int recursions, double cumulativeProbability, double mean, double stddev) {
	double x = ((cumulativeProbability - fa) / (fb - fa)) * (b - a) + a;
	double fx = fa + integrator->integrate(a, x, ProbabilityDistributionBase::normal, mean, stddev);
	++recursions;
	if ((recursions == integrator->getMaxSteps()) || (abs(fx - cumulativeProbability) <= integrator->getPrecision())) {
		return x;
	} else if (fx < cumulativeProbability) {
		return ProbabilityDistribution::_findInverseNormal(x, fx, b, fb, recursions, cumulativeProbability, mean, stddev);
	} else {
		return ProbabilityDistribution::_findInverseNormal(a, fa, x, fx, recursions, cumulativeProbability, mean, stddev);
	}
}

double ProbabilityDistribution::_findInverseTStudent(double a, double fa, double b, double fb, unsigned int recursions, double cumulativeProbability, double mean, double stddev, double degreeFreedom) {
	double x = ((cumulativeProbability - fa) / (fb - fa)) * (b - a) + a;
	double fx = fa + integrator->integrate(a, x, ProbabilityDistributionBase::tStudent, mean, stddev, degreeFreedom);
	if (fx != fx) { // check if NaN
		return fx;
	}
	++recursions;
	if ((recursions == integrator->getMaxSteps()) || (abs(fx - cumulativeProbability) <= integrator->getPrecision())) {
		return x;
	} else if (fx < cumulativeProbability) {
		return ProbabilityDistribution::_findInverseTStudent(x, fx, b, fb, recursions, cumulativeProbability, mean, stddev, degreeFreedom);
	} else {
		return ProbabilityDistribution::_findInverseTStudent(a, fa, x, fx, recursions, cumulativeProbability, mean, stddev, degreeFreedom);
	}
}

double ProbabilityDistribution::_findInverseChi2(double a, double fa, double b, double fb, unsigned int recursions, double cumulativeProbability, double degreeFreedom) {
	double x = ((cumulativeProbability - fa) / (fb - fa)) * (b - a) + a;
	double fx = fa + integrator->integrate(a, x, ProbabilityDistributionBase::chi2, degreeFreedom);
	++recursions;
	if ((recursions == integrator->getMaxSteps()) || (abs(fx - cumulativeProbability) <= integrator->getPrecision())) {
		return x;
	} else if (fx < cumulativeProbability) {
		return ProbabilityDistribution::_findInverseChi2(x, fx, b, fb, recursions, cumulativeProbability, degreeFreedom);
	} else {
		return ProbabilityDistribution::_findInverseChi2(a, fa, x, fx, recursions, cumulativeProbability, degreeFreedom);
	}
}
