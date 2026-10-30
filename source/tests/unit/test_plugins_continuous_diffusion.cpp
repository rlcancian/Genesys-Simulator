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
#include <stdexcept>
#include <vector>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/Persistence.h"
#include "kernel/simulator/Event.h"
#include "plugins/data/Continuous/DiffusionField.h"
#include "plugins/data/Continuous/ODESolver.h"
#include "plugins/data/Logic/Variable.h"
#include "plugins/data/BiochemicalSimulation/BioNetwork.h"
#include "plugins/data/BiochemicalSimulation/BioReaction.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "plugins/components/Continuous/DiffusionSimulate.h"
#include "plugins/components/Continuous/ContinuousSystemComponent.h"
#include "plugins/components/BiochemicalSimulation/BioSimulate.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/Logic/Create.h"

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
	void InitBetweenReplicationsProbe() {
		_initBetweenReplications();
		// The production path defers autonomous evolution to a zero-time kernel
		// event so replication-start handlers can populate the calendar first.
		if (! _parentModel->getFutureEvents()->empty()) {
			auto* event = dynamic_cast<InternalEvent*>(_parentModel->getFutureEvents()->front());
			if (event != nullptr && event->description() == "ContinuousModelAutomaticStart") {
				_parentModel->getFutureEvents()->pop_front();
				event->dispatchEvent();
				delete event;
			}
		}
	}
	void startAtNonzeroGlobalTime(void*) {
		setStepsToSynchronize(2u);
		_startSucceeded = simulate(_startErrorMessage);
	}
	void startObservableRun(void*) {
		setStepsToSynchronize(5u);
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

class CalendarBoundContinuousComponentProbe : public ContinuousModelComponent {
public:
	CalendarBoundContinuousComponentProbe(Model* model, DiffusionField* field)
		: ContinuousModelComponent(model, "CalendarBoundContinuousComponentProbe", "CalendarBoundContinuous"),
		  _field(field) {}
	std::vector<Entity*> dispatchedEntities;
	unsigned int continuationDispatches = 0;
	bool completed = false;
protected:
	void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override {
		dispatchedEntities.push_back(entity);
		if (inputPortNumber == CONTINUATION_INPUT_PORT) {
			++continuationDispatches;
			EXPECT_TRUE(_handleContinuousContinuation(entity, inputPortNumber));
			return;
		}
		ContinuousModelDataDefinition::ExecutionRequest request;
		std::string error;
		EXPECT_TRUE(_requestContinuousExecution(_field, request, entity, error)) << error;
	}
	void _onContinuousExecutionResult(const ContinuousModelDataDefinition::ExecutionResult& result) override {
		completed = result.succeeded();
	}
private:
	DiffusionField* _field;
};

class CalendarDiscreteObserverProbe : public ModelComponent {
public:
	CalendarDiscreteObserverProbe(Model* model, DiffusionField* field)
		: ModelComponent(model, "CalendarDiscreteObserverProbe", "CalendarDiscreteObserver"), _field(field) {}
	double observedConfirmedTime = -1.0;
	double observedGlobalTime = -1.0;
protected:
	void _onDispatchEvent(Entity* entity, unsigned int) override {
		observedConfirmedTime = _field->getCurrentTime();
		observedGlobalTime = _parentModel->getSimulation()->getSimulatedTime();
		_parentModel->removeEntity(entity);
	}
private:
	DiffusionField* _field;
};

class OdeCalendarDiscreteProbe : public ModelComponent {
public:
	OdeCalendarDiscreteProbe(Model* model, ODESolver* solver, Variable* parameter = nullptr,
	                         double parameterValue = 0.0)
		: ModelComponent(model, "OdeCalendarDiscreteProbe", "OdeCalendarDiscreteProbe"),
		  _solver(solver), _parameter(parameter), _parameterValue(parameterValue) {}
	double observedTime = -1.0;
	double observedGlobalTime = -1.0;
	double observedState = -1.0;
	double observedParameter = -1.0;
	double parsedParameter = -1.0;
protected:
	void _onDispatchEvent(Entity* entity, unsigned int) override {
		observedGlobalTime = _parentModel->getSimulation()->getSimulatedTime();
		observedTime = _solver->getCurrentTime();
		observedState = _solver->getStateValue(0u);
		if (_parameter != nullptr) {
			observedParameter = _parameter->getValue();
			parsedParameter = _parentModel->parseExpression(_parameter->getName());
		}
		if (_parameter != nullptr) _parameter->setValue(_parameterValue);
		else _solver->setEquationExpressions({"0"});
		_parentModel->removeEntity(entity);
	}
private:
	ODESolver* _solver;
	Variable* _parameter;
	double _parameterValue;
};

class BioCalendarDiscreteProbe : public ModelComponent {
public:
	BioCalendarDiscreteProbe(Model* model, BioNetwork* network, BioSpecies* species,
	                         BioReaction* reaction)
		: ModelComponent(model, "BioCalendarDiscreteProbe", "BioCalendarDiscreteProbe"),
		  _network(network), _species(species), _reaction(reaction) {}
	double observedTime = -1.0;
	double observedGlobalTime = -1.0;
	double observedAmount = -1.0;
protected:
	void _onDispatchEvent(Entity* entity, unsigned int) override {
		observedGlobalTime = _parentModel->getSimulation()->getSimulatedTime();
		observedTime = _network->getCurrentTime();
		observedAmount = _species->getAmount();
		_reaction->setRateConstant(0.2);
		_parentModel->removeEntity(entity);
	}
private:
	BioNetwork* _network;
	BioSpecies* _species;
	BioReaction* _reaction;
};

class ContinuousProcessArrivalProbe {
public:
	explicit ContinuousProcessArrivalProbe(std::string componentName)
		: _componentName(std::move(componentName)) {}
	std::vector<double> times;
	std::vector<Util::identification> entityIds;
	void onProcessEvent(SimulationEvent* event) {
		Event* current = event != nullptr ? event->getCurrentEvent() : nullptr;
		if (current == nullptr || current->getComponent() == nullptr ||
		    current->getComponent()->getName() != _componentName) return;
		times.push_back(event->getSimulatedTime());
		if (current->getEntity() != nullptr) entityIds.push_back(current->getEntity()->getId());
	}
private:
	std::string _componentName;
};

struct ScheduleContinuousComponentScenario {
	Model* model;
	ModelComponent* continuousComponent;
	ModelComponent* discreteObserver;
	double discreteTime = 5.5;
	double secondActivationTime = -1.0;
	Util::identification requestedEntityId = 0;
	Util::identification secondRequestedEntityId = 0;
	void schedule(SimulationEvent*) {
		Entity* request = model->createEntity("ContinuousRequest");
		requestedEntityId = request->getId();
		model->sendEntityToComponent(request, continuousComponent, 0.0);
		if (secondActivationTime >= 0.0) {
			Entity* secondRequest = model->createEntity("ContinuousRequestTwo");
			secondRequestedEntityId = secondRequest->getId();
			model->sendEntityToComponent(secondRequest, continuousComponent, secondActivationTime);
		}
		model->sendEntityToComponent(model->createEntity("DiscreteInterruption"), discreteObserver, discreteTime);
	}
};

struct ScheduleCalendarBoundComponentScenario {
	DiffusionField* field;
	CalendarBoundContinuousComponentProbe* component;
	CalendarDiscreteObserverProbe* observer;
	Model* model;
	double activationDelay = 0.0;
	double discreteDelay = 5.5;
	void schedule(SimulationEvent*) {
		field->setStepsToSynchronize(10u);
		model->sendEntityToComponent(model->createEntity("ContinuousRequester"), component, activationDelay);
		model->sendEntityToComponent(model->createEntity("DiscreteAt5_5"), observer, discreteDelay);
	}
};

class ObservableDefinitionProbe : public ModelDataDefinition {
public:
	ObservableDefinitionProbe(Model* model, std::string name)
		: ModelDataDefinition(model, "ObservableDefinitionProbe", std::move(name)) {}
	void onInternalState(const ContinuousModelDataDefinition::InternalStateEvent& event) {
		steps.push_back(event.step);
		localTimes.push_back(event.localTime);
		absoluteTimes.push_back(event.absoluteTime);
		if (const auto* view = event.stateAs<DiffusionField::StateView>())
			workingValues.push_back(view->values.at(1));
	}
	void onOdeState(const ContinuousModelDataDefinition::InternalStateEvent& event) {
		if (const auto* view = event.stateAs<ODESolver::StateView>()) odeWorkingValues.push_back(view->values.at(0));
	}
	void onBioState(const ContinuousModelDataDefinition::InternalStateEvent& event) {
		if (const auto* view = event.stateAs<BioNetwork::StateView>()) bioWorkingAmounts.push_back(view->amounts.at(0));
	}
	std::vector<std::uint64_t> steps;
	std::vector<double> localTimes;
	std::vector<double> absoluteTimes;
	std::vector<double> workingValues;
	std::vector<double> odeWorkingValues;
	std::vector<double> bioWorkingAmounts;
};

class SubscriptionMutationProbe : public ModelDataDefinition {
public:
	SubscriptionMutationProbe(Model* model, std::string name, ContinuousModelDataDefinition* source)
		: ModelDataDefinition(model, "SubscriptionMutationProbe", std::move(name)), _source(source) {}
	void onInternalState(const ContinuousModelDataDefinition::InternalStateEvent& event) {
		steps.push_back(event.step);
		if (cancelOnNotification) _source->cancelExecution("cancelled by observer");
		if (throwOnNotification) throw std::runtime_error("observer fixture failure");
		if (_removeOnFirst && !_mutated) {
			_mutated = true;
			_source->removeInternalStateEventHandler(removeId);
			_source->removeInternalStateEventHandler(selfId);
			lateId = _source->addInternalStateEventHandler(lateObserver, &SubscriptionMutationProbe::onInternalState);
		}
	}
	void configureMutation(ContinuousModelDataDefinition::InternalStateEventHandlerId self,
		ContinuousModelDataDefinition::InternalStateEventHandlerId remove,
		SubscriptionMutationProbe* late) {
		selfId = self;
		removeId = remove;
		lateObserver = late;
		_removeOnFirst = true;
	}
	ContinuousModelDataDefinition::InternalStateEventHandlerId selfId = 0;
	ContinuousModelDataDefinition::InternalStateEventHandlerId removeId = 0;
	ContinuousModelDataDefinition::InternalStateEventHandlerId lateId = 0;
	SubscriptionMutationProbe* lateObserver = nullptr;
	std::vector<std::uint64_t> steps;
	bool cancelOnNotification = false;
	bool throwOnNotification = false;
private:
	ContinuousModelDataDefinition* _source;
	bool _removeOnFirst = false;
	bool _mutated = false;
};

class ObservableComponentProbe : public ModelComponent {
public:
	ObservableComponentProbe(Model* model, DiffusionField* field)
		: ModelComponent(model, "ObservableComponentProbe", "ObservableComponentProbe"), _field(field) {}
	void onInternalState(const ContinuousModelDataDefinition::InternalStateEvent& event) {
		if (const auto* view = event.stateAs<DiffusionField::StateView>()) {
			observedLocalTimes.push_back(event.localTime);
			observedWorkingValues.push_back(view->values.at(1));
		}
		observedAbsoluteTimes.push_back(event.absoluteTime);
		observedGlobalTimes.push_back(_parentModel->getSimulation()->getSimulatedTime());
		if (std::abs(event.localTime - 3.0) < 1e-12) {
			Entity* entity = _parentModel->createEntity("ObserverGeneratedEvent");
			_parentModel->sendEntityToComponent(entity, this, event.absoluteTime -
				_parentModel->getSimulation()->getSimulatedTime() + generatedEventOffset);
		}
	}
	std::vector<double> observedLocalTimes;
	std::vector<double> observedWorkingValues;
	std::vector<double> observedAbsoluteTimes;
	std::vector<double> observedGlobalTimes;
	std::vector<double> arrivalTimes;
	std::vector<double> confirmedLocalTimes;
	std::vector<double> confirmedValues;
	double generatedEventOffset = 0.0;
protected:
	void _onDispatchEvent(Entity* entity, unsigned int) override {
		arrivalTimes.push_back(_parentModel->getSimulation()->getSimulatedTime());
		confirmedLocalTimes.push_back(_field->getCurrentTime());
		confirmedValues.push_back(_field->getFieldValue({1}));
		_parentModel->removeEntity(entity);
	}
private:
	DiffusionField* _field;
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

struct ScheduleObservableCalendarScenario {
	DiffusionFieldProbe* field;
	ObservableComponentProbe* component;
	Model* model;
	void schedule(SimulationEvent*) {
		auto* start = new InternalEvent(100.0, "StartObservableContinuousRun");
		start->setEventHandler(field, &DiffusionFieldProbe::startObservableRun, nullptr);
		model->getFutureEvents()->insert(start);
		Entity* entity = model->createEntity("DiscreteAt102");
		model->sendEntityToComponent(entity, component, 102.5);
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

TEST(DiffusionFieldPluginTest, DiffusionSimulateRejectsNonzeroLocalWindowStartExplicitly) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	DiffusionField field(model, "FieldForNonzeroWindow");
	DiffusionSimulateProbe component(model, "DiffusionNonzeroWindow");
	component.setDiffusionField(&field);
	component.setUseFieldTimeWindow(false);
	component.setStartTime(1.0);
	component.setStopTime(2.0);
	component.setStepSize(0.1);

	std::string errorMessage;
	EXPECT_FALSE(component.CheckProbe(errorMessage));
	EXPECT_NE(errorMessage.find("requires StartTime=0"), std::string::npos);
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

TEST(DiffusionFieldPluginTest, RejectedComponentWindowDoesNotMutateDefinitionConfiguration) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	DiffusionFieldProbe field(model, "RejectedWindowIsAtomic");
	field.setDimensions(0u); // Force the full subclass validation to reject the request.
	field.setStopTime(2.0);
	field.setStepSize(0.5);
	ContinuousModelDataDefinition::ExecutionRequest request;
	request.useDefinitionTimeWindow = false;
	request.stopTime = 5.0;
	request.stepSize = 0.25;
	std::string errorMessage;
	EXPECT_FALSE(field.requestSimulation(0u, request, errorMessage));
	EXPECT_DOUBLE_EQ(field.getStopTime(), 2.0);
	EXPECT_DOUBLE_EQ(field.getStepSize(), 0.5);
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

TEST(ContinuousObservableTest, ModelDataDefinitionObserverSeesEveryWorkingStepAndConfirmedGettersStayBehind) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	DiffusionFieldProbe field(model, "ObservableField");
	field.setDimensions(1u);
	field.setPointsPerDimension(3u);
	field.setStepSize(1.0);
	field.setStopTime(2.0);
	field.setStepsToSynchronize(2u);
	ObservableDefinitionProbe observer(model, "DefinitionObserver");
	auto id = field.addInternalStateEventHandler(&observer, &ObservableDefinitionProbe::onInternalState);
	ASSERT_NE(id, 0u);
	std::string errorMessage;
	ASSERT_TRUE(field.simulate(errorMessage)) << errorMessage;
	ASSERT_EQ(observer.steps, (std::vector<std::uint64_t>{1u, 2u}));
	EXPECT_EQ(observer.localTimes, (std::vector<double>{1.0, 2.0}));
	EXPECT_EQ(observer.absoluteTimes, (std::vector<double>{1.0, 2.0}));
	ASSERT_EQ(observer.workingValues.size(), 2u);
	EXPECT_LT(observer.workingValues.back(), observer.workingValues.front());
	EXPECT_DOUBLE_EQ(field.getCurrentTime(), 0.0);
	EXPECT_DOUBLE_EQ(field.getFieldValue({1}), 1.0);
	EXPECT_EQ(model->getFutureEvents()->size(), 1u);
	auto* sync = dynamic_cast<InternalEvent*>(model->getFutureEvents()->front());
	ASSERT_NE(sync, nullptr);
	model->getFutureEvents()->pop_front();
	sync->dispatchEvent();
	delete sync;
	EXPECT_DOUBLE_EQ(field.getCurrentTime(), 2.0);
	EXPECT_NEAR(field.getFieldValue({1}), observer.workingValues.back(), 1e-12);
	field.removeInternalStateEventHandler(id);
}

TEST(ContinuousObservableTest, PartialFinalStepIsNotifiedOnceAndSynchronizationRemainsOnCalendar) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	DiffusionFieldProbe field(model, "ObservablePartialField");
	field.setDimensions(1u);
	field.setPointsPerDimension(3u);
	field.setStepSize(1.0);
	field.setStopTime(2.5);
	field.setStepsToSynchronize(2u);
	ObservableDefinitionProbe observer(model, "PartialObserver");
	field.addInternalStateEventHandler(&observer, &ObservableDefinitionProbe::onInternalState);
	std::string errorMessage;
	ASSERT_TRUE(field.simulate(errorMessage)) << errorMessage;
	EXPECT_EQ(observer.localTimes, (std::vector<double>{1.0, 2.0}));
	ASSERT_EQ(model->getFutureEvents()->size(), 1u);
	auto* first = dynamic_cast<InternalEvent*>(model->getFutureEvents()->front());
	ASSERT_NE(first, nullptr);
	EXPECT_DOUBLE_EQ(first->getTime(), 2.0);
	model->getFutureEvents()->pop_front();
	first->dispatchEvent();
	delete first;
	EXPECT_EQ(observer.localTimes, (std::vector<double>{1.0, 2.0, 2.5}));
	ASSERT_EQ(model->getFutureEvents()->size(), 1u);
	auto* final = dynamic_cast<InternalEvent*>(model->getFutureEvents()->front());
	ASSERT_NE(final, nullptr);
	EXPECT_DOUBLE_EQ(final->getTime(), 2.5);
	model->getFutureEvents()->pop_front();
	final->dispatchEvent();
	delete final;
	EXPECT_EQ(observer.localTimes, (std::vector<double>{1.0, 2.0, 2.5}));
	EXPECT_DOUBLE_EQ(field.getCurrentTime(), 2.5);
}

TEST(ContinuousObservableTest, RemovalAndRegistrationDuringNotificationAreDeterministic) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	DiffusionFieldProbe field(model, "ObservableMutationField");
	field.setDimensions(1u);
	field.setPointsPerDimension(3u);
	field.setStopTime(2.0);
	field.setStepSize(1.0);
	field.setStepsToSynchronize(0u);
	SubscriptionMutationProbe first(model, "ObserverA", &field);
	SubscriptionMutationProbe removed(model, "ObserverB", &field);
	SubscriptionMutationProbe late(model, "ObserverC", &field);
	first.selfId = field.addInternalStateEventHandler(&first,
		&SubscriptionMutationProbe::onInternalState, -1);
	const auto removedId = field.addInternalStateEventHandler(&removed,
		&SubscriptionMutationProbe::onInternalState, 0);
	first.configureMutation(first.selfId, removedId, &late);
	std::string errorMessage;
	ASSERT_TRUE(field.simulate(errorMessage)) << errorMessage;
	EXPECT_EQ(first.steps, (std::vector<std::uint64_t>{1u}));
	EXPECT_TRUE(removed.steps.empty());
	EXPECT_EQ(late.steps, (std::vector<std::uint64_t>{2u}));
	EXPECT_TRUE(model->getFutureEvents()->empty());
	field.removeInternalStateEventHandler(first.lateId);
}

TEST(ContinuousObservableTest, PriorityAndDuplicateSubscriptionsHaveDeterministicOrder) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	DiffusionFieldProbe field(model, "ObservablePriorityField");
	field.setDimensions(1u);
	field.setPointsPerDimension(3u);
	field.setStopTime(1.0);
	field.setStepSize(1.0);
	field.setStepsToSynchronize(0u);
	ObservableDefinitionProbe first(model, "PriorityFirst");
	ObservableDefinitionProbe second(model, "PrioritySecond");
	std::vector<int> order;
	const auto firstId = field.addInternalStateEventHandler(&first,
		[&order](const auto&) { order.push_back(1); }, 10);
	const auto secondId = field.addInternalStateEventHandler(&second,
		[&order](const auto&) { order.push_back(2); }, -10);
	const auto duplicateId = field.addInternalStateEventHandler(&first,
		[&order](const auto&) { order.push_back(3); }, 10);
	ASSERT_NE(firstId, 0u);
	ASSERT_NE(secondId, 0u);
	ASSERT_NE(duplicateId, 0u);
	EXPECT_NE(firstId, duplicateId);
	std::string errorMessage;
	ASSERT_TRUE(field.simulate(errorMessage)) << errorMessage;
	EXPECT_EQ(order, (std::vector<int>{2, 1, 3}));
	field.removeInternalStateEventHandler(firstId);
	field.removeInternalStateEventHandler(secondId);
	field.removeInternalStateEventHandler(duplicateId);
}

