// Focused tests for the G0.2 EFSM Network diagram GUI overlay persistence:
// EFSMNetworkDiagramCodec plus its integration into
// GraphicalModelSerializer::saveGraphicalModel()/loadGraphicalModel(). These
// tests exercise the real .gui save/load path, not just the codec in
// isolation, even though no interactive EFSM viewer exists yet.

#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/ModelGraphicsView.h"
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
    // Two parallel transitions Idle->Busy sharing an empty name, plus one named one.
    network.addTransition(new EFSMTransition(&idle, &busy, ""));
    network.addTransition(new EFSMTransition(&idle, &busy, ""));
    network.addTransition(new EFSMTransition(&idle, &busy, "Go"));
    auto* adapter = new ModalModelDefault(model, "RoundTripAdapter");
    adapter->setNetwork(&network);

    SerializerHarness harness(&simulator);

    EFSMNetworkDiagramLayout layout;
    layout.networkId = static_cast<unsigned long>(network.getId());
    layout.networkName = QString::fromStdString(network.getName());
    layout.states.append({static_cast<unsigned long>(idle.getId()), "Idle", QPointF(10.0, 20.0)});
    layout.states.append({static_cast<unsigned long>(busy.getId()), "Busy", QPointF(310.5, 220.25)});
    layout.transitions.append({"Idle", "Busy", "", 0, {QPointF(50, 60)}, true, QPointF(70, 80)});
    layout.transitions.append({"Idle", "Busy", "", 1, {}, false, QPointF()});
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

    // The two unnamed parallel transitions must remain distinguished by ordinal.
    int unnamedOrdinal0 = -1;
    int unnamedOrdinal1 = -1;
    int namedOrdinal = -1;
    for (const auto& transitionLayout : restored.transitions) {
        if (transitionLayout.transitionName.isEmpty() && transitionLayout.ordinal == 0) {
            unnamedOrdinal0 = transitionLayout.ordinal;
            EXPECT_TRUE(transitionLayout.hasLabelPosition);
            ASSERT_EQ(transitionLayout.controlPoints.size(), 1);
            EXPECT_NEAR(transitionLayout.controlPoints.front().x(), 50.0, 0.001);
        } else if (transitionLayout.transitionName.isEmpty() && transitionLayout.ordinal == 1) {
            unnamedOrdinal1 = transitionLayout.ordinal;
            EXPECT_FALSE(transitionLayout.hasLabelPosition);
        } else if (transitionLayout.transitionName == "Go") {
            namedOrdinal = transitionLayout.ordinal;
        }
    }
    EXPECT_EQ(unnamedOrdinal0, 0);
    EXPECT_EQ(unnamedOrdinal1, 1);
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
