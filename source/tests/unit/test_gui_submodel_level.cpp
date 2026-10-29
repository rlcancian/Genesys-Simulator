// Focused Qt6 tests for ModelDataDefinition-only hierarchical editing contexts.
// These tests exercise the existing ModelGraphicsScene rather than a new Network scene.

#include "graphicals/GraphicalModelDataDefinition.h"
#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/ModelGraphicsView.h"
#include "services/GraphicalModelBuilder.h"

#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelComponentManager.h"
#include "plugins/components/ModalModel/FSMState.h"
#include "plugins/data/ModalModel/EFSMNetwork.h"
#include "plugins/data/DiscreteProcessing/Queue.h"

#include <gtest/gtest.h>

#include <QRectF>
#include <QPointF>
#include <QTextEdit>

#include <map>

namespace {

GraphicalModelDataDefinition* graphicalDefinitionFor(
    ModelGraphicsScene& scene, ModelDataDefinition* element) {
    for (QGraphicsItem* item : scene.items()) {
        auto* graphical = dynamic_cast<GraphicalModelDataDefinition*>(item);
        if (graphical != nullptr && graphical->getDataDefinition() == element) {
            return graphical;
        }
    }
    return nullptr;
}

}  // namespace

TEST(GuiSubmodelLevel, NetworkStatesAppearWithoutProcessComponentsOrAttachments) {
    Simulator simulator;
    auto* plugins = simulator.getPluginManager();
    ASSERT_NE(plugins, nullptr);
    plugins->autoInsertPlugins();
    ASSERT_NE(plugins->find(Util::TypeOf<FSMState>()), nullptr);

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "Network");
    FSMState idle(model, "Idle");
    FSMState busy(model, "Busy");
    network.addState(&idle);
    network.addState(&busy);
    ASSERT_EQ(model->getComponentManager()->getAllComponents()->size(), 0u);

    ModelGraphicsScene scene(0, 0, 2000, 2000);
    scene.setSimulator(&simulator);
    scene.setModelLevelFilter(static_cast<unsigned int>(network.getId()));
    // Direct context members are not attached/shared-data links; toggles for
    // those links must not remove the Network's own mathematical states.
    scene.setShowEditableDataDefinitions(false);
    scene.setShowSharedDataDefinitions(false);
    scene.setShowStatisticsDataDefinitions(false);
    scene.setShowRecursiveDataDefinitions(false);

    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, &scene);

    auto* idleItem = graphicalDefinitionFor(scene, &idle);
    auto* busyItem = graphicalDefinitionFor(scene, &busy);
    ASSERT_NE(idleItem, nullptr);
    ASSERT_NE(busyItem, nullptr);
    EXPECT_TRUE(scene.existDiagram());
    EXPECT_FALSE(idleItem->sceneBoundingRect().intersects(busyItem->sceneBoundingRect()));
    EXPECT_EQ(scene.getAllGraphicalDiagramsConnections()->size(), 0);
    EXPECT_EQ(network.getStates()->size(), 2u);
}

TEST(GuiSubmodelLevel, SwitchingEditingContextRemovesStaleItemsWithoutChangingDomain) {
    Simulator simulator;
    auto* plugins = simulator.getPluginManager();
    ASSERT_NE(plugins, nullptr);
    plugins->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    EFSMNetwork first(model, "First");
    EFSMNetwork second(model, "Second");
    FSMState a(model, "A");
    FSMState b(model, "B");
    first.addState(&a);
    second.addState(&b);

    ModelGraphicsScene scene(0, 0, 2000, 2000);
    scene.setSimulator(&simulator);
    scene.setModelLevelFilter(static_cast<unsigned int>(first.getId()));
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, &scene);
    ASSERT_NE(graphicalDefinitionFor(scene, &a), nullptr);
    EXPECT_EQ(graphicalDefinitionFor(scene, &b), nullptr);

    scene.setModelLevelFilter(static_cast<unsigned int>(second.getId()));
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, &scene);
    EXPECT_EQ(graphicalDefinitionFor(scene, &a), nullptr);
    ASSERT_NE(graphicalDefinitionFor(scene, &b), nullptr);
    EXPECT_EQ(first.getStates()->size(), 1u);
    EXPECT_EQ(second.getStates()->size(), 1u);
    EXPECT_EQ(a.getLevel(), static_cast<unsigned int>(first.getId()));
    EXPECT_EQ(b.getLevel(), static_cast<unsigned int>(second.getId()));
}

