#include "SimulatorSessionService.h"

#include <algorithm>
#include <cctype>
#include <exception>
#include <filesystem>
#include <mutex>
#include <optional>
#include <regex>

namespace {
bool _isBlankSpecification(const std::string& specification) {
    return std::all_of(specification.begin(), specification.end(), [](unsigned char character) {
        return std::isspace(character) != 0;
    });
}

void _populateModelInfoFromModel(Model* model, SimulatorSessionService::ModelInfoResult& outInfo) {
    outInfo.exists = true;
    outInfo.modelId = static_cast<unsigned int>(model->getId());
    outInfo.hasChanged = model->hasChanged();
    outInfo.level = model->getLevel();

    if (const ModelInfo* modelInfo = model->getInfos(); modelInfo != nullptr) {
        outInfo.name = modelInfo->getName();
        outInfo.analystName = modelInfo->getAnalystName();
        outInfo.projectTitle = modelInfo->getProjectTitle();
        outInfo.version = modelInfo->getVersion();
        outInfo.description = modelInfo->getDescription();
    }

    if (ComponentManager* componentManager = model->getComponentManager(); componentManager != nullptr) {
        outInfo.componentCount = componentManager->getNumberOfComponents();
    }
}

void _populateSimulationStatusFromModel(Model* model, SimulatorSessionService::SimulationStatusResult& outStatus) {
    outStatus.hasCurrentModel = true;
    ModelSimulation* simulation = model->getSimulation();
    if (simulation == nullptr) {
        return;
    }

    outStatus.isRunning = simulation->isRunning();
    outStatus.isPaused = simulation->isPaused();
    outStatus.simulatedTime = simulation->getSimulatedTime();
    outStatus.currentReplicationNumber = simulation->getCurrentReplicationNumber();
    outStatus.numberOfReplications = simulation->getNumberOfReplications();
    outStatus.replicationLength = simulation->getReplicationLength();
    outStatus.warmUpPeriod = simulation->getWarmUpPeriod();
    outStatus.pauseOnEvent = simulation->isPauseOnEvent();
    outStatus.pauseOnReplication = simulation->isPauseOnReplication();
    outStatus.initializeStatistics = simulation->isInitializeStatistics();
    outStatus.initializeSystem = simulation->isInitializeSystem();
}
}  // namespace

SimulatorSessionService::SimulatorSessionService(SessionManager& sessionManager, WorkerJobManager& workerJobManager)
    : _sessionManager(sessionManager), _workerJobManager(workerJobManager) {}

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

SimulatorSessionService::WorkerInfoResult SimulatorSessionService::getWorkerInfo() const {
    // A temporary simulator instance provides stable identity metadata for public worker discovery.
    Simulator simulator;

    WorkerInfoResult result{};
    result.role = "worker";
    result.application = "genesys_webhook";
    result.apiFamily = "genesys-web-worker";
    result.apiVersion = "v1";
    result.simulatorName = simulator.getName();
    result.simulatorVersionName = simulator.getVersion();
    result.simulatorVersionNumber = simulator.getVersionNumber();
    return result;
}

SimulatorSessionService::WorkerCapabilitiesResult SimulatorSessionService::getWorkerCapabilities() const {
    WorkerCapabilitiesResult result{};
    result.supportsSessionApi = true;
    result.supportsSessionScopedSimulator = true;
    result.supportsModelCreation = true;
    result.supportsModelPersistence = true;
    result.supportsSimulationStatus = true;
    result.supportsSimulationConfig = true;
    result.supportsSynchronousRun = true;
    result.supportsSynchronousStep = true;
    // Stage 4 keeps synchronous execution and exposes state inspection via job lookup.
    result.supportsDistributedJobs = true;
    result.supportsJobPolling = true;
    result.supportsBackgroundExecution = false;
    // Stage 2 introduces a worker model-ingress endpoint based on plain text language import.
    result.supportsModelUpload = true;
    result.supportsStreamingEvents = false;
    result.supportsJobResultRetrieval = true;
    return result;
}

