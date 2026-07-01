#ifndef RUNGEKUTTA4ODESOLVER_H
#define RUNGEKUTTA4ODESOLVER_H

#include <vector>

#include "OdeSolver_if.h"

// Classic 4th-order Runge-Kutta with a fixed step. This is the original solver
// and the math inside is unchanged. The only difference now is that code asks
// the OdeSolverFactory for it (by the name "RungeKutta4") instead of building it
// directly, so it can be swapped for another method like Dormand-Prince without
// touching the caller.
//
// 4 stages, error shrinks like O(h^4), and it does exactly 4 slope evaluations
// per step. There's no error estimate and no step adaptation, so the accuracy is
// whatever the chosen step size gives you.
class RungeKutta4OdeSolver : public OdeSolver_if {
public:
	bool advance(const OdeSystem_if& system, double t0, double dt, const double* y0, double* y1) const override {
		const unsigned int n = system.dimension();
		if (n == 0 || y0 == nullptr || y1 == nullptr || dt < 0.0) {
			return false;
		}

		std::vector<double> k1(n, 0.0);
		std::vector<double> k2(n, 0.0);
		std::vector<double> k3(n, 0.0);
		std::vector<double> k4(n, 0.0);
		std::vector<double> temp(n, 0.0);

		system.evaluate(t0, y0, k1.data());
		for (unsigned int i = 0; i < n; ++i) {
			temp[i] = y0[i] + 0.5 * dt * k1[i];
		}

		system.evaluate(t0 + 0.5 * dt, temp.data(), k2.data());
		for (unsigned int i = 0; i < n; ++i) {
			temp[i] = y0[i] + 0.5 * dt * k2[i];
		}

		system.evaluate(t0 + 0.5 * dt, temp.data(), k3.data());
		for (unsigned int i = 0; i < n; ++i) {
			temp[i] = y0[i] + dt * k3[i];
		}

		system.evaluate(t0 + dt, temp.data(), k4.data());
		for (unsigned int i = 0; i < n; ++i) {
			y1[i] = y0[i] + (dt / 6.0) * (k1[i] + 2.0 * k2[i] + 2.0 * k3[i] + k4[i]);
		}
		return true;
	}
};

#endif /* RUNGEKUTTA4ODESOLVER_H */
