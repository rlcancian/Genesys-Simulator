// Unit tests for the fixed-inscription ColoredPetriNetNetwork subset.

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
#include "plugins/components/ModalModel/PetriPlace.h"
#include "plugins/data/ModalModel/CPNArc.h"
#include "plugins/data/ModalModel/CPNTransition.h"
#include "plugins/data/ModalModel/ColoredPetriNetNetwork.h"
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

class ColoredPetriNetNetworkProbe : public ColoredPetriNetNetwork {
public:
	ColoredPetriNetNetworkProbe(Model* model, const std::string& name = "") : ColoredPetriNetNetwork(model, name) {}
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

CPNArc* inputArc(Model* model, PetriPlace* place, CPNTransition* transition, unsigned int quantity, const std::string& color, const std::string& name) {
	auto* arc = new CPNArc(model, place, transition, CPNArc::Direction::PlaceToTransition, name);
	arc->setInscription(color, quantity);
	return arc;
}

CPNArc* outputArc(Model* model, CPNTransition* transition, PetriPlace* place, unsigned int quantity, const std::string& color, const std::string& name) {
	auto* arc = new CPNArc(model, place, transition, CPNArc::Direction::TransitionToPlace, name);
	arc->setInscription(color, quantity);
	return arc;
}

} // namespace

TEST(ColoredPetriNetNetworkTest, ConstructionCreatesFireAndFiredPorts) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	ColoredPetriNetNetwork network(model, "CPN");

	EXPECT_EQ(network.getNumInputPorts(), 1u);
	EXPECT_EQ(network.getInputPortName(0), "fire");
	EXPECT_EQ(network.getNumOutputPorts(), 1u);
	EXPECT_EQ(network.getOutputPortName(0), "fired");
	EXPECT_EQ(network.getFiringMode(), ColoredPetriNetNetwork::FiringMode::SingleDeterministic);
}

TEST(ColoredPetriNetNetworkTest, BipartiteArcRegistrationRejectsUnknownEndpoints) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	ColoredPetriNetNetwork network(model, "CPN");
	PetriPlace p(model, "P");
	CPNTransition t(model, "T");
	CPNArc arc(model, &p, &t, CPNArc::Direction::PlaceToTransition, "PT");
	arc.setInscription("red", 1);

	EXPECT_FALSE(network.addArc(&arc));
	network.addPlace(&p);
	EXPECT_FALSE(network.addArc(&arc));
	network.addTransition(&t);
	EXPECT_TRUE(network.addArc(&arc));
	EXPECT_EQ(network.getArcs()->size(), 1u);
}

TEST(ColoredPetriNetNetworkTest, FixedInscriptionsEnableAndFireAtomically) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	ColoredPetriNetNetwork network(model, "CPN");
	PetriPlace input(model, "Input");
	PetriPlace output(model, "Output");
	CPNTransition transition(model, "MoveRed");
	network.addPlace(&input);
	network.addPlace(&output);
	network.addTransition(&transition);
	network.setInitialTokens(&input, "red", 2);
	network.setInitialTokens(&output, "blue", 0);
	auto* in = inputArc(model, &input, &transition, 2, "red", "In");
	auto* out = outputArc(model, &transition, &output, 1, "blue", "Out");
	ASSERT_TRUE(network.addArc(in));
	ASSERT_TRUE(network.addArc(out));

	ASSERT_TRUE(network.isEnabled(&transition));
	NetworkActivationResult result = network.activate(NetworkActivationFrame(network.getNumInputPorts()));

	ASSERT_TRUE(result.isPresent(0));
	EXPECT_DOUBLE_EQ(result.getValue(0), 1.0);
	EXPECT_EQ(input.getTokens("red"), 0u);
	EXPECT_EQ(output.getTokens("blue"), 1u);
	EXPECT_FALSE(network.isEnabled(&transition));
	EXPECT_DOUBLE_EQ(network.getActivationCount(), 1.0);
}

TEST(ColoredPetriNetNetworkTest, InsufficientTokensDoNotConsumePartialMarking) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	ColoredPetriNetNetwork network(model, "CPN");
	PetriPlace p1(model, "P1");
	PetriPlace p2(model, "P2");
	PetriPlace outPlace(model, "Out");
	CPNTransition transition(model, "NeedsBoth");
	network.addPlace(&p1);
	network.addPlace(&p2);
	network.addPlace(&outPlace);
	network.addTransition(&transition);
	p1.setTokens(1, "red");
	p2.setTokens(0, "red");
	ASSERT_TRUE(network.addArc(inputArc(model, &p1, &transition, 1, "red", "P1T")));
	ASSERT_TRUE(network.addArc(inputArc(model, &p2, &transition, 1, "red", "P2T")));
	ASSERT_TRUE(network.addArc(outputArc(model, &transition, &outPlace, 1, "red", "TOut")));

	EXPECT_FALSE(network.isEnabled(&transition));
	NetworkActivationResult result = network.activate(NetworkActivationFrame(network.getNumInputPorts()));

	EXPECT_FALSE(result.isPresent(0));
	EXPECT_EQ(p1.getTokens("red"), 1u);
	EXPECT_EQ(p2.getTokens("red"), 0u);
	EXPECT_EQ(outPlace.getTokens("red"), 0u);
}

