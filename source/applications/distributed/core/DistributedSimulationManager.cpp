#include "DistributedSimulationManager.h"

#include "DistributedExecutionManager.h"
#include "DistributedResultsAggregator.h"
#include "LocalSimulationExecutor.h"
#include "RemoteSimulationExecutor.h"
#include "WorkerHttpClient.h"
#include "WorkerRegistry.h"

#include <map>
#include <memory>

namespace genesys::distributed {

namespace {
constexpr const char* kLocalTarget = "local";
} // namespace

AggregatedResult DistributedSimulationManager::run(const DistributedSimulationConfig& config) {
    // Discovery must be fast (short timeout); execution may take minutes (long receive timeout).
    // Both connect quickly. Two clients keep these opposing requirements from sharing one value.
    WorkerHttpClient discoveryClient(config.discoveryTimeoutSeconds, config.discoveryTimeoutSeconds);
    WorkerHttpClient executionClient(config.discoveryTimeoutSeconds, config.runTimeoutSeconds);
    WorkerRegistry registry;

    // 1. Discover and validate the configured workers.
    WorkerDiscoveryService discovery(discoveryClient, registry);
    discovery.discover(config.workers);

    // 2. Build the execution targets: every available worker, plus local when requested.
    std::vector<SchedulerTarget> targets;
    std::vector<std::unique_ptr<SimulationExecutor>> ownedExecutors;
    std::map<std::string, SimulationExecutor*> executors;

    for (const WorkerDescriptor& worker : registry.available()) {
        const std::string endpoint = worker.endpoint();
        targets.push_back({endpoint, false});
        ownedExecutors.push_back(std::make_unique<RemoteSimulationExecutor>(executionClient, worker.host, worker.port));
        executors[endpoint] = ownedExecutors.back().get();
    }

    if (config.includeLocal) {
        targets.push_back({kLocalTarget, true});
        ownedExecutors.push_back(std::make_unique<LocalSimulationExecutor>());
        executors[kLocalTarget] = ownedExecutors.back().get();
    }

    AggregatedResult emptyGuard;
    if (targets.empty()) {
        emptyGuard.totalReplicationsRequested = config.totalReplications;
        emptyGuard.failures.push_back("no available workers and local execution disabled");
        return emptyGuard;
    }

    // 3. Partition the replications into batches (one per target, distinct seeds).
    DistributedScheduler scheduler;
    const std::vector<ReplicationBatch> batches =
        scheduler.partition(config.totalReplications, targets, config.baseSeed);

    std::vector<DistributedSimulationJob> jobs;
    jobs.reserve(batches.size());
    for (const ReplicationBatch& batch : batches) {
        jobs.push_back({config.modelText, batch});
    }

    // 4. Execute with failover, then aggregate the partial results.
    DistributedExecutionManager executionManager(executors, registry, config.maxRetries);
    const std::vector<BatchExecution> outcomes = executionManager.execute(jobs);

    DistributedResultsAggregator aggregator;
    AggregatedResult result = aggregator.aggregate(outcomes);

    // 5. Build the per-worker / discovery report (which workers were found, their final state,
    //    and how many replications each one actually completed).
    std::map<std::string, unsigned int> completedByTarget;
    for (const BatchExecution& outcome : outcomes) {
        if (outcome.result.success && !outcome.ranOn.empty()) {
            completedByTarget[outcome.ranOn] += outcome.result.numberOfReplications;
        }
    }
    for (const WorkerDescriptor& worker : registry.all()) {
        WorkerReport report;
        report.endpoint = worker.endpoint();
        report.isLocal = false;
        report.state = worker.state == WorkerState::Available     ? "available"
                       : worker.state == WorkerState::Unavailable ? "unavailable"
                                                                  : "unknown";
        report.latencyMs = worker.observedLatencyMs;
        report.failureCount = worker.failureCount;
        const auto completed = completedByTarget.find(report.endpoint);
        report.replicationsCompleted = completed != completedByTarget.end() ? completed->second : 0;
        result.workers.push_back(report);
    }
    if (config.includeLocal) {
        WorkerReport report;
        report.endpoint = kLocalTarget;
        report.isLocal = true;
        report.state = "local";
        const auto completed = completedByTarget.find(kLocalTarget);
        report.replicationsCompleted = completed != completedByTarget.end() ? completed->second : 0;
        result.workers.push_back(report);
    }

    return result;
}

} // namespace genesys::distributed
