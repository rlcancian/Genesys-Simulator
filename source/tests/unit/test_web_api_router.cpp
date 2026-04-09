#include <gtest/gtest.h>

#include "applications/web/api/ApiRouter.h"
#include "applications/web/auth/TokenService.h"
#include "applications/web/service/SimulatorSessionService.h"
#include "applications/web/session/SessionManager.h"

#include <memory>
#include <string>

namespace {
struct ApiRouterFixture {
    TokenService tokenService;
    SessionManager sessionManager{tokenService, [] { return std::make_unique<Simulator>(); }};
    SimulatorSessionService simulatorService{sessionManager};
    ApiRouter router{simulatorService};
};
}  // namespace

TEST(WebSessionManagerTest, CreateSessionProducesUniqueTokens) {
    TokenService tokenService;
    SessionManager sessionManager(tokenService, [] { return std::make_unique<Simulator>(); });

    SessionContext* first = sessionManager.createSession();
    SessionContext* second = sessionManager.createSession();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    EXPECT_NE(first->sessionId, second->sessionId);
    EXPECT_NE(first->accessToken, second->accessToken);
}

TEST(WebApiRouterTest, HealthEndpointReturnsUp) {
    ApiRouterFixture fixture;

    HttpRequest request;
    request.method = "GET";
    request.path = "/health";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 200);
    EXPECT_NE(response.body.find("\"ok\":true"), std::string::npos);
    EXPECT_NE(response.body.find("\"status\":\"up\""), std::string::npos);
}

TEST(WebApiRouterTest, AuthSessionThenSimulatorInfoWithBearerToken) {
    ApiRouterFixture fixture;

    HttpRequest createRequest;
    createRequest.method = "POST";
    createRequest.path = "/api/v1/auth/session";

    const HttpResponse createResponse = fixture.router.handle(createRequest);
    ASSERT_EQ(createResponse.status, 201);

    const std::string marker = "\"accessToken\":\"";
    const auto tokenStart = createResponse.body.find(marker);
    ASSERT_NE(tokenStart, std::string::npos);
    const auto valueStart = tokenStart + marker.size();
    const auto valueEnd = createResponse.body.find('"', valueStart);
    ASSERT_NE(valueEnd, std::string::npos);
    const std::string token = createResponse.body.substr(valueStart, valueEnd - valueStart);
    ASSERT_FALSE(token.empty());

    HttpRequest infoRequest;
    infoRequest.method = "GET";
    infoRequest.path = "/api/v1/simulator/info";
    infoRequest.headers["authorization"] = "Bearer " + token;

    const HttpResponse infoResponse = fixture.router.handle(infoRequest);

    EXPECT_EQ(infoResponse.status, 200);
    EXPECT_NE(infoResponse.body.find("\"name\":"), std::string::npos);
    EXPECT_NE(infoResponse.body.find("\"versionNumber\":"), std::string::npos);
}

TEST(WebApiRouterTest, SimulatorInfoWithoutTokenReturnsUnauthorized) {
    ApiRouterFixture fixture;

    HttpRequest infoRequest;
    infoRequest.method = "GET";
    infoRequest.path = "/api/v1/simulator/info";

    const HttpResponse infoResponse = fixture.router.handle(infoRequest);

    EXPECT_EQ(infoResponse.status, 401);
    EXPECT_NE(infoResponse.body.find("\"ok\":false"), std::string::npos);
}