bool SimulatorSessionService::tryCreateModel(const std::string& accessToken, ModelInfoResult& outInfo) {
    SessionContext* session = _sessionManager.getSessionByToken(accessToken);
    if (session == nullptr || session->simulator == nullptr) {
        return false;
    }

    std::scoped_lock lock(session->mutex);
    ModelManager* modelManager = session->simulator->getModelManager();
    if (modelManager == nullptr) {
        return false;
    }

    Model* model = modelManager->newModel();
    if (model == nullptr) {
        return false;
    }

    outInfo = ModelInfoResult{};
    _populateModelInfoFromModel(model, outInfo);
    return true;
}

bool SimulatorSessionService::tryGetCurrentModelInfo(const std::string& accessToken, ModelInfoResult& outInfo) {
    SessionContext* session = _sessionManager.getSessionByToken(accessToken);
    if (session == nullptr || session->simulator == nullptr) {
        return false;
    }

    std::scoped_lock lock(session->mutex);
    ModelManager* modelManager = session->simulator->getModelManager();
    if (modelManager == nullptr) {
        return false;
    }

    outInfo = ModelInfoResult{};
    Model* model = modelManager->current();
    if (model == nullptr) {
        return true;
    }

    _populateModelInfoFromModel(model, outInfo);
    return true;
}

SimulatorSessionService::SimulationStatusResult SimulatorSessionService::getSimulationStatus(const std::string& accessToken) {
    SessionContext* session = _sessionManager.getSessionByToken(accessToken);
    if (session == nullptr || session->simulator == nullptr) {
        return SimulationStatusResult{false, true};
    }

    std::scoped_lock lock(session->mutex);
    ModelManager* modelManager = session->simulator->getModelManager();
    if (modelManager == nullptr) {
        return SimulationStatusResult{false, false};
    }

    SimulationStatusResult result{};
    result.success = true;
    Model* model = modelManager->current();
    if (model == nullptr) {
        return result;
    }

    _populateSimulationStatusFromModel(model, result);
    return result;
}

SimulatorSessionService::SimulationConfigResult SimulatorSessionService::configureSimulation(
    const std::string& accessToken,
    const SimulationConfigInput& input
) {
    SessionContext* session = _sessionManager.getSessionByToken(accessToken);
    if (session == nullptr || session->simulator == nullptr) {
        return SimulationConfigResult{false, true, false, SimulationStatusResult{}};
    }

    std::scoped_lock lock(session->mutex);
    ModelManager* modelManager = session->simulator->getModelManager();
    if (modelManager == nullptr) {
        return SimulationConfigResult{false, false, true, SimulationStatusResult{}};
    }

    Model* model = modelManager->current();
    if (model == nullptr) {
        return SimulationConfigResult{false, false, true, SimulationStatusResult{}};
    }

    ModelSimulation* simulation = model->getSimulation();
    if (simulation == nullptr) {
        return SimulationConfigResult{false, false, true, SimulationStatusResult{}};
    }

    simulation->setNumberOfReplications(input.numberOfReplications);
    simulation->setReplicationLength(input.replicationLength);
    simulation->setWarmUpPeriod(input.warmUpPeriod);
    simulation->setPauseOnEvent(input.pauseOnEvent);
    simulation->setPauseOnReplication(input.pauseOnReplication);
    simulation->setInitializeStatistics(input.initializeStatistics);
    simulation->setInitializeSystem(input.initializeSystem);

    SimulationConfigResult result{};
    result.success = true;
    result.status.success = true;
    _populateSimulationStatusFromModel(model, result.status);
    return result;
}

