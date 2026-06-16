#include <gtest/gtest.h>
#include <vector>

#include "kernel/simulator/Event.h"
#include "kernel/simulator/Persistence.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/essentialPlugins/Attribute.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelComponent.h"
#include "plugins/components/ModalModel/DefaultNode.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "plugins/components/ModalModel/FSMState.h"
#include "plugins/components/ModalModel/ModalModelFSM.h"

class ModalModelFSMProbe : public ModalModelFSM {
public:
	ModalModelFSMProbe(Model* model, const std::string& name = "") : ModalModelFSM(model, name) {}

	bool CheckProbe(std::string& errorMessage) {
		return _check(errorMessage);
	}

	void CreateAttachedAttributesProbe() {
		_createAttachedAttributes();
	}

	void SaveInstanceProbe(PersistenceRecord* fields, bool saveDefaultValues = true) {
		_saveInstance(fields, saveDefaultValues);
	}
};

class FakeModalPersistence : public Persistence_if {
public:
	bool save(std::string) override { return false; }
	bool load(std::string) override { return false; }
	bool hasChanged() override { return false; }
	void setHasChanged(bool) override {}
	bool getOption(Persistence_if::Options) override { return false; }
	void setOption(Persistence_if::Options, bool) override {}
	std::string getFormatedField(PersistenceRecord*) override { return ""; }
};

class ModalCollectorSinkProbe : public ModelComponent {
public:
	ModalCollectorSinkProbe(Model* model, const std::string& name = "")
		: ModelComponent(model, "ModalCollectorSinkProbe", name) {}

	const std::vector<Entity*>& received() const {
		return _received;
	}

protected:
	void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override {
		(void)inputPortNumber;
		_received.push_back(entity);
	}

	bool _loadInstance(PersistenceRecord* fields) override {
		return ModelComponent::_loadInstance(fields);
	}

	void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override {
		ModelComponent::_saveInstance(fields, saveDefaultValues);
	}

private:
	std::vector<Entity*> _received;
};

TEST(ModalModelEFSMTest, TransitionRequiresMatchingInputEventAndGuard) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	FSMState source(model, "Source");
	FSMState destination(model, "Destination");
	DefaultNodeTransition transition(&source, &destination, "Go");

	transition.setInputEvent("2");
	transition.setGuardExpression("1==1");
	EXPECT_FALSE(transition.canFire(model, nullptr, "1"));
	EXPECT_TRUE(transition.canFire(model, nullptr, "2"));

	transition.setGuardExpression("0");
	EXPECT_FALSE(transition.canFire(model, nullptr, "2"));
}

TEST(ModalModelEFSMTest, DispatchExecutesExitTransitionAndEntryActionsAndUpdatesState) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	auto* guard = new Attribute(model, "GuardFlag");
	auto* entryCount = new Attribute(model, "EntryCount");
	auto* exitCount = new Attribute(model, "ExitCount");
	auto* transitionCount = new Attribute(model, "TransitionCount");
	ASSERT_NE(guard, nullptr);
	ASSERT_NE(entryCount, nullptr);
	ASSERT_NE(exitCount, nullptr);
	ASSERT_NE(transitionCount, nullptr);

	ModalModelFSMProbe modal(model, "DoorFSM");
	FSMState idle(model, "Idle");
	FSMState active(model, "Active");
	ModalCollectorSinkProbe sink(model, "Sink");
	DefaultNodeTransition open(&idle, &active, "Open");

	idle.setInitialNode(true);
	idle.setExitActionExpression("ExitCount=ExitCount+1");
	active.setEntryActionExpression("EntryCount=EntryCount+1");
	open.setInputEvent("1");
	open.setGuardExpression("GuardFlag==1");
	open.setOutputExpression("TransitionCount=TransitionCount+1");

	modal.addNode(&active);
	modal.addNode(&idle);
	modal.setEntryNode(&idle);
	modal.addTransition(&open);
	modal.setMaxTransitionsPerDispatch(1);
	modal.connectTo(&sink);
	modal.CreateAttachedAttributesProbe();

	Entity* entity = model->createEntity("Order", true);
	ASSERT_NE(entity, nullptr);
	entity->setAttributeValue("GuardFlag", 1.0);
	entity->setAttributeValue("EntryCount", 0.0);
	entity->setAttributeValue("ExitCount", 0.0);
	entity->setAttributeValue("TransitionCount", 0.0);

	model->getFutureEvents()->insert(new Event(0.0, entity, &modal, 1));
	model->getSimulation()->setReplicationLength(1.0);
	model->getSimulation()->start();

	ASSERT_EQ(sink.received().size(), 1u);
	EXPECT_EQ(sink.received().front(), entity);
	EXPECT_DOUBLE_EQ(entity->getAttributeValue("ExitCount"), 1.0);
	EXPECT_DOUBLE_EQ(entity->getAttributeValue("TransitionCount"), 1.0);
	EXPECT_DOUBLE_EQ(entity->getAttributeValue("EntryCount"), 1.0);
	EXPECT_DOUBLE_EQ(entity->getAttributeValue("Entity.ModalModel.DoorFSM.CurrentNode"), 0.0);
	EXPECT_DOUBLE_EQ(entity->getAttributeValue("Entity.ModalModel.DoorFSM.LastNode"), static_cast<double>(active.getId()));
}

TEST(ModalModelEFSMTest, FSMCheckRequiresFSMStatesAndInitialState) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	ModalModelFSMProbe modal(model, "CheckedFSM");
	FSMState state(model, "State");
	modal.addNode(&state);
	modal.setEntryNode(&state);

	std::string errorMessage;
	EXPECT_FALSE(modal.CheckProbe(errorMessage));
	EXPECT_NE(errorMessage.find("initial state"), std::string::npos);

	state.setInitialNode(true);
	errorMessage.clear();
	EXPECT_TRUE(modal.CheckProbe(errorMessage));
}

TEST(ModalModelEFSMTest, SavePersistsEFSMTransitionSpecificFields) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	ModalModelFSMProbe modal(model, "PersistedFSM");
	FSMState idle(model, "Idle");
	FSMState active(model, "Active");
	EFSMTransition open(&idle, &active, "Open");

	idle.setInitialNode(true);
	open.setTriggerEvent("1");
	open.setGuardExpression("1==1");
	open.setProbabilityExpression("0.75");
	open.setTransitionKind(DefaultNodeTransition::TransitionKind::PROBABILISTIC);

	modal.addNode(&idle);
	modal.addNode(&active);
	modal.setEntryNode(&idle);
	modal.addTransition(&open);

	FakeModalPersistence persistence;
	PersistenceRecord fields(persistence);
	modal.SaveInstanceProbe(&fields, true);

	EXPECT_EQ(fields.loadField("transitionTypename0", ""), Util::TypeOf<EFSMTransition>());
	EXPECT_EQ(fields.loadField("transitionInputEvent0", ""), "1");
	EXPECT_EQ(fields.loadField("transitionTriggerEvent0", ""), "1");
	EXPECT_EQ(fields.loadField("transitionProbabilityExpression0", ""), "0.75");
}
