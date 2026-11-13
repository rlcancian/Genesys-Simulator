#include "graphicals/GraphicalModelComponent.h"
#include "graphicals/GraphicalConnection.h"
#include "graphicals/GraphicalModelDataDefinition.h"
#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/ModelGraphicsView.h"
#include "controllers/PropertyEditorController.h"
#include "controllers/PluginCatalogController.h"
#include "propertyeditor/ObjectPropertyBrowser.h"
#include "codeeditor/CodeEditor.h"
#include "services/GraphicalModelBuilder.h"
#include "services/GraphicalModelSerializer.h"
#include "cellular_automata/CellularAutomataViewerWindow.h"

#include "../../kernel/simulator/essentialPlugins/Counter.h"
#include "../../kernel/simulator/persistence/GenSerializer.h"
#include "../../kernel/simulator/model/Model.h"
#include "../../kernel/simulator/model/ModelDataDefinition.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Plugin.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/SimulationControlAndResponse.h"
#include "../../kernel/simulator/essentialPlugins/StatisticsCollector.h"
#include "../../plugins/components/DiscreteProcessing/auxiliar/QueueableItem.h"
#include "../../plugins/components/DiscreteProcessing/auxiliar/SeizableItem.h"
#include "plugins/components/DiscreteProcessing/Release.h"
#include "plugins/components/DiscreteProcessing/Seize.h"
#include "plugins/components/Decisions/Decide.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/Synchronization/Signal.h"
#include "plugins/components/Synchronization/Wait.h"
#include "applications/gui/genesys/actions/DecideConditionUndoCommand.h"
#include "plugins/components/ExternalIntegration/CppForG.h"
#include "plugins/data/DiscreteProcessing/Queue.h"
#include "plugins/data/DiscreteProcessing/Resource.h"
#include "plugins/data/Synchronization/SignalData.h"
#include "plugins/data/ModalModel/GraphNode.h"

#include <gtest/gtest.h>

#include <QApplication>
#include <QAction>
#include <QByteArray>
#include <QColor>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QGraphicsItemGroup>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QList>
#include <QPlainTextEdit>
#include <QRectF>
#include <QScrollBar>
#include <QSlider>
#include <QTemporaryDir>
#include <QTextEdit>
#include <QTimer>
#include <QTreeWidget>
#include <QUndoStack>
#include <QPushButton>
#include <QKeyEvent>

#include <algorithm>
#include <functional>
#include <map>

TEST(PluginCatalogPresentation, MainPaletteContainsOnlyComponentsAndReloadIsStable) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    const unsigned int registeredBefore = simulator.getPluginManager()->size();
    QTreeWidget tree;
    std::map<std::string, QColor> categoryColors;
    PluginCatalogController catalog(&simulator, &tree, nullptr, &categoryColors);
    catalog.reloadFromPluginManager();
    auto countLeafItems = [&tree, &simulator]() {
        unsigned int count = 0;
        std::function<void(QTreeWidgetItem*)> visit = [&](QTreeWidgetItem* item) {
            if (item->childCount() == 0) {
                ++count;
                Plugin* plugin = simulator.getPluginManager()->find(item->text(0).toStdString());
                EXPECT_NE(plugin, nullptr);
                if (plugin != nullptr) EXPECT_TRUE(plugin->getPluginInfo()->isComponent());
            } else {
                for (int i = 0; i < item->childCount(); ++i) visit(item->child(i));
            }
        };
        for (int i = 0; i < tree.topLevelItemCount(); ++i) visit(tree.topLevelItem(i));
        return count;
    };
    const unsigned int firstCount = countLeafItems();
    EXPECT_GT(firstCount, 0u);
    EXPECT_EQ(simulator.getPluginManager()->size(), registeredBefore);
    catalog.reloadFromPluginManager();
    EXPECT_EQ(countLeafItems(), firstCount);
    EXPECT_NE(simulator.getPluginManager()->find(Util::TypeOf<Resource>()), nullptr);
}

TEST(DecideDynamicPortPresentation, OutputPortCountTracksConditionsPlusElse) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    Plugin* plugin = simulator.getPluginManager()->find(Util::TypeOf<Decide>());
    ASSERT_NE(plugin, nullptr);
    auto* decide = new Decide(model, "DynamicDecide");
    auto countPorts = [&](Decide* value) {
        GraphicalModelComponent graphical(plugin, value, QPointF(100, 100), QColor("#336699"));
        return graphical.getGraphicalOutputPorts().size();
    };
    EXPECT_EQ(countPorts(decide), 1);
    decide->addConditions("A > 0");
    EXPECT_EQ(countPorts(decide), 2);
    GraphicalModelComponent graphical(plugin, decide, QPointF(100, 100), QColor("#336699"));
    graphical.applyDecideConditionPortEdit(1, true);
    EXPECT_EQ(graphical.getGraphicalOutputPorts().size(), 3);
    graphical.applyDecideConditionPortEdit(1, false);
    EXPECT_EQ(graphical.getGraphicalOutputPorts().size(), 2);
    decide->addConditions("B > 0");
    decide->addConditions("C > 0");
    EXPECT_EQ(countPorts(decide), 4);
}

TEST(DecideDynamicPortPresentation, AppliesQueuedConditionPortEditsInOrder) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    Plugin* plugin = simulator.getPluginManager()->find(Util::TypeOf<Decide>());
    ASSERT_NE(plugin, nullptr);
    auto* decide = new Decide(model, "QueuedDynamicDecide");
    GraphicalModelComponent graphical(plugin, decide, QPointF(100, 100), Qt::blue);

    // Model the queued Property Editor callback: several list edits may have
    // happened before the event loop delivers its refresh.
    decide->editConditionInsertAt(0, "first");
    decide->editConditionInsertAt(1, "second");
    decide->editConditionInsertAt(1, "middle");
    decide->editConditionRemoveAt(1);

    unsigned int index = 0;
    bool inserted = false;
    while (decide->takeConditionPortEdit(index, inserted)) {
        graphical.applyDecideConditionPortEdit(index, inserted);
    }

    ASSERT_EQ(decide->getConditions()->size(), 2u);
    EXPECT_EQ(decide->getConditions()->getAtRank(0), "first");
    EXPECT_EQ(decide->getConditions()->getAtRank(1), "second");
    EXPECT_EQ(graphical.getGraphicalOutputPorts().size(), 3); // two conditions + Else
    for (int port = 0; port < graphical.getGraphicalOutputPorts().size(); ++port) {
        EXPECT_EQ(graphical.getGraphicalOutputPorts().at(port)->portNum(), static_cast<unsigned int>(port));
    }
}

TEST(DecideConditionUndo, MoveKeepsGraphicalConnectionsAndElseThroughUndoRedo) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    auto* decide = new Decide(model, "MoveGraphicalDecide");
    decide->addConditions("first");
    decide->addConditions("second");
    decide->addConditions("third");
    auto* target = new Dispose(model, "MoveGraphicalTarget");
    auto* elseTarget = new Dispose(model, "MoveGraphicalElse");
    Plugin* decidePlugin = simulator.getPluginManager()->find(Util::TypeOf<Decide>());
    Plugin* disposePlugin = simulator.getPluginManager()->find(Util::TypeOf<Dispose>());
    ASSERT_NE(decidePlugin, nullptr);
    ASSERT_NE(disposePlugin, nullptr);

    ModelGraphicsScene scene(0, 0, 1000, 800);
    QUndoStack undoStack;
    scene.setUndoStack(&undoStack);
    auto* graphical = new GraphicalModelComponent(decidePlugin, decide, QPointF(100, 100), Qt::blue);
    auto* graphicalTarget = new GraphicalModelComponent(disposePlugin, target, QPointF(400, 100), Qt::blue);
    auto* graphicalElse = new GraphicalModelComponent(disposePlugin, elseTarget, QPointF(400, 300), Qt::blue);
    scene.addItem(graphical);
    scene.addItem(graphicalTarget);
    scene.addItem(graphicalElse);
    auto* firstConnection = new Connection{target, {0}};
    auto* elseConnection = new Connection{elseTarget, {0}};
    decide->getConnectionManager()->insertAtPort(0, firstConnection);
    decide->getConnectionManager()->insertAtPort(3, elseConnection);
    auto* firstPort = graphical->getGraphicalOutputPorts().at(0);
    auto* elsePort = graphical->getGraphicalOutputPorts().at(3);
    const qreal firstPosition = firstPort->y();
    const qreal elsePosition = elsePort->y();
    scene.addGraphicalConnection(firstPort, graphicalTarget->getGraphicalInputPorts().at(0), 0, 0);
    scene.addGraphicalConnection(elsePort, graphicalElse->getGraphicalInputPorts().at(0), 0, 0);
    ASSERT_EQ(firstPort->getConnections()->size(), 1);
    auto* graphicalConnection = firstPort->getConnections()->front();

    decide->moveCondition(0, 2);
    graphical->moveDecideConditionPort(0, 2);
    Decide::ConditionEdit edit{Decide::ConditionEdit::Kind::Move, 0};
    edit.destinationIndex = 2;
    undoStack.push(new DecideConditionUndoCommand(decide, graphical, edit));
    EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(2), firstConnection);
    EXPECT_EQ(graphical->getGraphicalOutputPorts().at(2), firstPort);
    EXPECT_EQ(firstPort->portNum(), 2u);
    EXPECT_EQ(graphicalConnection->getPortSourceConnection(), 2u);
    EXPECT_GT(firstPort->y(), firstPosition);
    EXPECT_EQ(graphicalConnection->getSourceGraphicalPort(), firstPort);
    EXPECT_EQ(graphical->getGraphicalOutputPorts().at(3), elsePort);
    EXPECT_EQ(elsePort->y(), elsePosition);
    EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(3), elseConnection);
    undoStack.undo();
    EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(0), firstConnection);
    EXPECT_EQ(graphical->getGraphicalOutputPorts().at(0), firstPort);
    EXPECT_EQ(firstPort->portNum(), 0u);
    EXPECT_EQ(firstPort->y(), firstPosition);
    EXPECT_EQ(graphicalConnection->getPortSourceConnection(), 0u);
    undoStack.redo();
    EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(2), firstConnection);
    EXPECT_EQ(graphical->getGraphicalOutputPorts().at(2), firstPort);
    EXPECT_EQ(graphical->getGraphicalOutputPorts().at(3), elsePort);
}

