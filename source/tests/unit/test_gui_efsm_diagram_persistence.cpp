// Focused tests for the G0.2 EFSM Network diagram GUI overlay persistence:
// EFSMNetworkDiagramCodec plus its integration into
// GraphicalModelSerializer::saveGraphicalModel()/loadGraphicalModel(). These
// tests exercise the real .gui save/load path, not just the codec in
// isolation. The same target also covers the real scene projection and
// transition edit/undo command path.

#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/ModelGraphicsView.h"
#include "graphicals/EFSMTransitionGraphicsItem.h"
#include "propertyeditor/ObjectPropertyBrowser.h"
#include "graphicals/GraphicalModelComponent.h"
#include "actions/EFSMTransitionUndoCommand.h"
#include "actions/EFSMTransitionPropertiesUndoCommand.h"
#include "actions/EFSMInitialStateUndoCommand.h"
#include "actions/EFSMStateMembershipUndoCommand.h"
#include "actions/EFSMStatePropertiesUndoCommand.h"
#include "controllers/EditCommandController.h"
#include "controllers/PluginCatalogController.h"
#include "mainwindow.h"
#include "extensions/GuiExtensionContracts.h"
#include "extensions/GuiExtensionManager.h"
#include "extensions/GuiExtensionPluginCatalog.h"
#include "services/EFSMNetworkDiagramCodec.h"
#include "services/EFSMNetworkDiagramProjection.h"
#include "services/GraphicalModelBuilder.h"
#include "services/GraphicalModelSerializer.h"

#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Persistence.h"
#include "kernel/simulator/PropertyGenesys.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelDataManager.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "plugins/components/ModalModel/FSMState.h"
#include "plugins/components/ModalModel/ModalModelDefault.h"
#include "plugins/components/ModalModel/Submodel.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Transfer/Goto.h"
#include "plugins/components/Transfer/Label.h"
#include "plugins/data/Transfer/LabelDefinition.h"
#include "kernel/simulator/essentialPlugins/EntityType.h"
#include "kernel/simulator/model/ModelComponentManager.h"
#include "plugins/data/ModalModel/EFSMNetwork.h"
#include "plugins/data/ModalModel/NetworkActivation.h"
#include "plugins/data/ModalModel/GraphNetwork.h"
#include "plugins/data/ModalModel/GraphEdge.h"
#include "plugins/data/ModalModel/GraphNode.h"
#include "plugins/data/ModalModel/DirectedGraphNetwork.h"
#include "plugins/data/ModalModel/DirectedAcyclicGraphNetwork.h"
#include "plugins/data/ModalModel/ColoredPetriNetNetwork.h"
#include "plugins/data/ModalModel/MarkovChainNetwork.h"
#include "plugins/data/ModalModel/MarkovState.h"

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
#include <QTreeWidget>
#include <QInputDialog>
#include <QMessageBox>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsRectItem>
#include <QGraphicsPathItem>
#include <QMainWindow>
#include <QMenu>
#include <QSpinBox>

#include <algorithm>
#include <iterator>
#include <thread>

namespace {

constexpr const char* kEFSMGuiExtensionId = "modal.efsm.gui";

class ReferencePersistenceRuntime final : public Persistence_if {
public:
    bool save(std::string) override { return false; }
    bool load(std::string) override { return false; }
    bool hasChanged() override { return false; }
    void setHasChanged(bool) override {}
    bool getOption(Persistence_if::Options) override { return false; }
    void setOption(Persistence_if::Options, bool) override {}
    std::string getFormatedField(PersistenceRecord*) override { return {}; }
};

class ModalModelDefaultPersistenceProbe final : public ModalModelDefault {
public:
    using ModalModelDefault::ModalModelDefault;
    using ModalModelDefault::_loadInstance;
    using ModalModelDefault::_saveInstance;
    using ModalModelDefault::_check;
};

class ScopedGuiExtensionConfig final {
public:
    explicit ScopedGuiExtensionConfig(const QByteArray& path)
        : _wasSet(qEnvironmentVariableIsSet("GENESYS_GUI_EXTENSIONS_CONFIG")),
          _previous(qgetenv("GENESYS_GUI_EXTENSIONS_CONFIG")) {
        qputenv("GENESYS_GUI_EXTENSIONS_CONFIG", path);
    }
    ~ScopedGuiExtensionConfig() {
        if (_wasSet) qputenv("GENESYS_GUI_EXTENSIONS_CONFIG", _previous);
        else qunsetenv("GENESYS_GUI_EXTENSIONS_CONFIG");
    }
private:
    bool _wasSet;
    QByteArray _previous;
};

void setEFSMNetworkDiagramLayout(GraphicalModelSerializer* serializer, Model* model,
                                 unsigned long networkId, const EFSMNetworkDiagramLayout& layout) {
    serializer->setNetworkDiagramLayout(model, kEFSMGuiExtensionId, networkId, layout);
}

EFSMNetworkDiagramLayout efsmNetworkDiagramLayout(GraphicalModelSerializer* serializer,
                                                   Model* model, unsigned long networkId) {
    const std::any layout = serializer->networkDiagramLayout(model, kEFSMGuiExtensionId, networkId);
    if (const auto* efsm = std::any_cast<EFSMNetworkDiagramLayout>(&layout)) return *efsm;
    return {};
}

void forgetEFSMNetworkLayouts(GraphicalModelSerializer* serializer, Model* model) {
    serializer->forgetNetworkDiagramLayouts(model);
}

class ExposedModelGraphicsScene final : public ModelGraphicsScene {
public:
    ExposedModelGraphicsScene() : ModelGraphicsScene(-1000, -1000, 4000, 4000) {}
    using ModelGraphicsScene::mousePressEvent;
    using ModelGraphicsScene::mouseReleaseEvent;
    using ModelGraphicsScene::mouseDoubleClickEvent;
    using ModelGraphicsScene::keyPressEvent;
    using ModelGraphicsScene::dropEvent;
};

TEST(EFSMNetworkRelationGesture, DoubleClickCreatesUndoableDirectedRelationsAndSupportsCancellation) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "GestureNetwork");
    FSMState source(model, "GestureSource");
    FSMState destination(model, "GestureDestination");
    FSMState third(model, "GestureThird");
    network.addState(&source);
    network.addState(&destination);
    network.addState(&third);

    ExposedModelGraphicsScene scene;
    scene.setSimulator(&simulator);
    scene.setNetworkEditorOwner(&network);
    QUndoStack undoStack;
    scene.setUndoStack(&undoStack);
    scene.setModelLevelFilter(static_cast<unsigned int>(network.getId()));
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, &scene);
    EFSMNetworkDiagramProjection::synchronize(&simulator, &scene);
    auto* sourceItem = scene.findGraphicalModelDataDefinition(&source);
    auto* destinationItem = scene.findGraphicalModelDataDefinition(&destination);
    auto* thirdItem = scene.findGraphicalModelDataDefinition(&third);
    ASSERT_NE(sourceItem, nullptr);
    ASSERT_NE(destinationItem, nullptr);
    ASSERT_NE(thirdItem, nullptr);

    auto doubleClick = [&](QGraphicsItem* item) {
        QGraphicsSceneMouseEvent event(QEvent::GraphicsSceneMouseDoubleClick);
        event.setButton(Qt::LeftButton);
        event.setButtons(Qt::LeftButton);
        event.setScenePos(item->sceneBoundingRect().center());
        scene.mouseDoubleClickEvent(&event);
    };
    auto click = [&](QGraphicsItem* item) {
        QGraphicsSceneMouseEvent event(QEvent::GraphicsSceneMousePress);
        event.setButton(Qt::LeftButton);
        event.setButtons(Qt::LeftButton);
        event.setScenePos(item->sceneBoundingRect().center());
        scene.mousePressEvent(&event);
    };

    doubleClick(sourceItem);
    ASSERT_TRUE(scene.isNetworkRelationCreationActive());
    EXPECT_TRUE(network.getTransitions()->empty());
    click(destinationItem);
    ASSERT_FALSE(scene.isNetworkRelationCreationActive());
    ASSERT_EQ(network.getTransitions()->size(), 1u);
    EXPECT_EQ(network.getTransitions()->front()->getSource(), &source);
    EXPECT_EQ(network.getTransitions()->front()->getDestination(), &destination);
    EXPECT_EQ(undoStack.count(), 1);
    ASSERT_EQ(scene.extensionOverlayItems(QStringLiteral("efsm")).size(), 1);
    undoStack.undo();
    EXPECT_TRUE(network.getTransitions()->empty());
    EXPECT_TRUE(scene.extensionOverlayItems(QStringLiteral("efsm")).isEmpty());
    undoStack.redo();
    ASSERT_EQ(network.getTransitions()->size(), 1u);
    ASSERT_EQ(scene.extensionOverlayItems(QStringLiteral("efsm")).size(), 1);

    doubleClick(destinationItem);
    click(thirdItem);
    ASSERT_EQ(network.getTransitions()->size(), 2u);
    EFSMTransition* secondTransition = network.getTransitions()->list()->back();
    EXPECT_EQ(secondTransition->getSource(), &destination);
    EXPECT_EQ(secondTransition->getDestination(), &third);

    doubleClick(thirdItem);
    click(thirdItem);
    ASSERT_EQ(network.getTransitions()->size(), 3u);
    EFSMTransition* selfLoop = network.getTransitions()->list()->back();
    EXPECT_EQ(selfLoop->getSource(), &third);
    EXPECT_EQ(selfLoop->getDestination(), &third);

    doubleClick(thirdItem);
    QKeyEvent escape(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    scene.keyPressEvent(&escape);
    EXPECT_FALSE(scene.isNetworkRelationCreationActive());
    doubleClick(sourceItem);
    QGraphicsSceneMouseEvent emptyClick(QEvent::GraphicsSceneMousePress);
    emptyClick.setButton(Qt::LeftButton);
    emptyClick.setScenePos(QPointF(1500.0, 1500.0));
    scene.mousePressEvent(&emptyClick);
    EXPECT_FALSE(scene.isNetworkRelationCreationActive());
}

TEST(EFSMNetworkPropertyEditor, SelectedTransitionExposesMathematicalPropertiesAndUndoRedo) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "TransitionPropertyNetwork");
    FSMState source(model, "TransitionPropertySource");
    FSMState destination(model, "TransitionPropertyDestination");
    network.addState(&source);
    network.addState(&destination);
    auto* transition = new EFSMTransition(&source, &destination, "EditableTransition");
    network.addTransition(transition);

    ExposedModelGraphicsScene scene;
    scene.setSimulator(&simulator);
    scene.setNetworkEditorOwner(&network);
    scene.setModelLevelFilter(static_cast<unsigned int>(network.getId()));
    QUndoStack undoStack;
    scene.setUndoStack(&undoStack);
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, &scene);
    EFSMNetworkDiagramProjection::synchronize(&simulator, &scene);
    ASSERT_EQ(scene.extensionOverlayItems(QStringLiteral("efsm")).size(), 1);
    auto* graphicalTransition = dynamic_cast<EFSMTransitionGraphicsItem*>(
        scene.extensionOverlayItems(QStringLiteral("efsm")).first());
    ASSERT_NE(graphicalTransition, nullptr);

    ObjectPropertyBrowser browser;
    browser.setModelScene(&scene);
    browser.setActiveGraphicsItem(graphicalTransition);
    QApplication::processEvents();
    auto findProperty = [&browser](const QString& name) {
        for (QtBrowserItem* item : browser.topLevelItems()) {
            if (item->property()->propertyName() == name) return item;
            QList<QtBrowserItem*> pending = item->children();
            while (!pending.isEmpty()) {
                QtBrowserItem* child = pending.takeFirst();
                if (child->property()->propertyName() == name) return child;
                pending.append(child->children());
            }
        }
        return static_cast<QtBrowserItem*>(nullptr);
    };
    for (const QString& name : {QStringLiteral("Name"), QStringLiteral("Source State"),
                                 QStringLiteral("Destination State"), QStringLiteral("Guard Expression"),
                                 QStringLiteral("Output Expression"), QStringLiteral("Input Event"),
                                 QStringLiteral("Priority"), QStringLiteral("Trigger Event"),
                                 QStringLiteral("Probability Expression")}) {
        ASSERT_NE(findProperty(name), nullptr) << name.toStdString();
    }
    EXPECT_FALSE(findProperty(QStringLiteral("Source State"))->property()->isEnabled());
    EXPECT_FALSE(findProperty(QStringLiteral("Destination State"))->property()->isEnabled());

    QtBrowserItem* guard = findProperty(QStringLiteral("Guard Expression"));
    browser.setCurrentItem(guard);
    browser.editItem(guard);
    QApplication::processEvents();
    QLineEdit* editor = browser.findChild<QLineEdit*>();
    ASSERT_NE(editor, nullptr);
    editor->setText(QStringLiteral("Counter.Value > 0"));
    QKeyEvent enter(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    QApplication::sendEvent(editor, &enter);
    QApplication::processEvents();
    EXPECT_EQ(transition->getGuardExpression(), "Counter.Value > 0");
    ASSERT_EQ(undoStack.count(), 1);
    undoStack.undo();
    EXPECT_TRUE(transition->getGuardExpression().empty());
    undoStack.redo();
    EXPECT_EQ(transition->getGuardExpression(), "Counter.Value > 0");
}

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

TEST(GuiExtensionManager, ContextMenuContributionsAreDispatchedWithoutFormalismBranches) {
    class ContextMenuPlugin final : public GuiExtensionPlugin {
    public:
        std::string extensionId() const override { return "context-menu-test"; }
        std::string displayName() const override { return "Context menu test"; }
        std::string version() const override { return "1"; }
        std::vector<std::string> requiredModelPlugins() const override { return {}; }
        void registerContributions(GuiExtensionRegistry* registry) const override {
            registry->addContextMenu({
                "test.context-menu",
                [](const GuiExtensionRuntimeContext&, QGraphicsItem* item) { return item != nullptr; },
                [](QMenu* menu, const GuiExtensionRuntimeContext&, QGraphicsItem*) {
                    menu->addAction(QStringLiteral("Extension item action"));
                }});
        }
    } plugin;

    QMainWindow host;
    GuiExtensionRuntimeContext context;
    context.mainWindow = &host;
    GuiExtensionManager manager(&host);
    manager.setPlugins({&plugin});
    manager.rebuild(context);
    QMenu menu;
    QGraphicsRectItem item;
    manager.populateContextMenu(&menu, &item);
    ASSERT_EQ(menu.actions().size(), 1);
    EXPECT_EQ(menu.actions().front()->text(), QStringLiteral("Extension item action"));

    QMenu emptyMenu;
    manager.populateContextMenu(&emptyMenu, nullptr);
    EXPECT_TRUE(emptyMenu.actions().empty());
}

TEST(EFSMNetworkEditor, ContextMenuStartsConnectionsFromOneSelectedStateOnly) {
    MainWindow window;
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelNew_triggered", Qt::DirectConnection));
    Simulator* simulator = window.myScene()->getSimulator();
    ASSERT_NE(simulator, nullptr);
    simulator->getPluginManager()->autoInsertPlugins();
    Model* model = simulator->getModelManager()->current();
    ASSERT_NE(model, nullptr);
    auto* network = new EFSMNetwork(model, "ContextMenuNetwork");
    auto* first = new FSMState(model, "ContextFirst");
    auto* second = new FSMState(model, "ContextSecond");
    network->addState(first);
    network->addState(second);
    network->addTransition(new EFSMTransition(first, second, "Existing"));

    auto* view = window.findChild<ModelGraphicsView*>();
    ASSERT_NE(view, nullptr);
    ModelGraphicsScene* scene = window.myScene();
    ASSERT_NE(scene, nullptr);
    scene->setNetworkEditorOwner(network);
    scene->setModelLevelFilter(static_cast<unsigned int>(network->getId()));
    std::map<std::string, QColor> colors;
    GraphicalModelBuilder builder(simulator, view, scene, &colors, nullptr);
    builder.generateGraphicalModelFromModel();
    EFSMNetworkDiagramProjection::synchronize(simulator, scene);
    auto* firstItem = scene->findGraphicalModelDataDefinition(first);
    ASSERT_NE(firstItem, nullptr);

    GuiExtensionRuntimeContext context;
    context.simulator = simulator;
    context.mainWindow = &window;
    context.graphicsView = view;
    context.graphicsScene = scene;
    GuiExtensionManager manager(&window);
    manager.setLoadedModelPluginIds({Util::TypeOf<ModalModelDefault>()});
    manager.setPlugins(GuiExtensionPluginCatalog::resolvedPlugins());
    manager.rebuild(context);

    firstItem->setSelected(true);
    QMenu stateMenu;
    manager.populateContextMenu(&stateMenu, firstItem);
    ASSERT_EQ(stateMenu.actions().size(), 1);
    EXPECT_EQ(stateMenu.actions().front()->text(), QStringLiteral("Create EFSM Transition"));

    auto* transitionItem = dynamic_cast<EFSMTransitionGraphicsItem*>(scene->extensionOverlayItems(
        QStringLiteral("efsm")).front());
    ASSERT_NE(transitionItem, nullptr);
    QMenu relationMenu;
    scene->clearSelection();
    transitionItem->setSelected(true);
    manager.populateContextMenu(&relationMenu, transitionItem);
    EXPECT_TRUE(relationMenu.actions().isEmpty());
}

