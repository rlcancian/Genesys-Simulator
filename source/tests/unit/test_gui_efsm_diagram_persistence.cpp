// Focused tests for the G0.2 EFSM Network diagram GUI overlay persistence:
// EFSMNetworkDiagramCodec plus its integration into
// GraphicalModelSerializer::saveGraphicalModel()/loadGraphicalModel(). These
// tests exercise the real .gui save/load path, not just the codec in
// isolation. The same target also covers the real scene projection and
// transition edit/undo command path.

#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/ModelGraphicsView.h"
#include "graphicals/EFSMTransitionGraphicsItem.h"
#include "actions/EFSMTransitionUndoCommand.h"
#include "actions/EFSMTransitionPropertiesUndoCommand.h"
#include "actions/EFSMInitialStateUndoCommand.h"
#include "controllers/EditCommandController.h"
#include "services/EFSMNetworkDiagramCodec.h"
#include "services/GraphicalModelBuilder.h"
#include "services/GraphicalModelSerializer.h"

#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelDataManager.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "plugins/components/ModalModel/FSMState.h"
#include "plugins/components/ModalModel/ModalModelDefault.h"
#include "plugins/data/ModalModel/EFSMNetwork.h"

#include <gtest/gtest.h>

#include <QAction>
#include <QFile>
#include <QPlainTextEdit>
#include <QSlider>
#include <QTemporaryDir>
#include <QTextEdit>
#include <QUndoStack>

namespace {

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
        static_cast<unsigned long>(loadedNetwork->getId())).isEmpty());
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
    network.addTransition(new EFSMTransition(&idle, &done, "First"));
    network.addTransition(new EFSMTransition(&idle, &done, "Parallel"));
    network.addTransition(new EFSMTransition(&done, &done, "Loop"));

    SerializerHarness harness(&simulator);
    ModelGraphicsScene* scene = harness.graphicsView.getScene();
    ASSERT_NE(scene, nullptr);
    scene->setModelLevelFilter(static_cast<unsigned int>(network.getId()));

    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);
    ASSERT_EQ(scene->getGraphicalModelDataDefinitions()->size(), 2);
    ASSERT_EQ(scene->getEFSMNetworkDiagramItems()->size(), 3);

    auto* first = dynamic_cast<EFSMTransitionGraphicsItem*>(scene->getEFSMNetworkDiagramItems()->at(0));
    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first->ordinal(), 0);
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
    harness.serializer->setEFSMNetworkDiagramLayout(layout.networkId, layout);

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
        harness.serializer->efsmNetworkDiagramLayout(static_cast<unsigned long>(loadedNetwork->getId()));
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
    harness.serializer->setEFSMNetworkDiagramLayout(layout.networkId, layout);

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
        static_cast<unsigned long>(loadedNetwork->getId())).isEmpty());
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
        static_cast<unsigned long>(loadedNetwork->getId())).isEmpty());
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
    EFSMNetworkDiagramLayout layout;
    layout.networkId = static_cast<unsigned long>(network.getId());
    layout.states.append({static_cast<unsigned long>(idle.getId()), "Idle", QPointF(1, 1)});
    layout.states.append({static_cast<unsigned long>(busy.getId()), "Busy", QPointF(2, 2)});
    layout.transitions.append({"Idle", "Busy", "Go", 0, {}, false, QPointF()});
    harness.serializer->setEFSMNetworkDiagramLayout(layout.networkId, layout);

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
