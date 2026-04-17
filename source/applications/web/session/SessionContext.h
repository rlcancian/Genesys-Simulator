#pragma once

#include "kernel/simulator/Simulator.h"

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>

struct SessionContext {
    std::string sessionId;
    std::string accessToken;
    std::filesystem::path workspacePath;
    std::unique_ptr<Simulator> simulator;
    mutable std::mutex mutex;
};