TEST(ContinuousObservableTest, DestroyedObserverIsSkippedByLifetimeToken) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	DiffusionFieldProbe field(model, "ObservableLifetimeField");
	field.setDimensions(1u);
	field.setPointsPerDimension(3u);
	field.setStopTime(1.0);
	field.setStepSize(1.0);
	field.setStepsToSynchronize(0u);
	auto* observer = new ObservableDefinitionProbe(model, "DestroyedObserver");
	ASSERT_NE(field.addInternalStateEventHandler(observer, &ObservableDefinitionProbe::onInternalState), 0u);
	delete observer;
	std::string errorMessage;
	EXPECT_TRUE(field.simulate(errorMessage)) << errorMessage;
	EXPECT_EQ(field.getExecutionState(), ContinuousModelDataDefinition::ExecutionState::Completed);
}

TEST(ContinuousObservableTest, ObserverExceptionsFailTheRunAfterNotifyingRemainingObservers) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	DiffusionFieldProbe field(model, "ObservableExceptionField");
	field.setDimensions(1u);
	field.setPointsPerDimension(3u);
	field.setStopTime(1.0);
	field.setStepSize(1.0);
	field.setStepsToSynchronize(0u);
	SubscriptionMutationProbe throwing(model, "ThrowingObserver", &field);
	SubscriptionMutationProbe remaining(model, "RemainingObserver", &field);
	throwing.throwOnNotification = true;
	field.addInternalStateEventHandler(&throwing, &SubscriptionMutationProbe::onInternalState, -1);
	field.addInternalStateEventHandler(&remaining, &SubscriptionMutationProbe::onInternalState, 0);
	std::string errorMessage;
	EXPECT_FALSE(field.simulate(errorMessage));
	EXPECT_EQ(throwing.steps, (std::vector<std::uint64_t>{1u}));
	EXPECT_EQ(remaining.steps, (std::vector<std::uint64_t>{1u}));
	EXPECT_EQ(field.getExecutionState(), ContinuousModelDataDefinition::ExecutionState::Failed);
	EXPECT_NE(field.getLastErrorMessage().find("observer fixture failure"), std::string::npos);
}

