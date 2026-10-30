// Focused tests for the G0.2 EFSM Network diagram GUI overlay persistence:
// EFSMNetworkDiagramCodec plus its integration into
// GraphicalModelSerializer::saveGraphicalModel()/loadGraphicalModel(). These
// tests exercise the real .gui save/load path, not just the codec in
// isolation. The same target also covers the real scene projection and
// transition edit/undo command path.

#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/ModelGraphicsView.h"
#include "graphicals/EFSMTransitionGraphicsItem.h"
#include "graphicals/GraphicalModelComponent.h"
#include "actions/EFSMTransitionUndoCommand.h"
#include "actions/EFSMTransitionPropertiesUndoCommand.h"
#include "actions/EFSMInitialStateUndoCommand.h"
#include "actions/EFSMStateMembershipUndoCommand.h"
#include "actions/EFSMStatePropertiesUndoCommand.h"
#include "actions/ModalModelBindingsUndoCommand.h"
#include "controllers/EditCommandController.h"
#include "mainwindow.h"
#include "services/EFSMNetworkDiagramCodec.h"
#include "services/EFSMNetworkDiagramProjection.h"
#include "services/GraphicalModelBuilder.h"
#include "services/GraphicalModelSerializer.h"

#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelDataManager.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "plugins/components/ModalModel/FSMState.h"
#include "plugins/components/ModalModel/ModalModelDefault.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/data/ModalModel/EFSMNetwork.h"

#include <gtest/gtest.h>

#include <QAction>
#include <QFile>
#include <QPlainTextEdit>
#include <QSlider>
#include <QTemporaryDir>
#include <QTextEdit>
#include <QTextStream>
#include <QUndoStack>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QTimer>
#include <QTabWidget>
#include <QInputDialog>
#include <QMessageBox>
#include <QGraphicsSceneMouseEvent>
#include <QSpinBox>

#include <algorithm>

namespace {

class ExposedModelGraphicsScene final : public ModelGraphicsScene {
public:
    ExposedModelGraphicsScene() : ModelGraphicsScene(-1000, -1000, 4000, 4000) {}
    using ModelGraphicsScene::mousePressEvent;
    using ModelGraphicsScene::mouseReleaseEvent;
};

// Minimal widget/action bundle required by GraphicalModelSerializer's
// constructor, mirroring the pattern already established by the
// GuiGmddLayout.SerializerRoundTrip* tests.
struct SerializerHarness {
    ModelGraphicsView graphicsView;
    QPlainTextEdit modelTextEditor;
    QTextEdit console;
    QSlider zoomSlider;
    QAction actionShowGrid;
    QAction actionShowRule;
    QAction actionShowSnap;
    QAction actionShowGuides;
    QAction actionShowInternalElements;
    QAction actionShowEditableElements;
    QAction actionShowAttachedElements;
    QAction actionShowRecursiveElements;
    QString modelFilename;
    std::map<std::string, QColor> pluginCategoryColor;
    std::unique_ptr<GraphicalModelBuilder> builder;
    std::unique_ptr<GraphicalModelSerializer> serializer;

    explicit SerializerHarness(Simulator* simulator) {
        graphicsView.setSimulator(simulator);
        ModelGraphicsScene* scene = graphicsView.getScene();
        scene->setSimulator(simulator);

        for (QAction* action : {&actionShowGrid, &actionShowRule, &actionShowSnap, &actionShowGuides,
                                 &actionShowInternalElements, &actionShowEditableElements,
                                 &actionShowAttachedElements, &actionShowRecursiveElements}) {
            action->setCheckable(true);
        }

        builder = std::make_unique<GraphicalModelBuilder>(
            simulator, &graphicsView, scene, &pluginCategoryColor, &console);

        auto clearSceneForReload = [scene]() {
            scene->clear();
            scene->getGraphicalModelComponents()->clear();
            scene->getGraphicalConnections()->clear();
            scene->getGraphicalModelDataDefinitions()->clear();
            scene->getGraphicalDiagramsConnections()->clear();
            scene->getAllComponents()->clear();
            scene->getAllConnections()->clear();
            scene->getAllDataDefinitions()->clear();
            scene->getAllGraphicalDiagramsConnections()->clear();
        };

        serializer = std::make_unique<GraphicalModelSerializer>(
            simulator,
            &graphicsView,
            &modelTextEditor,
            &graphicsView,
            &zoomSlider,
            &actionShowGrid,
            &actionShowRule,
            &actionShowSnap,
            &actionShowGuides,
            &actionShowInternalElements,
            &actionShowEditableElements,
            &actionShowAttachedElements,
            &actionShowRecursiveElements,
            &console,
            &modelFilename,
            clearSceneForReload,
            []() {},
            []() {},
            []() {},
            []() {});
    }
};

FSMState* findStateByName(EFSMNetwork* network, const std::string& name) {
    for (FSMState* state : *network->getStates()->list()) {
        if (state != nullptr && state->getName() == name) {
            return state;
        }
    }
    return nullptr;
}

EFSMNetwork* findNetworkByName(Model* model, const std::string& name) {
    auto* data = model->getDataManager()->getDataDefinition(Util::TypeOf<EFSMNetwork>(), name);
    return dynamic_cast<EFSMNetwork*>(data);
}

} // namespace

TEST(EFSMNetworkDiagramPersistence, EmptyLayoutProducesNoEntriesAndLeavesOtherSectionsIntact) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "EmptyNetwork");
    FSMState a(model, "A");
    network.addState(&a);
    auto* adapter = new ModalModelDefault(model, "EmptyAdapter");
    adapter->setNetwork(&network);

    SerializerHarness harness(&simulator);

    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());
    const QString guiFilename = temporaryDir.filePath("empty.gui");
    ASSERT_TRUE(harness.serializer->saveGraphicalModel(guiFilename));

    QFile saved(guiFilename);
    ASSERT_TRUE(saved.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = QString::fromUtf8(saved.readAll());
    saved.close();
    EXPECT_TRUE(content.contains("# Graphical EFSM Network Diagrams"));
    EXPECT_FALSE(content.contains("EFSMNetworkDiagram"));
    EXPECT_TRUE(content.contains("# Graphical Model Data Definitions"));
    EXPECT_TRUE(content.contains("# Groups"));

    Model* loadedModel = harness.serializer->loadGraphicalModel(guiFilename.toStdString());
    ASSERT_NE(loadedModel, nullptr);
    EFSMNetwork* loadedNetwork = findNetworkByName(loadedModel, "EmptyNetwork");
    ASSERT_NE(loadedNetwork, nullptr);
    EXPECT_TRUE(harness.serializer->efsmNetworkDiagramLayout(
        loadedModel, static_cast<unsigned long>(loadedNetwork->getId())).isEmpty());
}

TEST(EFSMNetworkDiagramPersistence, WriterSkipsLayoutWhoseNetworkNameNoLongerMatchesItsId) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "CurrentName");
    FSMState state(model, "State");
    network.addState(&state);

    EFSMNetworkDiagramLayout staleLayout;
    staleLayout.networkId = static_cast<unsigned long>(network.getId());
    staleLayout.networkName = QStringLiteral("PreviousName");
    staleLayout.states.append({static_cast<unsigned long>(state.getId()), QStringLiteral("State"), QPointF(4, 5)});
    QHash<unsigned long, EFSMNetworkDiagramLayout> layouts;
    layouts.insert(staleLayout.networkId, staleLayout);

    QString output;
    QTextStream stream(&output);
    EFSMNetworkDiagramCodec::writeSection(stream, &simulator, layouts);
    EXPECT_TRUE(output.isEmpty());
}

TEST(EFSMNetworkDiagramPersistence, CachedLayoutsStayScopedToTheirRootModel) {
    Simulator simulator;
    Model* firstModel = simulator.getModelManager()->newModel();
    ASSERT_NE(firstModel, nullptr);
    Model* secondModel = simulator.getModelManager()->newModel();
    ASSERT_NE(secondModel, nullptr);
    SerializerHarness harness(&simulator);

    EFSMNetworkDiagramLayout firstLayout;
    firstLayout.networkId = 123;
    firstLayout.networkName = QStringLiteral("FirstModelNetwork");
    firstLayout.states.append({1, QStringLiteral("A"), QPointF(10, 20)});
    EFSMNetworkDiagramLayout secondLayout;
    secondLayout.networkId = 123;
    secondLayout.networkName = QStringLiteral("SecondModelNetwork");
    secondLayout.states.append({1, QStringLiteral("B"), QPointF(30, 40)});
    harness.serializer->setEFSMNetworkDiagramLayout(firstModel, 123, firstLayout);
    harness.serializer->setEFSMNetworkDiagramLayout(secondModel, 123, secondLayout);

    EXPECT_EQ(harness.serializer->efsmNetworkDiagramLayout(firstModel, 123).networkName,
              QStringLiteral("FirstModelNetwork"));
    EXPECT_EQ(harness.serializer->efsmNetworkDiagramLayout(secondModel, 123).networkName,
              QStringLiteral("SecondModelNetwork"));
    harness.serializer->forgetEFSMNetworkLayouts(firstModel);
    EXPECT_TRUE(harness.serializer->efsmNetworkDiagramLayout(firstModel, 123).isEmpty());
    EXPECT_EQ(harness.serializer->efsmNetworkDiagramLayout(secondModel, 123).networkName,
              QStringLiteral("SecondModelNetwork"));
}

TEST(EFSMNetworkDiagramProjection, BuildsStatesAndFormalTransitionsWithoutChangingTheDomain) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    EFSMNetwork network(model, "ViewerNetwork");
    FSMState idle(model, "Idle");
    FSMState done(model, "Done");
    network.setInitialState(&idle);
    network.addState(&done);
    done.setFinalNode(true);
    auto* guardedUnnamed = new EFSMTransition(&idle, &done);
    guardedUnnamed->setInputEvent("arrive");
    guardedUnnamed->setGuardExpression("ready == 1");
    guardedUnnamed->setOutputExpression("result = 1");
    network.addTransition(guardedUnnamed);
    network.addTransition(new EFSMTransition(&idle, &done));
    network.addTransition(new EFSMTransition(&done, &done, "Loop"));

    SerializerHarness harness(&simulator);
    ModelGraphicsScene* scene = harness.graphicsView.getScene();
    ASSERT_NE(scene, nullptr);
    scene->setModelLevelFilter(static_cast<unsigned int>(network.getId()));

    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);
    ASSERT_EQ(scene->getGraphicalModelDataDefinitions()->size(), 2);
    ASSERT_EQ(scene->getEFSMNetworkDiagramItems()->size(), 3);
    EXPECT_NE(scene->findGraphicalModelDataDefinition(&idle)->getColor(),
              scene->findGraphicalModelDataDefinition(&done)->getColor());

    auto* first = dynamic_cast<EFSMTransitionGraphicsItem*>(scene->getEFSMNetworkDiagramItems()->at(0));
    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first->ordinal(), 0);
    EXPECT_EQ(first->transitionLabel(), QStringLiteral("on arrive [ready == 1] / result = 1"));
    EXPECT_EQ(first->toolTip(), first->transitionLabel());
    auto* parallel = dynamic_cast<EFSMTransitionGraphicsItem*>(scene->getEFSMNetworkDiagramItems()->at(1));
    ASSERT_NE(parallel, nullptr);
    EXPECT_EQ(parallel->ordinal(), 1);
    EXPECT_NE(first->path(), parallel->path());
    EXPECT_TRUE(first->flags().testFlag(QGraphicsItem::ItemIsSelectable));
    EXPECT_EQ(scene->getGraphicalConnections()->size(), 0);
    EXPECT_EQ(network.getStates()->size(), 2u);
    EXPECT_EQ(network.getTransitions()->size(), 3u);

    // Rebuilding the same context replaces only the projection layer.
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);
    ASSERT_EQ(scene->getEFSMNetworkDiagramItems()->size(), 3);
    EXPECT_EQ(scene->getGraphicalModelDataDefinitions()->size(), 2);

    EFSMNetwork empty(model, "EmptyViewerNetwork");
    scene->setModelLevelFilter(static_cast<unsigned int>(empty.getId()));
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);
    EXPECT_TRUE(scene->getEFSMNetworkDiagramItems()->isEmpty());
    EXPECT_TRUE(scene->getGraphicalModelDataDefinitions()->isEmpty());

    EFSMNetwork singleStateNetwork(model, "SingleStateViewerNetwork");
    FSMState onlyState(model, "Only");
    singleStateNetwork.addState(&onlyState);
    scene->setModelLevelFilter(static_cast<unsigned int>(singleStateNetwork.getId()));
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);
    EFSMNetworkDiagramProjection::synchronize(&simulator, scene);
    EXPECT_EQ(scene->getGraphicalModelDataDefinitions()->size(), 1);
    EXPECT_TRUE(scene->getEFSMNetworkDiagramItems()->isEmpty());
}

