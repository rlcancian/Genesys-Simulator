// Unit tests for DefaultNetwork (source/plugins/data/ModalModel), the Phase 1
// core of the GenESyS ModalModel/network-of-computation architecture:
// see docs/ai_assistants/reference/GENESYS_MODAL_MODEL_NETWORK_ARCHITECTURE.md.
//
// Covers: construction/defaults, input/output port schema declaration and
// lookup, the activation counter (including the reportStatistics gate and
// the "counts activations, not transitions" contract), _check() validation,
// persistence round-trip, and replication reset.
//
// Build target: genesys_test_default_network.

#include <gtest/gtest.h>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/Persistence.h"
#include "plugins/data/ModalModel/DefaultNetwork.h"
#include "plugins/data/ModalModel/NetworkActivation.h"

namespace {

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

// Probe exposing DefaultNetwork's protected hooks so tests can drive
// validation, persistence and replication reset directly.
class DefaultNetworkProbe : public DefaultNetwork {
public:
	DefaultNetworkProbe(Model* model, const std::string& name = "") : DefaultNetwork(model, name) {}
	bool CheckProbe(std::string& errorMessage) { return _check(errorMessage); }
	void SaveInstanceProbe(PersistenceRecord* fields, bool saveDefaultValues = false) { _saveInstance(fields, saveDefaultValues); }
	bool LoadInstanceProbe(PersistenceRecord* fields) { return _loadInstance(fields); }
	void InitBetweenReplicationsProbe() { _initBetweenReplications(); }
};

} // namespace

TEST(DefaultNetworkTest, DefaultsToNoPortsAndZeroActivations) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	DefaultNetwork network(model, "EmptyNetwork");

	EXPECT_EQ(network.getNumInputPorts(), 0u);
	EXPECT_EQ(network.getNumOutputPorts(), 0u);
	EXPECT_DOUBLE_EQ(network.getActivationCount(), 0.0);
}

TEST(DefaultNetworkTest, AddInputAndOutputPortsAssignsSequentialIndices) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();

	DefaultNetwork network(model, "PortSchemaNetwork");
	EXPECT_EQ(network.addInputPort("Temperature"), 0u);
	EXPECT_EQ(network.addInputPort("Pressure"), 1u);
	EXPECT_EQ(network.addOutputPort("State"), 0u);
	EXPECT_EQ(network.addOutputPort("Alarm"), 1u);

	ASSERT_EQ(network.getNumInputPorts(), 2u);
	ASSERT_EQ(network.getNumOutputPorts(), 2u);
	EXPECT_EQ(network.getInputPortName(0), "Temperature");
	EXPECT_EQ(network.getInputPortName(1), "Pressure");
	EXPECT_EQ(network.getOutputPortName(0), "State");
	EXPECT_EQ(network.getOutputPortName(1), "Alarm");

	EXPECT_EQ(network.getInputPortIndex("Pressure"), 1);
	EXPECT_EQ(network.getOutputPortIndex("Alarm"), 1);
	EXPECT_EQ(network.getInputPortIndex("NoSuchPort"), -1);
	EXPECT_EQ(network.getOutputPortIndex("NoSuchPort"), -1);
}

TEST(DefaultNetworkTest, AddingDuplicatePortNameReturnsExistingIndexInstead) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();

	DefaultNetwork network(model, "DuplicatePortNetwork");
	EXPECT_EQ(network.addInputPort("Signal"), 0u);
	EXPECT_EQ(network.addInputPort("Signal"), 0u); // no duplicate created
	EXPECT_EQ(network.getNumInputPorts(), 1u);
}

TEST(DefaultNetworkTest, InvalidPortIndexReadsAreBoundedAndSafe) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();

	DefaultNetwork network(model, "BoundsNetwork");
	network.addInputPort("OnlyInput");

	EXPECT_EQ(network.getInputPortName(5), "");
	EXPECT_EQ(network.getOutputPortName(0), ""); // no output ports declared at all
}

TEST(DefaultNetworkTest, BaseActivateProducesAllAbsentResultSizedToOutputs) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();

	DefaultNetwork network(model, "BaseActivateNetwork");
	network.addInputPort("In0");
	network.addOutputPort("Out0");
	network.addOutputPort("Out1");

	NetworkActivationFrame frame(network.getNumInputPorts());
	frame.setPresent(0, 42.0);

	NetworkActivationResult result = network.activate(frame);

	ASSERT_EQ(result.size(), 2u);
	EXPECT_FALSE(result.isPresent(0));
	EXPECT_FALSE(result.isPresent(1));
	EXPECT_EQ(result.countPresent(), 0u);
}