TEST(ContinuousObservableTest, ObserverCanCancelRunAndPendingSynchronization) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	DiffusionFieldProbe field(model, "ObservableCancelField");
	field.setDimensions(1u);
	field.setPointsPerDimension(3u);
	field.setStopTime(3.0);
	field.setStepSize(1.0);
	field.setStepsToSynchronize(2u);
	SubscriptionMutationProbe observer(model, "CancellingObserver", &field);
	observer.cancelOnNotification = true;
	field.addInternalStateEventHandler(&observer, &SubscriptionMutationProbe::onInternalState);
	std::string errorMessage;
	EXPECT_TRUE(field.simulate(errorMessage)) << errorMessage;
	EXPECT_EQ(observer.steps, (std::vector<std::uint64_t>{1u}));
	EXPECT_EQ(field.getExecutionState(), ContinuousModelDataDefinition::ExecutionState::Cancelled);
	EXPECT_TRUE(model->getFutureEvents()->empty());
}

TEST(ContinuousObservableTest, ODESolverPublishesOnlyConfirmedVariablesWhileObserverSeesWorkVector) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	Variable state(model, "ObservableOdeState");
	state.setValue(1.0);
	ODESolver solver(model, "ObservableOde");
	solver.setStateVariableNames({"ObservableOdeState"});
	solver.setEquationExpressions({"-ObservableOdeState"});
	solver.setInitialStateValues({1.0});
	solver.setStepSize(1.0);
	solver.setStopTime(2.0);
	solver.setStepsToSynchronize(2u);
	ObservableDefinitionProbe observer(model, "OdeDefinitionObserver");
	solver.addInternalStateEventHandler(&observer, &ObservableDefinitionProbe::onOdeState);
	std::string errorMessage;
	ASSERT_TRUE(solver.simulate(errorMessage)) << errorMessage;
	ASSERT_EQ(observer.odeWorkingValues.size(), 2u);
	EXPECT_LT(observer.odeWorkingValues.back(), observer.odeWorkingValues.front());
	EXPECT_DOUBLE_EQ(solver.getStateValue(0u), 1.0);
	EXPECT_DOUBLE_EQ(state.getValue(), 1.0);
	auto* sync = dynamic_cast<InternalEvent*>(model->getFutureEvents()->front());
	ASSERT_NE(sync, nullptr);
	model->getFutureEvents()->pop_front();
	sync->dispatchEvent();
	delete sync;
	EXPECT_NEAR(solver.getStateValue(0u), 0.140625, 1e-12);
	EXPECT_NEAR(state.getValue(), solver.getStateValue(0u), 1e-12);
}

