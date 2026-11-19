// Unit tests for EFSMNetwork, the finite-state specialization of the
// ModalModel/Network architecture.

#include <gtest/gtest.h>

#include "kernel/simulator/Persistence.h"
#include "kernel/simulator/PluginInformation.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "plugins/components/ModalModel/FSMState.h"
#include "plugins/data/ModalModel/EFSMNetwork.h"
#include "plugins/data/ModalModel/DefaultNetwork.h"
#include "plugins/data/ModalModel/NetworkActivation.h"
#include "plugins/data/Logic/Variable.h"

#include <memory>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>
#include <string>
#include <unistd.h>

namespace {

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

class EFSMNetworkProbe : public EFSMNetwork {
public:
	EFSMNetworkProbe(Model* model, const std::string& name = "") : EFSMNetwork(model, name) {}
	bool CheckProbe(std::string& errorMessage) { return _check(errorMessage); }
	void SaveInstanceProbe(PersistenceRecord* fields, bool saveDefaultValues = true) { _saveInstance(fields, saveDefaultValues); }
	bool LoadInstanceProbe(PersistenceRecord* fields) { return _loadInstance(fields); }
	void InitBetweenReplicationsProbe() { _initBetweenReplications(); }
};

class RefinementLifecycleProbe final : public DefaultNetwork {
public:
    RefinementLifecycleProbe(Model* model, const std::string& name)
        : DefaultNetwork(model, name) { addInputPort("input"); addOutputPort("output"); }

    void initializeRefinement() override {
        ++initialized;
        events.push_back("initialize");
    }
    NetworkActivationResult fireRefinement(const NetworkActivationFrame& frame) override {
        ++fired;
        events.push_back("fire");
        if (throwsOnFire) throw std::runtime_error("refinement-boom");
        NetworkActivationResult result(getNumOutputPorts());
        if (frame.isPresent(0)) result.setPresent(0, frame.getValue(0));
        return result;
    }
    bool postfireRefinement() override {
        ++postfired;
        events.push_back("postfire");
        if (throwsOnPostfire) throw std::runtime_error("postfire-boom");
        return !terminateOnPostfire;
    }
    int initialized = 0;
    int fired = 0;
    int postfired = 0;
    bool throwsOnFire = false;
    bool throwsOnPostfire = false;
    bool terminateOnPostfire = false;
    std::vector<std::string> events;
};

} // namespace

TEST(EFSMNetworkTest, ProgrammaticStatesBelongToNetworkEditingLevel) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	EFSMNetwork network(model, "Machine");
	FSMState idle(model, "Idle");
	FSMState busy(model, "Busy");

	ASSERT_EQ(idle.getLevel(), 0u);
	ASSERT_EQ(busy.getLevel(), 0u);
	network.addState(&idle);
	network.addState(&busy);

	const auto networkLevel = static_cast<unsigned int>(network.getId());
	EXPECT_NE(networkLevel, 0u);
	EXPECT_EQ(idle.getLevel(), networkLevel);
	EXPECT_EQ(busy.getLevel(), networkLevel);
	// A second registration is a no-op and does not create new domain objects.
	network.addState(&busy);
	EXPECT_EQ(network.getStates()->size(), 2u);
}

TEST(EFSMNetworkTest, InitialStateIdentityAndFlagsRemainConsistent) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    EFSMNetwork network(model, "UniqueInitialFlags");
    FSMState first(model, "First");
    FSMState second(model, "Second");
    network.addState(&first);
    network.addState(&second);
    EXPECT_EQ(network.getInitialState(), &first);
    EXPECT_TRUE(first.isInitialNode());
    EXPECT_FALSE(second.isInitialNode());

    network.setInitialState(&second);
    EXPECT_EQ(network.getInitialState(), &second);
    EXPECT_FALSE(first.isInitialNode());
    EXPECT_TRUE(second.isInitialNode());

    network.setInitialState(nullptr);
    EXPECT_EQ(network.getInitialState(), nullptr);
    EXPECT_FALSE(first.isInitialNode());
    EXPECT_FALSE(second.isInitialNode());

    network.setInitialState(&first);
    EXPECT_EQ(network.getInitialState(), &first);
    EXPECT_TRUE(first.isInitialNode());
    EXPECT_FALSE(second.isInitialNode());
}

TEST(EFSMNetworkLifecycle, PreemptiveTransitionsSkipSourceRefinementFire) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "Preemption");
    FSMState start(model, "Start");
    FSMState done(model, "Done");
    RefinementLifecycleProbe refinement(model, "SourceRefinement");
    start.setRefinementNetwork(&refinement);
    network.setInitialState(&start);
    network.addState(&done);
    auto* t = new EFSMTransition(&start, &done, "Preempt");
    t->setGuardExpression("1");
    t->setPreemptive(true);
    network.addTransition(t);

    const NetworkActivationFrame frame(network.getNumInputPorts());
    network.activate(frame);
    EXPECT_EQ(network.getCurrentState(), &done);
    EXPECT_EQ(refinement.fired, 0);
    EXPECT_EQ(refinement.postfired, 0);
    EXPECT_EQ(refinement.initialized, 1);
}

TEST(EFSMNetworkLifecycle, NonPreemptiveTransitionsFireAndPostfireRefinement) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    EFSMNetwork network(model, "Nonpreemptive");
    FSMState start(model, "Start");
    FSMState done(model, "Done");
    RefinementLifecycleProbe refinement(model, "SourceRefinement");
    start.setRefinementNetwork(&refinement);
    network.setInitialState(&start);
    network.addState(&done);
    auto* t = new EFSMTransition(&start, &done, "Normal");
    t->setGuardExpression("1");
    network.addTransition(t);

    NetworkActivationFrame frame(network.getNumInputPorts());
    frame.setPresent(0, 7.0);
    const auto result = network.activate(frame);
    EXPECT_EQ(network.getCurrentState(), &done);
    EXPECT_EQ(refinement.initialized, 1);
    EXPECT_EQ(refinement.fired, 1);
    EXPECT_EQ(refinement.postfired, 1);
    ASSERT_EQ(refinement.events.size(), 3);
    EXPECT_EQ(refinement.events[0], "initialize");
    EXPECT_EQ(refinement.events[1], "fire");
    EXPECT_EQ(refinement.events[2], "postfire");
    ASSERT_TRUE(result.isPresent(0));
    EXPECT_DOUBLE_EQ(result.getValue(0), 7.0);
}

TEST(EFSMNetworkLifecycle, HistoryPreservesAndResetReinitializesDestinationRefinement) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    EFSMNetwork network(model, "History");
    FSMState a(model, "A");
    FSMState b(model, "B");
    RefinementLifecycleProbe refinement(model, "BRefinement");
    b.setRefinementNetwork(&refinement);
    network.setInitialState(&a);
    network.addState(&b);
    auto* enter = new EFSMTransition(&a, &b, "EnterB");
    enter->setGuardExpression("1");
    auto* leave = new EFSMTransition(&b, &a, "LeaveB");
    leave->setGuardExpression("1");
    enter->setHistory(true);
    network.addTransition(enter);
    network.addTransition(leave);

    NetworkActivationFrame frame(network.getNumInputPorts());
    network.activate(frame);
    EXPECT_EQ(network.getCurrentState(), &b);
    EXPECT_EQ(refinement.initialized, 1);
    network.activate(frame);
    EXPECT_EQ(network.getCurrentState(), &a);
    EXPECT_EQ(refinement.fired, 1);
    network.activate(frame);
    EXPECT_EQ(network.getCurrentState(), &b);
    EXPECT_EQ(refinement.initialized, 1);
    network.activate(frame);
    enter->setHistory(false);
    network.activate(frame);
    EXPECT_EQ(network.getCurrentState(), &b);
    EXPECT_EQ(refinement.initialized, 2);
}