TEST(GuiExtensionManager, NetworkCanvasAcceptsOnlyFormalismDataDefinitions) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "DropPolicyNetwork");

    EXPECT_TRUE(GuiExtensionPluginCatalog::acceptsNetworkEditorDrop(
        &network, Util::TypeOf<FSMState>(), false));
    EXPECT_FALSE(GuiExtensionPluginCatalog::acceptsNetworkEditorDrop(
        &network, Util::TypeOf<Create>(), true));
    EXPECT_FALSE(GuiExtensionPluginCatalog::acceptsNetworkEditorDrop(
        &network, Util::TypeOf<GraphNode>(), false));
    EXPECT_FALSE(GuiExtensionPluginCatalog::acceptsNetworkEditorDrop(
        nullptr, Util::TypeOf<FSMState>(), false));
}

TEST(GuiExtensionManager, PluginPaletteShowsOnlyActiveNetworkElementTypes) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    QTreeWidget palette;
    std::map<std::string, QColor> categoryColors;
    PluginCatalogController controller(&simulator, &palette, nullptr, &categoryColors);
    auto containsPlugin = [&palette](const std::string& type) {
        return !palette.findItems(QString::fromStdString(type), Qt::MatchExactly | Qt::MatchRecursive, 0).isEmpty();
    };

    controller.reloadFromPluginManager();
    EXPECT_TRUE(containsPlugin(Util::TypeOf<Create>()));
    EXPECT_FALSE(containsPlugin(Util::TypeOf<GraphNode>()));

    GraphNetwork graph(model, "GraphPalette");
    controller.reloadFromPluginManager(&graph);
    EXPECT_TRUE(containsPlugin(Util::TypeOf<GraphNode>()));
    EXPECT_FALSE(containsPlugin(Util::TypeOf<Create>()));
    EXPECT_FALSE(containsPlugin(Util::TypeOf<GraphEdge>()));

    MarkovChainNetwork markov(model, "MarkovPalette");
    controller.reloadFromPluginManager(&markov);
    EXPECT_TRUE(containsPlugin(Util::TypeOf<MarkovState>()));
    EXPECT_FALSE(containsPlugin(Util::TypeOf<GraphNode>()));

    ColoredPetriNetNetwork cpn(model, "CpnPalette");
    controller.reloadFromPluginManager(&cpn);
    EXPECT_TRUE(containsPlugin(Util::TypeOf<PetriPlace>()));
    EXPECT_TRUE(containsPlugin(Util::TypeOf<CPNTransition>()));
    EXPECT_FALSE(containsPlugin(Util::TypeOf<MarkovState>()));

    EFSMNetwork efsm(model, "EfsmPalette");
    controller.reloadFromPluginManager(&efsm);
    EXPECT_TRUE(containsPlugin(Util::TypeOf<FSMState>()));
    EXPECT_FALSE(containsPlugin(Util::TypeOf<PetriPlace>()));

    controller.reloadFromPluginManager();
    EXPECT_TRUE(containsPlugin(Util::TypeOf<Create>()));
    EXPECT_FALSE(containsPlugin(Util::TypeOf<FSMState>()));
}

TEST(GuiExtensionManager, NetworkCanvasRejectsProcessComponentBeforeModelMutation) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "DropBoundaryNetwork");
    ExposedModelGraphicsScene scene;
    scene.setSimulator(&simulator);
    scene.setModelLevelFilter(static_cast<unsigned int>(network.getId()));
    scene.setNetworkEditorOwner(&network);

    Plugin* componentPlugin = simulator.getPluginManager()->find(Util::TypeOf<Create>());
    ASSERT_NE(componentPlugin, nullptr);
    QTreeWidgetItem draggedItem;
    draggedItem.setWhatsThis(0, QString::fromStdString(componentPlugin->getPluginInfo()->getPluginTypename()));
    scene.setObjectBeingDragged(&draggedItem);
    const unsigned int initialComponents = model->getComponentManager()->getAllComponents()->size();
    QGraphicsSceneDragDropEvent event(QEvent::GraphicsSceneDrop);
    scene.dropEvent(&event);

    EXPECT_FALSE(event.isAccepted());
    EXPECT_EQ(model->getComponentManager()->getAllComponents()->size(), initialComponents);
    EXPECT_TRUE(scene.graphicalModelComponentItems().isEmpty());

    Plugin* statePlugin = simulator.getPluginManager()->find(Util::TypeOf<FSMState>());
    ASSERT_NE(statePlugin, nullptr);
    draggedItem.setWhatsThis(0, QString::fromStdString(statePlugin->getPluginInfo()->getPluginTypename()));
    QGraphicsSceneDragDropEvent allowedEvent(QEvent::GraphicsSceneDrop);
    allowedEvent.setScenePos(QPointF(120, 80));
    scene.dropEvent(&allowedEvent);
    EXPECT_EQ(network.getStates()->size(), 1u);
    EXPECT_EQ(scene.getGraphicalModelDataDefinitions()->size(), 1);
}

TEST(GuiExtensionManager, ProcessSubmodelStillAcceptsProcessComponentDrops) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* submodel = new Submodel(model, "RegularSubmodel");
    ExposedModelGraphicsScene scene;
    scene.setSimulator(&simulator);
    scene.setModelLevelFilter(static_cast<unsigned int>(submodel->getId()));
    scene.setNetworkEditorOwner(submodel);
    QUndoStack undoStack;
    scene.setUndoStack(&undoStack);
    PropertyEditorGenesys propertyEditor;
    std::map<SimulationControl*, DataComponentProperty*> propertyList;
    std::map<SimulationControl*, DataComponentEditor*> propertyEditorUi;
    std::map<SimulationControl*, ComboBoxEnum*> propertyCombo;
    scene.setPropertyEditor(&propertyEditor);
    scene.setPropertyList(&propertyList);
    scene.setPropertyEditorUI(&propertyEditorUi);
    scene.setComboBox(&propertyCombo);
    Plugin* createPlugin = simulator.getPluginManager()->find(Util::TypeOf<Create>());
    ASSERT_NE(createPlugin, nullptr);
    QTreeWidgetItem draggedItem;
    draggedItem.setWhatsThis(0, QString::fromStdString(createPlugin->getPluginInfo()->getPluginTypename()));
    scene.setObjectBeingDragged(&draggedItem);
    QGraphicsSceneDragDropEvent event(QEvent::GraphicsSceneDrop);
    event.setScenePos(QPointF(120, 80));

    const std::size_t componentsBefore = model->getComponentManager()->getNumberOfComponents();
    scene.dropEvent(&event);
    EXPECT_EQ(model->getComponentManager()->getNumberOfComponents(), componentsBefore + 1);
    EXPECT_EQ(scene.getGraphicalModelComponents()->size(), 1);
}

TEST(GraphNetworkGuiExtension, ProjectsParallelAndSelfLoopEdgesFromTheNetworkTopology) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    DirectedGraphNetwork network(model, "DirectedProjection");
    auto* first = new GraphNode(model, "First");
    auto* second = new GraphNode(model, "Second");
    ASSERT_TRUE(network.addNode(first));
    ASSERT_TRUE(network.addNode(second));
    auto* parallelA = new GraphEdge(model, first, second, "ParallelA");
    auto* parallelB = new GraphEdge(model, first, second, "ParallelB");
    auto* selfLoop = new GraphEdge(model, first, first, "SelfLoop");
    ASSERT_TRUE(network.addEdge(parallelA));
    ASSERT_TRUE(network.addEdge(parallelB));
    ASSERT_TRUE(network.addEdge(selfLoop));

    ExposedModelGraphicsScene scene;
    scene.setSimulator(&simulator);
    scene.setNetworkEditorOwner(&network);
    QUndoStack undoStack;
    scene.setUndoStack(&undoStack);
    Plugin* nodePlugin = simulator.getPluginManager()->find(Util::TypeOf<GraphNode>());
    ASSERT_NE(nodePlugin, nullptr);
    scene.addGraphicalModelDataDefinition(nodePlugin, first, QPointF(20, 20));
    scene.addGraphicalModelDataDefinition(nodePlugin, second, QPointF(240, 120));

    GuiExtensionPluginCatalog::synchronizeDiagramProjections(&simulator, &scene);
    QList<QGraphicsItem*> projected = scene.extensionOverlayItems(QStringLiteral("modal.graph.gui"));
    ASSERT_EQ(projected.size(), 3);
    for (QGraphicsItem* item : projected) {
        auto* pathItem = dynamic_cast<QGraphicsPathItem*>(item);
        ASSERT_NE(pathItem, nullptr);
        EXPECT_FALSE(pathItem->path().isEmpty());
    }
    GuiExtensionPluginCatalog::synchronizeDiagramProjections(&simulator, &scene);
    EXPECT_EQ(scene.extensionOverlayItems(QStringLiteral("modal.graph.gui")).size(), 3);
}

TEST(GraphNetworkGuiExtension, RendersArrowsOnlyForDirectedGraphFamilies) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    GraphNetwork undirected(model, "UndirectedGraph");
    DirectedGraphNetwork directed(model, "DirectedGraph");
    auto* undirectedSource = new GraphNode(model, "UndirectedSource");
    auto* undirectedDestination = new GraphNode(model, "UndirectedDestination");
    auto* directedSource = new GraphNode(model, "DirectedSource");
    auto* directedDestination = new GraphNode(model, "DirectedDestination");
    ASSERT_TRUE(undirected.addNode(undirectedSource));
    ASSERT_TRUE(undirected.addNode(undirectedDestination));
    ASSERT_TRUE(directed.addNode(directedSource));
    ASSERT_TRUE(directed.addNode(directedDestination));
    ASSERT_TRUE(undirected.addEdge(new GraphEdge(model, undirectedSource, undirectedDestination, "UndirectedEdge")));
    ASSERT_TRUE(directed.addEdge(new GraphEdge(model, directedSource, directedDestination, "DirectedEdge")));

    ExposedModelGraphicsScene scene;
    scene.setSimulator(&simulator);
    Plugin* nodePlugin = simulator.getPluginManager()->find(Util::TypeOf<GraphNode>());
    ASSERT_NE(nodePlugin, nullptr);
    scene.setNetworkEditorOwner(&undirected);
    scene.addGraphicalModelDataDefinition(nodePlugin, undirectedSource, QPointF(20, 20));
    scene.addGraphicalModelDataDefinition(nodePlugin, undirectedDestination, QPointF(220, 20));
    GuiExtensionPluginCatalog::synchronizeDiagramProjections(&simulator, &scene);
    auto undirectedItems = scene.extensionOverlayItems(QStringLiteral("modal.graph.gui"));
    ASSERT_EQ(undirectedItems.size(), 1);
    auto* undirectedPath = dynamic_cast<QGraphicsPathItem*>(undirectedItems.front());
    ASSERT_NE(undirectedPath, nullptr);
    const int undirectedElementCount = undirectedPath->path().elementCount();
    EXPECT_EQ(undirectedElementCount, 2);

    scene.clearExtensionOverlayItems(QStringLiteral("modal.graph.gui"));
    scene.clear();
    scene.setNetworkEditorOwner(&directed);
    scene.addGraphicalModelDataDefinition(nodePlugin, directedSource, QPointF(20, 20));
    scene.addGraphicalModelDataDefinition(nodePlugin, directedDestination, QPointF(220, 20));
    GuiExtensionPluginCatalog::synchronizeDiagramProjections(&simulator, &scene);
    auto directedItems = scene.extensionOverlayItems(QStringLiteral("modal.graph.gui"));
    ASSERT_EQ(directedItems.size(), 1);
    auto* directedPath = dynamic_cast<QGraphicsPathItem*>(directedItems.front());
    ASSERT_NE(directedPath, nullptr);
    EXPECT_GT(directedPath->path().elementCount(), undirectedElementCount);
}

TEST(GraphNetworkGuiExtension, RemovesNodeAndIncidentEdgesWithUndoRedoAndStableGeometry) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* network = new DirectedGraphNetwork(model, "UndoNodeGraph");
    auto* first = new GraphNode(model, "First");
    auto* middle = new GraphNode(model, "Middle");
    auto* last = new GraphNode(model, "Last");
    ASSERT_TRUE(network->addNode(first));
    ASSERT_TRUE(network->addNode(middle));
    ASSERT_TRUE(network->addNode(last));
    auto* firstEdge = new GraphEdge(model, first, middle, "FirstEdge");
    auto* secondEdge = new GraphEdge(model, middle, last, "SecondEdge");
    ASSERT_TRUE(network->addEdge(firstEdge));
    ASSERT_TRUE(network->addEdge(secondEdge));

    ExposedModelGraphicsScene scene;
    scene.setSimulator(&simulator);
    scene.setModelLevelFilter(static_cast<unsigned int>(network->getId()));
    scene.setNetworkEditorOwner(network);
    QUndoStack undoStack;
    scene.setUndoStack(&undoStack);
    Plugin* nodePlugin = simulator.getPluginManager()->find(Util::TypeOf<GraphNode>());
    ASSERT_NE(nodePlugin, nullptr);
    scene.addGraphicalModelDataDefinition(nodePlugin, first, QPointF(20, 40));
    auto* middleItem = scene.addGraphicalModelDataDefinition(nodePlugin, middle, QPointF(220, 140));
    scene.addGraphicalModelDataDefinition(nodePlugin, last, QPointF(420, 40));
    const QPointF originalMiddlePosition = middleItem->pos();
    middleItem->setSelected(true);

    GuiActionContribution removeNode;
    for (const GuiExtensionPlugin* plugin : GuiExtensionPluginCatalog::resolvedPlugins()) {
        if (plugin == nullptr || plugin->extensionId() != "modal.graph.gui") continue;
        GuiExtensionRegistry registry;
        plugin->registerContributions(&registry);
        for (const auto& action : registry.actions()) if (action.id == "modal.graph.remove-node") removeNode = action;
    }
    ASSERT_TRUE(static_cast<bool>(removeNode.trigger));
    GuiExtensionRuntimeContext context;
    context.simulator = &simulator;
    context.graphicsScene = &scene;
    ASSERT_TRUE(removeNode.isEnabled(context));
    removeNode.trigger(context);
    EXPECT_EQ(network->getNodes().size(), 2u);
    EXPECT_TRUE(network->getEdges().empty());
    EXPECT_EQ(model->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<GraphNode>()), 2u);
    EXPECT_EQ(model->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<GraphEdge>()), 0u);

    undoStack.undo();
    ASSERT_EQ(network->getNodes().size(), 3u);
    ASSERT_EQ(network->getEdges().size(), 2u);
    EXPECT_EQ(network->getEdges().at(0), firstEdge);
    EXPECT_EQ(network->getEdges().at(1), secondEdge);
    EXPECT_EQ(scene.findGraphicalModelDataDefinition(middle)->pos(), originalMiddlePosition);
    undoStack.redo();
    EXPECT_EQ(network->getNodes().size(), 2u);
    EXPECT_TRUE(network->getEdges().empty());
    undoStack.undo();
    ASSERT_EQ(network->getNodes().size(), 3u);
    EXPECT_EQ(network->getEdges().at(0), firstEdge);
    EXPECT_EQ(network->getEdges().at(1), secondEdge);
}