TEST(DecideConditionUndo, RemoveUndoRedoRestoresOnlyTheRemovedConnectionAndElse) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    auto* decide = new Decide(model, "UndoDecide");
    decide->addConditions("first");
    decide->addConditions("second");
    auto* removedTarget = new Dispose(model, "RemovedTarget");
    auto* elseTarget = new Dispose(model, "ElseTarget");
    Plugin* decidePlugin = simulator.getPluginManager()->find(Util::TypeOf<Decide>());
    Plugin* disposePlugin = simulator.getPluginManager()->find(Util::TypeOf<Dispose>());
    ASSERT_NE(decidePlugin, nullptr);
    ASSERT_NE(disposePlugin, nullptr);

    ModelGraphicsScene scene(0, 0, 1000, 800);
    QUndoStack undoStack;
    scene.setUndoStack(&undoStack);
    auto* graphicalDecide = new GraphicalModelComponent(decidePlugin, decide, QPointF(100, 100), Qt::blue);
    auto* graphicalRemovedTarget = new GraphicalModelComponent(disposePlugin, removedTarget, QPointF(400, 100), Qt::blue);
    auto* graphicalElseTarget = new GraphicalModelComponent(disposePlugin, elseTarget, QPointF(400, 300), Qt::blue);
    scene.addItem(graphicalDecide);
    scene.addItem(graphicalRemovedTarget);
    scene.addItem(graphicalElseTarget);
    scene.getGraphicalModelComponents()->append(graphicalDecide);
    scene.getGraphicalModelComponents()->append(graphicalRemovedTarget);
    scene.getGraphicalModelComponents()->append(graphicalElseTarget);

    auto* removedConnection = new Connection{removedTarget, {0}};
    auto* elseConnection = new Connection{elseTarget, {0}};
    decide->getConnectionManager()->insertAtPort(0, removedConnection);
    decide->getConnectionManager()->insertAtPort(2, elseConnection);
    scene.addGraphicalConnection(graphicalDecide->getGraphicalOutputPorts().at(0),
        graphicalRemovedTarget->getGraphicalInputPorts().at(0), 0, 0);
    scene.addGraphicalConnection(graphicalDecide->getGraphicalOutputPorts().at(2),
        graphicalElseTarget->getGraphicalInputPorts().at(0), 0, 0);

    decide->editConditionRemoveAt(0);
    unsigned int changedIndex = 0;
    bool inserted = true;
    ASSERT_TRUE(decide->takeConditionPortEdit(changedIndex, inserted));
    EXPECT_EQ(changedIndex, 0u);
    EXPECT_FALSE(inserted);
    graphicalDecide->applyDecideConditionPortEdit(changedIndex, inserted);
    Decide::ConditionEdit edit;
    ASSERT_TRUE(decide->takeConditionEdit(edit));
    undoStack.push(new DecideConditionUndoCommand(decide, graphicalDecide, edit));
    ASSERT_EQ(decide->getConditions()->size(), 1u);
    ASSERT_EQ(graphicalDecide->getGraphicalOutputPorts().size(), 2);
    EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(1), elseConnection);
    EXPECT_EQ(scene.getGraphicalConnections()->size(), 1);

    undoStack.undo();
    ASSERT_EQ(decide->getConditions()->size(), 2u);
    ASSERT_EQ(graphicalDecide->getGraphicalOutputPorts().size(), 3);
    ASSERT_NE(decide->getConnectionManager()->getConnectionAtPort(0), nullptr);
    EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(0)->component, removedTarget);
    EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(2), elseConnection);
    ASSERT_EQ(scene.getGraphicalConnections()->size(), 2);
    EXPECT_EQ(dynamic_cast<GraphicalConnection*>(scene.getGraphicalConnections()->at(0))->getDestinationGraphicalComponent(), graphicalElseTarget);
    EXPECT_EQ(dynamic_cast<GraphicalConnection*>(scene.getGraphicalConnections()->at(1))->getDestinationGraphicalComponent(), graphicalRemovedTarget);

    undoStack.redo();
    ASSERT_EQ(decide->getConditions()->size(), 1u);
    ASSERT_EQ(graphicalDecide->getGraphicalOutputPorts().size(), 2);
    EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(1), elseConnection);
    EXPECT_EQ(decide->getConnectionManager()->getConnectionAtPort(0), nullptr);
    ASSERT_EQ(scene.getGraphicalConnections()->size(), 1);
    EXPECT_EQ(dynamic_cast<GraphicalConnection*>(scene.getGraphicalConnections()->front())->getDestinationGraphicalComponent(), graphicalElseTarget);
}

TEST(DecideConditionUndo, InsertAndReplaceUndoRedoRestoreOrderedConditionTextAndPorts) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    auto* decide = new Decide(model, "UndoInsertDecide");
    Plugin* plugin = simulator.getPluginManager()->find(Util::TypeOf<Decide>());
    ASSERT_NE(plugin, nullptr);
    ModelGraphicsScene scene(0, 0, 800, 600);
    QUndoStack undoStack;
    scene.setUndoStack(&undoStack);
    auto* graphical = new GraphicalModelComponent(plugin, decide, QPointF(100, 100), Qt::blue);
    scene.addItem(graphical);
    scene.getGraphicalModelComponents()->append(graphical);

    decide->editConditionInsertAt(0, "x > 0");
    unsigned int changedIndex = 0;
    bool inserted = false;
    ASSERT_TRUE(decide->takeConditionPortEdit(changedIndex, inserted));
    graphical->applyDecideConditionPortEdit(changedIndex, inserted);
    Decide::ConditionEdit insertEdit;
    ASSERT_TRUE(decide->takeConditionEdit(insertEdit));
    undoStack.push(new DecideConditionUndoCommand(decide, graphical, insertEdit));
    ASSERT_EQ(graphical->getGraphicalOutputPorts().size(), 2);
    undoStack.undo();
    EXPECT_TRUE(decide->getConditions()->empty());
    EXPECT_EQ(graphical->getGraphicalOutputPorts().size(), 1);
    undoStack.redo();
    ASSERT_EQ(decide->getConditions()->size(), 1u);
    EXPECT_EQ(decide->getConditions()->getAtRank(0), "x > 0");
    EXPECT_EQ(graphical->getGraphicalOutputPorts().size(), 2);

    decide->editConditionReplaceAt(0, "x >= 0");
    Decide::ConditionEdit replaceEdit;
    ASSERT_TRUE(decide->takeConditionEdit(replaceEdit));
    undoStack.push(new DecideConditionUndoCommand(decide, graphical, replaceEdit));
    EXPECT_EQ(decide->getConditions()->getAtRank(0), "x >= 0");
    undoStack.undo();
    EXPECT_EQ(decide->getConditions()->getAtRank(0), "x > 0");
    undoStack.redo();
    EXPECT_EQ(decide->getConditions()->getAtRank(0), "x >= 0");
}

namespace {
QtProperty* findPropertyByName(QtProperty* property, const QString& name) {
    if (property == nullptr) return nullptr;
    if (property->propertyName() == name) return property;
    for (QtProperty* child : property->subProperties()) {
        if (QtProperty* found = findPropertyByName(child, name)) return found;
    }
    return nullptr;
}

void openCodeEditor(ObjectPropertyBrowser* browser, const QString& propertyName,
                    const std::function<void(QDialog*)>& interactWithDialog) {
    QtProperty* property = nullptr;
    for (QtProperty* root : browser->properties()) {
        if ((property = findPropertyByName(root, propertyName)) != nullptr) break;
    }
    ASSERT_NE(property, nullptr);
    const QList<QtBrowserItem*> items = browser->items(property);
    ASSERT_FALSE(items.isEmpty());
    browser->setCurrentItem(items.front());
    QTimer::singleShot(0, browser, [interactWithDialog]() {
        auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        ASSERT_NE(dialog, nullptr);
        interactWithDialog(dialog);
    });
    QKeyEvent enter(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    QApplication::sendEvent(browser, &enter);
}
}

TEST(SourceCodePropertyEditor, CancelLeavesTheOriginalCodeUntouched) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    auto* cpp = new CppForG(model, "CancelableCpp");
    cpp->setOnDispatchEventCode("original\nline");
    ObjectPropertyBrowser browser;
    browser.setActiveObject(nullptr, cpp);
    QApplication::processEvents();

    openCodeEditor(&browser, "OnDispatchEventCode", [](QDialog* dialog) {
        auto* editor = dialog->findChild<CodeEditor*>();
        auto* buttons = dialog->findChild<QDialogButtonBox*>();
        ASSERT_NE(editor, nullptr);
        ASSERT_NE(buttons, nullptr);
        editor->setPlainText("discard this");
        buttons->button(QDialogButtonBox::Cancel)->click();
    });
    EXPECT_EQ(cpp->getOnDispatchEventCode(), "original\nline");
}

TEST(GraphNodePropertyEditor, ExposesTheInheritedNodeNameProperty) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    GraphNode node(model, "EditableNode");

    ObjectPropertyBrowser browser;
    browser.setActiveObject(nullptr, &node);
    QApplication::processEvents();

    QtProperty* name = nullptr;
    for (QtProperty* root : browser.properties()) {
        if ((name = findPropertyByName(root, QStringLiteral("Name"))) != nullptr) break;
    }
    ASSERT_NE(name, nullptr);
    EXPECT_FALSE(browser.items(name).isEmpty());
}

TEST(SourceCodePropertyEditor, ApplyCommitsCodeWithoutClosingTheEditor) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    auto* cpp = new CppForG(model, "ApplicableCpp");
    ObjectPropertyBrowser browser;
    browser.setActiveObject(nullptr, cpp);
    QApplication::processEvents();

    openCodeEditor(&browser, "OnDispatchEventCode", [](QDialog* dialog) {
        auto* editor = dialog->findChild<CodeEditor*>();
        auto* apply = dialog->findChild<QPushButton*>(QString(), Qt::FindDirectChildrenOnly);
        if (apply == nullptr) {
            for (QPushButton* button : dialog->findChildren<QPushButton*>()) {
                if (button->text() == "Apply") { apply = button; break; }
            }
        }
        auto* buttons = dialog->findChild<QDialogButtonBox*>();
        ASSERT_NE(editor, nullptr);
        ASSERT_NE(apply, nullptr);
        ASSERT_NE(buttons, nullptr);
        editor->setPlainText("applied\nλ");
        apply->click();
        EXPECT_EQ(dialog->result(), 0);
        buttons->button(QDialogButtonBox::Cancel)->click();
    });
    EXPECT_EQ(cpp->getOnDispatchEventCode(), "applied\nλ");
}

