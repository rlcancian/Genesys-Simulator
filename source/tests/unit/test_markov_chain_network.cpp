// Unit tests for MarkovChainNetwork, the DTMC specialization of DefaultNetwork.

#include <gtest/gtest.h>

#include "kernel/simulator/Event.h"
#include "kernel/simulator/Persistence.h"
#include "kernel/simulator/Plugin.h"
#include "kernel/simulator/PluginInformation.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/essentialPlugins/Attribute.h"
#include "kernel/simulator/essentialPlugins/Entity.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelComponent.h"
#include "plugins/components/ModalModel/ModalModelDefault.h"
#include "plugins/data/ModalModel/MarkovChainNetwork.h"
#include "plugins/data/ModalModel/NetworkActivation.h"

#include <memory>
#include <vector>

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

class MarkovChainNetworkProbe : public MarkovChainNetwork {
public:
	MarkovChainNetworkProbe(Model* model, const std::string& name = "") : MarkovChainNetwork(model, name) {}
	bool CheckProbe(std::string& errorMessage) { return _check(errorMessage); }
	void SaveInstanceProbe(PersistenceRecord* fields, bool saveDefaultValues = true) { _saveInstance(fields, saveDefaultValues); }
	bool LoadInstanceProbe(PersistenceRecord* fields) { return _loadInstance(fields); }
	void InitBetweenReplicationsProbe() { _initBetweenReplications(); }
};

class ModalModelDefaultProbe final : public ModalModelDefault {
public:
	ModalModelDefaultProbe(Model* model, const std::string& name) : ModalModelDefault(model, name) {}

	void dispatch(Entity* entity, unsigned int inputPortNumber = 0) {
		_onDispatchEvent(entity, inputPortNumber);
	}
};

class CollectorSinkComponentProbe final : public ModelComponent {
public:
	CollectorSinkComponentProbe(Model* model, const std::string& name)
		: ModelComponent(model, "CollectorSinkComponentProbe", name) {}

	const std::vector<Entity*>& receivedEntities() const {
		return _receivedEntities;
	}

protected:
	void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override {
		(void)inputPortNumber;
		_receivedEntities.push_back(entity);
	}

private:
	std::vector<Entity*> _receivedEntities;
};

void drainFutureEvents(Model* model) {
	ASSERT_NE(model, nullptr);
	while (!model->getFutureEvents()->empty()) {
		Event* event = model->getFutureEvents()->front();
		model->getFutureEvents()->pop_front();
		ModelComponent::DispatchEvent(event);
		delete event;
	}
}

MarkovChainNetwork::MarkovTransition* transition(MarkovState* source, MarkovState* destination, double probability, const std::string& name) {
	return new MarkovChainNetwork::MarkovTransition(source, destination, probability, name);
}

} // namespace

TEST(MarkovChainNetworkTest, ConstructionCreatesStepAndStatePorts) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	MarkovChainNetwork network(model, "Chain");

	EXPECT_EQ(network.getNumInputPorts(), 1u);
	EXPECT_EQ(network.getInputPortName(0), "step");
	EXPECT_EQ(network.getNumOutputPorts(), 1u);
	EXPECT_EQ(network.getOutputPortName(0), "state");
	EXPECT_EQ(network.getStates()->size(), 0u);
	EXPECT_EQ(network.getTransitions()->size(), 0u);
	EXPECT_EQ(network.getInitialState(), nullptr);
	EXPECT_EQ(network.getCurrentState(), nullptr);
}

TEST(MarkovChainNetworkTest, CheckAcceptsStochasticRowsAndRejectsInvalidRows) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	MarkovChainNetworkProbe valid(model, "ValidChain");
	MarkovState a(model, "A");
	MarkovState b(model, "B");
	valid.setInitialState(&a);
	valid.addState(&b);
	valid.addTransition(transition(&a, &a, 0.25, "AA"));
	valid.addTransition(transition(&a, &b, 0.75, "AB"));
	valid.addTransition(transition(&b, &b, 1.0, "BB"));

	std::string errorMessage;
	EXPECT_TRUE(valid.CheckProbe(errorMessage)) << errorMessage;

	MarkovChainNetworkProbe invalid(model, "InvalidChain");
	MarkovState x(model, "X");
	MarkovState y(model, "Y");
	invalid.setInitialState(&x);
	invalid.addState(&y);
	invalid.addTransition(transition(&x, &y, 0.4, "XY"));
	invalid.addTransition(transition(&y, &y, 1.2, "YY"));

	errorMessage.clear();
	EXPECT_FALSE(invalid.CheckProbe(errorMessage));
	EXPECT_NE(errorMessage.find("must sum to 1.0"), std::string::npos);
	EXPECT_NE(errorMessage.find("probability must be in [0,1]"), std::string::npos);
}

