#include <gtest/gtest.h>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/essentialPlugins/Attribute.h"
#include "kernel/simulator/essentialPlugins/Entity.h"
#include "kernel/simulator/essentialPlugins/EntityType.h"
#include "plugins/data/MaterialHandling/Station.h"

namespace {

Entity* createStationEntity(Model* model, Station* station, const std::string& name) {
    EXPECT_NE(model, nullptr);
    EXPECT_NE(station, nullptr);

    new Attribute(model, "Entity.Station");
    new Attribute(model, "Entity.ArrivalAt" + station->getName());

    EntityType* entityType = new EntityType(model, name + "Type");
    entityType->setReportStatistics(false);

    Entity* entity = model->createEntity(name, true);
    entity->setEntityType(entityType);
    return entity;
}

} // namespace

TEST(StationStatisticsLifecycleTest, DefaultStatisticsInitializeOnFirstEnterAndSupportLeave) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Station station(model, "StationLazyStatistics");
    Entity* entity = createStationEntity(model, &station, "StationLazyStatisticsEntity");
    ASSERT_NE(entity, nullptr);

    ASSERT_TRUE(station.isReportStatistics());
    EXPECT_EQ(station.getInternalData("NumberInStation"), nullptr);
    EXPECT_EQ(station.getInternalData("TimeInStation"), nullptr);

    station.enter(entity);

    ASSERT_NE(station.getInternalData("NumberInStation"), nullptr);
    ASSERT_NE(station.getInternalData("TimeInStation"), nullptr);
    EXPECT_DOUBLE_EQ(entity->getAttributeValue("Entity.Station"), static_cast<double>(station.getId()));

    ModelDataDefinition* numberCollector = station.getInternalData("NumberInStation");
    ModelDataDefinition* timeCollector = station.getInternalData("TimeInStation");

    station.leave(entity);

    EXPECT_DOUBLE_EQ(entity->getAttributeValue("Entity.Station"), 0.0);
    EXPECT_EQ(station.getInternalData("NumberInStation"), numberCollector);
    EXPECT_EQ(station.getInternalData("TimeInStation"), timeCollector);
}

TEST(StationStatisticsLifecycleTest, DisabledStatisticsDoNotCreateCollectors) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Station station(model, "StationWithoutStatistics");
    station.setReportStatistics(false);
    Entity* entity = createStationEntity(model, &station, "StationWithoutStatisticsEntity");
    ASSERT_NE(entity, nullptr);

    station.enter(entity);
    station.leave(entity);

    EXPECT_EQ(station.getInternalData("NumberInStation"), nullptr);
    EXPECT_EQ(station.getInternalData("TimeInStation"), nullptr);
    EXPECT_DOUBLE_EQ(entity->getAttributeValue("Entity.Station"), 0.0);
}
