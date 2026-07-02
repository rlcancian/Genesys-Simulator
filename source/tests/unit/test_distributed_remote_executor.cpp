// Unit tests for RemoteSimulationExecutor — the component that submits a job to a remote worker
// over HTTP (Tema 13 §14.2 "submissão correta de jobs a workers remotos", and §14.5 timeout/failure
// at the remote level). WorkerHttpClient is a virtual interface, so a fake client returns canned
// responses for the whole session -> import -> create job -> run -> result lifecycle without a
// network or a live worker.

#include <gtest/gtest.h>

#include "applications/distributed/core/RemoteSimulationExecutor.h"
#include "applications/distributed/core/WorkerHttpClient.h"

#include <string>

using namespace genesys::distributed;

namespace {

HttpClientResponse okResponse(int status, const std::string& body) {
    HttpClientResponse response;
    response.ok = true;
    response.status = status;
    response.body = body;
    return response;
}

HttpClientResponse transportError(const std::string& error) {
    HttpClientResponse response;
    response.ok = false;  // no HTTP response received (connect/timeout/read failure)
    response.status = 0;
    response.error = error;
    return response;
}

const std::string kResultBody =
    "{\"state\":\"completed\",\"numberOfReplications\":5,"
    "\"statistics\":[{\"name\":\"Queue.Wait\",\"numReplications\":5,\"average\":2.5,"
    "\"variance\":0.25,\"min\":1.0,\"max\":4.0,\"numObservations\":5}],"
    "\"counters\":[{\"name\":\"Out.Count\",\"total\":42.0}]}";

// Fake worker driving the full job lifecycle. Each step returns a configurable canned response;
// a test overrides one step to simulate a failure. Routing is by path substring (most specific
// first: "/run" before the generic "/jobs"). Call counters expose what the executor actually did.
class FakeWorker : public WorkerHttpClient {
public:
    HttpClientResponse session = okResponse(201, "{\"accessToken\":\"tok-123\"}");
    HttpClientResponse import = okResponse(200, "{\"componentCount\":3}");
    HttpClientResponse createJob = okResponse(201, "{\"jobId\":\"job-1\"}");
    HttpClientResponse run = okResponse(200, "{}");
    HttpClientResponse result = okResponse(200, kResultBody);

    mutable int sessionCalls = 0;
    mutable int importCalls = 0;
    mutable int createCalls = 0;
    mutable int runCalls = 0;
    mutable int resultCalls = 0;

    HttpClientResponse post(const std::string& /*host*/, int /*port*/, const std::string& path,
                            const std::string& /*body*/, const std::string& /*contentType*/ = "application/json",
                            const std::string& /*bearerToken*/ = "") const override {
        if (path.find("/auth/session") != std::string::npos) { ++sessionCalls; return session; }
        if (path.find("/import-language") != std::string::npos) { ++importCalls; return import; }
        if (path.find("/run") != std::string::npos) { ++runCalls; return run; }
        if (path.find("/jobs") != std::string::npos) { ++createCalls; return createJob; }
        return transportError("unexpected POST " + path);
    }

    HttpClientResponse get(const std::string& /*host*/, int /*port*/, const std::string& path,
                           const std::string& /*bearerToken*/ = "") const override {
        if (path.find("/result") != std::string::npos) { ++resultCalls; return result; }
        return transportError("unexpected GET " + path);
    }
};

DistributedSimulationJob job(unsigned int replications, std::uint32_t seed) {
    DistributedSimulationJob j;
    j.modelText = "0 ModelInfo \"M\"";
    j.batch.targetEndpoint = "127.0.0.1:8101";
    j.batch.numberOfReplications = replications;
    j.batch.seed = seed;
    return j;
}

} // namespace

