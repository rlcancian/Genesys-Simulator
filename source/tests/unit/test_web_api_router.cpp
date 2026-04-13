#include <gtest/gtest.h>

#include "applications/web/api/ApiRouter.h"
#include "applications/web/auth/TokenService.h"
#include "applications/web/service/SimulatorSessionService.h"
#include "applications/web/worker/WorkerJobManager.h"
#include "applications/web/session/SessionManager.h"

#include <array>
#include <memory>
#include <string>

namespace {
struct ApiRouterFixture {
    TokenService tokenService;
    SessionManager sessionManager{tokenService, [] { return std::make_unique<Simulator>(); }};
    WorkerJobManager workerJobManager;
    SimulatorSessionService simulatorService{sessionManager, workerJobManager};
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

std::string minimalValidModelSpecification() {
    // Grounded in repository model syntax from models/Smart_Record.gen.
    return
        "# Genesys Simulation Model\n"
        "# Simulator, Model and Simulation infos\n"
        "0   Simulator  \"GenESyS - GENeric and Expansible SYstem Simulator\" versionNumber=230914 0   ModelInfo  \"Model 1\" "
        "version=\"1.0\" projectTitle=\"\" description=\"\" analystName=\"\" 0   ModelSimulation \"\" traceLevel=9 "
        "replicationLength=1000.000000 numberOfReplications=30 \n"
        "# Model Data Definitions\n"
        "68  Resource   \"Resource_1\" capacity=5 69  Queue      \"Queue_1\" \n"
        "# Model Components\n"
        "63  Create     \"Create_1\" entityType=\"entitytype\" nextId=64 64  Process    \"Process_1\" delayExpression=\"unif(2, 6)\" "
        "queueable=\"Queue_1\" nextId=70 requestSeizable[0]=\"Resource_1\" 70  Record     \"Record_1\" "
        "fileName=\"recordNumberInQueue.txt\" nextId=71 expressionName=\"Number in queue\" expression=\"nq(Queue_1)\" "
        "71  Record     \"Record_2\" fileName=\"recordNumberBusy.txt\" nextId=72 expressionName=\"resource_1 number busy\" "
        "expression=\"NR(Resource_1)\" 72  Record     \"Record_3\" fileName=\"recordBeta.txt\" nextId=73 "
        "expressionName=\"Just a Beta distribution\" expression=\"beta(2,8,0,100)\" 73  Dispose    \"Dispose_1\" nexts=0  ";
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

TEST(WebApiRouterTest, WorkerInfoPublicEndpointReturnsOk) {
    ApiRouterFixture fixture;

    HttpRequest request;
    request.method = "GET";
    request.path = "/api/v1/worker/info";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 200);
    EXPECT_NE(response.body.find("\"ok\":true"), std::string::npos);
}

TEST(WebApiRouterTest, WorkerInfoContainsRoleAndSimulatorVersionFields) {
    ApiRouterFixture fixture;

    HttpRequest request;
    request.method = "GET";
    request.path = "/api/v1/worker/info";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 200);
    EXPECT_NE(response.body.find("\"role\":\"worker\""), std::string::npos);
    EXPECT_NE(response.body.find("\"simulatorVersionName\":"), std::string::npos);
    EXPECT_NE(response.body.find("\"simulatorVersionNumber\":"), std::string::npos);
}

TEST(WebApiRouterTest, WorkerInfoPostReturnsMethodNotAllowed) {
    ApiRouterFixture fixture;

    HttpRequest request;
    request.method = "POST";
    request.path = "/api/v1/worker/info";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 405);
}

TEST(WebApiRouterTest, WorkerCapabilitiesPublicEndpointReturnsOk) {
    ApiRouterFixture fixture;

    HttpRequest request;
    request.method = "GET";
    request.path = "/api/v1/worker/capabilities";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 200);
    EXPECT_NE(response.body.find("\"ok\":true"), std::string::npos);
}

