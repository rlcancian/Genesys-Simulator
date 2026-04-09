#include "SimulatorSessionService.h"

#include <mutex>

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