SimulatorSessionService::SimulationActionResult SimulatorSessionService::runSimulation(const std::string& accessToken) {
    SessionContext* session = _sessionManager.getSessionByToken(accessToken);
    if (session == nullptr || session->simulator == nullptr) {
        return SimulationActionResult{false, true, false, SimulationStatusResult{}};
    }

    std::scoped_lock lock(session->mutex);
    ModelManager* modelManager = session->simulator->getModelManager();
    if (modelManager == nullptr) {
        return SimulationActionResult{false, false, true, SimulationStatusResult{}};
    }

    Model* model = modelManager->current();
    if (model == nullptr) {
        return SimulationActionResult{false, false, true, SimulationStatusResult{}};
    }

    ModelSimulation* simulation = model->getSimulation();
    if (simulation == nullptr) {
        return SimulationActionResult{false, false, true, SimulationStatusResult{}};
    }

    try {
        simulation->start();
    } catch (...) {
        return SimulationActionResult{false, false, false, SimulationStatusResult{}};
    }

    SimulationActionResult result{};
    result.success = true;
    result.status.success = true;
    _populateSimulationStatusFromModel(model, result.status);
    return result;
}

SimulatorSessionService::SimulationActionResult SimulatorSessionService::stepSimulation(const std::string& accessToken) {
    SessionContext* session = _sessionManager.getSessionByToken(accessToken);
    if (session == nullptr || session->simulator == nullptr) {
        return SimulationActionResult{false, true, false, SimulationStatusResult{}};
    }

    std::scoped_lock lock(session->mutex);
    ModelManager* modelManager = session->simulator->getModelManager();
    if (modelManager == nullptr) {
        return SimulationActionResult{false, false, true, SimulationStatusResult{}};
    }

    Model* model = modelManager->current();
    if (model == nullptr) {
        return SimulationActionResult{false, false, true, SimulationStatusResult{}};
    }

    ModelSimulation* simulation = model->getSimulation();
    if (simulation == nullptr) {
        return SimulationActionResult{false, false, true, SimulationStatusResult{}};
    }

    try {
        simulation->step();
    } catch (...) {
        return SimulationActionResult{false, false, false, SimulationStatusResult{}};
    }

    SimulationActionResult result{};
    result.success = true;
    result.status.success = true;
    _populateSimulationStatusFromModel(model, result.status);
    return result;
}

SimulatorSessionService::ModelPersistenceResult SimulatorSessionService::saveCurrentModel(
    const std::string& accessToken,
    const std::string& filename
) {
    if (!_isSafeFilename(filename)) {
        return ModelPersistenceResult{false, PersistenceError::InvalidFilename, filename, ModelInfoResult{}};
    }

    SessionContext* session = _sessionManager.getSessionByToken(accessToken);
    if (session == nullptr || session->simulator == nullptr) {
        return ModelPersistenceResult{false, PersistenceError::InvalidToken, filename, ModelInfoResult{}};
    }

    std::scoped_lock lock(session->mutex);
    ModelManager* modelManager = session->simulator->getModelManager();
    if (modelManager == nullptr) {
        return ModelPersistenceResult{false, PersistenceError::OperationFailed, filename, ModelInfoResult{}};
    }

    Model* model = modelManager->current();
    if (model == nullptr) {
        return ModelPersistenceResult{false, PersistenceError::MissingCurrentModel, filename, ModelInfoResult{}};
    }

    const std::filesystem::path modelPath = session->workspacePath / filename;
    if (!modelManager->saveModel(modelPath.string())) {
        return ModelPersistenceResult{false, PersistenceError::OperationFailed, filename, ModelInfoResult{}};
    }

    ModelPersistenceResult result{};
    result.success = true;
    result.error = PersistenceError::None;
    result.filename = filename;
    _populateModelInfoFromModel(model, result.modelInfo);
    return result;
}

SimulatorSessionService::ModelPersistenceResult SimulatorSessionService::loadModel(
    const std::string& accessToken,
    const std::string& filename
) {
    if (!_isSafeFilename(filename)) {
        return ModelPersistenceResult{false, PersistenceError::InvalidFilename, filename, ModelInfoResult{}};
    }

    SessionContext* session = _sessionManager.getSessionByToken(accessToken);
    if (session == nullptr || session->simulator == nullptr) {
        return ModelPersistenceResult{false, PersistenceError::InvalidToken, filename, ModelInfoResult{}};
    }

    std::scoped_lock lock(session->mutex);
    ModelManager* modelManager = session->simulator->getModelManager();
    if (modelManager == nullptr) {
        return ModelPersistenceResult{false, PersistenceError::OperationFailed, filename, ModelInfoResult{}};
    }

    const std::filesystem::path modelPath = session->workspacePath / filename;
    if (!std::filesystem::exists(modelPath) || !std::filesystem::is_regular_file(modelPath)) {
        return ModelPersistenceResult{false, PersistenceError::FileNotFound, filename, ModelInfoResult{}};
    }

    Model* model = modelManager->loadModel(modelPath.string());
    if (model == nullptr) {
        return ModelPersistenceResult{false, PersistenceError::OperationFailed, filename, ModelInfoResult{}};
    }

    ModelPersistenceResult result{};
    result.success = true;
    result.error = PersistenceError::None;
    result.filename = filename;
    _populateModelInfoFromModel(model, result.modelInfo);
    return result;
}

