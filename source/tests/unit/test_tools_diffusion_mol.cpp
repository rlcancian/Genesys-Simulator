// Unit tests for the N-D diffusion Method-of-Lines core.
//
// Links only against `genesys_tools` (header-only numerics), so it builds and
// runs without the kernel or Qt. Checks the discrete Laplacian with a sine mode
// that decays at a known rate in 1-D/2-D/3-D, the ~2nd-order spatial accuracy,
// mass conservation under Neumann boundaries, that the two solvers agree on the
// same system, and that bad config is handled.
//
// Build target: genesys_test_tools_diffusion_mol.

#include <gtest/gtest.h>

#include <cmath>
#include <string>
#include <vector>

#include "tools/DiffusionMethodOfLinesSystem.h"
#include "tools/OdeSolverFactory.h"

namespace {
using DMS = DiffusionMethodOfLinesSystem;
constexpr double PI = 3.14159265358979323846;

bool integrate(const DMS& sys, const std::string& key, std::vector<double>& field,
               double T, unsigned int macroSteps) {
	auto solver = OdeSolverFactory::create(key);
	std::vector<double> next(field.size(), 0.0);
	const double dt = T / macroSteps;
	double t = 0.0;
	for (unsigned int s = 0; s < macroSteps; ++s) {
		if (!solver->advance(sys, t, dt, field.data(), next.data())) return false;
		field = next;
		t += dt;
	}
	return true;
}

double eigenmodeMaxError(const std::vector<double>& y0, const std::vector<double>& yT,
                         double lambda, double T) {
	const double factor = std::exp(-lambda * T);
	double e = 0.0;
	for (std::size_t i = 0; i < yT.size(); ++i) {
		e = std::max(e, std::fabs(yT[i] - y0[i] * factor));
	}
	return e;
}
} // namespace

TEST(DiffusionMolTest, OneDimensionalSineModeDecaysAtDiscreteRate) {
	DMS sys({41}, {0.05}, 0.2, DMS::Boundary::Dirichlet);
	ASSERT_TRUE(sys.isValid());
	std::vector<double> y(sys.totalNodes());
	sys.fillSineModes({2}, 1.0, y.data());
	const std::vector<double> y0 = y;
	const double lambda = sys.sineModeDecayRate({2});
	ASSERT_TRUE(integrate(sys, "DormandPrince54", y, 0.5, 10));
	EXPECT_LT(eigenmodeMaxError(y0, y, lambda, 0.5), 1e-7);
}

TEST(DiffusionMolTest, TwoDimensionalProductModeDecays) {
	DMS sys({21, 21}, {0.05, 0.05}, 0.15, DMS::Boundary::Dirichlet);
	std::vector<double> y(sys.totalNodes());
	sys.fillSineModes({1, 2}, 1.0, y.data());
	const std::vector<double> y0 = y;
	const double lambda = sys.sineModeDecayRate({1, 2});
	ASSERT_TRUE(integrate(sys, "DormandPrince54", y, 0.4, 8));
	EXPECT_LT(eigenmodeMaxError(y0, y, lambda, 0.4), 1e-7);
}

TEST(DiffusionMolTest, ThreeDimensionalProductModeDecays) {
	DMS sys({13, 13, 13}, {0.08, 0.08, 0.08}, 0.1, DMS::Boundary::Dirichlet);
	EXPECT_EQ(sys.totalNodes(), 13u * 13u * 13u);
	std::vector<double> y(sys.totalNodes());
	sys.fillSineModes({1, 1, 2}, 1.0, y.data());
	const std::vector<double> y0 = y;
	const double lambda = sys.sineModeDecayRate({1, 1, 2});
	ASSERT_TRUE(integrate(sys, "DormandPrince54", y, 0.3, 6));
	EXPECT_LT(eigenmodeMaxError(y0, y, lambda, 0.3), 1e-6);
}

TEST(DiffusionMolTest, SpatialDiscretizationIsSecondOrder) {
	// Compare to the CONTINUOUS solution sin(pi x) exp(-D pi^2 t); halving h
	// must reduce the error by ~4x (order ~2).
	const double D = 0.1, T = 0.2, L = 1.0;
	auto errorFor = [&](unsigned int n) {
		const double h = L / (n - 1);
		DMS sys({n}, {h}, D, DMS::Boundary::Dirichlet);
		std::vector<double> y(sys.totalNodes());
		sys.fillSineModes({1}, 1.0, y.data());
		std::vector<double> next(y.size());
		OdeSolverFactory::create("DormandPrince54")->advance(sys, 0.0, T, y.data(), next.data());
		double e = 0.0;
		for (unsigned int m = 0; m < n; ++m) {
			const double x = m * h;
			const double exact = std::sin(PI * x / L) * std::exp(-D * PI * PI * T);
			e = std::max(e, std::fabs(next[m] - exact));
		}
		return e;
	};
	const double e1 = errorFor(21);
	const double e2 = errorFor(41); // half h
	const double order = std::log2(e1 / e2);
	EXPECT_GT(order, 1.8);
	EXPECT_LT(order, 2.2);
}