TEST(GraphNetworkGuiExtension, ShowsBfsAnalysisAsStructuralHighlighting) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    GraphNetwork network(model, "AnalysisGraph");
    auto* first = new GraphNode(model, "A");
    auto* second = new GraphNode(model, "B");
    auto* third = new GraphNode(model, "C");
    network.addNode(first);
    network.addNode(second);
    network.addNode(third);
    ASSERT_TRUE(network.addEdge(new GraphEdge(model, first, second, "AB")));
    ASSERT_TRUE(network.addEdge(new GraphEdge(model, second, third, "BC")));
    ExposedModelGraphicsScene scene;
    scene.setSimulator(&simulator);
    scene.setNetworkEditorOwner(&network);
    Plugin* nodePlugin = simulator.getPluginManager()->find(Util::TypeOf<GraphNode>());
    ASSERT_NE(nodePlugin, nullptr);
    auto* firstItem = scene.addGraphicalModelDataDefinition(nodePlugin, first, QPointF(20, 20));
    scene.addGraphicalModelDataDefinition(nodePlugin, second, QPointF(220, 20));
    scene.addGraphicalModelDataDefinition(nodePlugin, third, QPointF(420, 20));
    firstItem->setSelected(true);

    GuiActionContribution analyze;
    for (const GuiExtensionPlugin* plugin : GuiExtensionPluginCatalog::resolvedPlugins()) {
        if (plugin == nullptr || plugin->extensionId() != "modal.graph.gui") continue;
        GuiExtensionRegistry registry;
        plugin->registerContributions(&registry);
        for (const auto& action : registry.actions()) if (action.id == "modal.graph.analyze-traversal") analyze = action;
    }
    ASSERT_TRUE(static_cast<bool>(analyze.trigger));
    GuiExtensionRuntimeContext context;
    context.simulator = &simulator;
    context.graphicsScene = &scene;
    ASSERT_TRUE(analyze.isEnabled(context));
    QTimer::singleShot(0, []() {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        ASSERT_NE(dialog, nullptr);
        const auto choices = dialog->findChildren<QComboBox*>();
        ASSERT_EQ(choices.size(), 1);
        choices.front()->setCurrentIndex(0);
        dialog->accept();
    });
    analyze.trigger(context);

    EXPECT_EQ(network.breadthFirstSearch(first).visitOrder.size(), 3u);
    EXPECT_EQ(network.breadthFirstSearch(first).distanceTo(third), 2u);
    EXPECT_EQ(scene.extensionOverlayItems(QStringLiteral("modal.graph.analysis")).size(), 8);
    EXPECT_DOUBLE_EQ(network.getActivationCount(), 0.0);
    bool sawBfsVisit = false;
    for (QGraphicsItem* item : scene.extensionOverlayItems(QStringLiteral("modal.graph.analysis"))) {
        if (item->toolTip().contains(QStringLiteral("BFS visit"))) sawBfsVisit = true;
    }
    EXPECT_TRUE(sawBfsVisit);
}

TEST(GraphNetworkGuiExtension, GenericGuiPersistenceRestoresNodeGeometryByStableModelIdentity) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* network = new DirectedGraphNetwork(model, "PersistedGraph");
    auto* first = new GraphNode(model, "Source");
    auto* second = new GraphNode(model, "Target");
    ASSERT_TRUE(network->addNode(first));
    ASSERT_TRUE(network->addNode(second));
    ASSERT_TRUE(network->addEdge(new GraphEdge(model, first, second, "Arc")));
    auto* adapter = new ModalModelDefault(model, "GraphPersistenceAdapter");
    adapter->setNetwork(network);

    SerializerHarness harness(&simulator);
    ModelGraphicsScene* scene = harness.graphicsView.getScene();
    ASSERT_NE(scene, nullptr);
    scene->setModelLevelFilter(static_cast<unsigned int>(network->getId()));
    scene->setNetworkEditorOwner(network);
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);
    auto* firstItem = scene->findGraphicalModelDataDefinition(first);
    auto* secondItem = scene->findGraphicalModelDataDefinition(second);
    ASSERT_NE(firstItem, nullptr);
    ASSERT_NE(secondItem, nullptr);
    firstItem->setPos(17.25, 31.5);
    secondItem->setPos(320.75, 241.125);

    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());
    const QString guiFilename = temporaryDir.filePath("graph_network.gui");
    ASSERT_TRUE(harness.serializer->saveGraphicalModel(guiFilename));
    Model* loadedModel = harness.serializer->loadGraphicalModel(guiFilename.toStdString());
    ASSERT_NE(loadedModel, nullptr);
    auto* loadedNetwork = dynamic_cast<DirectedGraphNetwork*>(loadedModel->getDataManager()->getDataDefinition(
        Util::TypeOf<DirectedGraphNetwork>(), std::string("PersistedGraph")));
    ASSERT_NE(loadedNetwork, nullptr);
    ASSERT_EQ(loadedNetwork->getNodes().size(), 2u);
    ASSERT_EQ(loadedNetwork->getEdges().size(), 1u);
    ASSERT_EQ(simulator.getModelManager()->current(), loadedModel);
    ASSERT_EQ(loadedModel->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<GraphNode>()), 2u);
    ASSERT_EQ(loadedModel->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<GraphEdge>()), 1u);
    ASSERT_NE(simulator.getPluginManager()->find(Util::TypeOf<GraphNode>()), nullptr);
    ASSERT_EQ(loadedNetwork->getNodes().front()->getLevel(), loadedNetwork->getId());
    scene->setModelLevelFilter(static_cast<unsigned int>(loadedNetwork->getId()));
    scene->setNetworkEditorOwner(loadedNetwork);
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);
    ASSERT_EQ(scene->getGraphicalModelDataDefinitions()->size(), 3);
    auto* restoredFirst = scene->findGraphicalModelDataDefinition(
        loadedModel->getDataManager()->getDataDefinition(Util::TypeOf<GraphNode>(), std::string("Source")));
    auto* restoredSecond = scene->findGraphicalModelDataDefinition(
        loadedModel->getDataManager()->getDataDefinition(Util::TypeOf<GraphNode>(), std::string("Target")));
    ASSERT_NE(restoredFirst, nullptr);
    ASSERT_NE(restoredSecond, nullptr);
    EXPECT_EQ(restoredFirst->pos(), QPointF(17.25, 31.5));
    EXPECT_EQ(restoredSecond->pos(), QPointF(320.75, 241.125));
    EXPECT_NE(loadedNetwork->getEdges().front()->getSource(), nullptr);
    EXPECT_NE(loadedNetwork->getEdges().front()->getDestination(), nullptr);
}

TEST(MarkovChainNetworkGuiExtension, GenericGuiPersistenceRestoresStateGeometryAndTopology) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* network = new MarkovChainNetwork(model, "PersistedMarkov");
    auto* first = new MarkovState(model, "Start");
    auto* second = new MarkovState(model, "Finish");
    network->addState(first);
    network->addState(second);
    network->setInitialState(first);
    network->addTransition(new MarkovChainNetwork::MarkovTransition(first, second, 1.0, "Advance"));

    SerializerHarness harness(&simulator);
    ModelGraphicsScene* scene = harness.graphicsView.getScene();
    scene->setModelLevelFilter(static_cast<unsigned int>(network->getId()));
    scene->setNetworkEditorOwner(network);
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);
    auto* firstItem = scene->findGraphicalModelDataDefinition(first);
    auto* secondItem = scene->findGraphicalModelDataDefinition(second);
    ASSERT_NE(firstItem, nullptr);
    ASSERT_NE(secondItem, nullptr);
    firstItem->setPos(45.5, 67.25);
    secondItem->setPos(270.75, 190.125);

    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());
    const QString guiFilename = temporaryDir.filePath("markov_network.gui");
    ASSERT_TRUE(harness.serializer->saveGraphicalModel(guiFilename));
    Model* loadedModel = harness.serializer->loadGraphicalModel(guiFilename.toStdString());
    ASSERT_NE(loadedModel, nullptr);
    auto* loadedNetwork = dynamic_cast<MarkovChainNetwork*>(loadedModel->getDataManager()->getDataDefinition(
        Util::TypeOf<MarkovChainNetwork>(), std::string("PersistedMarkov")));
    ASSERT_NE(loadedNetwork, nullptr);
    ASSERT_EQ(loadedNetwork->getStates()->size(), 2u);
    ASSERT_EQ(loadedNetwork->getTransitions()->size(), 1u);
    ASSERT_EQ(loadedModel->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<MarkovState>()), 2u);
    ASSERT_NE(loadedNetwork->getInitialState(), nullptr);
    EXPECT_EQ(loadedNetwork->getInitialState()->getName(), "Start");
    EXPECT_DOUBLE_EQ(loadedNetwork->getTransitions()->front()->getProbability(), 1.0);
    scene->setModelLevelFilter(static_cast<unsigned int>(loadedNetwork->getId()));
    scene->setNetworkEditorOwner(loadedNetwork);
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);
    auto* restoredFirst = scene->findGraphicalModelDataDefinition(
        loadedModel->getDataManager()->getDataDefinition(Util::TypeOf<MarkovState>(), std::string("Start")));
    auto* restoredSecond = scene->findGraphicalModelDataDefinition(
        loadedModel->getDataManager()->getDataDefinition(Util::TypeOf<MarkovState>(), std::string("Finish")));
    ASSERT_NE(restoredFirst, nullptr);
    ASSERT_NE(restoredSecond, nullptr);
    EXPECT_EQ(restoredFirst->pos(), QPointF(45.5, 67.25));
    EXPECT_EQ(restoredSecond->pos(), QPointF(270.75, 190.125));
}

TEST(GuiExtensionCatalog, DisabledMarkovEditorPreservesGuiGeometryAndKernelExecution) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* network = new MarkovChainNetwork(model, "OptionalMarkovEditor");
    auto* state = new MarkovState(model, "Absorbing");
    network->addState(state);
    network->addTransition(new MarkovChainNetwork::MarkovTransition(state, state, 1.0, "Stay"));

    SerializerHarness harness(&simulator);
    auto* scene = harness.graphicsView.getScene();
    scene->setModelLevelFilter(static_cast<unsigned int>(network->getId()));
    scene->setNetworkEditorOwner(network);
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);
    auto* stateItem = scene->findGraphicalModelDataDefinition(state);
    ASSERT_NE(stateItem, nullptr);
    stateItem->setPos(188.25, 92.5);

    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());
    const QString guiFilename = temporaryDir.filePath("optional_markov.gui");
    const QString configFilename = temporaryDir.filePath("gui_extensions.json");
    ASSERT_TRUE(harness.serializer->saveGraphicalModel(guiFilename));
    QFile config(configFilename);
    ASSERT_TRUE(config.open(QIODevice::WriteOnly | QIODevice::Text));
    config.write("{\"disabled\":[\"modal.markov.gui\"]}");
    config.close();
    ScopedGuiExtensionConfig scopedConfig(configFilename.toUtf8());

    const auto enabledPlugins = GuiExtensionPluginCatalog::resolvedPlugins();
    EXPECT_EQ(std::find_if(enabledPlugins.begin(), enabledPlugins.end(), [](const GuiExtensionPlugin* plugin) {
        return plugin != nullptr && plugin->extensionId() == "modal.markov.gui";
    }), enabledPlugins.end());
    Model* loadedModel = harness.serializer->loadGraphicalModel(guiFilename.toStdString());
    ASSERT_NE(loadedModel, nullptr);
    auto* loadedNetwork = dynamic_cast<MarkovChainNetwork*>(loadedModel->getDataManager()->getDataDefinition(
        Util::TypeOf<MarkovChainNetwork>(), std::string("OptionalMarkovEditor")));
    ASSERT_NE(loadedNetwork, nullptr);
    scene->setModelLevelFilter(static_cast<unsigned int>(loadedNetwork->getId()));
    scene->setNetworkEditorOwner(loadedNetwork);
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);
    auto* loadedState = loadedModel->getDataManager()->getDataDefinition(
        Util::TypeOf<MarkovState>(), std::string("Absorbing"));
    auto* loadedStateItem = scene->findGraphicalModelDataDefinition(loadedState);
    ASSERT_NE(loadedStateItem, nullptr);
    EXPECT_EQ(loadedStateItem->pos(), QPointF(188.25, 92.5));

    const QString roundTripFilename = temporaryDir.filePath("optional_markov_round_trip.gui");
    ASSERT_TRUE(harness.serializer->saveGraphicalModel(roundTripFilename));
    loadedModel = harness.serializer->loadGraphicalModel(roundTripFilename.toStdString());
    ASSERT_NE(loadedModel, nullptr);
    loadedNetwork = dynamic_cast<MarkovChainNetwork*>(loadedModel->getDataManager()->getDataDefinition(
        Util::TypeOf<MarkovChainNetwork>(), std::string("OptionalMarkovEditor")));
    ASSERT_NE(loadedNetwork, nullptr);
    scene->setModelLevelFilter(static_cast<unsigned int>(loadedNetwork->getId()));
    scene->setNetworkEditorOwner(loadedNetwork);
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);
    loadedState = loadedModel->getDataManager()->getDataDefinition(
        Util::TypeOf<MarkovState>(), std::string("Absorbing"));
    loadedStateItem = scene->findGraphicalModelDataDefinition(loadedState);
    ASSERT_NE(loadedStateItem, nullptr);
    EXPECT_EQ(loadedStateItem->pos(), QPointF(188.25, 92.5));

    const NetworkActivationResult activation = loadedNetwork->activate(
        NetworkActivationFrame(loadedNetwork->getNumInputPorts()));
    EXPECT_TRUE(activation.isPresent(0));
    EXPECT_EQ(loadedNetwork->getCurrentState(), loadedState);
    ASSERT_NE(loadedNetwork->getLastTransition(), nullptr);
    EXPECT_EQ(loadedNetwork->getLastTransition()->getName(), "Stay");
}

TEST(ColoredPetriNetGuiExtension, ValidationActionReportsInvalidGuardExpressions) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    ColoredPetriNetNetwork network(model, "InvalidGuardNetwork");
    auto* place = new PetriPlace(model, "Input");
    auto* transition = new CPNTransition(model, "BadGuard");
    transition->setGuardExpression("(");
    network.addPlace(place);
    network.addTransition(transition);
    ExposedModelGraphicsScene scene;
    scene.setSimulator(&simulator);
    scene.setNetworkEditorOwner(&network);
    GuiActionContribution validate;
    for (const GuiExtensionPlugin* plugin : GuiExtensionPluginCatalog::resolvedPlugins()) {
        if (plugin == nullptr || plugin->extensionId() != "modal.cpn.gui") continue;
        GuiExtensionRegistry registry;
        plugin->registerContributions(&registry);
        for (const auto& action : registry.actions())
            if (action.id == "modal.cpn.validate-network") validate = action;
    }
    ASSERT_TRUE(static_cast<bool>(validate.trigger));
    GuiExtensionRuntimeContext context;
    context.simulator = &simulator;
    context.graphicsScene = &scene;
    ASSERT_TRUE(validate.isEnabled(context));
    QTimer::singleShot(0, []() {
        auto* message = qobject_cast<QMessageBox*>(QApplication::activeModalWidget());
        ASSERT_NE(message, nullptr);
        EXPECT_NE(message->text().indexOf(QStringLiteral("CPN transition guard")), -1);
        message->accept();
    });
    validate.trigger(context);
}

