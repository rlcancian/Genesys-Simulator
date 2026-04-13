#include "ApiRouter.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <regex>

ApiRouter::ApiRouter(SimulatorSessionService& simulatorService) : _simulatorService(simulatorService) {}

HttpResponse ApiRouter::handle(const HttpRequest& request) const {
    if (request.method.empty() || request.path.empty()) {
        return _jsonError(400, "BAD_REQUEST", "Missing HTTP method or path");
    }

    if (request.path == "/health") {
        if (request.method != "GET") {
            return _jsonError(405, "METHOD_NOT_ALLOWED", "Only GET is allowed for /health");
        }
        return HttpResponse{200, "application/json", "{\"ok\":true,\"status\":\"up\"}"};
    }

    if (request.path == "/api/v1/worker/info") {
        if (request.method != "GET") {
            return _jsonError(405, "METHOD_NOT_ALLOWED", "Only GET is allowed for /api/v1/worker/info");
        }

        const auto info = _simulatorService.getWorkerInfo();
        return HttpResponse{200, "application/json", "{\"ok\":true,\"data\":" + _workerInfoDataJson(info) + "}"};
    }

    if (request.path == "/api/v1/worker/capabilities") {
        if (request.method != "GET") {
            return _jsonError(405, "METHOD_NOT_ALLOWED", "Only GET is allowed for /api/v1/worker/capabilities");
        }

        const auto capabilities = _simulatorService.getWorkerCapabilities();
        return HttpResponse{
            200,
            "application/json",
            "{\"ok\":true,\"data\":" + _workerCapabilitiesDataJson(capabilities) + "}"
        };
    }

    if (request.path == "/api/v1/worker/models/import-language") {
        if (request.method != "POST") {
            return _jsonError(405, "METHOD_NOT_ALLOWED", "Only POST is allowed for /api/v1/worker/models/import-language");
        }

        const std::string token = _extractBearerToken(request);
        if (token.empty()) {
            return _jsonError(401, "UNAUTHORIZED", "Missing or invalid Bearer token");
        }

        const auto result = _simulatorService.importModelFromLanguage(token, request.body);
        if (!result.success) {
            return _mapModelImportError(result);
        }

        return HttpResponse{200, "application/json", "{\"ok\":true,\"data\":" + _modelInfoDataJson(result.modelInfo) + "}"};
    }

    if (request.path == "/api/v1/auth/session") {
        if (request.method != "POST") {
            return _jsonError(405, "METHOD_NOT_ALLOWED", "Only POST is allowed for /api/v1/auth/session");
        }

        const auto session = _simulatorService.createSession();
        const std::string body =
            "{\"ok\":true,\"data\":{"
            "\"sessionId\":\"" + _escapeJson(session.sessionId) + "\","
            "\"accessToken\":\"" + _escapeJson(session.accessToken) + "\","
            "\"tokenType\":\"Bearer\"}}";

        return HttpResponse{201, "application/json", body};
    }

    if (request.path == "/api/v1/simulator/info") {
        if (request.method != "GET") {
            return _jsonError(405, "METHOD_NOT_ALLOWED", "Only GET is allowed for /api/v1/simulator/info");
        }

        const std::string token = _extractBearerToken(request);
        if (token.empty()) {
            return _jsonError(401, "UNAUTHORIZED", "Missing or invalid Bearer token");
        }

        SimulatorSessionService::SimulatorInfoResult info{};
        if (!_simulatorService.tryGetSimulatorInfo(token, info)) {
            return _jsonError(401, "UNAUTHORIZED", "Invalid or expired session token");
        }

        const std::string body =
            "{\"ok\":true,\"data\":{"
            "\"name\":\"" + _escapeJson(info.name) + "\","
            "\"versionName\":\"" + _escapeJson(info.versionName) + "\","
            "\"versionNumber\":" + std::to_string(info.versionNumber) + "}}";

        return HttpResponse{200, "application/json", body};
    }

    if (request.path == "/api/v1/models") {
        if (request.method != "POST") {
            return _jsonError(405, "METHOD_NOT_ALLOWED", "Only POST is allowed for /api/v1/models");
        }

        const std::string token = _extractBearerToken(request);
        if (token.empty()) {
            return _jsonError(401, "UNAUTHORIZED", "Missing or invalid Bearer token");
        }

        SimulatorSessionService::ModelInfoResult info{};
        if (!_simulatorService.tryCreateModel(token, info)) {
            return _jsonError(401, "UNAUTHORIZED", "Invalid or expired session token");
        }

        return HttpResponse{201, "application/json", "{\"ok\":true,\"data\":" + _modelInfoDataJson(info) + "}"};
    }

    if (request.path == "/api/v1/models/current") {
        if (request.method != "GET") {
            return _jsonError(405, "METHOD_NOT_ALLOWED", "Only GET is allowed for /api/v1/models/current");
        }

        const std::string token = _extractBearerToken(request);
        if (token.empty()) {
            return _jsonError(401, "UNAUTHORIZED", "Missing or invalid Bearer token");
        }

        SimulatorSessionService::ModelInfoResult info{};
        if (!_simulatorService.tryGetCurrentModelInfo(token, info)) {
            return _jsonError(401, "UNAUTHORIZED", "Invalid or expired session token");
        }

        return HttpResponse{200, "application/json", "{\"ok\":true,\"data\":" + _modelInfoDataJson(info) + "}"};
    }

    if (request.path == "/api/v1/models/save") {
        if (request.method != "POST") {
            return _jsonError(405, "METHOD_NOT_ALLOWED", "Only POST is allowed for /api/v1/models/save");
        }

        const std::string token = _extractBearerToken(request);
        if (token.empty()) {
            return _jsonError(401, "UNAUTHORIZED", "Missing or invalid Bearer token");
        }

        std::string filename;
        if (!_tryExtractFilenameFromBody(request.body, filename)) {
            return _jsonError(400, "BAD_REQUEST", "Invalid request body: expected JSON with filename");
        }

        const auto result = _simulatorService.saveCurrentModel(token, filename);
        if (!result.success) {
            return _mapPersistenceError(result);
        }

        const std::string body =
            "{\"ok\":true,\"data\":{"
            "\"filename\":\"" + _escapeJson(result.filename) + "\","
            "\"model\":" + _modelInfoDataJson(result.modelInfo) + "}}";
        return HttpResponse{200, "application/json", body};
    }

    if (request.path == "/api/v1/models/load") {
        if (request.method != "POST") {
            return _jsonError(405, "METHOD_NOT_ALLOWED", "Only POST is allowed for /api/v1/models/load");
        }

        const std::string token = _extractBearerToken(request);
        if (token.empty()) {
            return _jsonError(401, "UNAUTHORIZED", "Missing or invalid Bearer token");
        }

        std::string filename;
        if (!_tryExtractFilenameFromBody(request.body, filename)) {
            return _jsonError(400, "BAD_REQUEST", "Invalid request body: expected JSON with filename");
        }

        const auto result = _simulatorService.loadModel(token, filename);
        if (!result.success) {
            return _mapPersistenceError(result);
        }

        const std::string body =
            "{\"ok\":true,\"data\":{"
            "\"filename\":\"" + _escapeJson(result.filename) + "\","
            "\"model\":" + _modelInfoDataJson(result.modelInfo) + "}}";
        return HttpResponse{200, "application/json", body};
    }

    if (request.path == "/api/v1/simulation/status") {
        if (request.method != "GET") {
            return _jsonError(405, "METHOD_NOT_ALLOWED", "Only GET is allowed for /api/v1/simulation/status");
        }

        const std::string token = _extractBearerToken(request);
        if (token.empty()) {
            return _jsonError(401, "UNAUTHORIZED", "Missing or invalid Bearer token");
        }

        const auto result = _simulatorService.getSimulationStatus(token);
        if (!result.success) {
            if (result.invalidToken) {
                return _jsonError(401, "UNAUTHORIZED", "Invalid or expired session token");
            }
            return _jsonError(500, "INTERNAL_ERROR", "Unable to query simulation status");
        }

        return HttpResponse{200, "application/json", "{\"ok\":true,\"data\":" + _simulationStatusDataJson(result) + "}"};
    }

    if (request.path == "/api/v1/simulation/config") {
        if (request.method != "POST") {
            return _jsonError(405, "METHOD_NOT_ALLOWED", "Only POST is allowed for /api/v1/simulation/config");
        }

        const std::string token = _extractBearerToken(request);
        if (token.empty()) {
            return _jsonError(401, "UNAUTHORIZED", "Missing or invalid Bearer token");
        }

        const auto config = _parseSimulationConfigBody(request.body);
        if (!config.has_value()) {
            return _jsonError(
                400,
                "INVALID_SIMULATION_CONFIG",
                "Invalid request body: required fields are numberOfReplications, replicationLength, warmUpPeriod, pauseOnEvent, pauseOnReplication, initializeStatistics, initializeSystem"
            );
        }

        const auto result = _simulatorService.configureSimulation(token, config.value());
        if (!result.success) {
            if (result.invalidToken) {
                return _jsonError(401, "UNAUTHORIZED", "Invalid or expired session token");
            }
            if (result.missingCurrentModel) {
                return _jsonError(409, "NO_CURRENT_MODEL", "No current model available to configure simulation");
            }
            return _jsonError(500, "INTERNAL_ERROR", "Unable to configure simulation");
        }

        return HttpResponse{
            200,
            "application/json",
            "{\"ok\":true,\"data\":" + _simulationStatusDataJson(result.status) + "}"
        };
    }

    if (request.path == "/api/v1/simulation/run") {
        if (request.method != "POST") {
            return _jsonError(405, "METHOD_NOT_ALLOWED", "Only POST is allowed for /api/v1/simulation/run");
        }

        const std::string token = _extractBearerToken(request);
        if (token.empty()) {
            return _jsonError(401, "UNAUTHORIZED", "Missing or invalid Bearer token");
        }

        const auto result = _simulatorService.runSimulation(token);
        if (!result.success) {
            if (result.invalidToken) {
                return _jsonError(401, "UNAUTHORIZED", "Invalid or expired session token");
            }
            if (result.missingCurrentModel) {
                return _jsonError(409, "NO_CURRENT_MODEL", "No current model available to run simulation");
            }
            return _jsonError(500, "INTERNAL_ERROR", "Unable to run simulation");
        }

        return HttpResponse{
            200,
            "application/json",
            "{\"ok\":true,\"data\":" + _simulationStatusDataJson(result.status) + "}"
        };
    }

    if (request.path == "/api/v1/simulation/step") {
        if (request.method != "POST") {
            return _jsonError(405, "METHOD_NOT_ALLOWED", "Only POST is allowed for /api/v1/simulation/step");
        }

        const std::string token = _extractBearerToken(request);
        if (token.empty()) {
            return _jsonError(401, "UNAUTHORIZED", "Missing or invalid Bearer token");
        }

        const auto result = _simulatorService.stepSimulation(token);
        if (!result.success) {
            if (result.invalidToken) {
                return _jsonError(401, "UNAUTHORIZED", "Invalid or expired session token");
            }
            if (result.missingCurrentModel) {
                return _jsonError(409, "NO_CURRENT_MODEL", "No current model available to step simulation");
            }
            return _jsonError(500, "INTERNAL_ERROR", "Unable to step simulation");
        }

        return HttpResponse{
            200,
            "application/json",
            "{\"ok\":true,\"data\":" + _simulationStatusDataJson(result.status) + "}"
        };
    }

    return _jsonError(404, "NOT_FOUND", "Route not found");
}