SimulatorSessionService::ModelImportResult SimulatorSessionService::importModelFromLanguage(
    const std::string& accessToken,
    const std::string& modelSpecification
) {
    if (modelSpecification.empty() || _isBlankSpecification(modelSpecification)) {
        return ModelImportResult{false, ModelImportError::EmptySpecification, ModelInfoResult{}};
    }

    SessionContext* session = _sessionManager.getSessionByToken(accessToken);
    if (session == nullptr || session->simulator == nullptr) {
        return ModelImportResult{false, ModelImportError::InvalidToken, ModelInfoResult{}};
    }

    std::scoped_lock lock(session->mutex);
    ModelManager* modelManager = session->simulator->getModelManager();
    if (modelManager == nullptr) {
        return ModelImportResult{false, ModelImportError::OperationFailed, ModelInfoResult{}};
    }

    // Import uses the real kernel parser pathway that accepts the model language as plain text.
    Model* model = modelManager->createFromLanguage(modelSpecification);
    if (model == nullptr) {
        return ModelImportResult{false, ModelImportError::InvalidSpecification, ModelInfoResult{}};
    }

    ModelImportResult result{};
    result.success = true;
    result.error = ModelImportError::None;
    _populateModelInfoFromModel(model, result.modelInfo);
    return result;
}


SimulatorSessionService::WorkerJobCreationResult SimulatorSessionService::createWorkerJob(const std::string& accessToken) {
    SessionContext* session = _sessionManager.getSessionByToken(accessToken);
    if (session == nullptr || session->simulator == nullptr) {
        return WorkerJobCreationResult{false, WorkerJobError::InvalidToken, WorkerJobInfoResult{}};
    }

    std::scoped_lock lock(session->mutex);
    ModelManager* modelManager = session->simulator->getModelManager();
    if (modelManager == nullptr) {
        return WorkerJobCreationResult{false, WorkerJobError::OperationFailed, WorkerJobInfoResult{}};
    }

    Model* model = modelManager->current();
    if (model == nullptr) {
        return WorkerJobCreationResult{false, WorkerJobError::MissingCurrentModel, WorkerJobInfoResult{}};
    }

    WorkerJob job = _workerJobManager.createQueuedJob(session->sessionId);
    const std::string snapshotFilename = std::string("job_") + job.jobId + ".gen";

    // Persisting a snapshot decouples future worker execution from live model mutations.
    const std::filesystem::path snapshotPath = session->workspacePath / snapshotFilename;
    if (!model->save(snapshotPath.string())) {
        return WorkerJobCreationResult{false, WorkerJobError::OperationFailed, WorkerJobInfoResult{}};
    }

    if (!_workerJobManager.setSnapshotFilename(job.jobId, snapshotFilename)) {
        return WorkerJobCreationResult{false, WorkerJobError::OperationFailed, WorkerJobInfoResult{}};
    }

    const std::optional<WorkerJob> storedJob = _workerJobManager.getJob(job.jobId);
    if (!storedJob.has_value()) {
        return WorkerJobCreationResult{false, WorkerJobError::OperationFailed, WorkerJobInfoResult{}};
    }

    return WorkerJobCreationResult{true, WorkerJobError::None, _toWorkerJobInfoResult(storedJob.value())};
}