namespace {

class QueueWithPublicAttachments : public Queue {
public:
    explicit QueueWithPublicAttachments(Model* model, const std::string& name)
        : Queue(model, name) {}

    void attachDataDefinition(const std::string& key, ModelDataDefinition* dataDefinition) {
        _attachedDataInsert(key, dataDefinition);
    }

    void insertInternalDataDefinition(const std::string& key, ModelDataDefinition* dataDefinition) {
        _internalDataInsert(key, dataDefinition);
    }
};

GraphicalModelDataDefinition* findGraphicalDataDefinition(ModelGraphicsScene& scene,
                                                          ModelDataDefinition* dataDefinition) {
    if (scene.getAllDataDefinitions() == nullptr) {
        return nullptr;
    }
    for (GraphicalModelDataDefinition* graphicalDefinition : *scene.getAllDataDefinitions()) {
        if (graphicalDefinition != nullptr && graphicalDefinition->getDataDefinition() == dataDefinition) {
            return graphicalDefinition;
        }
    }
    return nullptr;
}

GraphicalDiagramConnection* findDiagramConnection(ModelGraphicsScene& scene,
                                                  GraphicalModelDataDefinition* dataDefinition,
                                                  QGraphicsItem* linkedTo) {
    if (scene.getGraphicalDiagramsConnections() == nullptr) {
        return nullptr;
    }
    for (QGraphicsItem* item : *scene.getGraphicalDiagramsConnections()) {
        auto* connection = dynamic_cast<GraphicalDiagramConnection*>(item);
        if (connection != nullptr
            && connection->getDataDefinition() == dataDefinition
            && connection->getLinkedDataDefinition() == linkedTo) {
            return connection;
        }
    }
    return nullptr;
}

GraphicalModelDataDefinition* findGraphicalDataDefinitionByTypeAndName(ModelGraphicsScene& scene,
                                                                       const std::string& className,
                                                                       const std::string& name) {
    if (scene.getAllDataDefinitions() == nullptr) {
        return nullptr;
    }
    for (GraphicalModelDataDefinition* graphicalDefinition : *scene.getAllDataDefinitions()) {
        if (graphicalDefinition == nullptr || graphicalDefinition->getDataDefinition() == nullptr) {
            continue;
        }
        if (graphicalDefinition->getDataDefinition()->getClassname() == className
            && graphicalDefinition->getDataDefinition()->getName() == name) {
            return graphicalDefinition;
        }
    }
    return nullptr;
}

GraphicalModelComponent* findGraphicalComponentByName(ModelGraphicsScene& scene, const std::string& name) {
    if (scene.getAllComponents() == nullptr) {
        return nullptr;
    }
    for (GraphicalModelComponent* component : *scene.getAllComponents()) {
        if (component != nullptr
            && component->getComponent() != nullptr
            && component->getComponent()->getName() == name) {
            return component;
        }
    }
    return nullptr;
}

SimulationControl* findSimulationControlByName(List<SimulationControl*>* controls, const std::string& name) {
    if (controls == nullptr) {
        return nullptr;
    }

    for (SimulationControl* control : *controls->list()) {
        if (control != nullptr && control->getName() == name) {
            return control;
        }
    }
    return nullptr;
}

QList<QRectF> sceneRects(std::initializer_list<GraphicalModelDataDefinition*> definitions) {
    QList<QRectF> rects;
    for (GraphicalModelDataDefinition* definition : definitions) {
        if (definition != nullptr) {
            rects.append(definition->sceneBoundingRect());
        }
    }
    return rects;
}

void expectNoSceneRectOverlap(const QList<QRectF>& rects) {
    for (int left = 0; left < rects.size(); ++left) {
        for (int right = left + 1; right < rects.size(); ++right) {
            EXPECT_FALSE(rects.at(left).intersects(rects.at(right)))
                << "rect " << left << " intersects rect " << right;
        }
    }
}

QLineF expectedDiagramConnectionLine(const QRectF& startRect, const QRectF& endRect) {
    QPointF startPoint;
    QPointF endPoint;

    if (startRect.bottom() < endRect.top()) {
        startPoint = QPointF(startRect.center().x(), startRect.bottom() - 10.0);
        endPoint = QPointF(endRect.center().x(), endRect.top() + 10.0);
    } else if (startRect.top() > endRect.bottom()) {
        startPoint = QPointF(startRect.center().x(), startRect.top() + 10.0);
        endPoint = QPointF(endRect.center().x(), endRect.bottom() - 10.0);
    } else if (startRect.right() < endRect.left()) {
        startPoint = QPointF(startRect.right() - 10.0, startRect.center().y());
        endPoint = QPointF(endRect.left() + 10.0, endRect.center().y());
    } else {
        startPoint = QPointF(startRect.left() + 10.0, startRect.center().y());
        endPoint = QPointF(endRect.right() - 10.0, endRect.center().y());
    }

    return QLineF(startPoint, endPoint);
}

} // namespace

TEST(GuiCellularAutomata, ViewerCreatesBoundaryConditionCombo) {
    CellularAutomataViewerWindow window;
    auto* boundaryCombo = window.findChild<QComboBox*>(QStringLiteral("cellularAutomataBoundaryCombo"));
    ASSERT_NE(boundaryCombo, nullptr);
    EXPECT_GE(boundaryCombo->count(), 2);
    EXPECT_GE(boundaryCombo->findText(QStringLiteral("Fixed")), 0);
}

TEST(GuiGmddLayout, SeizeEditableReferencesStayAboveAndLowerDefinitionsUseTwoRows) {
    Simulator simulator;
    PluginManager* pluginManager = simulator.getPluginManager();
    ASSERT_NE(pluginManager, nullptr);
    pluginManager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Queue* queue = new Queue(model, "Queue_1");
    Resource* resource = new Resource(model, "Resource_1");
    Queue* sharedQueueA = new Queue(model, "SharedQueue_A");
    Queue* sharedQueueB = new Queue(model, "SharedQueue_B");
    Seize* seize = new Seize(model, "Seize_1");
    ASSERT_NE(queue, nullptr);
    ASSERT_NE(resource, nullptr);
    ASSERT_NE(sharedQueueA, nullptr);
    ASSERT_NE(sharedQueueB, nullptr);
    ASSERT_NE(seize, nullptr);

    seize->setQueueableItem(new QueueableItem(queue));
    seize->addRequest(new SeizableItem(resource));
    ModelDataDefinition::CreateInternalData(seize);

    Counter* directCounter = new Counter(model, "Seize_1.DirectCounter", seize);
    StatisticsCollector* directStatistic = new StatisticsCollector(model, "Seize_1.DirectStatistic", seize);
    ASSERT_NE(directCounter, nullptr);
    ASSERT_NE(directStatistic, nullptr);
    (*seize->getInternalData())["DirectCounter"] = directCounter;
    (*seize->getInternalData())["DirectStatistic"] = directStatistic;
    (*seize->getAttachedData())["SharedQueue_A"] = sharedQueueA;
    (*seize->getAttachedData())["SharedQueue_B"] = sharedQueueB;

    ModelGraphicsScene scene(0, 0, 2000, 2000);
    scene.setSimulator(&simulator);
    scene.setShowEditableDataDefinitions(true);
    scene.setShowStatisticsDataDefinitions(true);
    scene.setShowSharedDataDefinitions(true);
    scene.setShowRecursiveDataDefinitions(false);

    Plugin* seizePlugin = pluginManager->find(Util::TypeOf<Seize>());
    ASSERT_NE(seizePlugin, nullptr);
    auto* graphicalSeize = new GraphicalModelComponent(
        seizePlugin,
        seize,
        QPointF(600.0, 600.0),
        QColor(105, 105, 105));
    scene.addItem(graphicalSeize);

    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, &scene);

    GraphicalModelDataDefinition* queueGmdd = findGraphicalDataDefinition(scene, queue);
    GraphicalModelDataDefinition* resourceGmdd = findGraphicalDataDefinition(scene, resource);
    GraphicalModelDataDefinition* counterGmdd = findGraphicalDataDefinition(scene, directCounter);
    GraphicalModelDataDefinition* statisticGmdd = findGraphicalDataDefinition(scene, directStatistic);
    GraphicalModelDataDefinition* sharedQueueAGmdd = findGraphicalDataDefinition(scene, sharedQueueA);
    GraphicalModelDataDefinition* sharedQueueBGmdd = findGraphicalDataDefinition(scene, sharedQueueB);
    ASSERT_NE(queueGmdd, nullptr);
    ASSERT_NE(resourceGmdd, nullptr);
    ASSERT_NE(counterGmdd, nullptr);
    ASSERT_NE(statisticGmdd, nullptr);
    ASSERT_NE(sharedQueueAGmdd, nullptr);
    ASSERT_NE(sharedQueueBGmdd, nullptr);

    const QRectF componentRect = graphicalSeize->sceneBoundingRect();
    EXPECT_TRUE(queueGmdd->isEditableInPropertyEditor());
    EXPECT_TRUE(resourceGmdd->isEditableInPropertyEditor());
    EXPECT_LT(queueGmdd->sceneBoundingRect().bottom(), componentRect.top());
    EXPECT_LT(resourceGmdd->sceneBoundingRect().bottom(), componentRect.top());

    EXPECT_FALSE(counterGmdd->isEditableInPropertyEditor());
    EXPECT_FALSE(statisticGmdd->isEditableInPropertyEditor());
    EXPECT_GT(counterGmdd->sceneBoundingRect().top(), componentRect.bottom());
    EXPECT_GT(statisticGmdd->sceneBoundingRect().top(), componentRect.bottom());

    EXPECT_FALSE(sharedQueueAGmdd->isEditableInPropertyEditor());
    EXPECT_FALSE(sharedQueueBGmdd->isEditableInPropertyEditor());
    EXPECT_GT(sharedQueueAGmdd->sceneBoundingRect().top(), componentRect.bottom());
    EXPECT_GT(sharedQueueBGmdd->sceneBoundingRect().top(), componentRect.bottom());

    const qreal statisticRowBottom = std::max(counterGmdd->sceneBoundingRect().bottom(),
                                             statisticGmdd->sceneBoundingRect().bottom());
    EXPECT_GT(sharedQueueAGmdd->sceneBoundingRect().top(), statisticRowBottom);
    EXPECT_GT(sharedQueueBGmdd->sceneBoundingRect().top(), statisticRowBottom);

    expectNoSceneRectOverlap(sceneRects({
        queueGmdd,
        resourceGmdd,
        counterGmdd,
        statisticGmdd,
        sharedQueueAGmdd,
        sharedQueueBGmdd
    }));
}

