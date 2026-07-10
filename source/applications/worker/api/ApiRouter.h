#pragma once

#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
#include "../service/SimulatorSessionService.h"

#include <optional>

/**
 * @brief Routes HTTP requests to Web API operations.
 */
class ApiRouter {
public:
    /**
     * @brief Creates an API router bound to the simulator session service.
     * @param simulatorService Service used to execute API operations.
     */
    explicit ApiRouter(SimulatorSessionService& simulatorService);

    /**
     * @brief Handles one HTTP request and returns the corresponding response.
     * @param request HTTP request containing method, path, headers, and body.
     * @return HTTP response with status, content type, and JSON payload.
     */
    HttpResponse handle(const HttpRequest& request) const;

private:
    SimulatorSessionService& _simulatorService;

    /**
     * @brief Produces a standardized JSON error response.
     * @param status HTTP status code.
     * @param code Stable API error code.
     * @param message Human-readable error message.
     * @return JSON HTTP error response.
     */
    static HttpResponse _jsonError(int status, const char* code, const char* message);
    /**
     * @brief Extracts the Bearer token from the Authorization header.
     * @param request HTTP request containing headers.
     * @return Token string or empty string when missing/invalid.
     */
    static std::string _extractBearerToken(const HttpRequest& request);
    /**
     * @brief Parses a filename field from a JSON request body.
     * @param body Raw request body.
     * @param outFilename Receives parsed filename.
     * @return True when a non-empty filename is parsed.
     */
    static bool _tryExtractFilenameFromBody(const std::string& body, std::string& outFilename);
    /**
     * @brief Parses an unsigned integer field from a JSON request body.
     * @param body Raw request body.
     * @param fieldName Target JSON field name.
     * @param outValue Receives parsed numeric value.
     * @return True when parsing succeeds.
     */
    static bool _tryExtractUnsignedIntField(const std::string& body, const std::string& fieldName, unsigned int& outValue);
    /**
     * @brief Parses a floating-point field from a JSON request body.
     * @param body Raw request body.
     * @param fieldName Target JSON field name.
     * @param outValue Receives parsed numeric value.
     * @return True when parsing succeeds.
     */
    static bool _tryExtractDoubleField(const std::string& body, const std::string& fieldName, double& outValue);
    /**
     * @brief Parses a boolean field from a JSON request body.
     * @param body Raw request body.
     * @param fieldName Target JSON field name.
     * @param outValue Receives parsed boolean value.
     * @return True when parsing succeeds.
     */
    static bool _tryExtractBooleanField(const std::string& body, const std::string& fieldName, bool& outValue);
    /**
     * @brief Parses the simulation configuration request payload.
     * @param body Raw request body.
     * @return Parsed configuration or empty optional when invalid.
     */
    static std::optional<SimulatorSessionService::SimulationConfigInput> _parseSimulationConfigBody(const std::string& body);
    /**
     * @brief Serializes simulation status data into a JSON object string.
     * @param status Simulation status result to serialize.
     * @return JSON object string.
     */
    static std::string _simulationStatusDataJson(const SimulatorSessionService::SimulationStatusResult& status);
    /**
     * @brief Maps model persistence errors to transport-level HTTP responses.
     * @param result Persistence operation result.
     * @return HTTP response containing mapped status and error body.
     */
    static HttpResponse _mapPersistenceError(const SimulatorSessionService::ModelPersistenceResult& result);
    /**
     * @brief Maps model import errors to transport-level HTTP responses.
     * @param result Import operation result.
     * @return HTTP response containing mapped status and error body.
     */
    static HttpResponse _mapModelImportError(const SimulatorSessionService::ModelImportResult& result);
    /**
     * @brief Escapes backslash and quote characters for JSON strings.
     * @param value Raw string value.
     * @return JSON-escaped string.
     */
    static std::string _escapeJson(const std::string& value);
    /**
     * @brief Serializes model information into a JSON object string.
     * @param info Model information result.
     * @return JSON object string.
     */
    static std::string _modelInfoDataJson(const SimulatorSessionService::ModelInfoResult& info);
    /**
     * @brief Serializes worker identity information into a JSON object string.
     * @param info Worker information result.
     * @return JSON object string.
     */
    static std::string _workerInfoDataJson(const SimulatorSessionService::WorkerInfoResult& info);
    /**
     * @brief Serializes worker capability flags into a JSON object string.
     * @param capabilities Worker capabilities result.
     * @return JSON object string.
     */
    static std::string _workerCapabilitiesDataJson(const SimulatorSessionService::WorkerCapabilitiesResult& capabilities);
    /**
     * @brief Tries to parse a worker job identifier from `/api/v1/worker/jobs/{jobId}`.
     * @param path HTTP request path.
     * @param outJobId Receives parsed job identifier when the path matches.
     * @return True when the path format is valid and a non-empty id was extracted.
     */
    static bool _tryExtractWorkerJobIdFromPath(const std::string& path, std::string& outJobId);
    /**
     * @brief Tries to parse a worker job identifier from `/api/v1/worker/jobs/{jobId}/run`.
     * @param path HTTP request path.
     * @param outJobId Receives parsed job identifier when the path matches.
     * @return True when the path format is valid and a non-empty id was extracted.
     */
    static bool _tryExtractWorkerJobRunIdFromPath(const std::string& path, std::string& outJobId);
    /**
     * @brief Tries to parse a worker job identifier from `/api/v1/worker/jobs/{jobId}/result`.
     * @param path HTTP request path.
     * @param outJobId Receives parsed job identifier when the path matches.
     * @return True when the path format is valid and a non-empty id was extracted.
     */
    static bool _tryExtractWorkerJobResultIdFromPath(const std::string& path, std::string& outJobId);
    /**
     * @brief Converts worker job states into API string values.
     * @param state Worker job state value.
     * @return Lowercase state string expected by clients.
     */
    static const char* _workerJobStateToString(WorkerJobState state);
    /**
     * @brief Serializes worker job metadata into a JSON object string.
     * @param job Worker job information result.
     * @return JSON object string.
     */
    static std::string _workerJobDataJson(const SimulatorSessionService::WorkerJobInfoResult& job);
    /**
     * @brief Serializes worker terminal result metadata into a JSON object string.
     * @param result Worker job terminal result information.
     * @return JSON object string.
     */
    static std::string _workerJobResultDataJson(const SimulatorSessionService::WorkerJobResultInfo& result);
    /**
     * @brief Maps worker job operation errors to transport-level HTTP responses.
     * @param error Worker job error code.
     * @param includeMissingModelMessage Controls no-model message wording for create/get routes.
     * @return HTTP response containing mapped status and error body.
     */
    static HttpResponse _mapWorkerJobError(SimulatorSessionService::WorkerJobError error, bool includeMissingModelMessage);
};
