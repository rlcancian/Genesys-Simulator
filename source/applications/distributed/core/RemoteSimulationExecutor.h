#pragma once

#include "SimulationExecutor.h"
#include "WorkerHttpClient.h"

#include <string>

namespace genesys::distributed {

/**
 * @brief Runs a replication batch on a remote worker by driving its HTTP job lifecycle.
 *
 * Bound to a single worker endpoint. For each batch it (lazily) creates a session, imports the
 * model, creates a job configured with the batch's replication count and seed, runs it, and
 * retrieves the statistics from the worker result. Any transport or worker error is mapped to a
 * failed BatchResult rather than throwing, so the execution manager can retry elsewhere.
 */
class RemoteSimulationExecutor : public SimulationExecutor {
public:
    RemoteSimulationExecutor(WorkerHttpClient& client, std::string host, int port);

    BatchResult execute(const DistributedSimulationJob& job) override;

private:
    bool _ensureSession(std::string& error);
    static BatchResult _failure(const std::string& error);

    WorkerHttpClient& _client;
    std::string _host;
    int _port;
    std::string _token;  // Reused across batches once the session is created.
};

} // namespace genesys::distributed