TEST(GuiGmddLayout, PropertyEditorSelectionKeepsAttachedEditableDataDefinitionsEditable) {
    Simulator simulator;
    PluginManager* pluginManager = simulator.getPluginManager();
    ASSERT_NE(pluginManager, nullptr);
    pluginManager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Queue* queue = new Queue(model, "Queue_PropertyEditor");
    Seize* seize = new Seize(model, "Seize_PropertyEditor");
    ASSERT_NE(queue, nullptr);
    ASSERT_NE(seize, nullptr);
    seize->setQueueableItem(new QueueableItem(queue));
    ModelDataDefinition::CreateInternalData(seize);

    ModelGraphicsView graphicsView;
    graphicsView.setSimulator(&simulator);
    ModelGraphicsScene* scene = graphicsView.getScene();
    ASSERT_NE(scene, nullptr);
    scene->setShowEditableDataDefinitions(true);
    scene->setShowStatisticsDataDefinitions(true);
    scene->setShowSharedDataDefinitions(true);
    scene->setShowRecursiveDataDefinitions(false);

    Plugin* seizePlugin = pluginManager->find(Util::TypeOf<Seize>());
    ASSERT_NE(seizePlugin, nullptr);
    auto* graphicalSeize = new GraphicalModelComponent(
        seizePlugin,
        seize,
        QPointF(600.0, 600.0),
        QColor(105, 105, 105));
    scene->addItem(graphicalSeize);

    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);

    GraphicalModelDataDefinition* queueGmdd = findGraphicalDataDefinition(*scene, queue);
    ASSERT_NE(queueGmdd, nullptr);
    ASSERT_TRUE(queueGmdd->isEditableInPropertyEditor());

    ObjectPropertyBrowser propertyBrowser;
    PropertyEditorGenesys propertyEditor;
    std::map<SimulationControl*, DataComponentProperty*> propertyList;
    std::map<SimulationControl*, DataComponentEditor*> propertyEditorUi;
    std::map<SimulationControl*, ComboBoxEnum*> propertyCombo;
    PropertyEditorController controller(
        &propertyBrowser,
        &graphicsView,
        &propertyEditor,
        &propertyList,
        &propertyEditorUi,
        &propertyCombo,
        {},
        {},
        {},
        {},
        {},
        {},
        {});

    queueGmdd->setSelected(true);
    controller.sceneSelectionChanged();

    EXPECT_TRUE(queueGmdd->isEditableInPropertyEditor());
}

TEST(GuiGmddLayout, ComponentLoadRestoresEditableAuxiliaryItemControlsAndResourceEditability) {
    Simulator savingSimulator;
    PluginManager* savingPluginManager = savingSimulator.getPluginManager();
    ASSERT_NE(savingPluginManager, nullptr);
    savingPluginManager->autoInsertPlugins();

    Model* savingModel = savingSimulator.getModelManager()->newModel();
    ASSERT_NE(savingModel, nullptr);

    Queue* savingQueue = new Queue(savingModel, "Loaded.Queue");
    Resource* savingResource = new Resource(savingModel, "Loaded.Resource");
    Seize* savingSeize = new Seize(savingModel, "Loaded.Seize");
    Release* savingRelease = new Release(savingModel, "Loaded.Release");
    ASSERT_NE(savingQueue, nullptr);
    ASSERT_NE(savingResource, nullptr);
    ASSERT_NE(savingSeize, nullptr);
    ASSERT_NE(savingRelease, nullptr);
    savingSeize->setQueueableItem(new QueueableItem(savingQueue));
    savingSeize->addRequest(new SeizableItem(savingResource));
    savingRelease->addReleaseRequests(new SeizableItem(savingResource));
    ModelDataDefinition::CreateInternalData(savingSeize);
    ModelDataDefinition::CreateInternalData(savingRelease);

    GenSerializer savingSerializer(savingModel);
    auto seizeFields = std::unique_ptr<PersistenceRecord>(savingSerializer.newPersistenceRecord());
    auto releaseFields = std::unique_ptr<PersistenceRecord>(savingSerializer.newPersistenceRecord());
    ModelComponent::SaveInstance(seizeFields.get(), savingSeize);
    ModelComponent::SaveInstance(releaseFields.get(), savingRelease);

    Simulator loadingSimulator;
    PluginManager* loadingPluginManager = loadingSimulator.getPluginManager();
    ASSERT_NE(loadingPluginManager, nullptr);
    loadingPluginManager->autoInsertPlugins();

    Model* loadingModel = loadingSimulator.getModelManager()->newModel();
    ASSERT_NE(loadingModel, nullptr);
    auto* loadingQueue = new Queue(loadingModel, "Loaded.Queue");
    auto* loadingResource = new Resource(loadingModel, "Loaded.Resource");
    ASSERT_NE(loadingQueue, nullptr);
    ASSERT_NE(loadingResource, nullptr);

    auto* loadedSeize = dynamic_cast<Seize*>(Seize::LoadInstance(loadingModel, seizeFields.get()));
    auto* loadedRelease = dynamic_cast<Release*>(Release::LoadInstance(loadingModel, releaseFields.get()));
    ASSERT_NE(loadedSeize, nullptr);
    ASSERT_NE(loadedRelease, nullptr);
    ModelDataDefinition::CreateInternalData(loadedSeize);
    ModelDataDefinition::CreateInternalData(loadedRelease);

    ASSERT_NE(loadedSeize->getQueueableItem(), nullptr);
    ASSERT_EQ(loadedSeize->getSeizeRequests()->size(), 1u);
    ASSERT_EQ(loadedRelease->getReleaseRequests()->size(), 1u);

    EXPECT_GT(loadedSeize->getQueueableItem()->getSimulationControls()->size(), 0u);
    EXPECT_GT(loadedSeize->getSeizeRequests()->list()->front()->getSimulationControls()->size(), 0u);
    EXPECT_GT(loadedRelease->getReleaseRequests()->list()->front()->getSimulationControls()->size(), 0u);

    SimulationControl* queueableControl = findSimulationControlByName(loadedSeize->getSimulationControls(), "QueueableItem");
    SimulationControl* requestsControl = findSimulationControlByName(loadedSeize->getSimulationControls(), "Requests");
    SimulationControl* releaseRequestsControl = findSimulationControlByName(loadedRelease->getSimulationControls(), "ReleaseRequests");
    ASSERT_NE(queueableControl, nullptr);
    ASSERT_NE(requestsControl, nullptr);
    ASSERT_NE(releaseRequestsControl, nullptr);
    ASSERT_NE(queueableControl->getEditableChildSimulationControls(), nullptr);
    ASSERT_NE(requestsControl->getEditableChildSimulationControls(0), nullptr);
    ASSERT_NE(releaseRequestsControl->getEditableChildSimulationControls(0), nullptr);
    EXPECT_GT(queueableControl->getEditableChildSimulationControls()->size(), 0u);
    EXPECT_GT(requestsControl->getEditableChildSimulationControls(0)->size(), 0u);
    EXPECT_GT(releaseRequestsControl->getEditableChildSimulationControls(0)->size(), 0u);

    ModelGraphicsView graphicsView;
    graphicsView.setSimulator(&loadingSimulator);
    ModelGraphicsScene* scene = graphicsView.getScene();
    ASSERT_NE(scene, nullptr);
    scene->setShowEditableDataDefinitions(true);
    scene->setShowStatisticsDataDefinitions(true);
    scene->setShowSharedDataDefinitions(true);
    scene->setShowRecursiveDataDefinitions(false);

    PropertyEditorGenesys propertyEditor;
    std::map<SimulationControl*, DataComponentProperty*> propertyList;
    std::map<SimulationControl*, DataComponentEditor*> propertyEditorUi;
    std::map<SimulationControl*, ComboBoxEnum*> propertyCombo;
    scene->setPropertyEditor(&propertyEditor);
    scene->setPropertyList(&propertyList);
    scene->setPropertyEditorUI(&propertyEditorUi);
    scene->setComboBox(&propertyCombo);

    Plugin* seizePlugin = loadingPluginManager->find(Util::TypeOf<Seize>());
    Plugin* releasePlugin = loadingPluginManager->find(Util::TypeOf<Release>());
    ASSERT_NE(seizePlugin, nullptr);
    ASSERT_NE(releasePlugin, nullptr);
    auto* graphicalSeize = new GraphicalModelComponent(
        seizePlugin,
        loadedSeize,
        QPointF(610.0, 640.0),
        QColor("#336699"));
    auto* graphicalRelease = new GraphicalModelComponent(
        releasePlugin,
        loadedRelease,
        QPointF(860.0, 640.0),
        QColor("#884422"));
    scene->addItem(graphicalSeize);
    scene->getGraphicalModelComponents()->append(graphicalSeize);
    scene->getAllComponents()->append(graphicalSeize);
    scene->addItem(graphicalRelease);
    scene->getGraphicalModelComponents()->append(graphicalRelease);
    scene->getAllComponents()->append(graphicalRelease);

    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&loadingSimulator, scene);
    GraphicalModelDataDefinition* loadedResourceGmdd = findGraphicalDataDefinition(*scene, loadingResource);
    ASSERT_NE(loadedResourceGmdd, nullptr);
    EXPECT_TRUE(loadedResourceGmdd->isEditableInPropertyEditor());

    ObjectPropertyBrowser propertyBrowser;
    PropertyEditorController controller(
        &propertyBrowser,
        &graphicsView,
        &propertyEditor,
        &propertyList,
        &propertyEditorUi,
        &propertyCombo,
        {},
        {},
        {},
        {},
        {},
        {},
        {});
    scene->clearSelection();
    loadedResourceGmdd->setSelected(true);
    controller.sceneSelectionChanged();
    EXPECT_TRUE(loadedResourceGmdd->isEditableInPropertyEditor());
}

