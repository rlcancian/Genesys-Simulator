// Integration tests for the DiffusionField plugin (source/plugins/data/Continuous).
//
// These exercise the plugin end to end — through the real Model/Simulator kernel —
// rather than only the header-only Method-of-Lines core (that is covered by
// test_tools_diffusion_mol.cpp). Here we check that the plugin: defaults to RK4,
// matches the analytical eigenmode decay, agrees between the two solvers, conserves
// mass under Neumann boundaries, works in 3-D, round-trips its configuration through
// persistence, and rejects invalid solvers/configuration at validation time.
//
// Self-contained: it links the plugin + kernel and does not modify any existing file.
//
// Build target: genesys_test_plugins_continuous_diffusion.

#include <gtest/gtest.h>

#include <cmath>
#include <string>
#include <vector>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/Persistence.h"
#include "plugins/data/Continuous/DiffusionField.h"

namespace {

// M_PI is not standard C++, so define pi locally.
constexpr double PI = 3.14159265358979323846;

// Minimal persistence runtime so a PersistenceRecord can be built for the
// save/load round-trip test without a real backend.
class FakeModelPersistenceRuntime : public Persistence_if {
public:
	bool save(std::string) override { return false; }
	bool load(std::string) override { return false; }
	bool hasChanged() override { return false; }
	void setHasChanged(bool) override {}
	bool getOption(Persistence_if::Options) override { return false; }
	void setOption(Persistence_if::Options, bool) override {}
	std::string getFormatedField(PersistenceRecord*) override { return ""; }
};

// Probe subclass that exposes DiffusionField's protected check/save/load hooks so
// the tests can drive validation and persistence directly.
class DiffusionFieldProbe : public DiffusionField {
public:
	DiffusionFieldProbe(Model* model, const std::string& name = "") : DiffusionField(model, name) {}
	bool CheckProbe(std::string& errorMessage) { return _check(errorMessage); }
	void SaveInstanceProbe(PersistenceRecord* fields, bool saveDefaultValues = false) { _saveInstance(fields, saveDefaultValues); }
	bool LoadInstanceProbe(PersistenceRecord* fields) { return _loadInstance(fields); }
};

// Decay rate of a sine mode on a uniform N-D grid (from the discrete Laplacian):
// lambda = D * N * (4/h^2) * sin^2(mode*pi / (2*(points-1))).
double discreteSineDecayRate(unsigned int dims, unsigned int points, double length,
                             double D, unsigned int mode) {
	const double h = length / (points - 1);
	const double s = std::sin(mode * PI / (2.0 * (points - 1)));
	return D * dims * (4.0 / (h * h)) * s * s;
}

} // namespace

TEST(DiffusionFieldPluginTest, DefaultsToRungeKutta4) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	DiffusionFieldProbe field(model, "DiffusionDefaultSolver");
	EXPECT_EQ(field.getOdeSolver(), "RungeKutta4");
	EXPECT_EQ(field.getDimensions(), 2u);
}

TEST(DiffusionFieldPluginTest, MatchesAnalyticalEigenmodeDecay) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	DiffusionFieldProbe field(model, "DiffusionEigenmode");
	field.setDimensions(1u);
	field.setPointsPerDimension(41u);
	field.setDomainLength(1.0);
	field.setDiffusionCoefficient(0.2);
	field.setBoundaryCondition("Dirichlet");
	field.setInitialCondition("SineMode");
	field.setInitialParameter(2.0); // mode 2
	field.setOdeSolver("DormandPrince54");

	std::string errorMessage;
	ASSERT_TRUE(field.simulate(0.0, 0.5, 0.05, errorMessage)) << errorMessage;
	EXPECT_EQ(field.getLastStatus(), "Completed");

	// Sample an interior node and compare to v0 * exp(-lambda T).
	const double h = 1.0 / 40.0;
	const unsigned int m = 13;
	const double v0 = std::sin(2.0 * PI * (m * h) / 1.0);
	const double lambda = discreteSineDecayRate(1u, 41u, 1.0, 0.2, 2u);
	const double expected = v0 * std::exp(-lambda * 0.5);
	EXPECT_NEAR(field.getFieldValue({m}), expected, 1e-4);
}

TEST(DiffusionFieldPluginTest, SolversAgree) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	auto run = [&](const std::string& solver, double sampleNode) {
		DiffusionFieldProbe field(model, "DiffusionAgree_" + solver);
		field.setDimensions(1u);
		field.setPointsPerDimension(41u);
		field.setDomainLength(1.0);
		field.setDiffusionCoefficient(0.05);
		field.setInitialCondition("SineMode");
		field.setInitialParameter(3.0);
		field.setOdeSolver(solver);
		std::string err;
		EXPECT_TRUE(field.simulate(0.0, 0.2, 0.005, err)) << err;
		return field.getFieldValue({static_cast<unsigned int>(sampleNode)});
	};
	const double rk = run("RungeKutta4", 20);
	const double dp = run("DormandPrince54", 20);
	EXPECT_NEAR(rk, dp, 1e-4);
}

