#include "AIAssistant.h"

#include "kernel/simulator/SimulationControlAndResponse.h"
#include "kernel/simulator/essentialPlugins/Entity.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelDataManager.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <iomanip>
#include <regex>
#include <sstream>
#include <utility>

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() { return &AIAssistant::GetPluginInformation; }
#endif

namespace {
std::string trim(std::string value) {
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
    return value;
}
std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return std::tolower(c); });
    return value;
}
bool jsonString(const std::string& json, const std::string& key, std::string& value) {
    std::smatch match;
    std::regex pattern("\\\"" + key + "\\\"\\s*:\\s*\\\"((?:\\\\.|[^\\\"])*)\\\"");
    if (!std::regex_search(json, match, pattern)) return false;
    value = match[1].str(); return true;
}
bool jsonNumber(const std::string& json, const std::string& key, double& value) {
    std::smatch match;
    std::regex pattern("\\\"" + key + "\\\"\\s*:\\s*(-?[0-9]+(?:\\.[0-9]+)?(?:[eE][+-]?[0-9]+)?)");
    if (!std::regex_search(json, match, pattern)) return false;
    value = std::stod(match[1].str()); return true;
}
}

AIAssistant::AIAssistant(Model* model, std::string name)
    : ModelComponent(model, Util::TypeOf<AIAssistant>(), name) {
    auto addString = [this](const std::string& property, auto getter, auto setter) {
        auto* control = new SimulationControlString(getter, setter, Util::TypeOf<AIAssistant>(), getName(), property);
        _parentModel->getControls()->insert(control); _addSimulationControl(control);
    };
    addString("AISupport", std::bind(&AIAssistant::getAISupportName, this), std::bind(&AIAssistant::setAISupportName, this, std::placeholders::_1));
    addString("PromptTemplate", std::bind(&AIAssistant::getPromptTemplate, this), std::bind(&AIAssistant::setPromptTemplate, this, std::placeholders::_1));
    addString("ConversationScope", std::bind(&AIAssistant::getConversationScope, this), std::bind(&AIAssistant::setConversationScope, this, std::placeholders::_1));
    addString("ConversationKeyExpression", std::bind(&AIAssistant::getConversationKeyExpression, this), std::bind(&AIAssistant::setConversationKeyExpression, this, std::placeholders::_1));
    addString("ActionMappings", std::bind(&AIAssistant::getActionMappings, this), std::bind(&AIAssistant::setActionMappings, this, std::placeholders::_1));
    addString("DelayExpression", std::bind(&AIAssistant::getDelayExpression, this), std::bind(&AIAssistant::setDelayExpression, this, std::placeholders::_1));
    addString("SaveConfidenceAttribute", std::bind(&AIAssistant::getSaveConfidenceAttribute, this), std::bind(&AIAssistant::setSaveConfidenceAttribute, this, std::placeholders::_1));
    addString("SaveSuccessAttribute", std::bind(&AIAssistant::getSaveSuccessAttribute, this), std::bind(&AIAssistant::setSaveSuccessAttribute, this, std::placeholders::_1));
    auto* defaultPort = new SimulationControlDouble([this]() { return static_cast<double>(getDefaultOutputPort()); }, [this](double v) { setDefaultOutputPort(static_cast<unsigned int>(std::max(0.0, v))); }, Util::TypeOf<AIAssistant>(), getName(), "DefaultOutputPort");
    auto* errorPort = new SimulationControlDouble([this]() { return static_cast<double>(getErrorOutputPort()); }, [this](double v) { setErrorOutputPort(static_cast<unsigned int>(std::max(0.0, v))); }, Util::TypeOf<AIAssistant>(), getName(), "ErrorOutputPort");
    auto* confidence = new SimulationControlDouble(std::bind(&AIAssistant::getMinimumConfidence, this), std::bind(&AIAssistant::setMinimumConfidence, this, std::placeholders::_1), Util::TypeOf<AIAssistant>(), getName(), "MinimumConfidence");
    auto* directPort = new SimulationControlBool(std::bind(&AIAssistant::isAllowDirectOutputPort, this), std::bind(&AIAssistant::setAllowDirectOutputPort, this, std::placeholders::_1), Util::TypeOf<AIAssistant>(), getName(), "AllowDirectOutputPort");
    SimulationControl* controls[] = {defaultPort, errorPort, confidence, directPort};
    for (SimulationControl* control : controls) { _parentModel->getControls()->insert(control); _addSimulationControl(control); }
}

