#ifndef DIFFUSIONMETHODOFLINESSYSTEM_H
#define DIFFUSIONMETHODOFLINESSYSTEM_H

#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

#include "OdeSystem_if.h"

// The diffusion (heat) equation du/dt = D * Laplacian(u) in N dimensions, turned
// into a system of ODEs dy/dt = f(t, y) by the Method of Lines.
//
// The idea of the Method of Lines: discretize space but leave time continuous.
// We put a regular grid over space and approximate the Laplacian at each grid
// point with the usual central difference (second derivative is about
// (left - 2*middle + right) / h^2), summed over every dimension. That gives one
// ODE per grid point, so the whole PDE becomes a big system dy/dt = D * L * y,
// where L is the discrete Laplacian. Any OdeSolver_if (RK4 or Dormand-Prince)
// can then integrate it in time. This class is only the right-hand side f(t, y);
// it doesn't do the time stepping itself.
//
// N can be any number of dimensions - it's just the length of the "shape"
// vector. The grid is stored as one flat array (row-major, last index varies
// fastest), and coordinate(node, d) recovers a point's index along axis d, so
// the same code works for 1-D, 2-D, 3-D and beyond.
//
// Boundary conditions:
//  - Dirichlet (fixed value): edge points never change (dy/dt = 0) and keep
//    whatever the initial field put there (use 0 for a zero boundary).
//  - Neumann (no flux through the edge): we mirror the point just outside the
//    grid, which forces the derivative at the edge to be zero. With Neumann the
//    total mass stays constant (see totalMass).
//
// Useful fact for the tests: a product of sines, prod_d sin(k_d*pi*x_d/L_d), is
// a shape that the discrete Laplacian decays cleanly as exp(-lambda*t), where
// lambda is given by sineModeDecayRate(). That lets the tests check the answer
// in any dimension against a known formula.
class DiffusionMethodOfLinesSystem : public OdeSystem_if {
public:
	// Pi (M_PI isn't standard C++, so just define it here).
	static constexpr double kPi = 3.14159265358979323846;

	enum class Boundary {
		Dirichlet, // fixed value on the edges
		Neumann    // no flux through the edges (conserves mass)
	};

	DiffusionMethodOfLinesSystem() = default;

	// shape    = number of grid points per axis; N = shape.size(). Each must be
	//            >= 3 so there's at least one interior point.
	// spacing  = grid spacing h per axis (same length as shape).
	// diffusionCoefficient = D >= 0.
	// boundary = which boundary condition to use on every edge.
	DiffusionMethodOfLinesSystem(std::vector<unsigned int> shape,
	                             std::vector<double> spacing,
	                             double diffusionCoefficient,
	                             Boundary boundary)
		: _shape(std::move(shape)),
		  _spacing(std::move(spacing)),
		  _D(diffusionCoefficient),
		  _boundary(boundary) {
		rebuild();
	}

	// --- OdeSystem_if contract ---------------------------------------------

	unsigned int dimension() const override { return _totalNodes; }

	// dy/dt = D * (discrete Laplacian of y) at every grid point. One pass over the
	// grid; each point looks at its 2 neighbours along each axis. Does nothing if
	// the config is invalid or a pointer is null.
	void evaluate(double, const double* y, double* dydt) const override {
		if (!_valid || y == nullptr || dydt == nullptr) {
			return;
		}
		const unsigned int N = static_cast<unsigned int>(_shape.size());
		for (unsigned int node = 0; node < _totalNodes; ++node) {
			// Dirichlet edges are fixed, so they don't change.
			if (_boundary == Boundary::Dirichlet && isBoundaryNode(node)) {
				dydt[node] = 0.0;
				continue;
			}
			double laplacian = 0.0;
			for (unsigned int d = 0; d < N; ++d) {
				const unsigned int m = coordinate(node, d);
				const unsigned int last = _shape[d] - 1u;
				const std::size_t s = _stride[d];
				double secondDifference;
				if (m == 0u) {
					// Left edge. Only reached for Neumann (Dirichlet is handled
					// above): mirroring the outside point gives 2*(inner - edge).
					secondDifference = 2.0 * (y[node + s] - y[node]);
				} else if (m == last) {
					// Right edge (same Neumann mirroring).
					secondDifference = 2.0 * (y[node - s] - y[node]);
				} else {
					// Inside along this axis: the usual central difference.
					secondDifference = y[node + s] - 2.0 * y[node] + y[node - s];
				}
				laplacian += secondDifference * _invH2[d];
			}
			dydt[node] = _D * laplacian;
		}
	}

	// --- Geometry / configuration accessors --------------------------------

	bool isValid() const { return _valid; }
	unsigned int totalNodes() const { return _totalNodes; }
	const std::vector<unsigned int>& shape() const { return _shape; }
	const std::vector<double>& spacing() const { return _spacing; }
	double diffusionCoefficient() const { return _D; }
	Boundary boundary() const { return _boundary; }

	// Grid index of a node along axis d (undoes the flat row-major layout).
	unsigned int coordinate(unsigned int node, unsigned int d) const {
		return static_cast<unsigned int>((node / _stride[d]) % _shape[d]);
	}

	// Flat array index from a full set of grid coordinates (length must be N).
	unsigned int nodeIndex(const std::vector<unsigned int>& multiIndex) const {
		std::size_t idx = 0;
		for (std::size_t d = 0; d < _shape.size(); ++d) {
			idx += static_cast<std::size_t>(multiIndex[d]) * _stride[d];
		}
		return static_cast<unsigned int>(idx);
	}