TEST(MarkovChainNetworkTest, ActivationPerformsExactlyOneDeterministicStepAndPublishesStateIndex) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	MarkovChainNetwork network(model, "Chain");
	MarkovState a(model, "A");
	MarkovState b(model, "B");
	MarkovState c(model, "C");
	network.setInitialState(&a);
	network.addState(&b);
	network.addState(&c);
	network.addTransition(transition(&a, &b, 1.0, "AB"));
	network.addTransition(transition(&b, &c, 1.0, "BC"));
	network.addTransition(transition(&c, &c, 1.0, "CC"));

	NetworkActivationResult first = network.activate(NetworkActivationFrame(network.getNumInputPorts()));
	ASSERT_TRUE(first.isPresent(0));
	EXPECT_EQ(network.getCurrentState(), &b);
	EXPECT_DOUBLE_EQ(first.getValue(0), 1.0);
	EXPECT_DOUBLE_EQ(network.getActivationCount(), 1.0);

	NetworkActivationResult second = network.activate(NetworkActivationFrame(network.getNumInputPorts()));
	ASSERT_TRUE(second.isPresent(0));
	EXPECT_EQ(network.getCurrentState(), &c);
	EXPECT_DOUBLE_EQ(second.getValue(0), 2.0);
	EXPECT_DOUBLE_EQ(network.getActivationCount(), 2.0);
}

TEST(MarkovChainNetworkTest, AbsorbingStateRemainsCurrentAcrossActivations) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	MarkovChainNetwork network(model, "AbsorbingChain");
	MarkovState absorbing(model, "Absorbing");
	network.setInitialState(&absorbing);
	network.addTransition(transition(&absorbing, &absorbing, 1.0, "Stay"));

	network.activate(NetworkActivationFrame(network.getNumInputPorts()));
	network.activate(NetworkActivationFrame(network.getNumInputPorts()));

	EXPECT_EQ(network.getCurrentState(), &absorbing);
	EXPECT_DOUBLE_EQ(network.getActivationCount(), 2.0);
}

TEST(MarkovChainNetworkTest, ReplicationResetRestoresInitialStateAndSamplerSequence) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	MarkovChainNetworkProbe network(model, "Chain");
	MarkovState a(model, "A");
	MarkovState b(model, "B");
	network.setInitialState(&a);
	network.addState(&b);
	network.addTransition(transition(&a, &b, 1.0, "AB"));
	network.addTransition(transition(&b, &b, 1.0, "BB"));

	network.activate(NetworkActivationFrame(network.getNumInputPorts()));
	ASSERT_EQ(network.getCurrentState(), &b);
	ASSERT_DOUBLE_EQ(network.getActivationCount(), 1.0);

	network.InitBetweenReplicationsProbe();

	EXPECT_EQ(network.getCurrentState(), &a);
	EXPECT_DOUBLE_EQ(network.getActivationCount(), 0.0);
}

TEST(MarkovChainNetworkTest, PersistenceRoundTripPreservesStatesTransitionsAndCurrentState) {
	Simulator simulator;
	ASSERT_NE(simulator.getPluginManager()->insert("markovstate.so"), nullptr);
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	MarkovChainNetworkProbe source(model, "PersistentChain");
	source.setProbabilityTolerance(1e-8);
	MarkovState a(model, "A");
	MarkovState b(model, "B");
	MarkovState c(model, "C");
	source.setInitialState(&a);
	source.addState(&b);
	source.addState(&c);
	source.addTransition(transition(&a, &b, 1.0, "AB"));
	source.addTransition(transition(&b, &c, 1.0, "BC"));
	source.addTransition(transition(&c, &c, 1.0, "CC"));
	source.setCurrentState(&b);

	FakeModelPersistenceRuntime persistence;
	PersistenceRecord fields(persistence);
	source.SaveInstanceProbe(&fields, true);

	MarkovChainNetworkProbe loaded(model, "LoadedChain");
	ASSERT_TRUE(loaded.LoadInstanceProbe(&fields));

	EXPECT_EQ(loaded.getNumInputPorts(), 1u);
	EXPECT_EQ(loaded.getNumOutputPorts(), 1u);
	EXPECT_DOUBLE_EQ(loaded.getProbabilityTolerance(), 1e-8);
	ASSERT_EQ(loaded.getStates()->size(), 3u);
	ASSERT_EQ(loaded.getTransitions()->size(), 3u);
	ASSERT_NE(loaded.getInitialState(), nullptr);
	ASSERT_NE(loaded.getCurrentState(), nullptr);
	EXPECT_EQ(loaded.getInitialState()->getName(), "A");
	EXPECT_EQ(loaded.getCurrentState()->getName(), "B");

	auto* loadedTransition = loaded.getTransitions()->front();
	ASSERT_NE(loadedTransition, nullptr);
	EXPECT_EQ(loadedTransition->getName(), "AB");
	EXPECT_EQ(loadedTransition->getSource()->getName(), "A");
	EXPECT_EQ(loadedTransition->getDestination()->getName(), "B");
	EXPECT_DOUBLE_EQ(loadedTransition->getProbability(), 1.0);
}

