#include "graphicals/GraphicalModelComponent.h"
#include "graphicals/GraphicalModelDataDefinition.h"
#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/ModelGraphicsView.h"
#include "controllers/PropertyEditorController.h"
#include "propertyeditor/ObjectPropertyBrowser.h"
#include "services/GraphicalModelBuilder.h"
#include "services/GraphicalModelSerializer.h"

#include "kernel/simulator/Counter.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/StatisticsCollector.h"
#include "../../plugins/components/DiscreteProcessing/auxiliar/QueueableItem.h"
#include "../../plugins/components/DiscreteProcessing/auxiliar/SeizableItem.h"
#include "plugins/components/DiscreteProcessing/Seize.h"
#include "plugins/data/DiscreteProcessing/Queue.h"
#include "plugins/data/DiscreteProcessing/Resource.h"

#include <gtest/gtest.h>

#include <QApplication>
#include <QAction>
#include <QByteArray>
#include <QColor>
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
#include <QUndoStack>

#include <algorithm>

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
    const QString expectedSavedComponentColor = graphicalSeize->getColor().name(QColor::HexRgb);

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
    EXPECT_EQ(loadedComponent->getColor().name(QColor::HexRgb), expectedSavedComponentColor);

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
}

int main(int argc, char** argv) {
    qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