TEST(ColoredPetriNetGuiExtension, GenericGuiPersistenceRestoresNodeGeometryAndTopology) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* network = new ColoredPetriNetNetwork(model, "PersistedCPN");
    auto* input = new PetriPlace(model, "Input");
    auto* output = new PetriPlace(model, "Output");
    auto* transition = new CPNTransition(model, "Transfer");
    network->addPlace(input);
    network->addPlace(output);
    network->addTransition(transition);
    network->setInitialTokens(input, "red", 3);
    auto* incoming = new CPNArc(model, input, transition, CPNArc::Direction::PlaceToTransition, "InputArc");
    incoming->setInscription("red", 1);
    auto* outgoing = new CPNArc(model, output, transition, CPNArc::Direction::TransitionToPlace, "OutputArc");
    outgoing->setInscription("red", 1);
    ASSERT_TRUE(network->addArc(incoming));
    ASSERT_TRUE(network->addArc(outgoing));

    SerializerHarness harness(&simulator);
    ModelGraphicsScene* scene = harness.graphicsView.getScene();
    scene->setModelLevelFilter(static_cast<unsigned int>(network->getId()));
    scene->setNetworkEditorOwner(network);
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);
    auto* inputItem = scene->findGraphicalModelDataDefinition(input);
    auto* outputItem = scene->findGraphicalModelDataDefinition(output);
    auto* transitionItem = scene->findGraphicalModelDataDefinition(transition);
    ASSERT_NE(inputItem, nullptr);
    ASSERT_NE(outputItem, nullptr);
    ASSERT_NE(transitionItem, nullptr);
    inputItem->setPos(30.25, 80.5);
    transitionItem->setPos(230.75, 80.5);
    outputItem->setPos(430.125, 80.5);

    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());
    const QString guiFilename = temporaryDir.filePath("cpn_network.gui");
    ASSERT_TRUE(harness.serializer->saveGraphicalModel(guiFilename));
    Model* loadedModel = harness.serializer->loadGraphicalModel(guiFilename.toStdString());
    ASSERT_NE(loadedModel, nullptr);
    auto* loadedNetwork = dynamic_cast<ColoredPetriNetNetwork*>(loadedModel->getDataManager()->getDataDefinition(
        Util::TypeOf<ColoredPetriNetNetwork>(), std::string("PersistedCPN")));
    ASSERT_NE(loadedNetwork, nullptr);
    ASSERT_EQ(loadedNetwork->getPlaces()->size(), 2u);
    ASSERT_EQ(loadedNetwork->getTransitions()->size(), 1u);
    ASSERT_EQ(loadedNetwork->getArcs()->size(), 2u);
    ASSERT_EQ(loadedModel->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<CPNArc>()), 2u);
    EXPECT_EQ(loadedNetwork->getInitialTokens(loadedNetwork->getPlaces()->front(), "red"), 3u);
    scene->setModelLevelFilter(static_cast<unsigned int>(loadedNetwork->getId()));
    scene->setNetworkEditorOwner(loadedNetwork);
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);
    auto* restoredInput = scene->findGraphicalModelDataDefinition(
        loadedModel->getDataManager()->getDataDefinition(Util::TypeOf<PetriPlace>(), std::string("Input")));
    auto* restoredOutput = scene->findGraphicalModelDataDefinition(
        loadedModel->getDataManager()->getDataDefinition(Util::TypeOf<PetriPlace>(), std::string("Output")));
    auto* restoredTransition = scene->findGraphicalModelDataDefinition(
        loadedModel->getDataManager()->getDataDefinition(Util::TypeOf<CPNTransition>(), std::string("Transfer")));
    ASSERT_NE(restoredInput, nullptr);
    ASSERT_NE(restoredOutput, nullptr);
    ASSERT_NE(restoredTransition, nullptr);
    EXPECT_EQ(restoredInput->pos(), QPointF(30.25, 80.5));
    EXPECT_EQ(restoredTransition->pos(), QPointF(230.75, 80.5));
    EXPECT_EQ(restoredOutput->pos(), QPointF(430.125, 80.5));
}

TEST(MarkovChainNetworkGuiExtension, ProjectsStatesAndCreatesTransitionsWithUndoRedo) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    MarkovChainNetwork network(model, "EditableMarkov");
    auto* first = new MarkovState(model, "Start");
    auto* second = new MarkovState(model, "Finish");
    network.addState(first);
    network.addState(second);
    network.setInitialState(first);
    network.setCurrentState(second);

    ExposedModelGraphicsScene scene;
    scene.setSimulator(&simulator);
    scene.setNetworkEditorOwner(&network);
    QUndoStack undoStack;
    scene.setUndoStack(&undoStack);
    Plugin* statePlugin = simulator.getPluginManager()->find(Util::TypeOf<MarkovState>());
    ASSERT_NE(statePlugin, nullptr);
    auto* firstItem = scene.addGraphicalModelDataDefinition(statePlugin, first, QPointF(20, 20));
    auto* secondItem = scene.addGraphicalModelDataDefinition(statePlugin, second, QPointF(220, 20));

    GuiExtensionPluginCatalog::synchronizeDiagramProjections(&simulator, &scene);
    EXPECT_EQ(scene.extensionOverlayItems(QStringLiteral("modal.markov.gui")).size(), 2);
    firstItem->setSelected(true);
    GuiActionContribution createTransition;
    for (const GuiExtensionPlugin* plugin : GuiExtensionPluginCatalog::resolvedPlugins()) {
        if (plugin == nullptr || plugin->extensionId() != "modal.markov.gui") continue;
        GuiExtensionRegistry registry;
        plugin->registerContributions(&registry);
        for (const auto& action : registry.actions()) {
            if (action.id == "modal.markov.create-transition") createTransition = action;
        }
    }
    ASSERT_TRUE(static_cast<bool>(createTransition.trigger));
    GuiExtensionRuntimeContext context;
    context.simulator = &simulator;
    context.graphicsScene = &scene;
    ASSERT_TRUE(createTransition.isEnabled(context));
    createTransition.trigger(context);
    ASSERT_TRUE(scene.isNetworkRelationCreationActive());
    QGraphicsSceneMouseEvent destinationClick(QEvent::GraphicsSceneMousePress);
    destinationClick.setButton(Qt::LeftButton);
    destinationClick.setButtons(Qt::LeftButton);
    destinationClick.setScenePos(secondItem->sceneBoundingRect().center());
    scene.mousePressEvent(&destinationClick);
    ASSERT_EQ(network.getTransitions()->size(), 1u);
    auto* transition = network.getTransitions()->front();
    EXPECT_EQ(transition->getSource(), first);
    EXPECT_EQ(transition->getDestination(), second);
    EXPECT_DOUBLE_EQ(transition->getProbability(), 1.0);
    EXPECT_EQ(scene.extensionOverlayItems(QStringLiteral("modal.markov.gui")).size(), 4);
    undoStack.undo();
    EXPECT_TRUE(network.getTransitions()->empty());
    EXPECT_EQ(scene.extensionOverlayItems(QStringLiteral("modal.markov.gui")).size(), 2);
    undoStack.redo();
    ASSERT_EQ(network.getTransitions()->size(), 1u);
    EXPECT_EQ(network.getTransitions()->front(), transition);
    EXPECT_EQ(scene.extensionOverlayItems(QStringLiteral("modal.markov.gui")).size(), 4);

    auto actionById = [](const std::string& id) {
        GuiActionContribution result;
        for (const GuiExtensionPlugin* plugin : GuiExtensionPluginCatalog::resolvedPlugins()) {
            if (plugin == nullptr || plugin->extensionId() != "modal.markov.gui") continue;
            GuiExtensionRegistry registry;
            plugin->registerContributions(&registry);
            for (const auto& action : registry.actions()) if (action.id == id) result = action;
        }
        return result;
    };
    auto removeTransition = actionById("modal.markov.remove-transition");
    ASSERT_TRUE(static_cast<bool>(removeTransition.trigger));
    scene.clearSelection();
    QGraphicsItem* transitionItem = nullptr;
    for (QGraphicsItem* item : scene.extensionOverlayItems(QStringLiteral("modal.markov.gui"))) {
        if (dynamic_cast<QGraphicsPathItem*>(item) != nullptr && item->data(1).isValid()) transitionItem = item;
    }
    ASSERT_NE(transitionItem, nullptr);
    transitionItem->setSelected(true);
    ASSERT_TRUE(removeTransition.isEnabled(context));
    removeTransition.trigger(context);
    EXPECT_TRUE(network.getTransitions()->empty());
    undoStack.undo();
    ASSERT_EQ(network.getTransitions()->size(), 1u);
    scene.clearSelection();
    for (QGraphicsItem* item : scene.extensionOverlayItems(QStringLiteral("modal.markov.gui"))) {
        if (dynamic_cast<QGraphicsPathItem*>(item) != nullptr && item->data(1).isValid()) transitionItem = item;
    }
    ASSERT_NE(transitionItem, nullptr);
    auto* probabilityCommand = GuiExtensionPluginCatalog::createNetworkElementPropertyCommand(
        &network, transitionItem, "Probability", "0.35");
    ASSERT_NE(probabilityCommand, nullptr);
    undoStack.push(probabilityCommand);
    EXPECT_DOUBLE_EQ(transition->getProbability(), 0.35);
    undoStack.undo();
    EXPECT_DOUBLE_EQ(transition->getProbability(), 1.0);
    undoStack.redo();
    EXPECT_DOUBLE_EQ(transition->getProbability(), 0.35);

    auto setInitial = actionById("modal.markov.set-initial-state");
    ASSERT_TRUE(static_cast<bool>(setInitial.trigger));
    scene.clearSelection();
    secondItem->setSelected(true);
    ASSERT_TRUE(setInitial.isEnabled(context));
    setInitial.trigger(context);
    EXPECT_EQ(network.getInitialState(), second);
    undoStack.undo();
    EXPECT_EQ(network.getInitialState(), first);
    undoStack.redo();
    EXPECT_EQ(network.getInitialState(), second);
    network.setCurrentState(first);
    network.activate(NetworkActivationFrame(network.getNumInputPorts()));
    ASSERT_EQ(network.getLastTransition(), transition);
    GuiExtensionPluginCatalog::synchronizeDiagramProjections(&simulator, &scene);
    bool sampledTransitionHighlighted = false;
    for (QGraphicsItem* item : scene.extensionOverlayItems(QStringLiteral("modal.markov.gui"))) {
        auto* path = dynamic_cast<QGraphicsPathItem*>(item);
        if (path != nullptr && path->data(1).isValid())
            sampledTransitionHighlighted = path->pen().color() == QColor(225, 90, 35);
    }
    EXPECT_TRUE(sampledTransitionHighlighted);
}

TEST(MarkovChainNetworkGuiExtension, DroppedMarkovStateJoinsTheNetworkAndScene) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    MarkovChainNetwork network(model, "DroppedMarkovStateNetwork");
    ExposedModelGraphicsScene scene;
    scene.setSimulator(&simulator);
    scene.setModelLevelFilter(static_cast<unsigned int>(network.getId()));
    scene.setNetworkEditorOwner(&network);
    Plugin* statePlugin = simulator.getPluginManager()->find(Util::TypeOf<MarkovState>());
    ASSERT_NE(statePlugin, nullptr);
    QTreeWidgetItem draggedItem;
    draggedItem.setWhatsThis(0, QString::fromStdString(statePlugin->getPluginInfo()->getPluginTypename()));
    scene.setObjectBeingDragged(&draggedItem);
    QGraphicsSceneDragDropEvent event(QEvent::GraphicsSceneDrop);
    event.setScenePos(QPointF(150, 90));
    scene.dropEvent(&event);

    ASSERT_EQ(network.getStates()->size(), 1u);
    EXPECT_EQ(network.getStates()->front()->getLevel(), network.getId());
    ASSERT_EQ(scene.getGraphicalModelDataDefinitions()->size(), 1);
    EXPECT_EQ(dynamic_cast<GraphicalModelDataDefinition*>(scene.getGraphicalModelDataDefinitions()->front())
                  ->getDataDefinition(), network.getStates()->front());
}

TEST(MarkovChainNetworkGuiExtension, ValidationActionReportsInvalidProbabilityRows) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    MarkovChainNetwork network(model, "InvalidMarkovRow");
    auto* state = new MarkovState(model, "IncompleteRow");
    network.addState(state);
    ExposedModelGraphicsScene scene;
    scene.setSimulator(&simulator);
    scene.setNetworkEditorOwner(&network);
    GuiActionContribution validate;
    for (const GuiExtensionPlugin* plugin : GuiExtensionPluginCatalog::resolvedPlugins()) {
        if (plugin == nullptr || plugin->extensionId() != "modal.markov.gui") continue;
        GuiExtensionRegistry registry;
        plugin->registerContributions(&registry);
        for (const auto& action : registry.actions())
            if (action.id == "modal.markov.validate-network") validate = action;
    }
    ASSERT_TRUE(static_cast<bool>(validate.trigger));
    GuiExtensionRuntimeContext context;
    context.simulator = &simulator;
    context.graphicsScene = &scene;
    ASSERT_TRUE(validate.isEnabled(context));
    QTimer::singleShot(0, []() {
        auto* message = qobject_cast<QMessageBox*>(QApplication::activeModalWidget());
        ASSERT_NE(message, nullptr);
        EXPECT_NE(message->text().indexOf(QStringLiteral("must sum to 1.0")), -1);
        message->accept();
    });
    validate.trigger(context);
}

TEST(MarkovChainNetworkGuiExtension, RemovesStateAndTransitionsWithUndoRedoAndStableGeometry) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* network = new MarkovChainNetwork(model, "UndoMarkovState");
    auto* first = new MarkovState(model, "First");
    auto* middle = new MarkovState(model, "Middle");
    auto* last = new MarkovState(model, "Last");
    network->addState(first);
    network->addState(middle);
    network->addState(last);
    network->setInitialState(first);
    network->setCurrentState(middle);
    auto* firstTransition = new MarkovChainNetwork::MarkovTransition(first, middle, 0.75, "FirstTransition");
    auto* secondTransition = new MarkovChainNetwork::MarkovTransition(middle, last, 1.0, "SecondTransition");
    network->addTransition(firstTransition);
    network->addTransition(secondTransition);

    ExposedModelGraphicsScene scene;
    scene.setSimulator(&simulator);
    scene.setModelLevelFilter(static_cast<unsigned int>(network->getId()));
    scene.setNetworkEditorOwner(network);
    QUndoStack undoStack;
    scene.setUndoStack(&undoStack);
    Plugin* statePlugin = simulator.getPluginManager()->find(Util::TypeOf<MarkovState>());
    ASSERT_NE(statePlugin, nullptr);
    scene.addGraphicalModelDataDefinition(statePlugin, first, QPointF(20, 40));
    auto* middleItem = scene.addGraphicalModelDataDefinition(statePlugin, middle, QPointF(220, 140));
    scene.addGraphicalModelDataDefinition(statePlugin, last, QPointF(420, 40));
    const QPointF originalMiddlePosition = middleItem->pos();
    middleItem->setSelected(true);

    GuiActionContribution removeState;
    for (const GuiExtensionPlugin* plugin : GuiExtensionPluginCatalog::resolvedPlugins()) {
        if (plugin == nullptr || plugin->extensionId() != "modal.markov.gui") continue;
        GuiExtensionRegistry registry;
        plugin->registerContributions(&registry);
        for (const auto& action : registry.actions()) if (action.id == "modal.markov.remove-state") removeState = action;
    }
    ASSERT_TRUE(static_cast<bool>(removeState.trigger));
    GuiExtensionRuntimeContext context;
    context.simulator = &simulator;
    context.graphicsScene = &scene;
    ASSERT_TRUE(removeState.isEnabled(context));
    removeState.trigger(context);
    EXPECT_EQ(network->getStates()->size(), 2u);
    EXPECT_TRUE(network->getTransitions()->empty());
    EXPECT_EQ(network->getCurrentState(), nullptr);

    undoStack.undo();
    ASSERT_EQ(network->getStates()->size(), 3u);
    ASSERT_EQ(network->getTransitions()->size(), 2u);
    EXPECT_EQ(network->getTransitions()->front(), firstTransition);
    EXPECT_EQ(*std::next(network->getTransitions()->list()->begin()), secondTransition);
    EXPECT_EQ(network->getInitialState(), first);
    EXPECT_EQ(network->getCurrentState(), middle);
    EXPECT_EQ(scene.findGraphicalModelDataDefinition(middle)->pos(), originalMiddlePosition);
    undoStack.redo();
    EXPECT_EQ(network->getStates()->size(), 2u);
    EXPECT_TRUE(network->getTransitions()->empty());
    undoStack.undo();
    EXPECT_EQ(network->getTransitions()->front(), firstTransition);
    EXPECT_EQ(*std::next(network->getTransitions()->list()->begin()), secondTransition);
}