TEST(MarkovChainNetworkTest, EmpiricalFrequencyUsesConfiguredProbabilities) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	MarkovChainNetwork network(model, "SamplingChain");
	MarkovState a(model, "A");
	MarkovState b(model, "B");
	MarkovState c(model, "C");
	network.setInitialState(&a);
	network.addState(&b);
	network.addState(&c);
	network.addTransition(transition(&a, &b, 0.25, "AB"));
	network.addTransition(transition(&a, &c, 0.75, "AC"));
	network.addTransition(transition(&b, &b, 1.0, "BB"));
	network.addTransition(transition(&c, &c, 1.0, "CC"));

	unsigned int bCount = 0;
	unsigned int cCount = 0;
	const unsigned int samples = 1000;
	for (unsigned int i = 0; i < samples; i++) {
		network.setCurrentState(&a);
		network.activate(NetworkActivationFrame(network.getNumInputPorts()));
		if (network.getCurrentState() == &b) {
			bCount++;
		} else if (network.getCurrentState() == &c) {
			cCount++;
		}
	}

	const double bFrequency = static_cast<double>(bCount) / static_cast<double>(samples);
	const double cFrequency = static_cast<double>(cCount) / static_cast<double>(samples);
	EXPECT_GT(bFrequency, 0.18);
	EXPECT_LT(bFrequency, 0.32);
	EXPECT_GT(cFrequency, 0.68);
	EXPECT_LT(cFrequency, 0.82);
}

TEST(MarkovChainNetworkTest, SharedChainActivatedThroughTwoModalModelsKeepsNetworkOwnedState) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	MarkovChainNetwork network(model, "SharedChain");
	MarkovState a(model, "A");
	MarkovState b(model, "B");
	MarkovState c(model, "C");
	network.setInitialState(&a);
	network.addState(&b);
	network.addState(&c);
	network.addTransition(transition(&a, &b, 1.0, "AB"));
	network.addTransition(transition(&b, &c, 1.0, "BC"));
	network.addTransition(transition(&c, &c, 1.0, "CC"));
	Attribute stateAttribute(model, "SharedState");

	ModalModelDefaultProbe modalA(model, "ModalA");
	ModalModelDefaultProbe modalB(model, "ModalB");
	CollectorSinkComponentProbe sinkA(model, "SinkA");
	CollectorSinkComponentProbe sinkB(model, "SinkB");
	modalA.getConnectionManager()->insert(&sinkA);
	modalB.getConnectionManager()->insert(&sinkB);
	modalA.setNetwork(&network);
	modalB.setNetworkName("SharedChain");
	modalA.setOutputBinding(0, "SharedState");
	modalB.setOutputBinding(0, "SharedState");

	Entity* entityA = model->createEntity("EntityA", true);
	Entity* entityB = model->createEntity("EntityB", true);
	modalA.dispatch(entityA);
	modalB.dispatch(entityB);
	drainFutureEvents(model);

	EXPECT_EQ(network.getCurrentState(), &c);
	EXPECT_DOUBLE_EQ(network.getActivationCount(), 2.0);
	ASSERT_EQ(sinkA.receivedEntities().size(), 1u);
	ASSERT_EQ(sinkB.receivedEntities().size(), 1u);
	EXPECT_DOUBLE_EQ(entityA->getAttributeValue("SharedState"), 1.0);
	EXPECT_DOUBLE_EQ(entityB->getAttributeValue("SharedState"), 2.0);
	EXPECT_EQ(modalB.getNetwork(), &network);
	(void)stateAttribute;
}

TEST(MarkovChainNetworkTest, PluginRegistrationCreatesMarkovNetworkDataDefinitions) {
	Simulator simulator;
	Plugin* statePlugin = simulator.getPluginManager()->insert("markovstate.so");
	Plugin* networkPlugin = simulator.getPluginManager()->insert("markovchainnetwork.so");
	ASSERT_NE(statePlugin, nullptr);
	ASSERT_NE(networkPlugin, nullptr);
	Model* model = simulator.getModelManager()->newModel();

	ModelDataDefinition* state = statePlugin->newInstance(model, "S");
	ModelDataDefinition* network = networkPlugin->newInstance(model, "Chain");

	EXPECT_NE(dynamic_cast<MarkovState*>(state), nullptr);
	EXPECT_NE(dynamic_cast<MarkovChainNetwork*>(network), nullptr);

	std::unique_ptr<PluginInformation> info(MarkovChainNetwork::GetPluginInformation());
	ASSERT_NE(info, nullptr);
	EXPECT_FALSE(info->isComponent());
	EXPECT_EQ(info->getPluginTypename(), Util::TypeOf<MarkovChainNetwork>());
	EXPECT_EQ(info->getCategory(), "ModalModel");
	ASSERT_NE(info->getDataDefinitionLoader(), nullptr);
	ASSERT_NE(info->getDataDefinitionConstructor(), nullptr);
}
