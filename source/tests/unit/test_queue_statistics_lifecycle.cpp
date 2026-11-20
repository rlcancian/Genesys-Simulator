#include <gtest/gtest.h>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/essentialPlugins/Entity.h"
#include "plugins/data/DiscreteProcessing/Queue.h"

namespace {

class QueueProducerProbe final : public ModelComponent {
public:
    QueueProducerProbe(Model* model, const std::string& name)
        : ModelComponent(model, "QueueProducerProbe", name) {}

protected:
    void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override {
        (void)entity;
        (void)inputPortNumber;
    }

    bool _loadInstance(PersistenceRecord* fields) override {
        return ModelComponent::_loadInstance(fields);
    }

    void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override {
        ModelComponent::_saveInstance(fields, saveDefaultValues);
    }
};

} // namespace

TEST(QueueStatisticsLifecycleTest, DefaultStatisticsInitializeOnFirstInsertion) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Queue queue(model, "QueueLazyStatistics");
    QueueProducerProbe producer(model, "QueueLazyStatisticsProducer");

    ASSERT_TRUE(queue.isReportStatistics());
    EXPECT_EQ(queue.getInternalData("NumberInQueue"), nullptr);
    EXPECT_EQ(queue.getInternalData("TimeInQueue"), nullptr);

    Entity* entity = model->createEntity("QueueLazyStatisticsEntity", true);
    queue.insertElement(new Waiting(entity, 0.0, &producer));

    EXPECT_EQ(queue.size(), 1u);
    ASSERT_NE(queue.getInternalData("NumberInQueue"), nullptr);
    ASSERT_NE(queue.getInternalData("TimeInQueue"), nullptr);

    queue.removeElement(queue.first());
    EXPECT_EQ(queue.size(), 0u);
}

TEST(QueueStatisticsLifecycleTest, RepeatedOperationsReuseTheSameCollectors) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Queue queue(model, "QueueStableStatistics");
    QueueProducerProbe producer(model, "QueueStableStatisticsProducer");

    queue.insertElement(new Waiting(model->createEntity("QueueStableStatisticsE0", true), 0.0, &producer));
    ModelDataDefinition* numberCollector = queue.getInternalData("NumberInQueue");
    ModelDataDefinition* timeCollector = queue.getInternalData("TimeInQueue");
    ASSERT_NE(numberCollector, nullptr);
    ASSERT_NE(timeCollector, nullptr);

    queue.insertElement(new Waiting(model->createEntity("QueueStableStatisticsE1", true), 0.0, &producer));

    EXPECT_EQ(queue.getInternalData("NumberInQueue"), numberCollector);
    EXPECT_EQ(queue.getInternalData("TimeInQueue"), timeCollector);

    queue.removeElement(queue.first());
    queue.removeElement(queue.first());
    EXPECT_EQ(queue.size(), 0u);
}

TEST(QueueStatisticsLifecycleTest, DisabledStatisticsDoNotCreateCollectors) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Queue queue(model, "QueueWithoutStatistics");
    QueueProducerProbe producer(model, "QueueWithoutStatisticsProducer");
    queue.setReportStatistics(false);

    queue.insertElement(new Waiting(model->createEntity("QueueWithoutStatisticsEntity", true), 0.0, &producer));

    EXPECT_EQ(queue.size(), 1u);
    EXPECT_EQ(queue.getInternalData("NumberInQueue"), nullptr);
    EXPECT_EQ(queue.getInternalData("TimeInQueue"), nullptr);

    queue.removeElement(queue.first());
    EXPECT_EQ(queue.size(), 0u);
}
