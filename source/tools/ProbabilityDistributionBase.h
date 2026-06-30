/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/*
 * File:   BaseProbabilityDistribution.h
 * Author: rlcancian
 *
 * Created on 4 de agosto de 2022, 11:03
 */

#ifndef PROBABILITYDISTRIBUTIONBASE_H
#define PROBABILITYDISTRIBUTIONBASE_H

#include "Solver_if.h"
#include <string>
#include <map>

/**
 * @brief Mathematical base library for distribution-related functions.
 *
 * Architectural role:
 * - Provides static PDF/PMF and helper formulas used by higher-level tools.
 * - Not an OO hierarchy of distribution objects yet.
 *
 * Mathematical meaning:
 * - Public methods represent density/mass functions or mathematically-related
 *   helpers for known distributions.
 */
class ProbabilityDistributionBase {
public:
	/** Beta distribution PDF. */
	static double beta(double x, double alpha, double beta);
	/** Chi-square distribution PDF. */
	static double chi2(double x, double degreeFreedom);
	/** Erlang distribution PDF. */
	static double erlang(double x, double shape, double scale); // int M
	/** Exponential distribution PDF. */
	static double exponential(double x, double mean);
	/** Fisher-Snedecor F distribution PDF. */
	static double fisherSnedecor(double x, double d1, double d2);
	/** Gamma distribution PDF. */
	static double gamma(double x, double shape, double scale);
	/** Log-normal distribution PDF. */
	static double logNormal(double x, double mean, double stddev);
	/** Normal distribution PDF. */
	static double normal(double x, double mean, double stddev);
	/** Poisson distribution PMF. */
	static double poisson(double x, double mean);
	/** Triangular distribution PDF. */
	static double triangular(double x, double min, double mode, double max);
	/** Student t distribution PDF. */
	static double tStudent(double x, double mean, double stddev, double degreeFreedom);
	/** Uniform distribution PDF. */
	static double uniform(double x, double min, double max);
	/** Weibull distribution PDF. */
	static double weibull(double x, double shape, double scale);
protected:
	/** Gamma special function approximation/helper. */
	static double _gammaFunction(double x);
	/** Beta special function helper. */
	static double _betaFunction(double x, double y);
	/** Factorial helper, typically used by PMFs. */
	static double _factorial(double x);
};

#endif /* PROBABILITYDISTRIBUTIONBASE_H */
