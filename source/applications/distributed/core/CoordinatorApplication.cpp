#include "CoordinatorApplication.h"

#include "Json.h"

#include <cmath>
#include <iomanip>
#include <sstream>

namespace genesys::distributed {

namespace {

constexpr int kRuleWidth = 88;

// Repeats a (possibly multi-byte UTF-8) unit `times` times — used to draw horizontal rules.
std::string repeat(const char* unit, int times) {
    std::string out;
    for (int i = 0; i < times; ++i) {
        out += unit;
    }
    return out;
}

// Compact human-readable real: up to 5 significant digits, no trailing-zero noise.
std::string num(double value) {
    std::ostringstream stream;
    stream << std::setprecision(5) << value;
    return stream.str();
}

// Counters are integral counts; render them without a decimal point.
std::string integer(double value) {
    return std::to_string(static_cast<long long>(std::llround(value)));
}

} // namespace

AggregatedResult CoordinatorApplication::execute(const DistributedSimulationConfig& config) const {
    DistributedSimulationManager manager;
    return manager.run(config);
}

std::string CoordinatorApplication::renderSummary(const AggregatedResult& result) const {
    const std::string heavyRule = repeat("═", kRuleWidth);          // ═
    const std::string lightRule = "  " + repeat("─", kRuleWidth - 2); // ─

    std::ostringstream out;
    out << heavyRule << "\n";
    out << "  DISTRIBUTED SIMULATION RESULT\n";
    out << heavyRule << "\n";

    const unsigned int requested = result.totalReplicationsRequested;
    const unsigned int completed = result.totalReplicationsCompleted;
    out << "  Replications   " << completed << " / " << requested << " completed";
    if (requested > 0) {
        out << "  (" << static_cast<int>((100.0 * completed / requested) + 0.5) << "%)";
    }
    out << "\n\n";

    // Statistics: one aligned row per collector (mean / std dev / CI / range).
    out << "  STATISTICS  ·  " << result.statistics.size()
        << (result.statistics.size() == 1 ? " collector\n" : " collectors\n");
    out << lightRule << "\n";
    if (result.statistics.empty()) {
        out << "  (none)\n";
    } else {
        out << "  " << std::left << std::setw(32) << "Collector"
            << std::right << std::setw(9) << "Mean" << std::setw(11) << "Std Dev"
            << std::setw(16) << "95% CI" << std::setw(9) << "Min" << std::setw(9) << "Max" << "\n";
        for (const AggregatedStatistic& stat : result.statistics) {
            const std::string ci = "+/- " + num(stat.halfWidthConfidenceInterval);
            out << "  " << std::left << std::setw(32) << stat.name
                << std::right << std::setw(9) << num(stat.average)
                << std::setw(11) << num(stat.stddev) << std::setw(16) << ci
                << std::setw(9) << num(stat.min) << std::setw(9) << num(stat.max) << "\n";
        }
    }
    out << "\n";

    // Counters: name and summed total.
    out << "  COUNTERS  ·  " << result.counters.size() << "\n";
    out << lightRule << "\n";
    if (result.counters.empty()) {
        out << "  (none)\n";
    } else {
        for (const AggregatedCounter& counter : result.counters) {
            out << "  " << std::left << std::setw(32) << counter.name
                << std::right << std::setw(14) << integer(counter.total) << "\n";
        }
    }
    out << "\n";

    // Workers: discovery state and the share each target actually completed.
    out << "  WORKERS  ·  " << result.workers.size() << "\n";
    out << lightRule << "\n";
    for (const WorkerReport& worker : result.workers) {
        out << "  " << std::left << std::setw(19) << worker.endpoint
            << std::setw(13) << worker.state
            << std::right << std::setw(8) << worker.replicationsCompleted << " reps";
        if (!worker.isLocal) {
            if (worker.latencyMs >= 0) {
                out << std::setw(7) << worker.latencyMs << " ms";
            } else {
                out << std::setw(10) << "";
            }
            out << std::setw(5) << worker.failureCount << " failures";
        }
        out << "\n";
    }

    if (!result.failures.empty()) {
        out << "\n  FAILURES  ·  " << result.failures.size() << "\n";
        out << lightRule << "\n";
        for (const std::string& failure : result.failures) {
            out << "  - " << failure << "\n";
        }
    }

    out << heavyRule << "\n";
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

    json += ",\"workers\":[";
    for (std::size_t i = 0; i < result.workers.size(); ++i) {
        const WorkerReport& worker = result.workers[i];
        if (i > 0) {
            json += ",";
        }
        json += "{\"endpoint\":\"" + json::escape(worker.endpoint) + "\"";
        json += ",\"isLocal\":" + std::string(worker.isLocal ? "true" : "false");
        json += ",\"state\":\"" + json::escape(worker.state) + "\"";
        json += ",\"latencyMs\":" + std::to_string(worker.latencyMs);
        json += ",\"failureCount\":" + std::to_string(worker.failureCount);
        json += ",\"replicationsCompleted\":" + std::to_string(worker.replicationsCompleted);
        json += "}";
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