TEST(EFSMNetworkLifecycle, ErrorTransitionHandlesOnlyRefinementFailures) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    EFSMNetwork network(model, "ErrorHandling");
    FSMState active(model, "Active");
    FSMState recovered(model, "Recovered");
    RefinementLifecycleProbe refinement(model, "FailingRefinement");
    refinement.throwsOnFire = true;
    active.setRefinementNetwork(&refinement);
    network.setInitialState(&active);
    network.addState(&recovered);
    auto* errorTransition = new EFSMTransition(&active, &recovered, "Recover");
    errorTransition->setGuardExpression("1");
    errorTransition->setErrorTransition(true);
    network.addTransition(errorTransition);
    EXPECT_NO_THROW(network.activate(NetworkActivationFrame(network.getNumInputPorts())));
    EXPECT_EQ(network.getCurrentState(), &recovered);
    EXPECT_EQ(network.getLastTransition(), errorTransition);
    EXPECT_EQ(network.getLastRefinementError(), "refinement-boom");
    EXPECT_EQ(refinement.postfired, 0);
}

TEST(EFSMNetworkLifecycle, ErrorTransitionHandlesRefinementPostfireFailure) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    EFSMNetwork network(model, "PostfireError");
    FSMState source(model, "Active");
    FSMState destination(model, "Recovered");
    RefinementLifecycleProbe refinement(model, "PostfireThrower");
    source.setRefinementNetwork(&refinement);
    refinement.throwsOnPostfire = true;
    network.setInitialState(&source);
    network.addState(&destination);
    auto* exceptionHandler = new EFSMTransition(&source, &destination, "Recover");
    exceptionHandler->setGuardExpression("1");
    exceptionHandler->setErrorTransition(true);
    exceptionHandler->setOutputActions("output = 77");
    network.addTransition(exceptionHandler);
    NetworkActivationResult recovery;
    EXPECT_NO_THROW(recovery = network.activate(NetworkActivationFrame(network.getNumInputPorts())));
    ASSERT_TRUE(recovery.isPresent(0));
    EXPECT_DOUBLE_EQ(recovery.getValue(0), 77.0);
    EXPECT_EQ(network.getCurrentState(), &destination);
    EXPECT_EQ(network.getLastTransition(), exceptionHandler);
    EXPECT_EQ(network.getLastRefinementError(), "postfire-boom");
    EXPECT_EQ(refinement.fired, 1);
    EXPECT_EQ(refinement.postfired, 1);
}

TEST(EFSMNetworkLifecycle, PostfireFailureOverridesUncommittedOrdinaryTransition) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    EFSMNetwork network(model, "PostfirePrecedence");
    FSMState active(model, "Active");
    FSMState normalTarget(model, "NormalTarget");
    FSMState recovery(model, "Recovery");
    RefinementLifecycleProbe refinement(model, "PostfireFailing");
    active.setRefinementNetwork(&refinement);
    refinement.throwsOnPostfire = true;
    network.setInitialState(&active);
    network.addState(&normalTarget);
    network.addState(&recovery);
    auto* ordinary = new EFSMTransition(&active, &normalTarget, "Normal");
    ordinary->setGuardExpression("1");
    ordinary->setOutputExpression("9");
    network.addTransition(ordinary);
    auto* failure = new EFSMTransition(&active, &recovery, "OnError");
    failure->setErrorTransition(true);
    failure->setGuardExpression("1");
    network.addTransition(failure);

    NetworkActivationFrame frame(network.getNumInputPorts());
    const auto result = network.activate(frame);
    EXPECT_EQ(network.getCurrentState(), &recovery);
    EXPECT_EQ(network.getLastTransition(), failure);
    EXPECT_EQ(network.getLastRefinementError(), "postfire-boom");
    EXPECT_EQ(refinement.fired, 1);
    EXPECT_EQ(refinement.postfired, 1);
    EXPECT_FALSE(result.isPresent(0)); // uncommitted output is not exposed
}

TEST(EFSMNetworkLifecycle, UnhandledRefinementExceptionPropagates) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    EFSMNetwork network(model, "UnhandledError");
    FSMState initial(model, "Initial");
    RefinementLifecycleProbe refinement(model, "ThrowingRefinement");
    refinement.throwsOnFire = true;
    initial.setRefinementNetwork(&refinement);
    network.setInitialState(&initial);
    EXPECT_THROW(network.activate(NetworkActivationFrame(network.getNumInputPorts())),
                 std::runtime_error);
    EXPECT_EQ(network.getCurrentState(), &initial);
    EXPECT_EQ(network.getLastRefinementError(), "refinement-boom");
}

TEST(EFSMNetworkLifecycle, TerminationFiresOnlyWhenPostfireDeclaresCompletion) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    EFSMNetwork network(model, "Termination");
    FSMState active(model, "Active");
    FSMState complete(model, "Complete");
    RefinementLifecycleProbe refinement(model, "FiniteRefinement");
    active.setRefinementNetwork(&refinement);
    network.setInitialState(&active);
    network.addState(&complete);
    auto* t = new EFSMTransition(&active, &complete, "Done");
    t->setTermination(true);
    t->setGuardExpression("1");
    network.addTransition(t);
    NetworkActivationFrame frame(network.getNumInputPorts());
    network.activate(frame);
    EXPECT_EQ(network.getCurrentState(), &active);
    EXPECT_EQ(refinement.postfired, 1);
    refinement.terminateOnPostfire = true;
    network.activate(frame);
    EXPECT_EQ(network.getCurrentState(), &complete);
    EXPECT_EQ(refinement.postfired, 2);
}

TEST(EFSMNetworkLifecycle, ImmediateTransitionsChainWithoutExtraOrdinarySteps) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    EFSMNetwork network(model, "Immediate");
    FSMState a(model, "A");
    FSMState b(model, "B");
    FSMState c(model, "C");
    FSMState d(model, "D");
    network.setInitialState(&a);
    network.addState(&b);
    network.addState(&c);
    network.addState(&d);
    auto* ab = new EFSMTransition(&a, &b, "AB");
    auto* bc = new EFSMTransition(&b, &c, "BC");
    auto* cd = new EFSMTransition(&c, &d, "CD");
    ab->setGuardExpression("1");
    bc->setGuardExpression("1");
    bc->setImmediate(true);
    cd->setGuardExpression("1");
    network.addTransition(ab);
    network.addTransition(bc);
    network.addTransition(cd);
    const NetworkActivationFrame frame(network.getNumInputPorts());
    network.activate(frame);
    EXPECT_EQ(network.getCurrentState(), &c);
    EXPECT_EQ(network.getLastTransition(), bc);
    network.activate(frame);
    EXPECT_EQ(network.getCurrentState(), &d);
}