TEST(WebApiRouterTest, WorkerCapabilitiesReflectCurrentImplementation) {
    ApiRouterFixture fixture;

    HttpRequest request;
    request.method = "GET";
    request.path = "/api/v1/worker/capabilities";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 200);
    EXPECT_NE(response.body.find("\"supportsSessionApi\":true"), std::string::npos);
    EXPECT_NE(response.body.find("\"supportsSessionScopedSimulator\":true"), std::string::npos);
    EXPECT_NE(response.body.find("\"supportsModelCreation\":true"), std::string::npos);
    EXPECT_NE(response.body.find("\"supportsModelPersistence\":true"), std::string::npos);
    EXPECT_NE(response.body.find("\"supportsSimulationStatus\":true"), std::string::npos);
    EXPECT_NE(response.body.find("\"supportsSimulationConfig\":true"), std::string::npos);
    EXPECT_NE(response.body.find("\"supportsSynchronousRun\":true"), std::string::npos);
    EXPECT_NE(response.body.find("\"supportsSynchronousStep\":true"), std::string::npos);
    EXPECT_NE(response.body.find("\"supportsDistributedJobs\":true"), std::string::npos);
    EXPECT_NE(response.body.find("\"supportsJobPolling\":true"), std::string::npos);
    EXPECT_NE(response.body.find("\"supportsBackgroundExecution\":false"), std::string::npos);
    EXPECT_NE(response.body.find("\"supportsModelUpload\":true"), std::string::npos);
    EXPECT_NE(response.body.find("\"supportsStreamingEvents\":false"), std::string::npos);
}

TEST(WebApiRouterTest, WorkerCapabilitiesPostReturnsMethodNotAllowed) {
    ApiRouterFixture fixture;

    HttpRequest request;
    request.method = "POST";
    request.path = "/api/v1/worker/capabilities";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 405);
}

TEST(WebApiRouterTest, WorkerImportLanguageWithoutTokenReturnsUnauthorized) {
    ApiRouterFixture fixture;

    HttpRequest request;
    request.method = "POST";
    request.path = "/api/v1/worker/models/import-language";
    request.body = minimalValidModelSpecification();

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 401);
    EXPECT_NE(response.body.find("\"ok\":false"), std::string::npos);
}

TEST(WebApiRouterTest, WorkerImportLanguageWithEmptyBodyReturnsBadRequest) {
    ApiRouterFixture fixture;
    const std::string token = createSessionAndGetToken(fixture.router);

    HttpRequest request;
    request.method = "POST";
    request.path = "/api/v1/worker/models/import-language";
    request.headers["authorization"] = "Bearer " + token;
    request.body = "";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 400);
    EXPECT_NE(response.body.find("\"EMPTY_MODEL_SPECIFICATION\""), std::string::npos);
}

TEST(WebApiRouterTest, WorkerImportLanguageGetMethodReturnsMethodNotAllowed) {
    ApiRouterFixture fixture;

    HttpRequest request;
    request.method = "GET";
    request.path = "/api/v1/worker/models/import-language";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 405);
}

TEST(WebApiRouterTest, WorkerImportLanguageHappyPathReturnsModelMetadata) {
    ApiRouterFixture fixture;
    const std::string token = createSessionAndGetToken(fixture.router);

    HttpRequest request;
    request.method = "POST";
    request.path = "/api/v1/worker/models/import-language";
    request.headers["authorization"] = "Bearer " + token;
    request.body = minimalValidModelSpecification();

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 200);
    EXPECT_NE(response.body.find("\"ok\":true"), std::string::npos);
    EXPECT_NE(response.body.find("\"exists\":true"), std::string::npos);
    EXPECT_NE(response.body.find("\"modelId\":"), std::string::npos);
    EXPECT_NE(response.body.find("\"name\":"), std::string::npos);
    EXPECT_NE(response.body.find("\"componentCount\":"), std::string::npos);
}

TEST(WebApiRouterTest, WorkerJobsCreateWithoutTokenReturnsUnauthorized) {
    ApiRouterFixture fixture;

    HttpRequest request;
    request.method = "POST";
    request.path = "/api/v1/worker/jobs";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 401);
    EXPECT_NE(response.body.find("\"ok\":false"), std::string::npos);
}