TEST(ContinuousObservableTest, BioNetworkKeepsSpeciesAndPublicResultAtLastConfirmedBoundary) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	BioSpecies substrate(model, "ObservableSubstrate");
	substrate.setInitialAmount(10.0);
	substrate.setAmount(10.0);
	BioSpecies product(model, "ObservableProduct");
	product.setInitialAmount(0.0);
	product.setAmount(0.0);
	BioReaction reaction(model, "ObservableReaction");
	reaction.addReactant("ObservableSubstrate", 1.0);
	reaction.addProduct("ObservableProduct", 1.0);
	reaction.setRateConstant(0.2);
	BioNetwork network(model, "ObservableNetwork");
	network.addSpecies("ObservableSubstrate");
	network.addSpecies("ObservableProduct");
	network.addReaction("ObservableReaction");
	network.setStepSize(1.0);
	network.setStopTime(2.0);
	network.setStepsToSynchronize(2u);
	ObservableDefinitionProbe observer(model, "BioDefinitionObserver");
	network.addInternalStateEventHandler(&observer, &ObservableDefinitionProbe::onBioState);
	std::string errorMessage;
	ASSERT_TRUE(network.simulate(errorMessage)) << errorMessage;
	ASSERT_EQ(observer.bioWorkingAmounts.size(), 2u);
	EXPECT_LT(observer.bioWorkingAmounts.back(), observer.bioWorkingAmounts.front());
	EXPECT_DOUBLE_EQ(substrate.getAmount(), 10.0);
	EXPECT_DOUBLE_EQ(network.getCurrentTime(), 0.0);
	EXPECT_EQ(network.getLastSimulationResult().sampleCount(), 1u);
	auto* sync = dynamic_cast<InternalEvent*>(model->getFutureEvents()->front());
	ASSERT_NE(sync, nullptr);
	model->getFutureEvents()->pop_front();
	sync->dispatchEvent();
	delete sync;
	EXPECT_NEAR(substrate.getAmount(), observer.bioWorkingAmounts.back(), 1e-12);
	EXPECT_DOUBLE_EQ(network.getCurrentTime(), 2.0);
	EXPECT_EQ(network.getLastSimulationResult().sampleCount(), 3u);
}

TEST(ContinuousObservableTest, ComponentObserverSchedulesIntermediateCalendarEventWithoutPublishingWorkState) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	DiffusionFieldProbe field(model, "ObservableCalendarField");
	field.setDimensions(1u);
	field.setPointsPerDimension(3u);
	field.setStepSize(1.0);
	field.setStopTime(10.0);
	field.setStepsToSynchronize(0u); // Explicit request at global time 100 starts the run.
	ObservableComponentProbe component(model, &field);
	model->getComponentManager()->remove(&component); // Test callback target, not a graph node in this fixture.
	auto id = field.addInternalStateEventHandler(&component, &ObservableComponentProbe::onInternalState);
	ASSERT_NE(id, 0u);
	component.generatedEventOffset = 0.5;
	ScheduleObservableCalendarScenario schedule{&field, &component, model};
	model->getOnEventManager()->addOnReplicationStartHandler(
		&schedule, &ScheduleObservableCalendarScenario::schedule);
	model->getSimulation()->setNumberOfReplications(1u);
	model->getSimulation()->setReplicationLength(110.0, Util::TimeUnit::second);
	model->getSimulation()->setShowReportsAfterReplication(false);
	model->getSimulation()->setShowReportsAfterSimulation(false);
	ASSERT_TRUE(model->check());
	model->getSimulation()->start();
	ASSERT_TRUE(field.startSucceeded()) << field.startErrorMessage();
	ASSERT_EQ(component.arrivalTimes.size(), 2u);
	EXPECT_DOUBLE_EQ(component.arrivalTimes[0], 102.5);
	EXPECT_DOUBLE_EQ(component.arrivalTimes[1], 103.5);
	EXPECT_DOUBLE_EQ(component.confirmedLocalTimes[0], 2.0);
	EXPECT_DOUBLE_EQ(component.confirmedLocalTimes[1], 2.0);
	EXPECT_LT(component.confirmedValues[0], 1.0);
	ASSERT_EQ(component.observedLocalTimes.size(), 10u);
	EXPECT_DOUBLE_EQ(component.observedLocalTimes[2], 3.0);
	EXPECT_DOUBLE_EQ(component.observedAbsoluteTimes[2], 103.0);
	EXPECT_GE(component.observedGlobalTimes[2], 103.0);
	EXPECT_GE(component.observedGlobalTimes[5], 103.0);
	EXPECT_LT(component.observedWorkingValues[2], 1.0);
	EXPECT_NEAR(field.getCurrentTime(), 10.0, 1e-12);
	field.removeInternalStateEventHandler(id);
}

