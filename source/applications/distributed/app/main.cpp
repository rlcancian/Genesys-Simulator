#include "core/CoordinatorApplication.h"
#include "core/DistributedConfigLoader.h"
#include "core/DistributedSimulationManager.h"

#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

using namespace genesys::distributed;

namespace {

std::string readFile(const std::string& path, bool& ok) {
    std::ifstream file(path);
    if (!file) {
        ok = false;
        return "";
    }
    std::ostringstream stream;
    stream << file.rdbuf();
    ok = true;
    return stream.str();
}

void printUsage() {
    std::cerr << "Usage:\n"
              << "  genesys_distributed_app --config <file.json>\n"
              << "  genesys_distributed_app --model <file> --replications <N> [--local]\n"
              << "      [--worker host:port]... [--output <file>] [--max-retries <N>]\n"
              << "      [--base-seed <N>] [--timeout <seconds>] [--discovery-timeout <seconds>]\n";
}

DistributedSimulationConfig toSimulationConfig(const RunConfig& runConfig, std::string modelText) {
    DistributedSimulationConfig config;
    config.workers = runConfig.workers;
    config.modelText = std::move(modelText);
    config.totalReplications = runConfig.totalReplications;
    config.includeLocal = runConfig.includeLocal;
    config.maxRetries = runConfig.maxRetries;
    config.baseSeed = runConfig.baseSeed;
    config.discoveryTimeoutSeconds = runConfig.discoveryTimeoutSeconds;
    config.runTimeoutSeconds = runConfig.runTimeoutSeconds;
    return config;
}

} // namespace

int main(int argc, char** argv) {
    const std::vector<std::string> args(argv + 1, argv + argc);
    if (args.empty()) {
        printUsage();
        return 1;
    }

    std::string error;
    std::optional<RunConfig> runConfig;
    if (args.size() >= 2 && args[0] == "--config") {
        bool ok = false;
        const std::string text = readFile(args[1], ok);
        if (!ok) {
            std::cerr << "Cannot read config file: " << args[1] << "\n";
            return 1;
        }
        runConfig = DistributedConfigLoader::fromJson(text, error);
    } else {
        runConfig = DistributedConfigLoader::fromArgs(args, error);
    }

    if (!runConfig.has_value()) {
        std::cerr << "Configuration error: " << error << "\n";
        printUsage();
        return 1;
    }

    bool modelOk = false;
    const std::string modelText = readFile(runConfig->modelFile, modelOk);
    if (!modelOk) {
        std::cerr << "Cannot read model file: " << runConfig->modelFile << "\n";
        return 1;
    }

    CoordinatorApplication app;
    const AggregatedResult result = app.execute(toSimulationConfig(*runConfig, modelText));

    std::cout << app.renderSummary(result);

    if (!runConfig->outputFile.empty()) {
        std::ofstream output(runConfig->outputFile);
        if (!output) {
            std::cerr << "Cannot write output file: " << runConfig->outputFile << "\n";
            return 1;
        }
        output << app.renderJson(result);
        std::cout << "Aggregated result written to " << runConfig->outputFile << "\n";
    }

    return 0;
}