TEST(WebApiRouterTest, WorkerJobsCreateWithoutCurrentModelReturnsConflict) {
    ApiRouterFixture fixture;
    const std::string token = createSessionAndGetToken(fixture.router);

    HttpRequest request;
    request.method = "POST";
    request.path = "/api/v1/worker/jobs";
    request.headers["authorization"] = "Bearer " + token;

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 409);
    EXPECT_NE(response.body.find("\"NO_CURRENT_MODEL\""), std::string::npos);
}

TEST(WebApiRouterTest, WorkerJobsCreateWithCurrentModelReturnsCreatedQueuedMetadata) {
    ApiRouterFixture fixture;
    const std::string token = createSessionAndGetToken(fixture.router);

    HttpRequest importRequest;
    importRequest.method = "POST";
    importRequest.path = "/api/v1/worker/models/import-language";
    importRequest.headers["authorization"] = "Bearer " + token;
    importRequest.body = minimalValidModelSpecification();
    const HttpResponse importResponse = fixture.router.handle(importRequest);
    ASSERT_EQ(importResponse.status, 200);

    HttpRequest createJobRequest;
    createJobRequest.method = "POST";
    createJobRequest.path = "/api/v1/worker/jobs";
    createJobRequest.headers["authorization"] = "Bearer " + token;

    const HttpResponse createJobResponse = fixture.router.handle(createJobRequest);

    EXPECT_EQ(createJobResponse.status, 201);
    EXPECT_NE(createJobResponse.body.find("\"ok\":true"), std::string::npos);
    EXPECT_NE(createJobResponse.body.find("\"jobId\":"), std::string::npos);
    EXPECT_NE(createJobResponse.body.find("\"state\":\"queued\""), std::string::npos);
    EXPECT_NE(createJobResponse.body.find("\"snapshotFilename\":\"job_"), std::string::npos);
}

TEST(WebApiRouterTest, WorkerJobsGetWithoutTokenReturnsUnauthorized) {
    ApiRouterFixture fixture;

    HttpRequest request;
    request.method = "GET";
    request.path = "/api/v1/worker/jobs/job-1";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 401);
    EXPECT_NE(response.body.find("\"ok\":false"), std::string::npos);
}

TEST(WebApiRouterTest, WorkerJobsGetUnknownIdReturnsNotFound) {
    ApiRouterFixture fixture;
    const std::string token = createSessionAndGetToken(fixture.router);

    HttpRequest request;
    request.method = "GET";
    request.path = "/api/v1/worker/jobs/job-9999";
    request.headers["authorization"] = "Bearer " + token;

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 404);
    EXPECT_NE(response.body.find("\"WORKER_JOB_NOT_FOUND\""), std::string::npos);
}

TEST(WebApiRouterTest, WorkerJobsRunWithoutTokenReturnsUnauthorized) {
    ApiRouterFixture fixture;

    HttpRequest request;
    request.method = "POST";
    request.path = "/api/v1/worker/jobs/job-1/run";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 401);
    EXPECT_NE(response.body.find("\"ok\":false"), std::string::npos);
}

TEST(WebApiRouterTest, WorkerJobsRunWrongMethodReturnsMethodNotAllowed) {
    ApiRouterFixture fixture;

    HttpRequest request;
    request.method = "GET";
    request.path = "/api/v1/worker/jobs/job-1/run";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 405);
}

TEST(WebApiRouterTest, WorkerJobsRunUnknownIdReturnsNotFound) {
    ApiRouterFixture fixture;
    const std::string token = createSessionAndGetToken(fixture.router);

    HttpRequest request;
    request.method = "POST";
    request.path = "/api/v1/worker/jobs/job-9999/run";
    request.headers["authorization"] = "Bearer " + token;

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 404);
    EXPECT_NE(response.body.find("\"WORKER_JOB_NOT_FOUND\""), std::string::npos);
}

