#include <gtest/gtest.h>

#include "applications/distributed/core/DistributedExecutionManager.h"
#include "applications/distributed/core/WorkerRegistry.h"

#include <map>
#include <string>

using namespace genesys::distributed;

namespace {

// Executor that succeeds or fails based on a flag, recording how many times it ran.
class StubExecutor : public SimulationExecutor {
public:
    explicit StubExecutor(bool succeeds, double markerAverage = 0.0,
                          FailureKind failureKind = FailureKind::WorkerUnavailable,
                          std::string error = "stub failure")
        : _succeeds(succeeds),
          _markerAverage(markerAverage),
          _failureKind(failureKind),
          _error(std::move(error)) {}

    BatchResult execute(const DistributedSimulationJob& job) override {
        ++runCount;
        BatchResult result;
        if (!_succeeds) {
            result.success = false;
            result.failureKind = _failureKind;
            result.error = _error;
            return result;
        }
        result.success = true;
        result.numberOfReplications = job.batch.numberOfReplications;
        CollectorStat stat;
        stat.name = "marker";
        stat.numReplications = job.batch.numberOfReplications;
        stat.average = _markerAverage;
        result.statistics.push_back(stat);
        return result;
    }

    int runCount = 0;

private:
    bool _succeeds;
    double _markerAverage;
    FailureKind _failureKind;
    std::string _error;
};

DistributedSimulationJob job(const std::string& target, unsigned int replications, std::uint32_t seed) {
    DistributedSimulationJob j;
    j.modelText = "model";
    j.batch.targetEndpoint = target;
    j.batch.numberOfReplications = replications;
    j.batch.seed = seed;
    return j;
}

WorkerDescriptor availableWorker(const std::string& host, int port) {
    WorkerDescriptor descriptor;
    descriptor.host = host;
    descriptor.port = port;
    descriptor.state = WorkerState::Available;
    return descriptor;
}

} // namespace

TEST(DistributedExecutionManager, ShouldRunAllBatchesOnAssignedTargetsWhenHealthy) {
    // Arrange
    StubExecutor a(true);
    StubExecutor b(true);
    WorkerRegistry registry;
    registry.upsert(availableWorker("host-a", 1));
    registry.upsert(availableWorker("host-b", 2));
    std::map<std::string, SimulationExecutor*> executors = {{"host-a:1", &a}, {"host-b:2", &b}};
    DistributedExecutionManager manager(executors, registry, 2);

    // Act
    const auto outcomes = manager.execute({job("host-a:1", 5, 10), job("host-b:2", 5, 20)});

    // Assert
    ASSERT_EQ(outcomes.size(), 2u);
    EXPECT_TRUE(outcomes[0].result.success);
    EXPECT_EQ(outcomes[0].ranOn, "host-a:1");
    EXPECT_TRUE(outcomes[1].result.success);
    EXPECT_EQ(outcomes[1].ranOn, "host-b:2");
    EXPECT_FALSE(outcomes[0].lost);
}

TEST(DistributedExecutionManager, ShouldFailoverToAnotherTargetWhenAssignedFails) {
    // Arrange: the assigned worker fails; a healthy alternative exists.
    StubExecutor failing(false);
    StubExecutor healthy(true);
    WorkerRegistry registry;
    registry.upsert(availableWorker("bad", 1));
    registry.upsert(availableWorker("good", 2));
    std::map<std::string, SimulationExecutor*> executors = {{"bad:1", &failing}, {"good:2", &healthy}};
    DistributedExecutionManager manager(executors, registry, 2);

    // Act
    const auto outcomes = manager.execute({job("bad:1", 5, 10)});

    // Assert: the batch succeeded on the alternative; failed worker marked unavailable.
    ASSERT_EQ(outcomes.size(), 1u);
    EXPECT_TRUE(outcomes[0].result.success);
    EXPECT_EQ(outcomes[0].ranOn, "good:2");
    EXPECT_EQ(registry.find("bad", 1)->state, WorkerState::Unavailable);
    EXPECT_EQ(outcomes[0].failedTargets.size(), 1u);
}

TEST(DistributedExecutionManager, ShouldMarkBatchLostWhenAllTargetsFail) {
    // Arrange: every target fails.
    StubExecutor a(false);
    StubExecutor b(false);
    WorkerRegistry registry;
    registry.upsert(availableWorker("a", 1));
    registry.upsert(availableWorker("b", 2));
    std::map<std::string, SimulationExecutor*> executors = {{"a:1", &a}, {"b:2", &b}};
    DistributedExecutionManager manager(executors, registry, 5);

    // Act
    const auto outcomes = manager.execute({job("a:1", 5, 10)});

    // Assert: lost, no successful result, both targets marked unavailable.
    ASSERT_EQ(outcomes.size(), 1u);
    EXPECT_FALSE(outcomes[0].result.success);
    EXPECT_TRUE(outcomes[0].lost);
    EXPECT_TRUE(outcomes[0].ranOn.empty());
    EXPECT_EQ(registry.find("a", 1)->state, WorkerState::Unavailable);
    EXPECT_EQ(registry.find("b", 2)->state, WorkerState::Unavailable);
}