TEST(ContinuousCalendarTest, ComponentEntityReturnsAtNextGridPointAfterDiscreteEvent) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	DiffusionFieldProbe field(model, "CalendarBoundField");
	field.setDimensions(1u);
	field.setPointsPerDimension(3u);
	field.setStepSize(1.0);
	field.setStopTime(10.0);
	field.setStepsToSynchronize(10u); // Auto-start is deferred until replication-start events exist.
	CalendarBoundContinuousComponentProbe component(model, &field);
	CalendarDiscreteObserverProbe observer(model, &field);
	model->getComponentManager()->remove(&component);
	model->getComponentManager()->remove(&observer);
	ScheduleCalendarBoundComponentScenario setup{&field, &component, &observer, model};
	model->getOnEventManager()->addOnReplicationStartHandler(
		&setup, &ScheduleCalendarBoundComponentScenario::schedule);
	model->getSimulation()->setNumberOfReplications(1u);
	model->getSimulation()->setReplicationLength(20.0, Util::TimeUnit::second);
	model->getSimulation()->setShowReportsAfterReplication(false);
	model->getSimulation()->setShowReportsAfterSimulation(false);
	ASSERT_TRUE(model->check());
	model->getSimulation()->start();
	ASSERT_TRUE(component.completed);
	EXPECT_EQ(component.dispatchedEntities.size(), 2u);
	EXPECT_EQ(component.dispatchedEntities[0], component.dispatchedEntities[1]);
	EXPECT_EQ(component.continuationDispatches, 1u);
	EXPECT_DOUBLE_EQ(observer.observedConfirmedTime, 5.0);
	EXPECT_DOUBLE_EQ(field.getCurrentTime(), 10.0);
}

TEST(ContinuousCalendarTest, DiffusionSimulateRetainsEntityAcrossCalendarInterruption) {
	Simulator simulator;
	simulator.getPluginManager()->autoInsertPlugins();
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	DiffusionField field(model, "DiffusionComponentCalendarField");
	field.setDimensions(1u);
	field.setPointsPerDimension(3u);
	field.setDomainLength(1.0);
	field.setDiffusionCoefficient(0.1);
	field.setInitialCondition("SineMode");
	field.setInitialParameter(1.0);
	field.setStepSize(1.0);
	field.setStopTime(10.0);
	field.setStepsToSynchronize(10u);

	auto* component = new DiffusionSimulate(model, "DiffusionCalendarComponent");
	ASSERT_NE(component, nullptr);
	component->setDiffusionField(&field);
	component->setUseFieldTimeWindow(true);
	// DiffusionSimulate is exercised directly here because this test target does
	// not register its dynamic plugin type in the global plugin manager.
	model->getComponentManager()->remove(component);
	auto* source = new Create(model, "DiffusionCalendarSource");
	auto* sink = new Dispose(model, "DiffusionCalendarSink");
	ASSERT_NE(source, nullptr);
	ASSERT_NE(sink, nullptr);
	source->setTimeBetweenCreationsExpression("1");
	source->setMaxCreations(0u);
	source->setEntitiesPerCreation(0u);
	sink->setReportStatistics(false);
	// The test target links the component implementation statically but does
	// not register its runtime plugin descriptor. Keep the checked model graph
	// valid while both scheduled entities are injected directly into this
	// instance.
	source->connectTo(sink);
	component->connectTo(sink);

	CalendarDiscreteObserverProbe observer(model, &field);
	model->getComponentManager()->remove(&observer);
	ScheduleContinuousComponentScenario setup{model, component, &observer, 5.5, 2.5};
	model->getOnEventManager()->addOnReplicationStartHandler(
		&setup, &ScheduleContinuousComponentScenario::schedule);
	ContinuousProcessArrivalProbe arrivals("DiffusionCalendarSink");
	model->getOnEventManager()->addOnProcessEventHandler(
		&arrivals, &ContinuousProcessArrivalProbe::onProcessEvent);
	model->getSimulation()->setNumberOfReplications(1u);
	model->getSimulation()->setReplicationLength(20.0, Util::TimeUnit::second);
	model->getSimulation()->setShowReportsAfterReplication(false);
	model->getSimulation()->setShowReportsAfterSimulation(false);
	ASSERT_TRUE(model->check()) << model->getTracer()->errorMessages()->list()->front();
	model->getSimulation()->start();

	EXPECT_DOUBLE_EQ(observer.observedGlobalTime, 5.5);
	EXPECT_DOUBLE_EQ(observer.observedConfirmedTime, 5.0);
	EXPECT_EQ(field.getExecutionState(), ContinuousModelDataDefinition::ExecutionState::Completed)
		<< field.getLastErrorMessage();
	EXPECT_DOUBLE_EQ(field.getCurrentTime(), 10.0);
	EXPECT_TRUE(component->getLastSucceeded()) << component->getLastMessage();
	ASSERT_EQ(arrivals.times.size(), 2u);
	EXPECT_DOUBLE_EQ(arrivals.times[0], 10.0);
	EXPECT_DOUBLE_EQ(arrivals.times[1], 10.0);
	ASSERT_EQ(arrivals.entityIds.size(), 2u);
	EXPECT_NE(arrivals.entityIds[0], arrivals.entityIds[1]);
	EXPECT_EQ(arrivals.entityIds[0], setup.requestedEntityId);
	EXPECT_EQ(arrivals.entityIds[1], setup.secondRequestedEntityId);
}

TEST(ContinuousCalendarTest, NonzeroOriginPreservesLocalGridAcrossCalendarContinuation) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	DiffusionFieldProbe field(model, "CalendarBoundFieldAt100");
	field.setDimensions(1u);
	field.setPointsPerDimension(3u);
	field.setStepSize(1.0);
	field.setStopTime(10.0);
	field.setStepsToSynchronize(0u); // Prevent automatic start before calendar setup.
	CalendarBoundContinuousComponentProbe component(model, &field);
	CalendarDiscreteObserverProbe observer(model, &field);
	model->getComponentManager()->remove(&component);
	model->getComponentManager()->remove(&observer);
	ScheduleCalendarBoundComponentScenario setup{&field, &component, &observer, model, 100.0, 105.5};
	model->getOnEventManager()->addOnReplicationStartHandler(
		&setup, &ScheduleCalendarBoundComponentScenario::schedule);
	model->getSimulation()->setNumberOfReplications(1u);
	model->getSimulation()->setReplicationLength(120.0, Util::TimeUnit::second);
	model->getSimulation()->setShowReportsAfterReplication(false);
	model->getSimulation()->setShowReportsAfterSimulation(false);
	ASSERT_TRUE(model->check());
	model->getSimulation()->start();
	ASSERT_TRUE(component.completed);
	EXPECT_EQ(component.dispatchedEntities.size(), 2u);
	EXPECT_EQ(component.dispatchedEntities[0], component.dispatchedEntities[1]);
	EXPECT_EQ(component.continuationDispatches, 1u);
	EXPECT_DOUBLE_EQ(observer.observedConfirmedTime, 5.0);
	EXPECT_DOUBLE_EQ(field.getCurrentTime(), 10.0);
	EXPECT_DOUBLE_EQ(model->getSimulation()->getSimulatedTime(), 110.0);
}

