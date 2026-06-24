// Integration-level equivalence test for the distributed layer (AC-07a / AC-07b).
//
// Scope note: the exact statistical equivalence of the pooled aggregation is proven
// numerically in test_distributed_aggregator.cpp (pooled moments == statistics over the
// combined values, to full precision). This test validates the end-to-end orchestration
// coherence: splitting N replications into batches, running them through the real local
// simulator, and aggregating yields a result coherent with a single monolithic run of N
// (same completed-replication count and same statistics structure), and is reproducible.
//
// Component-level statistics are not exercised here because, in this static build, the
// worker/local engine does not register named component plugins (no dynamic plugin
// connector), so a language model parses with zero components. The simple model below runs
// the engine and the full pipeline without relying on component statistics.

#include <gtest/gtest.h>

#include "applications/distributed/core/DistributedExecutionManager.h"
#include "applications/distributed/core/DistributedResultsAggregator.h"
#include "applications/distributed/core/DistributedScheduler.h"
#include "applications/distributed/core/LocalSimulationExecutor.h"

#include <string>
#include <vector>

using namespace genesys::distributed;

namespace {

const std::string kSimpleModel =
    "0   ModelInfo  \"M\" 0   ModelSimulation \"\" replicationLength=10 numberOfReplications=2 "
    "63  Create \"Create_1\" nextId=73 73  Dispose \"Dispose_1\" nexts=0 ";

// Runs each batch sequentially through a local executor and aggregates the outcomes.
AggregatedResult runDistributedLocally(const std::string& model,
                                       unsigned int total,
                                       int targetCount,
                                       std::uint32_t baseSeed) {
    std::vector<SchedulerTarget> targets;
    for (int i = 0; i < targetCount; ++i) {
        targets.push_back({"local-" + std::to_string(i), true});
    }

    DistributedScheduler scheduler;
    const std::vector<ReplicationBatch> batches = scheduler.partition(total, targets, baseSeed);

    LocalSimulationExecutor executor;
    std::vector<BatchExecution> outcomes;
    for (const ReplicationBatch& batch : batches) {
        DistributedSimulationJob job{model, batch};
        BatchExecution outcome;
        outcome.batch = batch;
        outcome.result = executor.execute(job);
        outcome.ranOn = batch.targetEndpoint;
        outcomes.push_back(outcome);
    }

    DistributedResultsAggregator aggregator;
    return aggregator.aggregate(outcomes);
}

} // namespace

TEST(DistributedEquivalence, ShouldMatchMonolithicReplicationCountWhenSplit) {
    // Arrange: monolithic run of 12 replications.
    LocalSimulationExecutor monolithic;
    DistributedSimulationJob monoJob{kSimpleModel, {}};
    monoJob.batch.numberOfReplications = 12;
    monoJob.batch.seed = DistributedScheduler::kDefaultBaseSeed;
    const BatchResult monoResult = monolithic.execute(monoJob);
    ASSERT_TRUE(monoResult.success) << monoResult.error;

    // Act: distributed run of the same total split across 3 batches.
    const AggregatedResult distributed = runDistributedLocally(kSimpleModel, 12, 3, 1000u);

    // Assert: coherent replication accounting and matching statistics structure.
    EXPECT_EQ(distributed.totalReplicationsRequested, 12u);
    EXPECT_EQ(distributed.totalReplicationsCompleted, 12u);
    EXPECT_EQ(distributed.totalReplicationsCompleted, monoResult.numberOfReplications);
    EXPECT_TRUE(distributed.failures.empty());
    EXPECT_EQ(distributed.statistics.size(), monoResult.statistics.size());
    EXPECT_EQ(distributed.counters.size(), monoResult.counters.size());
}

TEST(DistributedEquivalence, ShouldSumBatchesToTotalForVariousSplits) {
    // Act + Assert: regardless of how N is split, the completed total is preserved.
    for (int targets = 1; targets <= 5; ++targets) {
        const AggregatedResult result = runDistributedLocally(kSimpleModel, 10, targets, 500u);
        EXPECT_EQ(result.totalReplicationsRequested, 10u) << "targets=" << targets;
        EXPECT_EQ(result.totalReplicationsCompleted, 10u) << "targets=" << targets;
        EXPECT_TRUE(result.failures.empty()) << "targets=" << targets;
    }
}

TEST(DistributedEquivalence, ShouldBeReproducibleAcrossRuns) {
    // Act: same configuration twice.
    const AggregatedResult first = runDistributedLocally(kSimpleModel, 9, 3, 2024u);
    const AggregatedResult second = runDistributedLocally(kSimpleModel, 9, 3, 2024u);

    // Assert: identical aggregated outcome (deterministic given the same seeds).
    EXPECT_EQ(first.totalReplicationsCompleted, second.totalReplicationsCompleted);
    ASSERT_EQ(first.statistics.size(), second.statistics.size());
    for (std::size_t i = 0; i < first.statistics.size(); ++i) {
        EXPECT_EQ(first.statistics[i].name, second.statistics[i].name);
        EXPECT_DOUBLE_EQ(first.statistics[i].average, second.statistics[i].average);
        EXPECT_DOUBLE_EQ(first.statistics[i].variance, second.statistics[i].variance);
    }
}