PluginInformation* AIAssistant::GetPluginInformation() {
    auto* info = new PluginInformation(Util::TypeOf<AIAssistant>(), &AIAssistant::LoadInstance, &AIAssistant::NewInstance);
    info->setCategory("AI"); info->setMinimumInputs(1); info->setMaximumInputs(1);
    info->setMinimumOutputs(1); info->setMaximumOutputs(999);
    info->insertDynamicLibFileDependence("aisupport.so");
    info->setDescriptionHelp("Calls an AISupport synchronously when an entity arrives. Expressions inside {braces} are evaluated by the GenESyS parser. The JSON response selects an output through ActionMappings and may assign numeric entity attributes. DelayExpression adds simulated time after the response.");
    return info;
}

ModelComponent* AIAssistant::LoadInstance(Model* model, PersistenceRecord* fields) {
    auto* component = new AIAssistant(model); component->_loadInstance(fields); return component;
}
ModelDataDefinition* AIAssistant::NewInstance(Model* model, std::string name) { return new AIAssistant(model, name); }

void AIAssistant::setAISupport(AISupport* value) {
    _aiSupport = value; _aiSupportName = value != nullptr ? value->getName() : "";
    if (value != nullptr) _attachedDataInsert("AISupport", value); else _attachedDataRemove("AISupport");
}
AISupport* AIAssistant::getAISupport() const { return _aiSupport; }
void AIAssistant::setAISupportName(std::string value) { _aiSupportName = std::move(value); _aiSupport = nullptr; }
std::string AIAssistant::getAISupportName() const { return _aiSupport != nullptr ? _aiSupport->getName() : _aiSupportName; }
#define AI_STRING_PROPERTY(Name, Field) \
void AIAssistant::set##Name(std::string value) { Field = std::move(value); } \
std::string AIAssistant::get##Name() const { return Field; }
AI_STRING_PROPERTY(PromptTemplate, _promptTemplate)
AI_STRING_PROPERTY(ConversationScope, _conversationScope)
AI_STRING_PROPERTY(ConversationKeyExpression, _conversationKeyExpression)
AI_STRING_PROPERTY(ActionMappings, _actionMappings)
AI_STRING_PROPERTY(DelayExpression, _delayExpression)
AI_STRING_PROPERTY(SaveConfidenceAttribute, _saveConfidenceAttribute)
AI_STRING_PROPERTY(SaveSuccessAttribute, _saveSuccessAttribute)
#undef AI_STRING_PROPERTY
void AIAssistant::setDefaultOutputPort(unsigned int v) { _defaultOutputPort = v; }
unsigned int AIAssistant::getDefaultOutputPort() const { return _defaultOutputPort; }
void AIAssistant::setErrorOutputPort(unsigned int v) { _errorOutputPort = v; }
unsigned int AIAssistant::getErrorOutputPort() const { return _errorOutputPort; }
void AIAssistant::setAllowDirectOutputPort(bool v) { _allowDirectOutputPort = v; }
bool AIAssistant::isAllowDirectOutputPort() const { return _allowDirectOutputPort; }
void AIAssistant::setMinimumConfidence(double v) { _minimumConfidence = v; }
double AIAssistant::getMinimumConfidence() const { return _minimumConfidence; }
std::string AIAssistant::getLastPrompt() const { return _lastPrompt; }
std::string AIAssistant::getLastResponse() const { return _lastResponse; }
std::string AIAssistant::getLastError() const { return _lastError; }
bool AIAssistant::renderPrompt(std::string& result, std::string& error) { return _renderPrompt(result, error); }

std::string AIAssistant::show() { return ModelComponent::show() + ",aiSupport=\"" + getAISupportName() + "\",actionMappings=\"" + _actionMappings + "\""; }

void AIAssistant::_onDispatchEvent(Entity* entity, unsigned int) {
    AISupport* support = _resolveSupport();
    if (support == nullptr) { _fail(entity, "AISupport '" + _aiSupportName + "' was not found."); return; }
    std::string error;
    if (!_renderPrompt(_lastPrompt, error)) { _fail(entity, error); return; }
    std::string key = _conversationKey(entity, error);
    if (!error.empty()) { _fail(entity, error); return; }
    AIConversationResponse response = support->converse(key, _lastPrompt);
    _lastResponse = response.content;
    if (!response.success) { _fail(entity, response.errorMessage); return; }
    ParsedResponse parsed = _parseResponse(response.content);
    if (!parsed.valid) { _fail(entity, parsed.error); return; }
    bool routed = false; unsigned int port = _selectPort(parsed, routed);
    if (!routed || parsed.confidence < _minimumConfidence) { _fail(entity, "AI response has no known action or sufficient confidence."); return; }
    for (const auto& [attribute, value] : parsed.values) entity->setAttributeValue(attribute, value, "", true);
    if (!_saveConfidenceAttribute.empty()) entity->setAttributeValue(_saveConfidenceAttribute, parsed.confidence, "", true);
    if (!_saveSuccessAttribute.empty()) entity->setAttributeValue(_saveSuccessAttribute, 1.0, "", true);
    bool delayOk = false; std::string delayError;
    double delay = _parentModel->parseExpression(_delayExpression, delayOk, delayError);
    if (!delayOk || delay < 0.0) { _fail(entity, "Invalid DelayExpression: " + delayError); return; }
    traceSimulation(this, "AI action '" + parsed.action + "' selected output " + std::to_string(port));
    _lastError.clear(); _forward(entity, port, delay);
}

