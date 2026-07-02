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

	bool LoadInstanceProbe(PersistenceRecord* fields) {
		return _loadInstance(fields);
	}

	void DispatchProbe(Entity* entity, unsigned int inputPortNumber) {
		_onDispatchEvent(entity, inputPortNumber);
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

	const std::vector<unsigned int>& inputPorts() const {
		return _inputPorts;
	}

protected:
	void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override {
		_received.push_back(entity);
		_inputPorts.push_back(inputPortNumber);
	}

	bool _loadInstance(PersistenceRecord* fields) override {
		return ModelComponent::_loadInstance(fields);
	}

	void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override {
		ModelComponent::_saveInstance(fields, saveDefaultValues);
	}

private:
	std::vector<Entity*> _received;
	std::vector<unsigned int> _inputPorts;
};

TEST(ModalModelEFSMTest, ModalModelFSMKeepsFSMClassname) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	ModalModelFSM modal(model, "TypedFSM");

	EXPECT_EQ(modal.getClassname(), Util::TypeOf<ModalModelFSM>());
	EXPECT_NE(modal.getClassname(), Util::TypeOf<ModalModelDefault>());
}

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

TEST(ModalModelEFSMTest, DispatchUpdatesStateAndSchedulesOutput) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	ModalModelFSMProbe modal(model, "DoorFSM");
	FSMState idle(model, "Idle");
	FSMState active(model, "Active");
	ModalCollectorSinkProbe sink(model, "Sink");
	DefaultNodeTransition open(&idle, &active, "Open");

	idle.setInitialNode(true);
	open.setInputEvent("1");
	open.setGuardExpression("1");

	modal.addNode(&active);
	modal.addNode(&idle);
	modal.setEntryNode(&idle);
	modal.addTransition(&open);
	modal.setMaxTransitionsPerDispatch(1);
	modal.connectTo(&sink);
	modal.CreateAttachedAttributesProbe();

	Entity* entity = model->createEntity("Order", true);
	ASSERT_NE(entity, nullptr);

	modal.DispatchProbe(entity, 1);

	ASSERT_EQ(model->getFutureEvents()->size(), 1u);
	Event* nextEvent = model->getFutureEvents()->front();
	ASSERT_NE(nextEvent, nullptr);
	EXPECT_EQ(nextEvent->getEntity(), entity);
	EXPECT_EQ(nextEvent->getComponent(), &sink);
	EXPECT_EQ(nextEvent->getComponentinputPortNumber(), 0u);
	EXPECT_DOUBLE_EQ(entity->getAttributeValue("Entity.ModalModel.DoorFSM.CurrentNode"), 0.0);
	EXPECT_DOUBLE_EQ(entity->getAttributeValue("Entity.ModalModel.DoorFSM.LastNode"), static_cast<double>(active.getId()));
}

TEST(ModalModelEFSMTest, DispatchToNonFinalStateUsesOutputPortZero) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	ModalModelFSMProbe modal(model, "NonFinalFSM");
	FSMState idle(model, "Idle");
	FSMState active(model, "Active");
	ModalCollectorSinkProbe normalSink(model, "NormalSink");
	ModalCollectorSinkProbe finalSink(model, "FinalSink");
	DefaultNodeTransition open(&idle, &active, "Open");

	idle.setInitialNode(true);
	open.setGuardExpression("1");

	modal.addNode(&idle);
	modal.addNode(&active);
	modal.setEntryNode(&idle);
	modal.addTransition(&open);
	modal.getConnectionManager()->insertAtPort(0, new Connection({&normalSink, {0}}));
	modal.getConnectionManager()->insertAtPort(1, new Connection({&finalSink, {1}}));
	modal.CreateAttachedAttributesProbe();

	Entity* entity = model->createEntity("Order", true);
	ASSERT_NE(entity, nullptr);

	modal.DispatchProbe(entity, 1);

	ASSERT_EQ(model->getFutureEvents()->size(), 1u);
	Event* nextEvent = model->getFutureEvents()->front();
	ASSERT_NE(nextEvent, nullptr);
	EXPECT_EQ(nextEvent->getEntity(), entity);
	EXPECT_EQ(nextEvent->getComponent(), &normalSink);
	EXPECT_EQ(nextEvent->getComponentinputPortNumber(), 0u);
}

