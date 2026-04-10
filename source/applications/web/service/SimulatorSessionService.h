#pragma once

#include "../session/SessionManager.h"

#include <string>

class SimulatorSessionService {
public:
    enum class PersistenceError {
        None,
        InvalidToken,
        InvalidFilename,
        MissingCurrentModel,
        FileNotFound,
        OperationFailed
    };

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

    struct ModelPersistenceResult {
        bool success = false;
        PersistenceError error = PersistenceError::None;
        std::string filename;
        ModelInfoResult modelInfo;
    };

    struct SimulationStatusResult {
        bool success = false;
        bool invalidToken = false;
        bool hasCurrentModel = false;
        bool isRunning = false;
        bool isPaused = false;
        double simulatedTime = 0.0;
        unsigned int currentReplicationNumber = 0;
        unsigned int numberOfReplications = 0;
        double replicationLength = 0.0;
        double warmUpPeriod = 0.0;
        bool pauseOnEvent = false;
        bool pauseOnReplication = false;
        bool initializeStatistics = false;
        bool initializeSystem = false;
    };

    struct SimulationConfigInput {
        unsigned int numberOfReplications = 0;
        double replicationLength = 0.0;
        double warmUpPeriod = 0.0;
        bool pauseOnEvent = false;
        bool pauseOnReplication = false;
        bool initializeStatistics = false;
        bool initializeSystem = false;
    };

    struct SimulationConfigResult {
        bool success = false;
        bool invalidToken = false;
        bool missingCurrentModel = false;
        SimulationStatusResult status;
    };

    struct SimulationActionResult {
        bool success = false;
        bool invalidToken = false;
        bool missingCurrentModel = false;
        SimulationStatusResult status;
    };

    explicit SimulatorSessionService(SessionManager& sessionManager);

    CreateSessionResult createSession();
    bool tryGetSimulatorInfo(const std::string& accessToken, SimulatorInfoResult& outInfo);
    bool tryCreateModel(const std::string& accessToken, ModelInfoResult& outInfo);
    bool tryGetCurrentModelInfo(const std::string& accessToken, ModelInfoResult& outInfo);
    SimulationStatusResult getSimulationStatus(const std::string& accessToken);
    SimulationConfigResult configureSimulation(const std::string& accessToken, const SimulationConfigInput& input);
    SimulationActionResult runSimulation(const std::string& accessToken);
    SimulationActionResult stepSimulation(const std::string& accessToken);
    ModelPersistenceResult saveCurrentModel(const std::string& accessToken, const std::string& filename);
    ModelPersistenceResult loadModel(const std::string& accessToken, const std::string& filename);

private:
    static bool _isSafeFilename(const std::string& filename);

    SessionManager& _sessionManager;
};
