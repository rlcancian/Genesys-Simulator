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
#include "kernel/simulator/Event.h"
#include "plugins/data/Continuous/DiffusionField.h"
#include "plugins/data/Continuous/ODESolver.h"
#include "plugins/components/Continuous/DiffusionSimulate.h"

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
	// Triggers the kernel's between-replications hook, which (with stepsToSynchronize > 0)
	// posts the first InternalEvent onto the model's future-event queue.
	void InitBetweenReplicationsProbe() { _initBetweenReplications(); }
	void startAtNonzeroGlobalTime(void*) {
		setStepsToSynchronize(2u);
		_startSucceeded = simulate(_startErrorMessage);
	}
	bool startSucceeded() const { return _startSucceeded; }
	const std::string& startErrorMessage() const { return _startErrorMessage; }

private:
	bool _startSucceeded = false;
	std::string _startErrorMessage;
};

// Probe exposing DiffusionSimulate's protected check/dispatch hooks so a test can
// validate the component and drive it as if an entity had arrived.
class DiffusionSimulateProbe : public DiffusionSimulate {
public:
	DiffusionSimulateProbe(Model* model, const std::string& name = "") : DiffusionSimulate(model, name) {}
	bool CheckProbe(std::string& errorMessage) { return _check(errorMessage); }
	void DispatchEventProbe(Entity* entity) { _onDispatchEvent(entity, 0u); }
};

struct StartDiffusionAtCalendarTime {
	DiffusionField* field;
	Model* model;

	void schedule(SimulationEvent*) {
		auto* event = new InternalEvent(100.0, "StartDiffusionAt100");
		event->setEventHandler(static_cast<DiffusionFieldProbe*>(field),
			&DiffusionFieldProbe::startAtNonzeroGlobalTime, nullptr);
		model->getFutureEvents()->insert(event);
	}
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

TEST(ODESolverPluginTest, DefaultsToOneStepCalendarSynchronization) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	ODESolver solver(model, "ReactiveOdeSolver");
	EXPECT_EQ(solver.getStepsToSynchronize(), 1u);
}

TEST(ODESolverPluginTest, RejectsPersistedNonzeroLegacyStartTime) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	FakeModelPersistenceRuntime runtime;
	PersistenceRecord fields(runtime);
	fields.saveField("startTime", 2.0, 0.0, true);
	EXPECT_EQ(ODESolver::LoadInstance(model, &fields), nullptr);
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

	// Restart from the supported zero time origin and verify mass conservation.
	ASSERT_TRUE(field.simulate(0.0, 0.5, 0.05, err)) << err;

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

// --- DiffusionSimulate component (discrete -> continuous bridge) ---

TEST(DiffusionFieldPluginTest, DiffusionSimulateComponentRunsField) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	DiffusionField field(model, "FieldForComponent");
	field.setDimensions(1u);
	field.setPointsPerDimension(21u);
	field.setDomainLength(1.0);
	field.setDiffusionCoefficient(0.1);
	field.setInitialCondition("SineMode");
	field.setInitialParameter(1.0);
	field.setOdeSolver("DormandPrince54");
	field.setStartTime(0.0);
	field.setStopTime(0.1);
	field.setStepSize(0.05);

	DiffusionSimulateProbe component(model, "DiffusionComp");
	component.setDiffusionField(&field);
	component.setUseFieldTimeWindow(true);

	std::string checkError;
	ASSERT_TRUE(component.CheckProbe(checkError)) << checkError;

	// Dispatch with a null entity: the component runs the referenced field and
	// safely skips forwarding, exactly like BioSimulate.
	component.DispatchEventProbe(nullptr);
	EXPECT_TRUE(component.getLastSucceeded());
	EXPECT_EQ(field.getLastStatus(), "Completed");
}

TEST(DiffusionFieldPluginTest, DiffusionSimulateRejectsMissingField) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	DiffusionSimulateProbe component(model, "DiffusionCompNoField");
	std::string checkError;
	EXPECT_FALSE(component.CheckProbe(checkError)); // no DiffusionField referenced

	component.DispatchEventProbe(nullptr); // must not crash
	EXPECT_FALSE(component.getLastSucceeded());
}

// --- Event-clock synchronization (StepsToSynchronize) ---

