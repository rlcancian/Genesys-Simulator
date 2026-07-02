#include "WorkerRegistry.h"

namespace genesys::distributed {

std::string WorkerRegistry::_key(const std::string& host, int port) {
    return host + ":" + std::to_string(port);
}

void WorkerRegistry::upsert(const WorkerDescriptor& descriptor) {
    std::scoped_lock lock(_mutex);
    _workers[_key(descriptor.host, descriptor.port)] = descriptor;
}

bool WorkerRegistry::markAvailable(const std::string& host, int port, long long observedLatencyMs) {
    std::scoped_lock lock(_mutex);
    const auto iterator = _workers.find(_key(host, port));
    if (iterator == _workers.end()) {
        return false;
    }
    iterator->second.state = WorkerState::Available;
    iterator->second.observedLatencyMs = observedLatencyMs;
    iterator->second.lastError.clear();
    return true;
}

bool WorkerRegistry::markUnavailable(const std::string& host, int port, const std::string& error) {
    std::scoped_lock lock(_mutex);
    const auto iterator = _workers.find(_key(host, port));
    if (iterator == _workers.end()) {
        return false;
    }
    iterator->second.state = WorkerState::Unavailable;
    iterator->second.failureCount += 1;
    iterator->second.lastError = error;
    return true;
}

std::vector<WorkerDescriptor> WorkerRegistry::available() const {
    std::scoped_lock lock(_mutex);
    std::vector<WorkerDescriptor> result;
    for (const auto& [key, descriptor] : _workers) {
        if (descriptor.state == WorkerState::Available) {
            result.push_back(descriptor);
        }
    }
    return result;
}

std::vector<WorkerDescriptor> WorkerRegistry::all() const {
    std::scoped_lock lock(_mutex);
    std::vector<WorkerDescriptor> result;
    result.reserve(_workers.size());
    for (const auto& [key, descriptor] : _workers) {
        result.push_back(descriptor);
    }
    return result;
}

std::optional<WorkerDescriptor> WorkerRegistry::find(const std::string& host, int port) const {
    std::scoped_lock lock(_mutex);
    const auto iterator = _workers.find(_key(host, port));
    if (iterator == _workers.end()) {
        return std::nullopt;
    }
    return iterator->second;
}

std::size_t WorkerRegistry::size() const {
    std::scoped_lock lock(_mutex);
    return _workers.size();
}

} // namespace genesys::distributed
