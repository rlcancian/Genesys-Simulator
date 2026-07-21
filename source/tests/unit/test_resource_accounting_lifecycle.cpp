#include <gtest/gtest.h>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "plugins/data/DiscreteProcessing/Resource.h"

namespace {

const char* kResourceAccountingKeys[] = {
    "ProportionSeized",
    "CapacityUtilization",
    "TimeSeized",
    "TimeFailed",
    "TotalTimeSeized",
    "TotalTimeFailed",
    "Seizes",
    "Releases",
    "CostBusy",
    "CostIdle",
    "CostPerUse"
};

void expectNoAccountingData(Resource* resource) {
    ASSERT_NE(resource, nullptr);
    for (const char* key : kResourceAccountingKeys) {
        EXPECT_EQ(resource->getInternalData(key), nullptr) << key;
    }
}

void expectAllAccountingData(Resource* resource) {
    ASSERT_NE(resource, nullptr);
    for (const char* key : kResourceAccountingKeys) {
        EXPECT_NE(resource->getInternalData(key), nullptr) << key;
    }
}

} // namespace

TEST(ResourceAccountingLifecycleTest, DefaultStatisticsInitializeOnFirstSeizeAndSupportRelease) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Resource resource(model, "ResourceLazyAccounting");
    ASSERT_TRUE(resource.isReportStatistics());
    expectNoAccountingData(&resource);

    EXPECT_TRUE(resource.seize(1u));
    EXPECT_EQ(resource.getNumberBusy(), 1u);
    EXPECT_EQ(resource.getResourceState(), Resource::ResourceState::BUSY);
    expectAllAccountingData(&resource);

    ModelDataDefinition* seizesCounter = resource.getInternalData("Seizes");
    ModelDataDefinition* releasesCounter = resource.getInternalData("Releases");
    ModelDataDefinition* costPerUseCounter = resource.getInternalData("CostPerUse");

    resource.release(1u);
    EXPECT_EQ(resource.getNumberBusy(), 0u);
    EXPECT_EQ(resource.getResourceState(), Resource::ResourceState::IDLE);

    EXPECT_EQ(resource.getInternalData("Seizes"), seizesCounter);
    EXPECT_EQ(resource.getInternalData("Releases"), releasesCounter);
    EXPECT_EQ(resource.getInternalData("CostPerUse"), costPerUseCounter);
}

TEST(ResourceAccountingLifecycleTest, DisabledStatisticsSupportSeizeAndReleaseWithoutAccountingData) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Resource resource(model, "ResourceWithoutAccounting");
    resource.setReportStatistics(false);
    expectNoAccountingData(&resource);

    EXPECT_TRUE(resource.seize(1u));
    EXPECT_EQ(resource.getNumberBusy(), 1u);
    EXPECT_EQ(resource.getResourceState(), Resource::ResourceState::BUSY);

    resource.release(1u);
    EXPECT_EQ(resource.getNumberBusy(), 0u);
    EXPECT_EQ(resource.getResourceState(), Resource::ResourceState::IDLE);
    expectNoAccountingData(&resource);
}

TEST(ResourceAccountingLifecycleTest, RepeatedOperationsReuseAccountingObjects) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Resource resource(model, "ResourceStableAccounting");

    ASSERT_TRUE(resource.seize(1u));
    resource.release(1u);
    expectAllAccountingData(&resource);

    ModelDataDefinition* timeSeized = resource.getInternalData("TimeSeized");
    ModelDataDefinition* totalTimeSeized = resource.getInternalData("TotalTimeSeized");
    ModelDataDefinition* costBusy = resource.getInternalData("CostBusy");

    ASSERT_TRUE(resource.seize(1u));
    resource.release(1u);

    EXPECT_EQ(resource.getInternalData("TimeSeized"), timeSeized);
    EXPECT_EQ(resource.getInternalData("TotalTimeSeized"), totalTimeSeized);
    EXPECT_EQ(resource.getInternalData("CostBusy"), costBusy);
}