TEST(EFSMNetworkDiagramProjection, MovingAStateRefreshesItsFormalTransitionGeometry) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "MoveNetwork");
    FSMState source(model, "Source");
    FSMState destination(model, "Destination");
    network.setInitialState(&source);
    network.addState(&destination);
    network.addTransition(new EFSMTransition(&source, &destination, "MoveEdge"));

    ExposedModelGraphicsScene scene;
    scene.setSimulator(&simulator);
    QUndoStack movementUndoStack;
    scene.setUndoStack(&movementUndoStack);
    scene.setModelLevelFilter(static_cast<unsigned int>(network.getId()));
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, &scene);
    EFSMNetworkDiagramProjection::synchronize(&simulator, &scene);
    auto* sourceItem = scene.findGraphicalModelDataDefinition(&source);
    ASSERT_NE(sourceItem, nullptr);
    ASSERT_EQ(scene.getEFSMNetworkDiagramItems()->size(), 1);
    auto* edgeItem = dynamic_cast<EFSMTransitionGraphicsItem*>(scene.getEFSMNetworkDiagramItems()->first());
    ASSERT_NE(edgeItem, nullptr);
    const QPointF before = edgeItem->path().pointAtPercent(0.5);

    const QPointF pressPoint = sourceItem->sceneBoundingRect().center();
    QGraphicsSceneMouseEvent press(QEvent::GraphicsSceneMousePress);
    press.setButton(Qt::LeftButton);
    press.setButtons(Qt::LeftButton);
    press.setScenePos(pressPoint);
    press.setButtonDownScenePos(Qt::LeftButton, pressPoint);
    scene.mousePressEvent(&press);
    sourceItem->setPos(sourceItem->pos() + QPointF(125.0, 90.0));
    sourceItem->setSelected(true);
    QGraphicsSceneMouseEvent release(QEvent::GraphicsSceneMouseRelease);
    release.setButton(Qt::LeftButton);
    release.setScenePos(sourceItem->sceneBoundingRect().center());
    scene.mouseReleaseEvent(&release);

    ASSERT_EQ(scene.getEFSMNetworkDiagramItems()->size(), 1);
    edgeItem = dynamic_cast<EFSMTransitionGraphicsItem*>(scene.getEFSMNetworkDiagramItems()->first());
    ASSERT_NE(edgeItem, nullptr);
    const QPointF moved = edgeItem->path().pointAtPercent(0.5);
    EXPECT_NE(moved, before);
    EXPECT_EQ(network.getTransitions()->size(), 1u);
    ASSERT_EQ(movementUndoStack.count(), 1);
    movementUndoStack.undo();
    ASSERT_EQ(scene.getEFSMNetworkDiagramItems()->size(), 1);
    edgeItem = dynamic_cast<EFSMTransitionGraphicsItem*>(scene.getEFSMNetworkDiagramItems()->first());
    ASSERT_NE(edgeItem, nullptr);
    EXPECT_NEAR(edgeItem->path().pointAtPercent(0.5).x(), before.x(), 0.01);
    EXPECT_NEAR(edgeItem->path().pointAtPercent(0.5).y(), before.y(), 0.01);
    movementUndoStack.redo();
    ASSERT_EQ(scene.getEFSMNetworkDiagramItems()->size(), 1);
    edgeItem = dynamic_cast<EFSMTransitionGraphicsItem*>(scene.getEFSMNetworkDiagramItems()->first());
    ASSERT_NE(edgeItem, nullptr);
    EXPECT_NE(edgeItem->path().pointAtPercent(0.5), before);
}

TEST(EFSMNetworkUndo, DeleteActionAndUndoRefreshTheSharedSceneProjection) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "EditableNetwork");
    FSMState idle(model, "Idle");
    FSMState done(model, "Done");
    network.addState(&idle);
    network.addState(&done);
    auto* transition = new EFSMTransition(&idle, &done, "Finish");
    auto* parallel = new EFSMTransition(&idle, &done, "ParallelFinish");
    network.addTransition(transition);
    network.addTransition(parallel);

    ModelGraphicsView view;
    view.setSimulator(&simulator);
    ModelGraphicsScene* scene = view.getScene();
    ASSERT_NE(scene, nullptr);
    scene->setSimulator(&simulator);
    scene->setModelLevelFilter(static_cast<unsigned int>(network.getId()));
    QUndoStack undoStack;
    scene->setUndoStack(&undoStack);
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);
    ASSERT_EQ(scene->getEFSMNetworkDiagramItems()->size(), 2);

    auto* graphicalTransition = dynamic_cast<EFSMTransitionGraphicsItem*>(scene->getEFSMNetworkDiagramItems()->front());
    ASSERT_NE(graphicalTransition, nullptr);
    auto* graphicalParallel = dynamic_cast<EFSMTransitionGraphicsItem*>(scene->getEFSMNetworkDiagramItems()->at(1));
    ASSERT_NE(graphicalParallel, nullptr);
    graphicalTransition->setSelected(true);
    graphicalParallel->setSelected(true);
    EditCommandController editController(&simulator, &view, []() {}, nullptr, nullptr, nullptr, nullptr, nullptr);
    editController.onActionEditDeleteTriggered();

    EXPECT_EQ(network.getTransitions()->size(), 0u);
    EXPECT_TRUE(scene->getEFSMNetworkDiagramItems()->isEmpty());
    ASSERT_EQ(undoStack.count(), 1);
    editController.onActionEditUndoTriggered();
    ASSERT_EQ(network.getTransitions()->size(), 2u);
    EXPECT_EQ(network.getTransitions()->front(), transition);
    EXPECT_EQ(network.getTransitions()->getAtRank(1), parallel);
    ASSERT_EQ(scene->getEFSMNetworkDiagramItems()->size(), 2);
    auto* restored = dynamic_cast<EFSMTransitionGraphicsItem*>(scene->getEFSMNetworkDiagramItems()->front());
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->transition(), transition);
    auto* restoredParallel = dynamic_cast<EFSMTransitionGraphicsItem*>(scene->getEFSMNetworkDiagramItems()->at(1));
    ASSERT_NE(restoredParallel, nullptr);
    EXPECT_EQ(restoredParallel->transition(), parallel);

    editController.onActionEditRedoTriggered();
    EXPECT_TRUE(network.getTransitions()->empty());
    EXPECT_TRUE(scene->getEFSMNetworkDiagramItems()->isEmpty());
}

TEST(EFSMNetworkDiagramPersistence, RoundTripRestoresStatePositionsAndParallelTransitionOrdinals) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    EFSMNetwork network(model, "Net");
    FSMState idle(model, "Idle");
    FSMState busy(model, "Busy");
    network.addState(&idle);
    network.addState(&busy);
    // Two parallel transitions Idle->Busy plus one named one. Empty-name
    // identity is covered by the direct codec test below because the current
    // .gen loader materializes omitted empty names as T<ordinal> defaults.
    network.addTransition(new EFSMTransition(&idle, &busy, "Parallel0"));
    network.addTransition(new EFSMTransition(&idle, &busy, "Parallel1"));
    network.addTransition(new EFSMTransition(&idle, &busy, "Go"));
    auto* adapter = new ModalModelDefault(model, "RoundTripAdapter");
    adapter->setNetwork(&network);

    SerializerHarness harness(&simulator);

    EFSMNetworkDiagramLayout layout;
    layout.networkId = static_cast<unsigned long>(network.getId());
    layout.networkName = QString::fromStdString(network.getName());
    layout.states.append({static_cast<unsigned long>(idle.getId()), "Idle", QPointF(10.0, 20.0)});
    layout.states.append({static_cast<unsigned long>(busy.getId()), "Busy", QPointF(310.5, 220.25)});
    layout.transitions.append({"Idle", "Busy", "Parallel0", 0, {QPointF(50, 60)}, true, QPointF(70, 80)});
    layout.transitions.append({"Idle", "Busy", "Parallel1", 1, {}, false, QPointF()});
    layout.transitions.append({"Idle", "Busy", "Go", 2, {}, false, QPointF()});
    harness.serializer->setEFSMNetworkDiagramLayout(model, layout.networkId, layout);

    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());
    const QString guiFilename = temporaryDir.filePath("round_trip.gui");
    ASSERT_TRUE(harness.serializer->saveGraphicalModel(guiFilename));

    Model* loadedModel = harness.serializer->loadGraphicalModel(guiFilename.toStdString());
    ASSERT_NE(loadedModel, nullptr);
    EFSMNetwork* loadedNetwork = findNetworkByName(loadedModel, "Net");
    ASSERT_NE(loadedNetwork, nullptr);
    EXPECT_EQ(loadedNetwork->getStates()->size(), 2u);
    EXPECT_EQ(loadedNetwork->getTransitions()->size(), 3u);

    const EFSMNetworkDiagramLayout restored =
        harness.serializer->efsmNetworkDiagramLayout(loadedModel, static_cast<unsigned long>(loadedNetwork->getId()));
    ASSERT_EQ(restored.states.size(), 2);
    ASSERT_EQ(restored.transitions.size(), 3);

    FSMState* loadedIdle = findStateByName(loadedNetwork, "Idle");
    FSMState* loadedBusy = findStateByName(loadedNetwork, "Busy");
    ASSERT_NE(loadedIdle, nullptr);
    ASSERT_NE(loadedBusy, nullptr);

    auto findState = [&restored](unsigned long id) -> const EFSMNetworkDiagramLayout::StatePosition* {
        for (const auto& state : restored.states) {
            if (state.stateId == id) {
                return &state;
            }
        }
        return nullptr;
    };
    const auto* idlePosition = findState(static_cast<unsigned long>(loadedIdle->getId()));
    const auto* busyPosition = findState(static_cast<unsigned long>(loadedBusy->getId()));
    ASSERT_NE(idlePosition, nullptr);
    ASSERT_NE(busyPosition, nullptr);
    EXPECT_NEAR(idlePosition->position.x(), 10.0, 0.001);
    EXPECT_NEAR(idlePosition->position.y(), 20.0, 0.001);
    EXPECT_NEAR(busyPosition->position.x(), 310.5, 0.001);
    EXPECT_NEAR(busyPosition->position.y(), 220.25, 0.001);

    // The two parallel transitions remain distinguished by ordinal.
    int parallelOrdinal0 = -1;
    int parallelOrdinal1 = -1;
    int namedOrdinal = -1;
    for (const auto& transitionLayout : restored.transitions) {
        if (transitionLayout.transitionName == "Parallel0") {
            parallelOrdinal0 = transitionLayout.ordinal;
            EXPECT_TRUE(transitionLayout.hasLabelPosition);
            ASSERT_EQ(transitionLayout.controlPoints.size(), 1);
            EXPECT_NEAR(transitionLayout.controlPoints.front().x(), 50.0, 0.001);
        } else if (transitionLayout.transitionName == "Parallel1") {
            parallelOrdinal1 = transitionLayout.ordinal;
            EXPECT_FALSE(transitionLayout.hasLabelPosition);
        } else if (transitionLayout.transitionName == "Go") {
            namedOrdinal = transitionLayout.ordinal;
        }
    }
    EXPECT_EQ(parallelOrdinal0, 0);
    EXPECT_EQ(parallelOrdinal1, 1);
    EXPECT_EQ(namedOrdinal, 2);
}

TEST(EFSMNetworkDiagramPersistence, RoundTripIsDeterministicAcrossRepeatedSaves) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "Net");
    FSMState a(model, "A");
    FSMState b(model, "B");
    network.addState(&a);
    network.addState(&b);
    network.addTransition(new EFSMTransition(&a, &b, "T"));
    auto* adapter = new ModalModelDefault(model, "DeterministicAdapter");
    adapter->setNetwork(&network);

    SerializerHarness harness(&simulator);
    EFSMNetworkDiagramLayout layout;
    layout.networkId = static_cast<unsigned long>(network.getId());
    layout.states.append({static_cast<unsigned long>(a.getId()), "A", QPointF(1.0, 2.0)});
    layout.states.append({static_cast<unsigned long>(b.getId()), "B", QPointF(3.0, 4.0)});
    layout.transitions.append({"A", "B", "T", 0, {}, false, QPointF()});
    harness.serializer->setEFSMNetworkDiagramLayout(model, layout.networkId, layout);

    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());
    const QString guiFilenameFirst = temporaryDir.filePath("first.gui");
    const QString guiFilenameSecond = temporaryDir.filePath("second.gui");
    ASSERT_TRUE(harness.serializer->saveGraphicalModel(guiFilenameFirst));
    ASSERT_TRUE(harness.serializer->saveGraphicalModel(guiFilenameSecond));

    auto extractEfsmBlock = [](const QString& filename) {
        QFile file(filename);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        const QString content = QString::fromUtf8(file.readAll());
        const int markerIndex = content.indexOf("# Graphical EFSM Network Diagrams");
        const int nextSection = content.indexOf("\n# Groups", markerIndex + 1);
        const int endIndex = nextSection >= 0 ? nextSection : content.size();
        return content.mid(markerIndex, endIndex - markerIndex);
    };
    EXPECT_EQ(extractEfsmBlock(guiFilenameFirst), extractEfsmBlock(guiFilenameSecond));
}

TEST(EFSMNetworkDiagramPersistence, LegacyGuiFileWithoutEfsmSectionLoadsWithoutError) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "Net");
    FSMState a(model, "A");
    network.addState(&a);
    auto* adapter = new ModalModelDefault(model, "LegacyAdapter");
    adapter->setNetwork(&network);

    SerializerHarness harness(&simulator);
    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());
    const QString guiFilename = temporaryDir.filePath("legacy.gui");
    ASSERT_TRUE(harness.serializer->saveGraphicalModel(guiFilename));

    // Simulate a legacy file saved before G0.2 existed by stripping the new section.
    QFile file(guiFilename);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = QString::fromUtf8(file.readAll());
    file.close();
    const int markerIndex = content.indexOf("# Graphical EFSM Network Diagrams");
    ASSERT_NE(markerIndex, -1);
    const int nextSection = content.indexOf("\n# Groups", markerIndex + 1);
    const int endIndex = nextSection >= 0 ? nextSection : content.size();
    content.remove(markerIndex, endIndex - markerIndex);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
    file.write(content.toUtf8());
    file.close();

    Model* loadedModel = harness.serializer->loadGraphicalModel(guiFilename.toStdString());
    ASSERT_NE(loadedModel, nullptr);
    EFSMNetwork* loadedNetwork = findNetworkByName(loadedModel, "Net");
    ASSERT_NE(loadedNetwork, nullptr);
    EXPECT_EQ(loadedNetwork->getStates()->size(), 1u);
    EXPECT_TRUE(harness.serializer->efsmNetworkDiagramLayout(
        loadedModel, static_cast<unsigned long>(loadedNetwork->getId())).isEmpty());
}

