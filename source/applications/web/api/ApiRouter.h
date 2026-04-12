#pragma once

#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
#include "../service/SimulatorSessionService.h"

#include <optional>

class ApiRouter {
public:
    explicit ApiRouter(SimulatorSessionService& simulatorService);

    HttpResponse handle(const HttpRequest& request) const;

private:
    SimulatorSessionService& _simulatorService;

    static HttpResponse _jsonError(int status, const char* code, const char* message);
    static std::string _extractBearerToken(const HttpRequest& request);
    static bool _tryExtractFilenameFromBody(const std::string& body, std::string& outFilename);
    static bool _tryExtractUnsignedIntField(const std::string& body, const std::string& fieldName, unsigned int& outValue);
    static bool _tryExtractDoubleField(const std::string& body, const std::string& fieldName, double& outValue);
    static bool _tryExtractBooleanField(const std::string& body, const std::string& fieldName, bool& outValue);
    static std::optional<SimulatorSessionService::SimulationConfigInput> _parseSimulationConfigBody(const std::string& body);
    static std::string _simulationStatusDataJson(const SimulatorSessionService::SimulationStatusResult& status);
    static HttpResponse _mapPersistenceError(const SimulatorSessionService::ModelPersistenceResult& result);
    static std::string _escapeJson(const std::string& value);
    static std::string _modelInfoDataJson(const SimulatorSessionService::ModelInfoResult& info);
};
