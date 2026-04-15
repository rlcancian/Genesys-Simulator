#pragma once

#include "WorkerJob.h"

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

/**
 * @brief Owns worker job records in memory and assigns unique identifiers.
 */
class WorkerJobManager {
public:
    /**
     * @brief Creates and stores a queued worker job with generated identifiers.
     * @param sessionId Identifier of the owning session.
     * @return Newly stored worker job metadata.
     */
    WorkerJob createQueuedJob(const std::string& sessionId);

    /**
     * @brief Replaces the snapshot filename for an existing job.
     * @param jobId Job identifier to update.
     * @param snapshotFilename Snapshot filename persisted in session workspace.
     * @return True when the job exists and was updated.
     */
    bool setSnapshotFilename(const std::string& jobId, const std::string& snapshotFilename);
    /**
     * @brief Updates the lifecycle state for an existing job.
     * @param jobId Job identifier to update.
     * @param state New worker job state.
     * @return True when the job exists and was updated.
     */
    bool setState(const std::string& jobId, WorkerJobState state);

    /**
     * @brief Replaces the message text for an existing job.
     * @param jobId Job identifier to update.
     * @param message Human-readable status or error message.
     * @return True when the job exists and was updated.
     */
    bool setMessage(const std::string& jobId, const std::string& message);

    /**
     * @brief Persists the terminal simulation summary for an existing job.
     * @param jobId Job identifier to update.
     * @param terminalResult Terminal simulation summary values.
     * @return True when the job exists and was updated.
     */
    bool setTerminalResult(const std::string& jobId, const WorkerJobTerminalResult& terminalResult);

    /**
     * @brief Fetches a previously stored job record.
     * @param jobId Job identifier.
     * @return Copy of the job when found.
     */
    std::optional<WorkerJob> getJob(const std::string& jobId) const;

private:
    /**
     * @brief Returns a creation marker string tied to generation sequence.
     * @param numericId Numeric sequence id assigned to a new job.
     * @return Stable string marker for creation ordering.
     */
    static std::string _makeCreationMarker(unsigned long long numericId);

    mutable std::mutex _mutex;
    std::unordered_map<std::string, WorkerJob> _jobs;
    unsigned long long _nextJobNumber = 1;
};