TEST(EFSMNetworkLifecycle, RefinementRuntimeControlsExposeBoundAndReadOnlyError) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "RuntimeControls");
    SimulationControl* limit = nullptr;
    SimulationControl* error = nullptr;
    for (SimulationControl* control : *network.getSimulationControls()->list()) {
        if (control == nullptr) continue;
        if (control->getName() == "Max Immediate Transitions Per Activation") limit = control;
        if (control->getName() == "Last Refinement Error") error = control;
    }
    ASSERT_NE(limit, nullptr);
    ASSERT_NE(error, nullptr);
    EXPECT_EQ(limit->getValue(), "64");
    limit->setValue("7");
    EXPECT_EQ(network.getMaxImmediateTransitionsPerActivation(), 7u);
    EXPECT_THROW(limit->setValue("0"), std::invalid_argument);
    EXPECT_TRUE(error->getValue().empty());
    EXPECT_THROW(error->setValue("manual-override"), std::logic_error);
}

TEST(EFSMNetworkLifecycle, ImmediateTransitionCanBecomeEnabledOnLaterActivation) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "DelayedImmediate");
    FSMState current(model, "Current");
    FSMState target(model, "Target");
    network.setInitialState(&current);
    network.addState(&target);
    auto* edge = new EFSMTransition(&current, &target, "Immediate");
    edge->setImmediate(true);
    edge->setGuardExpression("0");
    network.addTransition(edge);
    const NetworkActivationFrame frame(network.getNumInputPorts());
    network.activate(frame);
    EXPECT_EQ(network.getCurrentState(), &current);
    EXPECT_EQ(network.getLastTransition(), nullptr);
    edge->setGuardExpression("1");
    network.activate(frame);
    EXPECT_EQ(network.getCurrentState(), &target);
    EXPECT_EQ(network.getLastTransition(), edge);
}

TEST(EFSMNetworkLifecycle, ImmediateAndOrdinaryPreemptiveCompeteByPriorityOnReactionStart) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "UnifiedPriority");
    FSMState current(model, "Current");
    FSMState immediateTarget(model, "ImmediateTarget");
    FSMState ordinaryTarget(model, "OrdinaryTarget");
    network.setInitialState(&current);
    network.addState(&immediateTarget);
    network.addState(&ordinaryTarget);
    auto* immediate = new EFSMTransition(&current, &immediateTarget, "Immediate");
    immediate->setGuardExpression("1");
    immediate->setImmediate(true);
    immediate->setPreemptive(true);
    immediate->setPriority(10);
    auto* ordinary = new EFSMTransition(&current, &ordinaryTarget, "Ordinary");
    ordinary->setGuardExpression("1");
    ordinary->setPreemptive(true);
    ordinary->setPriority(1);
    network.addTransition(immediate);
    network.addTransition(ordinary);
    network.activate(NetworkActivationFrame(network.getNumInputPorts()));
    EXPECT_EQ(network.getCurrentState(), &ordinaryTarget);
    EXPECT_EQ(network.getLastTransition(), ordinary);
}

TEST(EFSMNetworkLifecycle, ImmediateSelfLoopStopsAtConfiguredSafetyBound) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    EFSMNetwork network(model, "ImmediateCycle");
    FSMState a(model, "A");
    network.setInitialState(&a);
    auto* cycle = new EFSMTransition(&a, &a, "Cycle");
    cycle->setGuardExpression("1");
    cycle->setImmediate(true);
    network.addTransition(cycle);
    network.setMaxImmediateTransitionsPerActivation(3);
    EXPECT_THROW(network.activate(NetworkActivationFrame(network.getNumInputPorts())),
                 std::runtime_error);
    EXPECT_EQ(network.getCurrentState(), &a);
}

TEST(EFSMNetworkLifecycle, NestedEFSMRefinementTerminationUsesFinalState) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    EFSMNetwork parent(model, "ParentEFSM");
    EFSMNetwork child(model, "ChildEFSM");
    FSMState parentActive(model, "ParentActive");
    FSMState parentFinal(model, "ParentFinal");
    FSMState childActive(model, "ChildActive");
    FSMState childFinal(model, "ChildFinal");
    childFinal.setFinalNode(true);
    child.setInitialState(&childActive);
    child.addState(&childFinal);
    auto* childAdvance = new EFSMTransition(&childActive, &childFinal, "ChildAdvance");
    childAdvance->setGuardExpression("1");
    child.addTransition(childAdvance);

    parentActive.setRefinementNetwork(&child);
    parent.setInitialState(&parentActive);
    parent.addState(&parentFinal);
    auto* termination = new EFSMTransition(&parentActive, &parentFinal, "ChildTerminated");
    termination->setGuardExpression("1");
    termination->setTermination(true);
    parent.addTransition(termination);

    const NetworkActivationFrame frame(parent.getNumInputPorts());
    parent.activate(frame);
    EXPECT_EQ(child.getCurrentState(), &childFinal);
    EXPECT_EQ(parent.getCurrentState(), &parentFinal);
    EXPECT_EQ(parent.getLastTransition(), termination);
}

TEST(EFSMNetworkLifecycle, RefinementNetworkReferenceIsPersistedAndResolvedByTypeAndName) {
    Simulator simulator;
    Model* sourceModel = simulator.getModelManager()->newModel();
    ASSERT_NE(sourceModel, nullptr);
    DefaultNetwork sourceRefinement(sourceModel, "Subnetwork");
    FSMState original(sourceModel, "MainState");
    original.setRefinementNetwork(&sourceRefinement);

    FakeModelPersistenceRuntime persistence;
    PersistenceRecord fields(persistence);
    ModelDataDefinition::SaveInstance(&fields, &original);

    Model* targetModel = simulator.getModelManager()->newModel();
    ASSERT_NE(targetModel, nullptr);
    DefaultNetwork targetRefinement(targetModel, "Subnetwork");
    auto* restored = dynamic_cast<FSMState*>(FSMState::LoadInstance(targetModel, &fields));
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->getRefinementNetwork(), &targetRefinement);
    EXPECT_EQ(restored->getRefinementNetworkIdentity(),
              targetRefinement.getClassname() + "::" + targetRefinement.getName());
    // A live reference remains attached to the model-owned object even if
    // it is renamed after a persistence round trip.
    targetRefinement.setName("RenamedSubnetwork");
    EXPECT_EQ(restored->getRefinementNetwork(), &targetRefinement);
    EXPECT_EQ(restored->getRefinementNetworkIdentity(),
              targetRefinement.getClassname() + "::RenamedSubnetwork");

    restored->setRefinementNetwork(nullptr);
    EXPECT_EQ(restored->getRefinementNetwork(), nullptr);
    EXPECT_FALSE(restored->hasUnresolvedRefinementNetwork());
}

TEST(EFSMNetworkLifecycle, RefinementReferenceControlSelectsAndRemovesWithoutCreatingNetworks) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    FSMState state(model, "State");
    EFSMNetwork refinement(model, "ExistingEFSM");
    SimulationControl* reference = nullptr;
    for (SimulationControl* control : *state.getSimulationControls()->list()) {
        if (control != nullptr && control->getName() == "Refinement Network") {
            reference = control;
            break;
        }
    }
    ASSERT_NE(reference, nullptr);
    EXPECT_TRUE(reference->supportsExistingObjectSelection());
    EXPECT_TRUE(reference->supportsReferenceRemoval());
    EXPECT_FALSE(reference->supportsObjectCreation());

    const auto before = model->getDataManager()->getDataDefinitionsAssignableTo<DefaultNetwork>().size();
    reference->setValue(refinement.getClassname() + "::" + refinement.getName());
    EXPECT_EQ(state.getRefinementNetwork(), &refinement);
    reference->setValue("", true);
    EXPECT_EQ(state.getRefinementNetwork(), nullptr);
    const auto after = model->getDataManager()->getDataDefinitionsAssignableTo<DefaultNetwork>().size();
    EXPECT_EQ(after, before);
}