TEST(EFSMNetworkDiagramPersistence, PlainGenLoadDoesNotDependOnGraphicalSection) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "Net");
    FSMState a(model, "A");
    network.addState(&a);
    auto* adapter = new ModalModelDefault(model, "PlainGenAdapter");
    adapter->setNetwork(&network);

    SerializerHarness harness(&simulator);
    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());
    const QString genFilename = temporaryDir.filePath("plain.gen");
    ASSERT_TRUE(simulator.getModelManager()->saveModel(genFilename.toStdString()));

    Model* loadedModel = harness.serializer->loadGraphicalModel(genFilename.toStdString());
    ASSERT_NE(loadedModel, nullptr);
    EFSMNetwork* loadedNetwork = findNetworkByName(loadedModel, "Net");
    ASSERT_NE(loadedNetwork, nullptr);
    EXPECT_TRUE(harness.serializer->efsmNetworkDiagramLayout(
        loadedModel, static_cast<unsigned long>(loadedNetwork->getId())).isEmpty());
}

TEST(EFSMNetworkDiagramPersistence, UnknownSchemaVersionIsRejectedWithoutCrashing) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "Net");
    FSMState a(model, "A");
    network.addState(&a);

    const unsigned long networkId = static_cast<unsigned long>(network.getId());
    const unsigned long stateId = static_cast<unsigned long>(a.getId());
    QStringList lines;
    lines << QString("%1\tEFSMNetworkDiagram\tclassname=EFSMNetwork\tname=Net\tversion=2").arg(networkId);
    lines << QString("%1\tEFSMState\tid=%2\tname=A\tposition=(1.0,2.0)").arg(networkId).arg(stateId);

    const auto resolved = EFSMNetworkDiagramCodec::parseSection(lines, model);
    EXPECT_TRUE(resolved.isEmpty());
}

TEST(EFSMNetworkDiagramPersistence, NetworkIdentityRequiresMatchingClassAndName) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork first(model, "FirstNetwork");
    EFSMNetwork second(model, "SecondNetwork");
    FSMState firstState(model, "FirstState");
    first.addState(&firstState);

    const unsigned long firstId = static_cast<unsigned long>(first.getId());
    const unsigned long secondId = static_cast<unsigned long>(second.getId());
    QStringList mismatchedName;
    mismatchedName << QString("%1\tEFSMNetworkDiagram\tclassname=EFSMNetwork\tname=SecondNetwork\tversion=1")
                          .arg(firstId);
    mismatchedName << QString("%1\tEFSMState\tid=%2\tname=FirstState\tposition=(1,2)")
                          .arg(firstId)
                          .arg(static_cast<unsigned long>(firstState.getId()));
    EXPECT_TRUE(EFSMNetworkDiagramCodec::parseSection(mismatchedName, model).isEmpty());

    QStringList emptyNameMismatch;
    emptyNameMismatch << QString("%1\tEFSMNetworkDiagram\tclassname=EFSMNetwork\tname=\tversion=1")
                             .arg(firstId);
    emptyNameMismatch << QString("%1\tEFSMState\tid=%2\tname=FirstState\tposition=(1,2)")
                             .arg(firstId)
                             .arg(static_cast<unsigned long>(firstState.getId()));
    EXPECT_TRUE(EFSMNetworkDiagramCodec::parseSection(emptyNameMismatch, model).isEmpty());

    QStringList invalidClass;
    invalidClass << QString("%1\tEFSMNetworkDiagram\tclassname=DefaultNetwork\tname=FirstNetwork\tversion=1")
                        .arg(firstId);
    invalidClass << QString("%1\tEFSMState\tid=%2\tname=FirstState\tposition=(1,2)")
                        .arg(firstId)
                        .arg(static_cast<unsigned long>(firstState.getId()));
    EXPECT_TRUE(EFSMNetworkDiagramCodec::parseSection(invalidClass, model).isEmpty());

    QStringList uniqueFallback;
    uniqueFallback << QString("999999\tEFSMNetworkDiagram\tclassname=EFSMNetwork\tname=FirstNetwork\tversion=1");
    uniqueFallback << QString("999999\tEFSMState\tid=%1\tname=%2\tposition=(1,2)")
                          .arg(static_cast<unsigned long>(first.getId()))
                          .arg("FirstState");
    const auto fallback = EFSMNetworkDiagramCodec::parseSection(uniqueFallback, model);
    EXPECT_TRUE(fallback.contains(firstId));
    EXPECT_FALSE(fallback.contains(secondId));
}

TEST(EFSMNetworkDiagramPersistence, AmbiguousNetworkNameFallbackIsRejected) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork first(model, "DuplicateNetwork");
    EFSMNetwork second(model, "DuplicateNetwork");

    QStringList lines;
    lines << "999999\tEFSMNetworkDiagram\tclassname=EFSMNetwork\tname=DuplicateNetwork\tversion=1";
    EXPECT_TRUE(EFSMNetworkDiagramCodec::parseSection(lines, model).isEmpty());
}

TEST(EFSMNetworkDiagramPersistence, StateIdentityRequiresMatchingIdAndName) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "Net");
    FSMState state(model, "ActualState");
    network.addState(&state);

    const unsigned long networkId = static_cast<unsigned long>(network.getId());
    const unsigned long stateId = static_cast<unsigned long>(state.getId());
    QStringList lines;
    lines << QString("%1\tEFSMNetworkDiagram\tclassname=EFSMNetwork\tname=Net\tversion=1").arg(networkId);
    lines << QString("%1\tEFSMState\tid=%2\tname=DifferentState\tposition=(1,2)")
                  .arg(networkId)
                  .arg(stateId);
    EXPECT_TRUE(EFSMNetworkDiagramCodec::parseSection(lines, model).isEmpty());
}

TEST(EFSMNetworkDiagramPersistence, UnnamedParallelTransitionsRequireExactNames) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "Net");
    FSMState source(model, "A");
    FSMState destination(model, "B");
    network.addState(&source);
    network.addState(&destination);
    network.addTransition(new EFSMTransition(&source, &destination, ""));
    network.addTransition(new EFSMTransition(&source, &destination, ""));

    const unsigned long networkId = static_cast<unsigned long>(network.getId());
    QStringList lines;
    lines << QString("%1\tEFSMNetworkDiagram\tclassname=EFSMNetwork\tname=Net\tversion=1").arg(networkId);
    lines << QString("%1\tEFSMTransition\tsource=A\tdestination=B\tname=\tordinal=0").arg(networkId);
    lines << QString("%1\tEFSMTransition\tsource=A\tdestination=B\tname=\tordinal=1").arg(networkId);
    const auto resolved = EFSMNetworkDiagramCodec::parseSection(lines, model);
    ASSERT_TRUE(resolved.contains(networkId));
    ASSERT_EQ(resolved.value(networkId).transitions.size(), 2);
    EXPECT_EQ(resolved.value(networkId).transitions.at(0).ordinal, 0);
    EXPECT_EQ(resolved.value(networkId).transitions.at(1).ordinal, 1);

    network.getTransitions()->getAtRank(1)->setName("Renamed");
    const auto renamed = EFSMNetworkDiagramCodec::parseSection(lines, model);
    ASSERT_TRUE(renamed.contains(networkId));
    ASSERT_EQ(renamed.value(networkId).transitions.size(), 1);
    EXPECT_EQ(renamed.value(networkId).transitions.front().ordinal, 0);

    QStringList missingName;
    missingName << QString("%1\tEFSMNetworkDiagram\tclassname=EFSMNetwork\tname=Net\tversion=1").arg(networkId);
    missingName << QString("%1\tEFSMTransition\tsource=A\tdestination=B\tordinal=0").arg(networkId);
    EXPECT_TRUE(EFSMNetworkDiagramCodec::parseSection(missingName, model).isEmpty());
}

TEST(EFSMNetworkDiagramPersistence, NonFiniteStatePositionIsRejected) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "Net");
    FSMState state(model, "State");
    network.addState(&state);

    const unsigned long networkId = static_cast<unsigned long>(network.getId());
    const unsigned long stateId = static_cast<unsigned long>(state.getId());
    QStringList lines;
    lines << QString("%1\tEFSMNetworkDiagram\tclassname=EFSMNetwork\tname=Net\tversion=1").arg(networkId);
    lines << QString("%1\tEFSMState\tid=%2\tname=State\tposition=(nan,2)")
                  .arg(networkId)
                  .arg(stateId);
    EXPECT_TRUE(EFSMNetworkDiagramCodec::parseSection(lines, model).isEmpty());
}

TEST(EFSMNetworkDiagramPersistence, MalformedAndIncompleteLinesAreSkippedIndividually) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "Net");
    FSMState a(model, "A");
    FSMState b(model, "B");
    network.addState(&a);
    network.addState(&b);

    const unsigned long networkId = static_cast<unsigned long>(network.getId());
    const unsigned long aId = static_cast<unsigned long>(a.getId());
    QStringList lines;
    lines << QString("%1\tEFSMNetworkDiagram\tclassname=EFSMNetwork\tname=Net\tversion=1").arg(networkId);
    // Malformed: missing position.
    lines << QString("%1\tEFSMState\tid=%2\tname=A").arg(networkId).arg(aId);
    // Malformed: non-numeric id.
    lines << QString("%1\tEFSMState\tid=notanumber\tname=B\tposition=(1,2)").arg(networkId);
    // Well-formed: should survive.
    lines << QString("%1\tEFSMState\tid=%2\tname=A\tposition=(5.0,6.0)").arg(networkId).arg(aId);
    // Garbage line with wrong structure entirely.
    lines << "not a valid line at all";

    const auto resolved = EFSMNetworkDiagramCodec::parseSection(lines, model);
    ASSERT_TRUE(resolved.contains(networkId));
    const EFSMNetworkDiagramLayout& layout = resolved.value(networkId);
    ASSERT_EQ(layout.states.size(), 1);
    EXPECT_EQ(layout.states.front().stateId, aId);
    EXPECT_NEAR(layout.states.front().position.x(), 5.0, 0.001);
}

TEST(EFSMNetworkDiagramPersistence, MissingStateReferenceIsDroppedNotFabricated) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "Net");
    FSMState a(model, "A");
    network.addState(&a);

    const unsigned long networkId = static_cast<unsigned long>(network.getId());
    QStringList lines;
    lines << QString("%1\tEFSMNetworkDiagram\tclassname=EFSMNetwork\tname=Net\tversion=1").arg(networkId);
    // References a state id/name that does not exist in this network.
    lines << QString("%1\tEFSMState\tid=999999\tname=Ghost\tposition=(1,2)").arg(networkId);

    const auto resolved = EFSMNetworkDiagramCodec::parseSection(lines, model);
    EXPECT_TRUE(resolved.isEmpty());
    EXPECT_EQ(model->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<FSMState>()), 1u);
}

TEST(EFSMNetworkDiagramPersistence, AmbiguousTransitionOrdinalIsSkippedAfterTopologyShrinks) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "Net");
    FSMState a(model, "A");
    FSMState b(model, "B");
    network.addState(&a);
    network.addState(&b);
    // Only one A->B transition exists now, but the persisted layout claims ordinal 1.
    network.addTransition(new EFSMTransition(&a, &b, ""));

    const unsigned long networkId = static_cast<unsigned long>(network.getId());
    QStringList lines;
    lines << QString("%1\tEFSMNetworkDiagram\tclassname=EFSMNetwork\tname=Net\tversion=1").arg(networkId);
    lines << QString("%1\tEFSMTransition\tsource=A\tdestination=B\tname=\tordinal=1").arg(networkId);

    const auto resolved = EFSMNetworkDiagramCodec::parseSection(lines, model);
    EXPECT_TRUE(resolved.isEmpty());
    EXPECT_EQ(network.getTransitions()->size(), 1u);
}

