#ifndef GLPKFLUXBALANCESOLVER_H
#define GLPKFLUXBALANCESOLVER_H

#include "tools/Biochemical/MetabolicFluxBalanceSolver.h"

#ifdef GENESYS_HAVE_GLPK

#include <glpk.h>
#include <cstring>
#include <limits>

/**
 * GLPK-backed LP solver for flux-balance problems.
 *
 * Solves the standard FBA problem:
 *   maximize/minimize c^T v
 *   subject to S v = 0   (steady-state mass balance)
 *              lb <= v <= ub
 *
 * Uses the GLPK simplex method (primal). This replaces the basis-enumeration
 * fallback in MetabolicFluxBalanceSolver for large metabolic networks
 * (M. genitalium: 641 reactions; yeast-GEM: 4131 reactions).
 *
 * System dependency: libglpk-dev (LGPL-3.0). Enabled when CMake detects GLPK
 * and sets GENESYS_HAVE_GLPK. Falls back to MetabolicFluxBalanceSolver for
 * small models when GLPK is not available.
 *
 * Installation:
 *   Ubuntu/Debian:  sudo apt-get install libglpk-dev
 *   Fedora/RHEL:    sudo dnf install glpk-devel
 *   macOS (Brew):   brew install glpk
 *
 * After installation, re-run CMake (any preset) to pick up GLPK automatically:
 *   cmake --preset terminal-app && cmake --build build/terminal-app
 *
 * CMake will print:
 *   GenESyS: GLPK found — MetabolicFluxBalance ... will use GLPK LP solver
 */
class GlpkFluxBalanceSolver {
public:
	static MetabolicFluxBalanceSolver::Solution solve(const MetabolicFluxBalanceSolver::Problem& problem) {
		MetabolicFluxBalanceSolver::Solution solution;
		const std::size_t nCols = problem.objective.size();
		const std::size_t nRows = problem.stoichiometry.size();

		if (nCols == 0u) {
			solution.errorMessage = "Empty objective vector.";
			return solution;
		}
		if (problem.lowerBounds.size() != nCols || problem.upperBounds.size() != nCols) {
			solution.errorMessage = "Invalid flux bound vector dimensions.";
			return solution;
		}

		glp_prob* lp = glp_create_prob();
		glp_set_prob_name(lp, "fba");
		glp_set_obj_dir(lp, problem.maximize ? GLP_MAX : GLP_MIN);

		// Add constraint rows (one per metabolite: S_i * v = 0)
		if (nRows > 0u) {
			glp_add_rows(lp, static_cast<int>(nRows));
			for (std::size_t i = 0; i < nRows; ++i) {
				glp_set_row_bnds(lp, static_cast<int>(i) + 1, GLP_FX, 0.0, 0.0);
			}
		}

		// Add flux variables (columns) with bounds and objective coefficients
		glp_add_cols(lp, static_cast<int>(nCols));
		for (std::size_t j = 0; j < nCols; ++j) {
			const int col = static_cast<int>(j) + 1;
			const double lb = problem.lowerBounds[j];
			const double ub = problem.upperBounds[j];
			const double kInf = std::numeric_limits<double>::infinity();
			if (lb == ub) {
				glp_set_col_bnds(lp, col, GLP_FX, lb, ub);
			} else if (lb <= -kInf && ub >= kInf) {
				glp_set_col_bnds(lp, col, GLP_FR, 0.0, 0.0);
			} else if (lb <= -kInf) {
				glp_set_col_bnds(lp, col, GLP_UP, 0.0, ub);
			} else if (ub >= kInf) {
				glp_set_col_bnds(lp, col, GLP_LO, lb, 0.0);
			} else {
				glp_set_col_bnds(lp, col, GLP_DB, lb, ub);
			}
			glp_set_obj_coef(lp, col, problem.objective[j]);
		}

		// Load stoichiometry matrix as sparse triplets (GLPK uses 1-based indices)
		// Count non-zeros first
		std::size_t nnz = 0u;
		for (const auto& row : problem.stoichiometry) {
			for (double val : row) {
				if (val != 0.0) ++nnz;
			}
		}

		if (nnz > 0u && nRows > 0u) {
			std::vector<int> ia(nnz + 1u);
			std::vector<int> ja(nnz + 1u);
			std::vector<double> ar(nnz + 1u);
			std::size_t k = 1u;
			for (std::size_t i = 0; i < nRows; ++i) {
				for (std::size_t j = 0; j < nCols && j < problem.stoichiometry[i].size(); ++j) {
					const double val = problem.stoichiometry[i][j];
					if (val != 0.0) {
						ia[k] = static_cast<int>(i) + 1;
						ja[k] = static_cast<int>(j) + 1;
						ar[k] = val;
						++k;
					}
				}
			}
			glp_load_matrix(lp, static_cast<int>(nnz), ia.data(), ja.data(), ar.data());
		}

		// Solve with simplex, messages suppressed
		glp_smcp parm;
		glp_init_smcp(&parm);
		parm.msg_lev = GLP_MSG_OFF;
		const int status = glp_simplex(lp, &parm);

		if (status == 0 && glp_get_status(lp) == GLP_OPT) {
			solution.feasible = true;
			solution.objectiveValue = glp_get_obj_val(lp);
			solution.fluxes.resize(nCols);
			for (std::size_t j = 0; j < nCols; ++j) {
				solution.fluxes[j] = glp_get_col_prim(lp, static_cast<int>(j) + 1);
			}
		} else {
			const int glpStatus = glp_get_status(lp);
			if (glpStatus == GLP_INFEAS || glpStatus == GLP_NOFEAS) {
				solution.errorMessage = "GLPK: problem is infeasible.";
			} else if (glpStatus == GLP_UNBND) {
				solution.errorMessage = "GLPK: problem is unbounded.";
			} else {
				solution.errorMessage = "GLPK: simplex failed (status=" + std::to_string(status) + ", glpStatus=" + std::to_string(glpStatus) + ").";
			}
		}

		glp_delete_prob(lp);
		return solution;
	}
};

#endif /* GENESYS_HAVE_GLPK */

#endif /* GLPKFLUXBALANCESOLVER_H */
