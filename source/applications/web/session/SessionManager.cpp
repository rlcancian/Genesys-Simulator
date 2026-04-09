#include "SessionManager.h"

#include <mutex>
#include <stdexcept>
#include <utility>

SessionManager::SessionManager(TokenService& tokenService, SimulatorFactory simulatorFactory)
    : _tokenService(tokenService), _simulatorFactory(std::move(simulatorFactory)) {}

SessionContext* SessionManager::createSession() {
    std::scoped_lock lock(_sessionsMutex);

    auto session = std::make_unique<SessionContext>();
    session->sessionId = _tokenService.generateSessionId();

    std::string token;
    do {
        token = _tokenService.generateAccessToken();
    } while (_sessionsByToken.contains(token));

    session->accessToken = token;
    session->simulator = _simulatorFactory();
    if (!session->simulator) {
        throw std::runtime_error("Failed to create Simulator for session");
    }

    SessionContext* raw = session.get();
    _sessionsByToken.emplace(token, std::move(session));
    return raw;
}

SessionContext* SessionManager::getSessionByToken(const std::string& token) {
    std::scoped_lock lock(_sessionsMutex);

    auto it = _sessionsByToken.find(token);
    if (it == _sessionsByToken.end()) {
        return nullptr;
    }
    return it->second.get();
}