TEST(EFSMNetworkDiagramPersistence, SaveAndLoadDoesNotChangeTopologyOrDuplicateObjects) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "Net");
    FSMState idle(model, "Idle");
    FSMState busy(model, "Busy");
    network.addState(&idle);
    network.addState(&busy);
    network.setInitialState(&idle);
    network.addTransition(new EFSMTransition(&idle, &busy, "Go"));
    auto* adapter = new ModalModelDefault(model, "IntegrityAdapter");
    adapter->setNetwork(&network);

    SerializerHarness harness(&simulator);
    ModelGraphicsScene* scene = harness.graphicsView.getScene();
    scene->setModelLevelFilter(static_cast<unsigned int>(network.getId()));
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);
    EFSMNetworkDiagramProjection::synchronize(&simulator, scene);
    GraphicalModelDataDefinition* idleItem = scene->findGraphicalModelDataDefinition(&idle);
    GraphicalModelDataDefinition* busyItem = scene->findGraphicalModelDataDefinition(&busy);
    ASSERT_NE(idleItem, nullptr);
    ASSERT_NE(busyItem, nullptr);
    idleItem->setPos(11.5, 22.25);
    busyItem->setPos(33.75, 44.125);
    EFSMNetworkDiagramLayout layout;
    ASSERT_TRUE(EFSMNetworkDiagramProjection::captureLayout(&network, scene, &layout));
    ASSERT_EQ(layout.states.size(), 2);
    EXPECT_EQ(layout.states.at(0).position, QPointF(11.5, 22.25));
    EXPECT_EQ(layout.states.at(1).position, QPointF(33.75, 44.125));
    harness.serializer->setEFSMNetworkDiagramLayout(model, layout.networkId, layout);

    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());
    const QString guiFilename = temporaryDir.filePath("integrity.gui");
    ASSERT_TRUE(harness.serializer->saveGraphicalModel(guiFilename));
    Model* loadedModel = harness.serializer->loadGraphicalModel(guiFilename.toStdString());
    ASSERT_NE(loadedModel, nullptr);

    EXPECT_EQ(loadedModel->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<EFSMNetwork>()), 1u);
    EFSMNetwork* loadedNetwork = findNetworkByName(loadedModel, "Net");
    ASSERT_NE(loadedNetwork, nullptr);
    EXPECT_EQ(loadedNetwork->getStates()->size(), 2u);
    EXPECT_EQ(loadedNetwork->getTransitions()->size(), 1u);
    ASSERT_NE(loadedNetwork->getInitialState(), nullptr);
    EXPECT_EQ(loadedNetwork->getInitialState()->getName(), "Idle");
    ASSERT_NE(loadedNetwork->getCurrentState(), nullptr);
    EXPECT_EQ(loadedNetwork->getCurrentState()->getName(), "Idle");
    EFSMTransition* loadedTransition = loadedNetwork->getTransitions()->front();
    ASSERT_NE(loadedTransition, nullptr);
    EXPECT_EQ(loadedTransition->getGuardExpression(), "");
    EXPECT_EQ(loadedTransition->getPriority(), 0u);
    const EFSMNetworkDiagramLayout restoredLayout = harness.serializer->efsmNetworkDiagramLayout(
        loadedModel, static_cast<unsigned long>(loadedNetwork->getId()));
    ASSERT_EQ(restoredLayout.states.size(), 2);
    EXPECT_EQ(restoredLayout.states.at(0).position, QPointF(11.5, 22.25));
    EXPECT_EQ(restoredLayout.states.at(1).position, QPointF(33.75, 44.125));
    scene->setModelLevelFilter(static_cast<unsigned int>(loadedNetwork->getId()));
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);
    EFSMNetworkDiagramProjection::restoreLayout(&simulator, loadedNetwork, scene, restoredLayout);
    FSMState* loadedIdle = findStateByName(loadedNetwork, "Idle");
    FSMState* loadedBusy = findStateByName(loadedNetwork, "Busy");
    ASSERT_NE(loadedIdle, nullptr);
    ASSERT_NE(loadedBusy, nullptr);
    ASSERT_NE(scene->findGraphicalModelDataDefinition(loadedIdle), nullptr);
    ASSERT_NE(scene->findGraphicalModelDataDefinition(loadedBusy), nullptr);
    EXPECT_EQ(scene->findGraphicalModelDataDefinition(loadedIdle)->pos(), QPointF(11.5, 22.25));
    EXPECT_EQ(scene->findGraphicalModelDataDefinition(loadedBusy)->pos(), QPointF(33.75, 44.125));
}

TEST(EFSMNetworkUndo, TransitionDeletionUndoRestoresTheSameOrdinalAndSemantics) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "UndoNetwork");
    FSMState idle(model, "Idle");
    FSMState busy(model, "Busy");
    network.addState(&idle);
    network.addState(&busy);
    auto* first = new EFSMTransition(&idle, &busy, "First");
    auto* removed = new EFSMTransition(&idle, &busy, "Removed");
    auto* last = new EFSMTransition(&busy, &idle, "Last");
    removed->setGuardExpression("1");
    removed->setOutputExpression("42");
    removed->setPriority(7);
    network.addTransition(first);
    network.addTransition(removed);
    network.addTransition(last);

    QUndoStack undoStack;
    undoStack.push(new EFSMTransitionUndoCommand(&network, removed));
    ASSERT_EQ(network.getTransitions()->size(), 2u);
    EXPECT_EQ(idle.getTransitions()->size(), 1u);
    undoStack.undo();

    ASSERT_EQ(network.getTransitions()->size(), 3u);
    EXPECT_EQ(network.getTransitions()->getAtRank(0), first);
    EXPECT_EQ(network.getTransitions()->getAtRank(1), removed);
    EXPECT_EQ(network.getTransitions()->getAtRank(2), last);
    EXPECT_EQ(idle.getTransitions()->getAtRank(1), removed);
    EXPECT_EQ(removed->getGuardExpression(), "1");
    EXPECT_EQ(removed->getOutputExpression(), "42");
    EXPECT_EQ(removed->getPriority(), 7u);

    undoStack.redo();
    EXPECT_EQ(network.getTransitions()->size(), 2u);
    undoStack.undo();
    EXPECT_EQ(network.getTransitions()->getAtRank(1), removed);
}

TEST(EFSMNetworkUndo, TransitionCreationUndoRedoTransfersOwnershipSafely) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "UndoNetwork");
    FSMState idle(model, "Idle");
    FSMState busy(model, "Busy");
    network.addState(&idle);
    network.addState(&busy);
    auto* existing = new EFSMTransition(&idle, &busy, "Existing");
    network.addTransition(existing);

    auto transition = std::make_unique<EFSMTransition>(&busy, &idle, "Created");
    transition->setGuardExpression("1");
    transition->setOutputExpression("9");
    EFSMTransition* created = transition.get();
    QUndoStack undoStack;
    undoStack.push(new EFSMTransitionUndoCommand(&network, std::move(transition)));
    ASSERT_EQ(network.getTransitions()->size(), 2u);
    EXPECT_EQ(network.getTransitions()->getAtRank(1), created);

    undoStack.undo();
    ASSERT_EQ(network.getTransitions()->size(), 1u);
    EXPECT_EQ(busy.getTransitions()->size(), 0u);
    undoStack.redo();
    ASSERT_EQ(network.getTransitions()->size(), 2u);
    EXPECT_EQ(network.getTransitions()->getAtRank(1), created);
    EXPECT_EQ(created->getGuardExpression(), "1");
    EXPECT_EQ(created->getOutputExpression(), "9");
}

TEST(EFSMNetworkUndo, TransitionPropertiesUndoRedoRestoresMathematicalValues) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "Network");
    FSMState idle(model, "Idle");
    FSMState busy(model, "Busy");
    network.addState(&idle);
    network.addState(&busy);
    auto* transition = new EFSMTransition(&idle, &busy, "begin");
    transition->setGuardExpression("ready == 1");
    transition->setOutputExpression("started = 1");
    transition->setInputEvent("start");
    transition->setPriority(2);
    transition->setTriggerEvent("trigger-start");
    transition->setProbabilityExpression("0.75");
    network.addTransition(transition);

    EFSMTransitionPropertiesUndoCommand::Properties edited;
    edited.name = "finish";
    edited.guardExpression = "done == 1";
    edited.outputExpression = "finished = 1";
    edited.inputEvent = "finish";
    edited.priority = 7;
    edited.triggerEvent = "trigger-finish";
    edited.probabilityExpression = "0.25";

    QUndoStack stack;
    stack.push(new EFSMTransitionPropertiesUndoCommand(&network, transition, edited));
    EXPECT_EQ(transition->getName(), "finish");
    EXPECT_EQ(transition->getGuardExpression(), "done == 1");
    EXPECT_EQ(transition->getOutputExpression(), "finished = 1");
    EXPECT_EQ(transition->getInputEvent(), "finish");
    EXPECT_EQ(transition->getPriority(), 7u);
    EXPECT_EQ(transition->getTriggerEvent(), "trigger-finish");
    EXPECT_EQ(transition->getProbabilityExpression(), "0.25");

    stack.undo();
    EXPECT_EQ(transition->getName(), "begin");
    EXPECT_EQ(transition->getGuardExpression(), "ready == 1");
    EXPECT_EQ(transition->getOutputExpression(), "started = 1");
    EXPECT_EQ(transition->getInputEvent(), "start");
    EXPECT_EQ(transition->getPriority(), 2u);
    EXPECT_EQ(transition->getTriggerEvent(), "trigger-start");
    EXPECT_EQ(transition->getProbabilityExpression(), "0.75");

    stack.redo();
    EXPECT_EQ(transition->getName(), "finish");
    EXPECT_EQ(transition->getPriority(), 7u);
}

TEST(EFSMNetworkUndo, InitialStateChangeUndoRedoRestoresPointerAndStateFlags) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "InitialUndoNetwork");
    FSMState idle(model, "Idle");
    FSMState done(model, "Done");
    network.addState(&idle);
    network.addState(&done);
    network.setInitialState(&idle);

    QUndoStack stack;
    stack.push(new EFSMInitialStateUndoCommand(&network, &done));
    EXPECT_EQ(network.getInitialState(), &done);
    EXPECT_TRUE(done.isInitialNode());
    EXPECT_FALSE(idle.isInitialNode());

    stack.undo();
    EXPECT_EQ(network.getInitialState(), &idle);
    EXPECT_TRUE(idle.isInitialNode());
    EXPECT_FALSE(done.isInitialNode());

    stack.redo();
    EXPECT_EQ(network.getInitialState(), &done);
    EXPECT_TRUE(done.isInitialNode());
    EXPECT_FALSE(idle.isInitialNode());
}

TEST(EFSMNetworkUndo, StateDeletionUndoRedoRestoresIncidentTransitionsAndManagerIdentity) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "StateDeleteNetwork");
    auto* adapter = new ModalModelDefault(model, "StateDeleteAdapter");
    adapter->setNetwork(&network);
    auto* sink = new Dispose(model, "StateDeleteSink");
    adapter->getConnectionManager()->insert(sink);
    auto* source = new Create(model, "StateDeleteSource");
    source->getConnectionManager()->insert(adapter);
    auto* idle = new FSMState(model, "Idle");
    auto* busy = new FSMState(model, "Busy");
    network.setInitialState(idle);
    network.addState(busy);
    auto* intoBusy = new EFSMTransition(idle, busy, "to-busy");
    auto* outOfBusy = new EFSMTransition(busy, idle, "to-idle");
    auto* parallel = new EFSMTransition(idle, busy, "parallel");
    network.addTransition(intoBusy);
    network.addTransition(outOfBusy);
    network.addTransition(parallel);

    const unsigned long busyId = static_cast<unsigned long>(busy->getId());
    QUndoStack stack;
    stack.push(new EFSMStateMembershipUndoCommand(model, &network, busy));
    ASSERT_EQ(network.getStates()->size(), 1u);
    EXPECT_EQ(network.getTransitions()->size(), 0u);
    EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<FSMState>(), busyId), nullptr);
    EXPECT_EQ(network.getInitialState(), idle);
    (void)model->check(); // The fixture exercises orphan scanning; unrelated model-check diagnostics do not gate identity.
    EXPECT_EQ(busy->getName(), "Busy");
    EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<FSMState>(), busyId), nullptr);

    stack.undo();
    ASSERT_EQ(network.getStates()->size(), 2u);
    ASSERT_EQ(network.getTransitions()->size(), 3u);
    EXPECT_EQ(network.getTransitions()->getAtRank(0), intoBusy);
    EXPECT_EQ(network.getTransitions()->getAtRank(1), outOfBusy);
    EXPECT_EQ(network.getTransitions()->getAtRank(2), parallel);
    EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<FSMState>(), busyId), busy);
    EXPECT_EQ(busy->getTransitions()->size(), 1u);
    (void)model->check();
    EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<FSMState>(), busyId), busy);

    stack.redo();
    EXPECT_EQ(network.getStates()->size(), 1u);
    EXPECT_TRUE(network.getTransitions()->empty());
    stack.undo();
    EXPECT_EQ(network.getStates()->size(), 2u);
    EXPECT_EQ(network.getTransitions()->size(), 3u);
}

TEST(EFSMNetworkUndo, StateCreationUndoRedoKeepsModelManagerAndNetworkMembershipAligned) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "StateCreateNetwork");
    auto* adapter = new ModalModelDefault(model, "StateCreateAdapter");
    adapter->setNetwork(&network);
    auto* sink = new Dispose(model, "StateCreateSink");
    adapter->getConnectionManager()->insert(sink);

    auto state = std::make_unique<FSMState>(model, "Created");
    FSMState* statePointer = state.get();
    const unsigned long stateId = static_cast<unsigned long>(statePointer->getId());
    QUndoStack stack;
    stack.push(new EFSMStateMembershipUndoCommand(model, &network, std::move(state)));
    ASSERT_EQ(network.getStates()->size(), 1u);
    EXPECT_EQ(network.getStates()->front(), statePointer);
    EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<FSMState>(), stateId), statePointer);

    stack.undo();
    EXPECT_TRUE(network.getStates()->empty());
    EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<FSMState>(), stateId), nullptr);

    stack.redo();
    ASSERT_EQ(network.getStates()->size(), 1u);
    EXPECT_EQ(network.getStates()->front(), statePointer);
    EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<FSMState>(), stateId), statePointer);
}

TEST(EFSMNetworkUndo, DeletingInitialStateSelectsReplacementAndRejectsDeletingLastState) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "InitialDeleteNetwork");
    auto* first = new FSMState(model, "First");
    auto* second = new FSMState(model, "Second");
    network.setInitialState(first);
    network.addState(second);

    QUndoStack stack;
    stack.push(new EFSMStateMembershipUndoCommand(model, &network, first));
    EXPECT_EQ(network.getInitialState(), second);
    EXPECT_FALSE(first->isInitialNode());
    EXPECT_TRUE(second->isInitialNode());
    stack.undo();
    EXPECT_EQ(network.getInitialState(), first);
    EXPECT_TRUE(first->isInitialNode());
    EXPECT_FALSE(second->isInitialNode());

    QUndoStack singleStateStack;
    EFSMNetwork singleStateNetwork(model, "SingleStateNetwork");
    auto* only = new FSMState(model, "Only");
    singleStateNetwork.addState(only);
    singleStateStack.push(new EFSMStateMembershipUndoCommand(model, &singleStateNetwork, only));
    EXPECT_EQ(singleStateNetwork.getStates()->size(), 1u);
    EXPECT_EQ(singleStateNetwork.getStates()->front(), only);
    EXPECT_EQ(singleStateStack.count(), 0);
}

