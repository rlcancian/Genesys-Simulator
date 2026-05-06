#include "WorkerJobManager.h"

/**
 * @brief Creates and stores a queued worker job with a monotonic identifier.
 */
WorkerJob WorkerJobManager::createQueuedJob(const std::string& sessionId) {
    std::scoped_lock lock(_mutex);

    const unsigned long long numericId = _nextJobNumber++;
    WorkerJob job;
    job.jobId = std::string("job-") + std::to_string(numericId);
    job.sessionId = sessionId;
    job.state = WorkerJobState::Queued;
    job.createdMarker = _makeCreationMarker(numericId);

    _jobs[job.jobId] = job;
    return job;
}

/**
 * @brief Updates the snapshot filename for a stored job.
 */
bool WorkerJobManager::setSnapshotFilename(const std::string& jobId, const std::string& snapshotFilename) {
    std::scoped_lock lock(_mutex);

    const auto iterator = _jobs.find(jobId);
    if (iterator == _jobs.end()) {
        return false;
    }

    iterator->second.snapshotFilename = snapshotFilename;
    return true;
}

/**
 * @brief Updates the lifecycle state for a stored job.
 */
bool WorkerJobManager::setState(const std::string& jobId, WorkerJobState state) {
    std::scoped_lock lock(_mutex);

    const auto iterator = _jobs.find(jobId);
    if (iterator == _jobs.end()) {
        return false;
    }

    iterator->second.state = state;
    return true;
}

/**
 * @brief Replaces the human-readable message for a stored job.
 */
bool WorkerJobManager::setMessage(const std::string& jobId, const std::string& message) {
    std::scoped_lock lock(_mutex);

    const auto iterator = _jobs.find(jobId);
    if (iterator == _jobs.end()) {
        return false;
    }

    iterator->second.message = message;
    return true;
}

/**
 * @brief Stores the terminal simulation result and marks it available.
 */
bool WorkerJobManager::setTerminalResult(const std::string& jobId, const WorkerJobTerminalResult& terminalResult) {
    std::scoped_lock lock(_mutex);

    const auto iterator = _jobs.find(jobId);
    if (iterator == _jobs.end()) {
        return false;
    }

    iterator->second.terminalResult = terminalResult;
    iterator->second.hasTerminalResult = true;
    return true;
}

/**
 * @brief Retrieves a copy of a job record when it exists.
 */
std::optional<WorkerJob> WorkerJobManager::getJob(const std::string& jobId) const {
    std::scoped_lock lock(_mutex);

    const auto iterator = _jobs.find(jobId);
    if (iterator == _jobs.end()) {
        return std::nullopt;
    }

    return iterator->second;
}

/**
 * @brief Formats the numeric sequence id used to track creation order.
 */
std::string WorkerJobManager::_makeCreationMarker(unsigned long long numericId) {
    return std::string("seq-") + std::to_string(numericId);
}
