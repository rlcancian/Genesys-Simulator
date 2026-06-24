#include <gtest/gtest.h>

#include "applications/distributed/core/CoordinatorApplication.h"
#include "applications/distributed/core/Json.h"

using namespace genesys::distributed;

namespace {

AggregatedResult sampleResult() {
    AggregatedResult result;
    result.totalReplicationsRequested = 30;
    result.totalReplicationsCompleted = 25;
    AggregatedStatistic stat;
    stat.name = "Queue.Wait";
    stat.numReplications = 25;
    stat.average = 2.5;
    stat.variance = 0.25;
    stat.stddev = 0.5;
    stat.min = 1.0;
    stat.max = 4.0;
    stat.halfWidthConfidenceInterval = 0.196;
    stat.confidenceLevel = 0.95;
    result.statistics.push_back(stat);
    result.counters.push_back({"Out.Count", 1234.0});
    result.failures.push_back("batch (seed=3, replications=5) lost");
    return result;
}

} // namespace

TEST(CoordinatorApplication, ShouldRenderJsonThatRoundTrips) {
    // Arrange
    CoordinatorApplication app;
    const AggregatedResult result = sampleResult();

    // Act
    const std::string json = app.renderJson(result);

    // Assert: structured fields are parseable back to the same values.
    EXPECT_EQ(json::getInt(json, "totalReplicationsRequested").value_or(0), 30);
    EXPECT_EQ(json::getInt(json, "totalReplicationsCompleted").value_or(0), 25);

    const auto statsArray = json::getArray(json, "statistics");
    ASSERT_TRUE(statsArray.has_value());
    const auto statObjects = json::splitObjects(statsArray.value());
    ASSERT_EQ(statObjects.size(), 1u);
    EXPECT_EQ(json::getString(statObjects[0], "name").value_or(""), "Queue.Wait");
    EXPECT_DOUBLE_EQ(json::getDouble(statObjects[0], "average").value_or(0.0), 2.5);
    EXPECT_DOUBLE_EQ(json::getDouble(statObjects[0], "stddev").value_or(0.0), 0.5);

    const auto countersArray = json::getArray(json, "counters");
    ASSERT_TRUE(countersArray.has_value());
    const auto counterObjects = json::splitObjects(countersArray.value());
    ASSERT_EQ(counterObjects.size(), 1u);
    EXPECT_DOUBLE_EQ(json::getDouble(counterObjects[0], "total").value_or(0.0), 1234.0);
}

TEST(CoordinatorApplication, ShouldRenderHumanReadableSummary) {
    // Arrange
    CoordinatorApplication app;
    const AggregatedResult result = sampleResult();

    // Act
    const std::string summary = app.renderSummary(result);

    // Assert: key facts appear in the text.
    EXPECT_NE(summary.find("25 completed / 30 requested"), std::string::npos);
    EXPECT_NE(summary.find("Queue.Wait"), std::string::npos);
    EXPECT_NE(summary.find("Out.Count"), std::string::npos);
    EXPECT_NE(summary.find("Failures"), std::string::npos);
}
