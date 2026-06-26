#ifndef DORMANDPRINCE54ODESOLVER_H
#define DORMANDPRINCE54ODESOLVER_H

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

#include "OdeSolver_if.h"

// Dormand-Prince 5(4): an adaptive Runge-Kutta method for systems of ODEs.
//
// It's a 7-stage method (Dormand & Prince, 1980) that, from the same slopes
// k1..k7, builds two estimates of the next state: one of order 5 (the answer we
// keep) and one of order 4. The gap between them estimates the error of the
// step, and we use that to grow or shrink the step on the fly - big steps where
// the solution is smooth, small steps where it changes fast. There's also a nice
// trick called FSAL: the last slope of an accepted step is also the first slope
// of the next one, so after the first step each step costs only 6 new slope
// evaluations instead of 7.
//
// The OdeSolver_if interface only gives us one fixed step, advance(t0 -> t0+dt).
// To still be adaptive, we take as many small sub-steps as we need inside that
// interval and land exactly on t0+dt. So BioNetwork (or any caller) keeps the
// same time grid it used with RK4, but gets better accuracy without changing
// anything on its side.
//
// advance() is const and keeps all its work arrays local, so the same instance
// can be reused safely. If the numbers blow up (NaN/inf) or the step has to
// shrink below what a double can represent, it gives up and returns false, just
// like the RK4 solver does on bad input.
class DormandPrince54OdeSolver : public OdeSolver_if {
public:
	// The defaults are fine for normal use. rtol/atol say how accurate each step
	// must be; safety keeps the step a little below the "ideal" size; minScale and
	// maxScale limit how fast the step can change; maxSubsteps is just a safety
	// net so a bad input can't make us loop forever.
	explicit DormandPrince54OdeSolver(double relativeTolerance = 1e-6,
	                                  double absoluteTolerance = 1e-9,
	                                  double safetyFactor = 0.9,
	                                  double minScale = 0.2,
	                                  double maxScale = 10.0,
	                                  unsigned int maxSubsteps = 100000u)
		: _rtol(relativeTolerance),
		  _atol(absoluteTolerance),
		  _safety(safetyFactor),
		  _minScale(minScale),
		  _maxScale(maxScale),
		  _maxSubsteps(maxSubsteps) {
	}

	double getRelativeTolerance() const { return _rtol; }
	double getAbsoluteTolerance() const { return _atol; }

	// Advances from t0 to t0+dt. Returns false on bad input, or if it can't reach
	// the requested tolerance.
	bool advance(const OdeSystem_if& system, double t0, double dt,
	             const double* y0, double* y1) const override {
		const unsigned int n = system.dimension();
		if (n == 0 || y0 == nullptr || y1 == nullptr || dt < 0.0) {
			return false;
		}

		// dt == 0: nothing to integrate, so just copy the state across.
		for (unsigned int i = 0; i < n; ++i) {
			y1[i] = y0[i];
		}
		if (dt == 0.0) {
			return true;
		}

		std::vector<double> y(y0, y0 + n);   // current state
		std::vector<double> yNext(n, 0.0);    // order-5 candidate for the next state
		std::vector<double> yErr(n, 0.0);     // estimated error of this step
		std::vector<double> k1(n, 0.0), k2(n, 0.0), k3(n, 0.0), k4(n, 0.0);
		std::vector<double> k5(n, 0.0), k6(n, 0.0), k7(n, 0.0);
		std::vector<double> tmp(n, 0.0);

		double t = t0;
		const double tEnd = t0 + dt;

		// First guess for the sub-step: the whole interval. If it's too big it
		// just gets rejected below and shrunk.
		double h = dt;

		// k1 = f(t0, y0). With FSAL we can reuse it between steps, so we only
		// compute it once here instead of every loop.
		system.evaluate(t, y.data(), k1.data());
		bool firstSameAsLast = false;

		unsigned int steps = 0;
		while (t < tEnd) {
			if (++steps > _maxSubsteps) {
				return false; // safety net so we don't loop forever
			}

			// If we're basically at the end already, stop - an extra tiny step
			// wouldn't change anything and could underflow.
			const double remaining = tEnd - t;
			if (remaining <= stepFloor(t)) {
				break;
			}
			// Don't step past the end of the interval.
			if (h > remaining) {
				h = remaining;
			}

			if (firstSameAsLast) {
				k1 = k7; // FSAL: reuse the last slope of the accepted step
			}

			// the 7 stages (coefficients in the table at the bottom of the class)
			for (unsigned int i = 0; i < n; ++i)
				tmp[i] = y[i] + h * (A21 * k1[i]);
			system.evaluate(t + C2 * h, tmp.data(), k2.data());

			for (unsigned int i = 0; i < n; ++i)
				tmp[i] = y[i] + h * (A31 * k1[i] + A32 * k2[i]);
			system.evaluate(t + C3 * h, tmp.data(), k3.data());

			for (unsigned int i = 0; i < n; ++i)
				tmp[i] = y[i] + h * (A41 * k1[i] + A42 * k2[i] + A43 * k3[i]);
			system.evaluate(t + C4 * h, tmp.data(), k4.data());

			for (unsigned int i = 0; i < n; ++i)
				tmp[i] = y[i] + h * (A51 * k1[i] + A52 * k2[i] + A53 * k3[i] + A54 * k4[i]);
			system.evaluate(t + C5 * h, tmp.data(), k5.data());

			for (unsigned int i = 0; i < n; ++i)
				tmp[i] = y[i] + h * (A61 * k1[i] + A62 * k2[i] + A63 * k3[i] + A64 * k4[i] + A65 * k5[i]);
			system.evaluate(t + C6 * h, tmp.data(), k6.data());

			// The 7th stage uses the order-5 weights - that's what makes FSAL work.
			for (unsigned int i = 0; i < n; ++i)
				yNext[i] = y[i] + h * (B5_1 * k1[i] + B5_3 * k3[i] + B5_4 * k4[i] + B5_5 * k5[i] + B5_6 * k6[i]);
			system.evaluate(t + h, yNext.data(), k7.data());

			// Error of this step = order-5 answer minus order-4 answer.
			for (unsigned int i = 0; i < n; ++i) {
				yErr[i] = h * (E1 * k1[i] + E3 * k3[i] + E4 * k4[i]
				             + E5 * k5[i] + E6 * k6[i] + E7 * k7[i]);
			}

			// Weighted RMS size of the error, scaled by the tolerances.
			double errNorm = 0.0;
			bool finite = true;
			for (unsigned int i = 0; i < n; ++i) {
				if (!std::isfinite(yNext[i]) || !std::isfinite(yErr[i])) {
					finite = false;
					break;
				}
				const double sc = _atol + _rtol * std::max(std::fabs(y[i]), std::fabs(yNext[i]));
				const double ratio = (sc > 0.0) ? (yErr[i] / sc) : 0.0;
				errNorm += ratio * ratio;
			}
			if (!finite) {
				return false;
			}
			errNorm = std::sqrt(errNorm / static_cast<double>(n));

			if (errNorm <= 1.0) {
				// error is within tolerance: accept the step
				t += h;
				y = yNext;
				firstSameAsLast = true; // k7 becomes the next k1 (FSAL)
				h *= scaleFactor(errNorm); // size of the next step
			} else {
				// too much error: throw the step away and try a smaller one
				h *= scaleFactor(errNorm);
				firstSameAsLast = false; // state didn't move, so recompute k1
				if (h < stepFloor(t)) {
					return false; // step got too small to make progress
				}
			}
		}

		for (unsigned int i = 0; i < n; ++i) {
			y1[i] = y[i];
		}
		return true;
	}

private:
	// How much to scale the step for next time, from the error size. Small error
	// -> bigger step, large error -> smaller step, with limits both ways.
	double scaleFactor(double errNorm) const {
		if (errNorm == 0.0) {
			return _maxScale; // no error at all: grow as much as we allow
		}
		// exponent is 1/(order+1) = 1/5 (the lower order of the pair is 4)
		const double factor = _safety * std::pow(errNorm, -0.2);
		return std::min(_maxScale, std::max(_minScale, factor));
	}