TEST(WebApiRouterTest, WorkerJobsCreateThenGetReturnsSameQueuedJobMetadata) {
    ApiRouterFixture fixture;
    const std::string token = createSessionAndGetToken(fixture.router);

    HttpRequest importRequest;
    importRequest.method = "POST";
    importRequest.path = "/api/v1/worker/models/import-language";
    importRequest.headers["authorization"] = "Bearer " + token;
    importRequest.body = minimalValidModelSpecification();
    const HttpResponse importResponse = fixture.router.handle(importRequest);
    ASSERT_EQ(importResponse.status, 200);

    HttpRequest createJobRequest;
    createJobRequest.method = "POST";
    createJobRequest.path = "/api/v1/worker/jobs";
    createJobRequest.headers["authorization"] = "Bearer " + token;
    const HttpResponse createJobResponse = fixture.router.handle(createJobRequest);
    ASSERT_EQ(createJobResponse.status, 201);

    const std::string jobId = extractJsonStringField(createJobResponse.body, "jobId");
    ASSERT_FALSE(jobId.empty());

    HttpRequest getJobRequest;
    getJobRequest.method = "GET";
    getJobRequest.path = "/api/v1/worker/jobs/" + jobId;
    getJobRequest.headers["authorization"] = "Bearer " + token;
    const HttpResponse getJobResponse = fixture.router.handle(getJobRequest);

    EXPECT_EQ(getJobResponse.status, 200);
    EXPECT_NE(getJobResponse.body.find("\"state\":\"queued\""), std::string::npos);
    EXPECT_NE(getJobResponse.body.find("\"jobId\":\"" + jobId + "\""), std::string::npos);
}

TEST(WebApiRouterTest, WorkerJobsRunThenGetShowsUpdatedFinishedState) {
    ApiRouterFixture fixture;
    const std::string token = createSessionAndGetToken(fixture.router);

    HttpRequest importRequest;
    importRequest.method = "POST";
    importRequest.path = "/api/v1/worker/models/import-language";
    importRequest.headers["authorization"] = "Bearer " + token;
    importRequest.body = minimalValidModelSpecification();
    const HttpResponse importResponse = fixture.router.handle(importRequest);
    ASSERT_EQ(importResponse.status, 200);

    HttpRequest createJobRequest;
    createJobRequest.method = "POST";
    createJobRequest.path = "/api/v1/worker/jobs";
    createJobRequest.headers["authorization"] = "Bearer " + token;
    const HttpResponse createJobResponse = fixture.router.handle(createJobRequest);
    ASSERT_EQ(createJobResponse.status, 201);

    const std::string jobId = extractJsonStringField(createJobResponse.body, "jobId");
    ASSERT_FALSE(jobId.empty());

    HttpRequest runJobRequest;
    runJobRequest.method = "POST";
    runJobRequest.path = "/api/v1/worker/jobs/" + jobId + "/run";
    runJobRequest.headers["authorization"] = "Bearer " + token;
    const HttpResponse runJobResponse = fixture.router.handle(runJobRequest);

    EXPECT_EQ(runJobResponse.status, 200);
    EXPECT_NE(runJobResponse.body.find("\"jobId\":\"" + jobId + "\""), std::string::npos);
    EXPECT_NE(runJobResponse.body.find("\"state\":\"finished\""), std::string::npos);

    HttpRequest getJobRequest;
    getJobRequest.method = "GET";
    getJobRequest.path = "/api/v1/worker/jobs/" + jobId;
    getJobRequest.headers["authorization"] = "Bearer " + token;
    const HttpResponse getJobResponse = fixture.router.handle(getJobRequest);

    EXPECT_EQ(getJobResponse.status, 200);
    EXPECT_NE(getJobResponse.body.find("\"jobId\":\"" + jobId + "\""), std::string::npos);
    EXPECT_NE(getJobResponse.body.find("\"state\":\"finished\""), std::string::npos);
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

TEST(WebApiRouterTest, SimulationStatusWithoutTokenReturnsUnauthorized) {
    ApiRouterFixture fixture;

    HttpRequest request;
    request.method = "GET";
    request.path = "/api/v1/simulation/status";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 401);
    EXPECT_NE(response.body.find("\"ok\":false"), std::string::npos);
}