HttpResponse ApiRouter::_jsonError(int status, const char* code, const char* message) {
    return HttpResponse{
        status,
        "application/json",
        "{\"ok\":false,\"error\":{\"code\":\"" + std::string(code) + "\",\"message\":\"" + std::string(message) + "\"}}"
    };
}

std::string ApiRouter::_extractBearerToken(const HttpRequest& request) {
    const std::string header = request.header("authorization");
    constexpr const char* prefix = "Bearer ";
    if (header.size() <= 7 || header.rfind(prefix, 0) != 0) {
        return {};
    }

    std::string token = header.substr(7);
    token.erase(token.begin(), std::find_if(token.begin(), token.end(), [](unsigned char c) { return !std::isspace(c); }));
    token.erase(std::find_if(token.rbegin(), token.rend(), [](unsigned char c) { return !std::isspace(c); }).base(), token.end());
    return token;
}

bool ApiRouter::_tryExtractFilenameFromBody(const std::string& body, std::string& outFilename) {
    static const std::regex filenameRegex("\"filename\"\\s*:\\s*\"([^\"]+)\"");
    std::smatch match;
    if (!std::regex_search(body, match, filenameRegex) || match.size() < 2) {
        return false;
    }
    outFilename = match[1].str();
    return !outFilename.empty();
}

