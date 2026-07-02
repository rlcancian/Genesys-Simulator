#pragma once

#include "DistributedScheduler.h"

#include <string>

namespace genesys::distributed {

/**
 * @brief A unit of distributed work: a model plus the replication batch to run for it.
 *
 * `modelText` is the GenESyS model language to simulate; `batch` carries how many replications
 * to run, the RNG seed to use, and which target (remote endpoint or local engine) should run it.
 */
struct DistributedSimulationJob {
    std::string modelText;
    ReplicationBatch batch;
};

} // namespace genesys::distributed
