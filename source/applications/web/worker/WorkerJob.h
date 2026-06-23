#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

/**
 * @brief Enumerates lifecycle states for a worker job.
 */
enum class WorkerJobState {
    Queued,
    Running,
    Finished,
    Failed
};

/**
 * @brief Distinguishes the kind of cross-replication aggregate captured for a job.
 */
enum class WorkerJobCollectorKind {
    Statistics,
    Counter
};

/**
 * @brief Cross-replication statistics captured for a single collector or counter.
 *
 * The simulation-level aggregate is a distribution over the per-replication values
 * (averages for statistics collectors, counts for counters). The raw moments below let
 * a distributed orchestrator merge partial results from several workers exactly.
 */
struct WorkerJobCollectorStat {
    /// Name of the collector/counter the aggregate belongs to.
    std::string name;
    /// Whether the aggregate comes from a statistics collector or a counter.
    WorkerJobCollectorKind kind = WorkerJobCollectorKind::Statistics;
    /// Number of replications aggregated (sample size at simulation level).
    unsigned int numReplications = 0;
    /// Mean of the per-replication values.
    double average = 0.0;
    /// Sample variance of the per-replication values.
    double variance = 0.0;
    /// Minimum per-replication value observed.
    double min = 0.0;
    /// Maximum per-replication value observed.
    double max = 0.0;
    /// Total number of underlying observations (informative).
    unsigned int numObservations = 0;
};

/**
 * @brief Stores the terminal simulation summary persisted for a finished or failed worker job.
 */
struct WorkerJobTerminalResult {
    /// Simulated time reached when the job completed.
    double simulatedTime = 0.0;
    /// Current replication number at completion.
    unsigned int currentReplicationNumber = 0;
    /// Total number of replications configured for the run.
    unsigned int numberOfReplications = 0;
    /// Replication length used by the run.
    double replicationLength = 0.0;
    /// Warm-up period used by the run.
    double warmUpPeriod = 0.0;
    /// Pause state when the simulator explicitly reported it.
    std::optional<bool> isPaused;
    /// Cross-replication aggregates per collector/counter, for distributed merging.
    std::vector<WorkerJobCollectorStat> collectors;
};

/**
 * @brief Stores metadata for a worker job snapshot owned by the web worker.
 */
struct WorkerJob {
    /// Stable job identifier returned to API clients.
    std::string jobId;
    /// Session that owns the job snapshot and execution context.
    std::string sessionId;
    /// Current lifecycle state of the job.
    WorkerJobState state = WorkerJobState::Queued;
    /// Snapshot filename stored in the session workspace.
    std::string snapshotFilename;
    /// Monotonic creation marker used to preserve job ordering.
    std::string createdMarker;
    /// Number of replications requested for this job (unset uses the model default).
    std::optional<unsigned int> numberOfReplications;
    /// RNG seed requested for this job (unset uses the model/sampler default).
    std::optional<std::uint32_t> seed;
    /// Human-readable status or error text.
    std::string message;
    /// True when terminalResult contains valid completion data.
    bool hasTerminalResult = false;
    /// Final simulation summary written when the job finishes.
    WorkerJobTerminalResult terminalResult;
};