bool ApiRouter::_tryExtractUnsignedIntField(const std::string& body, const std::string& fieldName, unsigned int& outValue) {
    const std::regex pattern("\"" + fieldName + "\"\\s*:\\s*([0-9]+)");
    std::smatch match;
    if (!std::regex_search(body, match, pattern) || match.size() < 2) {
        return false;
    }

    const std::string value = match[1].str();
    char* endPtr = nullptr;
    const unsigned long parsed = std::strtoul(value.c_str(), &endPtr, 10);
    if (endPtr == nullptr || *endPtr != '\0') {
        return false;
    }

    outValue = static_cast<unsigned int>(parsed);
    return true;
}

bool ApiRouter::_tryExtractDoubleField(const std::string& body, const std::string& fieldName, double& outValue) {
    const std::regex pattern("\"" + fieldName + "\"\\s*:\\s*(-?(?:[0-9]+(?:\\.[0-9]+)?|\\.[0-9]+)(?:[eE][+-]?[0-9]+)?)");
    std::smatch match;
    if (!std::regex_search(body, match, pattern) || match.size() < 2) {
        return false;
    }

    const std::string value = match[1].str();
    char* endPtr = nullptr;
    const double parsed = std::strtod(value.c_str(), &endPtr);
    if (endPtr == nullptr || *endPtr != '\0') {
        return false;
    }

    outValue = parsed;
    return true;
}

