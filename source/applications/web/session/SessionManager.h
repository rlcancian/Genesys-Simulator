#pragma once

#include "SessionContext.h"
#include "../auth/TokenService.h"

#include <functional>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

class SessionManager {
public:
    using SimulatorFactory = std::function<std::unique_ptr<Simulator>()>;

    SessionManager(TokenService& tokenService, SimulatorFactory simulatorFactory);

    SessionContext* createSession();
    SessionContext* getSessionByToken(const std::string& token);

private:
    static std::filesystem::path _buildWorkspaceRoot();

    TokenService& _tokenService;
    SimulatorFactory _simulatorFactory;
    std::filesystem::path _workspaceRoot;
    mutable std::mutex _sessionsMutex;
    std::unordered_map<std::string, std::unique_ptr<SessionContext>> _sessionsByToken;
};