TEST(EFSMNetworkLifecycle, QualifierPersistenceRoundTrip) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetworkProbe source(model, "Qualifiers");
    FSMState a(model, "A");
    FSMState b(model, "B");
    source.setInitialState(&a);
    source.addState(&b);
    auto* t = new EFSMTransition(&a, &b, "AllFlags");
    t->setImmediate(true);
    t->setPreemptive(true);
    t->setHistory(true);
    t->setErrorTransition(false);
    t->setTermination(false);
    source.addTransition(t);
    source.setMaxImmediateTransitionsPerActivation(21);
    FakeModelPersistenceRuntime persistence;
    PersistenceRecord fields(persistence);
    source.SaveInstanceProbe(&fields, true);

    EFSMNetworkProbe restored(model, "Restored");
    ASSERT_TRUE(restored.LoadInstanceProbe(&fields));
    ASSERT_EQ(restored.getTransitions()->size(), 1u);
    EFSMTransition* loaded = restored.getTransitions()->front();
    ASSERT_NE(loaded, nullptr);
    EXPECT_TRUE(loaded->isImmediate());
    EXPECT_TRUE(loaded->isPreemptive());
    EXPECT_TRUE(loaded->isHistory());
    EXPECT_FALSE(loaded->isErrorTransition());
    EXPECT_FALSE(loaded->isTermination());
    EXPECT_EQ(restored.getMaxImmediateTransitionsPerActivation(), 21u);
}

TEST(EFSMNetworkLifecycle, RecursiveRefinementActivationFailsWithoutStackOverflow) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork parent(model, "ParentCycle");
    EFSMNetwork child(model, "ChildCycle");
    FSMState outer(model, "Outer");
    FSMState inner(model, "Inner");
    parent.setInitialState(&outer);
    child.setInitialState(&inner);
    outer.setRefinementNetwork(&child);
    inner.setRefinementNetwork(&parent);

    const NetworkActivationFrame frame(parent.getNumInputPorts());
    EXPECT_THROW(parent.activate(frame), std::logic_error);
    EXPECT_EQ(parent.getCurrentState(), &outer);
    EXPECT_EQ(child.getCurrentState(), &inner);

    // An exception must release the activation guard, allowing recovery
    // after the cyclic reference is removed.
    inner.setRefinementNetwork(nullptr);
    EXPECT_NO_THROW(parent.activate(frame));
}

TEST(EFSMNetworkLifecycle, ValidationRejectsUnsupportedFlagCombinationsAndCycles) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    EFSMNetworkProbe parent(model, "Parent");
    EFSMNetwork child(model, "Child");
    FSMState initial(model, "Initial");
    FSMState nested(model, "Nested");
    parent.setInitialState(&initial);
    child.setInitialState(&nested);
    initial.setRefinementNetwork(&child);
    nested.setRefinementNetwork(&parent);
    std::string error;
    EXPECT_FALSE(parent.CheckProbe(error));
    EXPECT_NE(error.find("recursive refinement cycle"), std::string::npos);
    nested.setRefinementNetwork(nullptr);

    auto* illegal = new EFSMTransition(&initial, &initial, "IllegalError");
    illegal->setErrorTransition(true);
    illegal->setPreemptive(true);
    parent.addTransition(illegal);
    error.clear();
    EXPECT_FALSE(parent.CheckProbe(error));
    EXPECT_NE(error.find("cannot also be preemptive"), std::string::npos);
}

TEST(EFSMNetworkTest, ConstructionCreatesDefaultInputAndOutputPorts) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	EFSMNetwork network(model, "Machine");

	EXPECT_EQ(network.getNumInputPorts(), 1u);
	EXPECT_EQ(network.getNumOutputPorts(), 1u);
	EXPECT_EQ(network.getInputPortName(0), "input");
	EXPECT_EQ(network.getOutputPortName(0), "output");
	EXPECT_EQ(network.getStates()->size(), 0u);
	EXPECT_EQ(network.getTransitions()->size(), 0u);
	EXPECT_EQ(network.getCurrentState(), nullptr);
}

TEST(EFSMNetworkTest, DetachedTransitionCanBeUndoneAtItsOriginalOrdinal) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	EFSMNetwork network(model, "UndoMachine");
	FSMState idle(model, "Idle");
	FSMState busy(model, "Busy");
	network.addState(&idle);
	network.addState(&busy);
	auto* first = new EFSMTransition(&idle, &busy, "First");
	auto* middle = new EFSMTransition(&idle, &busy, "Middle");
	auto* last = new EFSMTransition(&busy, &idle, "Last");
	network.addTransition(first);
	network.addTransition(middle);
	network.addTransition(last);

	unsigned int ordinal = 99;
	std::unique_ptr<EFSMTransition> detached = network.detachTransition(middle, &ordinal);
	ASSERT_NE(detached, nullptr);
	EXPECT_EQ(ordinal, 1u);
	ASSERT_EQ(network.getTransitions()->size(), 2u);
	EXPECT_EQ(network.getTransitions()->getAtRank(0), first);
	EXPECT_EQ(network.getTransitions()->getAtRank(1), last);
	EXPECT_EQ(idle.getTransitions()->size(), 1u);
	EXPECT_EQ(idle.getTransitions()->front(), first);

	ASSERT_TRUE(network.restoreTransition(detached.get(), ordinal));
	detached.release(); // ownership transfers back to EFSMNetwork on success
	ASSERT_EQ(network.getTransitions()->size(), 3u);
	EXPECT_EQ(network.getTransitions()->getAtRank(0), first);
	EXPECT_EQ(network.getTransitions()->getAtRank(1), middle);
	EXPECT_EQ(network.getTransitions()->getAtRank(2), last);
	ASSERT_EQ(idle.getTransitions()->size(), 2u);
	EXPECT_EQ(idle.getTransitions()->getAtRank(1), middle);
}

TEST(EFSMNetworkTest, RestoreRejectsTransitionWithEndpointOutsideNetworkWithoutTakingOwnership) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	EFSMNetwork network(model, "UndoMachine");
	FSMState registered(model, "Registered");
	FSMState external(model, "External");
	network.addState(&registered);
	std::unique_ptr<EFSMTransition> transition(new EFSMTransition(&registered, &external, "Invalid"));

	EXPECT_FALSE(network.restoreTransition(transition.get(), 0));
	EXPECT_EQ(network.getTransitions()->size(), 0u);
	EXPECT_EQ(registered.getTransitions()->size(), 0u);
	EXPECT_EQ(transition->getDestination(), &external);
}

TEST(EFSMNetworkTest, ActivationFiresOneEnabledTransitionAndPublishesOutput) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	EFSMNetwork network(model, "Machine");
	FSMState idle(model, "Idle");
	FSMState busy(model, "Busy");
	EFSMTransition* transition = new EFSMTransition(&idle, &busy, "Start");
	transition->setGuardExpression("1");
	transition->setOutputExpression("99");
	network.setInitialState(&idle);
	network.addState(&busy);
	network.addTransition(transition);

	NetworkActivationFrame frame(network.getNumInputPorts());
	frame.setPresent(0, 1.0);
	NetworkActivationResult result = network.activate(frame);

	ASSERT_EQ(result.size(), 1u);
	EXPECT_TRUE(result.isPresent(0));
	EXPECT_DOUBLE_EQ(result.getValue(0), 99.0);
	EXPECT_EQ(network.getCurrentState(), &busy);
	EXPECT_EQ(network.getLastTransition(), transition);
	EXPECT_DOUBLE_EQ(network.getActivationCount(), 1.0);
}