TEST(DiffusionFieldPluginTest, StepsToSynchronizeAdvancesWithEventClock) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	DiffusionFieldProbe field(model, "DiffusionAutoSchedule");
	field.setDimensions(1u);
	field.setPointsPerDimension(11u);
	field.setDomainLength(1.0);
	field.setDiffusionCoefficient(0.1);
	field.setInitialCondition("SineMode");
	field.setInitialParameter(1.0);
	field.setOdeSolver("RungeKutta4");
	field.setStepsToSynchronize(1u);
	field.setStartTime(0.0);
	field.setStopTime(0.15);
	field.setStepSize(0.05);

	// Between-replications init posts the first InternalEvent at t = origin + step.
	field.InitBetweenReplicationsProbe();
	ASSERT_FALSE(model->getFutureEvents()->empty());
	EXPECT_NEAR(model->getFutureEvents()->front()->getTime(), 0.05, 1e-9);
	EXPECT_DOUBLE_EQ(field.getCurrentTime(), 0.0);

	// Drain the queue like the kernel would: each event advances the field one step
	// and reschedules the next, so the continuous solve tracks the event clock.
	unsigned int guard = 0;
	while (!model->getFutureEvents()->empty() && guard++ < 100u) {
		Event* event = model->getFutureEvents()->front();
		model->getFutureEvents()->pop_front();
		static_cast<InternalEvent*>(event)->dispatchEvent();
		delete event;
	}

	EXPECT_GE(field.getCurrentTime(), 0.15);
	EXPECT_EQ(field.getLastStatus(), "Completed");
}

TEST(DiffusionFieldPluginTest, SynchronizesAfterThreeStepsAndAtFinalPartialBatch) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    DiffusionFieldProbe field(model, "DiffusionThreeStepSync");
    field.setDimensions(1u);
    field.setPointsPerDimension(11u);
    field.setStepsToSynchronize(3u);
    field.setStepSize(0.05);
    field.setStopTime(0.25);

    field.InitBetweenReplicationsProbe();
    ASSERT_TRUE(field.isAwaitingSynchronization());
    EXPECT_DOUBLE_EQ(field.getCurrentTime(), 0.0);
    ASSERT_FALSE(model->getFutureEvents()->empty());
    EXPECT_NEAR(model->getFutureEvents()->front()->getTime(), 0.15, 1e-9);

    Event* first = model->getFutureEvents()->front();
    model->getFutureEvents()->pop_front();
    static_cast<InternalEvent*>(first)->dispatchEvent();
    delete first;

    ASSERT_TRUE(field.isAwaitingSynchronization());
    EXPECT_NEAR(field.getCurrentTime(), 0.15, 1e-9);
    ASSERT_FALSE(model->getFutureEvents()->empty());
    EXPECT_NEAR(model->getFutureEvents()->front()->getTime(), 0.25, 1e-9);

    Event* finalEvent = model->getFutureEvents()->front();
    model->getFutureEvents()->pop_front();
    static_cast<InternalEvent*>(finalEvent)->dispatchEvent();
    delete finalEvent;

    EXPECT_TRUE(model->getFutureEvents()->empty());
    EXPECT_FALSE(field.isAwaitingSynchronization());
    EXPECT_NEAR(field.getCurrentTime(), 0.25, 1e-9);
    EXPECT_EQ(field.getLastStatus(), "Completed");
}

TEST(DiffusionFieldPluginTest, SynchronizationIntervalsMatchStepCountsAndFinalPartialSteps) {
	struct Case {
		double stopTime;
		double stepSize;
		unsigned int stepsToSynchronize;
		std::vector<double> expectedTimes;
	};
	const std::vector<Case> cases = {
		{10.0, 1.0, 1u, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}},
		{10.0, 1.0, 2u, {2, 4, 6, 8, 10}},
		{10.0, 1.0, 3u, {3, 6, 9, 10}},
		{10.0, 1.0, 5u, {5, 10}},
		{10.0, 1.0, 20u, {10}},
		{1.0, 0.1, 3u, {0.3, 0.6, 0.9, 1.0}},
		{2.5, 1.0, 2u, {2, 2.5}}
	};

	for (const Case& testCase : cases) {
		Simulator simulator;
		Model* model = simulator.getModelManager()->newModel();
		ASSERT_NE(model, nullptr);
		DiffusionFieldProbe field(model, "SynchronizationMatrix");
		field.setDimensions(1u);
		field.setPointsPerDimension(3u);
		field.setStepsToSynchronize(testCase.stepsToSynchronize);
		field.setStepSize(testCase.stepSize);
		field.setStopTime(testCase.stopTime);
		field.InitBetweenReplicationsProbe();
		EXPECT_DOUBLE_EQ(field.getCurrentTime(), 0.0);

		for (double expectedTime : testCase.expectedTimes) {
			ASSERT_FALSE(model->getFutureEvents()->empty());
			Event* event = model->getFutureEvents()->front();
			model->getFutureEvents()->pop_front();
			EXPECT_NEAR(event->getTime(), expectedTime, 1e-12);
			static_cast<InternalEvent*>(event)->dispatchEvent();
			delete event;
			EXPECT_NEAR(field.getCurrentTime(), expectedTime, 1e-12);
		}
		EXPECT_TRUE(model->getFutureEvents()->empty());
		EXPECT_EQ(field.getLastStatus(), "Completed");
	}
}

