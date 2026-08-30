// Unit tests for the generic ModalModelDefault -> DefaultNetwork bridge.

#include <gtest/gtest.h>

#include "kernel/simulator/Event.h"
#include "kernel/simulator/Persistence.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/essentialPlugins/Attribute.h"
#include "kernel/simulator/essentialPlugins/Entity.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelComponent.h"
#include "plugins/components/ModalModel/ModalModelDefault.h"
#include "plugins/data/ModalModel/DefaultNetwork.h"
#include "plugins/data/ModalModel/NetworkActivation.h"

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

class TestNetwork final : public DefaultNetwork {
public:
	TestNetwork(Model* model, const std::string& name) : DefaultNetwork(model, name, "TestNetwork") {}

	void setOutput(unsigned int port, bool present, double value = 0.0) {
		if (_outputs.size() <= port) {
			_outputs.resize(port + 1);
		}
		_outputs[port] = {present, value};
	}

	const NetworkActivationFrame& lastFrame() const {
		return _lastFrame;
	}

protected:
	NetworkActivationResult _activate(const NetworkActivationFrame& frame) override {
		_lastFrame = frame;
		NetworkActivationResult result(getNumOutputPorts());
		for (unsigned int i = 0; i < _outputs.size() && i < result.size(); i++) {
			if (_outputs[i].present) {
				result.setPresent(i, _outputs[i].value);
			}
		}
		return result;
	}

private:
	NetworkActivationFrame _lastFrame;
	std::vector<NetworkPortValue> _outputs;
};

class ModalModelDefaultProbe final : public ModalModelDefault {
public:
	ModalModelDefaultProbe(Model* model, const std::string& name) : ModalModelDefault(model, name) {}

	void dispatch(Entity* entity, unsigned int inputPortNumber = 0) {
		_onDispatchEvent(entity, inputPortNumber);
	}

	bool checkProbe(std::string& errorMessage) {
		return _check(errorMessage);
	}

	bool loadProbe(PersistenceRecord* fields) {
		return _loadInstance(fields);
	}

	void saveProbe(PersistenceRecord* fields, bool saveDefaultValues = true) {
		_saveInstance(fields, saveDefaultValues);
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

} // namespace

TEST(ModalModelDefaultNetworkTest, ActivatesAttachedNetworkFromArrivingInputPort) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	TestNetwork network(model, "BridgeNetwork");
	network.addInputPort("Trigger");
	network.addOutputPort("Result");
	network.setOutput(0, true, 42.0);
	Attribute resultAttribute(model, "ResultValue");

	ModalModelDefaultProbe modal(model, "ModalBridge");
	CollectorSinkComponentProbe sink(model, "Sink");
	modal.getConnectionManager()->insert(&sink);
	modal.setNetwork(&network);
	modal.setInputBinding(0, "7");
	modal.setOutputBinding(0, "ResultValue");

	std::string errorMessage;
	ASSERT_TRUE(modal.checkProbe(errorMessage)) << errorMessage;

	Entity* entity = model->createEntity("TriggerEntity", true);
	modal.dispatch(entity, 0);
	drainFutureEvents(model);

	EXPECT_TRUE(network.lastFrame().isPresent(0));
	EXPECT_DOUBLE_EQ(network.lastFrame().getValue(0), 7.0);
	EXPECT_DOUBLE_EQ(network.getActivationCount(), 1.0);
	ASSERT_EQ(sink.receivedEntities().size(), 1u);
	EXPECT_EQ(sink.receivedEntities().front(), entity);
	EXPECT_DOUBLE_EQ(entity->getAttributeValue("ResultValue"), 42.0);
}

TEST(ModalModelDefaultNetworkTest, ConsumesEntityWhenNoOutputIsPresent) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	TestNetwork network(model, "NoOutputNetwork");
	network.addInputPort("Trigger");
	network.addOutputPort("Result");
	ModalModelDefaultProbe modal(model, "NoOutputModal");
	modal.setNetwork(&network);

	Entity* entity = model->createEntity("ConsumedEntity", true);
	ASSERT_NE(model->getDataManager()->getDataDefinition(Util::TypeOf<Entity>(), entity->getName()), nullptr);

	modal.dispatch(entity, 0);

	EXPECT_TRUE(model->getFutureEvents()->empty());
	EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<Entity>(), "ConsumedEntity"), nullptr);
	EXPECT_DOUBLE_EQ(network.getActivationCount(), 1.0);
}

