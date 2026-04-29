#ifndef METABOLICFLUXBALANCESOLVER_H
#define METABOLICFLUXBALANCESOLVER_H

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
#include <vector>

/**
 * Small LP solver for flux-balance problems with equality constraints and
 * variable bounds.
 *
 * It solves:
 *   maximize/minimize c^T v
 *   subject to A v = 0
 *              lb <= v <= ub
 *
 * using basis enumeration over bounded vertices. This is practical for the
 * small metabolic models currently exercised by the runtime tests and provides
 * a concrete solver-backed replacement for the previous stub objective logic.
 */
class MetabolicFluxBalanceSolver {
public:
	struct Problem {
		std::vector<std::vector<double>> stoichiometry;
		std::vector<double> lowerBounds;
		std::vector<double> upperBounds;
		std::vector<double> objective;
		bool maximize = true;
	};

	struct Solution {
		bool feasible = false;
		double objectiveValue = 0.0;
		std::vector<double> fluxes;
		std::string errorMessage;
	};

public:
	static Solution solve(const Problem& problem) {
		Solution solution;
		const std::size_t variableCount = problem.objective.size();
		if (variableCount == 0u) {
			solution.errorMessage = "Empty objective vector.";
			return solution;
		}
		if (problem.lowerBounds.size() != variableCount || problem.upperBounds.size() != variableCount) {
			solution.errorMessage = "Invalid flux bound vector dimensions.";
			return solution;
		}
		for (const std::vector<double>& row : problem.stoichiometry) {
			if (row.size() != variableCount) {
				solution.errorMessage = "Invalid stoichiometry matrix dimensions.";
				return solution;
			}
		}
		for (std::size_t i = 0; i < variableCount; ++i) {
			if (problem.upperBounds[i] < problem.lowerBounds[i]) {
				solution.errorMessage = "Upper bound smaller than lower bound.";
				return solution;
			}
		}

		std::vector<std::vector<double>> constraints = problem.stoichiometry;
		_removeRedundantRows(&constraints);
		const std::size_t rank = constraints.size();
		if (rank > variableCount) {
			solution.errorMessage = "Stoichiometry rank exceeds variable count.";
			return solution;
		}

		std::vector<std::size_t> basis;
		for (std::size_t i = 0; i < rank; ++i) {
			basis.push_back(i);
		}

		bool found = false;
		double bestObjective = problem.maximize ? -std::numeric_limits<double>::infinity()
		                                       : std::numeric_limits<double>::infinity();
		std::vector<double> bestFluxes(variableCount, 0.0);

		do {
			if (!_isIndependentBasis(constraints, basis)) {
				continue;
			}

			std::vector<std::size_t> nonBasis;
			for (std::size_t i = 0; i < variableCount; ++i) {
				if (std::find(basis.begin(), basis.end(), i) == basis.end()) {
					nonBasis.push_back(i);
				}
			}

			const std::size_t combinations = std::size_t{1} << nonBasis.size();
			for (std::size_t mask = 0; mask < combinations; ++mask) {
				std::vector<double> fluxes(variableCount, 0.0);
				for (std::size_t k = 0; k < nonBasis.size(); ++k) {
					const std::size_t variableIndex = nonBasis[k];
					const bool chooseUpper = ((mask >> k) & 1u) != 0u;
					fluxes[variableIndex] = chooseUpper ? problem.upperBounds[variableIndex] : problem.lowerBounds[variableIndex];
				}

				std::vector<std::vector<double>> basisMatrix(rank, std::vector<double>(rank, 0.0));
				std::vector<double> rhs(rank, 0.0);
				for (std::size_t row = 0; row < rank; ++row) {
					for (std::size_t col = 0; col < rank; ++col) {
						basisMatrix[row][col] = constraints[row][basis[col]];
					}
					for (std::size_t variableIndex : nonBasis) {
						rhs[row] -= constraints[row][variableIndex] * fluxes[variableIndex];
					}
				}

				std::vector<double> basisValues;
				if (!_solveLinearSystem(basisMatrix, rhs, &basisValues)) {
					continue;
				}
				for (std::size_t col = 0; col < rank; ++col) {
					fluxes[basis[col]] = basisValues[col];
				}
				if (!_withinBounds(fluxes, problem.lowerBounds, problem.upperBounds)) {
					continue;
				}
				if (!_satisfiesConstraints(constraints, fluxes)) {
					continue;
				}

				const double objectiveValue = _dot(problem.objective, fluxes);
				if (!found ||
				    (problem.maximize && objectiveValue > bestObjective + 1e-9) ||
				    (!problem.maximize && objectiveValue < bestObjective - 1e-9)) {
					found = true;
					bestObjective = objectiveValue;
					bestFluxes = fluxes;
				}
			}
		} while (_nextBasis(variableCount, rank, &basis));

		if (!found) {
			solution.errorMessage = "No feasible bounded flux solution found.";
			return solution;
		}

		solution.feasible = true;
		solution.objectiveValue = bestObjective;
		solution.fluxes = std::move(bestFluxes);
		return solution;
	}

private:
	static void _removeRedundantRows(std::vector<std::vector<double>>* matrix) {
		if (matrix == nullptr) {
			return;
		}
		std::vector<std::vector<double>> filtered;
		for (const std::vector<double>& row : *matrix) {
			bool allZero = true;
			for (double value : row) {
				if (std::fabs(value) > 1e-9) {
					allZero = false;
					break;
				}
			}
			if (!allZero) {
				filtered.push_back(row);
			}
		}

		std::vector<std::vector<double>> independent;
		for (const std::vector<double>& row : filtered) {
			std::vector<std::vector<double>> candidate = independent;
			candidate.push_back(row);
			if (_matrixRank(candidate) > independent.size()) {
				independent.push_back(row);
			}
		}
		*matrix = independent;
	}

