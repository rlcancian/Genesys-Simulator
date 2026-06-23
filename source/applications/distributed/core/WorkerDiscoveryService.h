#pragma once

#include "WorkerDescriptor.h"
#include "WorkerHttpClient.h"
#include "WorkerRegistry.h"

#include <string>
#include <vector>

namespace genesys::distributed {

/**
 * @brief Validates a static list of worker endpoints and registers the live ones.
 *
 * For each endpoint it queries `GET /api/v1/worker/info` and `GET /api/v1/worker/capabilities`,
 * builds a descriptor and decides availability. Every endpoint is recorded in the registry:
 * reachable and compatible ones as Available (with measured latency), the rest as Unavailable
 * (with a failure reason). Discovery never throws or aborts on a single bad endpoint.
 */
class WorkerDiscoveryService {
public:
    /** @brief A worker endpoint to validate. */
    struct Endpoint {
        std::string host;
        int port = 0;
    };

    WorkerDiscoveryService(WorkerHttpClient& client, WorkerRegistry& registry);

    /**
     * @brief Validates and registers each endpoint.
     * @return Number of endpoints that became Available.
     */
    int discover(const std::vector<Endpoint>& endpoints);

    /** @brief Validates a single endpoint and upserts the resulting descriptor. */
    WorkerDescriptor probe(const Endpoint& endpoint);

private:
    static bool _isCompatible(const WorkerDescriptor& descriptor);

    WorkerHttpClient& _client;
    WorkerRegistry& _registry;
};

} // namespace genesys::distributed
