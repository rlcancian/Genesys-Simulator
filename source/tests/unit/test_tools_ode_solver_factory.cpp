// Unit tests for the ODE solvers (factory + Dormand-Prince 5(4)).
//
// These link only against `genesys_tools` (the solver code is header-only under
// source/tools), so they build and run without the kernel or Qt. They cover the
// factory, the new solver's accuracy against known exact solutions, how fast its
// error shrinks, and that bad input is handled.
//
// Build target: genesys_test_tools_ode_solver_factory.

#include <gtest/gtest.h>

#include <cmath>
#include <memory>
#include <vector>

#include "tools/DormandPrince54OdeSolver.h"
#include "tools/OdeSolverFactory.h"
#include "tools/OdeSystem_if.h"
#include "tools/RungeKutta4OdeSolver.h"

namespace {

// dy/dt = -k y  =>  y(t) = y0 * exp(-k t)
class ExponentialDecay : public OdeSystem_if {
public:
	explicit ExponentialDecay(double k) : _k(k) {}
	unsigned int dimension() const override { return 1; }
	void evaluate(double, const double* y, double* dydt) const override { dydt[0] = -_k * y[0]; }
private:
	double _k;
};

// Harmonic oscillator: y0=pos, y1=vel; pos(t)=cos(w t) when pos(0)=1, vel(0)=0.
class HarmonicOscillator : public OdeSystem_if {
public:
	explicit HarmonicOscillator(double w) : _w(w) {}
	unsigned int dimension() const override { return 2; }
	void evaluate(double, const double* y, double* dydt) const override {
		dydt[0] = y[1];
		dydt[1] = -_w * _w * y[0];
	}
private:
	double _w;
};

// Integrate over [0,T] with `steps` equal advance() calls.
bool integrateTo(const OdeSolver_if& solver, const OdeSystem_if& system,
                 double T, unsigned int steps, std::vector<double>& state) {
	const double dt = T / static_cast<double>(steps);
	std::vector<double> next(state.size(), 0.0);
	double t = 0.0;
	for (unsigned int s = 0; s < steps; ++s) {
		if (!solver.advance(system, t, dt, state.data(), next.data())) return false;
		state = next;
		t += dt;
	}
	return true;
}

} // namespace

// Factory tests.

TEST(OdeSolverFactoryTest, RegistersBothBuiltInSolvers) {
	EXPECT_TRUE(OdeSolverFactory::isRegistered("RungeKutta4"));
	EXPECT_TRUE(OdeSolverFactory::isRegistered("DormandPrince54"));
	EXPECT_FALSE(OdeSolverFactory::isRegistered("Nonexistent"));
	EXPECT_GE(OdeSolverFactory::availableKeys().size(), 2u);
}

TEST(OdeSolverFactoryTest, CreatesNonNullSolversFromEnum) {
	auto rk = OdeSolverFactory::create(OdeSolverType::RungeKutta4);
	auto dp = OdeSolverFactory::create(OdeSolverType::DormandPrince54);
	ASSERT_NE(rk, nullptr);
	ASSERT_NE(dp, nullptr);
	EXPECT_NE(dynamic_cast<RungeKutta4OdeSolver*>(rk.get()), nullptr);
	EXPECT_NE(dynamic_cast<DormandPrince54OdeSolver*>(dp.get()), nullptr);
}

TEST(OdeSolverFactoryTest, CreatesSolversFromStringKey) {
	auto rk = OdeSolverFactory::create(std::string("RungeKutta4"));
	auto dp = OdeSolverFactory::create(std::string("DormandPrince54"));
	ASSERT_NE(rk, nullptr);
	ASSERT_NE(dp, nullptr);
	EXPECT_NE(dynamic_cast<RungeKutta4OdeSolver*>(rk.get()), nullptr);
	EXPECT_NE(dynamic_cast<DormandPrince54OdeSolver*>(dp.get()), nullptr);
}

TEST(OdeSolverFactoryTest, UnknownKeyFallsBackToRungeKutta4) {
	auto solver = OdeSolverFactory::create(std::string("DoesNotExist"));
	ASSERT_NE(solver, nullptr);
	EXPECT_NE(dynamic_cast<RungeKutta4OdeSolver*>(solver.get()), nullptr);
}

TEST(OdeSolverFactoryTest, KeyEnumRoundTrip) {
	OdeSolverType type;
	EXPECT_TRUE(OdeSolverFactory::fromKey("RungeKutta4", type));
	EXPECT_EQ(type, OdeSolverType::RungeKutta4);
	EXPECT_TRUE(OdeSolverFactory::fromKey("DormandPrince54", type));
	EXPECT_EQ(type, OdeSolverType::DormandPrince54);
	EXPECT_FALSE(OdeSolverFactory::fromKey("bogus", type));

	EXPECT_EQ(OdeSolverFactory::toKey(OdeSolverType::RungeKutta4), "RungeKutta4");
	EXPECT_EQ(OdeSolverFactory::toKey(OdeSolverType::DormandPrince54), "DormandPrince54");
}

