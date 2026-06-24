#pragma once

#include "DistributedSimulationJob.h"
#include "SimulationExecutor.h"
#include "SimulationResult.h"
#include "WorkerRegistry.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace genesys::distributed {

/** @brief Final outcome of running one batch, including retries and failover history. */
struct BatchExecution {
    ReplicationBatch batch;
    BatchResult result;             // result.success is the final success of the batch.
    std::string ranOn;              // target that produced the successful result ("" if lost).
    bool lost = false;              // true when every attempt failed.
    std::vector<std::string> failedTargets;
};

/**
 * @brief Runs replication batches across executors with timeout/failure handling and failover.
 *
 * Each batch first runs on its assigned target (these initial runs happen concurrently). A batch
 * that fails marks its worker unavailable in the registry and is re-routed to another available
 * target, up to `maxRetries` reassignments. If no attempt succeeds the batch is reported as lost.
 * Only a successful attempt contributes its statistics, so failed/retried attempts never
 * double-count.
 */
class DistributedExecutionManager {
public:
    DistributedExecutionManager(std::map<std::string, SimulationExecutor*> executors,
                                WorkerRegistry& registry,
                                int maxRetries);

    std::vector<BatchExecution> execute(const std::vector<DistributedSimulationJob>& jobs);

private:
    bool _isEligible(const std::string& target, const std::set<std::string>& tried) const;
    std::string _pickAlternative(const std::set<std::string>& tried) const;
    void _markFailure(const std::string& target, const std::string& error);

    std::map<std::string, SimulationExecutor*> _executors;
    WorkerRegistry& _registry;
    int _maxRetries;
};

} // namespace genesys::distributed