	static bool _isIndependentBasis(const std::vector<std::vector<double>>& constraints,
	                                const std::vector<std::size_t>& basis) {
		const std::size_t rank = constraints.size();
		if (basis.size() != rank) {
			return false;
		}
		if (rank == 0u) {
			return true;
		}
		std::vector<std::vector<double>> basisMatrix(rank, std::vector<double>(rank, 0.0));
		for (std::size_t row = 0; row < rank; ++row) {
			for (std::size_t col = 0; col < rank; ++col) {
				basisMatrix[row][col] = constraints[row][basis[col]];
			}
		}
		return _matrixRank(basisMatrix) == rank;
	}

	static bool _solveLinearSystem(std::vector<std::vector<double>> matrix,
	                               std::vector<double> rhs,
	                               std::vector<double>* solution) {
		if (solution == nullptr) {
			return false;
		}
		const std::size_t n = matrix.size();
		if (rhs.size() != n) {
			return false;
		}
		for (const std::vector<double>& row : matrix) {
			if (row.size() != n) {
				return false;
			}
		}

		for (std::size_t pivot = 0; pivot < n; ++pivot) {
			std::size_t pivotRow = pivot;
			double bestAbs = std::fabs(matrix[pivot][pivot]);
			for (std::size_t row = pivot + 1; row < n; ++row) {
				const double candidate = std::fabs(matrix[row][pivot]);
				if (candidate > bestAbs) {
					bestAbs = candidate;
					pivotRow = row;
				}
			}
			if (bestAbs <= 1e-9) {
				return false;
			}
			if (pivotRow != pivot) {
				std::swap(matrix[pivotRow], matrix[pivot]);
				std::swap(rhs[pivotRow], rhs[pivot]);
			}

			const double pivotValue = matrix[pivot][pivot];
			for (std::size_t col = pivot; col < n; ++col) {
				matrix[pivot][col] /= pivotValue;
			}
			rhs[pivot] /= pivotValue;

			for (std::size_t row = 0; row < n; ++row) {
				if (row == pivot) {
					continue;
				}
				const double factor = matrix[row][pivot];
				if (std::fabs(factor) <= 1e-12) {
					continue;
				}
				for (std::size_t col = pivot; col < n; ++col) {
					matrix[row][col] -= factor * matrix[pivot][col];
				}
				rhs[row] -= factor * rhs[pivot];
			}
		}

		*solution = rhs;
		return true;
	}

