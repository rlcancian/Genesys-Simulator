#pragma once

#include "SimulationExecutor.h"

namespace genesys::distributed {

/**
 * @brief Runs a replication batch in-process using a local GenESyS simulator.
 *
 * Mirrors the worker's execution path (same kernel `createFromLanguage` → configure → run),
 * so a locally executed batch is equivalent to a remote one with the same model and seed. Each
 * call uses its own simulator instance and returns the same BatchResult shape as remote workers.
 */
class LocalSimulationExecutor : public SimulationExecutor {
public:
    BatchResult execute(const DistributedSimulationJob& job) override;
};

} // namespace genesys::distributed