TEST(DiffusionMolTest, NeumannConservesMass) {
	DMS sys({31, 31}, {0.1, 0.1}, 0.3, DMS::Boundary::Neumann);
	std::vector<double> y(sys.totalNodes());
	sys.fillGaussian(1.0, 0.3, y.data());
	const double m0 = sys.totalMass(y.data());
	ASSERT_TRUE(integrate(sys, "DormandPrince54", y, 0.5, 10));
	EXPECT_NEAR(sys.totalMass(y.data()), m0, 1e-9);
	EXPECT_LT(sys.maxValue(y.data()), 1.0); // peak spreads/decays
}

TEST(DiffusionMolTest, BothSolversAgreeOnSameSystem) {
	DMS sys({41}, {0.025}, 0.05, DMS::Boundary::Dirichlet);
	std::vector<double> a(sys.totalNodes());
	sys.fillSineModes({3}, 1.0, a.data());
	std::vector<double> b = a;
	ASSERT_TRUE(integrate(sys, "RungeKutta4", a, 0.2, 400));
	ASSERT_TRUE(integrate(sys, "DormandPrince54", b, 0.2, 400));
	double maxDiff = 0.0;
	for (std::size_t i = 0; i < a.size(); ++i) maxDiff = std::max(maxDiff, std::fabs(a[i] - b[i]));
	EXPECT_LT(maxDiff, 1e-5);
}

TEST(DiffusionMolTest, AdaptiveSolverStaysStableOnFineGrid) {
	// Fine grid + a Gaussian (lots of sharp detail) + one big step: the adaptive
	// DP5(4) must stay stable (bounded, close to a refined reference).
	DMS sys({101}, {0.01}, 0.4, DMS::Boundary::Neumann);
	std::vector<double> ref(sys.totalNodes());
	sys.fillGaussian(1.0, 0.05, ref.data());
	std::vector<double> dp = ref;
	const double T = 0.02;

	std::vector<double> a = ref, b(a.size());
	auto refSolver = OdeSolverFactory::create("DormandPrince54");
	const double dt = T / 40;
	double t = 0.0;
	for (int i = 0; i < 40; ++i) { refSolver->advance(sys, t, dt, a.data(), b.data()); a = b; t += dt; }
	ref = a;

	ASSERT_TRUE(integrate(sys, "DormandPrince54", dp, T, 1)); // one big step
	double e = 0.0;
	for (std::size_t i = 0; i < ref.size(); ++i) e = std::max(e, std::fabs(dp[i] - ref[i]));
	EXPECT_LT(e, 1e-3);
	EXPECT_LT(sys.l2Norm(dp.data()), 2.0 * sys.l2Norm(ref.data())); // not blown up
}

TEST(DiffusionMolTest, RejectsInvalidConfiguration) {
	EXPECT_FALSE(DMS({2}, {0.1}, 0.1, DMS::Boundary::Dirichlet).isValid());        // <3 points
	EXPECT_FALSE(DMS({10}, {0.1, 0.1}, 0.1, DMS::Boundary::Dirichlet).isValid());  // size mismatch
	EXPECT_FALSE(DMS({10}, {-0.1}, 0.1, DMS::Boundary::Dirichlet).isValid());      // bad spacing
	EXPECT_EQ(DMS({2}, {0.1}, 0.1, DMS::Boundary::Dirichlet).dimension(), 0u);
}

TEST(DiffusionMolTest, RowMajorIndexingRoundTrips) {
	DMS sys({4, 5, 6}, {0.1, 0.1, 0.1}, 0.1, DMS::Boundary::Dirichlet);
	const std::vector<unsigned int> mi = {2, 3, 4};
	const unsigned int node = sys.nodeIndex(mi);
	EXPECT_EQ(sys.coordinate(node, 0), 2u);
	EXPECT_EQ(sys.coordinate(node, 1), 3u);
	EXPECT_EQ(sys.coordinate(node, 2), 4u);
}
