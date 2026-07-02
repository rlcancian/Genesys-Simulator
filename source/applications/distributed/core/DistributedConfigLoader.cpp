#include "DistributedConfigLoader.h"

#include "Json.h"

#include <cstdlib>

namespace genesys::distributed {

namespace {

bool validate(const RunConfig& config, std::string& error) {
    if (config.modelFile.empty()) {
        error = "a model file is required";
        return false;
    }
    if (config.totalReplications == 0) {
        error = "totalReplications must be greater than zero";
        return false;
    }
    if (config.workers.empty() && !config.includeLocal) {
        error = "no workers configured and local execution disabled";
        return false;
    }
    if (config.discoveryTimeoutSeconds <= 0 || config.runTimeoutSeconds <= 0) {
        error = "discoveryTimeoutSeconds and runTimeoutSeconds must be greater than zero";
        return false;
    }
    return true;
}

// Splits "host:port" into its parts; returns false when malformed.
bool parseEndpoint(const std::string& text, WorkerDiscoveryService::Endpoint& endpoint) {
    const std::size_t colon = text.rfind(':');
    if (colon == std::string::npos || colon == 0 || colon + 1 >= text.size()) {
        return false;
    }
    endpoint.host = text.substr(0, colon);
    endpoint.port = std::atoi(text.substr(colon + 1).c_str());
    return endpoint.port > 0;
}

} // namespace

std::optional<RunConfig> DistributedConfigLoader::fromJson(const std::string& jsonText, std::string& error) {
    RunConfig config;

    config.modelFile = json::getString(jsonText, "modelFile").value_or("");
    config.outputFile = json::getString(jsonText, "outputFile").value_or("");
    config.totalReplications =
        static_cast<unsigned int>(json::getInt(jsonText, "totalReplications").value_or(0));
    config.includeLocal = json::getBool(jsonText, "includeLocal").value_or(false);
    config.maxRetries = static_cast<int>(json::getInt(jsonText, "maxRetries").value_or(1));
    config.baseSeed = static_cast<std::uint32_t>(
        json::getInt(jsonText, "baseSeed").value_or(DistributedScheduler::kDefaultBaseSeed));
    config.discoveryTimeoutSeconds =
        static_cast<int>(json::getInt(jsonText, "discoveryTimeoutSeconds").value_or(5));
    config.runTimeoutSeconds =
        static_cast<int>(json::getInt(jsonText, "runTimeoutSeconds").value_or(300));

    if (const auto workersArray = json::getArray(jsonText, "workers"); workersArray.has_value()) {
        for (const std::string& object : json::splitObjects(workersArray.value())) {
            const auto host = json::getString(object, "host");
            const auto port = json::getInt(object, "port");
            if (!host.has_value() || !port.has_value()) {
                error = "worker entry missing host or port";
                return std::nullopt;
            }
            config.workers.push_back({host.value(), static_cast<int>(port.value())});
        }
    }

    if (!validate(config, error)) {
        return std::nullopt;
    }
    return config;
}

std::optional<RunConfig> DistributedConfigLoader::fromArgs(const std::vector<std::string>& args,
                                                           std::string& error) {
    RunConfig config;

    for (std::size_t i = 0; i < args.size(); ++i) {
        const std::string& arg = args[i];
        auto next = [&](const std::string& name) -> std::string {
            if (i + 1 >= args.size()) {
                error = "missing value for " + name;
                return "";
            }
            return args[++i];
        };

        if (arg == "--local") {
            config.includeLocal = true;
        } else if (arg == "--model") {
            config.modelFile = next(arg);
        } else if (arg == "--output") {
            config.outputFile = next(arg);
        } else if (arg == "--replications") {
            config.totalReplications = static_cast<unsigned int>(std::atoi(next(arg).c_str()));
        } else if (arg == "--max-retries") {
            config.maxRetries = std::atoi(next(arg).c_str());
        } else if (arg == "--base-seed") {
            config.baseSeed = static_cast<std::uint32_t>(std::strtoul(next(arg).c_str(), nullptr, 10));
        } else if (arg == "--timeout") {
            config.runTimeoutSeconds = std::atoi(next(arg).c_str());
        } else if (arg == "--discovery-timeout") {
            config.discoveryTimeoutSeconds = std::atoi(next(arg).c_str());
        } else if (arg == "--worker") {
            WorkerDiscoveryService::Endpoint endpoint;
            if (!parseEndpoint(next(arg), endpoint)) {
                error = "invalid --worker value (expected host:port)";
                return std::nullopt;
            }
            config.workers.push_back(endpoint);
        } else {
            error = "unknown argument: " + arg;
            return std::nullopt;
        }

        if (!error.empty()) {
            return std::nullopt;
        }
    }

    if (!validate(config, error)) {
        return std::nullopt;
    }
    return config;
}

} // namespace genesys::distributed