TEST(ColoredPetriNetGuiExtension, ProjectsMarkingAndFiresSelectedTransitionWithUndoRedo) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    ColoredPetriNetNetwork network(model, "EditableCPN");
    auto* input = new PetriPlace(model, "Input");
    auto* output = new PetriPlace(model, "Output");
    input->setTokens(2, "red");
    network.addPlace(input);
    network.addPlace(output);
    auto* transition = new CPNTransition(model, "MoveToken");
    network.addTransition(transition);
    auto* incoming = new CPNArc(model, input, transition, CPNArc::Direction::PlaceToTransition, "InputArc");
    incoming->setInscription("red", 1);
    auto* outgoing = new CPNArc(model, output, transition, CPNArc::Direction::TransitionToPlace, "OutputArc");
    outgoing->setInscription("red", 1);
    ASSERT_TRUE(network.addArc(incoming));
    ASSERT_TRUE(network.addArc(outgoing));

    ExposedModelGraphicsScene scene;
    scene.setSimulator(&simulator);
    scene.setNetworkEditorOwner(&network);
    QUndoStack undoStack;
    scene.setUndoStack(&undoStack);
    Plugin* placePlugin = simulator.getPluginManager()->find(Util::TypeOf<PetriPlace>());
    Plugin* transitionPlugin = simulator.getPluginManager()->find(Util::TypeOf<CPNTransition>());
    ASSERT_NE(placePlugin, nullptr);
    ASSERT_NE(transitionPlugin, nullptr);
    scene.addGraphicalModelDataDefinition(placePlugin, input, QPointF(20, 20));
    scene.addGraphicalModelDataDefinition(transitionPlugin, transition, QPointF(220, 20));
    scene.addGraphicalModelDataDefinition(placePlugin, output, QPointF(420, 20));
    GuiExtensionPluginCatalog::synchronizeDiagramProjections(&simulator, &scene);
    EXPECT_EQ(scene.extensionOverlayItems(QStringLiteral("modal.cpn.gui")).size(), 6);
    ASSERT_TRUE(network.isEnabled(transition));

    GuiActionContribution fire;
    for (const GuiExtensionPlugin* plugin : GuiExtensionPluginCatalog::resolvedPlugins()) {
        if (plugin == nullptr || plugin->extensionId() != "modal.cpn.gui") continue;
        GuiExtensionRegistry registry;
        plugin->registerContributions(&registry);
        for (const auto& action : registry.actions()) if (action.id == "modal.cpn.fire-transition") fire = action;
    }
    ASSERT_TRUE(static_cast<bool>(fire.trigger));
    GuiExtensionRuntimeContext context;
    context.simulator = &simulator;
    context.graphicsScene = &scene;
    auto* transitionItem = scene.findGraphicalModelDataDefinition(transition);
    ASSERT_NE(transitionItem, nullptr);
    auto* guardCommand = GuiExtensionPluginCatalog::createNetworkDataDefinitionPropertyCommand(&network, transition, "Guard Expression", "1");
    ASSERT_NE(guardCommand, nullptr); undoStack.push(guardCommand);
    auto* priorityCommand = GuiExtensionPluginCatalog::createNetworkDataDefinitionPropertyCommand(&network, transition, "Priority", "7");
    ASSERT_NE(priorityCommand, nullptr); undoStack.push(priorityCommand);
    EXPECT_EQ(transition->getGuardExpression(), "1");
    EXPECT_EQ(transition->getPriority(), 7u);
    undoStack.undo();
    undoStack.undo();
    EXPECT_TRUE(transition->getGuardExpression().empty());
    EXPECT_EQ(transition->getPriority(), 0u);
    undoStack.redo();
    undoStack.redo();
    EXPECT_EQ(transition->getPriority(), 7u);
    transitionItem->setSelected(true);
    ASSERT_TRUE(fire.isEnabled(context));
    fire.trigger(context);
    EXPECT_EQ(input->getTokens("red"), 1u);
    EXPECT_EQ(output->getTokens("red"), 1u);
    undoStack.undo();
    EXPECT_EQ(input->getTokens("red"), 2u);
    EXPECT_EQ(output->getTokens("red"), 0u);
    EXPECT_EQ(network.getLastFiredTransition(), nullptr);
    undoStack.redo();
    EXPECT_EQ(input->getTokens("red"), 1u);
    EXPECT_EQ(output->getTokens("red"), 1u);
    EXPECT_EQ(network.getLastFiredTransition(), transition);
    scene.clearSelection();
    GuiExtensionPluginCatalog::synchronizeDiagramProjections(&simulator, &scene);
    bool firedTransitionHighlighted = false;
    for (QGraphicsItem* item : scene.extensionOverlayItems(QStringLiteral("modal.cpn.gui"))) {
        auto* badge = dynamic_cast<QGraphicsRectItem*>(item);
        if (badge != nullptr) firedTransitionHighlighted = badge->pen().color() == QColor(225, 90, 35);
    }
    EXPECT_TRUE(firedTransitionHighlighted);
    int firedArcCount = 0;
    for (QGraphicsItem* item : scene.extensionOverlayItems(QStringLiteral("modal.cpn.gui"))) {
        auto* arc = dynamic_cast<QGraphicsPathItem*>(item);
        if (arc != nullptr && arc->data(1).isValid() && arc->pen().color() == QColor(225, 90, 35)) {
            ++firedArcCount;
        }
    }
    EXPECT_EQ(firedArcCount, 2);
}

TEST(ColoredPetriNetGuiExtension, RemovesTransitionWithIncidentArcsAndUndoPreservesMarkingAndOrder) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* network = new ColoredPetriNetNetwork(model, "UndoCpnTransition");
    auto* input = new PetriPlace(model, "Input");
    auto* output = new PetriPlace(model, "Output");
    auto* transition = new CPNTransition(model, "Move");
    input->setTokens(3, "red");
    network->addPlace(input);
    network->addPlace(output);
    network->addTransition(transition);
    network->setInitialTokens(input, "red", 3);
    auto* incoming = new CPNArc(model, input, transition, CPNArc::Direction::PlaceToTransition, "Incoming");
    auto* outgoing = new CPNArc(model, output, transition, CPNArc::Direction::TransitionToPlace, "Outgoing");
    incoming->setInscription("red", 1);
    outgoing->setInscription("red", 1);
    ASSERT_TRUE(network->addArc(incoming));
    ASSERT_TRUE(network->addArc(outgoing));
    ASSERT_TRUE(network->fire(transition));
    ASSERT_EQ(input->getTokens("red"), 2u);

    ExposedModelGraphicsScene scene;
    scene.setSimulator(&simulator);
    scene.setModelLevelFilter(static_cast<unsigned int>(network->getId()));
    scene.setNetworkEditorOwner(network);
    QUndoStack undoStack;
    scene.setUndoStack(&undoStack);
    Plugin* placePlugin = simulator.getPluginManager()->find(Util::TypeOf<PetriPlace>());
    Plugin* transitionPlugin = simulator.getPluginManager()->find(Util::TypeOf<CPNTransition>());
    ASSERT_NE(placePlugin, nullptr);
    ASSERT_NE(transitionPlugin, nullptr);
    scene.addGraphicalModelDataDefinition(placePlugin, input, QPointF(20, 40));
    auto* transitionItem = scene.addGraphicalModelDataDefinition(transitionPlugin, transition, QPointF(220, 140));
    scene.addGraphicalModelDataDefinition(placePlugin, output, QPointF(420, 40));
    const QPointF originalPosition = transitionItem->pos();
    transitionItem->setSelected(true);

    GuiActionContribution removeNode;
    for (const GuiExtensionPlugin* plugin : GuiExtensionPluginCatalog::resolvedPlugins()) {
        if (plugin == nullptr || plugin->extensionId() != "modal.cpn.gui") continue;
        GuiExtensionRegistry registry;
        plugin->registerContributions(&registry);
        for (const auto& action : registry.actions()) if (action.id == "modal.cpn.remove-node") removeNode = action;
    }
    ASSERT_TRUE(static_cast<bool>(removeNode.trigger));
    GuiExtensionRuntimeContext context;
    context.simulator = &simulator;
    context.graphicsScene = &scene;
    ASSERT_TRUE(removeNode.isEnabled(context));
    removeNode.trigger(context);
    EXPECT_EQ(network->getTransitions()->size(), 0u);
    EXPECT_EQ(network->getArcs()->size(), 0u);
    EXPECT_EQ(network->getLastFiredTransition(), nullptr);

    undoStack.undo();
    ASSERT_EQ(network->getTransitions()->size(), 1u);
    ASSERT_EQ(network->getArcs()->size(), 2u);
    EXPECT_EQ(network->getArcs()->front(), incoming);
    EXPECT_EQ(*std::next(network->getArcs()->list()->begin()), outgoing);
    EXPECT_EQ(network->getLastFiredTransition(), transition);
    EXPECT_EQ(network->getInitialTokens(input, "red"), 3u);
    EXPECT_EQ(input->getTokens("red"), 2u);
    EXPECT_EQ(scene.findGraphicalModelDataDefinition(transition)->pos(), originalPosition);
}

TEST(ColoredPetriNetGuiExtension, DroppedPlacesAndTransitionsJoinTheNetwork) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    ColoredPetriNetNetwork network(model, "DroppedCPNNodes");
    ExposedModelGraphicsScene scene;
    scene.setSimulator(&simulator);
    scene.setModelLevelFilter(static_cast<unsigned int>(network.getId()));
    scene.setNetworkEditorOwner(&network);
    QUndoStack undoStack;
    scene.setUndoStack(&undoStack);
    auto drop = [&](const std::string& pluginType) {
        QTreeWidgetItem draggedItem;
        draggedItem.setWhatsThis(0, QString::fromStdString(pluginType));
        scene.setObjectBeingDragged(&draggedItem);
        QGraphicsSceneDragDropEvent event(QEvent::GraphicsSceneDrop);
        event.setScenePos(QPointF(150 + scene.getGraphicalModelDataDefinitions()->size() * 180, 90));
        scene.dropEvent(&event);
    };
    drop(Util::TypeOf<PetriPlace>());
    drop(Util::TypeOf<CPNTransition>());
    EXPECT_EQ(network.getPlaces()->size(), 1u);
    EXPECT_EQ(network.getTransitions()->size(), 1u);
    EXPECT_EQ(scene.getGraphicalModelDataDefinitions()->size(), 2);
    auto* place = network.getPlaces()->front();
    auto* placeItem = scene.findGraphicalModelDataDefinition(place);
    ASSERT_NE(placeItem, nullptr);
    auto* markingCommand = GuiExtensionPluginCatalog::createNetworkDataDefinitionPropertyCommand(&network, place, "Initial Marking", "default=2;green=4");
    ASSERT_NE(markingCommand, nullptr); undoStack.push(markingCommand);
    EXPECT_EQ(network.getInitialTokens(place, "default"), 2u); EXPECT_EQ(network.getInitialTokens(place, "green"), 4u);
    undoStack.undo(); EXPECT_EQ(network.getInitialTokens(place, "green"), 0u);
    undoStack.redo(); EXPECT_EQ(network.getInitialTokens(place, "green"), 4u);
}

TEST(ColoredPetriNetGuiExtension, CreatesAndEditsBipartiteArcThroughSharedEditor) {
    Simulator simulator; simulator.getPluginManager()->autoInsertPlugins(); Model* model = simulator.getModelManager()->newModel(); ASSERT_NE(model, nullptr);
    ColoredPetriNetNetwork network(model, "ArcEditorCPN"); auto* place = new PetriPlace(model, "P"); auto* transition = new CPNTransition(model, "T");
    network.addPlace(place); network.addTransition(transition);
    ExposedModelGraphicsScene scene; scene.setSimulator(&simulator); scene.setNetworkEditorOwner(&network); QUndoStack undoStack; scene.setUndoStack(&undoStack);
    auto* placeItem = scene.addGraphicalModelDataDefinition(simulator.getPluginManager()->find(Util::TypeOf<PetriPlace>()), place, QPointF(20, 20));
    auto* transitionItem = scene.addGraphicalModelDataDefinition(simulator.getPluginManager()->find(Util::TypeOf<CPNTransition>()), transition, QPointF(220, 20)); transitionItem->setSelected(true);
    GuiActionContribution addArc;
    for (const GuiExtensionPlugin* plugin : GuiExtensionPluginCatalog::resolvedPlugins()) { if (!plugin || plugin->extensionId() != "modal.cpn.gui") continue; GuiExtensionRegistry registry; plugin->registerContributions(&registry); for (const auto& action : registry.actions()) if (action.id == "modal.cpn.create-arc") addArc = action; }
    ASSERT_TRUE(static_cast<bool>(addArc.trigger)); GuiExtensionRuntimeContext context; context.simulator = &simulator; context.graphicsScene = &scene;
    ASSERT_TRUE(addArc.isEnabled(context)); addArc.trigger(context); ASSERT_TRUE(scene.isNetworkRelationCreationActive());
    QGraphicsSceneMouseEvent click(QEvent::GraphicsSceneMousePress); click.setButton(Qt::LeftButton); click.setButtons(Qt::LeftButton); click.setScenePos(placeItem->sceneBoundingRect().center()); scene.mousePressEvent(&click);
    ASSERT_EQ(network.getArcs()->size(), 1u); CPNArc* arc = network.getArcs()->front(); EXPECT_TRUE(arc->isOutputArc()); EXPECT_EQ(arc->getInscription("default"), 1u);
    QGraphicsItem* arcItem = nullptr; for (QGraphicsItem* item : scene.extensionOverlayItems(QStringLiteral("modal.cpn.gui"))) if (dynamic_cast<QGraphicsPathItem*>(item) && item->data(1).isValid()) arcItem = item;
    ASSERT_NE(arcItem, nullptr); const auto props = GuiExtensionPluginCatalog::networkElementProperties(&network, arcItem); ASSERT_EQ(props.size(), 5u); EXPECT_EQ(props.back().name, "Inscription");
    auto* edit = GuiExtensionPluginCatalog::createNetworkElementPropertyCommand(&network, arcItem, "Inscription", "blue=3;green=2"); ASSERT_NE(edit, nullptr); undoStack.push(edit);
    EXPECT_EQ(arc->getInscription("blue"), 3u); EXPECT_EQ(arc->getInscription("green"), 2u); undoStack.undo(); EXPECT_EQ(arc->getInscription("default"), 1u); undoStack.redo(); EXPECT_EQ(arc->getInscription("green"), 2u);
    undoStack.undo(); undoStack.undo(); EXPECT_TRUE(network.getArcs()->empty()); undoStack.redo(); undoStack.redo(); ASSERT_EQ(network.getArcs()->size(), 1u); EXPECT_EQ(network.getArcs()->front(), arc);
}

TEST(GraphNetworkGuiExtension, DroppedGraphNodeJoinsTheNetworkAndScene) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    GraphNetwork network(model, "DroppedNodeGraph");
    ExposedModelGraphicsScene scene;
    scene.setSimulator(&simulator);
    scene.setModelLevelFilter(static_cast<unsigned int>(network.getId()));
    scene.setNetworkEditorOwner(&network);
    Plugin* nodePlugin = simulator.getPluginManager()->find(Util::TypeOf<GraphNode>());
    ASSERT_NE(nodePlugin, nullptr);
    QTreeWidgetItem draggedItem;
    draggedItem.setWhatsThis(0, QString::fromStdString(nodePlugin->getPluginInfo()->getPluginTypename()));
    scene.setObjectBeingDragged(&draggedItem);
    QGraphicsSceneDragDropEvent event(QEvent::GraphicsSceneDrop);
    event.setScenePos(QPointF(150, 90));
    scene.dropEvent(&event);

    ASSERT_EQ(network.getNodes().size(), 1u);
    EXPECT_EQ(network.getNodes().front()->getLevel(), network.getId());
    ASSERT_EQ(scene.getGraphicalModelDataDefinitions()->size(), 1);
    EXPECT_EQ(dynamic_cast<GraphicalModelDataDefinition*>(scene.getGraphicalModelDataDefinitions()->front())
                  ->getDataDefinition(), network.getNodes().front());
}

TEST(GraphNetworkGuiExtension, DroppedGraphNodeCanBeUndoneAndRedone) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    GraphNetwork network(model, "UndoDroppedNodeGraph");
    ExposedModelGraphicsScene scene;
    scene.setSimulator(&simulator);
    scene.setModelLevelFilter(static_cast<unsigned int>(network.getId()));
    scene.setNetworkEditorOwner(&network);
    QUndoStack undoStack;
    scene.setUndoStack(&undoStack);
    Plugin* nodePlugin = simulator.getPluginManager()->find(Util::TypeOf<GraphNode>());
    ASSERT_NE(nodePlugin, nullptr);
    QTreeWidgetItem draggedItem;
    draggedItem.setWhatsThis(0, QString::fromStdString(nodePlugin->getPluginInfo()->getPluginTypename()));
    scene.setObjectBeingDragged(&draggedItem);
    QGraphicsSceneDragDropEvent event(QEvent::GraphicsSceneDrop);
    event.setScenePos(QPointF(150, 90));
    scene.dropEvent(&event);

    ASSERT_EQ(network.getNodes().size(), 1u);
    auto* node = network.getNodes().front();
    auto* graphical = scene.findGraphicalModelDataDefinition(node);
    ASSERT_NE(graphical, nullptr);
    const QPointF originalPosition = graphical->pos();
    ASSERT_TRUE(undoStack.canUndo());
    undoStack.undo();
    EXPECT_TRUE(network.getNodes().empty());
    EXPECT_EQ(graphical->scene(), nullptr);
    EXPECT_EQ(scene.findGraphicalModelDataDefinition(node), nullptr);
    EXPECT_EQ(model->getDataManager()->getDataDefinition(
                  nodePlugin->getPluginInfo()->getPluginTypename(), node->getId()), nullptr);

    undoStack.redo();
    ASSERT_EQ(network.getNodes().size(), 1u);
    EXPECT_EQ(network.getNodes().front(), node);
    EXPECT_EQ(model->getDataManager()->getDataDefinition(
                  nodePlugin->getPluginInfo()->getPluginTypename(), node->getId()), node);
    EXPECT_EQ(scene.findGraphicalModelDataDefinition(node), graphical);
    EXPECT_EQ(graphical->pos(), originalPosition);
}