TEST(EFSMNetworkUndo, StatePropertiesUndoRedoPreservesNameActionsAndFinalFlag) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "StatePropertiesNetwork");
    auto* adapter = new ModalModelDefault(model, "StatePropertiesAdapter");
    adapter->setNetwork(&network);
    auto* state = new FSMState(model, "Working");
    network.addState(state);
    state->setEntryActionExpression("entered = 1");
    state->setExitActionExpression("left = 1");
    state->setFinalNode(false);

    EFSMStatePropertiesUndoCommand::Properties updated;
    updated.name = "Complete";
    updated.entryActionExpression = "entered = 2";
    updated.exitActionExpression = "left = 2";
    updated.finalNode = true;
    QUndoStack stack;
    stack.push(new EFSMStatePropertiesUndoCommand(&network, state, updated));
    EXPECT_EQ(state->getName(), "Complete");
    EXPECT_EQ(state->getEntryActionExpression(), "entered = 2");
    EXPECT_EQ(state->getExitActionExpression(), "left = 2");
    EXPECT_TRUE(state->isFinalNode());

    stack.undo();
    EXPECT_EQ(state->getName(), "Working");
    EXPECT_EQ(state->getEntryActionExpression(), "entered = 1");
    EXPECT_EQ(state->getExitActionExpression(), "left = 1");
    EXPECT_FALSE(state->isFinalNode());

    stack.redo();
    EXPECT_EQ(state->getName(), "Complete");
    EXPECT_TRUE(state->isFinalNode());
}

TEST(EFSMNetworkEditor, SelectedStateCanBeEditedAndUndoneThroughTheMainWindowAction) {
    MainWindow window;
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelNew_triggered", Qt::DirectConnection));
    Simulator* simulator = window.myScene()->getSimulator();
    ASSERT_NE(simulator, nullptr);
    simulator->getPluginManager()->autoInsertPlugins();
    Model* model = simulator->getModelManager()->current();
    ASSERT_NE(model, nullptr);
    auto* network = new EFSMNetwork(model, "InteractiveStateNetwork");
    auto* state = new FSMState(model, "Working");
    network->addState(state);

    ModelGraphicsScene* scene = window.myScene();
    scene->setModelLevelFilter(static_cast<unsigned int>(network->getId()));
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(simulator, scene);
    GraphicalModelDataDefinition* graphicalState = scene->findGraphicalModelDataDefinition(state);
    ASSERT_NE(graphicalState, nullptr);
    graphicalState->setSelected(true);

    QAction* editAction = nullptr;
    for (QAction* action : window.findChildren<QAction*>()) {
        if (action->text() == QStringLiteral("Edit EFSM State...")) {
            editAction = action;
            break;
        }
    }
    ASSERT_NE(editAction, nullptr);
    QTimer::singleShot(0, []() {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        if (dialog == nullptr) {
            return;
        }
        const auto edits = dialog->findChildren<QLineEdit*>();
        const auto checks = dialog->findChildren<QCheckBox*>();
        const auto buttons = dialog->findChildren<QDialogButtonBox*>();
        if (edits.size() == 3 && checks.size() == 1 && buttons.size() == 1) {
            edits.at(0)->setText(QStringLiteral("Complete"));
            edits.at(1)->setText(QStringLiteral("entered = 1"));
            edits.at(2)->setText(QStringLiteral("left = 1"));
            checks.at(0)->setChecked(true);
            buttons.at(0)->button(QDialogButtonBox::Ok)->click();
        }
    });
    editAction->trigger();
    EXPECT_EQ(state->getName(), "Complete");
    EXPECT_EQ(state->getEntryActionExpression(), "entered = 1");
    EXPECT_TRUE(state->isFinalNode());

    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionEditUndo_triggered", Qt::DirectConnection));
    EXPECT_EQ(state->getName(), "Working");
    EXPECT_FALSE(state->isFinalNode());
}

TEST(EFSMNetworkEditor, SelectedTransitionCanBeEditedAndUndoneThroughTheMainWindowAction) {
    MainWindow window;
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelNew_triggered", Qt::DirectConnection));
    Simulator* simulator = window.myScene()->getSimulator();
    ASSERT_NE(simulator, nullptr);
    simulator->getPluginManager()->autoInsertPlugins();
    Model* model = simulator->getModelManager()->current();
    ASSERT_NE(model, nullptr);
    auto* network = new EFSMNetwork(model, "InteractiveTransitionNetwork");
    auto* source = new FSMState(model, "Source");
    auto* destination = new FSMState(model, "Destination");
    network->addState(source);
    network->addState(destination);
    auto* transition = new EFSMTransition(source, destination, "Before");
    network->addTransition(transition);
    ModelGraphicsScene* scene = window.myScene();
    scene->setModelLevelFilter(static_cast<unsigned int>(network->getId()));
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(simulator, scene);
    EFSMNetworkDiagramProjection::synchronize(simulator, scene);
    ASSERT_EQ(scene->getEFSMNetworkDiagramItems()->size(), 1);
    auto* graphicalTransition = dynamic_cast<EFSMTransitionGraphicsItem*>(scene->getEFSMNetworkDiagramItems()->first());
    ASSERT_NE(graphicalTransition, nullptr);
    graphicalTransition->setSelected(true);

    QAction* editAction = nullptr;
    for (QAction* action : window.findChildren<QAction*>()) {
        if (action->text() == QStringLiteral("Edit EFSM Transition...")) editAction = action;
    }
    ASSERT_NE(editAction, nullptr);
    ASSERT_TRUE(editAction->isEnabled());
    QTimer::singleShot(0, []() {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        if (dialog == nullptr) return;
        const auto edits = dialog->findChildren<QLineEdit*>();
        const auto priorities = dialog->findChildren<QSpinBox*>();
        const auto buttons = dialog->findChildren<QDialogButtonBox*>();
        if (edits.size() >= 7 && priorities.size() == 1 && buttons.size() == 1) {
            edits.at(0)->setText(QStringLiteral("After"));
            edits.at(1)->setText(QStringLiteral("guard"));
            edits.at(2)->setText(QStringLiteral("output"));
            edits.at(3)->setText(QStringLiteral("input"));
            edits.at(5)->setText(QStringLiteral("trigger"));
            edits.at(6)->setText(QStringLiteral("probability"));
            priorities.at(0)->setValue(7);
            buttons.at(0)->button(QDialogButtonBox::Ok)->click();
        }
    });
    editAction->trigger();
    EXPECT_EQ(transition->getName(), "After");
    EXPECT_EQ(transition->getGuardExpression(), "guard");
    EXPECT_EQ(transition->getOutputExpression(), "output");
    EXPECT_EQ(transition->getInputEvent(), "input");
    EXPECT_EQ(transition->getTriggerEvent(), "trigger");
    EXPECT_EQ(transition->getProbabilityExpression(), "probability");
    EXPECT_EQ(transition->getPriority(), 7u);
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionEditUndo_triggered", Qt::DirectConnection));
    EXPECT_EQ(transition->getName(), "Before");
    EXPECT_TRUE(transition->getGuardExpression().empty());
    EXPECT_EQ(transition->getPriority(), 0u);
}

TEST(EFSMNetworkEditor, SharedNetworkOpensOneSubmodelTabFromMultipleAdapters) {
    MainWindow window;
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelNew_triggered", Qt::DirectConnection));
    Simulator* simulator = window.myScene()->getSimulator();
    ASSERT_NE(simulator, nullptr);
    simulator->getPluginManager()->autoInsertPlugins();
    Model* model = simulator->getModelManager()->current();
    ASSERT_NE(model, nullptr);
    auto* network = new EFSMNetwork(model, "SharedNavigationNetwork");
    auto* state = new FSMState(model, "Idle");
    network->addState(state);
    auto* firstAdapter = new ModalModelDefault(model, "FirstAdapter");
    auto* secondAdapter = new ModalModelDefault(model, "SecondAdapter");
    firstAdapter->setNetwork(network);
    secondAdapter->setNetwork(network);

    auto* view = window.findChild<ModelGraphicsView*>();
    ASSERT_NE(view, nullptr);
    std::map<std::string, QColor> colors;
    GraphicalModelBuilder builder(simulator, view, window.myScene(), &colors, nullptr);
    builder.generateGraphicalModelFromModel();
    QTabWidget* tabs = window.findChild<QTabWidget*>(QStringLiteral("tabWidgetModelGraphics"));
    ASSERT_NE(tabs, nullptr);
    const int rootCount = tabs->count();
    ASSERT_GE(rootCount, 1);

    auto graphicalAdapter = [scene = window.myScene()](ModalModelDefault* adapter) {
        for (QGraphicsItem* item : scene->items()) {
            auto* graphical = dynamic_cast<GraphicalModelComponent*>(item);
            if (graphical != nullptr && graphical->getComponent() == adapter) {
                return graphical;
            }
        }
        return static_cast<GraphicalModelComponent*>(nullptr);
    };
    auto* firstItem = graphicalAdapter(firstAdapter);
    auto* secondItem = graphicalAdapter(secondAdapter);
    ASSERT_NE(firstItem, nullptr);
    ASSERT_NE(secondItem, nullptr);

    QAction* openAction = nullptr;
    for (QAction* action : window.findChildren<QAction*>()) {
        if (action->text() == QStringLiteral("Open Submodel")) {
            openAction = action;
            break;
        }
    }
    ASSERT_NE(openAction, nullptr);
    firstItem->setSelected(true);
    EXPECT_TRUE(openAction->isEnabled());
    openAction->trigger();
    ASSERT_EQ(tabs->count(), rootCount + 1);
    auto* networkView = dynamic_cast<ModelGraphicsView*>(tabs->currentWidget());
    ASSERT_NE(networkView, nullptr);
    ASSERT_TRUE(networkView->getScene()->hasModelLevelFilter());
    EXPECT_EQ(networkView->getScene()->modelLevelFilter(), static_cast<unsigned int>(network->getId()));
    EXPECT_NE(networkView->getScene()->findGraphicalModelDataDefinition(state), nullptr);
    EXPECT_EQ(networkView->getScene()->getEFSMNetworkDiagramItems()->size(), 0);
    QAction* createStateAction = nullptr;
    for (QAction* action : window.findChildren<QAction*>()) {
        if (action->text() == QStringLiteral("Create EFSM State")) createStateAction = action;
    }
    ASSERT_NE(createStateAction, nullptr);
    EXPECT_TRUE(createStateAction->isEnabled());

    const int networkTabIndex = tabs->currentIndex();
    int rootTabIndex = -1;
    for (int index = 0; index < tabs->count(); ++index) {
        auto* candidateView = dynamic_cast<ModelGraphicsView*>(tabs->widget(index));
        if (candidateView == nullptr || candidateView->getScene() == nullptr
            || !candidateView->getScene()->hasModelLevelFilter()
            || candidateView->getScene()->modelLevelFilter() != 0) {
            continue;
        }
        for (QGraphicsItem* item : candidateView->getScene()->items()) {
            auto* graphical = dynamic_cast<GraphicalModelComponent*>(item);
            if (graphical != nullptr && graphical->getComponent() == firstAdapter) {
                rootTabIndex = index;
                break;
            }
        }
        if (rootTabIndex >= 0) {
            break;
        }
    }
    ASSERT_GE(rootTabIndex, 0);
    tabs->setCurrentIndex(rootTabIndex);
    ASSERT_TRUE(window.myScene()->hasModelLevelFilter());
    ASSERT_EQ(window.myScene()->modelLevelFilter(), 0u);
    window.myScene()->clearSelection();
    secondItem = graphicalAdapter(secondAdapter);
    ASSERT_NE(secondItem, nullptr);
    secondItem->setSelected(true);
    EXPECT_TRUE(openAction->isEnabled());
    openAction->trigger();
    QApplication::processEvents();
    EXPECT_EQ(tabs->count(), rootCount + 1);
    EXPECT_EQ(tabs->currentWidget(), networkView);
    EXPECT_EQ(tabs->currentIndex(), tabs->indexOf(networkView));
    EXPECT_EQ(networkTabIndex, tabs->indexOf(networkView));
    EXPECT_EQ(model->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<EFSMNetwork>()), 1u);

    ASSERT_TRUE(QMetaObject::invokeMethod(tabs, "tabCloseRequested", Qt::DirectConnection,
                                         Q_ARG(int, tabs->indexOf(networkView))));
    QApplication::processEvents();
    EXPECT_EQ(tabs->count(), rootCount);
    int activeRootIndex = -1;
    for (int index = 0; index < tabs->count(); ++index) {
        auto* candidateView = dynamic_cast<ModelGraphicsView*>(tabs->widget(index));
        if (candidateView == nullptr || candidateView->getScene() == nullptr) {
            continue;
        }
        for (QGraphicsItem* item : candidateView->getScene()->items()) {
            auto* graphical = dynamic_cast<GraphicalModelComponent*>(item);
            if (graphical != nullptr && graphical->getComponent() == firstAdapter) {
                activeRootIndex = index;
                break;
            }
        }
        if (activeRootIndex >= 0) {
            break;
        }
    }
    ASSERT_GE(activeRootIndex, 0);
    tabs->setCurrentIndex(activeRootIndex);
    window.myScene()->clearSelection();
    firstItem = graphicalAdapter(firstAdapter);
    ASSERT_NE(firstItem, nullptr);
    firstItem->setSelected(true);
    ASSERT_TRUE(openAction->isEnabled());
    openAction->trigger();
    EXPECT_EQ(tabs->count(), rootCount + 1);
    auto* reopenedView = dynamic_cast<ModelGraphicsView*>(tabs->currentWidget());
    ASSERT_NE(reopenedView, nullptr);
    EXPECT_EQ(reopenedView->getScene()->modelLevelFilter(), static_cast<unsigned int>(network->getId()));

    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelNew_triggered", Qt::DirectConnection));
    Model* secondModel = simulator->getModelManager()->current();
    ASSERT_NE(secondModel, nullptr);
    EXPECT_NE(secondModel, model);
    EXPECT_EQ(tabs->indexOf(reopenedView), networkTabIndex);
    tabs->setCurrentWidget(reopenedView);
    QApplication::processEvents();
    EXPECT_EQ(simulator->getModelManager()->current(), model);
    EXPECT_EQ(reopenedView->getScene()->getSimulator(), simulator);
    EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<EFSMNetwork>(), network->getId()), network);

    const unsigned int modelCountBeforeClose = simulator->getModelManager()->size();
    ASSERT_GT(modelCountBeforeClose, 1u);
    QTimer closePromptDriver;
    QObject::connect(&closePromptDriver, &QTimer::timeout, [&closePromptDriver]() {
        auto* prompt = qobject_cast<QMessageBox*>(QApplication::activeModalWidget());
        if (prompt != nullptr && prompt->button(QMessageBox::No) != nullptr) {
            closePromptDriver.stop();
            prompt->button(QMessageBox::No)->click();
        }
    });
    closePromptDriver.start(5);
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelClose_triggered", Qt::DirectConnection));
    closePromptDriver.stop();
    QApplication::processEvents();
    EXPECT_FALSE(simulator->getModelManager()->hasModel(model));
    EXPECT_EQ(simulator->getModelManager()->size(), modelCountBeforeClose - 1);
    EXPECT_EQ(tabs->indexOf(reopenedView), -1);
    EXPECT_NE(simulator->getModelManager()->current(), nullptr);
    EXPECT_NE(simulator->getModelManager()->current(), model);
}