TEST(ContinuousCalendarTest, InitialLookaheadUsesLastGridPointBeforeCalendarEvent) {
	struct Case { double discreteTime; double expectedSyncTime; std::uint64_t expectedSteps; };
	const std::vector<Case> cases = {
		{0.5, 1.0, 0u},
		{5.5, 5.0, 5u},
		{9.9, 9.0, 9u},
		{10.0, 10.0, 10u}
	};
	for (const Case& testCase : cases) {
		Simulator simulator;
		Model* model = simulator.getModelManager()->newModel();
		ASSERT_NE(model, nullptr);
		DiffusionFieldProbe field(model, "CalendarGridBoundary");
		field.setDimensions(1u);
		field.setPointsPerDimension(3u);
		field.setStepSize(1.0);
		field.setStopTime(10.0);
		field.setStepsToSynchronize(10u);
		ObservableComponentProbe discrete(model, &field);
		model->getComponentManager()->remove(&discrete);
		ObservableDefinitionProbe observer(model, "CalendarGridObserver");
		field.addInternalStateEventHandler(&observer, &ObservableDefinitionProbe::onInternalState);
		Entity* entity = model->createEntity("CalendarBoundaryEvent");
		model->sendEntityToComponent(entity, &discrete, testCase.discreteTime);
		std::string errorMessage;
		ASSERT_TRUE(field.simulate(errorMessage)) << errorMessage;
		EXPECT_EQ(observer.steps.size(), testCase.expectedSteps);
		const auto* events = model->getFutureEvents()->list();
		const InternalEvent* sync = nullptr;
		for (const Event* event : *events) {
			auto* candidate = dynamic_cast<const InternalEvent*>(event);
			if (candidate != nullptr && candidate->object() == &field) sync = candidate;
		}
		ASSERT_NE(sync, nullptr);
		EXPECT_DOUBLE_EQ(sync->getTime(), testCase.expectedSyncTime);
	}
}

TEST(ContinuousCalendarTest, ODEComponentWaitsAndResumesAfterDiscreteInterruption) {
	Simulator simulator;
	simulator.getPluginManager()->autoInsertPlugins();
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	Variable state(model, "ObservableOdeState");
	state.setValue(0.0);
	ODESolver solver(model, "OdeCalendarSolver");
	solver.setStateVariableNames({"ObservableOdeState"});
	solver.setEquationExpressions({"1"});
	solver.setInitialStateValues({0.0});
	solver.setStepSize(1.0);
	solver.setStopTime(10.0);
	solver.setStepsToSynchronize(10u);

	auto* component = new ContinuousSystemComponent(model, "OdeCalendarComponent");
	component->setOdeSolver(&solver);
	auto* sink = new Dispose(model, "OdeCalendarSink");
	sink->setReportStatistics(false);
	auto* source = new Create(model, "OdeCalendarSource");
	ASSERT_NE(source, nullptr);
	source->setTimeBetweenCreationsExpression("1");
	source->setMaxCreations(0u);
	source->setEntitiesPerCreation(0u);
	ASSERT_NE(sink, nullptr);
	source->connectTo(component);
	component->connectTo(sink);
	OdeCalendarDiscreteProbe observer(model, &solver);
	model->getComponentManager()->remove(&observer);
	ScheduleContinuousComponentScenario setup{model, component, &observer, 5.5};
	model->getOnEventManager()->addOnReplicationStartHandler(
		&setup, &ScheduleContinuousComponentScenario::schedule);
	ContinuousProcessArrivalProbe arrivals("OdeCalendarSink");
	model->getOnEventManager()->addOnProcessEventHandler(
		&arrivals, &ContinuousProcessArrivalProbe::onProcessEvent);
	model->getSimulation()->setNumberOfReplications(1u);
	model->getSimulation()->setReplicationLength(20.0, Util::TimeUnit::second);
	model->getSimulation()->setShowReportsAfterReplication(false);
	model->getSimulation()->setShowReportsAfterSimulation(false);
	ASSERT_TRUE(model->check()) << model->getTracer()->errorMessages()->list()->front();
	model->getSimulation()->start();

	EXPECT_DOUBLE_EQ(observer.observedTime, 5.0);
	EXPECT_DOUBLE_EQ(observer.observedGlobalTime, 5.5);
	EXPECT_NEAR(observer.observedState, 5.0, 1e-12);
	ASSERT_EQ(arrivals.times.size(), 1u);
	EXPECT_DOUBLE_EQ(arrivals.times.front(), 10.0);
	EXPECT_NEAR(solver.getStateValue(0u), 5.0, 1e-12);
	EXPECT_EQ(solver.getExecutionState(), ContinuousModelDataDefinition::ExecutionState::Completed);
}

TEST(ContinuousCalendarTest, ODEReadsDiscreteParameterChangeAfterCalendarResumption) {
	Simulator simulator;
	simulator.getPluginManager()->autoInsertPlugins();
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	Variable state(model, "OdeParameterState");
	state.setValue(0.0);
	Variable rate(model, "OdeDiscreteRate");
	rate.setInitialValuesText("1");
	ODESolver solver(model, "OdeParameterCalendarSolver");
	solver.setStateVariableNames({"OdeParameterState"});
	solver.setEquationExpressions({"OdeDiscreteRate"});
	solver.setInitialStateValues({0.0});
	solver.setStepSize(1.0);
	solver.setStopTime(10.0);
	solver.setStepsToSynchronize(10u);

	auto* component = new ContinuousSystemComponent(model, "OdeParameterCalendarComponent");
	component->setOdeSolver(&solver);
	auto* sink = new Dispose(model, "OdeParameterCalendarSink");
	sink->setReportStatistics(false);
	auto* source = new Create(model, "OdeParameterCalendarSource");
	source->setTimeBetweenCreationsExpression("1");
	source->setMaxCreations(0u);
	source->setEntitiesPerCreation(0u);
	source->connectTo(component);
	component->connectTo(sink);

	OdeCalendarDiscreteProbe observer(model, &solver, &rate, 2.0);
	model->getComponentManager()->remove(&observer);
	ScheduleContinuousComponentScenario setup{model, component, &observer, 5.5};
	model->getOnEventManager()->addOnReplicationStartHandler(
		&setup, &ScheduleContinuousComponentScenario::schedule);
	ContinuousProcessArrivalProbe arrivals("OdeParameterCalendarSink");
	model->getOnEventManager()->addOnProcessEventHandler(
		&arrivals, &ContinuousProcessArrivalProbe::onProcessEvent);
	model->getSimulation()->setNumberOfReplications(1u);
	model->getSimulation()->setReplicationLength(20.0, Util::TimeUnit::second);
	model->getSimulation()->setShowReportsAfterReplication(false);
	model->getSimulation()->setShowReportsAfterSimulation(false);
	EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), "OdeDiscreteRate"), &rate);
	ASSERT_TRUE(model->check()) << model->getTracer()->errorMessages()->list()->front();
	EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), "OdeDiscreteRate"), &rate);
	EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<Attribute>(), "OdeDiscreteRate"), nullptr);
	EXPECT_DOUBLE_EQ(model->parseExpression("OdeDiscreteRate"), 1.0);
	model->getSimulation()->start();

	EXPECT_DOUBLE_EQ(observer.observedTime, 5.0);
	EXPECT_DOUBLE_EQ(observer.observedGlobalTime, 5.5);
	EXPECT_DOUBLE_EQ(observer.observedParameter, 1.0);
	EXPECT_DOUBLE_EQ(observer.parsedParameter, 1.0);
	EXPECT_NEAR(observer.observedState, 5.0, 1e-12);
	ASSERT_EQ(arrivals.times.size(), 1u);
	EXPECT_DOUBLE_EQ(arrivals.times.front(), 10.0);
	EXPECT_NEAR(solver.getStateValue(0u), 15.0, 1e-12);
	EXPECT_EQ(solver.getExecutionState(), ContinuousModelDataDefinition::ExecutionState::Completed);
}