TEST(GraphNetworkGuiExtension, CreatesAndEditsEdgesThroughTheSharedEditorWithUndoRedo) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    DirectedGraphNetwork network(model, "EditableDirectedGraph");
    auto* first = new GraphNode(model, "Start");
    auto* second = new GraphNode(model, "Finish");
    ASSERT_TRUE(network.addNode(first));
    ASSERT_TRUE(network.addNode(second));

    ExposedModelGraphicsScene scene;
    scene.setSimulator(&simulator);
    scene.setNetworkEditorOwner(&network);
    QUndoStack undoStack;
    scene.setUndoStack(&undoStack);
    Plugin* nodePlugin = simulator.getPluginManager()->find(Util::TypeOf<GraphNode>());
    ASSERT_NE(nodePlugin, nullptr);
    auto* firstItem = scene.addGraphicalModelDataDefinition(nodePlugin, first, QPointF(20, 20));
    auto* secondItem = scene.addGraphicalModelDataDefinition(nodePlugin, second, QPointF(220, 20));
    firstItem->setSelected(true);

    GuiActionContribution createEdge;
    for (const GuiExtensionPlugin* plugin : GuiExtensionPluginCatalog::resolvedPlugins()) {
        if (plugin == nullptr || plugin->extensionId() != "modal.graph.gui") continue;
        GuiExtensionRegistry registry;
        plugin->registerContributions(&registry);
        for (const auto& action : registry.actions()) if (action.id == "modal.graph.create-edge") createEdge = action;
    }
    ASSERT_TRUE(static_cast<bool>(createEdge.trigger));
    GuiExtensionRuntimeContext context;
    context.simulator = &simulator;
    context.graphicsScene = &scene;
    ASSERT_TRUE(createEdge.isEnabled(context));
    createEdge.trigger(context);
    ASSERT_TRUE(scene.isNetworkRelationCreationActive());
    EXPECT_TRUE(network.getEdges().empty());

    QGraphicsSceneMouseEvent click(QEvent::GraphicsSceneMousePress);
    click.setButton(Qt::LeftButton);
    click.setButtons(Qt::LeftButton);
    click.setScenePos(secondItem->sceneBoundingRect().center());
    scene.mousePressEvent(&click);
    ASSERT_FALSE(scene.isNetworkRelationCreationActive());
    ASSERT_EQ(network.getEdges().size(), 1u);
    GraphEdge* edge = network.getEdges().front();
    EXPECT_EQ(edge->getSource(), first);
    EXPECT_EQ(edge->getDestination(), second);
    EXPECT_EQ(edge->getName(), "E0");
    EXPECT_EQ(scene.extensionOverlayItems(QStringLiteral("modal.graph.gui")).size(), 1);

    const auto overlayItems = scene.extensionOverlayItems(QStringLiteral("modal.graph.gui"));
    ASSERT_EQ(overlayItems.size(), 1);
    auto properties = GuiExtensionPluginCatalog::networkElementProperties(&network, overlayItems.front());
    ASSERT_EQ(properties.size(), 4u);
    EXPECT_EQ(properties[0].name, "Name");
    EXPECT_EQ(properties[1].name, "Source Node");
    EXPECT_FALSE(properties[1].editable);
    EXPECT_EQ(properties[2].name, "Destination Node");
    EXPECT_FALSE(properties[2].editable);
    EXPECT_EQ(properties[3].name, "Weight");
    QUndoCommand* weightCommand = GuiExtensionPluginCatalog::createNetworkElementPropertyCommand(
        &network, overlayItems.front(), "Weight", "4.25");
    ASSERT_NE(weightCommand, nullptr);
    undoStack.push(weightCommand);
    ASSERT_TRUE(edge->hasWeight());
    EXPECT_DOUBLE_EQ(edge->getWeight(), 4.25);
    undoStack.undo();
    EXPECT_FALSE(edge->hasWeight());
    undoStack.redo();
    EXPECT_DOUBLE_EQ(edge->getWeight(), 4.25);

    undoStack.undo();
    undoStack.undo();
    EXPECT_TRUE(network.getEdges().empty());
    EXPECT_TRUE(scene.extensionOverlayItems(QStringLiteral("modal.graph.gui")).isEmpty());
    undoStack.redo();
    undoStack.redo();
    ASSERT_EQ(network.getEdges().size(), 1u);
    EXPECT_EQ(scene.extensionOverlayItems(QStringLiteral("modal.graph.gui")).size(), 1);
}

TEST(GraphNetworkGuiExtension, RejectsCycleBeforeCreatingAnEdgeObject) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    DirectedAcyclicGraphNetwork network(model, "AcyclicEditorGraph");
    auto* first = new GraphNode(model, "A");
    auto* second = new GraphNode(model, "B");
    ASSERT_TRUE(network.addNode(first));
    ASSERT_TRUE(network.addNode(second));
    ExposedModelGraphicsScene scene;
    scene.setSimulator(&simulator);
    scene.setNetworkEditorOwner(&network);
    QUndoStack undoStack;
    scene.setUndoStack(&undoStack);
    Plugin* nodePlugin = simulator.getPluginManager()->find(Util::TypeOf<GraphNode>());
    ASSERT_NE(nodePlugin, nullptr);
    auto* firstItem = scene.addGraphicalModelDataDefinition(nodePlugin, first, QPointF(20, 20));
    auto* secondItem = scene.addGraphicalModelDataDefinition(nodePlugin, second, QPointF(220, 20));

    QUndoCommand* forward = GuiExtensionPluginCatalog::createNetworkRelationCommand(&network, firstItem, secondItem);
    ASSERT_NE(forward, nullptr);
    undoStack.push(forward);
    ASSERT_EQ(network.getEdges().size(), 1u);
    GraphEdge* edge = network.getEdges().front();
    EXPECT_EQ(edge->getSource(), first);
    EXPECT_EQ(edge->getDestination(), second);

    const unsigned int edgeObjectCount = model->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<GraphEdge>());
    EXPECT_EQ(GuiExtensionPluginCatalog::createNetworkRelationCommand(&network, secondItem, firstItem), nullptr);
    EXPECT_EQ(GuiExtensionPluginCatalog::createNetworkRelationCommand(&network, firstItem, firstItem), nullptr);
    EXPECT_EQ(network.getEdges().size(), 1u);
    EXPECT_EQ(model->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<GraphEdge>()), edgeObjectCount);
}

TEST(GuiExtensionManager, ActionContributionsRefreshForTheCurrentRuntimeContext) {
    class ContextActionPlugin final : public GuiExtensionPlugin {
    public:
        std::string extensionId() const override { return "context-action-test"; }
        std::string displayName() const override { return "Context action test"; }
        std::string version() const override { return "1"; }
        std::vector<std::string> requiredModelPlugins() const override { return {}; }
        void registerContributions(GuiExtensionRegistry* registry) const override {
            registry->addAction({
                "test.context-action", "Context action", "Model", {}, {}, {},
                [](const GuiExtensionRuntimeContext& context) { return context.graphicsScene != nullptr; },
                [](const GuiExtensionRuntimeContext& context) { return context.graphicsScene != nullptr; },
                [this](const GuiExtensionRuntimeContext& context) { lastTriggeredScene = context.graphicsScene; }});
        }
        mutable ModelGraphicsScene* lastTriggeredScene = nullptr;
    } plugin;

    QMainWindow host;
    ModelGraphicsScene scene(0, 0, 100, 100);
    GuiExtensionRuntimeContext context;
    context.mainWindow = &host;
    GuiExtensionManager manager(&host);
    manager.setPlugins({&plugin});
    manager.rebuild(context);

    QAction* action = host.findChild<QAction*>(QStringLiteral("test.context-action"));
    ASSERT_NE(action, nullptr);
    EXPECT_FALSE(action->isVisible());
    EXPECT_FALSE(action->isEnabled());

    context.graphicsScene = &scene;
    manager.refreshActionStates(context);
    EXPECT_TRUE(action->isVisible());
    EXPECT_TRUE(action->isEnabled());
    action->trigger();
    EXPECT_EQ(plugin.lastTriggeredScene, &scene);
}

TEST(GuiExtensionManager, NetworkProjectionRefreshIsQueuedToTheSceneThread) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    GraphNetwork network(model, "ThreadBoundProjection");
    auto* node = new GraphNode(model, "Loop");
    ASSERT_TRUE(network.addNode(node));
    ASSERT_TRUE(network.addEdge(new GraphEdge(model, node, node, "Self")));
    ExposedModelGraphicsScene scene;
    scene.setSimulator(&simulator);
    scene.setNetworkEditorOwner(&network);
    Plugin* nodePlugin = simulator.getPluginManager()->find(Util::TypeOf<GraphNode>());
    ASSERT_NE(nodePlugin, nullptr);
    scene.addGraphicalModelDataDefinition(nodePlugin, node, QPointF(80, 80));

    std::thread worker([&simulator, &scene]() {
        GuiExtensionPluginCatalog::synchronizeDiagramProjectionsOnSceneThread(&simulator, &scene);
    });
    worker.join();
    QCoreApplication::processEvents();
    EXPECT_EQ(scene.extensionOverlayItems(QStringLiteral("modal.graph.gui")).size(), 1);
}

TEST(GuiExtensionManager, QueuedNetworkProjectionIsDiscardedWhenSceneCloses) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    GraphNetwork network(model, "ClosingProjectionScene");
    auto* scene = new ExposedModelGraphicsScene();
    scene->setSimulator(&simulator);
    scene->setNetworkEditorOwner(&network);
    QPointer<ModelGraphicsScene> guardedScene(scene);

    std::thread worker([&simulator, scene]() {
        GuiExtensionPluginCatalog::synchronizeDiagramProjectionsOnSceneThread(&simulator, scene);
    });
    worker.join();
    delete scene;
    EXPECT_TRUE(guardedScene.isNull());
    QCoreApplication::processEvents();
}

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
    EXPECT_TRUE(efsmNetworkDiagramLayout(harness.serializer.get(),
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
    setEFSMNetworkDiagramLayout(harness.serializer.get(), firstModel, 123, firstLayout);
    setEFSMNetworkDiagramLayout(harness.serializer.get(), secondModel, 123, secondLayout);

    EXPECT_EQ(efsmNetworkDiagramLayout(harness.serializer.get(), firstModel, 123).networkName,
              QStringLiteral("FirstModelNetwork"));
    EXPECT_EQ(efsmNetworkDiagramLayout(harness.serializer.get(), secondModel, 123).networkName,
              QStringLiteral("SecondModelNetwork"));
    forgetEFSMNetworkLayouts(harness.serializer.get(), firstModel);
    EXPECT_TRUE(efsmNetworkDiagramLayout(harness.serializer.get(), firstModel, 123).isEmpty());
    EXPECT_EQ(efsmNetworkDiagramLayout(harness.serializer.get(), secondModel, 123).networkName,
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
    ASSERT_EQ(scene->extensionOverlayItems(QStringLiteral("efsm")).size(), 3);
    EXPECT_NE(scene->findGraphicalModelDataDefinition(&idle)->getColor(),
              scene->findGraphicalModelDataDefinition(&done)->getColor());

    auto* first = dynamic_cast<EFSMTransitionGraphicsItem*>(scene->extensionOverlayItems(QStringLiteral("efsm")).at(0));
    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first->ordinal(), 0);
    EXPECT_EQ(first->transitionLabel(), QStringLiteral("on arrive [ready == 1] / result = 1"));
    EXPECT_EQ(first->toolTip(), first->transitionLabel());
    auto* parallel = dynamic_cast<EFSMTransitionGraphicsItem*>(scene->extensionOverlayItems(QStringLiteral("efsm")).at(1));
    ASSERT_NE(parallel, nullptr);
    EXPECT_EQ(parallel->ordinal(), 1);
    EXPECT_NE(first->path(), parallel->path());
    EXPECT_TRUE(first->flags().testFlag(QGraphicsItem::ItemIsSelectable));
    EXPECT_EQ(scene->getGraphicalConnections()->size(), 0);
    EXPECT_EQ(network.getStates()->size(), 2u);
    EXPECT_EQ(network.getTransitions()->size(), 3u);

    guardedUnnamed->setGuardExpression("");
    network.activate(NetworkActivationFrame(network.getNumInputPorts()));
    EXPECT_EQ(network.getCurrentState(), &done);
    EXPECT_EQ(network.getLastTransition(), guardedUnnamed);
    EFSMNetworkDiagramProjection::synchronize(&simulator, scene);
    int firedTransitionCount = 0;
    for (QGraphicsItem* item : scene->extensionOverlayItems(QStringLiteral("efsm"))) {
        auto* transitionItem = dynamic_cast<EFSMTransitionGraphicsItem*>(item);
        if (transitionItem != nullptr && transitionItem->isFired()) ++firedTransitionCount;
    }
    EXPECT_EQ(firedTransitionCount, 1);
    const QColor currentStateColor = scene->findGraphicalModelDataDefinition(&done)->getColor();
    EXPECT_EQ(currentStateColor.red(), 105);
    EXPECT_EQ(currentStateColor.green(), 165);
    EXPECT_EQ(currentStateColor.blue(), 240);

    // Rebuilding the same context replaces only the projection layer.
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);
    ASSERT_EQ(scene->extensionOverlayItems(QStringLiteral("efsm")).size(), 3);
    EXPECT_EQ(scene->getGraphicalModelDataDefinitions()->size(), 2);

    EFSMNetwork empty(model, "EmptyViewerNetwork");
    scene->setModelLevelFilter(static_cast<unsigned int>(empty.getId()));
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);
    EXPECT_TRUE(scene->extensionOverlayItems(QStringLiteral("efsm")).isEmpty());
    EXPECT_TRUE(scene->getGraphicalModelDataDefinitions()->isEmpty());

    EFSMNetwork singleStateNetwork(model, "SingleStateViewerNetwork");
    FSMState onlyState(model, "Only");
    singleStateNetwork.addState(&onlyState);
    scene->setModelLevelFilter(static_cast<unsigned int>(singleStateNetwork.getId()));
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);
    EFSMNetworkDiagramProjection::synchronize(&simulator, scene);
    EXPECT_EQ(scene->getGraphicalModelDataDefinitions()->size(), 1);
    EXPECT_TRUE(scene->extensionOverlayItems(QStringLiteral("efsm")).isEmpty());
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
    ASSERT_EQ(scene.extensionOverlayItems(QStringLiteral("efsm")).size(), 1);
    auto* edgeItem = dynamic_cast<EFSMTransitionGraphicsItem*>(scene.extensionOverlayItems(QStringLiteral("efsm")).first());
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

    ASSERT_EQ(scene.extensionOverlayItems(QStringLiteral("efsm")).size(), 1);
    edgeItem = dynamic_cast<EFSMTransitionGraphicsItem*>(scene.extensionOverlayItems(QStringLiteral("efsm")).first());
    ASSERT_NE(edgeItem, nullptr);
    const QPointF moved = edgeItem->path().pointAtPercent(0.5);
    EXPECT_NE(moved, before);
    EXPECT_EQ(network.getTransitions()->size(), 1u);
    ASSERT_EQ(movementUndoStack.count(), 1);
    movementUndoStack.undo();
    ASSERT_EQ(scene.extensionOverlayItems(QStringLiteral("efsm")).size(), 1);
    edgeItem = dynamic_cast<EFSMTransitionGraphicsItem*>(scene.extensionOverlayItems(QStringLiteral("efsm")).first());
    ASSERT_NE(edgeItem, nullptr);
    EXPECT_NEAR(edgeItem->path().pointAtPercent(0.5).x(), before.x(), 0.01);
    EXPECT_NEAR(edgeItem->path().pointAtPercent(0.5).y(), before.y(), 0.01);
    movementUndoStack.redo();
    ASSERT_EQ(scene.extensionOverlayItems(QStringLiteral("efsm")).size(), 1);
    edgeItem = dynamic_cast<EFSMTransitionGraphicsItem*>(scene.extensionOverlayItems(QStringLiteral("efsm")).first());
    ASSERT_NE(edgeItem, nullptr);
    EXPECT_NE(edgeItem->path().pointAtPercent(0.5), before);
}

