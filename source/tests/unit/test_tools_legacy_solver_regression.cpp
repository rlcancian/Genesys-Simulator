#include <gtest/gtest.h>

#include "tools/Continuous/SolverDefaultImpl1.h"

namespace {

double quadratic2(double x, double) {
    return x * x;
}

double quadratic3(double x, double, double) {
    return x * x;
}

double quadratic4(double x, double, double, double) {
    return x * x;
}

double quadratic5(double x, double, double, double, double) {
    return x * x;
}

double zeroDerivative2(double, double) {
    return 0.0;
}

double zeroDerivative3(double, double, double) {
    return 0.0;
}

double zeroDerivative4(double, double, double, double) {
    return 0.0;
}

double zeroDerivative5(double, double, double, double, double) {
    return 0.0;
}

constexpr double kIntegralOfXSquaredFromZeroToOne = 1.0 / 3.0;
constexpr double kTolerance = 1.0e-12;

} // namespace

TEST(LegacySolverQuadratureRegression, EvenStepCountIntegratesQuadraticForEveryOverload) {
    SolverDefaultImpl1 solver(kTolerance, 4u);

    EXPECT_NEAR(solver.integrate(0.0, 1.0, quadratic2, 0.0),
                kIntegralOfXSquaredFromZeroToOne,
                kTolerance);
    EXPECT_NEAR(solver.integrate(0.0, 1.0, quadratic3, 0.0, 0.0),
                kIntegralOfXSquaredFromZeroToOne,
                kTolerance);
    EXPECT_NEAR(solver.integrate(0.0, 1.0, quadratic4, 0.0, 0.0, 0.0),
                kIntegralOfXSquaredFromZeroToOne,
                kTolerance);
    EXPECT_NEAR(solver.integrate(0.0, 1.0, quadratic5, 0.0, 0.0, 0.0, 0.0),
                kIntegralOfXSquaredFromZeroToOne,
                kTolerance);
}

TEST(LegacySolverQuadratureRegression, OddStepCountPreservesSimpsonContractForTwoArgumentFunction) {
    SolverDefaultImpl1 solver(kTolerance, 3u);

    EXPECT_NEAR(solver.integrate(0.0, 1.0, quadratic2, 0.0),
                kIntegralOfXSquaredFromZeroToOne,
                kTolerance);
}

TEST(LegacySolverQuadratureRegression, OddStepCountPreservesSimpsonContractForThreeArgumentFunction) {
    SolverDefaultImpl1 solver(kTolerance, 3u);

    EXPECT_NEAR(solver.integrate(0.0, 1.0, quadratic3, 0.0, 0.0),
                kIntegralOfXSquaredFromZeroToOne,
                kTolerance);
}

TEST(LegacySolverQuadratureRegression, OddStepCountPreservesSimpsonContractForFourArgumentFunction) {
    SolverDefaultImpl1 solver(kTolerance, 3u);

    EXPECT_NEAR(solver.integrate(0.0, 1.0, quadratic4, 0.0, 0.0, 0.0),
                kIntegralOfXSquaredFromZeroToOne,
                kTolerance);
}

TEST(LegacySolverQuadratureRegression, OddStepCountPreservesSimpsonContractForFiveArgumentFunction) {
    SolverDefaultImpl1 solver(kTolerance, 3u);

    EXPECT_NEAR(solver.integrate(0.0, 1.0, quadratic5, 0.0, 0.0, 0.0, 0.0),
                kIntegralOfXSquaredFromZeroToOne,
                kTolerance);
}

TEST(LegacySolverDerivativeRegression, ZeroDerivativePreservesInitialValueForTwoArgumentFunction) {
    SolverDefaultImpl1 solver;

    EXPECT_DOUBLE_EQ(solver.derivate(0.0, 42.0, zeroDerivative2, 7.0), 42.0);
}

TEST(LegacySolverDerivativeRegression, ZeroDerivativePreservesInitialValueForThreeArgumentFunction) {
    SolverDefaultImpl1 solver;

    EXPECT_DOUBLE_EQ(solver.derivate(0.0, 42.0, zeroDerivative3, 7.0, 11.0), 42.0);
}

TEST(LegacySolverDerivativeRegression, ZeroDerivativePreservesInitialValueForFourArgumentFunction) {
    SolverDefaultImpl1 solver;

    EXPECT_DOUBLE_EQ(solver.derivate(0.0, 42.0, zeroDerivative4, 7.0, 11.0, 13.0), 42.0);
}

TEST(LegacySolverDerivativeRegression, ZeroDerivativePreservesInitialValueForFiveArgumentFunction) {
    SolverDefaultImpl1 solver;

    EXPECT_DOUBLE_EQ(solver.derivate(0.0, 42.0, zeroDerivative5, 7.0, 11.0, 13.0, 17.0), 42.0);
}
