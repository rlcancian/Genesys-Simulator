#include "DistributedResultsAggregator.h"

#include <algorithm>
#include <cmath>
#include <map>

namespace genesys::distributed {

namespace {
// Running pooled sums for one collector across batches.
struct StatAccumulator {
    double sum = 0.0;          // Σ of per-replication averages (S).
    double sumSquares = 0.0;   // Σ of squares of per-replication averages (Q).
    unsigned int count = 0;    // total replications (N).
    double min = 0.0;
    double max = 0.0;
    bool seen = false;
};
} // namespace

AggregatedResult DistributedResultsAggregator::aggregate(
    const std::vector<BatchExecution>& outcomes) const {
    AggregatedResult result;

    // std::map keeps a deterministic (name-sorted) output order.
    std::map<std::string, StatAccumulator> statistics;
    std::map<std::string, double> counters;

    for (const BatchExecution& outcome : outcomes) {
        result.totalReplicationsRequested += outcome.batch.numberOfReplications;

        if (!outcome.result.success) {
            result.failures.push_back(
                "batch (seed=" + std::to_string(outcome.batch.seed) + ", replications=" +
                std::to_string(outcome.batch.numberOfReplications) + ") lost");
            continue;
        }

        result.totalReplicationsCompleted += outcome.result.numberOfReplications;

        for (const CollectorStat& stat : outcome.result.statistics) {
            StatAccumulator& accumulator = statistics[stat.name];
            const double n = static_cast<double>(stat.numReplications);
            // Reconstruct the within-batch sums from the reported moments.
            const double batchSum = stat.average * n;
            const double batchSumSquares = stat.variance * (n - 1.0) + n * stat.average * stat.average;

            accumulator.sum += batchSum;
            accumulator.sumSquares += batchSumSquares;
            accumulator.count += stat.numReplications;
            if (!accumulator.seen) {
                accumulator.min = stat.min;
                accumulator.max = stat.max;
                accumulator.seen = true;
            } else {
                accumulator.min = std::min(accumulator.min, stat.min);
                accumulator.max = std::max(accumulator.max, stat.max);
            }
        }

        for (const CounterStat& counter : outcome.result.counters) {
            counters[counter.name] += counter.total;
        }
    }

    for (const auto& [name, accumulator] : statistics) {
        AggregatedStatistic aggregated;
        aggregated.name = name;
        aggregated.numReplications = accumulator.count;
        aggregated.min = accumulator.min;
        aggregated.max = accumulator.max;
        aggregated.confidenceLevel = 0.95;

        const double n = static_cast<double>(accumulator.count);
        if (accumulator.count > 0) {
            aggregated.average = accumulator.sum / n;
        }
        if (accumulator.count >= 2) {
            const double variance =
                (accumulator.sumSquares - n * aggregated.average * aggregated.average) / (n - 1.0);
            aggregated.variance = std::max(0.0, variance);  // guard tiny negative from rounding.
            aggregated.stddev = std::sqrt(aggregated.variance);
            aggregated.halfWidthConfidenceInterval = kCriticalValue95 * aggregated.stddev / std::sqrt(n);
        }
        result.statistics.push_back(aggregated);
    }

    for (const auto& [name, total] : counters) {
        result.counters.push_back({name, total});
    }

    return result;
}

} // namespace genesys::distributed
