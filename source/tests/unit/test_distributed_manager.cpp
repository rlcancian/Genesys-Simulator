#include <gtest/gtest.h>

#include "applications/distributed/core/DistributedSimulationManager.h"

#include <string>

using namespace genesys::distributed;

namespace {

// Simple model that parses with the minimal plugin set (Create/Dispose from Logic).
const std::string kSimpleModel =
    "0   ModelInfo  \"M\" 0   ModelSimulation \"\" replicationLength=10 numberOfReplications=2 "
    "63  Create \"Create_1\" nextId=73 73  Dispose \"Dispose_1\" nexts=0 ";

} // namespace

TEST(DistributedSimulationManager, ShouldRunLocallyWhenNoRemoteWorkers) {
    // Arrange: no remote endpoints, local execution enabled.
    DistributedSimulationManager manager;
    DistributedSimulationConfig config;
    config.modelText = kSimpleModel;
    config.totalReplications = 6;
    config.includeLocal = true;

    // Act
    const AggregatedResult result = manager.run(config);

    // Assert: all replications ran locally and were aggregated, no failures.
    EXPECT_EQ(result.totalReplicationsRequested, 6u);
    EXPECT_EQ(result.totalReplicationsCompleted, 6u);
    EXPECT_TRUE(result.failures.empty());
}

TEST(DistributedSimulationManager, ShouldReportFailureWhenNoTargetsAvailable) {
    // Arrange: unreachable worker and local disabled.
    DistributedSimulationManager manager;
    DistributedSimulationConfig config;
    config.modelText = kSimpleModel;
    config.totalReplications = 4;
    config.includeLocal = false;
    config.workers = {{"127.0.0.1", 9}};  // discard port: unreachable.
    config.discoveryTimeoutSeconds = 2;

    // Act
    const AggregatedResult result = manager.run(config);

    // Assert: nothing ran; the failure is reported and requested count preserved.
    EXPECT_EQ(result.totalReplicationsRequested, 4u);
    EXPECT_EQ(result.totalReplicationsCompleted, 0u);
    EXPECT_FALSE(result.failures.empty());
}