TEST(GuiGmddLayout, RecursiveDataDefinitionExpansionShowsDataDefinitionsLinkedToDataDefinitions) {
    Simulator simulator;
    PluginManager* pluginManager = simulator.getPluginManager();
    ASSERT_NE(pluginManager, nullptr);
    pluginManager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    auto* queue = new QueueWithPublicAttachments(model, "ParentQueue");
    auto* resource = new Resource(model, "ChildResource");
    auto* childStatistic = new StatisticsCollector(model, "ChildStatistic", queue, false);
    auto* seize = new Seize(model, "Seize_Recursive");
    ASSERT_NE(queue, nullptr);
    ASSERT_NE(resource, nullptr);
    ASSERT_NE(childStatistic, nullptr);
    ASSERT_NE(seize, nullptr);

    queue->attachDataDefinition("ChildResource", resource);
    queue->insertInternalDataDefinition("ChildStatistic", childStatistic);
    seize->setQueueableItem(new QueueableItem(queue));
    ModelDataDefinition::CreateInternalData(seize);

    ModelGraphicsScene scene(0, 0, 2000, 2000);
    scene.setSimulator(&simulator);
    scene.setShowEditableDataDefinitions(true);
    scene.setShowStatisticsDataDefinitions(true);
    scene.setShowSharedDataDefinitions(true);
    scene.setShowRecursiveDataDefinitions(false);

    Plugin* seizePlugin = pluginManager->find(Util::TypeOf<Seize>());
    ASSERT_NE(seizePlugin, nullptr);
    auto* graphicalSeize = new GraphicalModelComponent(
        seizePlugin,
        seize,
        QPointF(600.0, 600.0),
        QColor(105, 105, 105));
    scene.addItem(graphicalSeize);

    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, &scene);

    GraphicalModelDataDefinition* queueGmdd = findGraphicalDataDefinition(scene, queue);
    GraphicalModelDataDefinition* resourceGmdd = findGraphicalDataDefinition(scene, resource);
    GraphicalModelDataDefinition* statisticGmdd = findGraphicalDataDefinition(scene, childStatistic);
    ASSERT_NE(queueGmdd, nullptr);
    EXPECT_EQ(resourceGmdd, nullptr);
    EXPECT_EQ(statisticGmdd, nullptr);

    scene.setShowRecursiveDataDefinitions(true);
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, &scene);

    queueGmdd = findGraphicalDataDefinition(scene, queue);
    resourceGmdd = findGraphicalDataDefinition(scene, resource);
    statisticGmdd = findGraphicalDataDefinition(scene, childStatistic);
    ASSERT_NE(queueGmdd, nullptr);
    ASSERT_NE(resourceGmdd, nullptr);
    ASSERT_NE(statisticGmdd, nullptr);
    EXPECT_NE(findDiagramConnection(scene, resourceGmdd, queueGmdd), nullptr);
    EXPECT_NE(findDiagramConnection(scene, statisticGmdd, queueGmdd), nullptr);
}

TEST(GuiGmddLayout, RequestSyncIsDeferredWhilePersistedLayoutRestoreIsActive) {
    Simulator simulator;
    PluginManager* pluginManager = simulator.getPluginManager();
    ASSERT_NE(pluginManager, nullptr);
    pluginManager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Queue* queue = new Queue(model, "Queue_Defer");
    Seize* seize = new Seize(model, "Seize_Defer");
    ASSERT_NE(queue, nullptr);
    ASSERT_NE(seize, nullptr);
    seize->setQueueableItem(new QueueableItem(queue));
    ModelDataDefinition::CreateInternalData(seize);

    ModelGraphicsScene scene(0, 0, 2000, 2000);
    scene.setSimulator(&simulator);
    scene.setShowEditableDataDefinitions(true);
    scene.setShowStatisticsDataDefinitions(true);
    scene.setShowSharedDataDefinitions(true);
    scene.setShowRecursiveDataDefinitions(false);

    Plugin* seizePlugin = pluginManager->find(Util::TypeOf<Seize>());
    ASSERT_NE(seizePlugin, nullptr);
    auto* graphicalSeize = new GraphicalModelComponent(
        seizePlugin,
        seize,
        QPointF(600.0, 600.0),
        QColor(105, 105, 105));
    scene.addItem(graphicalSeize);

    scene.setRestoringPersistedGuiLayout(true);
    scene.requestGraphicalDataDefinitionsSync();
    QApplication::processEvents();

    EXPECT_EQ(findGraphicalDataDefinition(scene, queue), nullptr);

    scene.setRestoringPersistedGuiLayout(false);
    QApplication::processEvents();

    EXPECT_NE(findGraphicalDataDefinition(scene, queue), nullptr);
}

TEST(GuiGmddLayout, SerializerRoundTripRestoresComponentColorAndDataDefinitionPosition) {
    Simulator simulator;
    PluginManager* pluginManager = simulator.getPluginManager();
    ASSERT_NE(pluginManager, nullptr);
    pluginManager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Queue* queue = new Queue(model, "Queue_Serializer");
    Seize* seize = new Seize(model, "Seize_Serializer");
    ASSERT_NE(queue, nullptr);
    ASSERT_NE(seize, nullptr);
    seize->setQueueableItem(new QueueableItem(queue));
    ModelDataDefinition::CreateInternalData(seize);

    ModelGraphicsView graphicsView;
    graphicsView.setSimulator(&simulator);
    ModelGraphicsScene* scene = graphicsView.getScene();
    ASSERT_NE(scene, nullptr);
    QUndoStack undoStack;
    scene->setUndoStack(&undoStack);
    scene->setShowEditableDataDefinitions(true);
    scene->setShowStatisticsDataDefinitions(true);
    scene->setShowSharedDataDefinitions(true);
    scene->setShowRecursiveDataDefinitions(false);
    PropertyEditorGenesys propertyEditor;
    std::map<SimulationControl*, DataComponentProperty*> propertyList;
    std::map<SimulationControl*, DataComponentEditor*> propertyEditorUi;
    std::map<SimulationControl*, ComboBoxEnum*> propertyCombo;
    scene->setPropertyEditor(&propertyEditor);
    scene->setPropertyList(&propertyList);
    scene->setPropertyEditorUI(&propertyEditorUi);
    scene->setComboBox(&propertyCombo);

    Plugin* seizePlugin = pluginManager->find(Util::TypeOf<Seize>());
    ASSERT_NE(seizePlugin, nullptr);
    const QColor desiredComponentColor("#336699");
    auto* graphicalSeize = new GraphicalModelComponent(
        seizePlugin,
        seize,
        QPointF(610.0, 640.0),
        desiredComponentColor);
    scene->addItem(graphicalSeize);
    scene->getGraphicalModelComponents()->append(graphicalSeize);
    scene->getAllComponents()->append(graphicalSeize);
    ASSERT_NE(graphicalSeize, nullptr);
    auto* movedDecide = new Decide(model, "Decide_SerializerMove");
    movedDecide->addConditions("1");
    movedDecide->addConditions("0");
    auto* movedTarget = new Dispose(model, "Dispose_SerializerMove");
    Plugin* decidePlugin = pluginManager->find(Util::TypeOf<Decide>());
    Plugin* disposePlugin = pluginManager->find(Util::TypeOf<Dispose>());
    ASSERT_NE(decidePlugin, nullptr);
    ASSERT_NE(disposePlugin, nullptr);
    auto* graphicalDecide = new GraphicalModelComponent(decidePlugin, movedDecide, QPointF(100, 100), Qt::blue);
    auto* graphicalTarget = new GraphicalModelComponent(disposePlugin, movedTarget, QPointF(400, 100), Qt::blue);
    scene->addItem(graphicalDecide);
    scene->addItem(graphicalTarget);
    scene->getGraphicalModelComponents()->append(graphicalDecide);
    scene->getGraphicalModelComponents()->append(graphicalTarget);
    scene->getAllComponents()->append(graphicalDecide);
    scene->getAllComponents()->append(graphicalTarget);
    movedDecide->getConnectionManager()->insertAtPort(0, new Connection{movedTarget, {0}});
    scene->addGraphicalConnection(graphicalDecide->getGraphicalOutputPorts().at(0),
        graphicalTarget->getGraphicalInputPorts().at(0), 0, 0);
    movedDecide->moveCondition(0, 1);
    graphicalDecide->moveDecideConditionPort(0, 1);
    const QString expectedCategoryColor = QColor(Qt::darkGreen).name(QColor::HexRgb);

    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);
    auto* queueGmdd = findGraphicalDataDefinition(*scene, queue);
    ASSERT_NE(queueGmdd, nullptr);
    const QPointF desiredGmddPos(240.0, 180.0);
    queueGmdd->setPos(desiredGmddPos);
    queueGmdd->setOldPosition(desiredGmddPos.x(), desiredGmddPos.y());

    QPlainTextEdit modelTextEditor;
    modelTextEditor.setPlainText(QString::fromStdString(model->showLanguage()));
    QTextEdit console;
    QSlider zoomSlider;
    zoomSlider.setRange(10, 400);
    zoomSlider.setValue(160);
    QAction actionShowGrid;
    QAction actionShowRule;
    QAction actionShowSnap;
    QAction actionShowGuides;
    QAction actionShowInternalElements;
    QAction actionShowEditableElements;
    QAction actionShowAttachedElements;
    QAction actionShowRecursiveElements;
    actionShowGrid.setCheckable(true);
    actionShowRule.setCheckable(true);
    actionShowSnap.setCheckable(true);
    actionShowGuides.setCheckable(true);
    actionShowInternalElements.setCheckable(true);
    actionShowEditableElements.setCheckable(true);
    actionShowAttachedElements.setCheckable(true);
    actionShowRecursiveElements.setCheckable(true);
    actionShowGrid.setChecked(true);
    actionShowRule.setChecked(true);
    actionShowSnap.setChecked(false);
    actionShowGuides.setChecked(true);
    actionShowInternalElements.setChecked(true);
    actionShowEditableElements.setChecked(true);
    actionShowAttachedElements.setChecked(true);
    actionShowRecursiveElements.setChecked(false);
    QString modelFilename;
    std::map<std::string, QColor> pluginCategoryColor;
    GraphicalModelBuilder builder(&simulator, &graphicsView, scene, &pluginCategoryColor, &console);

    auto clearSceneForReload = [&]() {
        scene->grid()->clear();
        scene->clearGraphicalModelConnections();
        scene->clearGraphicalModelComponents();
        scene->clearGraphicalDiagramConnections();
        scene->clearGraphicalModelDataDefinitions();
        scene->clearAnimations();
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

    GraphicalModelSerializer serializer(&simulator,
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
                                        [&]() { builder.generateGraphicalModelFromModel(); },
                                        [&]() {
                                            scene->setShowStatisticsDataDefinitions(actionShowInternalElements.isChecked());
                                            scene->requestGraphicalDataDefinitionsSync();
                                        },
                                        [&]() {
                                            scene->setShowEditableDataDefinitions(actionShowEditableElements.isChecked());
                                            scene->requestGraphicalDataDefinitionsSync();
                                        },
                                        [&]() {
                                            scene->setShowSharedDataDefinitions(actionShowAttachedElements.isChecked());
                                            scene->requestGraphicalDataDefinitionsSync();
                                        });

    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());
    const QString guiFilename = temporaryDir.filePath("round_trip.gui");
    ASSERT_TRUE(serializer.saveGraphicalModel(guiFilename));
    QFile savedGui(guiFilename);
    ASSERT_TRUE(savedGui.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString savedGuiContent = QString::fromUtf8(savedGui.readAll());
    savedGui.close();
    const int graphicalMarkerIndex = savedGuiContent.indexOf("# Genesys Graphic Model");
    ASSERT_NE(graphicalMarkerIndex, -1);
    const QString graphicalBlock = savedGuiContent.mid(graphicalMarkerIndex);
    EXPECT_TRUE(graphicalBlock.contains("# 0 GUI "));
    EXPECT_TRUE(graphicalBlock.contains("# 0 Show "));
    EXPECT_TRUE(graphicalBlock.contains("# Draws"));
    EXPECT_TRUE(graphicalBlock.contains("# Animations"));
    EXPECT_TRUE(graphicalBlock.contains("# Graphical Plugins"));
    EXPECT_TRUE(graphicalBlock.contains("# Graphical Model Data Definitions"));
    EXPECT_TRUE(graphicalBlock.contains("# Graphical Model Components"));
    EXPECT_TRUE(graphicalBlock.contains("# Groups"));
    EXPECT_FALSE(graphicalBlock.contains("# ["));
    EXPECT_FALSE(graphicalBlock.contains("# ]0+"));
    const QStringList graphicalLines = graphicalBlock.split('\n');
    for (const QString& graphicalLine : graphicalLines) {
        const QString trimmedLine = graphicalLine.trimmed();
        if (trimmedLine.isEmpty()) {
            continue;
        }
        EXPECT_TRUE(trimmedLine.startsWith("#")) << trimmedLine.toStdString();
    }

    graphicalSeize->setColor(QColor("#FF0000"));
    queueGmdd->setPos(QPointF(1200.0, 1200.0));

    Model* loadedModel = serializer.loadGraphicalModel(guiFilename.toStdString());
    ASSERT_NE(loadedModel, nullptr);
    QApplication::processEvents();

    GraphicalModelComponent* loadedComponent = findGraphicalComponentByName(*scene, "Seize_Serializer");
    ASSERT_NE(loadedComponent, nullptr);
    EXPECT_EQ(loadedComponent->getColor().name(QColor::HexRgb), expectedCategoryColor);
    auto* loadedDecide = dynamic_cast<Decide*>(loadedModel->getComponentManager()->find("Decide_SerializerMove"));
    ASSERT_NE(loadedDecide, nullptr);
    EXPECT_EQ(loadedDecide->getConditions()->getAtRank(1), "1");
    ASSERT_NE(loadedDecide->getConnectionManager()->getConnectionAtPort(1), nullptr);
    EXPECT_EQ(loadedDecide->getConnectionManager()->getConnectionAtPort(1)->component->getName(), "Dispose_SerializerMove");
    auto* loadedGraphicalDecide = findGraphicalComponentByName(*scene, "Decide_SerializerMove");
    ASSERT_NE(loadedGraphicalDecide, nullptr);
    ASSERT_EQ(loadedGraphicalDecide->getGraphicalOutputPorts().at(1)->getConnections()->size(), 1);
    EXPECT_EQ(loadedGraphicalDecide->getGraphicalOutputPorts().at(1)->getConnections()->front()->getPortSourceConnection(), 1u);

    GraphicalModelDataDefinition* loadedQueueGmdd = findGraphicalDataDefinitionByTypeAndName(
        *scene,
        Util::TypeOf<Queue>(),
        "Queue_Serializer");
    ASSERT_NE(loadedQueueGmdd, nullptr);
    EXPECT_NEAR(loadedQueueGmdd->scenePos().x(), desiredGmddPos.x(), 0.2);
    EXPECT_NEAR(loadedQueueGmdd->scenePos().y(), desiredGmddPos.y(), 0.2);
    if (scene->getGraphicalDiagramsConnections() != nullptr) {
        for (QGraphicsItem* item : *scene->getGraphicalDiagramsConnections()) {
            auto* connection = dynamic_cast<GraphicalDiagramConnection*>(item);
            if (connection == nullptr
                || connection->getDataDefinition() == nullptr
                || connection->getLinkedDataDefinition() == nullptr) {
                continue;
            }
            const QLineF expectedLine = expectedDiagramConnectionLine(
                connection->getDataDefinition()->sceneBoundingRect(),
                connection->getLinkedDataDefinition()->sceneBoundingRect());
            const QLineF restoredLine = connection->line();
            EXPECT_NEAR(restoredLine.x1(), expectedLine.x1(), 0.5);
            EXPECT_NEAR(restoredLine.y1(), expectedLine.y1(), 0.5);
            EXPECT_NEAR(restoredLine.x2(), expectedLine.x2(), 0.5);
            EXPECT_NEAR(restoredLine.y2(), expectedLine.y2(), 0.5);
        }
    }
}

