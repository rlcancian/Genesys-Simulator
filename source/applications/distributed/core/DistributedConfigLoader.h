#pragma once

#include "DistributedScheduler.h"
#include "WorkerDiscoveryService.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace genesys::distributed {

/** @brief Parsed run configuration for the orchestrator application. */
struct RunConfig {
    std::vector<WorkerDiscoveryService::Endpoint> workers;
    std::string modelFile;   // path to a GenESyS model-language file (read by the app).
    std::string outputFile;  // path to write the aggregated JSON result (optional).
    unsigned int totalReplications = 0;
    bool includeLocal = false;
    int maxRetries = 1;
    std::uint32_t baseSeed = DistributedScheduler::kDefaultBaseSeed;
    int httpTimeoutSeconds = 5;
};

/**
 * @brief Builds a RunConfig from a JSON document or from command-line arguments.
 *
 * Both parsers are pure (no filesystem access): the caller reads the JSON file and passes its
 * text. JSON is the primary source; CLI arguments are the fallback. A model file and a positive
 * replication count are required; everything else has defaults.
 */
class DistributedConfigLoader {
public:
    static std::optional<RunConfig> fromJson(const std::string& jsonText, std::string& error);
    static std::optional<RunConfig> fromArgs(const std::vector<std::string>& args, std::string& error);
};

} // namespace genesys::distributed
