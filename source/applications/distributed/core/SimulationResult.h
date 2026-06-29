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
 * @brief Why a batch failed, so the execution manager can react appropriately.
 *
 * - `None`: the batch succeeded.
 * - `WorkerUnavailable`: a transport/worker-side problem (connection, timeout, 5xx, crashed run).
 *   The target is unhealthy for now; mark it unavailable and retry the batch elsewhere.
 * - `ModelRejected`: the worker (or local engine) refused the model itself (e.g. an invalid model
 *   specification, HTTP 4xx on import). The worker is healthy; retrying anywhere is futile because
 *   every target parses the same model. Do not mark the worker unavailable; abort early.
 */
enum class FailureKind {
    None,
    WorkerUnavailable,
    ModelRejected,
};

/**
 * @brief Result of executing a single replication batch (locally or on a remote worker).
 *
 * On failure `success` is false and `error` explains why (and `failureKind` classifies it);
 * statistics/counters are empty.
 */
struct BatchResult {
    bool success = false;
    FailureKind failureKind = FailureKind::None;
    std::string error;
    unsigned int numberOfReplications = 0;
    std::vector<CollectorStat> statistics;
    std::vector<CounterStat> counters;
};

} // namespace genesys::distributed
