#include "SessionManager.h"

#include <filesystem>
#include <mutex>
#include <stdexcept>
#include <utility>

/**
 * @brief Builds the session registry and session workspace root.
 */
SessionManager::SessionManager(TokenService& tokenService, SimulatorFactory simulatorFactory)
    : _tokenService(tokenService),
      _simulatorFactory(std::move(simulatorFactory)),
      _workspaceRoot(_buildWorkspaceRoot()) {
    std::error_code ec;
    std::filesystem::create_directories(_workspaceRoot, ec);
    if (ec) {
        throw std::runtime_error("Failed to create workspace root directory for web sessions");
    }
}

/**
 * @brief Creates a session, allocates token, simulator instance and workspace.
 */
SessionContext* SessionManager::createSession() {
    std::scoped_lock lock(_sessionsMutex);

    auto session = std::make_unique<SessionContext>();
    session->sessionId = _tokenService.generateSessionId();

    std::string token;
    do {
        token = _tokenService.generateAccessToken();
    } while (_sessionsByToken.find(token) != _sessionsByToken.end());

    session->accessToken = token;
    session->simulator = _simulatorFactory();
    if (!session->simulator) {
        throw std::runtime_error("Failed to create Simulator for session");
    }
    session->workspacePath = _workspaceRoot / session->sessionId;

    std::error_code ec;
    std::filesystem::create_directories(session->workspacePath, ec);
    if (ec) {
        throw std::runtime_error("Failed to create workspace directory for session");
    }

    SessionContext* raw = session.get();
    _sessionsByToken.emplace(token, std::move(session));
    return raw;
}

/**
 * @brief Resolves a session by bearer token.
 */
SessionContext* SessionManager::getSessionByToken(const std::string& token) {
    std::scoped_lock lock(_sessionsMutex);

    auto it = _sessionsByToken.find(token);
    if (it == _sessionsByToken.end()) {
        return nullptr;
    }
    return it->second.get();
}

/**
 * @brief Returns the base workspace directory used for all web sessions.
 */
std::filesystem::path SessionManager::_buildWorkspaceRoot() {
    return std::filesystem::temp_directory_path() / "genesys_web_sessions";
}