// Regression coverage for the GUI model save/load bug investigation: the
// existing SerializerRoundTrip* tests above use exactly one Queue + one
// Seize, with no "next" connections and no cross-component shared
// reference -- the two scenarios that most plausibly explain a complex,
// long-edited model failing to reload correctly. This test exercises a
// small but genuinely "complex" model (two components, a next connection,
// and a Resource referenced by both) through the real file-based
// saveGraphicalModel/loadGraphicalModel path.
TEST(GuiGmddLayout, SerializerRoundTripRestoresConnectedComponentsAndSharedReference) {
    Simulator simulator;
    PluginManager* pluginManager = simulator.getPluginManager();
    ASSERT_NE(pluginManager, nullptr);
    pluginManager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Queue* queue = new Queue(model, "Queue_Complex");
    Resource* resource = new Resource(model, "Resource_Complex");
    Seize* seize = new Seize(model, "Seize_Complex");
    Release* release = new Release(model, "Release_Complex");
    auto* sharedSignalData = new SignalData(model, "SignalData_Complex");
    auto* wait = new Wait(model, "Wait_Complex");
    auto* signal = new Signal(model, "Signal_Complex");
    ASSERT_NE(queue, nullptr);
    ASSERT_NE(resource, nullptr);
    ASSERT_NE(seize, nullptr);
    ASSERT_NE(release, nullptr);
    ASSERT_NE(sharedSignalData, nullptr);
    ASSERT_NE(wait, nullptr);
    ASSERT_NE(signal, nullptr);

    wait->setWaitType(Wait::WaitType::WaitForSignal);
    wait->setQueue(queue);
    wait->setSignalData(sharedSignalData);
    signal->setSignalData(sharedSignalData);

    seize->setQueueableItem(new QueueableItem(queue));
    seize->addRequest(new SeizableItem(resource));
    release->addReleaseRequests(new SeizableItem(resource));
    seize->connectTo(release, 0);
    ModelDataDefinition::CreateInternalData(seize);
    ModelDataDefinition::CreateInternalData(release);

    ModelGraphicsView graphicsView;
    graphicsView.setSimulator(&simulator);
    ModelGraphicsScene* scene = graphicsView.getScene();
    ASSERT_NE(scene, nullptr);
    QUndoStack undoStack;
    scene->setUndoStack(&undoStack);
    scene->setShowEditableDataDefinitions(true);
    scene->setShowStatisticsDataDefinitions(true);
    scene->setShowSharedDataDefinitions(true);
    scene->setShowRecursiveDataDefinitions(false);
    PropertyEditorGenesys propertyEditor;
    std::map<SimulationControl*, DataComponentProperty*> propertyList;
    std::map<SimulationControl*, DataComponentEditor*> propertyEditorUi;
    std::map<SimulationControl*, ComboBoxEnum*> propertyCombo;
    scene->setPropertyEditor(&propertyEditor);
    scene->setPropertyList(&propertyList);
    scene->setPropertyEditorUI(&propertyEditorUi);
    scene->setComboBox(&propertyCombo);

    Plugin* seizePlugin = pluginManager->find(Util::TypeOf<Seize>());
    Plugin* releasePlugin = pluginManager->find(Util::TypeOf<Release>());
    Plugin* waitPlugin = pluginManager->find(Util::TypeOf<Wait>());
    Plugin* signalPlugin = pluginManager->find(Util::TypeOf<Signal>());
    ASSERT_NE(seizePlugin, nullptr);
    ASSERT_NE(releasePlugin, nullptr);
    ASSERT_NE(waitPlugin, nullptr);
    ASSERT_NE(signalPlugin, nullptr);
    auto* graphicalSeize = new GraphicalModelComponent(seizePlugin, seize, QPointF(400.0, 400.0), QColor("#336699"));
    auto* graphicalRelease = new GraphicalModelComponent(releasePlugin, release, QPointF(700.0, 400.0), QColor("#884422"));
    auto* graphicalWait = new GraphicalModelComponent(waitPlugin, wait, QPointF(400.0, 700.0), QColor("#336699"));
    auto* graphicalSignal = new GraphicalModelComponent(signalPlugin, signal, QPointF(700.0, 700.0), QColor("#884422"));
    scene->addItem(graphicalSeize);
    scene->getGraphicalModelComponents()->append(graphicalSeize);
    scene->getAllComponents()->append(graphicalSeize);
    scene->addItem(graphicalRelease);
    scene->getGraphicalModelComponents()->append(graphicalRelease);
    scene->getAllComponents()->append(graphicalRelease);
    scene->addItem(graphicalWait);
    scene->getGraphicalModelComponents()->append(graphicalWait);
    scene->getAllComponents()->append(graphicalWait);
    scene->addItem(graphicalSignal);
    scene->getGraphicalModelComponents()->append(graphicalSignal);
    scene->getAllComponents()->append(graphicalSignal);

    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);

    QPlainTextEdit modelTextEditor;
    modelTextEditor.setPlainText(QString::fromStdString(model->showLanguage()));
    QTextEdit console;
    QSlider zoomSlider;
    zoomSlider.setRange(10, 400);
    zoomSlider.setValue(100);
    QAction actionShowGrid;
    QAction actionShowRule;
    QAction actionShowSnap;
    QAction actionShowGuides;
    QAction actionShowInternalElements;
    QAction actionShowEditableElements;
    QAction actionShowAttachedElements;
    QAction actionShowRecursiveElements;
    actionShowGrid.setCheckable(true);
    actionShowRule.setCheckable(true);
    actionShowSnap.setCheckable(true);
    actionShowGuides.setCheckable(true);
    actionShowInternalElements.setCheckable(true);
    actionShowEditableElements.setCheckable(true);
    actionShowAttachedElements.setCheckable(true);
    actionShowRecursiveElements.setCheckable(true);
    actionShowGrid.setChecked(true);
    actionShowRule.setChecked(true);
    actionShowInternalElements.setChecked(true);
    actionShowEditableElements.setChecked(true);
    actionShowAttachedElements.setChecked(true);
    QString modelFilename;
    std::map<std::string, QColor> pluginCategoryColor;
    GraphicalModelBuilder builder(&simulator, &graphicsView, scene, &pluginCategoryColor, &console);

    auto clearSceneForReload = [&]() {
        scene->grid()->clear();
        scene->clearGraphicalModelConnections();
        scene->clearGraphicalModelComponents();
        scene->clearGraphicalDiagramConnections();
        scene->clearGraphicalModelDataDefinitions();
        scene->clearAnimations();
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

    GraphicalModelSerializer serializer(&simulator,
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
                                        [&]() { builder.generateGraphicalModelFromModel(); },
                                        [&]() {
                                            scene->setShowStatisticsDataDefinitions(actionShowInternalElements.isChecked());
                                            scene->requestGraphicalDataDefinitionsSync();
                                        },
                                        [&]() {
                                            scene->setShowEditableDataDefinitions(actionShowEditableElements.isChecked());
                                            scene->requestGraphicalDataDefinitionsSync();
                                        },
                                        [&]() {
                                            scene->setShowSharedDataDefinitions(actionShowAttachedElements.isChecked());
                                            scene->requestGraphicalDataDefinitionsSync();
                                        });

    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());
    const QString guiFilename = temporaryDir.filePath("complex_round_trip.gui");
    ASSERT_TRUE(serializer.saveGraphicalModel(guiFilename));

    Model* loadedModel = serializer.loadGraphicalModel(guiFilename.toStdString());
    ASSERT_NE(loadedModel, nullptr);
    QApplication::processEvents();

    Seize* loadedSeize = dynamic_cast<Seize*>(loadedModel->getComponentManager()->find("Seize_Complex"));
    Release* loadedRelease = dynamic_cast<Release*>(loadedModel->getComponentManager()->find("Release_Complex"));
    auto* loadedWait = dynamic_cast<Wait*>(loadedModel->getComponentManager()->find("Wait_Complex"));
    auto* loadedSignal = dynamic_cast<Signal*>(loadedModel->getComponentManager()->find("Signal_Complex"));
    ASSERT_NE(loadedSeize, nullptr);
    ASSERT_NE(loadedRelease, nullptr);
    ASSERT_NE(loadedWait, nullptr);
    ASSERT_NE(loadedSignal, nullptr);

    // The "next" transfer connection between the two top-level components must survive.
    ASSERT_GT(loadedSeize->getConnectionManager()->size(), 0u);
    EXPECT_EQ(loadedSeize->getConnectionManager()->getFrontConnection()->component, loadedRelease);

    // Both components reference the SAME Resource, not two independently-fabricated ones.
    ASSERT_NE(loadedSeize->getQueueableItem(), nullptr);
    EXPECT_NE(loadedSeize->getQueueableItem()->getQueue(), nullptr);
    ASSERT_EQ(loadedSeize->getSeizeRequests()->size(), 1u);
    ASSERT_EQ(loadedRelease->getReleaseRequests()->size(), 1u);
    Resource* seizeResource = loadedSeize->getSeizeRequests()->list()->front()->getResource();
    Resource* releaseResource = loadedRelease->getReleaseRequests()->list()->front()->getResource();
    ASSERT_NE(seizeResource, nullptr);
    ASSERT_NE(releaseResource, nullptr);
    EXPECT_EQ(seizeResource, releaseResource);
    EXPECT_EQ(loadedModel->getDataManager()->getDataDefinitionList(Util::TypeOf<Resource>())->size(), 1u);
    EXPECT_EQ(loadedModel->getDataManager()->getDataDefinitionList(Util::TypeOf<Queue>())->size(), 1u);
    EXPECT_NE(loadedWait->getSignalData(), nullptr);
    EXPECT_EQ(loadedWait->getSignalData(), loadedSignal->getSignalData());
    EXPECT_EQ(loadedWait->getSignalData()->getName(), "SignalData_Complex");
    ASSERT_NE(loadedWait->getQueue(), nullptr);
    EXPECT_EQ(loadedWait->getQueue()->getName(), "Queue_Complex");
    EXPECT_EQ(loadedModel->getDataManager()->getDataDefinitionList(Util::TypeOf<Queue>())->size(), 1u);
    EXPECT_EQ(loadedModel->getDataManager()->getDataDefinitionList(Util::TypeOf<SignalData>())->size(), 1u);
}

