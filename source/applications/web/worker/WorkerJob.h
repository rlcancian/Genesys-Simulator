#pragma once

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
 * @brief Stores metadata for a worker job snapshot owned by the web worker.
 */
struct WorkerJob {
    std::string jobId;
    std::string sessionId;
    WorkerJobState state = WorkerJobState::Queued;
    std::string snapshotFilename;
    std::string createdMarker;
    std::string message;
};