TEST(ColoredPetriNetNetworkTest, GuardAndPrioritySelectFirstEnabledTransitionDeterministically) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	ColoredPetriNetNetwork network(model, "CPN");
	PetriPlace input(model, "Input");
	PetriPlace slowOut(model, "SlowOut");
	PetriPlace fastOut(model, "FastOut");
	CPNTransition slow(model, "Slow");
	CPNTransition fast(model, "Fast");
	slow.setPriority(10);
	fast.setPriority(1);
	slow.setGuardExpression("1");
	fast.setGuardExpression("1");
	network.addPlace(&input);
	network.addPlace(&slowOut);
	network.addPlace(&fastOut);
	network.addTransition(&slow);
	network.addTransition(&fast);
	input.setTokens(2, "red");
	ASSERT_TRUE(network.addArc(inputArc(model, &input, &slow, 1, "red", "InSlow")));
	ASSERT_TRUE(network.addArc(inputArc(model, &input, &fast, 1, "red", "InFast")));
	ASSERT_TRUE(network.addArc(outputArc(model, &slow, &slowOut, 1, "red", "OutSlow")));
	ASSERT_TRUE(network.addArc(outputArc(model, &fast, &fastOut, 1, "red", "OutFast")));

	ASSERT_EQ(network.firstEnabledTransition(), &fast);
	ASSERT_TRUE(network.fire(network.firstEnabledTransition()));

	EXPECT_EQ(input.getTokens("red"), 1u);
	EXPECT_EQ(slowOut.getTokens("red"), 0u);
	EXPECT_EQ(fastOut.getTokens("red"), 1u);
}

TEST(ColoredPetriNetNetworkTest, ReplicationResetRestoresInitialMarking) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	ColoredPetriNetNetworkProbe network(model, "CPN");
	PetriPlace p(model, "P");
	CPNTransition t(model, "T");
	network.addPlace(&p);
	network.addTransition(&t);
	network.setInitialTokens(&p, "red", 3);
	ASSERT_TRUE(network.addArc(inputArc(model, &p, &t, 1, "red", "In")));
	ASSERT_TRUE(network.addArc(outputArc(model, &t, &p, 1, "blue", "Out")));

	network.activate(NetworkActivationFrame(network.getNumInputPorts()));
	ASSERT_EQ(p.getTokens("red"), 2u);
	ASSERT_EQ(p.getTokens("blue"), 1u);
	ASSERT_DOUBLE_EQ(network.getActivationCount(), 1.0);

	network.InitBetweenReplicationsProbe();

	EXPECT_EQ(p.getTokens("red"), 3u);
	EXPECT_EQ(p.getTokens("blue"), 0u);
	EXPECT_DOUBLE_EQ(network.getActivationCount(), 0.0);
}

TEST(ColoredPetriNetNetworkTest, CheckRejectsInvalidTopologyAndAcceptsWellFormedSubset) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	ColoredPetriNetNetworkProbe empty(model, "Empty");
	std::string errorMessage;
	EXPECT_FALSE(empty.CheckProbe(errorMessage));
	EXPECT_NE(errorMessage.find("requires at least one place"), std::string::npos);
	EXPECT_NE(errorMessage.find("requires at least one transition"), std::string::npos);

	ColoredPetriNetNetworkProbe valid(model, "Valid");
	PetriPlace p(model, "P");
	CPNTransition t(model, "T");
	valid.addPlace(&p);
	valid.addTransition(&t);
	ASSERT_TRUE(valid.addArc(inputArc(model, &p, &t, 1, "default", "PT")));

	errorMessage.clear();
	EXPECT_TRUE(valid.CheckProbe(errorMessage)) << errorMessage;
}

TEST(ColoredPetriNetNetworkTest, PersistenceRoundTripPreservesBipartiteTopologyAndMarkings) {
	Simulator simulator;
	ASSERT_NE(simulator.getPluginManager()->insert("petriplace.so"), nullptr);
	ASSERT_NE(simulator.getPluginManager()->insert("cpntransition.so"), nullptr);
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	ColoredPetriNetNetworkProbe source(model, "PersistentCPN");
	PetriPlace input(model, "Input");
	PetriPlace output(model, "Output");
	CPNTransition transition(model, "Transform");
	transition.setGuardExpression("1");
	transition.setPriority(4);
	source.addPlace(&input);
	source.addPlace(&output);
	source.addTransition(&transition);
	source.setInitialTokens(&input, "red", 2);
	source.setInitialTokens(&output, "blue", 0);
	ASSERT_TRUE(source.addArc(inputArc(model, &input, &transition, 2, "red", "In")));
	ASSERT_TRUE(source.addArc(outputArc(model, &transition, &output, 3, "blue", "Out")));
	ASSERT_TRUE(source.fire(&transition));

	FakeModelPersistenceRuntime persistence;
	PersistenceRecord fields(persistence);
	source.SaveInstanceProbe(&fields, true);

	ColoredPetriNetNetworkProbe loaded(model, "LoadedCPN");
	ASSERT_TRUE(loaded.LoadInstanceProbe(&fields));

	ASSERT_EQ(loaded.getPlaces()->size(), 2u);
	ASSERT_EQ(loaded.getTransitions()->size(), 1u);
	ASSERT_EQ(loaded.getArcs()->size(), 2u);
	EXPECT_EQ(loaded.getInitialTokens(loaded.getPlaces()->front(), "red"), 2u);
	EXPECT_EQ(loaded.getPlaces()->front()->getTokens("red"), 0u);
	EXPECT_EQ(loaded.getPlaces()->getAtRank(1)->getTokens("blue"), 3u);
	ASSERT_EQ(loaded.getInputArcs(loaded.getTransitions()->front()).size(), 1u);
	ASSERT_EQ(loaded.getOutputArcs(loaded.getTransitions()->front()).size(), 1u);
	EXPECT_EQ(loaded.getInputArcs(loaded.getTransitions()->front()).front()->getInscription("red"), 2u);
	EXPECT_EQ(loaded.getOutputArcs(loaded.getTransitions()->front()).front()->getInscription("blue"), 3u);
}

