#ifndef DISCRETEDISTRIBUTION_IF_H
#define DISCRETEDISTRIBUTION_IF_H

#include "Distribution_if.h"

/**
 * @brief Interface for discrete distributions.
 *
 * Mathematical meaning:
 * - pmf(k): probability mass function value at k.
 * - cdf(k): cumulative probability P(X <= k).
 * - quantile(p): smallest k such that CDF(k) >= p.
 */
class DiscreteDistribution_if : public Distribution_if {
public:
	virtual ~DiscreteDistribution_if() = default;
	virtual double pmf(unsigned int k) const = 0;
	virtual double cdf(unsigned int k) const = 0;
	virtual unsigned int quantile(double probability) const = 0;
};

#endif /* DISCRETEDISTRIBUTION_IF_H */