TEST(WebApiRouterTest, SimulationStatusWithoutCurrentModelReturnsHasCurrentModelFalse) {
    ApiRouterFixture fixture;
    const std::string token = createSessionAndGetToken(fixture.router);

    HttpRequest request;
    request.method = "GET";
    request.path = "/api/v1/simulation/status";
    request.headers["authorization"] = "Bearer " + token;

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 200);
    EXPECT_NE(response.body.find("\"ok\":true"), std::string::npos);
    EXPECT_NE(response.body.find("\"hasCurrentModel\":false"), std::string::npos);
}

TEST(WebApiRouterTest, SimulationConfigWithoutTokenReturnsUnauthorized) {
    ApiRouterFixture fixture;

    HttpRequest request;
    request.method = "POST";
    request.path = "/api/v1/simulation/config";
    request.body =
        "{\"numberOfReplications\":3,\"replicationLength\":120.0,\"warmUpPeriod\":10.0,"
        "\"pauseOnEvent\":false,\"pauseOnReplication\":false,\"initializeStatistics\":true,\"initializeSystem\":true}";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 401);
    EXPECT_NE(response.body.find("\"ok\":false"), std::string::npos);
}

TEST(WebApiRouterTest, SimulationRunWithoutTokenReturnsUnauthorized) {
    ApiRouterFixture fixture;

    HttpRequest request;
    request.method = "POST";
    request.path = "/api/v1/simulation/run";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 401);
    EXPECT_NE(response.body.find("\"ok\":false"), std::string::npos);
}

TEST(WebApiRouterTest, SimulationStepWithoutTokenReturnsUnauthorized) {
    ApiRouterFixture fixture;

    HttpRequest request;
    request.method = "POST";
    request.path = "/api/v1/simulation/step";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 401);
    EXPECT_NE(response.body.find("\"ok\":false"), std::string::npos);
}

TEST(WebApiRouterTest, SimulationRunWithoutCurrentModelReturnsConflict) {
    ApiRouterFixture fixture;
    const std::string token = createSessionAndGetToken(fixture.router);

    HttpRequest request;
    request.method = "POST";
    request.path = "/api/v1/simulation/run";
    request.headers["authorization"] = "Bearer " + token;

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 409);
    EXPECT_NE(response.body.find("\"NO_CURRENT_MODEL\""), std::string::npos);
}

TEST(WebApiRouterTest, SimulationStepWithoutCurrentModelReturnsConflict) {
    ApiRouterFixture fixture;
    const std::string token = createSessionAndGetToken(fixture.router);

    HttpRequest request;
    request.method = "POST";
    request.path = "/api/v1/simulation/step";
    request.headers["authorization"] = "Bearer " + token;

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 409);
    EXPECT_NE(response.body.find("\"NO_CURRENT_MODEL\""), std::string::npos);
}

TEST(WebApiRouterTest, SimulationConfigWithoutCurrentModelReturnsConflict) {
    ApiRouterFixture fixture;
    const std::string token = createSessionAndGetToken(fixture.router);

    HttpRequest request;
    request.method = "POST";
    request.path = "/api/v1/simulation/config";
    request.headers["authorization"] = "Bearer " + token;
    request.body =
        "{\"numberOfReplications\":3,\"replicationLength\":120.0,\"warmUpPeriod\":10.0,"
        "\"pauseOnEvent\":false,\"pauseOnReplication\":false,\"initializeStatistics\":true,\"initializeSystem\":true}";

    const HttpResponse response = fixture.router.handle(request);

    EXPECT_EQ(response.status, 409);
    EXPECT_NE(response.body.find("\"NO_CURRENT_MODEL\""), std::string::npos);
}

