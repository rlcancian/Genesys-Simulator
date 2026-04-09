#include "SimulatorSessionService.h"

#include <mutex>

SimulatorSessionService::SimulatorSessionService(SessionManager& sessionManager) : _sessionManager(sessionManager) {}

SimulatorSessionService::CreateSessionResult SimulatorSessionService::createSession() {
    SessionContext* session = _sessionManager.createSession();
    return CreateSessionResult{session->sessionId, session->accessToken};
}

bool SimulatorSessionService::tryGetSimulatorInfo(const std::string& accessToken, SimulatorInfoResult& outInfo) {
    SessionContext* session = _sessionManager.getSessionByToken(accessToken);
    if (session == nullptr || session->simulator == nullptr) {
        return false;
    }

    std::scoped_lock lock(session->mutex);
    outInfo.name = session->simulator->getName();
    outInfo.versionName = session->simulator->getVersion();
    outInfo.versionNumber = session->simulator->getVersionNumber();
    return true;
}