TEST(EFSMNetworkEditor, AdapterWithoutNetworkCannotOpenASubmodelContext) {
    MainWindow window;
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelNew_triggered", Qt::DirectConnection));
    Simulator* simulator = window.myScene()->getSimulator();
    ASSERT_NE(simulator, nullptr);
    simulator->getPluginManager()->autoInsertPlugins();
    Model* model = simulator->getModelManager()->current();
    ASSERT_NE(model, nullptr);
    auto* adapter = new ModalModelDefault(model, "UnattachedAdapter");
    auto* source = new Create(model, "UnattachedSource");
    auto* sink = new Dispose(model, "UnattachedSink");
    source->getConnectionManager()->insert(adapter);
    adapter->getConnectionManager()->insert(sink);

    auto* view = window.findChild<ModelGraphicsView*>();
    ASSERT_NE(view, nullptr);
    std::map<std::string, QColor> colors;
    GraphicalModelBuilder builder(simulator, view, window.myScene(), &colors, nullptr);
    builder.generateGraphicalModelFromModel();
    GraphicalModelComponent* graphicalAdapter = nullptr;
    for (QGraphicsItem* item : window.myScene()->items()) {
        auto* graphical = dynamic_cast<GraphicalModelComponent*>(item);
        if (graphical != nullptr && graphical->getComponent() == adapter) {
            graphicalAdapter = graphical;
            break;
        }
    }
    ASSERT_NE(graphicalAdapter, nullptr);
    QAction* openAction = nullptr;
    for (QAction* action : window.findChildren<QAction*>()) {
        if (action->text() == QStringLiteral("Open Submodel")) {
            openAction = action;
            break;
        }
    }
    ASSERT_NE(openAction, nullptr);
    QTabWidget* tabs = window.findChild<QTabWidget*>(QStringLiteral("tabWidgetModelGraphics"));
    ASSERT_NE(tabs, nullptr);
    const int tabCount = tabs->count();
    graphicalAdapter->setSelected(true);
    EXPECT_FALSE(openAction->isEnabled());
    openAction->trigger();
    EXPECT_EQ(tabs->count(), tabCount);
    EXPECT_EQ(adapter->getNetwork(), nullptr);
}

TEST(EFSMNetworkEditor, CreateNetworkActionAttachesANetworkWithoutRejectingAnIncompleteModel) {
    MainWindow window;
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelNew_triggered", Qt::DirectConnection));
    Simulator* simulator = window.myScene()->getSimulator();
    ASSERT_NE(simulator, nullptr);
    simulator->getPluginManager()->autoInsertPlugins();
    Model* model = simulator->getModelManager()->current();
    ASSERT_NE(model, nullptr);
    auto* existingNetwork = new EFSMNetwork(model, "TakenNetwork");
    auto* source = new Create(model, "NetworkCreateSource");
    auto* adapter = new ModalModelDefault(model, "NetworkCreateAdapter");
    auto* sink = new Dispose(model, "NetworkCreateSink");
    source->getConnectionManager()->insert(adapter);
    adapter->getConnectionManager()->insert(sink);
    auto* view = window.findChild<ModelGraphicsView*>();
    ASSERT_NE(view, nullptr);
    std::map<std::string, QColor> colors;
    GraphicalModelBuilder builder(simulator, view, window.myScene(), &colors, nullptr);
    builder.generateGraphicalModelFromModel();

    GraphicalModelComponent* graphicalAdapter = nullptr;
    for (QGraphicsItem* item : window.myScene()->items()) {
        auto* graphical = dynamic_cast<GraphicalModelComponent*>(item);
        if (graphical != nullptr && graphical->getComponent() == adapter) {
            graphicalAdapter = graphical;
            break;
        }
    }
    ASSERT_NE(graphicalAdapter, nullptr);
    graphicalAdapter->setSelected(true);
    QAction* createAction = nullptr;
    for (QAction* action : window.findChildren<QAction*>()) {
        if (action->text() == QStringLiteral("Create EFSM Network")) {
            createAction = action;
            break;
        }
    }
    ASSERT_NE(createAction, nullptr);
    QAction* openAction = nullptr;
    for (QAction* action : window.findChildren<QAction*>()) {
        if (action->text() == QStringLiteral("Open Submodel")) openAction = action;
    }
    ASSERT_NE(openAction, nullptr);
    EXPECT_FALSE(openAction->isEnabled());

    auto attemptNetworkCreation = [&](const QString& name, bool acceptPrompt) {
        bool promptHandled = false;
        bool warningShown = false;
        QTimer dialogDriver;
        QObject::connect(&dialogDriver, &QTimer::timeout, [&]() {
            QWidget* active = QApplication::activeModalWidget();
            if (auto* prompt = qobject_cast<QInputDialog*>(active)) {
                if (!promptHandled) {
                    promptHandled = true;
                    prompt->setTextValue(name);
                    auto* buttons = prompt->findChild<QDialogButtonBox*>();
                    buttons->button(acceptPrompt ? QDialogButtonBox::Ok : QDialogButtonBox::Cancel)->click();
                }
            } else if (auto* warning = qobject_cast<QMessageBox*>(active)) {
                warningShown = true;
                warning->accept();
            }
        });
        dialogDriver.start(5);
        createAction->trigger();
        dialogDriver.stop();
        return std::pair{promptHandled, warningShown};
    };

    const auto [cancelPromptHandled, cancelWarningShown] =
        attemptNetworkCreation(QStringLiteral("CanceledNetwork"), false);
    EXPECT_TRUE(cancelPromptHandled);
    EXPECT_FALSE(cancelWarningShown);
    EXPECT_EQ(adapter->getNetwork(), nullptr);
    EXPECT_EQ(model->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<EFSMNetwork>()), 1u);

    const auto [conflictPromptHandled, conflictWarningShown] =
        attemptNetworkCreation(QStringLiteral("TakenNetwork"), true);
    EXPECT_TRUE(conflictPromptHandled);
    EXPECT_TRUE(conflictWarningShown);
    EXPECT_EQ(adapter->getNetwork(), nullptr);
    EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<EFSMNetwork>(), existingNetwork->getId()),
              existingNetwork);
    EXPECT_EQ(model->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<EFSMNetwork>()), 1u);

    const auto [createPromptHandled, createWarningShown] =
        attemptNetworkCreation(QStringLiteral("CreatedNetwork"), true);
    EXPECT_TRUE(createPromptHandled);
    EXPECT_FALSE(createWarningShown);
    ASSERT_NE(adapter->getNetwork(), nullptr);
    EXPECT_EQ(adapter->getNetwork()->getName(), "CreatedNetwork");
    EXPECT_EQ(model->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<EFSMNetwork>()), 2u);
    window.myScene()->clearSelection();
    graphicalAdapter = nullptr;
    for (QGraphicsItem* item : window.myScene()->items()) {
        auto* graphical = dynamic_cast<GraphicalModelComponent*>(item);
        if (graphical != nullptr && graphical->getComponent() == adapter) {
            graphicalAdapter = graphical;
            break;
        }
    }
    ASSERT_NE(graphicalAdapter, nullptr);
    graphicalAdapter->setSelected(true);
    ASSERT_TRUE(openAction->isEnabled());
    openAction->trigger();
    ASSERT_TRUE(window.myScene()->hasModelLevelFilter());
    EXPECT_EQ(window.myScene()->modelLevelFilter(), static_cast<unsigned int>(adapter->getNetwork()->getId()));
    EXPECT_TRUE(window.myScene()->getGraphicalModelDataDefinitions()->isEmpty());
}

TEST(EFSMNetworkEditor, SelectNetworkActionAssociatesTheChosenIdentityWithoutGlobalValidation) {
    MainWindow window;
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelNew_triggered", Qt::DirectConnection));
    Simulator* simulator = window.myScene()->getSimulator();
    ASSERT_NE(simulator, nullptr);
    simulator->getPluginManager()->autoInsertPlugins();
    Model* model = simulator->getModelManager()->current();
    ASSERT_NE(model, nullptr);
    auto* first = new EFSMNetwork(model, "DuplicateLabel");
    auto* second = new EFSMNetwork(model, "DuplicateLabel");
    auto* adapter = new ModalModelDefault(model, "NetworkSelectAdapter");
    adapter->setNetwork(first);
    auto* source = new Create(model, "NetworkSelectSource");
    auto* sink = new Dispose(model, "NetworkSelectSink");
    source->getConnectionManager()->insert(adapter);
    adapter->getConnectionManager()->insert(sink);

    auto* view = window.findChild<ModelGraphicsView*>();
    ASSERT_NE(view, nullptr);
    std::map<std::string, QColor> colors;
    GraphicalModelBuilder builder(simulator, view, window.myScene(), &colors, nullptr);
    builder.generateGraphicalModelFromModel();
    GraphicalModelComponent* graphicalAdapter = nullptr;
    for (QGraphicsItem* item : window.myScene()->items()) {
        auto* graphical = dynamic_cast<GraphicalModelComponent*>(item);
        if (graphical != nullptr && graphical->getComponent() == adapter) {
            graphicalAdapter = graphical;
            break;
        }
    }
    ASSERT_NE(graphicalAdapter, nullptr);
    QAction* selectAction = nullptr;
    for (QAction* action : window.findChildren<QAction*>()) {
        if (action->text() == QStringLiteral("Select EFSM Network")) selectAction = action;
    }
    ASSERT_NE(selectAction, nullptr);
    graphicalAdapter->setSelected(true);
    ASSERT_TRUE(selectAction->isEnabled());

    QTimer::singleShot(0, [second]() {
        auto* prompt = qobject_cast<QInputDialog*>(QApplication::activeModalWidget());
        if (prompt == nullptr) return;
        auto* combo = prompt->findChild<QComboBox*>();
        auto* buttons = prompt->findChild<QDialogButtonBox*>();
        if (combo == nullptr || buttons == nullptr) return;
        combo->setCurrentIndex(combo->findText(QStringLiteral("%1 [#%2]")
                                                    .arg(QString::fromStdString(second->getName()))
                                                    .arg(second->getId())));
        buttons->button(QDialogButtonBox::Ok)->click();
    });
    selectAction->trigger();

    EXPECT_EQ(adapter->getNetwork(), second);
    EXPECT_NE(adapter->getNetwork(), first);
    EXPECT_EQ(model->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<EFSMNetwork>()), 2u);
    EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<EFSMNetwork>(), first->getId()), first);
}

