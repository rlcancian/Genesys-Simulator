#include "DistributedExecutionManager.h"

#include <future>
#include <utility>

namespace genesys::distributed {

namespace {
// A target is remote (and tracked in the registry) when it looks like "host:port".
bool isRemote(const std::string& target) {
    return target.find(':') != std::string::npos;
}

std::pair<std::string, int> splitEndpoint(const std::string& target) {
    const std::size_t colon = target.rfind(':');
    const std::string host = target.substr(0, colon);
    const int port = std::stoi(target.substr(colon + 1));
    return {host, port};
}
} // namespace

DistributedExecutionManager::DistributedExecutionManager(
    std::map<std::string, SimulationExecutor*> executors,
    WorkerRegistry& registry,
    int maxRetries)
    : _executors(std::move(executors)), _registry(registry), _maxRetries(maxRetries) {}

bool DistributedExecutionManager::_isEligible(const std::string& target,
                                              const std::set<std::string>& tried) const {
    if (tried.count(target) > 0 || _executors.count(target) == 0) {
        return false;
    }
    if (!isRemote(target)) {
        return true;  // Local executor is always eligible.
    }
    const auto [host, port] = splitEndpoint(target);
    const auto descriptor = _registry.find(host, port);
    return descriptor.has_value() && descriptor->state == WorkerState::Available;
}

std::string DistributedExecutionManager::_pickAlternative(const std::set<std::string>& tried) const {
    for (const auto& [target, executor] : _executors) {
        if (_isEligible(target, tried)) {
            return target;
        }
    }
    return "";
}

void DistributedExecutionManager::_markFailure(const std::string& target, const std::string& error) {
    if (!isRemote(target)) {
        return;  // Local failures do not change registry availability.
    }
    const auto [host, port] = splitEndpoint(target);
    _registry.markUnavailable(host, port, error);
}

std::vector<BatchExecution> DistributedExecutionManager::execute(
    const std::vector<DistributedSimulationJob>& jobs) {
    std::vector<BatchExecution> outcomes(jobs.size());
    std::vector<std::set<std::string>> tried(jobs.size());

    // Phase 1: run each batch on its assigned target concurrently.
    std::vector<std::future<BatchResult>> futures(jobs.size());
    for (std::size_t i = 0; i < jobs.size(); ++i) {
        outcomes[i].batch = jobs[i].batch;
        const std::string& target = jobs[i].batch.targetEndpoint;
        const auto iterator = _executors.find(target);
        if (iterator == _executors.end()) {
            continue;  // No executor for the assigned target: handled by failover below.
        }
        tried[i].insert(target);
        SimulationExecutor* executor = iterator->second;
        const DistributedSimulationJob job = jobs[i];
        futures[i] = std::async(std::launch::async, [executor, job]() { return executor->execute(job); });
    }

    // A rejected model fails identically on every target, so retrying anywhere is futile: we skip
    // failover and report the real reason instead of masking it as a string of worker failures.
    bool modelRejected = false;
    for (std::size_t i = 0; i < jobs.size(); ++i) {
        if (!futures[i].valid()) {
            continue;
        }
        const BatchResult result = futures[i].get();
        if (result.success) {
            outcomes[i].result = result;
            outcomes[i].ranOn = jobs[i].batch.targetEndpoint;
        } else {
            outcomes[i].result = result;  // keep the error so the aggregator can report it.
            outcomes[i].failedTargets.push_back(jobs[i].batch.targetEndpoint);
            if (result.failureKind == FailureKind::ModelRejected) {
                modelRejected = true;  // healthy worker, bad model: don't mark it unavailable.
            } else {
                _markFailure(jobs[i].batch.targetEndpoint, result.error);
            }
        }
    }

    // Phase 2: re-route batches that have not succeeded to other available targets (skipped
    // entirely when the model was rejected, since no target would accept it).
    for (std::size_t i = 0; i < jobs.size() && !modelRejected; ++i) {
        if (outcomes[i].result.success) {
            continue;
        }
        for (int retry = 0; retry < _maxRetries; ++retry) {
            const std::string alternative = _pickAlternative(tried[i]);
            if (alternative.empty()) {
                break;  // No other available target to try.
            }
            tried[i].insert(alternative);

            DistributedSimulationJob job = jobs[i];
            job.batch.targetEndpoint = alternative;
            const BatchResult result = _executors.at(alternative)->execute(job);
            if (result.success) {
                outcomes[i].result = result;
                outcomes[i].ranOn = alternative;
                break;
            }
            outcomes[i].result = result;  // keep the latest error.
            outcomes[i].failedTargets.push_back(alternative);
            if (result.failureKind == FailureKind::ModelRejected) {
                modelRejected = true;
                break;
            }
            _markFailure(alternative, result.error);
        }
    }

    // Any batch without a successful result is lost (it contributes no statistics).
    for (BatchExecution& outcome : outcomes) {
        if (!outcome.result.success) {
            outcome.lost = true;
        }
    }

    return outcomes;
}

} // namespace genesys::distributed
