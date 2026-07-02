#include "WorkerDiscoveryService.h"

#include "Json.h"

#include <chrono>

namespace genesys::distributed {

namespace {
constexpr const char* kInfoPath = "/api/v1/worker/info";
constexpr const char* kCapabilitiesPath = "/api/v1/worker/capabilities";

void parseCapabilities(const std::string& body, WorkerCapabilities& capabilities) {
    capabilities.supportsDistributedJobs = json::getBool(body, "supportsDistributedJobs").value_or(false);
    capabilities.supportsJobPolling = json::getBool(body, "supportsJobPolling").value_or(false);
    capabilities.supportsJobResultRetrieval = json::getBool(body, "supportsJobResultRetrieval").value_or(false);
    capabilities.supportsSynchronousRun = json::getBool(body, "supportsSynchronousRun").value_or(false);
    capabilities.supportsModelCreation = json::getBool(body, "supportsModelCreation").value_or(false);
}
} // namespace

WorkerDiscoveryService::WorkerDiscoveryService(WorkerHttpClient& client, WorkerRegistry& registry)
    : _client(client), _registry(registry) {}

bool WorkerDiscoveryService::_isCompatible(const WorkerDescriptor& descriptor) {
    return descriptor.role == "worker" &&
           descriptor.capabilities.supportsDistributedJobs &&
           descriptor.capabilities.supportsJobResultRetrieval;
}

WorkerDescriptor WorkerDiscoveryService::probe(const Endpoint& endpoint) {
    WorkerDescriptor descriptor;
    descriptor.host = endpoint.host;
    descriptor.port = endpoint.port;
    descriptor.state = WorkerState::Unavailable;

    const auto start = std::chrono::steady_clock::now();

    const HttpClientResponse info = _client.get(endpoint.host, endpoint.port, kInfoPath);
    if (!info.ok || info.status != 200) {
        descriptor.lastError = info.ok ? ("worker/info returned status " + std::to_string(info.status))
                                       : info.error;
        _registry.upsert(descriptor);
        return descriptor;
    }

    descriptor.role = json::getString(info.body, "role").value_or("");
    descriptor.application = json::getString(info.body, "application").value_or("");
    descriptor.apiVersion = json::getString(info.body, "apiVersion").value_or("");
    descriptor.simulatorName = json::getString(info.body, "simulatorName").value_or("");
    descriptor.simulatorVersionNumber = json::getInt(info.body, "simulatorVersionNumber").value_or(0);

    const HttpClientResponse capabilities = _client.get(endpoint.host, endpoint.port, kCapabilitiesPath);
    if (!capabilities.ok || capabilities.status != 200) {
        descriptor.lastError = capabilities.ok
                                   ? ("worker/capabilities returned status " + std::to_string(capabilities.status))
                                   : capabilities.error;
        _registry.upsert(descriptor);
        return descriptor;
    }
    parseCapabilities(capabilities.body, descriptor.capabilities);

    if (!_isCompatible(descriptor)) {
        descriptor.lastError = "worker is not compatible (role/capabilities)";
        _registry.upsert(descriptor);
        return descriptor;
    }

    const auto elapsed = std::chrono::steady_clock::now() - start;
    descriptor.observedLatencyMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    descriptor.state = WorkerState::Available;
    descriptor.lastError.clear();
    _registry.upsert(descriptor);
    return descriptor;
}

int WorkerDiscoveryService::discover(const std::vector<Endpoint>& endpoints) {
    int availableCount = 0;
    for (const Endpoint& endpoint : endpoints) {
        const WorkerDescriptor descriptor = probe(endpoint);
        if (descriptor.state == WorkerState::Available) {
            ++availableCount;
        }
    }
    return availableCount;
}

} // namespace genesys::distributed
