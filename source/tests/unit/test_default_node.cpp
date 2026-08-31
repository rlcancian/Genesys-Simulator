// Unit tests for DefaultNode, the modal-network node abstraction migrated to
// ModelDataDefinition.

#include <gtest/gtest.h>

#include "kernel/simulator/Persistence.h"
#include "kernel/simulator/PluginInformation.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "plugins/components/ModalModel/DefaultNode.h"

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

class DefaultNodeProbe : public DefaultNode {
public:
	DefaultNodeProbe(Model* model, const std::string& name = "") : DefaultNode(model, name) {}
	bool LoadInstanceProbe(PersistenceRecord* fields) { return _loadInstance(fields); }
	void SaveInstanceProbe(PersistenceRecord* fields, bool saveDefaultValues = false) { _saveInstance(fields, saveDefaultValues); }
};

} // namespace

TEST(DefaultNodeTest, DefaultsStartAsPureDataDefinitionWithoutTransitions) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();

	DefaultNode node(model, "NodeA");

	EXPECT_FALSE(node.isInitialNode());
	EXPECT_FALSE(node.isFinalNode());
	ASSERT_NE(node.getTransitions(), nullptr);
	EXPECT_EQ(node.getTransitions()->size(), 0u);
	EXPECT_NE(node.show().find("initialNode=false"), std::string::npos);
	EXPECT_NE(node.show().find("finalNode=false"), std::string::npos);
}

TEST(DefaultNodeTest, TransitionCollectionAcceptsAndRemovesUniquePointers) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();

	DefaultNode source(model, "SourceNode");
	DefaultNode destination(model, "DestinationNode");
	DefaultNodeTransition transition(&source, &destination, "T0");

	source.addTransition(&transition);
	source.addTransition(&transition);

	ASSERT_EQ(source.getTransitions()->size(), 2u);
	EXPECT_EQ(source.getTransitions()->front(), &transition);

	source.removeTransition(&transition);
	EXPECT_EQ(source.getTransitions()->size(), 0u);
}

TEST(DefaultNodeTest, PersistenceRoundTripPreservesFlagsAndPluginType) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();

	DefaultNodeProbe source(model, "SourceNode");
	source.setInitialNode(true);
	source.setFinalNode(true);

	FakeModelPersistenceRuntime persistence;
	PersistenceRecord fields(persistence);
	source.SaveInstanceProbe(&fields, true);

	DefaultNodeProbe destination(model, "DestinationNode");
	ASSERT_TRUE(destination.LoadInstanceProbe(&fields));

	EXPECT_TRUE(destination.isInitialNode());
	EXPECT_TRUE(destination.isFinalNode());

	PluginInformation* info = DefaultNode::GetPluginInformation();
	ASSERT_NE(info, nullptr);
	EXPECT_FALSE(info->isComponent());
	EXPECT_NE(info->getDataDefinitionLoader(), nullptr);
	EXPECT_NE(info->getDataDefinitionConstructor(), nullptr);
}