TEST(DefaultNetworkTest, ActivationCounterIncrementsEveryCallRegardlessOfOutcome) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();

	DefaultNetwork network(model, "CounterNetwork");
	ASSERT_TRUE(network.isReportStatistics());

	NetworkActivationFrame frame(0);
	network.activate(frame);
	network.activate(frame);
	network.activate(frame);

	// The base DefaultNetwork never produces a present output (no transition/firing
	// semantics), yet the counter must still count all three activations.
	EXPECT_DOUBLE_EQ(network.getActivationCount(), 3.0);
}

TEST(DefaultNetworkTest, DisabledStatisticsDoNotCountActivations) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();

	DefaultNetwork network(model, "NoStatsNetwork");
	network.setReportStatistics(false);

	NetworkActivationFrame frame(0);
	network.activate(frame);
	network.activate(frame);

	EXPECT_DOUBLE_EQ(network.getActivationCount(), 0.0);
}

TEST(DefaultNetworkTest, ReplicationResetRestoresActivationCounterToZero) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();

	DefaultNetworkProbe network(model, "ResetNetwork");
	NetworkActivationFrame frame(0);
	network.activate(frame);
	network.activate(frame);
	ASSERT_DOUBLE_EQ(network.getActivationCount(), 2.0);

	network.InitBetweenReplicationsProbe();

	EXPECT_DOUBLE_EQ(network.getActivationCount(), 0.0);

	// A second replication must not accumulate residual state from the first.
	network.activate(frame);
	EXPECT_DOUBLE_EQ(network.getActivationCount(), 1.0);
}

TEST(DefaultNetworkTest, CheckRejectsEmptyPortNames) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();

	DefaultNetworkProbe network(model, "InvalidPortNameNetwork");
	network.addInputPort("GoodInput");
	network.addInputPort(""); // simulates a corrupted/incompletely-configured port

	std::string errorMessage;
	EXPECT_FALSE(network.CheckProbe(errorMessage));
	EXPECT_FALSE(errorMessage.empty());
}

TEST(DefaultNetworkTest, CheckAcceptsAWellFormedPortSchema) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();

	DefaultNetworkProbe network(model, "ValidPortNameNetwork");
	network.addInputPort("In0");
	network.addOutputPort("Out0");

	std::string errorMessage;
	EXPECT_TRUE(network.CheckProbe(errorMessage));
	EXPECT_TRUE(errorMessage.empty());
}

TEST(DefaultNetworkTest, PersistenceRoundTripPreservesPortSchema) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();

	DefaultNetworkProbe source(model, "SourceNetwork");
	source.addInputPort("Temperature");
	source.addInputPort("Pressure");
	source.addOutputPort("State");

	FakeModelPersistenceRuntime persistence;
	PersistenceRecord fields(persistence);
	source.SaveInstanceProbe(&fields, true);

	DefaultNetworkProbe destination(model, "DestinationNetwork");
	ASSERT_TRUE(destination.LoadInstanceProbe(&fields));

	ASSERT_EQ(destination.getNumInputPorts(), 2u);
	ASSERT_EQ(destination.getNumOutputPorts(), 1u);
	EXPECT_EQ(destination.getInputPortName(0), "Temperature");
	EXPECT_EQ(destination.getInputPortName(1), "Pressure");
	EXPECT_EQ(destination.getOutputPortName(0), "State");
}

TEST(NetworkActivationFrameTest, AbsentIsNotTheSameAsPresentZero) {
	NetworkActivationFrame frame(2);
	frame.setPresent(0, 0.0);
	// input 1 is left absent

	EXPECT_TRUE(frame.isPresent(0));
	EXPECT_DOUBLE_EQ(frame.getValue(0), 0.0);
	EXPECT_FALSE(frame.isPresent(1));
	EXPECT_DOUBLE_EQ(frame.getValue(1), 0.0); // reads back as 0.0, but callers must check isPresent() first
}

TEST(NetworkActivationResultTest, CountPresentReflectsOnlyMarkedOutputs) {
	NetworkActivationResult result(3);
	result.setPresent(0, 1.0);
	result.setPresent(2, 3.0);
	// output 1 left absent

	EXPECT_EQ(result.countPresent(), 2u);
	EXPECT_TRUE(result.isPresent(0));
	EXPECT_FALSE(result.isPresent(1));
	EXPECT_TRUE(result.isPresent(2));
}
