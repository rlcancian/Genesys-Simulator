/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/*
 * File:   ProbabilityDistribution.h
 * Author: rlcancian
 *
 * Created on 22 de novembro de 2021, 17:24
 */

#ifndef PROBABILITYDISTRIBUTION_H
#define PROBABILITYDISTRIBUTION_H

#include "ProbabilityDistributionBase.h"

/**
 * @brief Static façade for inverse/CDF-related distribution utilities.
 *
 * Architectural role:
 * - Exposes quantile/inverse computations used by hypothesis testing and
 *   confidence-interval routines.
 *
 * Implementation notes:
 * - Uses numerical procedures and internal cache state.
 * - Current static design is legacy-oriented and not the final target model.
 *
 * Planned evolution:
 * - Future versions may delegate responsibilities to Distribution_if,
 *   Quadrature_if and RootFinder_if abstractions.
 */
class ProbabilityDistribution : public ProbabilityDistributionBase {
public:
	static double inverseChi2(double cumulativeProbability, double degreeFreedom);
	static double inverseFFisherSnedecor(double cumulativeProbability, double d1, double d2);
	static double inverseNormal(double cumulativeProbability, double mean, double stddev);
	static double inverseTStudent(double cumulativeProbability, double mean, double stddev, double degreeFreedom);
};
#endif /* PROBABILITYDISTRIBUTION_H */
