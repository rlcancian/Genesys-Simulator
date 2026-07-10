#ifndef QUADRATURE_IF_H
#define QUADRATURE_IF_H

/**
 * @brief One-dimensional numerical quadrature abstraction.
 *
 * Architectural role:
 * - Planned extraction of integration responsibility from Solver_if.
 */
class Quadrature_if {
public:
	virtual ~Quadrature_if() = default;
	virtual double integrate(double min, double max, double (*f)(double), double precision) const = 0;
};

#endif /* QUADRATURE_IF_H */
