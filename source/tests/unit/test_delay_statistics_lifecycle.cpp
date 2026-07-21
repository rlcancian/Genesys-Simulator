#include <gtest/gtest.h>

#include "kernel/simulator/Event.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/essentialPlugins/Attribute.h"
#include "kernel/simulator/essentialPlugins/Entity.h"
#include "kernel/simulator/essentialPlugins/EntityType.h"
#include "plugins/components/DiscreteProcessing/Delay.h"

#include <vector>

namespace {

class DelayProbe final : public Delay {
public:
    DelayProbe(Model* model, const std::string& name)
        : Delay(model, name) {}

    void dispatch(Entity* entity) {
        _onDispatchEvent(entity, 0u);
    }
};

class DelayCollectorSink final : public ModelComponent {
public:
    DelayCollectorSink(Model* model, const std::string& name)
        : ModelComponent(model, "DelayCollectorSink", name) {}

    const std::vector<Entity*>& receivedEntities() const {
        return _receivedEntities;
    }

protected:
    void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override {
        (void)inputPortNumber;
        _receivedEntities.push_back(entity);
    }

    bool _loadInstance(PersistenceRecord* fields) override {
        return ModelComponent::_loadInstance(fields);
    }

    void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override {
        ModelComponent::_saveInstance(fields, saveDefaultValues);
    }

private:
    std::vector<Entity*> _receivedEntities;
};

Entity* createDelayEntity(Model* model, const std::string& name) {
    EXPECT_NE(model, nullptr);

    new Attribute(model, "Entity.TotalWaitTime");

    EntityType* entityType = new EntityType(model, name + "Type");
    entityType->setReportStatistics(false);

    Entity* entity = model->createEntity(name, true);
    entity->setEntityType(entityType);
    return entity;
}

void drainFutureEvents(Model* model) {
    ASSERT_NE(model, nullptr);
    while (!model->getFutureEvents()->empty()) {
        Event* event = model->getFutureEvents()->front();
        model->getFutureEvents()->pop_front();
        ModelComponent::DispatchEvent(event);
        delete event;
    }
}

} // namespace

TEST(DelayStatisticsLifecycleTest, DefaultStatisticsInitializeOnFirstDispatchAndRouteEntity) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    DelayProbe delay(model, "DelayLazyStatistics");
    DelayCollectorSink sink(model, "DelayLazyStatisticsSink");
    delay.getConnectionManager()->insert(&sink);
    delay.setDelayExpression("0", Util::TimeUnit::second);

    Entity* entity = createDelayEntity(model, "DelayLazyStatisticsEntity");
    ASSERT_NE(entity, nullptr);

    ASSERT_TRUE(delay.isReportStatistics());
    EXPECT_EQ(delay.getInternalData("DelayTime"), nullptr);

    delay.dispatch(entity);
    drainFutureEvents(model);

    ASSERT_NE(delay.getInternalData("DelayTime"), nullptr);
    ASSERT_EQ(sink.receivedEntities().size(), 1u);
    EXPECT_EQ(sink.receivedEntities().front(), entity);
    EXPECT_DOUBLE_EQ(entity->getAttributeValue("Entity.TotalWaitTime"), 0.0);
}

TEST(DelayStatisticsLifecycleTest, DisabledStatisticsDoNotCreateCollector) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    DelayProbe delay(model, "DelayWithoutStatistics");
    delay.setReportStatistics(false);
    DelayCollectorSink sink(model, "DelayWithoutStatisticsSink");
    delay.getConnectionManager()->insert(&sink);
    delay.setDelayExpression("0", Util::TimeUnit::second);

    Entity* entity = createDelayEntity(model, "DelayWithoutStatisticsEntity");
    ASSERT_NE(entity, nullptr);

    delay.dispatch(entity);
    drainFutureEvents(model);

    EXPECT_EQ(delay.getInternalData("DelayTime"), nullptr);
    ASSERT_EQ(sink.receivedEntities().size(), 1u);
    EXPECT_EQ(sink.receivedEntities().front(), entity);
}
