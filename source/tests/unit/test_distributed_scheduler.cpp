#include <gtest/gtest.h>

#include "applications/distributed/core/DistributedScheduler.h"

#include <numeric>
#include <set>

using namespace genesys::distributed;

namespace {

unsigned int totalAssigned(const std::vector<ReplicationBatch>& batches) {
    unsigned int sum = 0;
    for (const auto& batch : batches) {
        sum += batch.numberOfReplications;
    }
    return sum;
}

std::vector<SchedulerTarget> remoteTargets(int count) {
    std::vector<SchedulerTarget> targets;
    for (int i = 0; i < count; ++i) {
        targets.push_back({"host-" + std::to_string(i) + ":8080", false});
    }
    return targets;
}

} // namespace

TEST(DistributedScheduler, ShouldSplitEvenlyWhenDivisible) {
    // Arrange
    DistributedScheduler scheduler;

    // Act
    const auto batches = scheduler.partition(9, remoteTargets(3));

    // Assert
    ASSERT_EQ(batches.size(), 3u);
    for (const auto& batch : batches) {
        EXPECT_EQ(batch.numberOfReplications, 3u);
    }
    EXPECT_EQ(totalAssigned(batches), 9u);
}

TEST(DistributedScheduler, ShouldDistributeRemainderToFirstTargets) {
    // Arrange
    DistributedScheduler scheduler;

    // Act: 10 over 3 -> 4,3,3
    const auto batches = scheduler.partition(10, remoteTargets(3));

    // Assert
    ASSERT_EQ(batches.size(), 3u);
    EXPECT_EQ(batches[0].numberOfReplications, 4u);
    EXPECT_EQ(batches[1].numberOfReplications, 3u);
    EXPECT_EQ(batches[2].numberOfReplications, 3u);
    EXPECT_EQ(totalAssigned(batches), 10u);
}

TEST(DistributedScheduler, ShouldSumToTotalAcrossManyConfigurations) {
    // Arrange
    DistributedScheduler scheduler;

    // Act + Assert: the assigned sum always equals the requested total.
    for (unsigned int total = 0; total <= 50; ++total) {
        for (int targets = 1; targets <= 7; ++targets) {
            const auto batches = scheduler.partition(total, remoteTargets(targets));
            EXPECT_EQ(totalAssigned(batches), total) << "total=" << total << " targets=" << targets;
        }
    }
}

TEST(DistributedScheduler, ShouldOmitEmptyBatchesWhenFewerReplicationsThanTargets) {
    // Arrange
    DistributedScheduler scheduler;

    // Act: 2 replications over 5 targets -> two batches of 1, three omitted.
    const auto batches = scheduler.partition(2, remoteTargets(5));

    // Assert
    ASSERT_EQ(batches.size(), 2u);
    EXPECT_EQ(batches[0].numberOfReplications, 1u);
    EXPECT_EQ(batches[1].numberOfReplications, 1u);
    EXPECT_EQ(totalAssigned(batches), 2u);
}

TEST(DistributedScheduler, ShouldAssignDistinctSeedsPerBatch) {
    // Arrange
    DistributedScheduler scheduler;

    // Act
    const auto batches = scheduler.partition(12, remoteTargets(4), 1000u);

    // Assert
    std::set<std::uint32_t> seeds;
    for (const auto& batch : batches) {
        seeds.insert(batch.seed);
    }
    EXPECT_EQ(seeds.size(), batches.size());
    EXPECT_EQ(batches[0].seed, 1000u);
    EXPECT_EQ(batches[1].seed, 1001u);
}

TEST(DistributedScheduler, ShouldIncludeLocalTargetInPartition) {
    // Arrange
    DistributedScheduler scheduler;
    std::vector<SchedulerTarget> targets = {
        {"host-0:8080", false},
        {"local", true},
    };

    // Act
    const auto batches = scheduler.partition(6, targets);

    // Assert: local participates as a normal target and the sum is preserved.
    ASSERT_EQ(batches.size(), 2u);
    EXPECT_EQ(totalAssigned(batches), 6u);
    bool sawLocal = false;
    for (const auto& batch : batches) {
        if (batch.isLocal) {
            sawLocal = true;
            EXPECT_EQ(batch.targetEndpoint, "local");
        }
    }
    EXPECT_TRUE(sawLocal);
}

TEST(DistributedScheduler, ShouldReturnEmptyWhenNoTargetsOrNoReplications) {
    // Arrange
    DistributedScheduler scheduler;

    // Act + Assert
    EXPECT_TRUE(scheduler.partition(10, {}).empty());
    EXPECT_TRUE(scheduler.partition(0, remoteTargets(3)).empty());
}