TEST(EFSMNetworkDiagramProjection, SelectingTransitionDoesNotTreatItsPathBoundsAsMovement) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "SelectTransitionNetwork");
    FSMState source(model, "SelectSource");
    FSMState destination(model, "SelectDestination");
    network.setInitialState(&source);
    network.addState(&destination);
    network.addTransition(new EFSMTransition(&source, &destination, "SelectEdge"));

    ExposedModelGraphicsScene scene;
    scene.setSimulator(&simulator);
    QUndoStack undoStack;
    scene.setUndoStack(&undoStack);
    scene.setModelLevelFilter(static_cast<unsigned int>(network.getId()));
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, &scene);
    auto* sourceItem = scene.findGraphicalModelDataDefinition(&source);
    auto* destinationItem = scene.findGraphicalModelDataDefinition(&destination);
    ASSERT_NE(sourceItem, nullptr);
    ASSERT_NE(destinationItem, nullptr);
    sourceItem->setPos(-180.0, 0.0);
    destinationItem->setPos(180.0, 0.0);
    EFSMNetworkDiagramProjection::synchronize(&simulator, &scene);
    ASSERT_EQ(scene.extensionOverlayItems(QStringLiteral("efsm")).size(), 1);
    auto* edge = dynamic_cast<EFSMTransitionGraphicsItem*>(scene.extensionOverlayItems(QStringLiteral("efsm")).first());
    ASSERT_NE(edge, nullptr);
    const QPointF clickPoint = edge->path().pointAtPercent(0.5);
    ASSERT_EQ(scene.itemAt(clickPoint, QTransform()), edge);

    QGraphicsSceneMouseEvent press(QEvent::GraphicsSceneMousePress);
    press.setButton(Qt::LeftButton);
    press.setButtons(Qt::LeftButton);
    press.setScenePos(clickPoint);
    press.setButtonDownScenePos(Qt::LeftButton, clickPoint);
    scene.mousePressEvent(&press);
    QGraphicsSceneMouseEvent release(QEvent::GraphicsSceneMouseRelease);
    release.setButton(Qt::LeftButton);
    release.setScenePos(clickPoint);
    scene.mouseReleaseEvent(&release);

    ASSERT_EQ(scene.extensionOverlayItems(QStringLiteral("efsm")).size(), 1);
    EXPECT_EQ(scene.extensionOverlayItems(QStringLiteral("efsm")).first(), edge);
    EXPECT_EQ(undoStack.count(), 0);
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
    ASSERT_EQ(scene->extensionOverlayItems(QStringLiteral("efsm")).size(), 2);

    auto* graphicalTransition = dynamic_cast<EFSMTransitionGraphicsItem*>(scene->extensionOverlayItems(QStringLiteral("efsm")).front());
    ASSERT_NE(graphicalTransition, nullptr);
    auto* graphicalParallel = dynamic_cast<EFSMTransitionGraphicsItem*>(scene->extensionOverlayItems(QStringLiteral("efsm")).at(1));
    ASSERT_NE(graphicalParallel, nullptr);
    graphicalTransition->setSelected(true);
    graphicalParallel->setSelected(true);
    EditCommandController editController(&simulator, &view, []() {}, nullptr, nullptr, nullptr, nullptr, nullptr);
    editController.onActionEditDeleteTriggered();

    EXPECT_EQ(network.getTransitions()->size(), 0u);
    EXPECT_TRUE(scene->extensionOverlayItems(QStringLiteral("efsm")).isEmpty());
    ASSERT_EQ(undoStack.count(), 1);
    editController.onActionEditUndoTriggered();
    ASSERT_EQ(network.getTransitions()->size(), 2u);
    EXPECT_EQ(network.getTransitions()->front(), transition);
    EXPECT_EQ(network.getTransitions()->getAtRank(1), parallel);
    ASSERT_EQ(scene->extensionOverlayItems(QStringLiteral("efsm")).size(), 2);
    auto* restored = dynamic_cast<EFSMTransitionGraphicsItem*>(scene->extensionOverlayItems(QStringLiteral("efsm")).front());
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->transition(), transition);
    auto* restoredParallel = dynamic_cast<EFSMTransitionGraphicsItem*>(scene->extensionOverlayItems(QStringLiteral("efsm")).at(1));
    ASSERT_NE(restoredParallel, nullptr);
    EXPECT_EQ(restoredParallel->transition(), parallel);

    editController.onActionEditRedoTriggered();
    EXPECT_TRUE(network.getTransitions()->empty());
    EXPECT_TRUE(scene->extensionOverlayItems(QStringLiteral("efsm")).isEmpty());
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
    auto* parallel0 = new EFSMTransition(&idle, &busy, "Parallel0");
    parallel0->setTriggerEvent("arrival");
    parallel0->setProbabilityExpression("0.75");
    parallel0->setGuardExpression("ready == 1");
    network.addTransition(parallel0);
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
    setEFSMNetworkDiagramLayout(harness.serializer.get(), model, layout.networkId, layout);

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
    EXPECT_EQ(loadedNetwork->getTransitions()->front()->getTriggerEvent(), "arrival");
    EXPECT_EQ(loadedNetwork->getTransitions()->front()->getProbabilityExpression(), "0.75");
    EXPECT_EQ(loadedNetwork->getTransitions()->front()->getGuardExpression(), "ready == 1");

    const EFSMNetworkDiagramLayout restored =
        efsmNetworkDiagramLayout(harness.serializer.get(), loadedModel, static_cast<unsigned long>(loadedNetwork->getId()));
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

TEST(ModalNetworkReferencePersistence, GenRoundTripKeepsTheExactTypeAndId) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* efsm = new EFSMNetwork(model, "OtherNetworkName");
    auto* graph = new GraphNetwork(model, "SameNetworkName");
    auto* adapter = new ModalModelDefault(model, "IdentityAdapter");
    adapter->setNetwork(graph);

    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());
    const std::string path = temporaryDir.filePath("typed_network_reference.gen").toStdString();
    ASSERT_TRUE(model->save(path));
    Model* loaded = simulator.getModelManager()->loadModel(path);
    ASSERT_NE(loaded, nullptr);
    auto* loadedAdapter = dynamic_cast<ModalModelDefault*>(loaded->getComponentManager()->find("IdentityAdapter"));
    ASSERT_NE(loadedAdapter, nullptr);
    auto* loadedGraph = dynamic_cast<GraphNetwork*>(loaded->getDataManager()->getDataDefinition(
        Util::TypeOf<GraphNetwork>(), graph->getId()));
    auto* loadedEfsm = dynamic_cast<EFSMNetwork*>(loaded->getDataManager()->getDataDefinition(
        Util::TypeOf<EFSMNetwork>(), efsm->getId()));
    ASSERT_NE(loadedGraph, nullptr);
    ASSERT_NE(loadedEfsm, nullptr);
    EXPECT_EQ(loadedAdapter->getNetwork(), loadedGraph);
    EXPECT_NE(loadedAdapter->getNetwork(), loadedEfsm);
}

TEST(ModalNetworkReferencePersistence, DamagedTypedIdentityFailsClosedWithDiagnostic) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* network = new GraphNetwork(model, "TypedReferenceTarget");
    ModalModelDefaultPersistenceProbe source(model, "TypedReferenceSource");
    source.setNetwork(network);
    ReferencePersistenceRuntime persistence;
    PersistenceRecord fields(persistence);
    source._saveInstance(&fields, true);
    fields.saveField("networkId", static_cast<int>(network->getId() + 10000), 0, true);

    ModalModelDefaultPersistenceProbe loaded(model, "DamagedTypedReference");
    ASSERT_TRUE(loaded._loadInstance(&fields));
    EXPECT_EQ(loaded.getNetwork(), nullptr);
    std::string diagnostic;
    EXPECT_FALSE(loaded._check(diagnostic));
    EXPECT_NE(diagnostic.find("missing or mismatched Network"), std::string::npos);
}

TEST(ModalNetworkReferencePersistence, LegacyNameOnlyReferenceResolvesOnlyWhenUnambiguous) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* graph = new GraphNetwork(model, "SharedLegacyName");
    auto* efsm = new EFSMNetwork(model, "SharedLegacyName");
    auto* uniqueGraph = new GraphNetwork(model, "UniqueLegacyName");
    ASSERT_NE(graph, nullptr);
    ASSERT_NE(efsm, nullptr);
    auto* adapter = new ModalModelDefault(model, "LegacyAdapter");

    adapter->setNetworkName("SharedLegacyName");
    EXPECT_EQ(adapter->getNetwork(), nullptr);
    std::string errorMessage;
    EXPECT_FALSE(ModelDataDefinition::Check(adapter, errorMessage));
    EXPECT_NE(errorMessage.find("ambiguous legacy Network name"), std::string::npos);

    adapter->setNetworkName(uniqueGraph->getName());
    ASSERT_NE(adapter->getNetwork(), nullptr);
    EXPECT_EQ(adapter->getNetwork(), uniqueGraph);
}

TEST(ModalNetworkReferencePersistence, DetachedBindingSchemaSurvivesGenRoundTrip) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* efsm = new EFSMNetwork(model, "DetachedSchemaEFSM");
    auto* cpn = new ColoredPetriNetNetwork(model, "DetachedSchemaCPN");
    auto* adapter = new ModalModelDefault(model, "DetachedSchemaAdapter");
    adapter->setNetwork(efsm);
    adapter->setInputBinding(0, "2");
    adapter->setOutputBinding(0, "accepted");
    adapter->setNetwork(nullptr);

    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());
    const std::string path = temporaryDir.filePath("detached_network_schema.gen").toStdString();
    ASSERT_TRUE(model->save(path));
    Model* loaded = simulator.getModelManager()->loadModel(path);
    ASSERT_NE(loaded, nullptr);
    auto* loadedAdapter = dynamic_cast<ModalModelDefault*>(loaded->getComponentManager()->find("DetachedSchemaAdapter"));
    auto* loadedCpn = dynamic_cast<ColoredPetriNetNetwork*>(loaded->getDataManager()->getDataDefinition(
        Util::TypeOf<ColoredPetriNetNetwork>(), cpn->getId()));
    ASSERT_NE(loadedAdapter, nullptr);
    ASSERT_NE(loadedCpn, nullptr);
    EXPECT_EQ(loadedAdapter->getNetwork(), nullptr);
    EXPECT_THROW(loadedAdapter->setNetwork(loadedCpn), std::invalid_argument);
    EXPECT_EQ(loadedAdapter->getNetwork(), nullptr);
    EXPECT_EQ(loadedAdapter->getInputBinding(0), "2");
    EXPECT_EQ(loadedAdapter->getOutputBinding(0), "accepted");
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
    setEFSMNetworkDiagramLayout(harness.serializer.get(), model, layout.networkId, layout);

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
    EXPECT_TRUE(efsmNetworkDiagramLayout(harness.serializer.get(),
        loadedModel, static_cast<unsigned long>(loadedNetwork->getId())).isEmpty());
}