TEST(RemoteSimulationExecutor, ShouldDriveFullJobLifecycleAndParseResult) {
    // Arrange
    FakeWorker worker;
    RemoteSimulationExecutor executor(worker, "127.0.0.1", 8101);

    // Act
    const BatchResult result = executor.execute(job(5, 99));

    // Assert: every lifecycle step was exercised exactly once.
    EXPECT_EQ(worker.sessionCalls, 1);
    EXPECT_EQ(worker.importCalls, 1);
    EXPECT_EQ(worker.createCalls, 1);
    EXPECT_EQ(worker.runCalls, 1);
    EXPECT_EQ(worker.resultCalls, 1);

    // Assert: the worker /result was parsed into statistics and counters.
    ASSERT_TRUE(result.success) << result.error;
    EXPECT_EQ(result.numberOfReplications, 5u);
    ASSERT_EQ(result.statistics.size(), 1u);
    EXPECT_EQ(result.statistics[0].name, "Queue.Wait");
    EXPECT_EQ(result.statistics[0].numReplications, 5u);
    EXPECT_DOUBLE_EQ(result.statistics[0].average, 2.5);
    EXPECT_DOUBLE_EQ(result.statistics[0].variance, 0.25);
    EXPECT_DOUBLE_EQ(result.statistics[0].min, 1.0);
    EXPECT_DOUBLE_EQ(result.statistics[0].max, 4.0);
    ASSERT_EQ(result.counters.size(), 1u);
    EXPECT_EQ(result.counters[0].name, "Out.Count");
    EXPECT_DOUBLE_EQ(result.counters[0].total, 42.0);
}

TEST(RemoteSimulationExecutor, ShouldClassifyImport4xxAsModelRejected) {
    // Arrange: the worker is healthy but refuses the model itself (invalid specification).
    FakeWorker worker;
    worker.import = okResponse(400, "{\"message\":\"INVALID_MODEL_SPECIFICATION\"}");
    RemoteSimulationExecutor executor(worker, "127.0.0.1", 8101);

    // Act
    const BatchResult result = executor.execute(job(5, 1));

    // Assert: rejection is classified as ModelRejected (no failover would help) with the real reason,
    // and the executor stops before creating/running a job.
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.failureKind, FailureKind::ModelRejected);
    EXPECT_NE(result.error.find("INVALID_MODEL_SPECIFICATION"), std::string::npos);
    EXPECT_EQ(worker.createCalls, 0);
    EXPECT_EQ(worker.runCalls, 0);
}

TEST(RemoteSimulationExecutor, ShouldTreatConnectFailureAsWorkerUnavailable) {
    // Arrange: the session call never gets an HTTP response (connection refused / dead worker).
    FakeWorker worker;
    worker.session = transportError("Connect failed");
    RemoteSimulationExecutor executor(worker, "127.0.0.1", 8101);

    // Act
    const BatchResult result = executor.execute(job(5, 1));

    // Assert: a transport failure marks the worker unavailable (eligible for failover elsewhere).
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.failureKind, FailureKind::WorkerUnavailable);
    EXPECT_EQ(worker.importCalls, 0);
}

TEST(RemoteSimulationExecutor, ShouldTreatRunTimeoutAsWorkerUnavailable) {
    // Arrange: the worker accepts the job but the synchronous /run never returns in time
    // (Tema 13 §14.5: "worker que aceita job mas não devolve resultado no tempo esperado").
    FakeWorker worker;
    worker.run = transportError("recv timeout");
    RemoteSimulationExecutor executor(worker, "127.0.0.1", 8101);

    // Act
    const BatchResult result = executor.execute(job(5, 1));

    // Assert: a job was created and run attempted, but the timeout is a worker-availability problem.
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.failureKind, FailureKind::WorkerUnavailable);
    EXPECT_EQ(worker.createCalls, 1);
    EXPECT_EQ(worker.runCalls, 1);
    EXPECT_EQ(worker.resultCalls, 0);
}

TEST(RemoteSimulationExecutor, ShouldReportFailureWhenWorkerJobStateIsFailed) {
    // Arrange: the run/result succeed at the HTTP level, but the job itself reports a failed state.
    FakeWorker worker;
    worker.result = okResponse(200, "{\"state\":\"failed\",\"message\":\"simulation crashed\"}");
    RemoteSimulationExecutor executor(worker, "127.0.0.1", 8101);

    // Act
    const BatchResult result = executor.execute(job(5, 1));

    // Assert: surfaced as a failure carrying the worker-reported reason.
    EXPECT_FALSE(result.success);
    EXPECT_NE(result.error.find("simulation crashed"), std::string::npos);
}

TEST(RemoteSimulationExecutor, ShouldFallBackToBatchReplicationsWhenResultOmitsCount) {
    // Arrange: a result without an explicit numberOfReplications falls back to the batch size.
    FakeWorker worker;
    worker.result = okResponse(200, "{\"state\":\"completed\",\"statistics\":[],\"counters\":[]}");
    RemoteSimulationExecutor executor(worker, "127.0.0.1", 8101);

    // Act
    const BatchResult result = executor.execute(job(7, 1));

    // Assert
    ASSERT_TRUE(result.success) << result.error;
    EXPECT_EQ(result.numberOfReplications, 7u);
}