TEST(EFSMNetworkTest, ActivationWithoutEnabledTransitionKeepsStateAndOutputsAbsent) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	EFSMNetwork network(model, "Machine");
	FSMState idle(model, "Idle");
	FSMState busy(model, "Busy");
	EFSMTransition* transition = new EFSMTransition(&idle, &busy, "Blocked");
	transition->setGuardExpression("0");
	transition->setOutputExpression("99");
	network.setInitialState(&idle);
	network.addState(&busy);
	network.addTransition(transition);

	NetworkActivationFrame frame(network.getNumInputPorts());
	NetworkActivationResult result = network.activate(frame);

	ASSERT_EQ(result.size(), 1u);
	EXPECT_FALSE(result.isPresent(0));
	EXPECT_EQ(network.getCurrentState(), &idle);
	EXPECT_DOUBLE_EQ(network.getActivationCount(), 1.0);
}

TEST(EFSMNetworkTest, ActivationChoosesLowestPriorityEnabledTransition) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	EFSMNetwork network(model, "Machine");
	FSMState initial(model, "Initial");
	FSMState slow(model, "Slow");
	FSMState fast(model, "Fast");
	EFSMTransition* lowPriority = new EFSMTransition(&initial, &slow, "SlowTransition");
	EFSMTransition* highPriority = new EFSMTransition(&initial, &fast, "FastTransition");
	lowPriority->setGuardExpression("1");
	lowPriority->setOutputExpression("10");
	lowPriority->setPriority(10);
	highPriority->setGuardExpression("1");
	highPriority->setOutputExpression("20");
	highPriority->setPriority(1);
	network.setInitialState(&initial);
	network.addState(&slow);
	network.addState(&fast);
	network.addTransition(lowPriority);
	network.addTransition(highPriority);

	NetworkActivationFrame frame(network.getNumInputPorts());
	NetworkActivationResult result = network.activate(frame);

	EXPECT_TRUE(result.isPresent(0));
	EXPECT_DOUBLE_EQ(result.getValue(0), 20.0);
	EXPECT_EQ(network.getCurrentState(), &fast);
}

TEST(EFSMNetworkTest, DefaultTransitionOnlyFiresWhenOrdinaryRelationsAreDisabled) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "FallbackMachine");
    FSMState initial(model, "Initial");
    FSMState ordinaryTarget(model, "Ordinary");
    FSMState fallbackTarget(model, "Fallback");
    network.setInitialState(&initial);
    network.addState(&ordinaryTarget);
    network.addState(&fallbackTarget);

    auto* ordinary = new EFSMTransition(&initial, &ordinaryTarget, "Normal");
    ordinary->setGuardExpression("1");
    ordinary->setOutputExpression("10");
    auto* fallback = new EFSMTransition(&initial, &fallbackTarget, "Default");
    fallback->setGuardExpression("1");
    fallback->setOutputExpression("20");
    fallback->setDefaultTransition(true);
    network.addTransition(ordinary);
    network.addTransition(fallback);

    const NetworkActivationFrame frame(network.getNumInputPorts());
    auto result = network.activate(frame);
    EXPECT_EQ(network.getCurrentState(), &ordinaryTarget);
    EXPECT_EQ(network.getLastTransition(), ordinary);
    ASSERT_TRUE(result.isPresent(0));
    EXPECT_DOUBLE_EQ(result.getValue(0), 10.0);

    network.setCurrentState(&initial);
    ordinary->setGuardExpression("0");
    result = network.activate(frame);
    EXPECT_EQ(network.getCurrentState(), &fallbackTarget);
    EXPECT_EQ(network.getLastTransition(), fallback);
    ASSERT_TRUE(result.isPresent(0));
    EXPECT_DOUBLE_EQ(result.getValue(0), 20.0);
}

TEST(EFSMNetworkTest, InputAndTriggerEventsUsePortPresenceNotPortValue) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "PortEventMachine");
    const unsigned int eventPort = network.addInputPort("trigger");
    FSMState waiting(model, "Waiting");
    FSMState done(model, "Done");
    network.setInitialState(&waiting);
    network.addState(&done);
    auto* t = new EFSMTransition(&waiting, &done, "EventDependent");
    t->setInputEvent("input");
    t->setTriggerEvent("trigger");
    t->setGuardExpression("1");
    network.addTransition(t);

    NetworkActivationFrame frame(network.getNumInputPorts());
    network.activate(frame);
    EXPECT_EQ(network.getCurrentState(), &waiting);
    EXPECT_EQ(network.getLastTransition(), nullptr);
    frame.setPresent(0, 0.0); // present-zero IS an input event
    network.activate(frame);
    EXPECT_EQ(network.getCurrentState(), &waiting); // missing second event
    frame.setPresent(eventPort, 0.0);
    network.activate(frame);
    EXPECT_EQ(network.getCurrentState(), &done);
    EXPECT_EQ(network.getLastTransition(), t);
}

TEST(EFSMNetworkTest, RejectsInputAndTriggerEventsThatAreNotPorts) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetworkProbe network(model, "BadEvents");
    FSMState a(model, "A");
    FSMState b(model, "B");
    network.setInitialState(&a);
    network.addState(&b);
    auto* t = new EFSMTransition(&a, &b, "BadPort");
    network.addTransition(t);
    t->setInputEvent("missingInput");
    t->setTriggerEvent("missingTrigger");
    std::string errors;
    EXPECT_FALSE(network.CheckProbe(errors));
    EXPECT_NE(errors.find("unknown Input Event input port"), std::string::npos);
    EXPECT_NE(errors.find("unknown Trigger Event input port"), std::string::npos);

    t->setInputEvent("input");
    t->setTriggerEvent("input");
    errors.clear();
    EXPECT_TRUE(network.CheckProbe(errors)) << errors;
}

TEST(EFSMNetworkTest, ProbabilityExpressionGatesTransitionWithBernoulliSemantics) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "BernoulliEligibility");
    FSMState waiting(model, "Waiting");
    FSMState done(model, "Done");
    network.setInitialState(&waiting);
    network.addState(&done);
    auto* t = new EFSMTransition(&waiting, &done, "Maybe");
    t->setGuardExpression("1");
    t->setProbabilityExpression("0");
    network.addTransition(t);
    const NetworkActivationFrame frame(network.getNumInputPorts());
    network.activate(frame);
    EXPECT_EQ(network.getCurrentState(), &waiting);
    EXPECT_EQ(network.getLastTransition(), nullptr);

    t->setProbabilityExpression("1");
    network.activate(frame);
    EXPECT_EQ(network.getCurrentState(), &done);
    EXPECT_EQ(network.getLastTransition(), t);

    network.setCurrentState(&waiting);
    t->setProbabilityExpression("1.25");
    EXPECT_THROW(network.activate(frame), std::domain_error);
    EXPECT_EQ(network.getCurrentState(), &waiting);
    EXPECT_EQ(network.getLastTransition(), nullptr);
    t->setProbabilityExpression("-0.01");
    EXPECT_THROW(network.activate(frame), std::domain_error);
    EXPECT_EQ(network.getCurrentState(), &waiting);
}

