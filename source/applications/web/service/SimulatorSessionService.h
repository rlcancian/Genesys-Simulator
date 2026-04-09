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

    explicit SimulatorSessionService(SessionManager& sessionManager);

    CreateSessionResult createSession();
    bool tryGetSimulatorInfo(const std::string& accessToken, SimulatorInfoResult& outInfo);

private:
    SessionManager& _sessionManager;
};
