#include "graphicals/GraphicalModelComponent.h"
#include "graphicals/GraphicalModelDataDefinition.h"
#include "graphicals/ModelGraphicsScene.h"
#include "services/GraphicalModelBuilder.h"

#include "kernel/simulator/Counter.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/StatisticsCollector.h"
#include "plugins/components/DiscreteProcessing/QueueableItem.h"
#include "plugins/components/DiscreteProcessing/SeizableItem.h"
#include "plugins/components/DiscreteProcessing/Seize.h"
#include "plugins/data/DiscreteProcessing/Queue.h"
#include "plugins/data/DiscreteProcessing/Resource.h"

#include <gtest/gtest.h>

#include <QApplication>
#include <QByteArray>
#include <QColor>
#include <QList>
#include <QRectF>

#include <algorithm>

namespace {

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

int main(int argc, char** argv) {
    qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
