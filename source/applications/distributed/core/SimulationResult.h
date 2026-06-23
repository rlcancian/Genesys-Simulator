#pragma once

#include <string>
#include <vector>

namespace genesys::distributed {

/**
 * @brief Cross-replication aggregate for a statistics collector within one batch.
 *
 * Carries the raw moments needed to merge batches exactly: the sample size (number of
 * replications), the mean and variance of the per-replication values, and the min/max.
 */
struct CollectorStat {
    std::string name;
    unsigned int numReplications = 0;
    double average = 0.0;
    double variance = 0.0;
    double min = 0.0;
    double max = 0.0;
    unsigned int numObservations = 0;
};

/** @brief Counter aggregate for one batch: the summed count across the batch's replications. */
struct CounterStat {
    std::string name;
    double total = 0.0;
};

/**
 * @brief Result of executing a single replication batch (locally or on a remote worker).
 *
 * On failure `success` is false and `error` explains why; statistics/counters are empty.
 */
struct BatchResult {
    bool success = false;
    std::string error;
    unsigned int numberOfReplications = 0;
    std::vector<CollectorStat> statistics;
    std::vector<CounterStat> counters;
};

} // namespace genesys::distributed
