#include <gtest/gtest.h>

#include <type_traits>

#include "tools/analysis/SolverDefaultImpl.h"
#include "tools/analysis/TraitsAnalysis.h"

namespace {

// Provides an integrand with a known exact integral for quadrature validation.
double quadratic(double value, double unusedParameter) {
    (void) unusedParameter;
    return value * value;
}

} // namespace

// Test objective: verifies SolverDefaultImpl is selected by TraitsAnalysis and integrates x^2.
TEST(AnalysisSolverTest, DefaultTraitSelectsSimpsonSolverAndIntegratesQuadratic) {
    using DefaultSolver = TraitsAnalysis<Solver_if>::Implementation;
    static_assert(std::is_same_v<DefaultSolver, SolverDefaultImpl>);

    DefaultSolver solver(TraitsAnalysis<Solver_if>::Precision, 100U);
    const double integral = solver.integrate(0.0, 1.0, quadratic, 0.0);

    EXPECT_NEAR(integral, 1.0 / 3.0, 1e-10);
}
