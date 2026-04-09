#ifndef ROOTFINDER_IF_H
#define ROOTFINDER_IF_H

/**
 * @brief Root-finding abstraction for scalar equations.
 */
class RootFinder_if {
public:
	virtual ~RootFinder_if() = default;
	virtual double findRoot(double min, double max, double (*f)(double), double tolerance) const = 0;
};

#endif /* ROOTFINDER_IF_H */
