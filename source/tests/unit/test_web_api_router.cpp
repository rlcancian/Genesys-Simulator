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

std::string extractJsonStringField(const std::string& json, const std::string& fieldName) {
    const std::string marker = "\"" + fieldName + "\":\"";
    const auto tokenStart = json.find(marker);
    if (tokenStart == std::string::npos) {
        return {};
    }

    const auto valueStart = tokenStart + marker.size();
    const auto valueEnd = json.find('"', valueStart);
    if (valueEnd == std::string::npos) {
        return {};
    }

    return json.substr(valueStart, valueEnd - valueStart);
}
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

    const std::string token = extractJsonStringField(createResponse.body, "accessToken");
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

TEST(WebApiRouterTest, CurrentModelWithoutTokenReturnsUnauthorized) {
    ApiRouterFixture fixture;

    HttpRequest request;
    request.method = "GET";
    request.path = "/api/v1/models/current";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 401);
    EXPECT_NE(response.body.find("\"ok\":false"), std::string::npos);
}

TEST(WebApiRouterTest, CurrentModelAfterSessionCreationReturnsExistsFalse) {
    ApiRouterFixture fixture;

    HttpRequest createRequest;
    createRequest.method = "POST";
    createRequest.path = "/api/v1/auth/session";
    const HttpResponse createResponse = fixture.router.handle(createRequest);
    ASSERT_EQ(createResponse.status, 201);

    const std::string token = extractJsonStringField(createResponse.body, "accessToken");
    ASSERT_FALSE(token.empty());

    HttpRequest currentRequest;
    currentRequest.method = "GET";
    currentRequest.path = "/api/v1/models/current";
    currentRequest.headers["authorization"] = "Bearer " + token;

    const HttpResponse currentResponse = fixture.router.handle(currentRequest);

    EXPECT_EQ(currentResponse.status, 200);
    EXPECT_NE(currentResponse.body.find("\"ok\":true"), std::string::npos);
    EXPECT_NE(currentResponse.body.find("\"exists\":false"), std::string::npos);
}

TEST(WebApiRouterTest, CreateModelWithTokenSucceeds) {
    ApiRouterFixture fixture;

    HttpRequest createSessionRequest;
    createSessionRequest.method = "POST";
    createSessionRequest.path = "/api/v1/auth/session";
    const HttpResponse createSessionResponse = fixture.router.handle(createSessionRequest);
    ASSERT_EQ(createSessionResponse.status, 201);

    const std::string token = extractJsonStringField(createSessionResponse.body, "accessToken");
    ASSERT_FALSE(token.empty());

    HttpRequest createModelRequest;
    createModelRequest.method = "POST";
    createModelRequest.path = "/api/v1/models";
    createModelRequest.headers["authorization"] = "Bearer " + token;

    const HttpResponse createModelResponse = fixture.router.handle(createModelRequest);

    EXPECT_EQ(createModelResponse.status, 201);
    EXPECT_NE(createModelResponse.body.find("\"ok\":true"), std::string::npos);
    EXPECT_NE(createModelResponse.body.find("\"exists\":true"), std::string::npos);
    EXPECT_NE(createModelResponse.body.find("\"modelId\":"), std::string::npos);
}

TEST(WebApiRouterTest, CurrentModelAfterCreationReturnsExpectedFields) {
    ApiRouterFixture fixture;

    HttpRequest createSessionRequest;
    createSessionRequest.method = "POST";
    createSessionRequest.path = "/api/v1/auth/session";
    const HttpResponse createSessionResponse = fixture.router.handle(createSessionRequest);
    ASSERT_EQ(createSessionResponse.status, 201);

    const std::string token = extractJsonStringField(createSessionResponse.body, "accessToken");
    ASSERT_FALSE(token.empty());

    HttpRequest createModelRequest;
    createModelRequest.method = "POST";
    createModelRequest.path = "/api/v1/models";
    createModelRequest.headers["authorization"] = "Bearer " + token;
    const HttpResponse createModelResponse = fixture.router.handle(createModelRequest);
    ASSERT_EQ(createModelResponse.status, 201);

    HttpRequest currentRequest;
    currentRequest.method = "GET";
    currentRequest.path = "/api/v1/models/current";
    currentRequest.headers["authorization"] = "Bearer " + token;

    const HttpResponse currentResponse = fixture.router.handle(currentRequest);

    EXPECT_EQ(currentResponse.status, 200);
    EXPECT_NE(currentResponse.body.find("\"ok\":true"), std::string::npos);
    EXPECT_NE(currentResponse.body.find("\"exists\":true"), std::string::npos);
    EXPECT_NE(currentResponse.body.find("\"modelId\":"), std::string::npos);
    EXPECT_NE(currentResponse.body.find("\"hasChanged\":"), std::string::npos);
    EXPECT_NE(currentResponse.body.find("\"level\":"), std::string::npos);
    EXPECT_NE(currentResponse.body.find("\"name\":"), std::string::npos);
    EXPECT_NE(currentResponse.body.find("\"analystName\":"), std::string::npos);
    EXPECT_NE(currentResponse.body.find("\"projectTitle\":"), std::string::npos);
    EXPECT_NE(currentResponse.body.find("\"version\":"), std::string::npos);
    EXPECT_NE(currentResponse.body.find("\"description\":"), std::string::npos);
    EXPECT_NE(currentResponse.body.find("\"componentCount\":"), std::string::npos);
}
