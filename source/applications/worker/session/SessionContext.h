#pragma once

#include "kernel/simulator/Simulator.h"

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>

/**
 * @brief Owns the state that belongs to one authenticated web session.
 *
 * Each session binds one access token to one workspace directory and one
 * simulator instance. The mutex protects session-local state during API calls
 * and background job execution.
 */
struct SessionContext {
    /// Stable session identifier returned to the client.
    std::string sessionId;
    /// Bearer token used to authenticate API calls.
    std::string accessToken;
    /// Filesystem workspace reserved for the session.
    std::filesystem::path workspacePath;
    /// Simulator instance scoped to this session.
    std::unique_ptr<Simulator> simulator;
    /// Guards session-local mutable state.
    mutable std::mutex mutex;
};
