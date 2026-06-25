#ifndef PROBABILITYDISTRIBUTION_H
#define PROBABILITYDISTRIBUTION_H

#include "ProbabilityDistributionBase.h"

/**
 * @brief Static facade for inverse/CDF-related distribution utilities.
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
	/** @brief Returns the chi-square quantile for a cumulative probability. */
	static double inverseChi2(double cumulativeProbability, double degreeFreedom);
	/** @brief Returns the Fisher-Snedecor F quantile for a cumulative probability. */
	static double inverseFFisherSnedecor(double cumulativeProbability, double d1, double d2);
	/** @brief Returns the normal quantile for a cumulative probability. */
	static double inverseNormal(double cumulativeProbability, double mean, double stddev);
	/** @brief Returns the Student-t quantile for a cumulative probability. */
	static double inverseTStudent(double cumulativeProbability, double mean, double stddev, double degreeFreedom);
};
#endif /* PROBABILITYDISTRIBUTION_H */