TEST(ContinuousCalendarTest, AutonomousOdeResetsMaxStepsForEachSynchronizationBatch) {
	Simulator simulator;
	simulator.getPluginManager()->autoInsertPlugins();
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	Variable state(model, "OdeBatchBudgetState");
	state.setValue(0.0);
	ODESolver solver(model, "OdeBatchBudgetSolver");
	solver.setStateVariableNames({"OdeBatchBudgetState"});
	solver.setEquationExpressions({"1"});
	solver.setInitialStateValues({0.0});
	solver.setMaxSteps(2);
	solver.setStepSize(0.1);
	solver.setStopTime(1.0);
	solver.setStepsToSynchronize(1u);
	auto* component = new ContinuousSystemComponent(model, "OdeBatchBudgetComponent");
	component->setOdeSolver(&solver);
	auto* sink = new Dispose(model, "OdeBatchBudgetSink");
	sink->setReportStatistics(false);
	auto* source = new Create(model, "OdeBatchBudgetSource");
	source->setEntitiesPerCreation(1u);
	source->setMaxCreations(1ul);
	source->connectTo(component);
	component->connectTo(sink);
	model->getSimulation()->setNumberOfReplications(1u);
	model->getSimulation()->setReplicationLength(2.0, Util::TimeUnit::second);
	model->getSimulation()->setShowReportsAfterReplication(false);
	model->getSimulation()->setShowReportsAfterSimulation(false);
	ASSERT_TRUE(model->check()) << model->getTracer()->errorMessages()->list()->front();

	model->getSimulation()->start();

	EXPECT_EQ(solver.getExecutionState(), ContinuousModelDataDefinition::ExecutionState::Completed)
		<< solver.getLastErrorMessage();
	EXPECT_NEAR(solver.getStateValue(0u), 1.0, 1e-10);
	EXPECT_NEAR(solver.getCurrentTime(), 1.0, 1e-12);
}

TEST(ContinuousCalendarTest, IndependentAutonomousOdesShareCalendarWithoutLosingTheirRuns) {
	Simulator simulator;
	simulator.getPluginManager()->autoInsertPlugins();
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	Variable firstState(model, "IndependentOdeStateOne");
	firstState.setValue(0.0);
	Variable secondState(model, "IndependentOdeStateTwo");
	secondState.setValue(0.0);
	ODESolver first(model, "IndependentOdeOne");
	first.setStateVariableNames({"IndependentOdeStateOne"});
	first.setEquationExpressions({"1"});
	first.setInitialStateValues({0.0});
	first.setStepSize(1.0);
	first.setStopTime(4.0);
	first.setStepsToSynchronize(4u);
	ODESolver second(model, "IndependentOdeTwo");
	second.setStateVariableNames({"IndependentOdeStateTwo"});
	second.setEquationExpressions({"2"});
	second.setInitialStateValues({0.0});
	second.setStepSize(1.0);
	second.setStopTime(6.0);
	second.setStepsToSynchronize(2u);
	auto* firstComponent = new ContinuousSystemComponent(model, "IndependentOdeComponentOne");
	firstComponent->setOdeSolver(&first);
	auto* secondComponent = new ContinuousSystemComponent(model, "IndependentOdeComponentTwo");
	secondComponent->setOdeSolver(&second);
	auto* firstSource = new Create(model, "IndependentOdeSourceOne");
	firstSource->setTimeBetweenCreationsExpression("1");
	firstSource->setMaxCreations(1u);
	firstSource->setEntitiesPerCreation(1u);
	auto* secondSource = new Create(model, "IndependentOdeSourceTwo");
	secondSource->setTimeBetweenCreationsExpression("1");
	secondSource->setMaxCreations(1u);
	secondSource->setEntitiesPerCreation(1u);
	auto* firstSink = new Dispose(model, "IndependentOdeSinkOne");
	firstSink->setReportStatistics(false);
	auto* secondSink = new Dispose(model, "IndependentOdeSinkTwo");
	secondSink->setReportStatistics(false);
	firstSource->connectTo(firstComponent);
	firstComponent->connectTo(firstSink);
	secondSource->connectTo(secondComponent);
	secondComponent->connectTo(secondSink);

	model->getSimulation()->setNumberOfReplications(1u);
	model->getSimulation()->setReplicationLength(10.0, Util::TimeUnit::second);
	model->getSimulation()->setShowReportsAfterReplication(false);
	model->getSimulation()->setShowReportsAfterSimulation(false);
	ASSERT_TRUE(model->check()) << model->getTracer()->errorMessages()->list()->front();
	ASSERT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<ODESolver>(), "IndependentOdeOne"), &first);
	ASSERT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<ODESolver>(), "IndependentOdeTwo"), &second);

	model->getSimulation()->start();

	EXPECT_EQ(first.getExecutionState(), ContinuousModelDataDefinition::ExecutionState::Completed)
		<< first.getLastErrorMessage();
	EXPECT_EQ(second.getExecutionState(), ContinuousModelDataDefinition::ExecutionState::Completed)
		<< second.getLastErrorMessage();
	EXPECT_NEAR(first.getStateValue(0u), 4.0, 1e-12);
	EXPECT_NEAR(second.getStateValue(0u), 12.0, 1e-12);
	EXPECT_DOUBLE_EQ(first.getCurrentTime(), 4.0);
	EXPECT_DOUBLE_EQ(second.getCurrentTime(), 6.0);
	EXPECT_DOUBLE_EQ(model->getSimulation()->getSimulatedTime(), 6.0);
}

TEST(ContinuousCalendarTest, OdeDiffusionAndBioNetworkShareOneRealEventCalendar) {
	Simulator simulator;
	simulator.getPluginManager()->autoInsertPlugins();
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	Variable odeState(model, "ThreeDomainOdeState");
	odeState.setValue(0.0);
	ODESolver ode(model, "ThreeDomainOde");
	ode.setStateVariableNames({"ThreeDomainOdeState"});
	ode.setEquationExpressions({"1"});
	ode.setInitialStateValues({0.0});
	ode.setStepSize(1.0);
	ode.setStopTime(4.0);
	ode.setStepsToSynchronize(2u);

	DiffusionField field(model, "ThreeDomainDiffusion");
	field.setDimensions(1u);
	field.setPointsPerDimension(3u);
	field.setDomainLength(1.0);
	field.setDiffusionCoefficient(0.1);
	field.setInitialCondition("SineMode");
	field.setInitialParameter(1.0);
	field.setStepSize(1.0);
	field.setStopTime(4.0);
	field.setStepsToSynchronize(3u);

	BioSpecies substrate(model, "ThreeDomainSubstrate");
	substrate.setInitialAmount(10.0);
	substrate.setAmount(10.0);
	BioSpecies product(model, "ThreeDomainProduct");
	product.setInitialAmount(0.0);
	product.setAmount(0.0);
	BioReaction reaction(model, "ThreeDomainReaction");
	reaction.addReactant("ThreeDomainSubstrate", 1.0);
	reaction.addProduct("ThreeDomainProduct", 1.0);
	reaction.setRateConstant(0.1);
	BioNetwork network(model, "ThreeDomainBioNetwork");
	network.addSpecies("ThreeDomainSubstrate");
	network.addSpecies("ThreeDomainProduct");
	network.addReaction("ThreeDomainReaction");
	network.setStepSize(1.0);
	network.setStopTime(4.0);
	network.setStepsToSynchronize(1u);
	auto* bioComponent = new BioSimulate(model, "ThreeDomainBioComponent");
	ASSERT_NE(bioComponent, nullptr);
	bioComponent->setBioNetwork(&network);
	auto* entitySource = new Create(model, "ThreeDomainEntitySource");
	auto* entitySink = new Dispose(model, "ThreeDomainEntitySink");
	ASSERT_NE(entitySource, nullptr);
	ASSERT_NE(entitySink, nullptr);
	entitySource->setMaxCreations(0u);
	entitySource->connectTo(bioComponent);
	bioComponent->connectTo(entitySink);

	struct StartDefinitions {
		ODESolver* ode;
		DiffusionField* field;
		BioNetwork* network;
		bool started = false;
		std::string error;
		void start(SimulationEvent*) {
			started = ode->simulate(error);
			if (!started) return;
			started = field->simulate(error);
			if (!started) return;
			started = network->simulate(error);
		}
	} startDefinitions{&ode, &field, &network};
	model->getOnEventManager()->addOnReplicationStartHandler(
		&startDefinitions, &StartDefinitions::start);
	model->getSimulation()->setNumberOfReplications(1u);
	model->getSimulation()->setReplicationLength(6.0, Util::TimeUnit::second);
	model->getSimulation()->setShowReportsAfterReplication(false);
	model->getSimulation()->setShowReportsAfterSimulation(false);
	ASSERT_TRUE(model->check()) << model->getTracer()->errorMessages()->list()->front();
	EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<BioSpecies>(), "ThreeDomainSubstrate"), &substrate);
	EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<BioSpecies>(), "ThreeDomainProduct"), &product);
	EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<BioReaction>(), "ThreeDomainReaction"), &reaction);
	model->getSimulation()->start();

	ASSERT_TRUE(startDefinitions.started) << startDefinitions.error;
	EXPECT_EQ(ode.getExecutionState(), ContinuousModelDataDefinition::ExecutionState::Completed)
		<< ode.getLastErrorMessage();
	EXPECT_EQ(field.getExecutionState(), ContinuousModelDataDefinition::ExecutionState::Completed)
		<< field.getLastErrorMessage();
	EXPECT_EQ(network.getExecutionState(), ContinuousModelDataDefinition::ExecutionState::Completed)
		<< network.getLastErrorMessage();
	EXPECT_NEAR(ode.getStateValue(0u), 4.0, 1e-12);
	EXPECT_DOUBLE_EQ(field.getCurrentTime(), 4.0);
	EXPECT_NEAR(substrate.getAmount() + product.getAmount(), 10.0, 1e-12);
	EXPECT_DOUBLE_EQ(model->getSimulation()->getSimulatedTime(), 4.0);
}

