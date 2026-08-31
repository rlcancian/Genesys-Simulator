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
#include "plugins/data/ModalModel/NetworkActivation.h"

#include <memory>

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

} // namespace

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

TEST(EFSMNetworkTest, ActivationFiresOneEnabledTransitionAndPublishesOutput) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	EFSMNetwork network(model, "Machine");
	FSMState idle(model, "Idle");
	FSMState busy(model, "Busy");
	EFSMTransition transition(&idle, &busy, "Start");
	transition.setGuardExpression("1");
	transition.setOutputExpression("99");
	network.setInitialState(&idle);
	network.addState(&busy);
	network.addTransition(&transition);

	NetworkActivationFrame frame(network.getNumInputPorts());
	frame.setPresent(0, 1.0);
	NetworkActivationResult result = network.activate(frame);

	ASSERT_EQ(result.size(), 1u);
	EXPECT_TRUE(result.isPresent(0));
	EXPECT_DOUBLE_EQ(result.getValue(0), 99.0);
	EXPECT_EQ(network.getCurrentState(), &busy);
	EXPECT_DOUBLE_EQ(network.getActivationCount(), 1.0);
}

TEST(EFSMNetworkTest, ActivationWithoutEnabledTransitionKeepsStateAndOutputsAbsent) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	EFSMNetwork network(model, "Machine");
	FSMState idle(model, "Idle");
	FSMState busy(model, "Busy");
	EFSMTransition transition(&idle, &busy, "Blocked");
	transition.setGuardExpression("0");
	transition.setOutputExpression("99");
	network.setInitialState(&idle);
	network.addState(&busy);
	network.addTransition(&transition);

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
	EFSMTransition lowPriority(&initial, &slow, "SlowTransition");
	EFSMTransition highPriority(&initial, &fast, "FastTransition");
	lowPriority.setGuardExpression("1");
	lowPriority.setOutputExpression("10");
	lowPriority.setPriority(10);
	highPriority.setGuardExpression("1");
	highPriority.setOutputExpression("20");
	highPriority.setPriority(1);
	network.setInitialState(&initial);
	network.addState(&slow);
	network.addState(&fast);
	network.addTransition(&lowPriority);
	network.addTransition(&highPriority);

	NetworkActivationFrame frame(network.getNumInputPorts());
	NetworkActivationResult result = network.activate(frame);

	EXPECT_TRUE(result.isPresent(0));
	EXPECT_DOUBLE_EQ(result.getValue(0), 20.0);
	EXPECT_EQ(network.getCurrentState(), &fast);
}

TEST(EFSMNetworkTest, ReplicationResetRestoresInitialStateAndActivationCounter) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	EFSMNetworkProbe network(model, "Machine");
	FSMState initial(model, "Initial");
	FSMState final(model, "Final");
	EFSMTransition transition(&initial, &final, "Advance");
	transition.setGuardExpression("1");
	network.setInitialState(&initial);
	network.addState(&final);
	network.addTransition(&transition);

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
	EFSMTransition transition(&initial, &destination, "Advance");
	transition.setGuardExpression("1");
	transition.setOutputExpression("2");
	transition.setProbabilityExpression("1");
	valid.setInitialState(&initial);
	valid.addState(&destination);
	valid.addTransition(&transition);

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
	EFSMTransition transition(&idle, &busy, "Start");
	transition.setGuardExpression("1");
	transition.setOutputExpression("77");
	transition.setInputEvent("go");
	transition.setPriority(3);
	transition.setProbabilityExpression("1");
	source.setInitialState(&idle);
	source.addState(&busy);
	source.addTransition(&transition);
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
