#pragma once

#include "../session/SessionManager.h"

#include <string>

/**
 * @brief Encapsulates session-scoped simulator operations exposed by the Web API.
 */
class SimulatorSessionService {
public:
    /**
     * @brief Enumerates persistence-specific errors for save/load model operations.
     */
    enum class PersistenceError {
        None,
        InvalidToken,
        InvalidFilename,
        MissingCurrentModel,
        FileNotFound,
        OperationFailed
    };

    /**
     * @brief Enumerates model import errors for worker language-ingress operations.
     */
    enum class ModelImportError {
        None,
        InvalidToken,
        EmptySpecification,
        InvalidSpecification,
        OperationFailed
    };

    /**
     * @brief Contains session creation identifiers returned to API clients.
     */
    struct CreateSessionResult {
        std::string sessionId;
        std::string accessToken;
    };

    /**
     * @brief Describes simulator identity metadata for a token-scoped session.
     */
    struct SimulatorInfoResult {
        std::string name;
        std::string versionName;
        unsigned int versionNumber;
    };

    /**
     * @brief Describes worker identity metadata for distributed discovery.
     */
    struct WorkerInfoResult {
        std::string role;
        std::string application;
        std::string apiFamily;
        std::string apiVersion;
        std::string simulatorName;
        std::string simulatorVersionName;
        unsigned int simulatorVersionNumber = 0;
    };

    /**
     * @brief Describes currently supported worker features.
     */
    struct WorkerCapabilitiesResult {
        bool supportsSessionApi = false;
        bool supportsSessionScopedSimulator = false;
        bool supportsModelCreation = false;
        bool supportsModelPersistence = false;
        bool supportsSimulationStatus = false;
        bool supportsSimulationConfig = false;
        bool supportsSynchronousRun = false;
        bool supportsSynchronousStep = false;
        bool supportsDistributedJobs = false;
        bool supportsJobPolling = false;
        bool supportsBackgroundExecution = false;
        bool supportsModelUpload = false;
        bool supportsStreamingEvents = false;
    };

    /**
     * @brief Describes the currently active model information for a session.
     */
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

    /**
     * @brief Contains save/load model operation output and error state.
     */
    struct ModelPersistenceResult {
        bool success = false;
        PersistenceError error = PersistenceError::None;
        std::string filename;
        ModelInfoResult modelInfo;
    };

    /**
     * @brief Contains model import output and error state.
     */
    struct ModelImportResult {
        bool success = false;
        ModelImportError error = ModelImportError::None;
        ModelInfoResult modelInfo;
    };

    /**
     * @brief Captures the current simulation execution state.
     */
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

    /**
     * @brief Defines user-provided simulation parameters to apply to the current model.
     */
    struct SimulationConfigInput {
        unsigned int numberOfReplications = 0;
        double replicationLength = 0.0;
        double warmUpPeriod = 0.0;
        bool pauseOnEvent = false;
        bool pauseOnReplication = false;
        bool initializeStatistics = false;
        bool initializeSystem = false;
    };

    /**
     * @brief Contains the result of applying simulation configuration.
     */
    struct SimulationConfigResult {
        bool success = false;
        bool invalidToken = false;
        bool missingCurrentModel = false;
        SimulationStatusResult status;
    };

    /**
     * @brief Contains the result of a simulation action such as run or step.
     */
    struct SimulationActionResult {
        bool success = false;
        bool invalidToken = false;
        bool missingCurrentModel = false;
        SimulationStatusResult status;
    };

    /**
     * @brief Builds a service bound to the provided session manager.
     * @param sessionManager Session manager used to resolve active sessions.
     */
    explicit SimulatorSessionService(SessionManager& sessionManager);

    /**
     * @brief Creates a new authenticated session.
     * @return Session identifiers and token for API usage.
     */
    CreateSessionResult createSession();
    /**
     * @brief Fetches simulator metadata using a session token.
     * @param accessToken Bearer token associated with a session.
     * @param outInfo Receives simulator identity information on success.
     * @return True when token is valid and metadata was populated.
     */
    bool tryGetSimulatorInfo(const std::string& accessToken, SimulatorInfoResult& outInfo);
    /**
     * @brief Fetches public worker identity metadata.
     * @return Worker identity information for discovery endpoints.
     */
    WorkerInfoResult getWorkerInfo() const;
    /**
     * @brief Fetches worker capability flags for the current implementation state.
     * @return Worker capability flags.
     */
    WorkerCapabilitiesResult getWorkerCapabilities() const;
    /**
     * @brief Creates a new model in the token-scoped simulator session.
     * @param accessToken Bearer token associated with a session.
     * @param outInfo Receives model information on success.
     * @return True when the model was created and output was populated.
     */
    bool tryCreateModel(const std::string& accessToken, ModelInfoResult& outInfo);
    /**
     * @brief Returns metadata for the current model in a session.
     * @param accessToken Bearer token associated with a session.
     * @param outInfo Receives current model data.
     * @return True when token is valid and query completed.
     */
    bool tryGetCurrentModelInfo(const std::string& accessToken, ModelInfoResult& outInfo);
    /**
     * @brief Returns simulation status for the session's current model.
     * @param accessToken Bearer token associated with a session.
     * @return Structured simulation status query result.
     */
    SimulationStatusResult getSimulationStatus(const std::string& accessToken);
    /**
     * @brief Applies simulation configuration to the current model.
     * @param accessToken Bearer token associated with a session.
     * @param input Configuration values to apply.
     * @return Structured configuration result and resulting status.
     */
    SimulationConfigResult configureSimulation(const std::string& accessToken, const SimulationConfigInput& input);
    /**
     * @brief Executes a synchronous simulation run.
     * @param accessToken Bearer token associated with a session.
     * @return Structured action result and updated simulation status.
     */
    SimulationActionResult runSimulation(const std::string& accessToken);
    /**
     * @brief Executes one synchronous simulation step.
     * @param accessToken Bearer token associated with a session.
     * @return Structured action result and updated simulation status.
     */
    SimulationActionResult stepSimulation(const std::string& accessToken);
    /**
     * @brief Saves the current model into the session workspace.
     * @param accessToken Bearer token associated with a session.
     * @param filename Safe basename to use for persistence.
     * @return Persistence output and error state.
     */
    ModelPersistenceResult saveCurrentModel(const std::string& accessToken, const std::string& filename);
    /**
     * @brief Loads a model from the session workspace by filename.
     * @param accessToken Bearer token associated with a session.
     * @param filename Safe basename to load.
     * @return Persistence output and error state.
     */
    ModelPersistenceResult loadModel(const std::string& accessToken, const std::string& filename);
    /**
     * @brief Imports a model from plain text model language into the token-scoped session.
     * @param accessToken Bearer token associated with a session.
     * @param modelSpecification Plain text model language content.
     * @return Import output and error state.
     */
    ModelImportResult importModelFromLanguage(const std::string& accessToken, const std::string& modelSpecification);

private:
    /**
     * @brief Validates if a filename is safe for session workspace access.
     * @param filename Candidate filename from API input.
     * @return True when filename is a safe basename.
     */
    static bool _isSafeFilename(const std::string& filename);

    SessionManager& _sessionManager;
};
