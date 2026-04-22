#include "graphicals/GraphicalModelComponent.h"
#include "graphicals/GraphicalModelDataDefinition.h"
#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/ModelGraphicsView.h"
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
#include <QList>
#include <QPlainTextEdit>
#include <QRectF>
#include <QSlider>
#include <QTemporaryDir>
#include <QTextEdit>

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
    scene->setShowEditableDataDefinitions(true);
    scene->setShowStatisticsDataDefinitions(true);
    scene->setShowSharedDataDefinitions(true);
    scene->setShowRecursiveDataDefinitions(false);

    Plugin* seizePlugin = pluginManager->find(Util::TypeOf<Seize>());
    ASSERT_NE(seizePlugin, nullptr);
    const QColor desiredComponentColor("#336699");
    auto* graphicalSeize = scene->addGraphicalModelComponent(
        seizePlugin,
        seize,
        QPointF(610.0, 640.0),
        desiredComponentColor);
    ASSERT_NE(graphicalSeize, nullptr);
    const QString expectedSavedComponentColor = graphicalSeize->getColor().name(QColor::HexRgb);

    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(&simulator, scene);
    auto* queueGmdd = findGraphicalDataDefinition(scene, queue);
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
}

int main(int argc, char** argv) {
    qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
