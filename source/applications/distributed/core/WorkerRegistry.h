#pragma once

#include "WorkerDescriptor.h"

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace genesys::distributed {

/**
 * @brief Thread-safe registry of known workers keyed by "host:port".
 *
 * Holds descriptors discovered/validated by the orchestrator and tracks their availability,
 * latency and failure history so the scheduler and execution manager can pick healthy workers.
 */
class WorkerRegistry {
public:
    /** @brief Inserts or replaces the descriptor for its endpoint. */
    void upsert(const WorkerDescriptor& descriptor);

    /**
     * @brief Marks a worker available and records the observed latency.
     * @return True when the worker exists.
     */
    bool markAvailable(const std::string& host, int port, long long observedLatencyMs);

    /**
     * @brief Marks a worker unavailable, increments its failure count and stores the error.
     * @return True when the worker exists.
     */
    bool markUnavailable(const std::string& host, int port, const std::string& error);

    /** @brief Returns descriptors currently in the Available state. */
    std::vector<WorkerDescriptor> available() const;

    /** @brief Returns all known descriptors. */
    std::vector<WorkerDescriptor> all() const;

    /** @brief Returns the descriptor for an endpoint when known. */
    std::optional<WorkerDescriptor> find(const std::string& host, int port) const;

    /** @brief Number of known workers. */
    std::size_t size() const;

private:
    static std::string _key(const std::string& host, int port);

    mutable std::mutex _mutex;
    std::unordered_map<std::string, WorkerDescriptor> _workers;
};

} // namespace genesys::distributed