TEST(ContinuousCalendarTest, BioSimulateResumesSharedRunForBothWaitingEntities) {
	Simulator simulator;
	simulator.getPluginManager()->autoInsertPlugins();
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	BioSpecies substrate(model, "BioCalendarSubstrate");
	substrate.setInitialAmount(10.0);
	substrate.setAmount(10.0);
	BioSpecies product(model, "BioCalendarProduct");
	product.setInitialAmount(0.0);
	product.setAmount(0.0);
	BioReaction reaction(model, "BioCalendarReaction");
	reaction.addReactant("BioCalendarSubstrate", 1.0);
	reaction.addProduct("BioCalendarProduct", 1.0);
	reaction.setRateConstant(0.1);
	BioNetwork network(model, "BioCalendarNetwork");
	network.addSpecies("BioCalendarSubstrate");
	network.addSpecies("BioCalendarProduct");
	network.addReaction("BioCalendarReaction");
	network.setStopTime(10.0);
	network.setStepSize(1.0);
	network.setStepsToSynchronize(10u);

	auto* component = new BioSimulate(model, "BioCalendarSimulate");
	component->setBioNetwork(&network);
	auto* sink = new Dispose(model, "BioCalendarSink");
	sink->setReportStatistics(false);
	auto* source = new Create(model, "BioCalendarSource");
	ASSERT_NE(source, nullptr);
	source->setTimeBetweenCreationsExpression("1");
	source->setMaxCreations(0u);
	source->setEntitiesPerCreation(0u);
	ASSERT_NE(sink, nullptr);
	source->connectTo(component);
	component->connectTo(sink);
	BioCalendarDiscreteProbe observer(model, &network, &substrate, &reaction);
	model->getComponentManager()->remove(&observer);
	ScheduleContinuousComponentScenario setup{model, component, &observer, 5.5, 0.25};
	model->getOnEventManager()->addOnReplicationStartHandler(
		&setup, &ScheduleContinuousComponentScenario::schedule);
	ContinuousProcessArrivalProbe arrivals("BioCalendarSink");
	model->getOnEventManager()->addOnProcessEventHandler(
		&arrivals, &ContinuousProcessArrivalProbe::onProcessEvent);
	model->getSimulation()->setNumberOfReplications(1u);
	model->getSimulation()->setReplicationLength(20.0, Util::TimeUnit::second);
	model->getSimulation()->setShowReportsAfterReplication(false);
	model->getSimulation()->setShowReportsAfterSimulation(false);
	ASSERT_TRUE(model->check()) << model->getTracer()->errorMessages()->list()->front();
	model->getSimulation()->start();

	EXPECT_EQ(network.getExecutionState(), ContinuousModelDataDefinition::ExecutionState::Completed)
		<< network.getLastErrorMessage() << " status=" << network.getLastStatus();
	EXPECT_TRUE(component->getLastSucceeded()) << component->getLastMessage();
	EXPECT_DOUBLE_EQ(observer.observedTime, 5.0);
	const double rk4Before = 1.0 - 0.1 + 0.1 * 0.1 / 2.0 -
		0.1 * 0.1 * 0.1 / 6.0 + 0.1 * 0.1 * 0.1 * 0.1 / 24.0;
	const double rk4After = 1.0 - 0.2 + 0.2 * 0.2 / 2.0 -
		0.2 * 0.2 * 0.2 / 6.0 + 0.2 * 0.2 * 0.2 * 0.2 / 24.0;
	EXPECT_NEAR(observer.observedAmount, 10.0 * std::pow(rk4Before, 5.0), 1e-12);
	ASSERT_EQ(arrivals.times.size(), 2u);
	EXPECT_DOUBLE_EQ(arrivals.times[0], 10.0);
	EXPECT_DOUBLE_EQ(arrivals.times[1], 10.0);
	EXPECT_NE(arrivals.entityIds[0], arrivals.entityIds[1]);
	EXPECT_EQ(network.getExecutionState(), ContinuousModelDataDefinition::ExecutionState::Completed);
	EXPECT_NE(network.getLastResponsePayload().find("\"status\":\"Completed\""), std::string::npos);
	EXPECT_NEAR(substrate.getAmount() + product.getAmount(), 10.0, 1e-12);
	EXPECT_NEAR(substrate.getAmount(), 10.0 * std::pow(rk4Before, 5.0) * std::pow(rk4After, 5.0), 1e-11);
}

TEST(ContinuousCalendarTest, SynchronousBioRunPublishesCompletedStatusInPayload) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	BioSpecies substrate(model, "BioPayloadSubstrate");
	substrate.setInitialAmount(2.0);
	substrate.setAmount(2.0);
	BioSpecies product(model, "BioPayloadProduct");
	product.setInitialAmount(0.0);
	product.setAmount(0.0);
	BioReaction reaction(model, "BioPayloadReaction");
	reaction.addReactant("BioPayloadSubstrate", 1.0);
	reaction.addProduct("BioPayloadProduct", 1.0);
	reaction.setRateConstant(0.1);
	BioNetwork network(model, "BioPayloadNetwork");
	network.addSpecies("BioPayloadSubstrate");
	network.addSpecies("BioPayloadProduct");
	network.addReaction("BioPayloadReaction");
	network.setStopTime(1.0);
	network.setStepSize(0.5);
	network.setStepsToSynchronize(0u);

	std::string errorMessage;
	ASSERT_TRUE(network.simulate(errorMessage)) << errorMessage;
	EXPECT_EQ(network.getLastStatus(), "Completed");
	EXPECT_NE(network.getLastResponsePayload().find("\"status\":\"Completed\""), std::string::npos);
}