bool ApiRouter::_tryExtractBooleanField(const std::string& body, const std::string& fieldName, bool& outValue) {
    const std::regex pattern("\"" + fieldName + "\"\\s*:\\s*(true|false)");
    std::smatch match;
    if (!std::regex_search(body, match, pattern) || match.size() < 2) {
        return false;
    }

    outValue = match[1].str() == "true";
    return true;
}

std::optional<SimulatorSessionService::SimulationConfigInput> ApiRouter::_parseSimulationConfigBody(const std::string& body) {
    SimulatorSessionService::SimulationConfigInput config{};
    if (!_tryExtractUnsignedIntField(body, "numberOfReplications", config.numberOfReplications) ||
        !_tryExtractDoubleField(body, "replicationLength", config.replicationLength) ||
        !_tryExtractDoubleField(body, "warmUpPeriod", config.warmUpPeriod) ||
        !_tryExtractBooleanField(body, "pauseOnEvent", config.pauseOnEvent) ||
        !_tryExtractBooleanField(body, "pauseOnReplication", config.pauseOnReplication) ||
        !_tryExtractBooleanField(body, "initializeStatistics", config.initializeStatistics) ||
        !_tryExtractBooleanField(body, "initializeSystem", config.initializeSystem)) {
        return std::nullopt;
    }

    return config;
}

std::string ApiRouter::_simulationStatusDataJson(const SimulatorSessionService::SimulationStatusResult& status) {
    std::string json = "{\"hasCurrentModel\":" + std::string(status.hasCurrentModel ? "true" : "false");
    if (!status.hasCurrentModel) {
        json += "}";
        return json;
    }

    json += ",\"isRunning\":" + std::string(status.isRunning ? "true" : "false");
    json += ",\"isPaused\":" + std::string(status.isPaused ? "true" : "false");
    json += ",\"simulatedTime\":" + std::to_string(status.simulatedTime);
    json += ",\"currentReplicationNumber\":" + std::to_string(status.currentReplicationNumber);
    json += ",\"numberOfReplications\":" + std::to_string(status.numberOfReplications);
    json += ",\"replicationLength\":" + std::to_string(status.replicationLength);
    json += ",\"warmUpPeriod\":" + std::to_string(status.warmUpPeriod);
    json += ",\"pauseOnEvent\":" + std::string(status.pauseOnEvent ? "true" : "false");
    json += ",\"pauseOnReplication\":" + std::string(status.pauseOnReplication ? "true" : "false");
    json += ",\"initializeStatistics\":" + std::string(status.initializeStatistics ? "true" : "false");
    json += ",\"initializeSystem\":" + std::string(status.initializeSystem ? "true" : "false");
    json += "}";
    return json;
}

HttpResponse ApiRouter::_mapPersistenceError(const SimulatorSessionService::ModelPersistenceResult& result) {
    switch (result.error) {
        case SimulatorSessionService::PersistenceError::InvalidToken:
            return _jsonError(401, "UNAUTHORIZED", "Invalid or expired session token");
        case SimulatorSessionService::PersistenceError::InvalidFilename:
            return _jsonError(400, "INVALID_FILENAME", "Filename must be a safe basename using [A-Za-z0-9._-]");
        case SimulatorSessionService::PersistenceError::MissingCurrentModel:
            return _jsonError(409, "NO_CURRENT_MODEL", "No current model available to save");
        case SimulatorSessionService::PersistenceError::FileNotFound:
            return _jsonError(404, "MODEL_FILE_NOT_FOUND", "Model file not found in session workspace");
        case SimulatorSessionService::PersistenceError::OperationFailed:
            return _jsonError(500, "MODEL_PERSISTENCE_FAILED", "Model persistence operation failed");
        case SimulatorSessionService::PersistenceError::None:
        default:
            return _jsonError(500, "INTERNAL_ERROR", "Unexpected persistence error state");
    }
}

