#include "RemoteSimulationExecutor.h"

#include "Json.h"

namespace genesys::distributed {

namespace {
constexpr const char* kSessionPath = "/api/v1/auth/session";
constexpr const char* kImportPath = "/api/v1/worker/models/import-language";
constexpr const char* kJobsPath = "/api/v1/worker/jobs";

// Parses the worker /result body into a BatchResult (statistics and counters arrays).
void parseResultInto(const std::string& body, BatchResult& result) {
    result.numberOfReplications =
        static_cast<unsigned int>(json::getInt(body, "numberOfReplications").value_or(0));

    if (const auto statisticsArray = json::getArray(body, "statistics"); statisticsArray.has_value()) {
        for (const std::string& object : json::splitObjects(statisticsArray.value())) {
            CollectorStat stat;
            stat.name = json::getString(object, "name").value_or("");
            stat.numReplications = static_cast<unsigned int>(json::getInt(object, "numReplications").value_or(0));
            stat.average = json::getDouble(object, "average").value_or(0.0);
            stat.variance = json::getDouble(object, "variance").value_or(0.0);
            stat.min = json::getDouble(object, "min").value_or(0.0);
            stat.max = json::getDouble(object, "max").value_or(0.0);
            stat.numObservations = static_cast<unsigned int>(json::getInt(object, "numObservations").value_or(0));
            result.statistics.push_back(stat);
        }
    }

    if (const auto countersArray = json::getArray(body, "counters"); countersArray.has_value()) {
        for (const std::string& object : json::splitObjects(countersArray.value())) {
            CounterStat counter;
            counter.name = json::getString(object, "name").value_or("");
            counter.total = json::getDouble(object, "total").value_or(0.0);
            result.counters.push_back(counter);
        }
    }
}
} // namespace

RemoteSimulationExecutor::RemoteSimulationExecutor(WorkerHttpClient& client, std::string host, int port)
    : _client(client), _host(std::move(host)), _port(port) {}

BatchResult RemoteSimulationExecutor::_failure(const std::string& error) {
    BatchResult result;
    result.success = false;
    result.error = error;
    return result;
}

bool RemoteSimulationExecutor::_ensureSession(std::string& error) {
    if (!_token.empty()) {
        return true;
    }
    const HttpClientResponse response = _client.post(_host, _port, kSessionPath, "");
    if (!response.ok || response.status != 201) {
        error = response.ok ? ("session creation returned status " + std::to_string(response.status))
                            : response.error;
        return false;
    }
    const auto token = json::getString(response.body, "accessToken");
    if (!token.has_value() || token->empty()) {
        error = "session response missing accessToken";
        return false;
    }
    _token = token.value();
    return true;
}

BatchResult RemoteSimulationExecutor::execute(const DistributedSimulationJob& job) {
    std::string sessionError;
    if (!_ensureSession(sessionError)) {
        return _failure("worker " + _host + ":" + std::to_string(_port) + " session failed: " + sessionError);
    }

    // Import the model into the session.
    const HttpClientResponse import =
        _client.post(_host, _port, kImportPath, job.modelText, "text/plain", _token);
    if (!import.ok || import.status != 200) {
        return _failure(import.ok ? ("model import returned status " + std::to_string(import.status))
                                  : import.error);
    }

    // Create the job configured with this batch's replication count and seed.
    const std::string jobBody = "{\"numberOfReplications\":" +
                                std::to_string(job.batch.numberOfReplications) +
                                ",\"seed\":" + std::to_string(job.batch.seed) + "}";
    const HttpClientResponse created =
        _client.post(_host, _port, kJobsPath, jobBody, "application/json", _token);
    if (!created.ok || created.status != 201) {
        return _failure(created.ok ? ("job creation returned status " + std::to_string(created.status))
                                   : created.error);
    }
    const auto jobId = json::getString(created.body, "jobId");
    if (!jobId.has_value() || jobId->empty()) {
        return _failure("job creation response missing jobId");
    }

    // Run the job synchronously; the worker returns 200 only when the run finished.
    const std::string runPath = std::string(kJobsPath) + "/" + jobId.value() + "/run";
    const HttpClientResponse run = _client.post(_host, _port, runPath, "", "application/json", _token);
    if (!run.ok || run.status != 200) {
        const std::string message = json::getString(run.body, "message").value_or("");
        return _failure(run.ok ? ("job run failed (status " + std::to_string(run.status) +
                                  (message.empty() ? ")" : "): " + message))
                               : run.error);
    }

    // Retrieve the terminal result with the per-replication statistics.
    const std::string resultPath = std::string(kJobsPath) + "/" + jobId.value() + "/result";
    const HttpClientResponse resultResponse = _client.get(_host, _port, resultPath, _token);
    if (!resultResponse.ok || resultResponse.status != 200) {
        return _failure(resultResponse.ok
                            ? ("result retrieval returned status " + std::to_string(resultResponse.status))
                            : resultResponse.error);
    }

    const std::string state = json::getString(resultResponse.body, "state").value_or("");
    if (state == "failed") {
        const std::string message = json::getString(resultResponse.body, "message").value_or("simulation failed");
        return _failure("worker reported job failure: " + message);
    }

    BatchResult result;
    result.success = true;
    parseResultInto(resultResponse.body, result);
    if (result.numberOfReplications == 0) {
        result.numberOfReplications = job.batch.numberOfReplications;
    }
    return result;
}

} // namespace genesys::distributed