TEST(DiffusionFieldPluginTest, SynchronizationUsesGlobalOriginCapturedAtStart) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	DiffusionFieldProbe field(model, "NonzeroGlobalOrigin");
	field.setDimensions(1u);
	field.setPointsPerDimension(3u);
	field.setStepSize(0.1);
	field.setStopTime(0.25);
	field.setStepsToSynchronize(0u);

	StartDiffusionAtCalendarTime starter{&field, model};
	model->getOnEventManager()->addOnReplicationStartHandler(
		&starter, &StartDiffusionAtCalendarTime::schedule);
	model->getSimulation()->setReplicationLength(101.0, Util::TimeUnit::second);
	model->getSimulation()->setShowReportsAfterReplication(false);
	model->getSimulation()->setShowReportsAfterSimulation(false);

	model->getSimulation()->start();

	EXPECT_TRUE(field.startSucceeded()) << field.startErrorMessage();
	EXPECT_EQ(field.getLastStatus(), "Completed");
	EXPECT_NEAR(field.getCurrentTime(), 0.25, 1e-12);
	EXPECT_NEAR(model->getSimulation()->getSimulatedTime(), 100.25, 1e-12);
	EXPECT_TRUE(model->getFutureEvents()->empty());
}

TEST(DiffusionFieldPluginTest, ZeroStopTimeCompletesWithoutSchedulingAndInvalidStepFails) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	DiffusionFieldProbe field(model, "ZeroStopTime");
	field.setStopTime(0.0);
	field.setStepsToSynchronize(1u);
	field.InitBetweenReplicationsProbe();
	EXPECT_TRUE(model->getFutureEvents()->empty());
	EXPECT_EQ(field.getLastStatus(), "Completed");

	field.setStepSize(0.0);
	std::string errorMessage;
	EXPECT_FALSE(field.simulate(errorMessage));
	EXPECT_NE(errorMessage.find("StepSize"), std::string::npos);
}

TEST(DiffusionFieldPluginTest, DestroyingFieldCancelsItsPendingSynchronizationEvent) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	auto* field = new DiffusionFieldProbe(model, "DestroyedWithPendingEvent");
	field->setDimensions(1u);
	field->setPointsPerDimension(3u);
	field->setStepsToSynchronize(1u);
	field->setStepSize(0.1);
	field->setStopTime(1.0);
	field->InitBetweenReplicationsProbe();
	ASSERT_EQ(model->getFutureEvents()->size(), 1u);

	delete field;
	EXPECT_TRUE(model->getFutureEvents()->empty());
}


TEST(DiffusionFieldPluginTest, ZeroStepsToSynchronizeRunsWithoutCalendarEvents) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    DiffusionFieldProbe field(model, "DiffusionNoSync");
    field.setDimensions(1u);
    field.setPointsPerDimension(11u);
    field.setStepsToSynchronize(0u);
    field.setStepSize(0.05);
    field.setStopTime(0.12);

    field.InitBetweenReplicationsProbe();
    EXPECT_TRUE(model->getFutureEvents()->empty());

    std::string errorMessage;
    ASSERT_TRUE(field.simulate(errorMessage)) << errorMessage;
    EXPECT_TRUE(model->getFutureEvents()->empty());
    EXPECT_NEAR(field.getCurrentTime(), 0.12, 1e-9);
    EXPECT_EQ(field.getLastStatus(), "Completed");
}