TEST(ModalModelDefaultNetworkTest, MultiplePresentOutputsCloneBeforeOutputSpecificMutation) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	TestNetwork network(model, "SplitNetwork");
	network.addInputPort("Trigger");
	network.addOutputPort("OutA");
	network.addOutputPort("OutB");
	network.setOutput(0, true, 10.0);
	network.setOutput(1, true, 20.0);
	Attribute outA(model, "OutAValue");
	Attribute outB(model, "OutBValue");

	ModalModelDefaultProbe modal(model, "SplitModal");
	CollectorSinkComponentProbe sinkA(model, "SinkA");
	CollectorSinkComponentProbe sinkB(model, "SinkB");
	modal.getConnectionManager()->insert(&sinkA);
	modal.getConnectionManager()->insert(&sinkB);
	modal.setNetwork(&network);
	modal.setOutputBinding(0, "OutAValue");
	modal.setOutputBinding(1, "OutBValue");

	Entity* entity = model->createEntity("SplitEntity", true);
	modal.dispatch(entity, 0);
	drainFutureEvents(model);

	ASSERT_EQ(sinkA.receivedEntities().size(), 1u);
	ASSERT_EQ(sinkB.receivedEntities().size(), 1u);
	Entity* cloneForA = sinkA.receivedEntities().front();
	Entity* originalForB = sinkB.receivedEntities().front();
	EXPECT_NE(cloneForA, originalForB);
	EXPECT_EQ(originalForB, entity);
	EXPECT_DOUBLE_EQ(cloneForA->getAttributeValue("OutAValue"), 10.0);
	EXPECT_DOUBLE_EQ(cloneForA->getAttributeValue("OutBValue"), 0.0);
	EXPECT_DOUBLE_EQ(originalForB->getAttributeValue("OutAValue"), 0.0);
	EXPECT_DOUBLE_EQ(originalForB->getAttributeValue("OutBValue"), 20.0);
}

TEST(ModalModelDefaultNetworkTest, PersistenceRoundTripPreservesNetworkReferenceAndBindings) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	TestNetwork network(model, "PersistentNetwork");
	network.addInputPort("Trigger");
	network.addOutputPort("Result");

	ModalModelDefaultProbe source(model, "SourceModal");
	source.setNetwork(&network);
	source.setInputBinding(0, "3");
	source.setOutputBinding(0, "PersistedResult");

	FakeModelPersistenceRuntime persistence;
	PersistenceRecord fields(persistence);
	source.saveProbe(&fields);

	ModalModelDefaultProbe loaded(model, "LoadedModal");
	ASSERT_TRUE(loaded.loadProbe(&fields));

	EXPECT_EQ(loaded.getNetwork(), &network);
	EXPECT_EQ(loaded.getNetworkName(), "PersistentNetwork");
	EXPECT_EQ(loaded.getInputBinding(0), "3");
	EXPECT_EQ(loaded.getOutputBinding(0), "PersistedResult");
}

TEST(ModalModelDefaultNetworkTest, TwoModalModelsCanShareOneNetworkInstance) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	TestNetwork network(model, "SharedNetwork");
	network.addInputPort("Trigger");
	network.addOutputPort("Result");
	network.setOutput(0, true, 5.0);
	Attribute resultAttribute(model, "SharedResult");

	ModalModelDefaultProbe modalA(model, "ModalA");
	ModalModelDefaultProbe modalB(model, "ModalB");
	CollectorSinkComponentProbe sinkA(model, "SinkA");
	CollectorSinkComponentProbe sinkB(model, "SinkB");
	modalA.getConnectionManager()->insert(&sinkA);
	modalB.getConnectionManager()->insert(&sinkB);
	modalA.setNetwork(&network);
	modalB.setNetworkName("SharedNetwork");
	modalA.setOutputBinding(0, "SharedResult");
	modalB.setOutputBinding(0, "SharedResult");

	Entity* entityA = model->createEntity("EntityA", true);
	Entity* entityB = model->createEntity("EntityB", true);
	modalA.dispatch(entityA, 0);
	modalB.dispatch(entityB, 0);
	drainFutureEvents(model);

	EXPECT_EQ(modalB.getNetwork(), &network);
	EXPECT_DOUBLE_EQ(network.getActivationCount(), 2.0);
	ASSERT_EQ(sinkA.receivedEntities().size(), 1u);
	ASSERT_EQ(sinkB.receivedEntities().size(), 1u);
	EXPECT_EQ(sinkA.receivedEntities().front(), entityA);
	EXPECT_EQ(sinkB.receivedEntities().front(), entityB);
	EXPECT_DOUBLE_EQ(entityA->getAttributeValue("SharedResult"), 5.0);
	EXPECT_DOUBLE_EQ(entityB->getAttributeValue("SharedResult"), 5.0);
}

TEST(ModalModelDefaultNetworkTest, CheckRejectsUnknownNetworkReference) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	ModalModelDefaultProbe modal(model, "BrokenModal");
	modal.setNetworkName("MissingNetwork");

	std::string errorMessage;
	EXPECT_FALSE(modal.checkProbe(errorMessage));
	EXPECT_NE(errorMessage.find("MissingNetwork"), std::string::npos);
}