AISupport* AIAssistant::_resolveSupport() {
    if (_aiSupport == nullptr && !_aiSupportName.empty()) {
        _aiSupport = dynamic_cast<AISupport*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<AISupport>(), _aiSupportName));
        if (_aiSupport != nullptr) _attachedDataInsert("AISupport", _aiSupport);
    }
    return _aiSupport;
}

bool AIAssistant::_renderPrompt(std::string& result, std::string& error) {
    result.clear(); std::size_t position = 0;
    while (position < _promptTemplate.size()) {
        if (_promptTemplate.compare(position, 2, "{{") == 0) {
            result += '{'; position += 2; continue;
        }
        if (_promptTemplate.compare(position, 2, "}}") == 0) {
            result += '}'; position += 2; continue;
        }
        if (_promptTemplate[position] != '{') {
            result += _promptTemplate[position++]; continue;
        }
        std::size_t close = _promptTemplate.find('}', position + 1);
        if (close == std::string::npos) { error = "Unclosed expression placeholder in PromptTemplate."; return false; }
        std::string expression = trim(_promptTemplate.substr(position + 1, close - position - 1));
        bool success = false; std::string parseError;
        double value = _parentModel->parseExpression(expression, success, parseError);
        if (!success) { error = "Could not evaluate placeholder {" + expression + "}: " + parseError; return false; }
        std::ostringstream formatted; formatted << std::setprecision(15) << value;
        result += formatted.str(); position = close + 1;
    }
    return true;
}

std::string AIAssistant::_conversationKey(Entity* entity, std::string& error) {
    std::string scope = lower(_conversationScope);
    if (scope == "none") return "";
    if (scope == "percomponent") return getName();
    if (scope == "perentity") return getName() + ":entity:" + std::to_string(entity->getId());
    if (scope == "expression") {
        bool success = false; std::string parseError;
        double value = _parentModel->parseExpression(_conversationKeyExpression, success, parseError);
        if (!success) { error = "Invalid ConversationKeyExpression: " + parseError; return ""; }
        std::ostringstream key; key << getName() << ":" << std::setprecision(15) << value; return key.str();
    }
    error = "ConversationScope must be None, PerComponent, PerEntity, or Expression."; return "";
}

AIAssistant::ParsedResponse AIAssistant::_parseResponse(const std::string& content) const {
    ParsedResponse result;
    if (!jsonString(content, "action", result.action) && !_allowDirectOutputPort) { result.error = "AI response does not contain a string action."; return result; }
    double number = 0.0;
    if (jsonNumber(content, "output_port", number)) result.outputPort = static_cast<int>(number);
    if (jsonNumber(content, "confidence", number)) result.confidence = number;
    jsonString(content, "explanation", result.explanation);
    std::smatch valuesMatch;
    if (std::regex_search(content, valuesMatch, std::regex("\\\"values\\\"\\s*:\\s*\\{([^}]*)\\}"))) {
        const std::string body = valuesMatch[1].str();
        std::regex item("\\\"([^\\\"]+)\\\"\\s*:\\s*(-?[0-9]+(?:\\.[0-9]+)?(?:[eE][+-]?[0-9]+)?)");
        for (auto it = std::sregex_iterator(body.begin(), body.end(), item); it != std::sregex_iterator(); ++it)
            result.values[(*it)[1].str()] = std::stod((*it)[2].str());
    }
    result.valid = true; return result;
}

std::map<std::string, unsigned int> AIAssistant::_parseActionMappings() const {
    std::map<std::string, unsigned int> mappings; std::stringstream input(_actionMappings); std::string item;
    while (std::getline(input, item, ',')) {
        std::size_t separator = item.find(':'); if (separator == std::string::npos) continue;
        try { mappings[lower(trim(item.substr(0, separator)))] = static_cast<unsigned int>(std::stoul(trim(item.substr(separator + 1)))); } catch (...) {}
    }
    return mappings;
}

