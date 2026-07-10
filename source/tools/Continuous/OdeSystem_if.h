#ifndef ODESYSTEM_IF_H
#define ODESYSTEM_IF_H

/**
 * @brief Interface for first-order ODE systems dy/dt = f(t, y).
 */
class OdeSystem_if {
public:
	virtual ~OdeSystem_if() = default;
	virtual unsigned int dimension() const = 0;
	virtual void evaluate(double t, const double* y, double* dydt) const = 0;
};

#endif /* ODESYSTEM_IF_H */
