#pragma once

#include <cstdint>
#include <string>

namespace genesys::distributed {

/** @brief Availability state of a known worker. */
enum class WorkerState {
    Unknown,
    Available,
    Unavailable
};

/**
 * @brief Worker capability flags relevant to distributed orchestration.
 *
 * Mirrors the booleans exposed by `GET /api/v1/worker/capabilities`. Only the flags the
 * orchestrator relies on are kept; unknown/irrelevant flags are ignored.
 */
struct WorkerCapabilities {
    bool supportsDistributedJobs = false;
    bool supportsJobPolling = false;
    bool supportsJobResultRetrieval = false;
    bool supportsSynchronousRun = false;
    bool supportsModelCreation = false;
};

/**
 * @brief Describes a remote worker: its endpoint, declared identity/capabilities and the
 * runtime state observed by the orchestrator (availability, latency, failure history).
 */
struct WorkerDescriptor {
    // Endpoint.
    std::string host;
    int port = 0;

    // Declared identity (from /worker/info).
    std::string role;
    std::string application;
    std::string apiVersion;
    std::string simulatorName;
    long long simulatorVersionNumber = 0;

    // Declared capabilities (from /worker/capabilities).
    WorkerCapabilities capabilities;

    // Runtime state observed by the orchestrator.
    WorkerState state = WorkerState::Unknown;
    long long observedLatencyMs = -1;  // -1 means not measured yet.
    unsigned int failureCount = 0;
    std::string lastError;

    /** @brief Returns the "host:port" key identifying this worker. */
    std::string endpoint() const {
        return host + ":" + std::to_string(port);
    }
};

} // namespace genesys::distributed
