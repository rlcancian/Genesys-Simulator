#include "AISupport.h"

#include "kernel/simulator/SimulationControlAndResponse.h"
#include "kernel/simulator/model/Model.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <utility>

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() { return &AISupport::GetPluginInformation; }
#endif

namespace {
std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}
}

AISupport::AISupport(Model* model, std::string name)
    : ModelDataDefinition(model, Util::TypeOf<AISupport>(), name) {
    auto addString = [this](const std::string& property, auto getter, auto setter) {
        auto* control = new SimulationControlString(getter, setter, Util::TypeOf<AISupport>(), getName(), property);
        _parentModel->getControls()->insert(control); _addSimulationControl(control);
    };
    addString("Provider", std::bind(&AISupport::getProvider, this), std::bind(&AISupport::setProvider, this, std::placeholders::_1));
    addString("Model", std::bind(&AISupport::getModel, this), std::bind(&AISupport::setModel, this, std::placeholders::_1));
    addString("BaseUrl", std::bind(&AISupport::getBaseUrl, this), std::bind(&AISupport::setBaseUrl, this, std::placeholders::_1));
    addString("ApiKeyEnvironmentVariable", std::bind(&AISupport::getApiKeyEnvironmentVariable, this), std::bind(&AISupport::setApiKeyEnvironmentVariable, this, std::placeholders::_1));
    addString("SystemInstructions", std::bind(&AISupport::getSystemInstructions, this), std::bind(&AISupport::setSystemInstructions, this, std::placeholders::_1));
    auto addDouble = [this](const std::string& property, auto getter, auto setter) {
        auto* control = new SimulationControlDouble(getter, setter, Util::TypeOf<AISupport>(), getName(), property);
        _parentModel->getControls()->insert(control); _addSimulationControl(control);
    };
    addDouble("Temperature", std::bind(&AISupport::getTemperature, this), std::bind(&AISupport::setTemperature, this, std::placeholders::_1));
    addDouble("MaxOutputTokens", [this]() { return static_cast<double>(getMaxOutputTokens()); }, [this](double v) { setMaxOutputTokens(static_cast<unsigned int>(std::max(0.0, v))); });
    addDouble("TimeoutSeconds", [this]() { return static_cast<double>(getTimeoutSeconds()); }, [this](double v) { setTimeoutSeconds(static_cast<unsigned int>(std::max(0.0, v))); });
    addDouble("MaxHistoryMessages", [this]() { return static_cast<double>(getMaxHistoryMessages()); }, [this](double v) { setMaxHistoryMessages(static_cast<unsigned int>(std::max(0.0, v))); });
    auto addBool = [this](const std::string& property, auto getter, auto setter) {
        auto* control = new SimulationControlBool(getter, setter, Util::TypeOf<AISupport>(), getName(), property);
        _parentModel->getControls()->insert(control); _addSimulationControl(control);
    };
    addBool("HighReasoningMode", std::bind(&AISupport::isHighReasoningMode, this), std::bind(&AISupport::setHighReasoningMode, this, std::placeholders::_1));
    addBool("ConversationEnabled", std::bind(&AISupport::isConversationEnabled, this), std::bind(&AISupport::setConversationEnabled, this, std::placeholders::_1));
    addBool("ClearHistoryBetweenReplications", std::bind(&AISupport::isClearHistoryBetweenReplications, this), std::bind(&AISupport::setClearHistoryBetweenReplications, this, std::placeholders::_1));
}

PluginInformation* AISupport::GetPluginInformation() {
    auto* info = new PluginInformation(Util::TypeOf<AISupport>(), &AISupport::LoadInstance, &AISupport::NewInstance);
    info->setCategory("AI");
    info->setDescriptionHelp("Shared AI provider configuration and conversation history for AIAssistant components. API keys are read from an environment variable and are never persisted.");
    return info;
}

ModelDataDefinition* AISupport::LoadInstance(Model* model, PersistenceRecord* fields) {
    auto* support = new AISupport(model);
    support->_loadInstance(fields);
    return support;
}

ModelDataDefinition* AISupport::NewInstance(Model* model, std::string name) { return new AISupport(model, name); }

AIConversationResponse AISupport::converse(const std::string& conversationKey, const std::string& prompt) {
    if (!_service.isInitialized()) _service.setConfiguration(_makeConfiguration());
    ++_callCount;
    AIConversationResponse response = _service.send(conversationKey, prompt);
    if (!response.success) ++_failureCount;
    return response;
}

void AISupport::clearConversation(const std::string& key) { _service.clearConversation(key); }
void AISupport::clearAllConversations() { _service.clearAllConversations(); }

#define AI_STRING_PROPERTY(Name, Field) \
void AISupport::set##Name(std::string value) { Field = std::move(value); _configurationChanged(); } \
std::string AISupport::get##Name() const { return Field; }
AI_STRING_PROPERTY(Provider, _provider)
AI_STRING_PROPERTY(Model, _model)
AI_STRING_PROPERTY(BaseUrl, _baseUrl)
AI_STRING_PROPERTY(ApiKeyEnvironmentVariable, _apiKeyEnvironmentVariable)
AI_STRING_PROPERTY(SystemInstructions, _systemInstructions)
#undef AI_STRING_PROPERTY

