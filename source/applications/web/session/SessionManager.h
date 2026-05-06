#pragma once

#include "SessionContext.h"
#include "../auth/TokenService.h"

#include <functional>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

/**
 * @brief Creates, stores and resolves authenticated web sessions.
 *
 * SessionManager owns the token-to-session map used by the web API. It also
 * creates per-session simulator instances and assigns each session an isolated
 * workspace directory.
 */
class SessionManager {
public:
    /// Factory used to build one simulator per session.
    using SimulatorFactory = std::function<std::unique_ptr<Simulator>()>;

    /**
     * @brief Builds a manager with the token service and simulator factory to use.
     * @param tokenService Service responsible for generating and validating tokens.
     * @param simulatorFactory Factory used to create a fresh simulator per session.
     */
    SessionManager(TokenService& tokenService, SimulatorFactory simulatorFactory);

    /**
     * @brief Creates a new session, workspace and simulator instance.
     * @return Pointer to the stored session context.
     */
    SessionContext* createSession();
    /**
     * @brief Resolves the session associated with a bearer token.
     * @param token Bearer token presented by the client.
     * @return Pointer to the matching session or nullptr when not found.
     */
    SessionContext* getSessionByToken(const std::string& token);

private:
    /**
     * @brief Returns the root workspace directory used for session folders.
     * @return Filesystem path where session-specific workspaces are created.
     */
    static std::filesystem::path _buildWorkspaceRoot();

    TokenService& _tokenService;
    SimulatorFactory _simulatorFactory;
    std::filesystem::path _workspaceRoot;
    mutable std::mutex _sessionsMutex;
    std::unordered_map<std::string, std::unique_ptr<SessionContext>> _sessionsByToken;
};