TEST(ModalModelEFSMTest, DispatchToFinalStateUsesOutputPortOneAndStops) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	ModalModelFSMProbe modal(model, "FinalFSM");
	FSMState start(model, "Start");
	FSMState done(model, "Done");
	ModalCollectorSinkProbe normalSink(model, "NormalSink");
	ModalCollectorSinkProbe finalSink(model, "FinalSink");
	DefaultNodeTransition finish(&start, &done, "Finish");
	DefaultNodeTransition shouldNotFire(&done, &start, "ShouldNotFire");

	start.setInitialNode(true);
	done.setFinalNode(true);
	finish.setGuardExpression("1");
	shouldNotFire.setGuardExpression("1");

	modal.addNode(&start);
	modal.addNode(&done);
	modal.setEntryNode(&start);
	modal.addTransition(&finish);
	modal.addTransition(&shouldNotFire);
	modal.setMaxTransitionsPerDispatch(5);
	modal.getConnectionManager()->insertAtPort(0, new Connection({&normalSink, {0}}));
	modal.getConnectionManager()->insertAtPort(1, new Connection({&finalSink, {1}}));
	modal.CreateAttachedAttributesProbe();

	Entity* entity = model->createEntity("Order", true);
	ASSERT_NE(entity, nullptr);

	modal.DispatchProbe(entity, 1);

	ASSERT_EQ(model->getFutureEvents()->size(), 1u);
	Event* nextEvent = model->getFutureEvents()->front();
	ASSERT_NE(nextEvent, nullptr);
	EXPECT_EQ(nextEvent->getEntity(), entity);
	EXPECT_EQ(nextEvent->getComponent(), &finalSink);
	EXPECT_EQ(nextEvent->getComponentinputPortNumber(), 1u);
	EXPECT_DOUBLE_EQ(entity->getAttributeValue("Entity.ModalModel.FinalFSM.CurrentNode"), 1.0);
	EXPECT_DOUBLE_EQ(entity->getAttributeValue("Entity.ModalModel.FinalFSM.LastNode"), static_cast<double>(done.getId()));
	EXPECT_EQ(modal.getCurrentNode(), &done);
}

TEST(ModalModelEFSMTest, DispatchStartingAtFinalStateDoesNotFireOutgoingTransition) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	ModalModelFSMProbe modal(model, "AlreadyFinalFSM");
	FSMState done(model, "Done");
	FSMState next(model, "Next");
	ModalCollectorSinkProbe normalSink(model, "NormalSink");
	ModalCollectorSinkProbe finalSink(model, "FinalSink");
	DefaultNodeTransition shouldNotFire(&done, &next, "ShouldNotFire");

	done.setInitialNode(true);
	done.setFinalNode(true);
	shouldNotFire.setGuardExpression("1");

	modal.addNode(&done);
	modal.addNode(&next);
	modal.setEntryNode(&done);
	modal.addTransition(&shouldNotFire);
	modal.setMaxTransitionsPerDispatch(5);
	modal.getConnectionManager()->insertAtPort(0, new Connection({&normalSink, {0}}));
	modal.getConnectionManager()->insertAtPort(1, new Connection({&finalSink, {1}}));
	modal.CreateAttachedAttributesProbe();

	Entity* entity = model->createEntity("Order", true);
	ASSERT_NE(entity, nullptr);

	modal.DispatchProbe(entity, 1);

	ASSERT_EQ(model->getFutureEvents()->size(), 1u);
	Event* nextEvent = model->getFutureEvents()->front();
	ASSERT_NE(nextEvent, nullptr);
	EXPECT_EQ(nextEvent->getEntity(), entity);
	EXPECT_EQ(nextEvent->getComponent(), &finalSink);
	EXPECT_EQ(nextEvent->getComponentinputPortNumber(), 1u);
	EXPECT_DOUBLE_EQ(entity->getAttributeValue("Entity.ModalModel.AlreadyFinalFSM.CurrentNode"), 0.0);
	EXPECT_EQ(modal.getCurrentNode(), &done);
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

	EXPECT_EQ(fields.loadField("typename", ""), Util::TypeOf<ModalModelFSM>());
	EXPECT_EQ(fields.loadField("transitionTypename[0]", ""), Util::TypeOf<EFSMTransition>());
	EXPECT_EQ(fields.loadField("transitionInputEvent[0]", ""), "1");
	EXPECT_EQ(fields.loadField("transitionTriggerEvent[0]", ""), "1");
	EXPECT_EQ(fields.loadField("transitionProbabilityExpression[0]", ""), "0.75");
}