TEST(EFSMNetworkDiagramPersistence, UnknownDiagramSectionSurvivesLoadAndSave) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    EFSMNetwork network(model, "Net");
    FSMState state(model, "A");
    network.addState(&state);

    SerializerHarness harness(&simulator);
    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());
    const QString inputFilename = temporaryDir.filePath("with-unknown.gui");
    const QString outputFilename = temporaryDir.filePath("round-trip.gui");
    ASSERT_TRUE(harness.serializer->saveGraphicalModel(inputFilename));

    QFile input(inputFilename);
    ASSERT_TRUE(input.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = QString::fromUtf8(input.readAll());
    input.close();
    const int insertionPoint = content.indexOf("\n# Groups");
    ASSERT_NE(insertionPoint, -1);
    content.insert(insertionPoint,
                   "\n# Future Formalism Diagrams\nfuture-record\tstable-id=42\n");
    ASSERT_TRUE(input.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
    input.write(content.toUtf8());
    input.close();

    ASSERT_NE(harness.serializer->loadGraphicalModel(inputFilename.toStdString()), nullptr);
    ASSERT_TRUE(harness.serializer->saveGraphicalModel(outputFilename));
    QFile output(outputFilename);
    ASSERT_TRUE(output.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString roundTrip = QString::fromUtf8(output.readAll());
    EXPECT_TRUE(roundTrip.contains(
        "# Future Formalism Diagrams\nfuture-record\tstable-id=42"));
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
    EXPECT_TRUE(efsmNetworkDiagramLayout(harness.serializer.get(),
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
    setEFSMNetworkDiagramLayout(harness.serializer.get(), model, layout.networkId, layout);

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
    const EFSMNetworkDiagramLayout restoredLayout = efsmNetworkDiagramLayout(harness.serializer.get(),
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

TEST(EFSMNetworkEditor, LegacyPropertyEditActionsAreNotRegistered) {
    for (const GuiExtensionPlugin* plugin : GuiExtensionPluginCatalog::resolvedPlugins()) {
        if (plugin == nullptr) continue;
        GuiExtensionRegistry registry;
        plugin->registerContributions(&registry);
        for (const auto& action : registry.actions()) {
            EXPECT_NE(action.id, "modal.efsm.edit-state");
            EXPECT_NE(action.id, "modal.efsm.edit-transition");
        }
        for (const auto& menu : registry.contextMenus()) {
            EXPECT_NE(menu.id, "modal.efsm.edit-transition.context");
        }
    }
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

    QAction* createStateAction = nullptr;
    for (QAction* action : window.findChildren<QAction*>()) {
        if (action->objectName() == QStringLiteral("modal.efsm.create-state")) createStateAction = action;
    }
    ASSERT_NE(createStateAction, nullptr);
    EXPECT_FALSE(createStateAction->isVisible());

    QAction* openAction = nullptr;
    for (QAction* action : window.findChildren<QAction*>()) {
        if (action->objectName() == QStringLiteral("actionOpenSelectedSubmodel")) {
            openAction = action;
        }
    }
    ASSERT_NE(openAction, nullptr);
    firstItem->setSelected(true);
    EXPECT_EQ(openAction->text(), QStringLiteral("Open Network"));
    EXPECT_TRUE(openAction->isEnabled());
    openAction->trigger();
    // Opening the submodel rebuilds the active extension context, so resolve
    // the action again instead of retaining an action owned by the old manager.
    createStateAction = nullptr;
    for (QAction* action : window.findChildren<QAction*>()) {
        if (action->objectName() == QStringLiteral("modal.efsm.create-state")) createStateAction = action;
    }
    ASSERT_NE(createStateAction, nullptr);
    ASSERT_EQ(tabs->count(), rootCount + 1);
    auto* networkView = dynamic_cast<ModelGraphicsView*>(tabs->currentWidget());
    ASSERT_NE(networkView, nullptr);
    auto* pluginPalette = window.findChild<QTreeWidget*>(QStringLiteral("treeWidget_Plugins"));
    ASSERT_NE(pluginPalette, nullptr);
    EXPECT_TRUE(pluginPalette->findItems(QString::fromStdString(Util::TypeOf<Create>()),
        Qt::MatchExactly | Qt::MatchRecursive, 0).isEmpty());
    EXPECT_FALSE(pluginPalette->findItems(QString::fromStdString(Util::TypeOf<FSMState>()),
        Qt::MatchExactly | Qt::MatchRecursive, 0).isEmpty());
    EXPECT_TRUE(tabs->tabText(tabs->currentIndex()).contains(QStringLiteral("SharedNavigationNetwork")));
    ASSERT_TRUE(networkView->getScene()->hasModelLevelFilter());
    EXPECT_EQ(networkView->getScene()->modelLevelFilter(), static_cast<unsigned int>(network->getId()));
    EXPECT_NE(networkView->getScene()->findGraphicalModelDataDefinition(state), nullptr);
    EXPECT_EQ(networkView->getScene()->extensionOverlayItems(QStringLiteral("efsm")).size(), 0);
    EXPECT_TRUE(createStateAction->isVisible());
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
    QApplication::processEvents();
    ASSERT_TRUE(window.myScene()->hasModelLevelFilter());
    ASSERT_EQ(window.myScene()->modelLevelFilter(), 0u);
    EXPECT_FALSE(pluginPalette->findItems(QString::fromStdString(Util::TypeOf<Create>()),
        Qt::MatchExactly | Qt::MatchRecursive, 0).isEmpty());
    EXPECT_TRUE(pluginPalette->findItems(QString::fromStdString(Util::TypeOf<FSMState>()),
        Qt::MatchExactly | Qt::MatchRecursive, 0).isEmpty());
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

    tabs->setCurrentIndex(rootTabIndex);
    QApplication::processEvents();
    auto* networkItem = window.myScene()->findGraphicalModelDataDefinition(network);
    if (networkItem == nullptr) {
        Plugin* networkPlugin = simulator->getPluginManager()->find(Util::TypeOf<EFSMNetwork>());
        ASSERT_NE(networkPlugin, nullptr);
        networkItem = window.myScene()->addGraphicalModelDataDefinition(
            networkPlugin, network, QPointF(520, 220));
    }
    ASSERT_NE(networkItem, nullptr);
    window.myScene()->clearSelection();
    networkItem->setSelected(true);
    QApplication::processEvents();
    EXPECT_EQ(openAction->text(), QStringLiteral("Open Network"));
    ASSERT_TRUE(openAction->isEnabled());
    openAction->trigger();
    QApplication::processEvents();
    EXPECT_EQ(tabs->count(), rootCount + 1);
    EXPECT_EQ(tabs->currentWidget(), networkView);
    EXPECT_EQ(networkView->getScene()->modelLevelFilter(), static_cast<unsigned int>(network->getId()));

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
        if (action->objectName() == QStringLiteral("actionOpenSelectedSubmodel")) {
            openAction = action;
        }
    }
    ASSERT_NE(openAction, nullptr);
    QTabWidget* tabs = window.findChild<QTabWidget*>(QStringLiteral("tabWidgetModelGraphics"));
    ASSERT_NE(tabs, nullptr);
    const int tabCount = tabs->count();
    graphicalAdapter->setSelected(true);
    EXPECT_EQ(openAction->text(), QStringLiteral("Open Network"));
    EXPECT_FALSE(openAction->isEnabled());
    for (QAction* action : window.findChildren<QAction*>()) {
        EXPECT_NE(action->text(), QStringLiteral("Create EFSM Network"));
        EXPECT_NE(action->text(), QStringLiteral("Select EFSM Network"));
    }
    openAction->trigger();
    EXPECT_EQ(tabs->count(), tabCount);
    EXPECT_EQ(adapter->getNetwork(), nullptr);
}

TEST(EFSMNetworkEditor, CreateTransitionActionStartsGraphicalConnectionWithoutDialog) {
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
    scene->setNetworkEditorOwner(network);
    scene->setModelLevelFilter(static_cast<unsigned int>(network->getId()));

    QAction* createState = nullptr;
    QAction* createTransition = nullptr;
    for (QAction* action : window.findChildren<QAction*>()) {
        if (action->text() == QStringLiteral("Create EFSM State")) createState = action;
        if (action->text() == QStringLiteral("Create EFSM Transition")) createTransition = action;
    }
    ASSERT_NE(createState, nullptr);
    ASSERT_NE(createTransition, nullptr);
    createState->setEnabled(true);
    createTransition->setEnabled(true);
    auto enterStateName = [](QAction* action, const QString& value) {
        QTimer::singleShot(0, [value]() {
            auto* prompt = qobject_cast<QInputDialog*>(QApplication::activeModalWidget());
            if (prompt != nullptr) {
                prompt->setTextValue(value);
                prompt->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Ok)->click();
            }
        });
        action->trigger();
    };
    enterStateName(createState, QStringLiteral("Idle"));
    enterStateName(createState, QStringLiteral("Done"));
    ASSERT_EQ(network->getStates()->size(), 2u);
    auto* idle = findStateByName(network, "Idle");
    auto* done = findStateByName(network, "Done");
    ASSERT_NE(idle, nullptr);
    ASSERT_NE(done, nullptr);
    auto* idleItem = scene->findGraphicalModelDataDefinition(idle);
    ASSERT_NE(idleItem, nullptr);
    scene->clearSelection();
    idleItem->setSelected(true);
    ASSERT_TRUE(scene->beginNetworkRelationCreation(idleItem));
    scene->cancelNetworkRelationCreation();
    createTransition->trigger();
    EXPECT_TRUE(scene->isNetworkRelationCreationActive());
    EXPECT_TRUE(network->getTransitions()->empty());
    scene->cancelNetworkRelationCreation();
}

TEST(EFSMNetworkEditor, AdapterBindingsHaveNoLegacyDialogAction) {
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
    for (QAction* action : window.findChildren<QAction*>()) {
        EXPECT_NE(action->text(), QStringLiteral("Edit EFSM Bindings..."));
    }
    EXPECT_EQ(adapter->getInputBinding(0), "1");
    EXPECT_EQ(adapter->getOutputBinding(0), "InitialResult");
    EXPECT_EQ(source->getConnectionManager()->size(), 1u);
    EXPECT_EQ(adapter->getConnectionManager()->size(), 1u);
}

TEST(EFSMNetworkEditor, MainWindowSaveCloseAndReopenRestoresAndExecutesEFSM) {
    MainWindow window;
    ASSERT_TRUE(QMetaObject::invokeMethod(&window, "on_actionModelNew_triggered", Qt::DirectConnection));
    Simulator* simulator = window.myScene()->getSimulator();
    ASSERT_NE(simulator, nullptr);
    simulator->getPluginManager()->autoInsertPlugins();
    Model* originalModel = simulator->getModelManager()->current();
    ASSERT_NE(originalModel, nullptr);

    auto* adapter = new ModalModelDefault(originalModel, "PersistedGuiAdapter");
    auto* network = new EFSMNetwork(originalModel, "PersistedGuiNetwork");
    adapter->setNetwork(network);
    auto* source = new Create(originalModel, "PersistedGuiSource");
    auto* sink = new Dispose(originalModel, "PersistedGuiSink");
    source->getConnectionManager()->insert(adapter);
    adapter->getConnectionManager()->insert(sink);
    auto* rootView = window.findChild<ModelGraphicsView*>();
    ASSERT_NE(rootView, nullptr);
    std::map<std::string, QColor> colors;
    GraphicalModelBuilder rootBuilder(simulator, rootView, window.myScene(), &colors, nullptr);
    rootBuilder.generateGraphicalModelFromModel();

    auto* openNetwork = window.findChild<QAction*>(QStringLiteral("actionOpenSelectedSubmodel"));
    ASSERT_NE(openNetwork, nullptr);
    auto* tabs = window.findChild<QTabWidget*>(QStringLiteral("tabWidgetModelGraphics"));
    ASSERT_NE(tabs, nullptr);
    const int rootTabCount = tabs->count();
    for (QGraphicsItem* item : window.myScene()->items()) {
        auto* graphical = dynamic_cast<GraphicalModelComponent*>(item);
        if (graphical != nullptr && graphical->getComponent() == adapter) {
            graphical->setSelected(true);
            break;
        }
    }
    EXPECT_EQ(openNetwork->text(), QStringLiteral("Open Network"));
    ASSERT_TRUE(openNetwork->isEnabled());
    openNetwork->trigger();
    ASSERT_EQ(tabs->count(), rootTabCount + 1);
    auto* networkView = dynamic_cast<ModelGraphicsView*>(tabs->currentWidget());
    ASSERT_NE(networkView, nullptr);
    ASSERT_EQ(networkView->getScene()->modelLevelFilter(), static_cast<unsigned int>(network->getId()));
    auto enterStateName = [](QAction* action, const QString& name) {
        bool dialogHandled = false;
        QTimer dialogDriver;
        QTimer dialogWatchdog;
        dialogWatchdog.setSingleShot(true);
        QObject::connect(&dialogWatchdog, &QTimer::timeout, []() {
            if (auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget())) dialog->reject();
        });
        QObject::connect(&dialogDriver, &QTimer::timeout, [&dialogHandled, &dialogDriver, name]() {
            auto* dialog = qobject_cast<QInputDialog*>(QApplication::activeModalWidget());
            if (dialog != nullptr) {
                dialogHandled = true;
                dialogDriver.stop();
                dialog->setTextValue(name);
                dialog->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Ok)->click();
            }
        });
        dialogDriver.start(5);
        dialogWatchdog.start(5000);
        action->trigger();
        dialogDriver.stop();
        dialogWatchdog.stop();
        EXPECT_TRUE(dialogHandled);
    };
    QAction* createState = nullptr;
    QAction* createTransition = nullptr;
    QAction* setInitial = nullptr;
    for (QAction* action : window.findChildren<QAction*>()) {
        if (action->text() == QStringLiteral("Create EFSM State")) createState = action;
        if (action->text() == QStringLiteral("Create EFSM Transition")) createTransition = action;
        if (action->text() == QStringLiteral("Set Selected EFSM State as Initial")) setInitial = action;
    }
    ASSERT_NE(createState, nullptr);
    ASSERT_NE(createTransition, nullptr);
    ASSERT_NE(setInitial, nullptr);
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
    auto* createCommand = GuiExtensionPluginCatalog::createNetworkRelationCommand(network, idleItem, doneItem);
    ASSERT_NE(createCommand, nullptr);
    ASSERT_NE(networkView->getScene()->getUndoStack(), nullptr);
    networkView->getScene()->getUndoStack()->push(createCommand);
    ASSERT_EQ(network->getTransitions()->size(), 1u);
    auto* transition = network->getTransitions()->front();
    EXPECT_EQ(transition->getSource(), idle);
    EXPECT_EQ(transition->getDestination(), done);
    auto* guardCommand = GuiExtensionPluginCatalog::createNetworkElementPropertyCommand(
        network, networkView->getScene()->extensionOverlayItems(QStringLiteral("efsm")).front(),
        "Guard Expression", "1");
    ASSERT_NE(guardCommand, nullptr);
    networkView->getScene()->getUndoStack()->push(guardCommand);
    auto* outputCommand = GuiExtensionPluginCatalog::createNetworkElementPropertyCommand(
        network, networkView->getScene()->extensionOverlayItems(QStringLiteral("efsm")).front(),
        "Output Expression", "91");
    ASSERT_NE(outputCommand, nullptr);
    networkView->getScene()->getUndoStack()->push(outputCommand);
    auto* priorityCommand = GuiExtensionPluginCatalog::createNetworkElementPropertyCommand(
        network, networkView->getScene()->extensionOverlayItems(QStringLiteral("efsm")).front(),
        "Priority", "7");
    ASSERT_NE(priorityCommand, nullptr);
    networkView->getScene()->getUndoStack()->push(priorityCommand);
    EXPECT_EQ(transition->getGuardExpression(), "1");
    EXPECT_EQ(transition->getOutputExpression(), "91");
    EXPECT_EQ(transition->getPriority(), 7u);
    networkView->getScene()->getUndoStack()->undo();
    networkView->getScene()->getUndoStack()->undo();
    networkView->getScene()->getUndoStack()->undo();
    EXPECT_TRUE(transition->getGuardExpression().empty());
    networkView->getScene()->getUndoStack()->redo();
    networkView->getScene()->getUndoStack()->redo();
    networkView->getScene()->getUndoStack()->redo();
    EXPECT_EQ(transition->getGuardExpression(), "1");

    QAction* deleteSelection = nullptr;
    for (QAction* action : window.findChildren<QAction*>()) {
        if (action->objectName() == QStringLiteral("actionEditDelete")) {
            deleteSelection = action;
            break;
        }
    }
    ASSERT_NE(deleteSelection, nullptr);
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
    EXPECT_EQ(transition->getGuardExpression(), "1");
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
    EXPECT_EQ(restoredNetwork->getTransitions()->front()->getName(), "T0");
    EXPECT_EQ(restoredNetwork->getTransitions()->front()->getGuardExpression(), "1");
    EXPECT_EQ(restoredNetwork->getTransitions()->front()->getOutputExpression(), "91");
    EXPECT_EQ(restoredNetwork->getTransitions()->front()->getPriority(), 7u);
    EXPECT_EQ(restoredModel->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<EFSMNetwork>()), 1u);
    const auto parsedLayouts = EFSMNetworkDiagramCodec::parseSection(savedGuiText.split(QLatin1Char('\n')), restoredModel);
    ASSERT_TRUE(parsedLayouts.contains(static_cast<unsigned long>(restoredNetwork->getId())));
    ASSERT_EQ(parsedLayouts.value(static_cast<unsigned long>(restoredNetwork->getId())).states.size(), 2);

    GraphicalModelComponent* graphicalAdapter = nullptr;
    for (QGraphicsItem* item : window.myScene()->items()) {
        auto* graphical = dynamic_cast<GraphicalModelComponent*>(item);
        if (graphical != nullptr && graphical->getComponent()->getName() == "PersistedGuiAdapter") {
            graphicalAdapter = graphical;
            break;
        }
    }
    ASSERT_NE(graphicalAdapter, nullptr);
    graphicalAdapter->setSelected(true);
    EXPECT_EQ(openNetwork->text(), QStringLiteral("Open Network"));
    ASSERT_TRUE(openNetwork->isEnabled());
    openNetwork->trigger();
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

    // This Network was built and edited through the GUI, persisted, and loaded
    // above. Exercise the restored mathematical Network through the kernel API
    // to verify that the saved guard and output retain executable semantics.
    const NetworkActivationResult activation = restoredNetwork->activate(
        NetworkActivationFrame(restoredNetwork->getNumInputPorts()));
    ASSERT_TRUE(activation.isPresent(0));
    EXPECT_DOUBLE_EQ(activation.getValue(0), 91.0);
    ASSERT_NE(restoredNetwork->getCurrentState(), nullptr);
    EXPECT_EQ(restoredNetwork->getCurrentState()->getName(), "PersistedDone");
}

TEST(TransferGuiPersistence, NewGotoLabelModelSurvivesGraphicalRoundTrip) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    model->getSimulation()->setReplicationLength(30.0, Util::TimeUnit::second);
    model->getSimulation()->setReplicationReportBaseTimeUnit(Util::TimeUnit::second);
    model->getSimulation()->setNumberOfReplications(1);

    auto* create = new Create(model, "CreateTransferGui");
    auto* sender = new Goto(model, "GotoTransferGui");
    auto* receiver = new Label(model, "LabelTransferGui");
    auto* sink = new Dispose(model, "DisposeTransferGui");
    auto* type = new EntityType(model, "TransferGuiType");
    create->setEntityType(type);
    create->setFirstCreation(0.0);
    create->setTimeBetweenCreationsExpression("1.0");
    create->setMaxCreations(2);
    create->connectTo(sender);
    receiver->connectTo(sink);
    sender->setLabelDefinition(receiver->getLabelDefinition());
    sender->setTimeDelayExpression("3.0");
    sender->setTimeDelayTimeUnit(Util::TimeUnit::second);
    ASSERT_TRUE(model->check());

    SerializerHarness harness(&simulator);
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString filename = dir.filePath("transfer.gui");
    ASSERT_TRUE(harness.serializer->saveGraphicalModel(filename));
    Model* restored = harness.serializer->loadGraphicalModel(filename.toStdString());
    ASSERT_NE(restored, nullptr);

    auto* loadedGoto = dynamic_cast<Goto*>(restored->getComponentManager()->find("GotoTransferGui"));
    auto* loadedLabel = dynamic_cast<Label*>(restored->getComponentManager()->find("LabelTransferGui"));
    ASSERT_NE(loadedGoto, nullptr);
    ASSERT_NE(loadedLabel, nullptr);
    ASSERT_NE(loadedLabel->getLabelDefinition(), nullptr);
    EXPECT_EQ(loadedGoto->getLabelDefinition(), loadedLabel->getLabelDefinition());
    EXPECT_EQ(loadedLabel->getLabelDefinition()->getEnterIntoLabelComponent(), loadedLabel);
    EXPECT_EQ(loadedGoto->getTimeDelayExpression(), "3.0");
    EXPECT_EQ(loadedGoto->getTimeDelayTimeUnit(), Util::TimeUnit::second);
    EXPECT_EQ(loadedGoto->getConnectionManager()->size(), 0u);
    EXPECT_EQ(loadedLabel->getConnectionManager()->size(), 1u);
    ASSERT_TRUE(restored->check());
    restored->getSimulation()->start();
    EXPECT_TRUE(restored->getTracer()->errorMessages()->empty());
    EXPECT_EQ(restored->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<Entity>()), 0u);
}
