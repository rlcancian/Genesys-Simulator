#include "ApiRouter.h"

#include <algorithm>
#include <cctype>

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