TEST(EFSMNetworkEditor, StatesAndTransitionsCanBeCreatedBeforeTheEFSMIsComplete) {
    MainWindow window;
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelNew_triggered", Qt::DirectConnection));
    Simulator* simulator = window.myScene()->getSimulator();
    ASSERT_NE(simulator, nullptr);
    simulator->getPluginManager()->autoInsertPlugins();
    Model* model = simulator->getModelManager()->current();
    ASSERT_NE(model, nullptr);
    auto* network = new EFSMNetwork(model, "IncrementalNetwork");
    auto* adapter = new ModalModelDefault(model, "IncrementalAdapter");
    adapter->setNetwork(network);
    auto* view = window.findChild<ModelGraphicsView*>();
    ASSERT_NE(view, nullptr);
    std::map<std::string, QColor> colors;
    GraphicalModelBuilder builder(simulator, view, window.myScene(), &colors, nullptr);
    builder.generateGraphicalModelFromModel();
    ModelGraphicsScene* scene = window.myScene();
    ASSERT_NE(scene, nullptr);
    scene->setModelLevelFilter(static_cast<unsigned int>(network->getId()));

    QAction* createState = nullptr;
    QAction* createTransition = nullptr;
    for (QAction* action : window.findChildren<QAction*>()) {
        if (action->text() == QStringLiteral("Create EFSM State")) createState = action;
        if (action->text() == QStringLiteral("Create EFSM Transition")) createTransition = action;
    }
    ASSERT_NE(createState, nullptr);
    ASSERT_NE(createTransition, nullptr);
    // Isolate the action handler after installing its network context. The real
    // navigation path and action-state synchronization have separate coverage.
    createState->setEnabled(true);
    createTransition->setEnabled(true);
    auto enterText = [](QAction* action, const QString& value) {
        QTimer::singleShot(0, [value]() {
            auto* prompt = qobject_cast<QInputDialog*>(QApplication::activeModalWidget());
            if (prompt != nullptr) {
                prompt->setTextValue(value);
                prompt->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Ok)->click();
            }
        });
        action->trigger();
    };
    enterText(createState, QStringLiteral("Idle"));
    enterText(createState, QStringLiteral("Done"));
    ASSERT_EQ(network->getStates()->size(), 2u);
    auto* idle = dynamic_cast<FSMState*>(model->getDataManager()->getDataDefinition(
        Util::TypeOf<FSMState>(), std::string("Idle")));
    auto* done = dynamic_cast<FSMState*>(model->getDataManager()->getDataDefinition(
        Util::TypeOf<FSMState>(), std::string("Done")));
    ASSERT_NE(idle, nullptr);
    ASSERT_NE(done, nullptr);
    scene->clearSelection();
    ASSERT_NE(scene->findGraphicalModelDataDefinition(idle), nullptr);
    ASSERT_NE(scene->findGraphicalModelDataDefinition(done), nullptr);
    scene->findGraphicalModelDataDefinition(idle)->setSelected(true);
    scene->findGraphicalModelDataDefinition(done)->setSelected(true);
    ASSERT_EQ(scene->selectedItems().size(), 2);
    QTimer::singleShot(0, []() {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        if (dialog == nullptr) return;
        const auto combos = dialog->findChildren<QComboBox*>();
        const auto buttons = dialog->findChildren<QDialogButtonBox*>();
        if (combos.size() == 2 && buttons.size() == 1) {
            combos.at(0)->setCurrentIndex(combos.at(0)->findText(QStringLiteral("Idle")));
            combos.at(1)->setCurrentIndex(combos.at(1)->findText(QStringLiteral("Done")));
            buttons.at(0)->button(QDialogButtonBox::Ok)->click();
        }
    });
    createTransition->trigger();

    ASSERT_EQ(network->getTransitions()->size(), 1u);
    EFSMTransition* transition = network->getTransitions()->front();
    EXPECT_EQ(transition->getSource(), idle);
    EXPECT_EQ(transition->getDestination(), done);
    EXPECT_TRUE(transition->getName().empty());

    scene->clearSelection();
    scene->findGraphicalModelDataDefinition(idle)->setSelected(true);
    scene->findGraphicalModelDataDefinition(done)->setSelected(true);
    QTimer::singleShot(0, []() {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        if (dialog == nullptr) return;
        const auto combos = dialog->findChildren<QComboBox*>();
        const auto edits = dialog->findChildren<QLineEdit*>();
        const auto buttons = dialog->findChildren<QDialogButtonBox*>();
        if (combos.size() == 2 && edits.size() == 1 && buttons.size() == 1) {
            combos.at(0)->setCurrentIndex(combos.at(0)->findText(QStringLiteral("Idle")));
            combos.at(1)->setCurrentIndex(combos.at(1)->findText(QStringLiteral("Done")));
            edits.at(0)->setText(QStringLiteral("Parallel"));
            buttons.at(0)->button(QDialogButtonBox::Ok)->click();
        }
    });
    createTransition->trigger();
    ASSERT_EQ(network->getTransitions()->size(), 2u);
    EFSMTransition* parallel = network->getTransitions()->list()->back();
    EXPECT_EQ(parallel->getSource(), idle);
    EXPECT_EQ(parallel->getDestination(), done);
    EXPECT_EQ(parallel->getName(), "Parallel");

    scene->clearSelection();
    scene->findGraphicalModelDataDefinition(idle)->setSelected(true);
    QTimer::singleShot(0, []() {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        if (dialog == nullptr) return;
        const auto combos = dialog->findChildren<QComboBox*>();
        const auto buttons = dialog->findChildren<QDialogButtonBox*>();
        if (combos.size() == 2 && buttons.size() == 1) {
            EXPECT_FALSE(combos.at(0)->isEnabled());
            EXPECT_FALSE(combos.at(1)->isEnabled());
            buttons.at(0)->button(QDialogButtonBox::Ok)->click();
        }
    });
    createTransition->trigger();
    ASSERT_EQ(network->getTransitions()->size(), 3u);
    EFSMTransition* selfLoop = network->getTransitions()->list()->back();
    EXPECT_EQ(selfLoop->getSource(), idle);
    EXPECT_EQ(selfLoop->getDestination(), idle);
    EXPECT_EQ(model->getDataManager()->getDataDefinition(Util::TypeOf<EFSMNetwork>(), network->getId()), network);
}

TEST(EFSMNetworkEditor, AdapterBindingsCanBeEditedAndUndoneFromTheProcessCanvas) {
    MainWindow window;
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelNew_triggered", Qt::DirectConnection));
    Simulator* simulator = window.myScene()->getSimulator();
    ASSERT_NE(simulator, nullptr);
    simulator->getPluginManager()->autoInsertPlugins();
    Model* model = simulator->getModelManager()->current();
    ASSERT_NE(model, nullptr);
    auto* network = new EFSMNetwork(model, "BindingNetwork");
    network->addInputPort("Arrival");
    network->addOutputPort("Result");
    auto* adapter = new ModalModelDefault(model, "BindingAdapter");
    adapter->setNetwork(network);
    adapter->setOutputBinding(0, "InitialResult");
    auto* source = new Create(model, "BindingSource");
    auto* sink = new Dispose(model, "BindingSink");
    source->getConnectionManager()->insert(adapter);
    adapter->getConnectionManager()->insert(sink);
    auto* view = window.findChild<ModelGraphicsView*>();
    ASSERT_NE(view, nullptr);
    std::map<std::string, QColor> colors;
    GraphicalModelBuilder builder(simulator, view, window.myScene(), &colors, nullptr);
    builder.generateGraphicalModelFromModel();
    GraphicalModelComponent* graphicalAdapter = nullptr;
    for (QGraphicsItem* item : window.myScene()->items()) {
        auto* graphical = dynamic_cast<GraphicalModelComponent*>(item);
        if (graphical != nullptr && graphical->getComponent() == adapter) {
            graphicalAdapter = graphical;
            break;
        }
    }
    ASSERT_NE(graphicalAdapter, nullptr);
    graphicalAdapter->setSelected(true);
    QAction* editBindings = nullptr;
    for (QAction* action : window.findChildren<QAction*>()) {
        if (action->text() == QStringLiteral("Edit EFSM Bindings...")) editBindings = action;
    }
    ASSERT_NE(editBindings, nullptr);
    ASSERT_TRUE(editBindings->isEnabled());
    QTimer dialogDriver;
    QObject::connect(&dialogDriver, &QTimer::timeout, [&dialogDriver]() {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        if (dialog == nullptr) return;
        auto* input = dialog->findChild<QLineEdit*>(QStringLiteral("efsmInputBinding_0"));
        auto* output = dialog->findChild<QLineEdit*>(QStringLiteral("efsmOutputBinding_0"));
        const auto buttons = dialog->findChildren<QDialogButtonBox*>();
        if (input != nullptr && output != nullptr && buttons.size() == 1) {
            input->setText(QStringLiteral("2 + 3"));
            output->setText(QStringLiteral("Outcome"));
            dialogDriver.stop();
            buttons.at(0)->button(QDialogButtonBox::Ok)->click();
        }
    });
    dialogDriver.start(5);
    editBindings->trigger();
    dialogDriver.stop();
    EXPECT_EQ(adapter->getInputBinding(0), "2 + 3");
    EXPECT_EQ(adapter->getOutputBinding(0), "Outcome");
    ASSERT_TRUE(window.myScene()->getUndoStack()->canUndo());
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionEditUndo_triggered", Qt::DirectConnection));
    EXPECT_EQ(adapter->getInputBinding(0), "1");
    EXPECT_EQ(adapter->getOutputBinding(0), "InitialResult");
    EXPECT_EQ(source->getConnectionManager()->size(), 1u);
    EXPECT_EQ(adapter->getConnectionManager()->size(), 1u);
}

