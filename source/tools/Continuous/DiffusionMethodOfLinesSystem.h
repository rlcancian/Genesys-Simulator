#ifndef DIFFUSIONMETHODOFLINESSYSTEM_H
#define DIFFUSIONMETHODOFLINESSYSTEM_H

#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

#include "OdeSystem_if.h"

class DiffusionMethodOfLinesSystem : public OdeSystem_if {
public:
	static constexpr double kPi = 3.14159265358979323846;

	enum class Boundary {
		Dirichlet,
		Neumann
	};

	DiffusionMethodOfLinesSystem() = default;

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

	unsigned int dimension() const override { return _totalNodes; }

	void evaluate(double, const double* y, double* dydt) const override {
		if (!_valid || y == nullptr || dydt == nullptr) {
			return;
		}
		const unsigned int N = static_cast<unsigned int>(_shape.size());
		for (unsigned int node = 0; node < _totalNodes; ++node) {
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
					secondDifference = 2.0 * (y[node + s] - y[node]);
				} else if (m == last) {
					secondDifference = 2.0 * (y[node - s] - y[node]);
				} else {
					secondDifference = y[node + s] - 2.0 * y[node] + y[node - s];
				}
				laplacian += secondDifference * _invH2[d];
			}
			dydt[node] = _D * laplacian;
		}
	}

	bool isValid() const { return _valid; }
	unsigned int totalNodes() const { return _totalNodes; }
	const std::vector<unsigned int>& shape() const { return _shape; }
	const std::vector<double>& spacing() const { return _spacing; }
	double diffusionCoefficient() const { return _D; }
	Boundary boundary() const { return _boundary; }

	unsigned int coordinate(unsigned int node, unsigned int d) const {
		return static_cast<unsigned int>((node / _stride[d]) % _shape[d]);
	}

	unsigned int nodeIndex(const std::vector<unsigned int>& multiIndex) const {
		std::size_t idx = 0;
		for (std::size_t d = 0; d < _shape.size(); ++d) {
			idx += static_cast<std::size_t>(multiIndex[d]) * _stride[d];
		}
		return static_cast<unsigned int>(idx);
	}

	bool isBoundaryNode(unsigned int node) const {
		for (unsigned int d = 0; d < _shape.size(); ++d) {
			const unsigned int m = coordinate(node, d);
			if (m == 0u || m == _shape[d] - 1u) {
				return true;
			}
		}
		return false;
	}

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

	double sineModeDecayRate(const std::vector<unsigned int>& modes) const {
		double lambda = 0.0;
		for (std::size_t d = 0; d < _shape.size(); ++d) {
			const double s = std::sin(modes[d] * kPi / (2.0 * (_shape[d] - 1u)));
			lambda += (4.0 / (_spacing[d] * _spacing[d])) * s * s;
		}
		return _D * lambda;
	}

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

	double l2Norm(const double* y) const {
		double sum = 0.0;
		for (unsigned int i = 0; i < _totalNodes; ++i) {
			sum += y[i] * y[i];
		}
		return std::sqrt(sum);
	}

	double maxValue(const double* y) const {
		double mx = -std::numeric_limits<double>::infinity();
		for (unsigned int i = 0; i < _totalNodes; ++i) {
			if (y[i] > mx) {
				mx = y[i];
			}
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
				return;
			}
		}

		_stride.assign(N, 1);
		for (std::size_t d = N - 1; d-- > 0;) {
			_stride[d] = _stride[d + 1] * _shape[d + 1];
		}
		std::size_t total = 1;
		for (unsigned int n : _shape) {
			if (total > std::numeric_limits<std::size_t>::max() / n) {
				return;
			}
			total *= n;
		}
		if (total > std::numeric_limits<unsigned int>::max()) {
			return;
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