SimulatorSessionService::WorkerJobQueryResult SimulatorSessionService::getWorkerJob(
    const std::string& accessToken,
    const std::string& jobId
) {
    SessionContext* session = _sessionManager.getSessionByToken(accessToken);
    if (session == nullptr || session->simulator == nullptr) {
        return WorkerJobQueryResult{false, WorkerJobError::InvalidToken, WorkerJobInfoResult{}};
    }

    const std::optional<WorkerJob> job = _workerJobManager.getJob(jobId);
    if (!job.has_value()) {
        return WorkerJobQueryResult{false, WorkerJobError::JobNotFound, WorkerJobInfoResult{}};
    }

    if (job->sessionId != session->sessionId) {
        return WorkerJobQueryResult{false, WorkerJobError::AccessDenied, WorkerJobInfoResult{}};
    }

    return WorkerJobQueryResult{true, WorkerJobError::None, _toWorkerJobInfoResult(job.value())};
}

SimulatorSessionService::WorkerJobRunResult SimulatorSessionService::runWorkerJob(
    const std::string& accessToken,
    const std::string& jobId
) {
    SessionContext* session = _sessionManager.getSessionByToken(accessToken);
    if (session == nullptr || session->simulator == nullptr) {
        return WorkerJobRunResult{false, WorkerJobError::InvalidToken, WorkerJobInfoResult{}};
    }

    const std::optional<WorkerJob> job = _workerJobManager.getJob(jobId);
    if (!job.has_value()) {
        return WorkerJobRunResult{false, WorkerJobError::JobNotFound, WorkerJobInfoResult{}};
    }

    if (job->sessionId != session->sessionId) {
        return WorkerJobRunResult{false, WorkerJobError::AccessDenied, WorkerJobInfoResult{}};
    }

    if (job->snapshotFilename.empty()) {
        _workerJobManager.setState(jobId, WorkerJobState::Failed);
        _workerJobManager.setMessage(jobId, "Worker job snapshot filename is missing");
        return WorkerJobRunResult{false, WorkerJobError::OperationFailed, WorkerJobInfoResult{}};
    }

    if (!_workerJobManager.setState(jobId, WorkerJobState::Running)) {
        return WorkerJobRunResult{false, WorkerJobError::OperationFailed, WorkerJobInfoResult{}};
    }
    _workerJobManager.setMessage(jobId, "");

    std::string failureMessage;
    bool executionSucceeded = false;
    WorkerJobTerminalResult terminalResult{};

    {
        std::scoped_lock lock(session->mutex);
        ModelManager* modelManager = session->simulator->getModelManager();
        if (modelManager == nullptr) {
            failureMessage = "Unable to access model manager";
        } else {
            try {
                // Stage 4/5 keep synchronous execution from the persisted job snapshot.
                const std::filesystem::path snapshotPath = session->workspacePath / job->snapshotFilename;
                Model* loadedModel = modelManager->loadModel(snapshotPath.string());
                if (loadedModel == nullptr) {
                    failureMessage = "Unable to load worker snapshot model";
                } else {
                    ModelSimulation* simulation = loadedModel->getSimulation();
                    if (simulation == nullptr) {
                        failureMessage = "Unable to access model simulation";
                    } else {
                        simulation->start();
                        executionSucceeded = true;

                        // Persist a minimal terminal summary that can be queried by /result.
                        terminalResult.simulatedTime = simulation->getSimulatedTime();
                        terminalResult.currentReplicationNumber = simulation->getCurrentReplicationNumber();
                        terminalResult.numberOfReplications = simulation->getNumberOfReplications();
                        terminalResult.replicationLength = simulation->getReplicationLength();
                        terminalResult.warmUpPeriod = simulation->getWarmUpPeriod();
                        terminalResult.isPaused = simulation->isPaused();
                    }
                }
            } catch (const std::exception& exception) {
                failureMessage = exception.what();
            } catch (...) {
                failureMessage = "Unexpected simulation execution failure";
            }
        }
    }

    if (!executionSucceeded) {
        if (_workerJobManager.setState(jobId, WorkerJobState::Failed)) {
            // Even on failures, keep a safe partial terminal summary for stage 5 result retrieval.
            _workerJobManager.setTerminalResult(jobId, terminalResult);
        }
        _workerJobManager.setMessage(jobId, failureMessage);
        return WorkerJobRunResult{false, WorkerJobError::OperationFailed, WorkerJobInfoResult{}};
    }

    if (!_workerJobManager.setState(jobId, WorkerJobState::Finished)) {
        return WorkerJobRunResult{false, WorkerJobError::OperationFailed, WorkerJobInfoResult{}};
    }
    if (!_workerJobManager.setTerminalResult(jobId, terminalResult)) {
        return WorkerJobRunResult{false, WorkerJobError::OperationFailed, WorkerJobInfoResult{}};
    }
    _workerJobManager.setMessage(jobId, "");

    const std::optional<WorkerJob> storedJob = _workerJobManager.getJob(jobId);
    if (!storedJob.has_value()) {
        return WorkerJobRunResult{false, WorkerJobError::OperationFailed, WorkerJobInfoResult{}};
    }

    return WorkerJobRunResult{true, WorkerJobError::None, _toWorkerJobInfoResult(storedJob.value())};
}

