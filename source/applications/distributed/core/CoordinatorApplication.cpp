#include "CoordinatorApplication.h"

#include "Json.h"

#include <sstream>

namespace genesys::distributed {

AggregatedResult CoordinatorApplication::execute(const DistributedSimulationConfig& config) const {
    DistributedSimulationManager manager;
    return manager.run(config);
}

std::string CoordinatorApplication::renderSummary(const AggregatedResult& result) const {
    std::ostringstream out;
    out << "Distributed simulation result\n";
    out << "  Replications: " << result.totalReplicationsCompleted << " completed / "
        << result.totalReplicationsRequested << " requested\n";

    out << "  Statistics (" << result.statistics.size() << "):\n";
    for (const AggregatedStatistic& stat : result.statistics) {
        out << "    " << stat.name << ": avg=" << stat.average << " stddev=" << stat.stddev
            << " ci" << static_cast<int>(stat.confidenceLevel * 100) << "%=+/-"
            << stat.halfWidthConfidenceInterval << " [" << stat.min << ", " << stat.max << "]"
            << " (n=" << stat.numReplications << ")\n";
    }

    out << "  Counters (" << result.counters.size() << "):\n";
    for (const AggregatedCounter& counter : result.counters) {
        out << "    " << counter.name << ": total=" << counter.total << "\n";
    }

    if (!result.failures.empty()) {
        out << "  Failures (" << result.failures.size() << "):\n";
        for (const std::string& failure : result.failures) {
            out << "    - " << failure << "\n";
        }
    }

    return out.str();
}

std::string CoordinatorApplication::renderJson(const AggregatedResult& result) const {
    std::string json = "{";
    json += "\"totalReplicationsRequested\":" + std::to_string(result.totalReplicationsRequested);
    json += ",\"totalReplicationsCompleted\":" + std::to_string(result.totalReplicationsCompleted);

    json += ",\"statistics\":[";
    for (std::size_t i = 0; i < result.statistics.size(); ++i) {
        const AggregatedStatistic& stat = result.statistics[i];
        if (i > 0) {
            json += ",";
        }
        json += "{\"name\":\"" + json::escape(stat.name) + "\"";
        json += ",\"numReplications\":" + std::to_string(stat.numReplications);
        json += ",\"average\":" + json::number(stat.average);
        json += ",\"variance\":" + json::number(stat.variance);
        json += ",\"stddev\":" + json::number(stat.stddev);
        json += ",\"min\":" + json::number(stat.min);
        json += ",\"max\":" + json::number(stat.max);
        json += ",\"halfWidthConfidenceInterval\":" + json::number(stat.halfWidthConfidenceInterval);
        json += ",\"confidenceLevel\":" + json::number(stat.confidenceLevel);
        json += "}";
    }
    json += "]";

    json += ",\"counters\":[";
    for (std::size_t i = 0; i < result.counters.size(); ++i) {
        if (i > 0) {
            json += ",";
        }
        json += "{\"name\":\"" + json::escape(result.counters[i].name) + "\"";
        json += ",\"total\":" + json::number(result.counters[i].total) + "}";
    }
    json += "]";

    json += ",\"failures\":[";
    for (std::size_t i = 0; i < result.failures.size(); ++i) {
        if (i > 0) {
            json += ",";
        }
        json += "\"" + json::escape(result.failures[i]) + "\"";
    }
    json += "]";

    json += "}";
    return json;
}

} // namespace genesys::distributed
