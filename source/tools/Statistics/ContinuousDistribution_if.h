#ifndef CONTINUOUSDISTRIBUTION_IF_H
#define CONTINUOUSDISTRIBUTION_IF_H

#include "Distribution_if.h"

/**
 * @brief Interface for continuous distributions.
 *
 * Mathematical meaning:
 * - pdf(x): probability density function value at x.
 * - cdf(x): cumulative probability P(X <= x).
 * - quantile(p): inverse CDF for p in [0, 1].
 */
class ContinuousDistribution_if : public Distribution_if {
public:
	virtual ~ContinuousDistribution_if() = default;
	virtual double pdf(double x) const = 0;
	virtual double cdf(double x) const = 0;
	virtual double quantile(double probability) const = 0;
};

#endif /* CONTINUOUSDISTRIBUTION_IF_H */
