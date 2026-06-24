#pragma once

#include "AggregatedResult.h"
#include "DistributedScheduler.h"
#include "WorkerDiscoveryService.h"

#include <cstdint>
#include <string>
#include <vector>

namespace genesys::distributed {

/** @brief Inputs for a distributed simulation run. */
struct DistributedSimulationConfig {
    std::vector<WorkerDiscoveryService::Endpoint> workers;  // static list to discover/validate.
    std::string modelText;                                  // GenESyS model language to run.
    unsigned int totalReplications = 0;
    bool includeLocal = false;                              // also use the in-process engine.
    int maxRetries = 1;
    std::uint32_t baseSeed = DistributedScheduler::kDefaultBaseSeed;
    int httpTimeoutSeconds = 5;
};

/**
 * @brief High-level facade that runs a simulation across discovered workers and the local engine.
 *
 * Orchestrates the full pipeline: discover/validate workers, partition the replications into
 * batches (one per available target, optionally including local), execute the batches with
 * failover, and aggregate the partial results into a single AggregatedResult. Reusable by the
 * terminal, GUI or web applications, which only need to provide a config.
 */
class DistributedSimulationManager {
public:
    AggregatedResult run(const DistributedSimulationConfig& config);
};

} // namespace genesys::distributed