HttpResponse ApiRouter::_mapModelImportError(const SimulatorSessionService::ModelImportResult& result) {
    switch (result.error) {
        case SimulatorSessionService::ModelImportError::InvalidToken:
            return _jsonError(401, "UNAUTHORIZED", "Invalid or expired session token");
        case SimulatorSessionService::ModelImportError::EmptySpecification:
            return _jsonError(400, "EMPTY_MODEL_SPECIFICATION", "Model specification body cannot be empty");
        case SimulatorSessionService::ModelImportError::InvalidSpecification:
            return _jsonError(400, "INVALID_MODEL_SPECIFICATION", "Model specification could not be parsed");
        case SimulatorSessionService::ModelImportError::OperationFailed:
            return _jsonError(500, "MODEL_IMPORT_FAILED", "Unable to import model specification");
        case SimulatorSessionService::ModelImportError::None:
        default:
            return _jsonError(500, "INTERNAL_ERROR", "Unexpected model import error state");
    }
}

std::string ApiRouter::_escapeJson(const std::string& value) {
    std::string out;
    out.reserve(value.size());
    for (char c : value) {
        if (c == '\\') {
            out += "\\\\";
        } else if (c == '"') {
            out += "\\\"";
        } else {
            out += c;
        }
    }
    return out;
}

std::string ApiRouter::_modelInfoDataJson(const SimulatorSessionService::ModelInfoResult& info) {
    if (!info.exists) {
        return "{\"exists\":false}";
    }

    return "{\"exists\":true,"
           "\"modelId\":" + std::to_string(info.modelId) + ","
           "\"hasChanged\":" + std::string(info.hasChanged ? "true" : "false") + ","
           "\"level\":" + std::to_string(info.level) + ","
           "\"name\":\"" + _escapeJson(info.name) + "\","
           "\"analystName\":\"" + _escapeJson(info.analystName) + "\","
           "\"projectTitle\":\"" + _escapeJson(info.projectTitle) + "\","
           "\"version\":\"" + _escapeJson(info.version) + "\","
           "\"description\":\"" + _escapeJson(info.description) + "\","
           "\"componentCount\":" + std::to_string(info.componentCount) + "}";
}

std::string ApiRouter::_workerInfoDataJson(const SimulatorSessionService::WorkerInfoResult& info) {
    return "{\"role\":\"" + _escapeJson(info.role) + "\","
           "\"application\":\"" + _escapeJson(info.application) + "\","
           "\"apiFamily\":\"" + _escapeJson(info.apiFamily) + "\","
           "\"apiVersion\":\"" + _escapeJson(info.apiVersion) + "\","
           "\"simulatorName\":\"" + _escapeJson(info.simulatorName) + "\","
           "\"simulatorVersionName\":\"" + _escapeJson(info.simulatorVersionName) + "\","
           "\"simulatorVersionNumber\":" + std::to_string(info.simulatorVersionNumber) + "}";
}

std::string ApiRouter::_workerCapabilitiesDataJson(const SimulatorSessionService::WorkerCapabilitiesResult& capabilities) {
    return "{\"supportsSessionApi\":" + std::string(capabilities.supportsSessionApi ? "true" : "false") + ","
           "\"supportsSessionScopedSimulator\":" + std::string(capabilities.supportsSessionScopedSimulator ? "true" : "false") +
           ","
           "\"supportsModelCreation\":" + std::string(capabilities.supportsModelCreation ? "true" : "false") + ","
           "\"supportsModelPersistence\":" + std::string(capabilities.supportsModelPersistence ? "true" : "false") + ","
           "\"supportsSimulationStatus\":" + std::string(capabilities.supportsSimulationStatus ? "true" : "false") + ","
           "\"supportsSimulationConfig\":" + std::string(capabilities.supportsSimulationConfig ? "true" : "false") + ","
           "\"supportsSynchronousRun\":" + std::string(capabilities.supportsSynchronousRun ? "true" : "false") + ","
           "\"supportsSynchronousStep\":" + std::string(capabilities.supportsSynchronousStep ? "true" : "false") + ","
           "\"supportsDistributedJobs\":" + std::string(capabilities.supportsDistributedJobs ? "true" : "false") + ","
           "\"supportsJobPolling\":" + std::string(capabilities.supportsJobPolling ? "true" : "false") + ","
           "\"supportsBackgroundExecution\":" + std::string(capabilities.supportsBackgroundExecution ? "true" : "false") +
           ","
           "\"supportsModelUpload\":" + std::string(capabilities.supportsModelUpload ? "true" : "false") + ","
           "\"supportsStreamingEvents\":" + std::string(capabilities.supportsStreamingEvents ? "true" : "false") + "}";
}