TEST(DistributedExecutionManager, ShouldNotDoubleCountWhenRetrying) {
    // Arrange: assigned fails, alternative succeeds with a known marker average.
    StubExecutor failing(false);
    StubExecutor healthy(true, 3.5);
    WorkerRegistry registry;
    registry.upsert(availableWorker("bad", 1));
    registry.upsert(availableWorker("good", 2));
    std::map<std::string, SimulationExecutor*> executors = {{"bad:1", &failing}, {"good:2", &healthy}};
    DistributedExecutionManager manager(executors, registry, 2);

    // Act
    const auto outcomes = manager.execute({job("bad:1", 5, 10)});

    // Assert: only the successful attempt's statistics are present (no accumulation of the failure).
    ASSERT_TRUE(outcomes[0].result.success);
    ASSERT_EQ(outcomes[0].result.statistics.size(), 1u);
    EXPECT_DOUBLE_EQ(outcomes[0].result.statistics[0].average, 3.5);
}

TEST(DistributedExecutionManager, ShouldNotMarkWorkerUnavailableWhenModelRejected) {
    // Arrange: both targets reject the model itself (a healthy worker, an invalid model).
    StubExecutor a(false, 0.0, FailureKind::ModelRejected, "invalid model specification");
    StubExecutor b(false, 0.0, FailureKind::ModelRejected, "invalid model specification");
    WorkerRegistry registry;
    registry.upsert(availableWorker("a", 1));
    registry.upsert(availableWorker("b", 2));
    std::map<std::string, SimulationExecutor*> executors = {{"a:1", &a}, {"b:2", &b}};
    DistributedExecutionManager manager(executors, registry, 5);

    // Act
    const auto outcomes = manager.execute({job("a:1", 5, 10)});

    // Assert: batch lost, but the worker stays Available and the real error is preserved.
    ASSERT_EQ(outcomes.size(), 1u);
    EXPECT_TRUE(outcomes[0].lost);
    EXPECT_EQ(outcomes[0].result.failureKind, FailureKind::ModelRejected);
    EXPECT_EQ(outcomes[0].result.error, "invalid model specification");
    EXPECT_EQ(registry.find("a", 1)->state, WorkerState::Available);
}

TEST(DistributedExecutionManager, ShouldNotFailoverWhenModelRejected) {
    // Arrange: the assigned target rejects the model; an otherwise healthy alternative exists.
    StubExecutor rejecting(false, 0.0, FailureKind::ModelRejected, "invalid model specification");
    StubExecutor healthy(true);
    WorkerRegistry registry;
    registry.upsert(availableWorker("a", 1));
    registry.upsert(availableWorker("b", 2));
    std::map<std::string, SimulationExecutor*> executors = {{"a:1", &rejecting}, {"b:2", &healthy}};
    DistributedExecutionManager manager(executors, registry, 5);

    // Act
    const auto outcomes = manager.execute({job("a:1", 5, 10)});

    // Assert: no retry on the alternative (futile for a bad model); the batch is lost.
    EXPECT_TRUE(outcomes[0].lost);
    EXPECT_EQ(rejecting.runCount, 1);
    EXPECT_EQ(healthy.runCount, 0);
}

TEST(DistributedExecutionManager, ShouldRespectMaxRetriesLimit) {
    // Arrange: assigned fails; only retries up to the limit even if more targets exist.
    StubExecutor assigned(false);
    StubExecutor alt1(false);
    StubExecutor alt2(false);
    WorkerRegistry registry;
    registry.upsert(availableWorker("a", 1));
    registry.upsert(availableWorker("b", 2));
    registry.upsert(availableWorker("c", 3));
    std::map<std::string, SimulationExecutor*> executors = {
        {"a:1", &assigned}, {"b:2", &alt1}, {"c:3", &alt2}};
    DistributedExecutionManager manager(executors, registry, 1);  // one reassignment only

    // Act
    const auto outcomes = manager.execute({job("a:1", 5, 10)});

    // Assert: assigned (1) + one retry (1) = two attempts; the batch is lost.
    EXPECT_TRUE(outcomes[0].lost);
    EXPECT_EQ(assigned.runCount + alt1.runCount + alt2.runCount, 2);
}