SimulatorSessionService::WorkerJobResultQueryResult SimulatorSessionService::getWorkerJobResult(
    const std::string& accessToken,
    const std::string& jobId
) {
    SessionContext* session = _sessionManager.getSessionByToken(accessToken);
    if (session == nullptr || session->simulator == nullptr) {
        return WorkerJobResultQueryResult{false, WorkerJobError::InvalidToken, WorkerJobResultInfo{}};
    }

    const std::optional<WorkerJob> job = _workerJobManager.getJob(jobId);
    if (!job.has_value()) {
        return WorkerJobResultQueryResult{false, WorkerJobError::JobNotFound, WorkerJobResultInfo{}};
    }

    if (job->sessionId != session->sessionId) {
        return WorkerJobResultQueryResult{false, WorkerJobError::AccessDenied, WorkerJobResultInfo{}};
    }

    if (job->state == WorkerJobState::Queued || job->state == WorkerJobState::Running || !job->hasTerminalResult) {
        return WorkerJobResultQueryResult{false, WorkerJobError::ResultNotReady, WorkerJobResultInfo{}};
    }

    return WorkerJobResultQueryResult{true, WorkerJobError::None, _toWorkerJobResultInfo(job.value())};
}

bool SimulatorSessionService::_isSafeFilename(const std::string& filename) {
    if (filename.empty()) {
        return false;
    }

    if (filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos ||
        filename.find("..") != std::string::npos) {
        return false;
    }

    const std::filesystem::path path(filename);
    if (path.is_absolute() || path.has_parent_path()) {
        return false;
    }

    static const std::regex safeNamePattern("^[A-Za-z0-9._-]+$");
    return std::regex_match(filename, safeNamePattern);
}

SimulatorSessionService::WorkerJobInfoResult SimulatorSessionService::_toWorkerJobInfoResult(const WorkerJob& job) {
    WorkerJobInfoResult result{};
    result.jobId = job.jobId;
    result.sessionId = job.sessionId;
    result.state = job.state;
    result.snapshotFilename = job.snapshotFilename;
    result.createdMarker = job.createdMarker;
    result.message = job.message;
    return result;
}

SimulatorSessionService::WorkerJobResultInfo SimulatorSessionService::_toWorkerJobResultInfo(const WorkerJob& job) {
    WorkerJobResultInfo result{};
    result.jobId = job.jobId;
    result.state = job.state;
    result.message = job.message;
    result.simulatedTime = job.terminalResult.simulatedTime;
    result.currentReplicationNumber = job.terminalResult.currentReplicationNumber;
    result.numberOfReplications = job.terminalResult.numberOfReplications;
    result.replicationLength = job.terminalResult.replicationLength;
    result.warmUpPeriod = job.terminalResult.warmUpPeriod;
    result.hasIsPaused = job.terminalResult.isPaused.has_value();
    result.isPaused = job.terminalResult.isPaused.value_or(false);
    return result;
}
