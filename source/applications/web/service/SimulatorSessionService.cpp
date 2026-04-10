#include "SimulatorSessionService.h"

#include <filesystem>
#include <mutex>
#include <regex>

namespace {
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
}  // namespace

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