TEST(ModalModelEFSMTest, LoadAndSavePreservesFSMNodesEntryAndEFSMTransitionFields) {
	Simulator simulator;
	ASSERT_NE(simulator.getPluginManager()->insert("fsmstate.so"), nullptr);
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	ModalModelFSMProbe original(model, "PersistedFSM");
	FSMState idle(model, "Idle");
	FSMState active(model, "Active");
	EFSMTransition open(&idle, &active, "Open");

	idle.setInitialNode(true);
	active.setFinalNode(true);
	active.setEntryActionExpression("x=x+1");
	open.setTriggerEvent("2");
	open.setGuardExpression("1==1");
	open.setOutputExpression("y=y+1");
	open.setProbabilityExpression("0.25");
	open.setPriority(3);
	open.setProbability(0.5);
	open.setTransitionKind(DefaultNodeTransition::TransitionKind::PROBABILISTIC);

	original.addNode(&idle);
	original.addNode(&active);
	original.setEntryNode(&idle);
	original.addTransition(&open);

	FakeModalPersistence persistence;
	PersistenceRecord saved(persistence);
	original.SaveInstanceProbe(&saved, true);

	ModalModelFSMProbe loaded(model, "LoadedFSM");
	ASSERT_TRUE(loaded.LoadInstanceProbe(&saved));

	ASSERT_EQ(loaded.getNodes()->size(), 2u);
	ASSERT_EQ(loaded.getTransitions()->size(), 1u);
	EXPECT_EQ(loaded.getClassname(), Util::TypeOf<ModalModelFSM>());
	ASSERT_NE(loaded.getEntryNode(), nullptr);
	EXPECT_EQ(loaded.getEntryNode()->getName(), "Idle");

	FSMState* loadedIdle = dynamic_cast<FSMState*>(loaded.getNodes()->getAtRank(0));
	FSMState* loadedActive = dynamic_cast<FSMState*>(loaded.getNodes()->getAtRank(1));
	ASSERT_NE(loadedIdle, nullptr);
	ASSERT_NE(loadedActive, nullptr);
	EXPECT_TRUE(loadedIdle->isInitialNode());
	EXPECT_TRUE(loadedActive->isFinalNode());
	EXPECT_EQ(loadedActive->getEntryActionExpression(), "x=x+1");

	EFSMTransition* loadedOpen = dynamic_cast<EFSMTransition*>(loaded.getTransitions()->front());
	ASSERT_NE(loadedOpen, nullptr);
	EXPECT_EQ(loadedOpen->getSource()->getName(), "Idle");
	EXPECT_EQ(loadedOpen->getDestination()->getName(), "Active");
	EXPECT_EQ(loadedOpen->getTriggerEvent(), "2");
	EXPECT_EQ(loadedOpen->getInputEvent(), "2");
	EXPECT_EQ(loadedOpen->getGuardExpression(), "1==1");
	EXPECT_EQ(loadedOpen->getOutputExpression(), "y=y+1");
	EXPECT_EQ(loadedOpen->getProbabilityExpression(), "0.25");
	EXPECT_EQ(loadedOpen->getPriority(), 3u);
	EXPECT_DOUBLE_EQ(loadedOpen->getProbability(), 0.5);
	EXPECT_EQ(loadedOpen->getTransitionKind(), DefaultNodeTransition::TransitionKind::PROBABILISTIC);

	PersistenceRecord resaved(persistence);
	loaded.SaveInstanceProbe(&resaved, true);
	EXPECT_EQ(resaved.loadField("typename", ""), Util::TypeOf<ModalModelFSM>());
	EXPECT_EQ(resaved.loadField("node[0].typename", ""), Util::TypeOf<FSMState>());
	EXPECT_EQ(resaved.loadField("entryNode", ""), "Idle");
	EXPECT_EQ(resaved.loadField("transitionTypename[0]", ""), Util::TypeOf<EFSMTransition>());
	EXPECT_EQ(resaved.loadField("transitionTriggerEvent[0]", ""), "2");
	EXPECT_EQ(resaved.loadField("transitionProbabilityExpression[0]", ""), "0.25");
}
