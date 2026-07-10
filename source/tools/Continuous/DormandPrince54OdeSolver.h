#ifndef DORMANDPRINCE54ODESOLVER_H
#define DORMANDPRINCE54ODESOLVER_H

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

#include "OdeSolver_if.h"

class DormandPrince54OdeSolver : public OdeSolver_if {
public:
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
		  _maxSubsteps(maxSubsteps) {}

	double getRelativeTolerance() const { return _rtol; }
	double getAbsoluteTolerance() const { return _atol; }

	bool advance(const OdeSystem_if& system, double t0, double dt,
	             const double* y0, double* y1) const override {
		const unsigned int n = system.dimension();
		if (n == 0 || y0 == nullptr || y1 == nullptr || dt < 0.0) {
			return false;
		}
		for (unsigned int i = 0; i < n; ++i) {
			y1[i] = y0[i];
		}
		if (dt == 0.0) {
			return true;
		}

		std::vector<double> y(y0, y0 + n), yNext(n, 0.0), yErr(n, 0.0), tmp(n, 0.0);
		std::vector<double> k1(n, 0.0), k2(n, 0.0), k3(n, 0.0), k4(n, 0.0);
		std::vector<double> k5(n, 0.0), k6(n, 0.0), k7(n, 0.0);
		double t = t0;
		const double tEnd = t0 + dt;
		double h = dt;
		system.evaluate(t, y.data(), k1.data());
		bool firstSameAsLast = false;

		unsigned int steps = 0;
		while (t < tEnd) {
			if (++steps > _maxSubsteps) {
				return false;
			}
			const double remaining = tEnd - t;
			if (remaining <= stepFloor(t)) {
				break;
			}
			if (h > remaining) {
				h = remaining;
			}
			if (firstSameAsLast) {
				k1 = k7;
			}

			for (unsigned int i = 0; i < n; ++i) tmp[i] = y[i] + h * A21 * k1[i];
			system.evaluate(t + C2 * h, tmp.data(), k2.data());
			for (unsigned int i = 0; i < n; ++i) tmp[i] = y[i] + h * (A31 * k1[i] + A32 * k2[i]);
			system.evaluate(t + C3 * h, tmp.data(), k3.data());
			for (unsigned int i = 0; i < n; ++i) tmp[i] = y[i] + h * (A41 * k1[i] + A42 * k2[i] + A43 * k3[i]);
			system.evaluate(t + C4 * h, tmp.data(), k4.data());
			for (unsigned int i = 0; i < n; ++i) tmp[i] = y[i] + h * (A51 * k1[i] + A52 * k2[i] + A53 * k3[i] + A54 * k4[i]);
			system.evaluate(t + C5 * h, tmp.data(), k5.data());
			for (unsigned int i = 0; i < n; ++i) tmp[i] = y[i] + h * (A61 * k1[i] + A62 * k2[i] + A63 * k3[i] + A64 * k4[i] + A65 * k5[i]);
			system.evaluate(t + C6 * h, tmp.data(), k6.data());
			for (unsigned int i = 0; i < n; ++i) yNext[i] = y[i] + h * (B5_1 * k1[i] + B5_3 * k3[i] + B5_4 * k4[i] + B5_5 * k5[i] + B5_6 * k6[i]);
			system.evaluate(t + h, yNext.data(), k7.data());

			for (unsigned int i = 0; i < n; ++i) {
				yErr[i] = h * (E1 * k1[i] + E3 * k3[i] + E4 * k4[i] + E5 * k5[i] + E6 * k6[i] + E7 * k7[i]);
			}

			double errNorm = 0.0;
			for (unsigned int i = 0; i < n; ++i) {
				if (!std::isfinite(yNext[i]) || !std::isfinite(yErr[i])) {
					return false;
				}
				const double sc = _atol + _rtol * std::max(std::fabs(y[i]), std::fabs(yNext[i]));
				const double ratio = (sc > 0.0) ? (yErr[i] / sc) : 0.0;
				errNorm += ratio * ratio;
			}
			errNorm = std::sqrt(errNorm / static_cast<double>(n));

			if (errNorm <= 1.0) {
				t += h;
				y = yNext;
				firstSameAsLast = true;
				h *= scaleFactor(errNorm);
			} else {
				h *= scaleFactor(errNorm);
				firstSameAsLast = false;
				if (h < stepFloor(t)) {
					return false;
				}
			}
		}

		for (unsigned int i = 0; i < n; ++i) {
			y1[i] = y[i];
		}
		return true;
	}

private:
	double scaleFactor(double errNorm) const {
		if (errNorm == 0.0) {
			return _maxScale;
		}
		return std::min(_maxScale, std::max(_minScale, _safety * std::pow(errNorm, -0.2)));
	}

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

	static constexpr double C2 = 1.0 / 5.0;
	static constexpr double C3 = 3.0 / 10.0;
	static constexpr double C4 = 4.0 / 5.0;
	static constexpr double C5 = 8.0 / 9.0;
	static constexpr double C6 = 1.0;
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
	static constexpr double B5_1 = 35.0 / 384.0;
	static constexpr double B5_3 = 500.0 / 1113.0;
	static constexpr double B5_4 = 125.0 / 192.0;
	static constexpr double B5_5 = -2187.0 / 6784.0;
	static constexpr double B5_6 = 11.0 / 84.0;
	static constexpr double E1 = 35.0 / 384.0 - 5179.0 / 57600.0;
	static constexpr double E3 = 500.0 / 1113.0 - 7571.0 / 16695.0;
	static constexpr double E4 = 125.0 / 192.0 - 393.0 / 640.0;
	static constexpr double E5 = -2187.0 / 6784.0 - (-92097.0 / 339200.0);
	static constexpr double E6 = 11.0 / 84.0 - 187.0 / 2100.0;
	static constexpr double E7 = -1.0 / 40.0;
};

#endif /* DORMANDPRINCE54ODESOLVER_H */