TEST(EFSMNetworkTest, ProbabilityExpressionUsesResettableSamplerAndIsStatisticallyPlausible) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetworkProbe network(model, "BernoulliReproducible");
    FSMState waiting(model, "Waiting");
    FSMState done(model, "Done");
    network.setInitialState(&waiting);
    network.addState(&done);
    auto* t = new EFSMTransition(&waiting, &done, "Sampled");
    t->setGuardExpression("1");
    t->setProbabilityExpression("0.35");
    network.addTransition(t);
    const NetworkActivationFrame frame(network.getNumInputPorts());

    constexpr int trials = 1500;
    const auto simulate = [&]() {
        std::vector<bool> observations;
        observations.reserve(trials);
        for (int i = 0; i < trials; ++i) {
            network.setCurrentState(&waiting);
            network.activate(frame);
            observations.push_back(network.getLastTransition() == t);
        }
        return observations;
    };
    const auto first = simulate();
    network.InitBetweenReplicationsProbe();
    const auto second = simulate();
    EXPECT_EQ(first, second);
    const double fraction = static_cast<double>(
        std::count(first.begin(), first.end(), true)) / trials;
    EXPECT_NEAR(fraction, 0.35, 0.055);
}

TEST(EFSMNetworkTest, DefaultFallbackDoesNotEvaluateProbabilityUnlessNeeded) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "LazyDefault");
    FSMState initial(model, "Initial");
    FSMState normalTarget(model, "Normal");
    FSMState defaultTarget(model, "Default");
    network.setInitialState(&initial);
    network.addState(&normalTarget);
    network.addState(&defaultTarget);
    auto* normal = new EFSMTransition(&initial, &normalTarget, "Normal");
    normal->setGuardExpression("1");
    auto* fallback = new EFSMTransition(&initial, &defaultTarget, "Fallback");
    fallback->setDefaultTransition(true);
    fallback->setProbabilityExpression("2"); // invalid, but not evaluated when normal is eligible
    network.addTransition(normal);
    network.addTransition(fallback);

    const NetworkActivationFrame frame(network.getNumInputPorts());
    EXPECT_NO_THROW(network.activate(frame));
    EXPECT_EQ(network.getCurrentState(), &normalTarget);
    EXPECT_EQ(network.getLastTransition(), normal);

    network.setCurrentState(&initial);
    normal->setGuardExpression("0");
    EXPECT_THROW(network.activate(frame), std::domain_error);
    EXPECT_EQ(network.getCurrentState(), &initial);
}

TEST(EFSMNetworkTest, NondeterministicTransitionFlagsAllowReproducibleConflictChoice) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetworkProbe network(model, "NondeterministicMachine");
    FSMState initial(model, "Initial");
    FSMState left(model, "Left");
    FSMState right(model, "Right");
    network.setInitialState(&initial);
    network.addState(&left);
    network.addState(&right);
    auto* a = new EFSMTransition(&initial, &left, "A");
    auto* b = new EFSMTransition(&initial, &right, "B");
    a->setGuardExpression("1");
    b->setGuardExpression("1");
    a->setNondeterministic(true);
    b->setNondeterministic(true);
    network.addTransition(a);
    network.addTransition(b);
    network.setConflictPolicy(EFSMNetwork::ConflictPolicy::MODEL_ERROR);

    const NetworkActivationFrame frame(network.getNumInputPorts());
    ASSERT_NO_THROW(network.activate(frame));
    FSMState* chosen = network.getCurrentState();
    EXPECT_TRUE(chosen == &left || chosen == &right);
    network.InitBetweenReplicationsProbe();
    network.activate(frame);
    EXPECT_EQ(network.getCurrentState(), chosen);
}

TEST(EFSMNetworkTest, OutputActionsPublishMultiplePortsAndSetActionsCommitVariables) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* count = new Variable(model, "count");
    ASSERT_NE(count, nullptr);
    count->setValue(2.0);
    EFSMNetworkProbe network(model, "ActionsMachine");
    network.addOutputPort("extra");
    FSMState idle(model, "Idle");
    FSMState busy(model, "Busy");
    network.setInitialState(&idle);
    network.addState(&busy);
    auto* edge = new EFSMTransition(&idle, &busy, "Update");
    edge->setGuardExpression("1");
    edge->setOutputActions("output = count + 1; extra = count * 2");
    edge->setSetActions("count = count + 4");
    network.addTransition(edge);

    const NetworkActivationResult result = network.activate(
        NetworkActivationFrame(network.getNumInputPorts()));
    ASSERT_EQ(result.size(), 2u);
    ASSERT_TRUE(result.isPresent(0));
    ASSERT_TRUE(result.isPresent(1));
    EXPECT_DOUBLE_EQ(result.getValue(0), 3.0);
    EXPECT_DOUBLE_EQ(result.getValue(1), 4.0);
    EXPECT_DOUBLE_EQ(count->getValue(), 6.0);
    EXPECT_EQ(network.getCurrentState(), &busy);
}

TEST(EFSMNetworkTest, ActionValidationRejectsUnknownTargetsAndMalformedAssignments) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetworkProbe network(model, "ActionsValidation");
    FSMState idle(model, "Idle");
    FSMState busy(model, "Busy");
    network.setInitialState(&idle);
    network.addState(&busy);
    auto* edge = new EFSMTransition(&idle, &busy, "BadActions");
    network.addTransition(edge);

    edge->setOutputActions("unknownPort = 2");
    std::string error;
    EXPECT_FALSE(network.CheckProbe(error));
    EXPECT_NE(error.find("unknown output port"), std::string::npos);
    edge->setOutputActions("");
    edge->setSetActions("unknownVariable = 2");
    error.clear();
    EXPECT_FALSE(network.CheckProbe(error));
    EXPECT_NE(error.find("unknown Variable"), std::string::npos);
    edge->setSetActions("invalid =");
    error.clear();
    EXPECT_FALSE(network.CheckProbe(error));
    EXPECT_NE(error.find("invalid Set Actions"), std::string::npos);

    edge->setSetActions("count = 1; count = 2");
    error.clear();
    EXPECT_FALSE(network.CheckProbe(error));
    EXPECT_NE(error.find("invalid Set Actions"), std::string::npos);

    edge->setSetActions("");
    edge->setOutputActions("output = 1; output = 2");
    error.clear();
    EXPECT_FALSE(network.CheckProbe(error));
    EXPECT_NE(error.find("invalid Output Actions"), std::string::npos);
}

TEST(EFSMNetworkTest, ModelErrorPolicyReportsAmbiguousActivationAndKeepsState) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	EFSMNetwork network(model, "Machine");
	FSMState initial(model, "Initial");
	FSMState left(model, "Left");
	FSMState right(model, "Right");
	EFSMTransition* first = new EFSMTransition(&initial, &left, "First");
	EFSMTransition* second = new EFSMTransition(&initial, &right, "Second");
	first->setGuardExpression("1");
	second->setGuardExpression("1");
	network.setInitialState(&initial);
	network.addState(&left);
	network.addState(&right);
	network.addTransition(first);
	network.addTransition(second);
	network.setConflictPolicy(EFSMNetwork::ConflictPolicy::MODEL_ERROR);

	NetworkActivationResult result = network.activate(NetworkActivationFrame(network.getNumInputPorts()));

	EXPECT_FALSE(result.isPresent(0));
	EXPECT_EQ(network.getCurrentState(), &initial);
	ASSERT_NE(model->getTracer()->errorMessages(), nullptr);
	EXPECT_NE(model->getTracer()->errorMessages()->front().find("multiple enabled transitions"), std::string::npos);
}