TEST(WebApiRouterTest, SimulationStatusAndConfigFlowUpdatesValues) {
    ApiRouterFixture fixture;
    const std::string token = createSessionAndGetToken(fixture.router);

    HttpRequest createModelRequest;
    createModelRequest.method = "POST";
    createModelRequest.path = "/api/v1/models";
    createModelRequest.headers["authorization"] = "Bearer " + token;
    ASSERT_EQ(fixture.router.handle(createModelRequest).status, 201);

    HttpRequest initialStatusRequest;
    initialStatusRequest.method = "GET";
    initialStatusRequest.path = "/api/v1/simulation/status";
    initialStatusRequest.headers["authorization"] = "Bearer " + token;
    const HttpResponse initialStatusResponse = fixture.router.handle(initialStatusRequest);
    ASSERT_EQ(initialStatusResponse.status, 200);
    EXPECT_NE(initialStatusResponse.body.find("\"hasCurrentModel\":true"), std::string::npos);
    EXPECT_NE(initialStatusResponse.body.find("\"numberOfReplications\":1"), std::string::npos);
    EXPECT_NE(initialStatusResponse.body.find("\"replicationLength\":60.000000"), std::string::npos);
    EXPECT_NE(initialStatusResponse.body.find("\"warmUpPeriod\":0.000000"), std::string::npos);

    HttpRequest configRequest;
    configRequest.method = "POST";
    configRequest.path = "/api/v1/simulation/config";
    configRequest.headers["authorization"] = "Bearer " + token;
    configRequest.body =
        "{\"numberOfReplications\":5,\"replicationLength\":120.0,\"warmUpPeriod\":10.0,"
        "\"pauseOnEvent\":true,\"pauseOnReplication\":true,\"initializeStatistics\":false,\"initializeSystem\":false}";
    const HttpResponse configResponse = fixture.router.handle(configRequest);
    ASSERT_EQ(configResponse.status, 200);
    EXPECT_NE(configResponse.body.find("\"numberOfReplications\":5"), std::string::npos);
    EXPECT_NE(configResponse.body.find("\"replicationLength\":120.000000"), std::string::npos);
    EXPECT_NE(configResponse.body.find("\"warmUpPeriod\":10.000000"), std::string::npos);
    EXPECT_NE(configResponse.body.find("\"pauseOnEvent\":true"), std::string::npos);
    EXPECT_NE(configResponse.body.find("\"pauseOnReplication\":true"), std::string::npos);
    EXPECT_NE(configResponse.body.find("\"initializeStatistics\":false"), std::string::npos);
    EXPECT_NE(configResponse.body.find("\"initializeSystem\":false"), std::string::npos);

    HttpRequest updatedStatusRequest;
    updatedStatusRequest.method = "GET";
    updatedStatusRequest.path = "/api/v1/simulation/status";
    updatedStatusRequest.headers["authorization"] = "Bearer " + token;
    const HttpResponse updatedStatusResponse = fixture.router.handle(updatedStatusRequest);
    ASSERT_EQ(updatedStatusResponse.status, 200);
    EXPECT_NE(updatedStatusResponse.body.find("\"numberOfReplications\":5"), std::string::npos);
    EXPECT_NE(updatedStatusResponse.body.find("\"replicationLength\":120.000000"), std::string::npos);
    EXPECT_NE(updatedStatusResponse.body.find("\"warmUpPeriod\":10.000000"), std::string::npos);
    EXPECT_NE(updatedStatusResponse.body.find("\"pauseOnEvent\":true"), std::string::npos);
    EXPECT_NE(updatedStatusResponse.body.find("\"pauseOnReplication\":true"), std::string::npos);
    EXPECT_NE(updatedStatusResponse.body.find("\"initializeStatistics\":false"), std::string::npos);
    EXPECT_NE(updatedStatusResponse.body.find("\"initializeSystem\":false"), std::string::npos);
}

