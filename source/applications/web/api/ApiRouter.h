#pragma once

#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
#include "../service/SimulatorSessionService.h"

class ApiRouter {
public:
    explicit ApiRouter(SimulatorSessionService& simulatorService);

    HttpResponse handle(const HttpRequest& request) const;

private:
    SimulatorSessionService& _simulatorService;

    static HttpResponse _jsonError(int status, const char* code, const char* message);
    static std::string _extractBearerToken(const HttpRequest& request);
    static std::string _escapeJson(const std::string& value);
    static std::string _modelInfoDataJson(const SimulatorSessionService::ModelInfoResult& info);
};