TEST(EFSMNetworkTest, NondeterministicChoiceUsesResettableKernelSampler) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	EFSMNetworkProbe network(model, "Machine");
	FSMState initial(model, "Initial");
	FSMState left(model, "Left");
	FSMState right(model, "Right");
	EFSMTransition* first = new EFSMTransition(&initial, &left, "First");
	EFSMTransition* second = new EFSMTransition(&initial, &right, "Second");
	first->setGuardExpression("1");
	second->setGuardExpression("1");
	network.setInitialState(&initial);
	network.addState(&left);
	network.addState(&right);
	network.addTransition(first);
	network.addTransition(second);
	network.setConflictPolicy(EFSMNetwork::ConflictPolicy::NONDETERMINISTIC_CHOICE);

	const NetworkActivationFrame frame(network.getNumInputPorts());
	network.activate(frame);
	FSMState* firstChoice = network.getCurrentState();
	network.setCurrentState(&initial);
	network.InitBetweenReplicationsProbe();
	network.activate(frame);

	EXPECT_EQ(network.getCurrentState(), firstChoice);
}

TEST(EFSMNetworkTest, ConflictPolicyPersistencePreservesSelectionMode) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	EFSMNetworkProbe source(model, "Machine");
	source.setConflictPolicy(EFSMNetwork::ConflictPolicy::MODEL_ERROR);
	FakeModelPersistenceRuntime persistence;
	PersistenceRecord fields(persistence);
	source.SaveInstanceProbe(&fields, true);

	EFSMNetworkProbe loaded(model, "LoadedMachine");
	ASSERT_TRUE(loaded.LoadInstanceProbe(&fields));
	EXPECT_EQ(loaded.getConflictPolicy(), EFSMNetwork::ConflictPolicy::MODEL_ERROR);
}

TEST(EFSMNetworkTest, ReplicationResetRestoresInitialStateAndActivationCounter) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	EFSMNetworkProbe network(model, "Machine");
	FSMState initial(model, "Initial");
	FSMState final(model, "Final");
	EFSMTransition* transition = new EFSMTransition(&initial, &final, "Advance");
	transition->setGuardExpression("1");
	network.setInitialState(&initial);
	network.addState(&final);
	network.addTransition(transition);

	NetworkActivationFrame frame(network.getNumInputPorts());
	network.activate(frame);
	ASSERT_EQ(network.getCurrentState(), &final);
	ASSERT_DOUBLE_EQ(network.getActivationCount(), 1.0);

	network.InitBetweenReplicationsProbe();

	EXPECT_EQ(network.getCurrentState(), &initial);
	EXPECT_DOUBLE_EQ(network.getActivationCount(), 0.0);
}

TEST(EFSMNetworkTest, CheckRejectsMissingStateAndAcceptsWellFormedMachine) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	EFSMNetworkProbe empty(model, "EmptyMachine");
	std::string errorMessage;
	EXPECT_FALSE(empty.CheckProbe(errorMessage));
	EXPECT_NE(errorMessage.find("requires at least one state"), std::string::npos);

	EFSMNetworkProbe valid(model, "ValidMachine");
	FSMState initial(model, "Initial");
	FSMState destination(model, "Destination");
	EFSMTransition* transition = new EFSMTransition(&initial, &destination, "Advance");
	transition->setGuardExpression("1");
	transition->setOutputExpression("2");
	transition->setProbabilityExpression("1");
	valid.setInitialState(&initial);
	valid.addState(&destination);
	valid.addTransition(transition);

	errorMessage.clear();
	EXPECT_TRUE(valid.CheckProbe(errorMessage)) << errorMessage;
}

TEST(EFSMNetworkTest, PersistenceRoundTripPreservesStatesTransitionsPortsAndCurrentState) {
	Simulator simulator;
	ASSERT_NE(simulator.getPluginManager()->insert("fsmstate.so"), nullptr);
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	EFSMNetworkProbe source(model, "PersistentMachine");
	FSMState idle(model, "Idle");
	FSMState busy(model, "Busy");
	idle.setEntryActionExpression("1");
	busy.setExitActionExpression("1");
	EFSMTransition* transition = new EFSMTransition(&idle, &busy, "Start");
	transition->setGuardExpression("1");
	transition->setOutputExpression("77");
	transition->setInputEvent("go");
	transition->setPriority(3);
	transition->setProbabilityExpression("1");
	transition->setDefaultTransition(true);
	transition->setNondeterministic(true);
	transition->setOutputActions("output = 77");
	transition->setSetActions("counter = 2");
	source.setInitialState(&idle);
	source.addState(&busy);
	source.addTransition(transition);
	source.setCurrentState(&busy);

	FakeModelPersistenceRuntime persistence;
	PersistenceRecord fields(persistence);
	source.SaveInstanceProbe(&fields, true);

	EFSMNetworkProbe loaded(model, "LoadedMachine");
	ASSERT_TRUE(loaded.LoadInstanceProbe(&fields));

	EXPECT_EQ(loaded.getNumInputPorts(), 1u);
	EXPECT_EQ(loaded.getNumOutputPorts(), 1u);
	EXPECT_EQ(loaded.getInputPortName(0), "input");
	EXPECT_EQ(loaded.getOutputPortName(0), "output");
	ASSERT_EQ(loaded.getStates()->size(), 2u);
	ASSERT_EQ(loaded.getTransitions()->size(), 1u);
	ASSERT_NE(loaded.getInitialState(), nullptr);
	ASSERT_NE(loaded.getCurrentState(), nullptr);
	EXPECT_EQ(loaded.getInitialState()->getName(), "Idle");
	EXPECT_EQ(loaded.getCurrentState()->getName(), "Busy");
	// Same-model internal round-trip must reuse ModelDataManager-owned states.
	ASSERT_EQ(model->getDataManager()->getDataDefinitionList(Util::TypeOf<FSMState>())->size(), 2u);
	EXPECT_EQ(loaded.getInitialState(), &idle);
	EXPECT_EQ(loaded.getCurrentState(), &busy);

	EFSMTransition* loadedTransition = loaded.getTransitions()->front();
	ASSERT_NE(loadedTransition, nullptr);
	EXPECT_EQ(loadedTransition->getName(), "Start");
	EXPECT_EQ(loadedTransition->getSource()->getName(), "Idle");
	EXPECT_EQ(loadedTransition->getDestination()->getName(), "Busy");
	EXPECT_EQ(loadedTransition->getGuardExpression(), "1");
	EXPECT_EQ(loadedTransition->getOutputExpression(), "77");
	EXPECT_EQ(loadedTransition->getInputEvent(), "go");
	EXPECT_EQ(loadedTransition->getPriority(), 3u);
	EXPECT_EQ(loadedTransition->getProbabilityExpression(), "1");
	EXPECT_TRUE(loadedTransition->isDefaultTransition());
	EXPECT_TRUE(loadedTransition->isNondeterministic());
	EXPECT_EQ(loadedTransition->getOutputActions(), "output = 77");
	EXPECT_EQ(loadedTransition->getSetActions(), "counter = 2");
}