TEST(WebApiRouterTest, SimulationStepWithCurrentModelReturnsUpdatedStatusEnvelope) {
    ApiRouterFixture fixture;
    const std::string token = createSessionAndGetToken(fixture.router);

    HttpRequest createModelRequest;
    createModelRequest.method = "POST";
    createModelRequest.path = "/api/v1/models";
    createModelRequest.headers["authorization"] = "Bearer " + token;
    ASSERT_EQ(fixture.router.handle(createModelRequest).status, 201);

    HttpRequest configRequest;
    configRequest.method = "POST";
    configRequest.path = "/api/v1/simulation/config";
    configRequest.headers["authorization"] = "Bearer " + token;
    configRequest.body =
        "{\"numberOfReplications\":1,\"replicationLength\":1.0,\"warmUpPeriod\":0.0,"
        "\"pauseOnEvent\":false,\"pauseOnReplication\":false,\"initializeStatistics\":true,\"initializeSystem\":true}";
    ASSERT_EQ(fixture.router.handle(configRequest).status, 200);

    HttpRequest stepRequest;
    stepRequest.method = "POST";
    stepRequest.path = "/api/v1/simulation/step";
    stepRequest.headers["authorization"] = "Bearer " + token;

    const HttpResponse response = fixture.router.handle(stepRequest);

    EXPECT_EQ(response.status, 200);
    EXPECT_NE(response.body.find("\"ok\":true"), std::string::npos);
    EXPECT_NE(response.body.find("\"hasCurrentModel\":true"), std::string::npos);
    EXPECT_NE(response.body.find("\"isRunning\":"), std::string::npos);
    EXPECT_NE(response.body.find("\"simulatedTime\":"), std::string::npos);
}

TEST(WebApiRouterTest, SimulationRunWithCurrentModelReturnsUpdatedStatusEnvelope) {
    ApiRouterFixture fixture;
    const std::string token = createSessionAndGetToken(fixture.router);

    HttpRequest createModelRequest;
    createModelRequest.method = "POST";
    createModelRequest.path = "/api/v1/models";
    createModelRequest.headers["authorization"] = "Bearer " + token;
    ASSERT_EQ(fixture.router.handle(createModelRequest).status, 201);

    HttpRequest configRequest;
    configRequest.method = "POST";
    configRequest.path = "/api/v1/simulation/config";
    configRequest.headers["authorization"] = "Bearer " + token;
    configRequest.body =
        "{\"numberOfReplications\":1,\"replicationLength\":1.0,\"warmUpPeriod\":0.0,"
        "\"pauseOnEvent\":false,\"pauseOnReplication\":false,\"initializeStatistics\":true,\"initializeSystem\":true}";
    ASSERT_EQ(fixture.router.handle(configRequest).status, 200);

    HttpRequest runRequest;
    runRequest.method = "POST";
    runRequest.path = "/api/v1/simulation/run";
    runRequest.headers["authorization"] = "Bearer " + token;

    const HttpResponse response = fixture.router.handle(runRequest);

    EXPECT_EQ(response.status, 200);
    EXPECT_NE(response.body.find("\"ok\":true"), std::string::npos);
    EXPECT_NE(response.body.find("\"hasCurrentModel\":true"), std::string::npos);
    EXPECT_NE(response.body.find("\"isRunning\":false"), std::string::npos);
    EXPECT_NE(response.body.find("\"simulatedTime\":"), std::string::npos);
}

TEST(WebApiRouterTest, SimulationConfigWithInvalidBodyReturnsBadRequest) {
    ApiRouterFixture fixture;
    const std::string token = createSessionAndGetToken(fixture.router);

    HttpRequest createModelRequest;
    createModelRequest.method = "POST";
    createModelRequest.path = "/api/v1/models";
    createModelRequest.headers["authorization"] = "Bearer " + token;
    ASSERT_EQ(fixture.router.handle(createModelRequest).status, 201);

    HttpRequest configRequest;
    configRequest.method = "POST";
    configRequest.path = "/api/v1/simulation/config";
    configRequest.headers["authorization"] = "Bearer " + token;
    configRequest.body = "{\"numberOfReplications\":2}";

    const HttpResponse configResponse = fixture.router.handle(configRequest);

    EXPECT_EQ(configResponse.status, 400);
    EXPECT_NE(configResponse.body.find("\"INVALID_SIMULATION_CONFIG\""), std::string::npos);
}

