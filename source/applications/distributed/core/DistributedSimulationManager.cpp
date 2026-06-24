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
    WorkerHttpClient client(config.httpTimeoutSeconds);
    WorkerRegistry registry;

    // 1. Discover and validate the configured workers.
    WorkerDiscoveryService discovery(client, registry);
    discovery.discover(config.workers);

    // 2. Build the execution targets: every available worker, plus local when requested.
    std::vector<SchedulerTarget> targets;
    std::vector<std::unique_ptr<SimulationExecutor>> ownedExecutors;
    std::map<std::string, SimulationExecutor*> executors;

    for (const WorkerDescriptor& worker : registry.available()) {
        const std::string endpoint = worker.endpoint();
        targets.push_back({endpoint, false});
        ownedExecutors.push_back(std::make_unique<RemoteSimulationExecutor>(client, worker.host, worker.port));
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
    return aggregator.aggregate(outcomes);
}

} // namespace genesys::distributed
