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

std::string createSessionAndGetToken(ApiRouter& router) {
    HttpRequest createSessionRequest;
    createSessionRequest.method = "POST";
    createSessionRequest.path = "/api/v1/auth/session";
    const HttpResponse createSessionResponse = router.handle(createSessionRequest);
    EXPECT_EQ(createSessionResponse.status, 201);

    const std::string token = extractJsonStringField(createSessionResponse.body, "accessToken");
    EXPECT_FALSE(token.empty());
    return token;
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

TEST(WebApiRouterTest, SaveModelWithoutTokenReturnsUnauthorized) {
    ApiRouterFixture fixture;

    HttpRequest request;
    request.method = "POST";
    request.path = "/api/v1/models/save";
    request.body = "{\"filename\":\"model.gen\"}";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 401);
    EXPECT_NE(response.body.find("\"ok\":false"), std::string::npos);
}

TEST(WebApiRouterTest, LoadModelWithoutTokenReturnsUnauthorized) {
    ApiRouterFixture fixture;

    HttpRequest request;
    request.method = "POST";
    request.path = "/api/v1/models/load";
    request.body = "{\"filename\":\"model.gen\"}";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 401);
    EXPECT_NE(response.body.find("\"ok\":false"), std::string::npos);
}

TEST(WebApiRouterTest, SaveModelWithInvalidBodyReturnsBadRequest) {
    ApiRouterFixture fixture;
    const std::string token = createSessionAndGetToken(fixture.router);

    HttpRequest request;
    request.method = "POST";
    request.path = "/api/v1/models/save";
    request.headers["authorization"] = "Bearer " + token;
    request.body = "{}";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 400);
    EXPECT_NE(response.body.find("\"ok\":false"), std::string::npos);
}

TEST(WebApiRouterTest, LoadModelWithInvalidBodyReturnsBadRequest) {
    ApiRouterFixture fixture;
    const std::string token = createSessionAndGetToken(fixture.router);

    HttpRequest request;
    request.method = "POST";
    request.path = "/api/v1/models/load";
    request.headers["authorization"] = "Bearer " + token;
    request.body = "{\"filename\":42}";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 400);
    EXPECT_NE(response.body.find("\"ok\":false"), std::string::npos);
}

TEST(WebApiRouterTest, SaveModelWithInvalidFilenameReturnsBadRequest) {
    ApiRouterFixture fixture;
    const std::string token = createSessionAndGetToken(fixture.router);

    HttpRequest createModelRequest;
    createModelRequest.method = "POST";
    createModelRequest.path = "/api/v1/models";
    createModelRequest.headers["authorization"] = "Bearer " + token;
    ASSERT_EQ(fixture.router.handle(createModelRequest).status, 201);

    HttpRequest saveRequest;
    saveRequest.method = "POST";
    saveRequest.path = "/api/v1/models/save";
    saveRequest.headers["authorization"] = "Bearer " + token;
    saveRequest.body = "{\"filename\":\"../model.gen\"}";

    const HttpResponse saveResponse = fixture.router.handle(saveRequest);

    EXPECT_EQ(saveResponse.status, 400);
    EXPECT_NE(saveResponse.body.find("\"INVALID_FILENAME\""), std::string::npos);
}

TEST(WebApiRouterTest, SaveAndLoadModelInSessionWorkspaceSucceeds) {
    ApiRouterFixture fixture;
    const std::string token = createSessionAndGetToken(fixture.router);

    HttpRequest createModelRequest;
    createModelRequest.method = "POST";
    createModelRequest.path = "/api/v1/models";
    createModelRequest.headers["authorization"] = "Bearer " + token;
    ASSERT_EQ(fixture.router.handle(createModelRequest).status, 201);

    HttpRequest saveRequest;
    saveRequest.method = "POST";
    saveRequest.path = "/api/v1/models/save";
    saveRequest.headers["authorization"] = "Bearer " + token;
    saveRequest.headers["content-type"] = "application/json";
    saveRequest.body = "{\"filename\":\"model.gen\"}";
    const HttpResponse saveResponse = fixture.router.handle(saveRequest);
    ASSERT_EQ(saveResponse.status, 200);
    EXPECT_NE(saveResponse.body.find("\"ok\":true"), std::string::npos);
    EXPECT_NE(saveResponse.body.find("\"filename\":\"model.gen\""), std::string::npos);

    HttpRequest loadRequest;
    loadRequest.method = "POST";
    loadRequest.path = "/api/v1/models/load";
    loadRequest.headers["authorization"] = "Bearer " + token;
    loadRequest.headers["content-type"] = "application/json";
    loadRequest.body = "{\"filename\":\"model.gen\"}";
    const HttpResponse loadResponse = fixture.router.handle(loadRequest);
    EXPECT_EQ(loadResponse.status, 200);
    EXPECT_NE(loadResponse.body.find("\"ok\":true"), std::string::npos);
    EXPECT_NE(loadResponse.body.find("\"filename\":\"model.gen\""), std::string::npos);
    EXPECT_NE(loadResponse.body.find("\"exists\":true"), std::string::npos);
}

TEST(WebApiRouterTest, LoadModelForMissingFileReturnsNotFound) {
    ApiRouterFixture fixture;
    const std::string token = createSessionAndGetToken(fixture.router);

    HttpRequest request;
    request.method = "POST";
    request.path = "/api/v1/models/load";
    request.headers["authorization"] = "Bearer " + token;
    request.body = "{\"filename\":\"missing.gen\"}";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 404);
    EXPECT_NE(response.body.find("\"ok\":false"), std::string::npos);
    EXPECT_NE(response.body.find("\"MODEL_FILE_NOT_FOUND\""), std::string::npos);
}