TEST(WebApiRouterTest, SimulationConfigRequiresAllFieldsInRequestBody) {
    ApiRouterFixture fixture;
    const std::string token = createSessionAndGetToken(fixture.router);

    HttpRequest createModelRequest;
    createModelRequest.method = "POST";
    createModelRequest.path = "/api/v1/models";
    createModelRequest.headers["authorization"] = "Bearer " + token;
    ASSERT_EQ(fixture.router.handle(createModelRequest).status, 201);

    const std::string baseBody =
        "\"numberOfReplications\":2,\"replicationLength\":90.0,\"warmUpPeriod\":15.0,"
        "\"pauseOnEvent\":true,\"pauseOnReplication\":false,\"initializeStatistics\":true,\"initializeSystem\":false";

    const std::array<std::string, 7> requiredFields = {
        "numberOfReplications",
        "replicationLength",
        "warmUpPeriod",
        "pauseOnEvent",
        "pauseOnReplication",
        "initializeStatistics",
        "initializeSystem"
    };

    for (const std::string& missingField : requiredFields) {
        std::string bodyWithoutField = baseBody;

        if (missingField == "numberOfReplications") {
            bodyWithoutField = "\"replicationLength\":90.0,\"warmUpPeriod\":15.0,"
                               "\"pauseOnEvent\":true,\"pauseOnReplication\":false,\"initializeStatistics\":true,"
                               "\"initializeSystem\":false";
        } else if (missingField == "replicationLength") {
            bodyWithoutField = "\"numberOfReplications\":2,\"warmUpPeriod\":15.0,"
                               "\"pauseOnEvent\":true,\"pauseOnReplication\":false,\"initializeStatistics\":true,"
                               "\"initializeSystem\":false";
        } else if (missingField == "warmUpPeriod") {
            bodyWithoutField = "\"numberOfReplications\":2,\"replicationLength\":90.0,"
                               "\"pauseOnEvent\":true,\"pauseOnReplication\":false,\"initializeStatistics\":true,"
                               "\"initializeSystem\":false";
        } else if (missingField == "pauseOnEvent") {
            bodyWithoutField = "\"numberOfReplications\":2,\"replicationLength\":90.0,\"warmUpPeriod\":15.0,"
                               "\"pauseOnReplication\":false,\"initializeStatistics\":true,\"initializeSystem\":false";
        } else if (missingField == "pauseOnReplication") {
            bodyWithoutField = "\"numberOfReplications\":2,\"replicationLength\":90.0,\"warmUpPeriod\":15.0,"
                               "\"pauseOnEvent\":true,\"initializeStatistics\":true,\"initializeSystem\":false";
        } else if (missingField == "initializeStatistics") {
            bodyWithoutField = "\"numberOfReplications\":2,\"replicationLength\":90.0,\"warmUpPeriod\":15.0,"
                               "\"pauseOnEvent\":true,\"pauseOnReplication\":false,\"initializeSystem\":false";
        } else if (missingField == "initializeSystem") {
            bodyWithoutField = "\"numberOfReplications\":2,\"replicationLength\":90.0,\"warmUpPeriod\":15.0,"
                               "\"pauseOnEvent\":true,\"pauseOnReplication\":false,\"initializeStatistics\":true";
        }

        HttpRequest configRequest;
        configRequest.method = "POST";
        configRequest.path = "/api/v1/simulation/config";
        configRequest.headers["authorization"] = "Bearer " + token;
        configRequest.body = "{" + bodyWithoutField + "}";

        const HttpResponse response = fixture.router.handle(configRequest);
        EXPECT_EQ(response.status, 400) << "Expected 400 when missing field: " << missingField;
        EXPECT_NE(response.body.find("\"INVALID_SIMULATION_CONFIG\""), std::string::npos);
    }
}