TEST(ColoredPetriNetNetworkTest, SharedNetActivatedThroughTwoModalModelsKeepsNetworkOwnedMarking) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	ColoredPetriNetNetwork network(model, "SharedCPN");
	PetriPlace p(model, "P");
	CPNTransition t(model, "Consume");
	network.addPlace(&p);
	network.addTransition(&t);
	network.setInitialTokens(&p, "red", 2);
	ASSERT_TRUE(network.addArc(inputArc(model, &p, &t, 1, "red", "In")));
	Attribute firedAttribute(model, "Fired");

	ModalModelDefaultProbe modalA(model, "ModalA");
	ModalModelDefaultProbe modalB(model, "ModalB");
	CollectorSinkComponentProbe sinkA(model, "SinkA");
	CollectorSinkComponentProbe sinkB(model, "SinkB");
	modalA.getConnectionManager()->insert(&sinkA);
	modalB.getConnectionManager()->insert(&sinkB);
	modalA.setNetwork(&network);
	modalB.setNetworkName("SharedCPN");
	modalA.setOutputBinding(0, "Fired");
	modalB.setOutputBinding(0, "Fired");

	Entity* entityA = model->createEntity("EntityA", true);
	Entity* entityB = model->createEntity("EntityB", true);
	modalA.dispatch(entityA);
	modalB.dispatch(entityB);
	drainFutureEvents(model);

	EXPECT_EQ(p.getTokens("red"), 0u);
	EXPECT_DOUBLE_EQ(network.getActivationCount(), 2.0);
	EXPECT_DOUBLE_EQ(entityA->getAttributeValue("Fired"), 1.0);
	EXPECT_DOUBLE_EQ(entityB->getAttributeValue("Fired"), 1.0);
	ASSERT_EQ(sinkA.receivedEntities().size(), 1u);
	ASSERT_EQ(sinkB.receivedEntities().size(), 1u);
	EXPECT_EQ(modalB.getNetwork(), &network);
	(void)firedAttribute;
}

TEST(ColoredPetriNetNetworkTest, PluginRegistrationCreatesCpnDataDefinitions) {
	Simulator simulator;
	Plugin* placePlugin = simulator.getPluginManager()->insert("petriplace.so");
	Plugin* transitionPlugin = simulator.getPluginManager()->insert("cpntransition.so");
	Plugin* arcPlugin = simulator.getPluginManager()->insert("cpnarc.so");
	Plugin* networkPlugin = simulator.getPluginManager()->insert("coloredpetrinetnetwork.so");
	ASSERT_NE(placePlugin, nullptr);
	ASSERT_NE(transitionPlugin, nullptr);
	ASSERT_NE(arcPlugin, nullptr);
	ASSERT_NE(networkPlugin, nullptr);
	Model* model = simulator.getModelManager()->newModel();

	EXPECT_NE(dynamic_cast<PetriPlace*>(placePlugin->newInstance(model, "P")), nullptr);
	EXPECT_NE(dynamic_cast<CPNTransition*>(transitionPlugin->newInstance(model, "T")), nullptr);
	EXPECT_NE(dynamic_cast<CPNArc*>(arcPlugin->newInstance(model, "A")), nullptr);
	EXPECT_NE(dynamic_cast<ColoredPetriNetNetwork*>(networkPlugin->newInstance(model, "CPN")), nullptr);

	std::unique_ptr<PluginInformation> info(ColoredPetriNetNetwork::GetPluginInformation());
	ASSERT_NE(info, nullptr);
	EXPECT_FALSE(info->isComponent());
	EXPECT_EQ(info->getPluginTypename(), Util::TypeOf<ColoredPetriNetNetwork>());
	EXPECT_EQ(info->getCategory(), "ModalModel");
	ASSERT_NE(info->getDataDefinitionLoader(), nullptr);
	ASSERT_NE(info->getDataDefinitionConstructor(), nullptr);
}