	// Smallest step that still moves t by a meaningful amount.
	static double stepFloor(double t) {
		return 16.0 * std::numeric_limits<double>::epsilon() * (std::fabs(t) + 1.0);
	}

private:
	double _rtol;
	double _atol;
	double _safety;
	double _minScale;
	double _maxScale;
	unsigned int _maxSubsteps;

	// Dormand-Prince 5(4) coefficients (Dormand & Prince, 1980).
	// Nodes c_i
	static constexpr double C2 = 1.0 / 5.0;
	static constexpr double C3 = 3.0 / 10.0;
	static constexpr double C4 = 4.0 / 5.0;
	static constexpr double C5 = 8.0 / 9.0;
	static constexpr double C6 = 1.0;
	// Stage coefficients a_ij
	static constexpr double A21 = 1.0 / 5.0;
	static constexpr double A31 = 3.0 / 40.0;
	static constexpr double A32 = 9.0 / 40.0;
	static constexpr double A41 = 44.0 / 45.0;
	static constexpr double A42 = -56.0 / 15.0;
	static constexpr double A43 = 32.0 / 9.0;
	static constexpr double A51 = 19372.0 / 6561.0;
	static constexpr double A52 = -25360.0 / 2187.0;
	static constexpr double A53 = 64448.0 / 6561.0;
	static constexpr double A54 = -212.0 / 729.0;
	static constexpr double A61 = 9017.0 / 3168.0;
	static constexpr double A62 = -355.0 / 33.0;
	static constexpr double A63 = 46732.0 / 5247.0;
	static constexpr double A64 = 49.0 / 176.0;
	static constexpr double A65 = -5103.0 / 18656.0;
	// 5th-order weights b_i (also row 7 of a_ij => FSAL; b2 = b7 = 0)
	static constexpr double B5_1 = 35.0 / 384.0;
	static constexpr double B5_3 = 500.0 / 1113.0;
	static constexpr double B5_4 = 125.0 / 192.0;
	static constexpr double B5_5 = -2187.0 / 6784.0;
	static constexpr double B5_6 = 11.0 / 84.0;
	// Error weights e_i = b_i(5th) - b_i*(4th)
	static constexpr double E1 = 35.0 / 384.0 - 5179.0 / 57600.0;
	static constexpr double E3 = 500.0 / 1113.0 - 7571.0 / 16695.0;
	static constexpr double E4 = 125.0 / 192.0 - 393.0 / 640.0;
	static constexpr double E5 = -2187.0 / 6784.0 - (-92097.0 / 339200.0);
	static constexpr double E6 = 11.0 / 84.0 - 187.0 / 2100.0;
	static constexpr double E7 = 0.0 - 1.0 / 40.0;
};

#endif /* DORMANDPRINCE54ODESOLVER_H */
