#ifndef SOLVERDEFAULTIMPL_H
#define SOLVERDEFAULTIMPL_H

#include "Solver_if.h"

/**
 * @brief Default numerical solver for the analysis package.
 *
 * Implementation status:
 * - Quadrature uses the composite Simpson 1/3 rule.
 * - Derivation/advancement overloads are retained only for legacy callers and
 *   are not used by the data-analysis algorithms.
 */
class SolverDefaultImpl : public Solver_if {
public:
	/** @brief Creates a Simpson quadrature solver with precision and step limit. */
	SolverDefaultImpl(double precision = 1e-6, unsigned int steps = 1e3);
	/** @brief Destroys the solver. */
	virtual ~SolverDefaultImpl() = default;
public:
	/** @brief Updates the quadrature precision. */
	virtual void setPrecision(double e) override;
	/** @brief Returns the configured quadrature precision. */
	virtual double getPrecision() override;
	/** @brief Updates the quadrature step limit. */
	virtual void setMaxSteps(double steps) override;
	/** @brief Returns the configured quadrature step limit. */
	virtual double getMaxSteps() override;
	/** @brief Integrates a one-parameter function with composite Simpson 1/3. */
	virtual double integrate(double min, double max, double (*f)(double, double), double p2) override;
	/** @brief Integrates a two-parameter function with composite Simpson 1/3. */
	virtual double integrate(double min, double max, double (*f)(double, double, double), double p2, double p3) override;
	/** @brief Integrates a three-parameter function with composite Simpson 1/3. */
	virtual double integrate(double min, double max, double (*f)(double, double, double, double), double p2, double p3, double p4) override;
	/** @brief Integrates a four-parameter function with composite Simpson 1/3. */
	virtual double integrate(double min, double max, double (*f)(double, double, double, double, double), double p2, double p3, double p4, double p5) override;
	/** @brief Advances a one-parameter legacy differential equation. */
	virtual double derivate(double initPoint, double initValue, double (*f)(double, double), double p2) override;
	/** @brief Preserves the unimplemented two-parameter legacy derivative API. */
	virtual double derivate(double initPoint, double initValue, double (*f)(double, double, double), double p2, double p3) override;
	/** @brief Preserves the unimplemented three-parameter legacy derivative API. */
	virtual double derivate(double initPoint, double initValue, double (*f)(double, double, double, double), double p2, double p3, double p4) override;
	/** @brief Preserves the unimplemented four-parameter legacy derivative API. */
	virtual double derivate(double initPoint, double initValue, double (*f)(double, double, double, double, double), double p2, double p3, double p4, double p5) override;
private:
	double _precision;
	unsigned int _numSteps;
	double _stepSize;
};

#endif /* SOLVERDEFAULTIMPL_H */