	static std::size_t _matrixRank(std::vector<std::vector<double>> matrix) {
		if (matrix.empty()) {
			return 0u;
		}
		const std::size_t rows = matrix.size();
		const std::size_t cols = matrix.front().size();
		std::size_t rank = 0u;
		std::size_t pivotRow = 0u;
		for (std::size_t col = 0; col < cols && pivotRow < rows; ++col) {
			std::size_t bestRow = pivotRow;
			double bestAbs = std::fabs(matrix[pivotRow][col]);
			for (std::size_t row = pivotRow + 1; row < rows; ++row) {
				const double candidate = std::fabs(matrix[row][col]);
				if (candidate > bestAbs) {
					bestAbs = candidate;
					bestRow = row;
				}
			}
			if (bestAbs <= 1e-9) {
				continue;
			}
			if (bestRow != pivotRow) {
				std::swap(matrix[bestRow], matrix[pivotRow]);
			}
			const double pivotValue = matrix[pivotRow][col];
			for (std::size_t activeCol = col; activeCol < cols; ++activeCol) {
				matrix[pivotRow][activeCol] /= pivotValue;
			}
			for (std::size_t row = pivotRow + 1; row < rows; ++row) {
				const double factor = matrix[row][col];
				if (std::fabs(factor) <= 1e-12) {
					continue;
				}
				for (std::size_t activeCol = col; activeCol < cols; ++activeCol) {
					matrix[row][activeCol] -= factor * matrix[pivotRow][activeCol];
				}
			}
			++pivotRow;
			++rank;
		}
		return rank;
	}

	static bool _withinBounds(const std::vector<double>& fluxes,
	                          const std::vector<double>& lowerBounds,
	                          const std::vector<double>& upperBounds) {
		for (std::size_t i = 0; i < fluxes.size(); ++i) {
			if (fluxes[i] < lowerBounds[i] - 1e-7 || fluxes[i] > upperBounds[i] + 1e-7) {
				return false;
			}
		}
		return true;
	}

	static bool _satisfiesConstraints(const std::vector<std::vector<double>>& constraints,
	                                  const std::vector<double>& fluxes) {
		for (const std::vector<double>& row : constraints) {
			double total = 0.0;
			for (std::size_t i = 0; i < row.size(); ++i) {
				total += row[i] * fluxes[i];
			}
			if (std::fabs(total) > 1e-6) {
				return false;
			}
		}
		return true;
	}

	static double _dot(const std::vector<double>& a, const std::vector<double>& b) {
		double value = 0.0;
		for (std::size_t i = 0; i < a.size(); ++i) {
			value += a[i] * b[i];
		}
		return value;
	}

	static bool _nextBasis(std::size_t variableCount,
	                       std::size_t basisSize,
	                       std::vector<std::size_t>* basis) {
		if (basis == nullptr) {
			return false;
		}
		if (basisSize == 0u) {
			return false;
		}
		for (std::size_t i = basisSize; i-- > 0;) {
			const std::size_t maxValue = variableCount - basisSize + i;
			if ((*basis)[i] < maxValue) {
				++(*basis)[i];
				for (std::size_t j = i + 1u; j < basisSize; ++j) {
					(*basis)[j] = (*basis)[j - 1u] + 1u;
				}
				return true;
			}
		}
		return false;
	}
};

#endif /* METABOLICFLUXBALANCESOLVER_H */
