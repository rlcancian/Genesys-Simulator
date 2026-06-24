#pragma once

#include "AggregatedResult.h"
#include "DistributedExecutionManager.h"

#include <vector>

namespace genesys::distributed {

/**
 * @brief Merges per-batch results into a single result equivalent to a centralized run.
 *
 * Statistics are merged by collector name using pooled sums reconstructed from each batch's
 * moments (count, mean, variance), giving the exact global mean, variance, standard deviation and
 * confidence interval. Counters are summed. Only successful batches contribute; lost batches are
 * reported as failures and excluded from the completed-replication count, so retries never
 * double-count.
 */
class DistributedResultsAggregator {
public:
    /** @brief Critical value for the 95% confidence interval (matches the kernel's convention). */
    static constexpr double kCriticalValue95 = 1.96;

    AggregatedResult aggregate(const std::vector<BatchExecution>& outcomes) const;
};

} // namespace genesys::distributed
