#ifndef SOLVER_IF_H
#define SOLVER_IF_H

/**
 * @brief Numerical solver contract used by tools/analysis.
 *
 * Architectural role:
 * - Provides quadrature for probability CDFs and legacy derivation overloads.
 *
 * Mathematical meaning:
 * - Function signatures use the first functional parameter as time @c t.
 * - Parameters p2..p5 represent model parameters passed through old-style
 *   function pointers.
 *
 * Limitations:
 * - The p2/p3/p4/p5 model reflects a historical constraint and is not the
 *   preferred long-term abstraction.
 *
 * The analysis package uses only integrate(). The derivate() overloads remain
 * available for compatibility with older external consumers.
 */
class Solver_if {
public:
	/** @brief Destroys the numerical solver interface. */
	virtual ~Solver_if() = default;
	/** @brief Sets the requested quadrature precision. */
	virtual void setPrecision(double e) = 0;
	/** @brief Returns the requested quadrature precision. */
	virtual double getPrecision() = 0;
	/** @brief Sets the maximum number of quadrature steps. */
	virtual void setMaxSteps(double steps) = 0;
	/** @brief Returns the configured maximum number of quadrature steps. */
	virtual double getMaxSteps() = 0;
	/** @brief Integrates a function with one supplied model parameter. */
	virtual double integrate(double min, double max, double (*f)(double, double), double p2) = 0;
	/** @brief Integrates a function with two supplied model parameters. */
	virtual double integrate(double min, double max, double (*f)(double, double, double), double p2, double p3) = 0;
	/** @brief Integrates a function with three supplied model parameters. */
	virtual double integrate(double min, double max, double (*f)(double, double, double, double), double p2, double p3, double p4) = 0;
	/** @brief Integrates a function with four supplied model parameters. */
	virtual double integrate(double min, double max, double (*f)(double, double, double, double, double), double p2, double p3, double p4, double p5) = 0;
	/** @brief Advances a one-parameter legacy differential equation. */
	virtual double derivate(double initPoint, double initValue, double (*f)(double, double), double p2) = 0;
	/** @brief Advances a two-parameter legacy differential equation. */
	virtual double derivate(double initPoint, double initValue, double (*f)(double, double, double), double p2, double p3) = 0;
	/** @brief Advances a three-parameter legacy differential equation. */
	virtual double derivate(double initPoint, double initValue, double (*f)(double, double, double, double), double p2, double p3, double p4) = 0;
	/** @brief Advances a four-parameter legacy differential equation. */
	virtual double derivate(double initPoint, double initValue, double (*f)(double, double, double, double, double), double p2, double p3, double p4, double p5) = 0;
};

#endif /* SOLVER_IF_H */