TEST(GuiGmddLayout, SerializerRoundTripRestoresViewStateGeometriesAndGroups) {
    Simulator simulator;
    PluginManager* pluginManager = simulator.getPluginManager();
    ASSERT_NE(pluginManager, nullptr);
    pluginManager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Queue* queue = new Queue(model, "Queue_Serializer_Visual");
    Seize* seize = new Seize(model, "Seize_Serializer_Visual");
    ASSERT_NE(queue, nullptr);
    ASSERT_NE(seize, nullptr);
    seize->setQueueableItem(new QueueableItem(queue));
    ModelDataDefinition::CreateInternalData(seize);

    ModelGraphicsView graphicsView;
    graphicsView.resize(1024, 768);
    graphicsView.setSimulator(&simulator);
    graphicsView.show();
    QApplication::processEvents();

    ModelGraphicsScene* scene = graphicsView.getScene();
    ASSERT_NE(scene, nullptr);
    QUndoStack undoStack;
    scene->setUndoStack(&undoStack);
    scene->setShowEditableDataDefinitions(true);
    scene->setShowStatisticsDataDefinitions(true);
    scene->setShowSharedDataDefinitions(true);
    scene->setShowRecursiveDataDefinitions(true);
    PropertyEditorGenesys propertyEditor;
    std::map<SimulationControl*, DataComponentProperty*> propertyList;
    std::map<SimulationControl*, DataComponentEditor*> propertyEditorUi;
    std::map<SimulationControl*, ComboBoxEnum*> propertyCombo;
    scene->setPropertyEditor(&propertyEditor);
    scene->setPropertyList(&propertyList);
    scene->setPropertyEditorUI(&propertyEditorUi);
    scene->setComboBox(&propertyCombo);

    Plugin* seizePlugin = pluginManager->find(Util::TypeOf<Seize>());
    ASSERT_NE(seizePlugin, nullptr);
    auto* graphicalSeize = new GraphicalModelComponent(
        seizePlugin,
        seize,
        QPointF(700.0, 730.0),
        QColor("#4455AA"));
    scene->addItem(graphicalSeize);
    scene->getGraphicalModelComponents()->append(graphicalSeize);
    scene->getAllComponents()->append(graphicalSeize);
    ASSERT_NE(graphicalSeize, nullptr);
    const QString expectedSavedComponentColor = graphicalSeize->getColor().name(QColor::HexRgb);

    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);
    auto* queueGmdd = findGraphicalDataDefinition(*scene, queue);
    ASSERT_NE(queueGmdd, nullptr);
    const QPointF expectedQueueGmddPos(360.0, 260.0);
    queueGmdd->setPos(expectedQueueGmddPos);
    queueGmdd->setOldPosition(expectedQueueGmddPos.x(), expectedQueueGmddPos.y());

    auto* lineItem = new QGraphicsLineItem(0.0, 0.0, 90.0, 40.0);
    lineItem->setPos(QPointF(120.0, 150.0));
    lineItem->setPen(QPen(QColor("#00AAEE"), 3.5, Qt::DashDotLine));
    lineItem->setOpacity(0.57);
    lineItem->setZValue(6.3);
    lineItem->setRotation(12.0);
    lineItem->setScale(1.2);
    lineItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
    lineItem->setFlag(QGraphicsItem::ItemIsMovable, true);
    scene->addItem(lineItem);
    scene->addDrawingGeometry(lineItem);

    auto* textItem = new QGraphicsTextItem("Persisted Visual Text");
    textItem->setPos(QPointF(250.0, 310.0));
    textItem->setDefaultTextColor(QColor("#112233"));
    QFont textFont = textItem->font();
    textFont.setPointSize(16);
    textFont.setItalic(true);
    textFont.setBold(true);
    textItem->setFont(textFont);
    textItem->setTextWidth(210.0);
    textItem->setOpacity(0.69);
    textItem->setZValue(7.5);
    textItem->setRotation(-8.0);
    textItem->setScale(1.1);
    textItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
    textItem->setFlag(QGraphicsItem::ItemIsMovable, false);
    scene->addItem(textItem);
    scene->addDrawingGeometry(textItem);

    auto* group = new QGraphicsItemGroup();
    group->addToGroup(graphicalSeize);
    group->addToGroup(textItem);
    group->setHandlesChildEvents(false);
    group->setFlag(QGraphicsItem::ItemIsSelectable, true);
    group->setFlag(QGraphicsItem::ItemIsMovable, true);
    scene->addItem(group);
    scene->getGraphicalGroups()->append(group);
    scene->insertOldPositionItem(group, group->pos());
    scene->insertOldPositionItem(graphicalSeize, graphicalSeize->pos());
    scene->insertOldPositionItem(textItem, textItem->pos());
    scene->insertComponentGroup(group, QList<GraphicalModelComponent*>({graphicalSeize}));

    auto* emptyGroup = new QGraphicsItemGroup();
    emptyGroup->setHandlesChildEvents(false);
    scene->addItem(emptyGroup);
    scene->getGraphicalGroups()->append(emptyGroup);
    scene->insertOldPositionItem(emptyGroup, emptyGroup->pos());

    QPlainTextEdit modelTextEditor;
    modelTextEditor.setPlainText(QString::fromStdString(model->showLanguage()));
    QTextEdit console;
    QSlider zoomSlider;
    zoomSlider.setRange(10, 400);
    zoomSlider.setValue(143);
    QAction actionShowGrid;
    QAction actionShowRule;
    QAction actionShowSnap;
    QAction actionShowGuides;
    QAction actionShowInternalElements;
    QAction actionShowEditableElements;
    QAction actionShowAttachedElements;
    QAction actionShowRecursiveElements;
    actionShowGrid.setCheckable(true);
    actionShowRule.setCheckable(true);
    actionShowSnap.setCheckable(true);
    actionShowGuides.setCheckable(true);
    actionShowInternalElements.setCheckable(true);
    actionShowEditableElements.setCheckable(true);
    actionShowAttachedElements.setCheckable(true);
    actionShowRecursiveElements.setCheckable(true);
    actionShowGrid.setChecked(false);
    actionShowRule.setChecked(true);
    actionShowSnap.setChecked(true);
    actionShowGuides.setChecked(false);
    actionShowInternalElements.setChecked(false);
    actionShowEditableElements.setChecked(true);
    actionShowAttachedElements.setChecked(false);
    actionShowRecursiveElements.setChecked(true);
    scene->setGridVisible(actionShowGrid.isChecked());
    scene->setSnapToGrid(actionShowSnap.isChecked());
    scene->setShowStatisticsDataDefinitions(actionShowInternalElements.isChecked());
    scene->setShowEditableDataDefinitions(actionShowEditableElements.isChecked());
    scene->setShowSharedDataDefinitions(actionShowAttachedElements.isChecked());
    scene->setShowRecursiveDataDefinitions(actionShowRecursiveElements.isChecked());
    graphicsView.setRuleVisible(actionShowRule.isChecked());
    graphicsView.setGuidesVisible(actionShowGuides.isChecked());

    QScrollBar* hBar = graphicsView.horizontalScrollBar();
    QScrollBar* vBar = graphicsView.verticalScrollBar();
    ASSERT_NE(hBar, nullptr);
    ASSERT_NE(vBar, nullptr);
    hBar->setValue(qBound(hBar->minimum(), hBar->minimum() + 35, hBar->maximum()));
    vBar->setValue(qBound(vBar->minimum(), vBar->minimum() + 55, vBar->maximum()));
    const int expectedViewpointX = hBar->value();
    const int expectedViewpointY = vBar->value();

    QString modelFilename;
    std::map<std::string, QColor> pluginCategoryColor;
    GraphicalModelBuilder builder(&simulator, &graphicsView, scene, &pluginCategoryColor, &console);

    auto clearSceneForReload = [&]() {
        scene->grid()->clear();
        scene->clearGraphicalModelConnections();
        scene->clearGraphicalModelComponents();
        scene->clearGraphicalDiagramConnections();
        scene->clearGraphicalModelDataDefinitions();
        scene->clearAnimations();
        scene->clear();
        scene->getGraphicalModelComponents()->clear();
        scene->getGraphicalConnections()->clear();
        scene->getGraphicalModelDataDefinitions()->clear();
        scene->getGraphicalDiagramsConnections()->clear();
        scene->getGraphicalGeometries()->clear();
        scene->getGraphicalGroups()->clear();
        scene->getAllComponents()->clear();
        scene->getAllConnections()->clear();
        scene->getAllDataDefinitions()->clear();
        scene->getAllGraphicalDiagramsConnections()->clear();
    };

    GraphicalModelSerializer serializer(&simulator,
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
                                        [&]() { builder.generateGraphicalModelFromModel(); },
                                        [&]() {
                                            scene->setShowStatisticsDataDefinitions(actionShowInternalElements.isChecked());
                                            scene->requestGraphicalDataDefinitionsSync();
                                        },
                                        [&]() {
                                            scene->setShowEditableDataDefinitions(actionShowEditableElements.isChecked());
                                            scene->requestGraphicalDataDefinitionsSync();
                                        },
                                        [&]() {
                                            scene->setShowSharedDataDefinitions(actionShowAttachedElements.isChecked());
                                            scene->requestGraphicalDataDefinitionsSync();
                                        });

    QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());
    const QString guiFilename = temporaryDir.filePath("round_trip_visual.gui");
    ASSERT_TRUE(serializer.saveGraphicalModel(guiFilename));

    graphicalSeize->setColor(QColor("#FF0000"));
    queueGmdd->setPos(QPointF(1500.0, 1500.0));
    lineItem->setPen(QPen(QColor("#FF00FF"), 1.0, Qt::SolidLine));
    textItem->setPlainText("Mutated");
    actionShowGrid.setChecked(true);
    actionShowAttachedElements.setChecked(true);
    zoomSlider.setValue(220);

    Model* loadedModel = serializer.loadGraphicalModel(guiFilename.toStdString());
    ASSERT_NE(loadedModel, nullptr);
    QApplication::processEvents();
    QApplication::processEvents();

    EXPECT_EQ(actionShowGrid.isChecked(), false);
    EXPECT_EQ(actionShowRule.isChecked(), true);
    EXPECT_EQ(actionShowSnap.isChecked(), true);
    EXPECT_EQ(actionShowGuides.isChecked(), false);
    EXPECT_EQ(actionShowInternalElements.isChecked(), false);
    EXPECT_EQ(actionShowEditableElements.isChecked(), true);
    EXPECT_EQ(actionShowAttachedElements.isChecked(), false);
    EXPECT_EQ(actionShowRecursiveElements.isChecked(), true);
    EXPECT_EQ(zoomSlider.value(), 143);
    EXPECT_EQ(scene->isGridVisible(), false);
    EXPECT_EQ(scene->getSnapToGrid(), true);
    EXPECT_EQ(scene->showStatisticsDataDefinitions(), false);
    EXPECT_EQ(scene->showEditableDataDefinitions(), true);
    EXPECT_EQ(scene->showSharedDataDefinitions(), false);
    EXPECT_EQ(scene->showRecursiveDataDefinitions(), true);

    EXPECT_EQ(hBar->value(), expectedViewpointX);
    EXPECT_EQ(vBar->value(), expectedViewpointY);

    GraphicalModelComponent* loadedComponent = findGraphicalComponentByName(*scene, "Seize_Serializer_Visual");
    ASSERT_NE(loadedComponent, nullptr);
    EXPECT_EQ(loadedComponent->getColor().name(QColor::HexRgb), expectedSavedComponentColor);

    GraphicalModelDataDefinition* loadedQueueGmdd = findGraphicalDataDefinitionByTypeAndName(
        *scene,
        Util::TypeOf<Queue>(),
        "Queue_Serializer_Visual");
    ASSERT_NE(loadedQueueGmdd, nullptr);
    EXPECT_NEAR(loadedQueueGmdd->scenePos().x(), expectedQueueGmddPos.x(), 0.2);
    EXPECT_NEAR(loadedQueueGmdd->scenePos().y(), expectedQueueGmddPos.y(), 0.2);

    QGraphicsLineItem* loadedLine = nullptr;
    QGraphicsTextItem* loadedText = nullptr;
    for (QGraphicsItem* geometry : *scene->getGraphicalGeometries()) {
        if (loadedLine == nullptr) {
            loadedLine = dynamic_cast<QGraphicsLineItem*>(geometry);
        }
        if (loadedText == nullptr) {
            auto* candidateText = dynamic_cast<QGraphicsTextItem*>(geometry);
            if (candidateText != nullptr && candidateText->toPlainText() == "Persisted Visual Text") {
                loadedText = candidateText;
            }
        }
    }

    ASSERT_NE(loadedLine, nullptr);
    EXPECT_EQ(loadedLine->pen().color().name(QColor::HexRgb), QString("#00aaee"));
    EXPECT_NEAR(loadedLine->pen().widthF(), 3.5, 0.01);
    EXPECT_EQ(loadedLine->pen().style(), Qt::DashDotLine);
    EXPECT_NEAR(loadedLine->opacity(), 0.57, 0.01);
    EXPECT_NEAR(loadedLine->zValue(), 6.3, 0.05);
    EXPECT_EQ(loadedLine->flags().testFlag(QGraphicsItem::ItemIsSelectable), false);
    EXPECT_EQ(loadedLine->flags().testFlag(QGraphicsItem::ItemIsMovable), true);

    ASSERT_NE(loadedText, nullptr);
    EXPECT_EQ(loadedText->defaultTextColor().name(QColor::HexRgb), QString("#112233"));
    EXPECT_EQ(loadedText->toPlainText(), QString("Persisted Visual Text"));
    EXPECT_EQ(loadedText->font().italic(), true);
    EXPECT_EQ(loadedText->font().bold(), true);
    EXPECT_EQ(loadedText->font().pointSize(), 16);
    EXPECT_NEAR(loadedText->textWidth(), 210.0, 0.2);
    EXPECT_NEAR(loadedText->opacity(), 0.69, 0.01);
    EXPECT_EQ(loadedText->flags().testFlag(QGraphicsItem::ItemIsSelectable), true);
    EXPECT_EQ(loadedText->flags().testFlag(QGraphicsItem::ItemIsMovable), false);

    bool foundGroupWithLoadedComponentAndText = false;
    for (QGraphicsItemGroup* graphicalGroup : *scene->getGraphicalGroups()) {
        if (graphicalGroup == nullptr) {
            continue;
        }
        const QList<QGraphicsItem*> children = graphicalGroup->childItems();
        if (children.contains(loadedComponent) && children.contains(loadedText)) {
            foundGroupWithLoadedComponentAndText = true;
            break;
        }
    }
    EXPECT_TRUE(foundGroupWithLoadedComponentAndText);

    bool foundEmptyGroup = false;
    for (QGraphicsItemGroup* graphicalGroup : *scene->getGraphicalGroups()) {
        if (graphicalGroup != nullptr && graphicalGroup->childItems().isEmpty()) {
            foundEmptyGroup = true;
            break;
        }
    }
    EXPECT_TRUE(foundEmptyGroup);
}

int main(int argc, char** argv) {
    qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