TEST(GuiSubmodelLevel, RebuildPreservesManualPositionOfExistingContextItem) {
    Simulator simulator;
    auto* plugins = simulator.getPluginManager();
    ASSERT_NE(plugins, nullptr);
    plugins->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    EFSMNetwork network(model, "Network");
    FSMState a(model, "A");
    FSMState b(model, "B");
    network.addState(&a);
    network.addState(&b);

    ModelGraphicsScene scene(0, 0, 2000, 2000);
    scene.setSimulator(&simulator);
    scene.setModelLevelFilter(static_cast<unsigned int>(network.getId()));
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, &scene);

    auto* item = graphicalDefinitionFor(scene, &a);
    ASSERT_NE(item, nullptr);
    const QPointF customPosition(432.0, 567.0);
    item->setPos(customPosition);
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, &scene);
    EXPECT_EQ(graphicalDefinitionFor(scene, &a), item);
    EXPECT_EQ(item->pos(), customPosition);
    EXPECT_NE(graphicalDefinitionFor(scene, &b), nullptr);
}

TEST(GuiSubmodelLevel, FullBuilderReconstructsDataOnlySubmodelLevel) {
    Simulator simulator;
    auto* plugins = simulator.getPluginManager();
    ASSERT_NE(plugins, nullptr);
    plugins->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    EFSMNetwork network(model, "EFSM");
    FSMState state(model, "OnlyState");
    network.addState(&state);
    ASSERT_TRUE(model->getComponentManager()->getAllComponents()->empty());

    ModelGraphicsView view;
    ModelGraphicsScene* scene = view.getScene();
    ASSERT_NE(scene, nullptr);
    scene->setSimulator(&simulator);
    QTextEdit console;
    std::map<std::string, QColor> colors;
    GraphicalModelBuilder builder(&simulator, &view, scene, &colors, &console);
    builder.setModelLevelFilter(static_cast<unsigned int>(network.getId()));

    builder.generateGraphicalModelFromModel();

    EXPECT_NE(graphicalDefinitionFor(*scene, &state), nullptr);
    EXPECT_TRUE(scene->existDiagram());
}

TEST(GuiSubmodelLevel, ArbitraryDataDefinitionCanOwnAnEditingContext) {
    Simulator simulator;
    auto* plugins = simulator.getPluginManager();
    ASSERT_NE(plugins, nullptr);
    plugins->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Queue owner(model, "ContextOwner");
    Queue nested(model, "NestedData");
    nested.setModelLevel(static_cast<unsigned int>(owner.getId()));

    ModelGraphicsScene scene(0, 0, 2000, 2000);
    scene.setSimulator(&simulator);
    scene.setModelLevelFilter(static_cast<unsigned int>(owner.getId()));
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, &scene);

    EXPECT_NE(graphicalDefinitionFor(scene, &nested), nullptr);
    EXPECT_EQ(graphicalDefinitionFor(scene, &owner), nullptr);
}

TEST(GuiSubmodelLevel, RootDisplayStillRequiresExistingVisibilityAssociations) {
    Simulator simulator;
    auto* plugins = simulator.getPluginManager();
    ASSERT_NE(plugins, nullptr);
    plugins->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Queue standalone(model, "UnreferencedQueue");
    ASSERT_EQ(standalone.getLevel(), 0u);

    ModelGraphicsScene scene(0, 0, 2000, 2000);
    scene.setSimulator(&simulator);
    scene.setModelLevelFilter(0u);
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, &scene);
    EXPECT_EQ(graphicalDefinitionFor(scene, &standalone), nullptr);
    EXPECT_FALSE(scene.existDiagram());
}
