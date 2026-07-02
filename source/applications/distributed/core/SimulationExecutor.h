#pragma once

#include "DistributedSimulationJob.h"
#include "SimulationResult.h"

namespace genesys::distributed {

/**
 * @brief Common interface for running a replication batch, regardless of where it runs.
 *
 * Implementations include a remote executor (driving a worker over HTTP) and a local executor
 * (running an in-process simulator). The execution manager treats both uniformly.
 */
class SimulationExecutor {
public:
    virtual ~SimulationExecutor() = default;

    /** @brief Runs the job's batch and returns its aggregated per-batch result. */
    virtual BatchResult execute(const DistributedSimulationJob& job) = 0;
};

} // namespace genesys::distributed
