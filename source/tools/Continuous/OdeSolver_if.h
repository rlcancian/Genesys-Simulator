#ifndef ODESOLVER_IF_H
#define ODESOLVER_IF_H

#include "OdeSystem_if.h"

/**
 * @brief ODE solver abstraction.
 *
 * Architectural role:
 * - Planned evolution of the "derivate" responsibility currently exposed by
 *   legacy Solver_if.
 */
class OdeSolver_if {
public:
	virtual ~OdeSolver_if() = default;
	virtual bool advance(const OdeSystem_if& system, double t0, double dt, const double* y0, double* y1) const = 0;
};

#endif /* ODESOLVER_IF_H */
