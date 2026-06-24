#pragma once

#include "AggregatedResult.h"
#include "DistributedSimulationManager.h"

#include <string>

namespace genesys::distributed {

/**
 * @brief Application-level orchestration: runs a distributed simulation and renders its result.
 *
 * Keeps the rendering logic (human-readable summary and structured JSON) free of I/O so it is
 * reusable by the standalone orchestrator and any other application, and unit-testable.
 */
class CoordinatorApplication {
public:
    /** @brief Runs the distributed simulation for the given configuration. */
    AggregatedResult execute(const DistributedSimulationConfig& config) const;

    /** @brief Renders a human-readable summary of the aggregated result. */
    std::string renderSummary(const AggregatedResult& result) const;

    /** @brief Renders the aggregated result as a structured JSON document. */
    std::string renderJson(const AggregatedResult& result) const;
};

} // namespace genesys::distributed
