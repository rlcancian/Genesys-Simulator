#pragma once

#include <string>
#include <vector>

namespace genesys::distributed {

/** @brief A statistics collector aggregated across all contributing batches. */
struct AggregatedStatistic {
    std::string name;
    unsigned int numReplications = 0;
    double average = 0.0;
    double variance = 0.0;
    double stddev = 0.0;
    double min = 0.0;
    double max = 0.0;
    double halfWidthConfidenceInterval = 0.0;
    double confidenceLevel = 0.95;
};

/** @brief A counter aggregated (summed) across all contributing batches. */
struct AggregatedCounter {
    std::string name;
    double total = 0.0;
};

/**
 * @brief Final unified result of a distributed simulation run.
 *
 * Equivalent to what a single centralized run would produce, plus bookkeeping about how many
 * replications were requested vs. completed and any partial losses.
 */
struct AggregatedResult {
    unsigned int totalReplicationsRequested = 0;
    unsigned int totalReplicationsCompleted = 0;
    std::vector<AggregatedStatistic> statistics;
    std::vector<AggregatedCounter> counters;
    std::vector<std::string> failures;
};

} // namespace genesys::distributed
