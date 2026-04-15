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
    double simulatedTime = 0.0;
    unsigned int currentReplicationNumber = 0;
    unsigned int numberOfReplications = 0;
    double replicationLength = 0.0;
    double warmUpPeriod = 0.0;
    std::optional<bool> isPaused;
};

/**
 * @brief Stores metadata for a worker job snapshot owned by the web worker.
 */
struct WorkerJob {
    std::string jobId;
    std::string sessionId;
    WorkerJobState state = WorkerJobState::Queued;
    std::string snapshotFilename;
    std::string createdMarker;
    std::string message;
    bool hasTerminalResult = false;
    WorkerJobTerminalResult terminalResult;
};
