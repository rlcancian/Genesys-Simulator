#include <gtest/gtest.h>

#include "applications/distributed/core/WorkerRegistry.h"

using namespace genesys::distributed;

namespace {

WorkerDescriptor makeWorker(const std::string& host, int port) {
    WorkerDescriptor descriptor;
    descriptor.host = host;
    descriptor.port = port;
    descriptor.role = "worker";
    descriptor.capabilities.supportsDistributedJobs = true;
    descriptor.capabilities.supportsJobResultRetrieval = true;
    return descriptor;
}

} // namespace

TEST(WorkerRegistry, ShouldStoreAndFindWorkerByEndpoint) {
    // Arrange
    WorkerRegistry registry;
    registry.upsert(makeWorker("127.0.0.1", 8080));

    // Act
    const auto found = registry.find("127.0.0.1", 8080);

    // Assert
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->endpoint(), "127.0.0.1:8080");
    EXPECT_EQ(registry.size(), 1u);
}

TEST(WorkerRegistry, ShouldReplaceDescriptorWhenUpsertingSameEndpoint) {
    // Arrange
    WorkerRegistry registry;
    registry.upsert(makeWorker("host", 1));
    WorkerDescriptor updated = makeWorker("host", 1);
    updated.simulatorName = "updated";

    // Act
    registry.upsert(updated);

    // Assert
    EXPECT_EQ(registry.size(), 1u);
    EXPECT_EQ(registry.find("host", 1)->simulatorName, "updated");
}

TEST(WorkerRegistry, ShouldListOnlyAvailableWorkers) {
    // Arrange
    WorkerRegistry registry;
    registry.upsert(makeWorker("a", 1));
    registry.upsert(makeWorker("b", 2));
    registry.upsert(makeWorker("c", 3));

    // Act
    registry.markAvailable("a", 1, 12);
    registry.markAvailable("c", 3, 34);
    registry.markUnavailable("b", 2, "timeout");

    // Assert
    const auto available = registry.available();
    EXPECT_EQ(available.size(), 2u);
    for (const auto& worker : available) {
        EXPECT_EQ(worker.state, WorkerState::Available);
        EXPECT_NE(worker.endpoint(), "b:2");
    }
}

TEST(WorkerRegistry, ShouldRecordLatencyWhenMarkedAvailable) {
    // Arrange
    WorkerRegistry registry;
    registry.upsert(makeWorker("a", 1));

    // Act
    registry.markAvailable("a", 1, 42);

    // Assert
    const auto worker = registry.find("a", 1);
    ASSERT_TRUE(worker.has_value());
    EXPECT_EQ(worker->state, WorkerState::Available);
    EXPECT_EQ(worker->observedLatencyMs, 42);
}

TEST(WorkerRegistry, ShouldAccumulateFailuresAndStoreLastError) {
    // Arrange
    WorkerRegistry registry;
    registry.upsert(makeWorker("a", 1));

    // Act
    registry.markUnavailable("a", 1, "connect refused");
    registry.markUnavailable("a", 1, "read timeout");

    // Assert
    const auto worker = registry.find("a", 1);
    ASSERT_TRUE(worker.has_value());
    EXPECT_EQ(worker->state, WorkerState::Unavailable);
    EXPECT_EQ(worker->failureCount, 2u);
    EXPECT_EQ(worker->lastError, "read timeout");
}

TEST(WorkerRegistry, ShouldReturnFalseWhenMarkingUnknownWorker) {
    // Arrange
    WorkerRegistry registry;

    // Act + Assert
    EXPECT_FALSE(registry.markAvailable("ghost", 1, 1));
    EXPECT_FALSE(registry.markUnavailable("ghost", 1, "x"));
    EXPECT_FALSE(registry.find("ghost", 1).has_value());
}
