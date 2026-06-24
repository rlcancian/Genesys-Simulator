#include <gtest/gtest.h>

#include "applications/distributed/core/WorkerDiscoveryService.h"
#include "applications/distributed/core/WorkerHttpClient.h"
#include "applications/distributed/core/WorkerRegistry.h"

#include <map>
#include <string>

using namespace genesys::distributed;

namespace {

const std::string kInfoOk =
    "{\"ok\":true,\"data\":{\"role\":\"worker\",\"application\":\"genesys_web_app\","
    "\"apiVersion\":\"v1\",\"simulatorName\":\"GenESyS\",\"simulatorVersionNumber\":260330}}";

const std::string kCapabilitiesOk =
    "{\"ok\":true,\"data\":{\"supportsDistributedJobs\":true,\"supportsJobPolling\":true,"
    "\"supportsJobResultRetrieval\":true,\"supportsSynchronousRun\":true,\"supportsModelCreation\":true}}";

const std::string kCapabilitiesIncompatible =
    "{\"ok\":true,\"data\":{\"supportsDistributedJobs\":false,\"supportsJobResultRetrieval\":false}}";

// Fake client returning canned responses keyed by "host:port path".
class FakeHttpClient : public WorkerHttpClient {
public:
    std::map<std::string, HttpClientResponse> responses;

    HttpClientResponse get(const std::string& host,
                           int port,
                           const std::string& path,
                           const std::string& /*bearerToken*/ = "") const override {
        const std::string key = host + ":" + std::to_string(port) + " " + path;
        const auto iterator = responses.find(key);
        if (iterator == responses.end()) {
            HttpClientResponse failure;
            failure.ok = false;
            failure.error = "Connect failed";
            return failure;
        }
        return iterator->second;
    }
};

HttpClientResponse okResponse(const std::string& body) {
    HttpClientResponse response;
    response.ok = true;
    response.status = 200;
    response.body = body;
    return response;
}

} // namespace

TEST(WorkerDiscovery, ShouldRegisterCompatibleWorkerAsAvailable) {
    // Arrange
    FakeHttpClient client;
    client.responses["host-a:8080 /api/v1/worker/info"] = okResponse(kInfoOk);
    client.responses["host-a:8080 /api/v1/worker/capabilities"] = okResponse(kCapabilitiesOk);
    WorkerRegistry registry;
    WorkerDiscoveryService discovery(client, registry);

    // Act
    const int available = discovery.discover({{"host-a", 8080}});

    // Assert
    EXPECT_EQ(available, 1);
    const auto worker = registry.find("host-a", 8080);
    ASSERT_TRUE(worker.has_value());
    EXPECT_EQ(worker->state, WorkerState::Available);
    EXPECT_EQ(worker->role, "worker");
    EXPECT_TRUE(worker->capabilities.supportsDistributedJobs);
}

TEST(WorkerDiscovery, ShouldRegisterMultipleAvailableWorkers) {
    // Arrange: two reachable, compatible workers.
    FakeHttpClient client;
    client.responses["host-a:8080 /api/v1/worker/info"] = okResponse(kInfoOk);
    client.responses["host-a:8080 /api/v1/worker/capabilities"] = okResponse(kCapabilitiesOk);
    client.responses["host-b:8081 /api/v1/worker/info"] = okResponse(kInfoOk);
    client.responses["host-b:8081 /api/v1/worker/capabilities"] = okResponse(kCapabilitiesOk);
    WorkerRegistry registry;
    WorkerDiscoveryService discovery(client, registry);

    // Act
    const int available = discovery.discover({{"host-a", 8080}, {"host-b", 8081}});

    // Assert: both workers registered and available.
    EXPECT_EQ(available, 2);
    EXPECT_EQ(registry.available().size(), 2u);
    EXPECT_EQ(registry.find("host-a", 8080)->state, WorkerState::Available);
    EXPECT_EQ(registry.find("host-b", 8081)->state, WorkerState::Available);
}

TEST(WorkerDiscovery, ShouldMarkUnreachableEndpointUnavailableWithoutAborting) {
    // Arrange: host-a is reachable, host-b has no canned response (connection fails).
    FakeHttpClient client;
    client.responses["host-a:8080 /api/v1/worker/info"] = okResponse(kInfoOk);
    client.responses["host-a:8080 /api/v1/worker/capabilities"] = okResponse(kCapabilitiesOk);
    WorkerRegistry registry;
    WorkerDiscoveryService discovery(client, registry);

    // Act
    const int available = discovery.discover({{"host-a", 8080}, {"host-b", 9999}});

    // Assert: discovery did not abort; both endpoints are registered, only one available.
    EXPECT_EQ(available, 1);
    EXPECT_EQ(registry.size(), 2u);
    EXPECT_EQ(registry.find("host-a", 8080)->state, WorkerState::Available);
    const auto unreachable = registry.find("host-b", 9999);
    ASSERT_TRUE(unreachable.has_value());
    EXPECT_EQ(unreachable->state, WorkerState::Unavailable);
    EXPECT_FALSE(unreachable->lastError.empty());
}

TEST(WorkerDiscovery, ShouldRejectIncompatibleWorker) {
    // Arrange
    FakeHttpClient client;
    client.responses["host-a:8080 /api/v1/worker/info"] = okResponse(kInfoOk);
    client.responses["host-a:8080 /api/v1/worker/capabilities"] = okResponse(kCapabilitiesIncompatible);
    WorkerRegistry registry;
    WorkerDiscoveryService discovery(client, registry);

    // Act
    const int available = discovery.discover({{"host-a", 8080}});

    // Assert
    EXPECT_EQ(available, 0);
    EXPECT_EQ(registry.find("host-a", 8080)->state, WorkerState::Unavailable);
}

TEST(WorkerDiscovery, ShouldMarkUnavailableWhenInfoReturnsErrorStatus) {
    // Arrange
    FakeHttpClient client;
    HttpClientResponse serverError;
    serverError.ok = true;
    serverError.status = 500;
    serverError.body = "{}";
    client.responses["host-a:8080 /api/v1/worker/info"] = serverError;
    WorkerRegistry registry;
    WorkerDiscoveryService discovery(client, registry);

    // Act
    const int available = discovery.discover({{"host-a", 8080}});

    // Assert
    EXPECT_EQ(available, 0);
    EXPECT_EQ(registry.find("host-a", 8080)->state, WorkerState::Unavailable);
}
