#pragma once

#include <optional>
#include <string>

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
    /// Human-readable status or error text.
    std::string message;
    /// True when terminalResult contains valid completion data.
    bool hasTerminalResult = false;
    /// Final simulation summary written when the job finishes.
    WorkerJobTerminalResult terminalResult;
};