void AISupport::setTemperature(double v) { _temperature = v; _configurationChanged(); }
double AISupport::getTemperature() const { return _temperature; }
void AISupport::setMaxOutputTokens(unsigned int v) { _maxOutputTokens = v; _configurationChanged(); }
unsigned int AISupport::getMaxOutputTokens() const { return _maxOutputTokens; }
void AISupport::setTimeoutSeconds(unsigned int v) { _timeoutSeconds = v; _configurationChanged(); }
unsigned int AISupport::getTimeoutSeconds() const { return _timeoutSeconds; }
void AISupport::setHighReasoningMode(bool v) { _highReasoningMode = v; _configurationChanged(); }
bool AISupport::isHighReasoningMode() const { return _highReasoningMode; }
void AISupport::setConversationEnabled(bool v) { _conversationEnabled = v; _configurationChanged(); }
bool AISupport::isConversationEnabled() const { return _conversationEnabled; }
void AISupport::setMaxHistoryMessages(unsigned int v) { _maxHistoryMessages = v; _configurationChanged(); }
unsigned int AISupport::getMaxHistoryMessages() const { return _maxHistoryMessages; }
void AISupport::setClearHistoryBetweenReplications(bool v) { _clearHistoryBetweenReplications = v; }
bool AISupport::isClearHistoryBetweenReplications() const { return _clearHistoryBetweenReplications; }
unsigned long AISupport::getCallCount() const { return _callCount; }
unsigned long AISupport::getFailureCount() const { return _failureCount; }

std::string AISupport::show() {
    return ModelDataDefinition::show() + ",provider=\"" + _provider + "\",model=\"" + _model + "\",calls=" + std::to_string(_callCount);
}

bool AISupport::_loadInstance(PersistenceRecord* f) {
    bool ok = ModelDataDefinition::_loadInstance(f);
    _provider = f->loadField("provider", DEFAULT.provider);
    _model = f->loadField("model", DEFAULT.model);
    _baseUrl = f->loadField("baseUrl", DEFAULT.baseUrl);
    _apiKeyEnvironmentVariable = f->loadField("apiKeyEnvironmentVariable", DEFAULT.apiKeyEnvironmentVariable);
    _systemInstructions = f->loadField("systemInstructions", DEFAULT.systemInstructions);
    _temperature = f->loadField("temperature", DEFAULT.temperature);
    _maxOutputTokens = f->loadField("maxOutputTokens", DEFAULT.maxOutputTokens);
    _timeoutSeconds = f->loadField("timeoutSeconds", DEFAULT.timeoutSeconds);
    _highReasoningMode = f->loadField("highReasoningMode", DEFAULT.highReasoningMode);
    _conversationEnabled = f->loadField("conversationEnabled", DEFAULT.conversationEnabled);
    _maxHistoryMessages = f->loadField("maxHistoryMessages", DEFAULT.maxHistoryMessages);
    _clearHistoryBetweenReplications = f->loadField("clearHistoryBetweenReplications", DEFAULT.clearHistoryBetweenReplications);
    _configurationChanged();
    return ok;
}

void AISupport::_saveInstance(PersistenceRecord* f, bool defaults) {
    ModelDataDefinition::_saveInstance(f, defaults);
    f->saveField("provider", _provider, DEFAULT.provider, defaults);
    f->saveField("model", _model, DEFAULT.model, defaults);
    f->saveField("baseUrl", _baseUrl, DEFAULT.baseUrl, defaults);
    f->saveField("apiKeyEnvironmentVariable", _apiKeyEnvironmentVariable, DEFAULT.apiKeyEnvironmentVariable, defaults);
    f->saveField("systemInstructions", _systemInstructions, DEFAULT.systemInstructions, defaults);
    f->saveField("temperature", _temperature, DEFAULT.temperature, defaults);
    f->saveField("maxOutputTokens", _maxOutputTokens, DEFAULT.maxOutputTokens, defaults);
    f->saveField("timeoutSeconds", _timeoutSeconds, DEFAULT.timeoutSeconds, defaults);
    f->saveField("highReasoningMode", _highReasoningMode, DEFAULT.highReasoningMode, defaults);
    f->saveField("conversationEnabled", _conversationEnabled, DEFAULT.conversationEnabled, defaults);
    f->saveField("maxHistoryMessages", _maxHistoryMessages, DEFAULT.maxHistoryMessages, defaults);
    f->saveField("clearHistoryBetweenReplications", _clearHistoryBetweenReplications, DEFAULT.clearHistoryBetweenReplications, defaults);
}

bool AISupport::_check(std::string& error) {
    const std::string provider = lower(_provider);
    if (provider != "openai" && provider != "anthropic" && provider != "local") error += "AISupport provider must be OpenAI, Anthropic, or Local. ";
    if (_model.empty()) error += "AISupport model must not be empty. ";
    if (_temperature < 0.0 || _temperature > 2.0) error += "AISupport temperature must be between 0 and 2. ";
    if (_maxOutputTokens == 0) error += "AISupport maxOutputTokens must be greater than zero. ";
    return error.empty();
}

void AISupport::_initBetweenReplications() {
    _callCount = 0; _failureCount = 0;
    if (_clearHistoryBetweenReplications) clearAllConversations();
}

void AISupport::_configurationChanged() {
    _service.setConfiguration(_makeConfiguration());
}

AIConversationConfiguration AISupport::_makeConfiguration() const {
    AIConversationConfiguration config;
    const std::string provider = lower(_provider);
    config.provider = provider == "openai" ? AIConversationProvider::OpenAI
                    : provider == "anthropic" ? AIConversationProvider::Anthropic
                    : AIConversationProvider::Local;
    config.model = _model; config.baseUrl = _baseUrl;
    config.apiKeyEnvironmentVariable = _apiKeyEnvironmentVariable;
    config.systemInstructions = _systemInstructions;
    config.temperature = _temperature; config.maxOutputTokens = _maxOutputTokens;
    config.timeoutSeconds = _timeoutSeconds; config.highReasoningMode = _highReasoningMode;
    config.conversationEnabled = _conversationEnabled; config.maxHistoryMessages = _maxHistoryMessages;
    return config;
}
