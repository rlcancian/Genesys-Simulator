#include <gtest/gtest.h>

#include "applications/distributed/core/LocalSimulationExecutor.h"

#include <string>

using namespace genesys::distributed;

namespace {

// A simple model that parses with the minimal plugin set (Create/Dispose from Logic).
const std::string kSimpleModel =
    "0   ModelInfo  \"M\" 0   ModelSimulation \"\" replicationLength=10 numberOfReplications=2 "
    "63  Create \"Create_1\" nextId=73 73  Dispose \"Dispose_1\" nexts=0 ";

DistributedSimulationJob makeJob(unsigned int replications, std::uint32_t seed) {
    DistributedSimulationJob job;
    job.modelText = kSimpleModel;
    job.batch.numberOfReplications = replications;
    job.batch.seed = seed;
    return job;
}

} // namespace

TEST(LocalSimulationExecutor, ShouldRunBatchWithRequestedReplications) {
    // Arrange
    LocalSimulationExecutor executor;

    // Act: the batch overrides the model's declared replication count (2 -> 5).
    const BatchResult result = executor.execute(makeJob(5, 42));

    // Assert
    ASSERT_TRUE(result.success) << result.error;
    EXPECT_EQ(result.numberOfReplications, 5u);
}

TEST(LocalSimulationExecutor, ShouldFailGracefullyOnUnparseableModel) {
    // Arrange
    LocalSimulationExecutor executor;
    DistributedSimulationJob job;
    job.modelText = "this is not a valid model";
    job.batch.numberOfReplications = 3;
    job.batch.seed = 1;

    // Act
    const BatchResult result = executor.execute(job);

    // Assert
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error.empty());
}

TEST(LocalSimulationExecutor, ShouldProduceStableResultForSameSeed) {
    // Arrange
    LocalSimulationExecutor executor;

    // Act: two runs with identical model, replication count and seed.
    const BatchResult first = executor.execute(makeJob(4, 7));
    const BatchResult second = executor.execute(makeJob(4, 7));

    // Assert: same structure and values across runs (reproducible).
    ASSERT_TRUE(first.success) << first.error;
    ASSERT_TRUE(second.success) << second.error;
    EXPECT_EQ(first.numberOfReplications, second.numberOfReplications);
    ASSERT_EQ(first.statistics.size(), second.statistics.size());
    for (std::size_t i = 0; i < first.statistics.size(); ++i) {
        EXPECT_EQ(first.statistics[i].name, second.statistics[i].name);
        EXPECT_DOUBLE_EQ(first.statistics[i].average, second.statistics[i].average);
        EXPECT_DOUBLE_EQ(first.statistics[i].variance, second.statistics[i].variance);
    }
    EXPECT_EQ(first.counters.size(), second.counters.size());
}