TEST(DiffusionFieldPluginTest, NeumannConservesMass) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	DiffusionFieldProbe field(model, "DiffusionNeumannMass");
	field.setDimensions(2u);
	field.setPointsPerDimension(31u);
	field.setDomainLength(3.0);
	field.setDiffusionCoefficient(0.3);
	field.setBoundaryCondition("Neumann");
	field.setInitialCondition("Gaussian");
	field.setInitialParameter(0.3); // sigma
	field.setOdeSolver("DormandPrince54");

	std::string err;

	// First run: initialize the field and obtain the reference mass.
	ASSERT_TRUE(field.simulate(0.0, 0.1, 0.05, err)) << err;
	const double massStart = field.getTotalMass();

	// Continue the simulation and verify mass conservation.
	ASSERT_TRUE(field.simulate(0.1, 0.5, 0.05, err)) << err;

	EXPECT_NEAR(field.getTotalMass(), massStart, 1e-6);
	EXPECT_LT(field.getMaxValue(), 1.0); // Gaussian spreads, peak drops
}

TEST(DiffusionFieldPluginTest, RunsInThreeDimensions) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	DiffusionFieldProbe field(model, "Diffusion3D");
	field.setDimensions(3u);          // works for any N
	field.setPointsPerDimension(13u);
	field.setDomainLength(1.0);
	field.setDiffusionCoefficient(0.1);
	field.setInitialCondition("SineMode");
	field.setInitialParameter(1.0);
	field.setOdeSolver("DormandPrince54");

	std::string err;
	ASSERT_TRUE(field.simulate(0.0, 0.3, 0.05, err)) << err;
	EXPECT_EQ(field.getField().size(), 13u * 13u * 13u);

	const unsigned int c = 6; // center node
	const double h = 1.0 / 12.0;
	const double v0 = std::pow(std::sin(PI * (c * h)), 3.0);
	const double lambda = discreteSineDecayRate(3u, 13u, 1.0, 0.1, 1u);
	EXPECT_NEAR(field.getFieldValue({c, c, c}), v0 * std::exp(-lambda * 0.3), 1e-3);
}

TEST(DiffusionFieldPluginTest, ConfigRoundTripsThroughPersistence) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	DiffusionFieldProbe source(model, "DiffusionPersistSource");
	source.setDimensions(3u);
	source.setPointsPerDimension(17u);
	source.setDomainLength(2.5);
	source.setDiffusionCoefficient(0.42);
	source.setBoundaryCondition("Neumann");
	source.setInitialCondition("Gaussian");
	source.setInitialParameter(0.2);
	source.setOdeSolver("DormandPrince54");

	FakeModelPersistenceRuntime persistence;
	PersistenceRecord fields(persistence);
	source.SaveInstanceProbe(&fields, true);

	DiffusionFieldProbe loaded(model, "DiffusionPersistLoaded");
	ASSERT_TRUE(loaded.LoadInstanceProbe(&fields));
	EXPECT_EQ(loaded.getDimensions(), 3u);
	EXPECT_EQ(loaded.getPointsPerDimension(), 17u);
	EXPECT_DOUBLE_EQ(loaded.getDomainLength(), 2.5);
	EXPECT_DOUBLE_EQ(loaded.getDiffusionCoefficient(), 0.42);
	EXPECT_EQ(loaded.getBoundaryCondition(), "Neumann");
	EXPECT_EQ(loaded.getInitialCondition(), "Gaussian");
	EXPECT_EQ(loaded.getOdeSolver(), "DormandPrince54");
}

TEST(DiffusionFieldPluginTest, RejectsInvalidSolverAndConfig) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	DiffusionFieldProbe badSolver(model, "DiffusionBadSolver");
	badSolver.setOdeSolver("NotARealSolver");
	std::string e1;
	EXPECT_FALSE(badSolver.CheckProbe(e1));
	EXPECT_NE(e1.find("unknown ODE solver"), std::string::npos);

	DiffusionFieldProbe badGrid(model, "DiffusionBadGrid");
	badGrid.setPointsPerDimension(2u); // below the minimum of 3
	std::string e2;
	EXPECT_FALSE(badGrid.CheckProbe(e2));

	DiffusionFieldProbe badBoundary(model, "DiffusionBadBoundary");
	badBoundary.setBoundaryCondition("Periodic"); // unsupported
	std::string e3;
	EXPECT_FALSE(badBoundary.CheckProbe(e3));
}
