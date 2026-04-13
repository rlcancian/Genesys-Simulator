#include "WorkerJobManager.h"

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

bool WorkerJobManager::setSnapshotFilename(const std::string& jobId, const std::string& snapshotFilename) {
    std::scoped_lock lock(_mutex);

    const auto iterator = _jobs.find(jobId);
    if (iterator == _jobs.end()) {
        return false;
    }

    iterator->second.snapshotFilename = snapshotFilename;
    return true;
}

bool WorkerJobManager::setState(const std::string& jobId, WorkerJobState state) {
    std::scoped_lock lock(_mutex);

    const auto iterator = _jobs.find(jobId);
    if (iterator == _jobs.end()) {
        return false;
    }

    iterator->second.state = state;
    return true;
}

bool WorkerJobManager::setMessage(const std::string& jobId, const std::string& message) {
    std::scoped_lock lock(_mutex);

    const auto iterator = _jobs.find(jobId);
    if (iterator == _jobs.end()) {
        return false;
    }

    iterator->second.message = message;
    return true;
}

std::optional<WorkerJob> WorkerJobManager::getJob(const std::string& jobId) const {
    std::scoped_lock lock(_mutex);

    const auto iterator = _jobs.find(jobId);
    if (iterator == _jobs.end()) {
        return std::nullopt;
    }

    return iterator->second;
}

std::string WorkerJobManager::_makeCreationMarker(unsigned long long numericId) {
    return std::string("seq-") + std::to_string(numericId);
}