	// True if the node sits on any edge of the grid.
	bool isBoundaryNode(unsigned int node) const {
		for (unsigned int d = 0; d < _shape.size(); ++d) {
			const unsigned int m = coordinate(node, d);
			if (m == 0u || m == _shape[d] - 1u) {
				return true;
			}
		}
		return false;
	}

	// --- Initial-field helpers (domain is [0, L] per axis, L = (points-1)*h) ---

	// Fills y with a product of sines, A * prod_d sin(k_d*pi*x_d/L_d). This is the
	// shape that decays at a known rate, so it's what the Dirichlet tests use.
	void fillSineModes(const std::vector<unsigned int>& modes, double amplitude, double* y) const {
		const unsigned int N = static_cast<unsigned int>(_shape.size());
		for (unsigned int node = 0; node < _totalNodes; ++node) {
			double value = amplitude;
			for (unsigned int d = 0; d < N; ++d) {
				const unsigned int m = coordinate(node, d);
				const double L = (_shape[d] - 1u) * _spacing[d];
				const double x = m * _spacing[d];
				value *= std::sin(modes[d] * kPi * x / L);
			}
			y[node] = value;
		}
	}

	// Fills y with a Gaussian bump centred in the grid, A * exp(-r^2/(2*sigma^2)).
	// Good for the Neumann tests (mass conservation and watching it spread out).
	void fillGaussian(double amplitude, double sigma, double* y) const {
		const unsigned int N = static_cast<unsigned int>(_shape.size());
		const double twoSigma2 = 2.0 * sigma * sigma;
		for (unsigned int node = 0; node < _totalNodes; ++node) {
			double r2 = 0.0;
			for (unsigned int d = 0; d < N; ++d) {
				const unsigned int m = coordinate(node, d);
				const double L = (_shape[d] - 1u) * _spacing[d];
				const double x = m * _spacing[d];
				const double dx = x - 0.5 * L;
				r2 += dx * dx;
			}
			y[node] = amplitude * std::exp(-r2 / twoSigma2);
		}
	}

	// Decay rate lambda for a sine field, for our discrete Laplacian (not the
	// continuous one): lambda = D * sum_d (4/h_d^2) * sin^2(k_d*pi/(2*(points-1))).
	// The sine field then follows y(t) = y(0) * exp(-lambda*t) exactly, which is
	// what the tests compare against.
	double sineModeDecayRate(const std::vector<unsigned int>& modes) const {
		double lambda = 0.0;
		for (std::size_t d = 0; d < _shape.size(); ++d) {
			const double s = std::sin(modes[d] * kPi / (2.0 * (_shape[d] - 1u)));
			lambda += (4.0 / (_spacing[d] * _spacing[d])) * s * s;
		}
		return _D * lambda;
	}

	// --- Diagnostics --------------------------------------------------------

	// Total "mass" = integral of the field over the grid, using the trapezoidal
	// rule (edge points count half). Under Neumann this should stay constant
	// (up to round-off), which is a good check that the boundary is right.
	double totalMass(const double* y) const {
		double mass = 0.0;
		double cellVolume = 1.0;
		for (double h : _spacing) {
			cellVolume *= h;
		}
		const unsigned int N = static_cast<unsigned int>(_shape.size());
		for (unsigned int node = 0; node < _totalNodes; ++node) {
			double weight = 1.0;
			for (unsigned int d = 0; d < N; ++d) {
				const unsigned int m = coordinate(node, d);
				if (m == 0u || m == _shape[d] - 1u) {
					weight *= 0.5;
				}
			}
			mass += weight * y[node];
		}
		return mass * cellVolume;
	}

	// L2 norm: sqrt of the sum of squares.
	double l2Norm(const double* y) const {
		double sum = 0.0;
		for (unsigned int i = 0; i < _totalNodes; ++i) {
			sum += y[i] * y[i];
		}
		return std::sqrt(sum);
	}

	/// Maximum field value (peak).
	double maxValue(const double* y) const {
		double mx = -std::numeric_limits<double>::infinity();
		for (unsigned int i = 0; i < _totalNodes; ++i) {
			if (y[i] > mx) mx = y[i];
		}
		return mx;
	}

private:
	void rebuild() {
		_valid = false;
		_totalNodes = 0;
		_stride.clear();
		_invH2.clear();

		const std::size_t N = _shape.size();
		if (N == 0 || _spacing.size() != N || _D < 0.0) {
			return;
		}
		for (std::size_t d = 0; d < N; ++d) {
			if (_shape[d] < 3u || _spacing[d] <= 0.0) {
				return; // need >=1 interior node and positive spacing per dim
			}
		}

		// Row-major strides: last dimension varies fastest.
		_stride.assign(N, 1);
		for (std::size_t d = N - 1; d-- > 0;) {
			_stride[d] = _stride[d + 1] * _shape[d + 1];
		}
		std::size_t total = 1;
		for (unsigned int n : _shape) {
			if (total > std::numeric_limits<std::size_t>::max() / n)
				return;
			total *= n;
		}
		_totalNodes = static_cast<unsigned int>(total);

		_invH2.assign(N, 0.0);
		for (std::size_t d = 0; d < N; ++d) {
			_invH2[d] = 1.0 / (_spacing[d] * _spacing[d]);
		}
		_valid = true;
	}

private:
	std::vector<unsigned int> _shape;
	std::vector<double> _spacing;
	double _D = 0.0;
	Boundary _boundary = Boundary::Dirichlet;

	bool _valid = false;
	unsigned int _totalNodes = 0;
	std::vector<std::size_t> _stride;
	std::vector<double> _invH2;
};

#endif /* DIFFUSIONMETHODOFLINESSYSTEM_H */
