#pragma once

#include "../session/SessionManager.h"

#include <string>

class SimulatorSessionService {
public:
    struct CreateSessionResult {
        std::string sessionId;
        std::string accessToken;
    };

    struct SimulatorInfoResult {
        std::string name;
        std::string versionName;
        unsigned int versionNumber;
    };

    struct ModelInfoResult {
        bool exists = false;
        unsigned int modelId = 0;
        bool hasChanged = false;
        unsigned int level = 0;
        std::string name;
        std::string analystName;
        std::string projectTitle;
        std::string version;
        std::string description;
        unsigned int componentCount = 0;
    };

    explicit SimulatorSessionService(SessionManager& sessionManager);

    CreateSessionResult createSession();
    bool tryGetSimulatorInfo(const std::string& accessToken, SimulatorInfoResult& outInfo);
    bool tryCreateModel(const std::string& accessToken, ModelInfoResult& outInfo);
    bool tryGetCurrentModelInfo(const std::string& accessToken, ModelInfoResult& outInfo);

private:
    SessionManager& _sessionManager;
};
