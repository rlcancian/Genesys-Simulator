// Integration-level equivalence test for the distributed layer (AC-07a / AC-07b).
//
// Scope note: the exact statistical equivalence of the pooled aggregation is proven
// numerically in test_distributed_aggregator.cpp (pooled moments == statistics over the
// combined values, to full precision). This test validates the end-to-end orchestration
// coherence: splitting N replications into batches, running them through the real local
// simulator, and aggregating yields a result coherent with a single monolithic run of N
// (same completed-replication count and same statistics), and is reproducible.
//
// The model below is multi-line (one record per line, as the .gen parser requires) and runs a
// real Create -> Delay -> Dispose pipeline, so the engine produces actual component statistics
// (e.g. the entity TotalTimeInSystem) and counters — these are exercised end to end here, since
// the local/worker engine now registers the built-in component plugins via autoInsertPlugins().

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
    "0   ModelInfo  \"M\"\n"
    "0   ModelSimulation \"\" replicationLength=20 numberOfReplications=2\n"
    "62  EntityType \"Part\"\n"
    "61  Create     \"Create_1\" entityType=\"Part\" nextId=64 timeBetweenCreations=\"norm(1.5,0.5)\"\n"
    "64  Delay      \"Delay_1\" delayExpression=\"norm(1.0,0.2)\" nextId=63\n"
    "63  Dispose    \"Dispose_1\" nexts=0\n";

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
    // The real Create -> Delay -> Dispose model must actually produce statistics/counters,
    // otherwise the structural assertions above would be vacuously true.
    EXPECT_FALSE(distributed.statistics.empty());
    EXPECT_FALSE(distributed.counters.empty());
}

TEST(DistributedEquivalence, ShouldReproduceMonolithicStatisticsExactlyForIdentityPartition) {
    // The strongest coherence check (AC-07a): for the identity partition (a single target with the
    // same base seed), routing N replications through scheduler + executor + aggregator must yield
    // EXACTLY the kernel's own statistics — the orchestration/aggregation layer adds no distortion.
    const unsigned int kReplications = 8;
    const std::uint32_t kSeed = DistributedScheduler::kDefaultBaseSeed;

    // Arrange: monolithic run of N with the base seed.
    LocalSimulationExecutor monolithic;
    DistributedSimulationJob monoJob{kSimpleModel, {}};
    monoJob.batch.numberOfReplications = kReplications;
    monoJob.batch.seed = kSeed;
    const BatchResult mono = monolithic.execute(monoJob);
    ASSERT_TRUE(mono.success) << mono.error;
    ASSERT_FALSE(mono.statistics.empty());

    // Act: distributed run over a single local target with the same base seed (-> one batch, seed kSeed).
    const AggregatedResult distributed = runDistributedLocally(kSimpleModel, kReplications, 1, kSeed);

    // The aggregator emits statistics name-sorted; the monolithic capture keeps engine order. Compare
    // by name so the test does not depend on ordering.
    ASSERT_EQ(distributed.statistics.size(), mono.statistics.size());
    for (const AggregatedStatistic& d : distributed.statistics) {
        const CollectorStat* m = nullptr;
        for (const CollectorStat& candidate : mono.statistics) {
            if (candidate.name == d.name) { m = &candidate; break; }
        }
        ASSERT_NE(m, nullptr) << "collector '" << d.name << "' missing from the monolithic result";
        EXPECT_EQ(d.numReplications, m->numReplications);
        EXPECT_DOUBLE_EQ(d.average, m->average);  // mean = (avg*n)/n is exact for n a power of two.
        EXPECT_DOUBLE_EQ(d.min, m->min);          // min/max pass through untouched.
        EXPECT_DOUBLE_EQ(d.max, m->max);
        // Variance is reconstructed from pooled moments (a subtraction with rounding), so allow a
        // tiny numerical tolerance rather than bit-exactness.
        EXPECT_NEAR(d.variance, m->variance, 1e-9);
    }
    ASSERT_EQ(distributed.counters.size(), mono.counters.size());
    for (const AggregatedCounter& d : distributed.counters) {
        const CounterStat* m = nullptr;
        for (const CounterStat& candidate : mono.counters) {
            if (candidate.name == d.name) { m = &candidate; break; }
        }
        ASSERT_NE(m, nullptr) << "counter '" << d.name << "' missing from the monolithic result";
        EXPECT_DOUBLE_EQ(d.total, m->total);
    }
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