unsigned int AIAssistant::_selectPort(const ParsedResponse& response, bool& success) const {
    auto mappings = _parseActionMappings(); auto found = mappings.find(lower(response.action));
    if (found != mappings.end()) { success = true; return found->second; }
    if (_allowDirectOutputPort && response.outputPort >= 0) { success = true; return static_cast<unsigned int>(response.outputPort); }
    success = false; return _defaultOutputPort;
}

void AIAssistant::_forward(Entity* entity, unsigned int port, double delay) {
    Connection* connection = getConnectionManager()->getConnectionAtPort(port);
    if (connection == nullptr || connection->component == nullptr) { traceError("AIAssistant output port " + std::to_string(port) + " is not connected."); return; }
    _parentModel->sendEntityToComponent(entity, connection, delay);
}

void AIAssistant::_fail(Entity* entity, const std::string& message) {
    _lastError = message; traceError("AIAssistant '" + getName() + "': " + message);
    if (!_saveSuccessAttribute.empty()) entity->setAttributeValue(_saveSuccessAttribute, 0.0, "", true);
    _forward(entity, _errorOutputPort, 0.0);
}

bool AIAssistant::_loadInstance(PersistenceRecord* f) {
    bool ok = ModelComponent::_loadInstance(f);
    _aiSupportName = f->loadField("aiSupport", DEFAULT.aiSupportName);
    _promptTemplate = f->loadField("promptTemplate", DEFAULT.promptTemplate);
    _conversationScope = f->loadField("conversationScope", DEFAULT.conversationScope);
    _conversationKeyExpression = f->loadField("conversationKeyExpression", DEFAULT.conversationKeyExpression);
    _actionMappings = f->loadField("actionMappings", DEFAULT.actionMappings);
    _delayExpression = f->loadField("delayExpression", DEFAULT.delayExpression);
    _defaultOutputPort = f->loadField("defaultOutputPort", DEFAULT.defaultOutputPort);
    _errorOutputPort = f->loadField("errorOutputPort", DEFAULT.errorOutputPort);
    _allowDirectOutputPort = f->loadField("allowDirectOutputPort", DEFAULT.allowDirectOutputPort);
    _minimumConfidence = f->loadField("minimumConfidence", DEFAULT.minimumConfidence);
    _saveConfidenceAttribute = f->loadField("saveConfidenceAttribute", DEFAULT.saveConfidenceAttribute);
    _saveSuccessAttribute = f->loadField("saveSuccessAttribute", DEFAULT.saveSuccessAttribute);
    return ok;
}

void AIAssistant::_saveInstance(PersistenceRecord* f, bool defaults) {
    ModelComponent::_saveInstance(f, defaults);
    f->saveField("aiSupport", getAISupportName(), DEFAULT.aiSupportName, defaults);
    f->saveField("promptTemplate", _promptTemplate, DEFAULT.promptTemplate, defaults);
    f->saveField("conversationScope", _conversationScope, DEFAULT.conversationScope, defaults);
    f->saveField("conversationKeyExpression", _conversationKeyExpression, DEFAULT.conversationKeyExpression, defaults);
    f->saveField("actionMappings", _actionMappings, DEFAULT.actionMappings, defaults);
    f->saveField("delayExpression", _delayExpression, DEFAULT.delayExpression, defaults);
    f->saveField("defaultOutputPort", _defaultOutputPort, DEFAULT.defaultOutputPort, defaults);
    f->saveField("errorOutputPort", _errorOutputPort, DEFAULT.errorOutputPort, defaults);
    f->saveField("allowDirectOutputPort", _allowDirectOutputPort, DEFAULT.allowDirectOutputPort, defaults);
    f->saveField("minimumConfidence", _minimumConfidence, DEFAULT.minimumConfidence, defaults);
    f->saveField("saveConfidenceAttribute", _saveConfidenceAttribute, DEFAULT.saveConfidenceAttribute, defaults);
    f->saveField("saveSuccessAttribute", _saveSuccessAttribute, DEFAULT.saveSuccessAttribute, defaults);
}

bool AIAssistant::_check(std::string& error) {
    bool ok = true;
    if (_resolveSupport() == nullptr) { error += "AIAssistant requires an existing AISupport. "; ok = false; }
    if (_promptTemplate.empty()) { error += "AIAssistant PromptTemplate must not be empty. "; ok = false; }
    if (_parseActionMappings().empty() && !_allowDirectOutputPort) { error += "AIAssistant requires at least one ActionMapping. "; ok = false; }
    ok &= _parentModel->checkExpression(_delayExpression, "AIAssistant DelayExpression", error);
    return ok;
}