TEST(EFSMNetworkTest, PersistencePreservesExplicitlyEmptyTransitionName) {
	Simulator simulator;
	ASSERT_NE(simulator.getPluginManager()->insert("fsmstate.so"), nullptr);
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	EFSMNetworkProbe source(model, "UnnamedMachine");
	FSMState idle(model, "Idle");
	FSMState busy(model, "Busy");
	EFSMTransition* unnamed = new EFSMTransition(&idle, &busy, "");
	unnamed->setGuardExpression("1");
	EFSMTransition* named = new EFSMTransition(&busy, &idle, "Named");
	source.setInitialState(&idle);
	source.addState(&busy);
	source.addTransition(unnamed);
	source.addTransition(named);

	FakeModelPersistenceRuntime persistence;
	PersistenceRecord fields(persistence);
	source.SaveInstanceProbe(&fields, false);

	ASSERT_TRUE(fields.find("transitionName[0]") != fields.end());
	EXPECT_EQ(fields.loadField("transitionName[0]", "missing"), "");
	ASSERT_TRUE(fields.find("transitionName[1]") != fields.end());
	EXPECT_EQ(fields.loadField("transitionName[1]", "missing"), "Named");

	EFSMNetworkProbe loaded(model, "LoadedUnnamedMachine");
	ASSERT_TRUE(loaded.LoadInstanceProbe(&fields));
	ASSERT_EQ(loaded.getTransitions()->size(), 2u);
	ASSERT_NE(loaded.getTransitions()->front(), nullptr);
	EXPECT_EQ(loaded.getTransitions()->front()->getName(), "");
	auto loadedTransitionIt = loaded.getTransitions()->list()->begin();
	ASSERT_NE(loadedTransitionIt, loaded.getTransitions()->list()->end());
	++loadedTransitionIt;
	ASSERT_NE(loadedTransitionIt, loaded.getTransitions()->list()->end());
	ASSERT_NE(*loadedTransitionIt, nullptr);
	EXPECT_EQ((*loadedTransitionIt)->getName(), "Named");

	// A legacy record without the field retains the established fallback.
	fields.erase("transitionName[0]");
	EFSMNetworkProbe legacyLoaded(model, "LegacyUnnamedMachine");
	ASSERT_TRUE(legacyLoaded.LoadInstanceProbe(&fields));
	ASSERT_EQ(legacyLoaded.getTransitions()->size(), 2u);
	ASSERT_NE(legacyLoaded.getTransitions()->front(), nullptr);
	EXPECT_EQ(legacyLoaded.getTransitions()->front()->getName(), "T[0]");
}

TEST(EFSMNetworkTest, ModelFileRoundTripPreservesConflictPolicyAndActivation) {
	Simulator simulator;
	simulator.getPluginManager()->autoInsertPlugins();
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	EFSMNetwork* network = new EFSMNetwork(model, "FileMachine");
	FSMState* idle = new FSMState(model, "Idle");
	FSMState* busy = new FSMState(model, "Busy");
	EFSMTransition* start = new EFSMTransition(idle, busy, "Start");
	start->setGuardExpression("1");
	start->setOutputExpression("91");
	start->setPriority(2u);
	network->setInitialState(idle);
	network->addState(busy);
	network->addTransition(start);
	network->setConflictPolicy(EFSMNetwork::ConflictPolicy::MODEL_ERROR);

	std::string errorMessage;
	ASSERT_TRUE(ModelDataDefinition::Check(network, errorMessage)) << errorMessage;

	const std::string filename = "/tmp/genesys_efsm_model_roundtrip_" + std::to_string(::getpid()) + ".gen";
	::unlink(filename.c_str());
	ASSERT_TRUE(model->save(filename));

	Model* loadedModel = simulator.getModelManager()->newModel();
	ASSERT_NE(loadedModel, nullptr);
	ASSERT_TRUE(loadedModel->load(filename));

	ModelDataDefinition* loadedDefinition =
		loadedModel->getDataManager()->getDataDefinition(Util::TypeOf<EFSMNetwork>(), "FileMachine");
	ASSERT_NE(loadedDefinition, nullptr);
	EFSMNetwork* loadedNetwork = dynamic_cast<EFSMNetwork*>(loadedDefinition);
	ASSERT_NE(loadedNetwork, nullptr);
	EXPECT_EQ(loadedNetwork->getConflictPolicy(), EFSMNetwork::ConflictPolicy::MODEL_ERROR);
	ASSERT_NE(loadedNetwork->getInitialState(), nullptr);
	EXPECT_EQ(loadedNetwork->getInitialState()->getName(), "Idle");
	EXPECT_EQ(loadedNetwork->getCurrentState()->getName(), "Idle");
	ASSERT_EQ(loadedNetwork->getStates()->size(), 2u);
	ASSERT_EQ(loadedNetwork->getTransitions()->size(), 1u);

	// Canonical identity: FSMState is ModelDataManager-owned. Nested network
	// fields must not materialize a second pair of Idle/Busy instances.
	ASSERT_EQ(loadedModel->getDataManager()->getDataDefinitionList(Util::TypeOf<FSMState>())->size(), 2u);
	ModelDataDefinition* idleCanonical =
		loadedModel->getDataManager()->getDataDefinition(Util::TypeOf<FSMState>(), "Idle");
	ModelDataDefinition* busyCanonical =
		loadedModel->getDataManager()->getDataDefinition(Util::TypeOf<FSMState>(), "Busy");
	ASSERT_NE(idleCanonical, nullptr);
	ASSERT_NE(busyCanonical, nullptr);
	EXPECT_EQ(loadedNetwork->getInitialState(), idleCanonical);
	EXPECT_EQ(loadedNetwork->getCurrentState(), idleCanonical);
	ASSERT_NE(loadedNetwork->getTransitions()->front(), nullptr);
	EXPECT_EQ(loadedNetwork->getTransitions()->front()->getSource(), idleCanonical);
	EXPECT_EQ(loadedNetwork->getTransitions()->front()->getDestination(), busyCanonical);
	for (FSMState* state : *loadedNetwork->getStates()->list()) {
		ASSERT_NE(state, nullptr);
		EXPECT_EQ(state, loadedModel->getDataManager()->getDataDefinition(Util::TypeOf<FSMState>(), state->getName()));
	}

	NetworkActivationResult result = loadedNetwork->activate(NetworkActivationFrame(loadedNetwork->getNumInputPorts()));
	EXPECT_TRUE(result.isPresent(0));
	EXPECT_DOUBLE_EQ(result.getValue(0), 91.0);
	ASSERT_NE(loadedNetwork->getCurrentState(), nullptr);
	EXPECT_EQ(loadedNetwork->getCurrentState()->getName(), "Busy");
	EXPECT_EQ(loadedNetwork->getCurrentState(), busyCanonical);

	::unlink(filename.c_str());
}

TEST(EFSMNetworkTest, PluginInformationDeclaresADataDefinitionInModalModelCategory) {
	std::unique_ptr<PluginInformation> info(EFSMNetwork::GetPluginInformation());

	ASSERT_NE(info, nullptr);
	EXPECT_FALSE(info->isComponent());
	EXPECT_EQ(info->getPluginTypename(), Util::TypeOf<EFSMNetwork>());
	EXPECT_EQ(info->getCategory(), "ModalModel");
	ASSERT_NE(info->getDataDefinitionLoader(), nullptr);
	ASSERT_NE(info->getDataDefinitionConstructor(), nullptr);
}