TEST(EFSMNetworkEditor, MainWindowSaveCloseAndReopenRestoresEFSMLayoutAndDomain) {
    MainWindow window;
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelNew_triggered", Qt::DirectConnection));
    Simulator* simulator = window.myScene()->getSimulator();
    ASSERT_NE(simulator, nullptr);
    simulator->getPluginManager()->autoInsertPlugins();
    Model* originalModel = simulator->getModelManager()->current();
    ASSERT_NE(originalModel, nullptr);

    auto* adapter = new ModalModelDefault(originalModel, "PersistedGuiAdapter");
    auto* source = new Create(originalModel, "PersistedGuiSource");
    auto* sink = new Dispose(originalModel, "PersistedGuiSink");
    source->getConnectionManager()->insert(adapter);
    adapter->getConnectionManager()->insert(sink);
    auto* rootView = window.findChild<ModelGraphicsView*>();
    ASSERT_NE(rootView, nullptr);
    std::map<std::string, QColor> colors;
    GraphicalModelBuilder rootBuilder(simulator, rootView, window.myScene(), &colors, nullptr);
    rootBuilder.generateGraphicalModelFromModel();

    auto findAdapterItem = [&window, adapter]() {
        for (QGraphicsItem* item : window.myScene()->items()) {
            auto* graphical = dynamic_cast<GraphicalModelComponent*>(item);
            if (graphical != nullptr && graphical->getComponent() == adapter) return graphical;
        }
        return static_cast<GraphicalModelComponent*>(nullptr);
    };
    GraphicalModelComponent* graphicalAdapter = findAdapterItem();
    ASSERT_NE(graphicalAdapter, nullptr);
    QAction* createNetwork = nullptr;
    QAction* openSubmodel = nullptr;
    for (QAction* action : window.findChildren<QAction*>()) {
        if (action->text() == QStringLiteral("Create EFSM Network")) createNetwork = action;
        if (action->text() == QStringLiteral("Open Submodel")) openSubmodel = action;
    }
    ASSERT_NE(createNetwork, nullptr);
    ASSERT_NE(openSubmodel, nullptr);
    graphicalAdapter->setSelected(true);
    ASSERT_TRUE(createNetwork->isEnabled());
    QTimer::singleShot(0, []() {
        auto* dialog = qobject_cast<QInputDialog*>(QApplication::activeModalWidget());
        if (dialog != nullptr) {
            dialog->setTextValue(QStringLiteral("PersistedGuiNetwork"));
            dialog->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Ok)->click();
        }
    });
    createNetwork->trigger();
    auto* network = dynamic_cast<EFSMNetwork*>(adapter->getNetwork());
    ASSERT_NE(network, nullptr);
    EXPECT_EQ(originalModel->getDataManager()->getDataDefinition(
                  Util::TypeOf<EFSMNetwork>(), network->getId()), network);
    window.myScene()->clearSelection();
    graphicalAdapter = findAdapterItem();
    ASSERT_NE(graphicalAdapter, nullptr);
    graphicalAdapter->setSelected(true);
    QApplication::processEvents();
    ASSERT_TRUE(openSubmodel->isEnabled());
    openSubmodel->trigger();

    auto* tabs = window.findChild<QTabWidget*>(QStringLiteral("tabWidgetModelGraphics"));
    ASSERT_NE(tabs, nullptr);
    auto* networkView = dynamic_cast<ModelGraphicsView*>(tabs->currentWidget());
    ASSERT_NE(networkView, nullptr);
    ASSERT_EQ(networkView->getScene()->modelLevelFilter(), static_cast<unsigned int>(network->getId()));
    auto enterStateName = [](QAction* action, const QString& name) {
        QTimer::singleShot(0, [name]() {
            auto* dialog = qobject_cast<QInputDialog*>(QApplication::activeModalWidget());
            if (dialog != nullptr) {
                dialog->setTextValue(name);
                dialog->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Ok)->click();
            }
        });
        action->trigger();
    };
    QAction* createState = nullptr;
    QAction* createTransition = nullptr;
    QAction* setInitial = nullptr;
    QAction* editTransition = nullptr;
    for (QAction* action : window.findChildren<QAction*>()) {
        if (action->text() == QStringLiteral("Create EFSM State")) createState = action;
        if (action->text() == QStringLiteral("Create EFSM Transition")) createTransition = action;
        if (action->text() == QStringLiteral("Set Selected EFSM State as Initial")) setInitial = action;
        if (action->text() == QStringLiteral("Edit EFSM Transition...")) editTransition = action;
    }
    ASSERT_NE(createState, nullptr);
    ASSERT_NE(createTransition, nullptr);
    ASSERT_NE(setInitial, nullptr);
    ASSERT_NE(editTransition, nullptr);
    enterStateName(createState, QStringLiteral("PersistedIdle"));
    enterStateName(createState, QStringLiteral("PersistedDone"));
    auto* idle = findStateByName(network, "PersistedIdle");
    auto* done = findStateByName(network, "PersistedDone");
    ASSERT_NE(idle, nullptr);
    ASSERT_NE(done, nullptr);
    auto* idleItem = networkView->getScene()->findGraphicalModelDataDefinition(idle);
    auto* doneItem = networkView->getScene()->findGraphicalModelDataDefinition(done);
    ASSERT_NE(idleItem, nullptr);
    ASSERT_NE(doneItem, nullptr);
    idleItem->setSelected(true);
    ASSERT_TRUE(setInitial->isEnabled());
    setInitial->trigger();
    EXPECT_EQ(network->getInitialState(), idle);
    networkView->getScene()->clearSelection();
    idleItem->setSelected(true);
    doneItem->setSelected(true);
    QApplication::processEvents();
    EXPECT_EQ(networkView->getScene()->selectedItems().size(), 2);
    ASSERT_TRUE(createTransition->isEnabled());
    QTimer::singleShot(0, []() {
        if (auto* message = qobject_cast<QMessageBox*>(QApplication::activeModalWidget())) {
            message->accept();
            return;
        }
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        if (dialog == nullptr) return;
        const auto combos = dialog->findChildren<QComboBox*>();
        const auto edits = dialog->findChildren<QLineEdit*>();
        const auto buttons = dialog->findChildren<QDialogButtonBox*>();
        if (combos.size() == 2 && !edits.isEmpty() && buttons.size() == 1) {
            combos.at(0)->setCurrentIndex(combos.at(0)->findText(QStringLiteral("PersistedIdle")));
            combos.at(1)->setCurrentIndex(combos.at(1)->findText(QStringLiteral("PersistedDone")));
            edits.at(0)->setText(QStringLiteral("Complete"));
            buttons.at(0)->button(QDialogButtonBox::Ok)->click();
        }
    });
    createTransition->trigger();
    ASSERT_EQ(network->getTransitions()->size(), 1u);
    auto* transition = network->getTransitions()->front();
    EXPECT_EQ(transition->getSource(), idle);
    EXPECT_EQ(transition->getDestination(), done);
    EFSMTransitionGraphicsItem* transitionItem = nullptr;
    for (QGraphicsItem* item : networkView->getScene()->items()) {
        if (auto* candidate = dynamic_cast<EFSMTransitionGraphicsItem*>(item)) {
            transitionItem = candidate;
            break;
        }
    }
    ASSERT_NE(transitionItem, nullptr);
    networkView->getScene()->clearSelection();
    transitionItem->setSelected(true);
    QApplication::processEvents();
    ASSERT_TRUE(editTransition->isEnabled());
    bool transitionEditHandled = false;
    QTimer editDialogDriver;
    QObject::connect(&editDialogDriver, &QTimer::timeout, [&]() {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        if (dialog == nullptr) return;
        const auto edits = dialog->findChildren<QLineEdit*>();
        const auto spins = dialog->findChildren<QSpinBox*>();
        const auto buttons = dialog->findChildren<QDialogButtonBox*>();
        if (edits.size() >= 6 && spins.size() == 1 && buttons.size() == 1) {
            edits.at(1)->setText(QStringLiteral("true"));
            edits.at(2)->setText(QStringLiteral("complete"));
            spins.at(0)->setValue(7);
            transitionEditHandled = true;
            editDialogDriver.stop();
            buttons.at(0)->button(QDialogButtonBox::Ok)->click();
        }
    });
    editDialogDriver.start(5);
    editTransition->trigger();
    editDialogDriver.stop();
    ASSERT_TRUE(transitionEditHandled);
    EXPECT_EQ(transition->getGuardExpression(), "true");
    EXPECT_EQ(transition->getOutputExpression(), "complete");
    EXPECT_EQ(transition->getPriority(), 7u);
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionEditUndo_triggered", Qt::DirectConnection));
    EXPECT_TRUE(transition->getGuardExpression().empty());
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionEditRedo_triggered", Qt::DirectConnection));
    EXPECT_EQ(transition->getGuardExpression(), "true");

    QAction* deleteSelection = nullptr;
    for (QAction* action : window.findChildren<QAction*>()) {
        if (action->objectName() == QStringLiteral("actionEditDelete")) {
            deleteSelection = action;
            break;
        }
    }
    ASSERT_NE(deleteSelection, nullptr);
    networkView->getScene()->clearSelection();
    transitionItem = nullptr;
    for (QGraphicsItem* item : networkView->getScene()->items()) {
        if (auto* candidate = dynamic_cast<EFSMTransitionGraphicsItem*>(item)) {
            transitionItem = candidate;
            break;
        }
    }
    ASSERT_NE(transitionItem, nullptr);
    transitionItem->setSelected(true);
    QApplication::processEvents();
    ASSERT_TRUE(deleteSelection->isEnabled());
    deleteSelection->trigger();
    EXPECT_TRUE(network->getTransitions()->empty());
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionEditUndo_triggered", Qt::DirectConnection));
    ASSERT_EQ(network->getTransitions()->size(), 1u);
    EXPECT_EQ(network->getTransitions()->front(), transition);
    EXPECT_EQ(transition->getGuardExpression(), "true");
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionEditRedo_triggered", Qt::DirectConnection));
    EXPECT_TRUE(network->getTransitions()->empty());
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionEditUndo_triggered", Qt::DirectConnection));
    ASSERT_EQ(network->getTransitions()->front(), transition);

    networkView->getScene()->clearSelection();
    doneItem = networkView->getScene()->findGraphicalModelDataDefinition(done);
    ASSERT_NE(doneItem, nullptr);
    doneItem->setSelected(true);
    QApplication::processEvents();
    ASSERT_TRUE(deleteSelection->isEnabled());
    deleteSelection->trigger();
    ASSERT_EQ(network->getStates()->size(), 1u);
    EXPECT_TRUE(network->getTransitions()->empty());
    EXPECT_EQ(originalModel->getDataManager()->getDataDefinition(
                  Util::TypeOf<FSMState>(), static_cast<unsigned long>(done->getId())), nullptr);
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionEditUndo_triggered", Qt::DirectConnection));
    ASSERT_EQ(network->getStates()->size(), 2u);
    ASSERT_EQ(network->getTransitions()->size(), 1u);
    EXPECT_EQ(network->getStates()->find(done) != network->getStates()->list()->end(), true);
    EXPECT_EQ(originalModel->getDataManager()->getDataDefinition(
                  Util::TypeOf<FSMState>(), static_cast<unsigned long>(done->getId())), done);
    EXPECT_EQ(network->getTransitions()->front(), transition);
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionEditRedo_triggered", Qt::DirectConnection));
    EXPECT_TRUE(network->getTransitions()->empty());
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionEditUndo_triggered", Qt::DirectConnection));
    ASSERT_EQ(network->getTransitions()->front(), transition);

    QUndoStack* editorHistory = networkView->getScene()->getUndoStack();
    ASSERT_NE(editorHistory, nullptr);
    const int mixedCommandCount = editorHistory->count();
    ASSERT_GE(mixedCommandCount, 6);
    for (int command = 0; command < mixedCommandCount; ++command) {
        ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionEditUndo_triggered", Qt::DirectConnection));
    }
    EXPECT_TRUE(network->getStates()->empty());
    EXPECT_TRUE(network->getTransitions()->empty());
    EXPECT_EQ(network->getInitialState(), nullptr);
    for (int command = 0; command < mixedCommandCount; ++command) {
        ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionEditRedo_triggered", Qt::DirectConnection));
    }
    ASSERT_EQ(network->getStates()->size(), 1u);
    EXPECT_TRUE(network->getTransitions()->empty());
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionEditUndo_triggered", Qt::DirectConnection));
    ASSERT_EQ(network->getStates()->size(), 2u);
    ASSERT_EQ(network->getTransitions()->size(), 1u);
    EXPECT_EQ(network->getInitialState(), idle);
    EXPECT_EQ(network->getTransitions()->front(), transition);
    EXPECT_EQ(transition->getGuardExpression(), "true");
    idleItem = networkView->getScene()->findGraphicalModelDataDefinition(idle);
    ASSERT_NE(idleItem, nullptr);
    const QPointF savedPosition(431.25, -127.5);
    idleItem->setPos(savedPosition);

    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QString filename = directory.filePath(QStringLiteral("efsm-editor-roundtrip.gui"));
    bool saveDialogHandled = false;
    bool saveConfirmationHandled = false;
    QTimer saveDialogDriver;
    QObject::connect(&saveDialogDriver, &QTimer::timeout, [&]() {
        QWidget* active = QApplication::activeModalWidget();
        if (auto* dialog = qobject_cast<QFileDialog*>(active)) {
            if (!saveDialogHandled) {
                saveDialogHandled = true;
                dialog->selectFile(filename);
                QMetaObject::invokeMethod(dialog, "accept", Qt::DirectConnection);
            }
        } else if (auto* message = qobject_cast<QMessageBox*>(active)) {
            saveConfirmationHandled = true;
            message->accept();
        }
    });
    saveDialogDriver.start(5);
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelSave_triggered", Qt::DirectConnection));
    saveDialogDriver.stop();
    ASSERT_TRUE(saveDialogHandled);
    ASSERT_TRUE(saveConfirmationHandled);
    ASSERT_TRUE(QFile::exists(filename));
    QFile savedGuiFile(filename);
    ASSERT_TRUE(savedGuiFile.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString savedGuiText = QString::fromUtf8(savedGuiFile.readAll());
    EXPECT_TRUE(savedGuiText.contains(QStringLiteral("position=(431.2500,-127.5000)"))) << savedGuiText.toStdString();
    savedGuiFile.close();

    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelClose_triggered", Qt::DirectConnection));
    EXPECT_FALSE(simulator->getModelManager()->hasModel(originalModel));

    bool openDialogHandled = false;
    bool openConfirmationHandled = false;
    QTimer openDialogDriver;
    QObject::connect(&openDialogDriver, &QTimer::timeout, [&]() {
        QWidget* active = QApplication::activeModalWidget();
        if (auto* dialog = qobject_cast<QFileDialog*>(active)) {
            if (!openDialogHandled) {
                openDialogHandled = true;
                dialog->selectFile(filename);
                QMetaObject::invokeMethod(dialog, "accept", Qt::DirectConnection);
            }
        } else if (auto* message = qobject_cast<QMessageBox*>(active)) {
            openConfirmationHandled = true;
            message->accept();
        }
    });
    openDialogDriver.start(5);
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelOpen_triggered", Qt::DirectConnection));
    openDialogDriver.stop();
    ASSERT_TRUE(openDialogHandled);
    ASSERT_TRUE(openConfirmationHandled);

    Model* restoredModel = simulator->getModelManager()->current();
    ASSERT_NE(restoredModel, nullptr);
    ASSERT_NE(restoredModel, originalModel);
    auto* restoredNetwork = findNetworkByName(restoredModel, "PersistedGuiNetwork");
    ASSERT_NE(restoredNetwork, nullptr);
    ASSERT_EQ(restoredNetwork->getStates()->size(), 2u);
    ASSERT_EQ(restoredNetwork->getTransitions()->size(), 1u);
    ASSERT_NE(restoredNetwork->getInitialState(), nullptr);
    EXPECT_EQ(restoredNetwork->getInitialState()->getName(), "PersistedIdle");
    EXPECT_EQ(restoredNetwork->getTransitions()->front()->getName(), "Complete");
    EXPECT_EQ(restoredNetwork->getTransitions()->front()->getGuardExpression(), "true");
    EXPECT_EQ(restoredNetwork->getTransitions()->front()->getOutputExpression(), "complete");
    EXPECT_EQ(restoredNetwork->getTransitions()->front()->getPriority(), 7u);
    EXPECT_EQ(restoredModel->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<EFSMNetwork>()), 1u);
    const auto parsedLayouts = EFSMNetworkDiagramCodec::parseSection(savedGuiText.split(QLatin1Char('\n')), restoredModel);
    ASSERT_TRUE(parsedLayouts.contains(static_cast<unsigned long>(restoredNetwork->getId())));
    ASSERT_EQ(parsedLayouts.value(static_cast<unsigned long>(restoredNetwork->getId())).states.size(), 2);

    graphicalAdapter = nullptr;
    for (QGraphicsItem* item : window.myScene()->items()) {
        auto* graphical = dynamic_cast<GraphicalModelComponent*>(item);
        if (graphical != nullptr && graphical->getComponent()->getName() == "PersistedGuiAdapter") {
            graphicalAdapter = graphical;
            break;
        }
    }
    ASSERT_NE(graphicalAdapter, nullptr);
    graphicalAdapter->setSelected(true);
    ASSERT_TRUE(openSubmodel->isEnabled());
    openSubmodel->trigger();
    networkView = dynamic_cast<ModelGraphicsView*>(tabs->currentWidget());
    ASSERT_NE(networkView, nullptr);
    ASSERT_TRUE(networkView->getScene()->hasModelLevelFilter());
    ASSERT_EQ(networkView->getScene()->modelLevelFilter(), static_cast<unsigned int>(restoredNetwork->getId()));
    auto* restoredIdle = findStateByName(restoredNetwork, "PersistedIdle");
    ASSERT_NE(restoredIdle, nullptr);
    idleItem = networkView->getScene()->findGraphicalModelDataDefinition(restoredIdle);
    ASSERT_NE(idleItem, nullptr);
    const auto restoredLayout = parsedLayouts.value(static_cast<unsigned long>(restoredNetwork->getId()));
    const auto restoredIdlePosition = std::find_if(
        restoredLayout.states.begin(), restoredLayout.states.end(), [](const auto& statePosition) {
            return statePosition.stateName == QStringLiteral("PersistedIdle");
        });
    ASSERT_NE(restoredIdlePosition, restoredLayout.states.end());
    EXPECT_EQ(restoredIdlePosition->position, savedPosition);
    EXPECT_EQ(idleItem->pos(), savedPosition);
    EXPECT_EQ(restoredModel->getComponentManager()->getAllComponents()->size(), 3u);
    std::map<std::string, ModelComponent*> restoredComponents;
    for (ModelComponent* component : *restoredModel->getComponentManager()->getAllComponents()) {
        if (component != nullptr) {
            restoredComponents.emplace(component->getName(), component);
        }
    }
    ASSERT_TRUE(restoredComponents.contains("PersistedGuiSource"));
    ASSERT_TRUE(restoredComponents.contains("PersistedGuiAdapter"));
    ASSERT_TRUE(restoredComponents.contains("PersistedGuiSink"));
    EXPECT_EQ(restoredComponents.at("PersistedGuiSource")->getConnectionManager()->size(), 1u);
    EXPECT_EQ(restoredComponents.at("PersistedGuiAdapter")->getConnectionManager()->size(), 1u);
    EXPECT_EQ(restoredComponents.at("PersistedGuiSink")->getConnectionManager()->size(), 0u);
}
