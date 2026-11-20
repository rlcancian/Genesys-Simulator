// Focused Qt6 tests for ModelDataDefinition-only hierarchical editing contexts.
// These tests exercise the existing ModelGraphicsScene rather than a new Network scene.

#include "graphicals/GraphicalModelDataDefinition.h"
#include "graphicals/GraphicalModelComponent.h"
#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/ModelGraphicsView.h"
#include "mainwindow.h"
#include "services/GraphicalModelBuilder.h"

#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelComponentManager.h"
#include "plugins/components/ModalModel/FSMState.h"
#include "plugins/components/ModalModel/Submodel.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/data/ModalModel/EFSMNetwork.h"
#include "plugins/data/DiscreteProcessing/Queue.h"

#include <gtest/gtest.h>

#include <QRectF>
#include <QPointF>
#include <QTextEdit>
#include <QAction>
#include <QApplication>
#include <QTabWidget>
#include <QTemporaryDir>

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

TEST(GuiSubmodelLevel, EmptySubmodelOpensAnEditingTab) {
    MainWindow window;
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelNew_triggered", Qt::DirectConnection));
    Simulator* simulator = window.myScene()->getSimulator();
    ASSERT_NE(simulator, nullptr);
    simulator->getPluginManager()->autoInsertPlugins();
    Model* model = simulator->getModelManager()->current();
    ASSERT_NE(model, nullptr);
    auto* submodel = new Submodel(model, "EmptySubmodel");

    std::map<std::string, QColor> colors;
    auto* view = window.findChild<ModelGraphicsView*>();
    ASSERT_NE(view, nullptr);
    GraphicalModelBuilder builder(simulator, view, window.myScene(), &colors, nullptr);
    builder.generateGraphicalModelFromModel();

    GraphicalModelComponent* graphicalSubmodel = nullptr;
    for (QGraphicsItem* item : window.myScene()->items()) {
        auto* graphical = dynamic_cast<GraphicalModelComponent*>(item);
        if (graphical != nullptr && graphical->getComponent() == submodel) {
            graphicalSubmodel = graphical;
            break;
        }
    }
    ASSERT_NE(graphicalSubmodel, nullptr);

    QAction* openAction = window.findChild<QAction*>(QStringLiteral("actionOpenSelectedSubmodel"));
    ASSERT_NE(openAction, nullptr);
    QTabWidget* tabs = window.findChild<QTabWidget*>(QStringLiteral("tabWidgetModelGraphics"));
    ASSERT_NE(tabs, nullptr);
    const int rootCount = tabs->count();
    graphicalSubmodel->setSelected(true);
    QApplication::processEvents();
    ASSERT_TRUE(openAction->isEnabled());
    openAction->trigger();

    ASSERT_EQ(tabs->count(), rootCount + 1);
    auto* submodelView = dynamic_cast<ModelGraphicsView*>(tabs->currentWidget());
    ASSERT_NE(submodelView, nullptr);
    EXPECT_EQ(submodelView->getScene()->modelLevelFilter(), static_cast<unsigned int>(submodel->getId()));
    EXPECT_TRUE(submodelView->getScene()->getGraphicalModelComponents()->isEmpty());
}

TEST(GuiSubmodelLevel, ComponentAddedToFilteredSubmodelUsesItsEditingContext) {
    MainWindow window;
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelNew_triggered", Qt::DirectConnection));
    Simulator* simulator = window.myScene()->getSimulator();
    ASSERT_NE(simulator, nullptr);
    simulator->getPluginManager()->autoInsertPlugins();
    Model* model = simulator->getModelManager()->current();
    ASSERT_NE(model, nullptr);
    auto* submodel = new Submodel(model, "DropTarget");

    std::map<std::string, QColor> colors;
    auto* view = window.findChild<ModelGraphicsView*>();
    ASSERT_NE(view, nullptr);
    GraphicalModelBuilder builder(simulator, view, window.myScene(), &colors, nullptr);
    builder.generateGraphicalModelFromModel();
    auto* graphicalSubmodel = dynamic_cast<GraphicalModelComponent*>(
        window.myScene()->findGraphicalModelComponent(submodel->getId()));
    ASSERT_NE(graphicalSubmodel, nullptr);
    graphicalSubmodel->setSelected(true);
    QApplication::processEvents();
    QAction* openAction = window.findChild<QAction*>(QStringLiteral("actionOpenSelectedSubmodel"));
    ASSERT_NE(openAction, nullptr);
    ASSERT_TRUE(openAction->isEnabled());
    openAction->trigger();

    auto* childPlugin = simulator->getPluginManager()->find(Util::TypeOf<Create>());
    ASSERT_NE(childPlugin, nullptr);
    auto* child = dynamic_cast<ModelComponent*>(childPlugin->newInstance(model, "FirstChild"));
    ASSERT_NE(child, nullptr);
    ASSERT_NE(window.myScene()->addGraphicalModelComponent(childPlugin, child, QPointF(25.0, 25.0)), nullptr);
    EXPECT_EQ(child->getLevel(), static_cast<unsigned int>(submodel->getId()));
}

TEST(GuiSubmodelLevel, ComponentEditingContextSurvivesModelRoundTrip) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* submodel = new Submodel(model, "PersistentSubmodel");
    auto* child = new Create(model, "PersistentChild");
    child->setModelLevel(static_cast<unsigned int>(submodel->getId()));

    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());
    const std::string filename = temporaryDir.filePath("submodel-level.gen").toStdString();
    ASSERT_TRUE(simulator.getModelManager()->saveModel(filename));

    Simulator reopener;
    reopener.getPluginManager()->autoInsertPlugins();
    Model* loadedModel = reopener.getModelManager()->loadModel(filename);
    ASSERT_NE(loadedModel, nullptr);
    auto* loadedSubmodel = dynamic_cast<Submodel*>(loadedModel->getComponentManager()->find("PersistentSubmodel"));
    auto* loadedChild = dynamic_cast<Create*>(loadedModel->getComponentManager()->find("PersistentChild"));
    ASSERT_NE(loadedSubmodel, nullptr);
    ASSERT_NE(loadedChild, nullptr);
    EXPECT_EQ(loadedChild->getLevel(), static_cast<unsigned int>(loadedSubmodel->getId()));
}