TEST(OdeSolverFactoryTest, RegisterCreatorIsOpenForExtension) {
	const std::string key = "CustomRk4Alias_UT";
	EXPECT_FALSE(OdeSolverFactory::isRegistered(key));
	const bool added = OdeSolverFactory::registerCreator(
			key, [] { return std::make_unique<RungeKutta4OdeSolver>(); });
	EXPECT_TRUE(added);
	EXPECT_TRUE(OdeSolverFactory::isRegistered(key));
	// Registering the same key again is rejected (no silent override).
	const bool addedAgain = OdeSolverFactory::registerCreator(
			key, [] { return std::make_unique<RungeKutta4OdeSolver>(); });
	EXPECT_FALSE(addedAgain);
	auto solver = OdeSolverFactory::create(key);
	ASSERT_NE(solver, nullptr);
}

// Dormand-Prince 5(4) accuracy against known solutions.

TEST(DormandPrince54Test, MatchesExponentialDecayAnalyticalSolution) {
	ExponentialDecay decay(0.7);
	DormandPrince54OdeSolver dp;
	const double y0 = 5.0, T = 3.0;
	std::vector<double> state{y0};
	ASSERT_TRUE(integrateTo(dp, decay, T, 30, state));
	EXPECT_NEAR(state[0], y0 * std::exp(-0.7 * T), 1e-6);
}

TEST(DormandPrince54Test, MatchesHarmonicOscillatorAnalyticalSolution) {
	HarmonicOscillator osc(2.0);
	DormandPrince54OdeSolver dp;
	const double T = 2.5;
	std::vector<double> state{1.0, 0.0};
	ASSERT_TRUE(integrateTo(dp, osc, T, 50, state));
	EXPECT_NEAR(state[0], std::cos(2.0 * T), 1e-6);
	EXPECT_NEAR(state[1], -2.0 * std::sin(2.0 * T), 1e-6);
}

TEST(DormandPrince54Test, IsAtLeastAsAccurateAsRk4OnSameMacroGrid) {
	ExponentialDecay decay(1.0);
	const double y0 = 1.0, T = 4.0, exact = y0 * std::exp(-T);
	RungeKutta4OdeSolver rk;
	DormandPrince54OdeSolver dp(1e-8, 1e-11);

	std::vector<double> sRk{y0}, sDp{y0};
	ASSERT_TRUE(integrateTo(rk, decay, T, 20, sRk));
	ASSERT_TRUE(integrateTo(dp, decay, T, 20, sDp));
	EXPECT_LT(std::fabs(sDp[0] - exact), std::fabs(sRk[0] - exact));
}

TEST(DormandPrince54Test, ToleranceTighteningReducesError) {
	ExponentialDecay decay(1.3);
	const double y0 = 2.0, T = 5.0, exact = y0 * std::exp(-1.3 * T);
	DormandPrince54OdeSolver loose(1e-3, 1e-6);
	DormandPrince54OdeSolver tight(1e-9, 1e-12);

	std::vector<double> sLoose{y0}, sTight{y0};
	ASSERT_TRUE(integrateTo(loose, decay, T, 1, sLoose));
	ASSERT_TRUE(integrateTo(tight, decay, T, 1, sTight));
	EXPECT_LT(std::fabs(sTight[0] - exact), std::fabs(sLoose[0] - exact));
}

// RK4 error-shrink check (sanity check on the old solver behind the factory).

TEST(RungeKutta4Test, ExhibitsFourthOrderConvergence) {
	ExponentialDecay decay(1.0);
	const double y0 = 1.0, T = 2.0, exact = std::exp(-T);
	auto rk = OdeSolverFactory::create(OdeSolverType::RungeKutta4);

	std::vector<double> s1{y0}, s2{y0};
	ASSERT_TRUE(integrateTo(*rk, decay, T, 20, s1));
	ASSERT_TRUE(integrateTo(*rk, decay, T, 40, s2)); // halve the step
	const double e1 = std::fabs(s1[0] - exact);
	const double e2 = std::fabs(s2[0] - exact);
	const double order = std::log2(e1 / e2);
	EXPECT_GT(order, 3.7); // ~4 expected for a 4th-order method
}

// Bad input must be handled the same as the old solver.

TEST(OdeSolverContractTest, RejectInvalidArguments) {
	ExponentialDecay decay(1.0);
	DormandPrince54OdeSolver dp;
	RungeKutta4OdeSolver rk;
	double in[] = {3.0};
	double out[] = {-1.0};

	EXPECT_FALSE(dp.advance(decay, 0.0, -1.0, in, out));
	EXPECT_FALSE(rk.advance(decay, 0.0, -1.0, in, out));
	EXPECT_FALSE(dp.advance(decay, 0.0, 1.0, nullptr, out));
	EXPECT_FALSE(dp.advance(decay, 0.0, 1.0, in, nullptr));
}

TEST(OdeSolverContractTest, ZeroStepCopiesStateUnchanged) {
	ExponentialDecay decay(1.0);
	DormandPrince54OdeSolver dp;
	double in[] = {3.0};
	double out[] = {-1.0};
	ASSERT_TRUE(dp.advance(decay, 0.0, 0.0, in, out));
	EXPECT_DOUBLE_EQ(out[0], 3.0);
}

TEST(OdeSolverContractTest, ZeroDimensionSystemIsRejected) {
	struct ZeroDim : OdeSystem_if {
		unsigned int dimension() const override { return 0; }
		void evaluate(double, const double*, double*) const override {}
	} zero;
	DormandPrince54OdeSolver dp;
	double in[] = {1.0};
	double out[] = {0.0};
	EXPECT_FALSE(dp.advance(zero, 0.0, 1.0, in, out));
}
