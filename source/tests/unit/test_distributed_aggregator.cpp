#include <gtest/gtest.h>

#include "applications/distributed/core/DistributedResultsAggregator.h"

#include <cmath>

using namespace genesys::distributed;

namespace {

BatchExecution successfulBatch(unsigned int seed,
                               unsigned int replications,
                               const std::vector<CollectorStat>& statistics,
                               const std::vector<CounterStat>& counters = {}) {
    BatchExecution outcome;
    outcome.batch.seed = seed;
    outcome.batch.numberOfReplications = replications;
    outcome.result.success = true;
    outcome.result.numberOfReplications = replications;
    outcome.result.statistics = statistics;
    outcome.result.counters = counters;
    outcome.ranOn = "worker";
    return outcome;
}

BatchExecution lostBatch(unsigned int seed, unsigned int replications) {
    BatchExecution outcome;
    outcome.batch.seed = seed;
    outcome.batch.numberOfReplications = replications;
    outcome.result.success = false;
    outcome.lost = true;
    return outcome;
}

BatchExecution rejectedBatch(unsigned int seed, unsigned int replications, const std::string& error) {
    BatchExecution outcome;
    outcome.batch.seed = seed;
    outcome.batch.numberOfReplications = replications;
    outcome.result.success = false;
    outcome.result.failureKind = FailureKind::ModelRejected;
    outcome.result.error = error;
    outcome.lost = true;
    return outcome;
}

CollectorStat stat(const std::string& name,
                    unsigned int n,
                    double average,
                    double variance,
                    double min,
                    double max) {
    return CollectorStat{name, n, average, variance, min, max, n};
}

} // namespace

TEST(DistributedResultsAggregator, ShouldPoolStatisticsExactlyLikeCombinedValues) {
    // Arrange: combined values [1,2,3,4] split into two batches [1,2] and [3,4].
    // batch A: n=2, mean=1.5, sample var=0.5, min=1, max=2
    // batch B: n=2, mean=3.5, sample var=0.5, min=3, max=4
    DistributedResultsAggregator aggregator;
    const std::vector<BatchExecution> outcomes = {
        successfulBatch(1, 2, {stat("X", 2, 1.5, 0.5, 1.0, 2.0)}),
        successfulBatch(2, 2, {stat("X", 2, 3.5, 0.5, 3.0, 4.0)}),
    };

    // Act
    const AggregatedResult result = aggregator.aggregate(outcomes);

    // Assert: equals statistics computed directly over [1,2,3,4].
    ASSERT_EQ(result.statistics.size(), 1u);
    const AggregatedStatistic& x = result.statistics[0];
    EXPECT_EQ(x.name, "X");
    EXPECT_EQ(x.numReplications, 4u);
    EXPECT_DOUBLE_EQ(x.average, 2.5);
    EXPECT_DOUBLE_EQ(x.variance, 5.0 / 3.0);  // sample variance of [1,2,3,4]
    EXPECT_DOUBLE_EQ(x.min, 1.0);
    EXPECT_DOUBLE_EQ(x.max, 4.0);
    EXPECT_DOUBLE_EQ(x.stddev, std::sqrt(5.0 / 3.0));
    EXPECT_DOUBLE_EQ(x.halfWidthConfidenceInterval, 1.96 * std::sqrt(5.0 / 3.0) / std::sqrt(4.0));
}

TEST(DistributedResultsAggregator, ShouldSumCountersAcrossBatches) {
    // Arrange
    DistributedResultsAggregator aggregator;
    const std::vector<BatchExecution> outcomes = {
        successfulBatch(1, 5, {}, {CounterStat{"Out", 100.0}}),
        successfulBatch(2, 5, {}, {CounterStat{"Out", 50.0}}),
    };

    // Act
    const AggregatedResult result = aggregator.aggregate(outcomes);

    // Assert
    ASSERT_EQ(result.counters.size(), 1u);
    EXPECT_EQ(result.counters[0].name, "Out");
    EXPECT_DOUBLE_EQ(result.counters[0].total, 150.0);
}

TEST(DistributedResultsAggregator, ShouldReportRequestedCompletedAndFailuresForLostBatches) {
    // Arrange: two successful (5 reps each) and one lost (5 reps).
    DistributedResultsAggregator aggregator;
    const std::vector<BatchExecution> outcomes = {
        successfulBatch(1, 5, {stat("X", 5, 1.0, 0.0, 1.0, 1.0)}),
        successfulBatch(2, 5, {stat("X", 5, 1.0, 0.0, 1.0, 1.0)}),
        lostBatch(3, 5),
    };

    // Act
    const AggregatedResult result = aggregator.aggregate(outcomes);

    // Assert
    EXPECT_EQ(result.totalReplicationsRequested, 15u);
    EXPECT_EQ(result.totalReplicationsCompleted, 10u);
    ASSERT_EQ(result.failures.size(), 1u);
    EXPECT_NE(result.failures[0].find("seed=3"), std::string::npos);
    // The lost batch contributed no replications to the aggregated statistic.
    ASSERT_EQ(result.statistics.size(), 1u);
    EXPECT_EQ(result.statistics[0].numReplications, 10u);
}

TEST(DistributedResultsAggregator, ShouldReportRealErrorAndRejectedLabelForModelRejectedBatches) {
    // Arrange: a batch the worker refused because the model itself was invalid.
    DistributedResultsAggregator aggregator;
    const std::vector<BatchExecution> outcomes = {
        rejectedBatch(7, 5, "model import returned status 400: INVALID_MODEL_SPECIFICATION"),
    };

    // Act
    const AggregatedResult result = aggregator.aggregate(outcomes);

    // Assert: the failure is labelled "rejected" and carries the worker's real reason.
    EXPECT_EQ(result.totalReplicationsRequested, 5u);
    EXPECT_EQ(result.totalReplicationsCompleted, 0u);
    ASSERT_EQ(result.failures.size(), 1u);
    EXPECT_NE(result.failures[0].find("rejected"), std::string::npos);
    EXPECT_NE(result.failures[0].find("INVALID_MODEL_SPECIFICATION"), std::string::npos);
}

TEST(DistributedResultsAggregator, ShouldHandleSingleReplicationBatchesWithoutCrashing) {
    // Arrange: two batches of one replication each (variance undefined within a batch -> 0).
    DistributedResultsAggregator aggregator;
    const std::vector<BatchExecution> outcomes = {
        successfulBatch(1, 1, {stat("X", 1, 2.0, 0.0, 2.0, 2.0)}),
        successfulBatch(2, 1, {stat("X", 1, 4.0, 0.0, 4.0, 4.0)}),
    };

    // Act
    const AggregatedResult result = aggregator.aggregate(outcomes);

    // Assert: combined [2,4] -> mean 3, sample variance 2.
    ASSERT_EQ(result.statistics.size(), 1u);
    EXPECT_EQ(result.statistics[0].numReplications, 2u);
    EXPECT_DOUBLE_EQ(result.statistics[0].average, 3.0);
    EXPECT_DOUBLE_EQ(result.statistics[0].variance, 2.0);
}
