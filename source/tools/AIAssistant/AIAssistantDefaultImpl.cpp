/*
 * File:   AIAssistantDefaultImpl.cpp
 * Author: Genesys Team
 *
 * Created on 28 de Maio de 2026
 */

#include "AIAssistantDefaultImpl.h"

// Full includes required for calls into facade / plugin / provider methods.
// AIAssistant_if.h uses only forward declarations.
#include "kernel/simulator/SimulatorFacade.h"
#include "kernel/simulator/Plugin.h"
#include "kernel/simulator/PluginInformation.h"

#include "OpenAIProviderClientImpl.h"
#include "AnthropicProviderClientImpl.h"
#include "LocalProviderClientImpl.h"

#include "kernel/simulator/SimulationControlAndResponse.h"
#include "kernel/util/Util.h"

#include <algorithm>   // std::fill
#include <chrono>      // steady_clock for timing
#include <cstdlib>     // std::getenv
#include <cstdio>      // std::snprintf
#include <sstream>
#include <unistd.h>    // mkstemp, write, close, unlink
#include <sys/stat.h>  // fchmod

// ---------------------------------------------------------------------------
// Module-level helpers
// ---------------------------------------------------------------------------

static std::string providerToString(AIProvider p) {
    switch (p) {
        case AIProvider::OpenAI:     return "OpenAI";
        case AIProvider::Anthropic:  return "Anthropic";
        case AIProvider::Local:      return "Local";
        case AIProvider::CustomHttp: return "CustomHttp";
        default:                     return "Undefined";
    }
}

static std::string promptPreview(const AIAssistantRequest& req) {
    const std::string& src = req.userPrompt.empty()
                             ? (req.modelDescription.empty()
                                ? req.experimentDescription
                                : req.modelDescription)
                             : req.userPrompt;
    return src.size() > 120 ? src.substr(0, 120) : src;
}

// ---------------------------------------------------------------------------
// Constructor / destructor
// ---------------------------------------------------------------------------

AIAssistantDefaultImpl::AIAssistantDefaultImpl() {
    _updateState();
}

AIAssistantDefaultImpl::~AIAssistantDefaultImpl() {
    // Overwrite the API key before destruction.
    clearApiKey();
    // Delete owned provider client, if any.
    if (_ownsProviderClient) {
        delete _providerClient;
        _providerClient = nullptr;
        _ownsProviderClient = false;
    }
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

void AIAssistantDefaultImpl::setConfiguration(const AIAssistantConfiguration& configuration) {
    _config = configuration;
    // API key is not part of AIAssistantConfiguration and is not transferred here.
    _updateState();
}

AIAssistantConfiguration AIAssistantDefaultImpl::getConfiguration() const {
    return _config;
}

void AIAssistantDefaultImpl::setProvider(AIProvider provider) {
    _config.provider = provider;
    _updateState();
}

AIProvider AIAssistantDefaultImpl::getProvider() const {
    return _config.provider;
}

void AIAssistantDefaultImpl::setProviderName(const std::string& providerName) {
    _config.providerName = providerName;
}

std::string AIAssistantDefaultImpl::getProviderName() const {
    return _config.providerName;
}

void AIAssistantDefaultImpl::setModelName(const std::string& modelName) {
    _config.modelName = modelName;
    _updateState();
}

std::string AIAssistantDefaultImpl::getModelName() const {
    return _config.modelName;
}

void AIAssistantDefaultImpl::setBaseUrl(const std::string& baseUrl) {
    _config.baseUrl = baseUrl;
    _updateState();
}

std::string AIAssistantDefaultImpl::getBaseUrl() const {
    return _config.baseUrl;
}

void AIAssistantDefaultImpl::setReasoningEffort(ReasoningEffort effort) {
    _config.reasoningEffort = effort;
}

ReasoningEffort AIAssistantDefaultImpl::getReasoningEffort() const {
    return _config.reasoningEffort;
}

void AIAssistantDefaultImpl::setTemperature(double temperature) {
    _config.temperature = temperature;
}

double AIAssistantDefaultImpl::getTemperature() const {
    return _config.temperature;
}

void AIAssistantDefaultImpl::setMaxOutputTokens(unsigned int maxOutputTokens) {
    _config.maxOutputTokens = maxOutputTokens;
}

unsigned int AIAssistantDefaultImpl::getMaxOutputTokens() const {
    return _config.maxOutputTokens;
}

void AIAssistantDefaultImpl::setTimeoutSeconds(unsigned int timeoutSeconds) {
    _config.timeoutSeconds = timeoutSeconds;
}

unsigned int AIAssistantDefaultImpl::getTimeoutSeconds() const {
    return _config.timeoutSeconds;
}

// ---------------------------------------------------------------------------
// Security and secrets
// ---------------------------------------------------------------------------

void AIAssistantDefaultImpl::setApiKey(const std::string& apiKey) {
    std::fill(_apiKey.begin(), _apiKey.end(), '\0');
    _apiKey = apiKey;
    _updateState();
}

void AIAssistantDefaultImpl::clearApiKey() {
    // Best-effort: overwrite buffer before releasing.
    // Note: the C++ standard does not guarantee the compiler will not optimise
    // away this fill. Use explicit_bzero / SecureZeroMemory in deployments where
    // the key must be reliably erased from physical memory.
    std::fill(_apiKey.begin(), _apiKey.end(), '\0');
    _apiKey.clear();
    _updateState();
}

bool AIAssistantDefaultImpl::hasApiKey() const {
    return !_apiKey.empty();
}

std::string AIAssistantDefaultImpl::getMaskedApiKey() const {
    if (_apiKey.empty()) {
        return "";
    }
    const std::size_t visibleTail = 4;
    if (_apiKey.size() <= visibleTail) {
        return "****";
    }
    return "****" + _apiKey.substr(_apiKey.size() - visibleTail);
}

void AIAssistantDefaultImpl::setApiKeyEnvironmentVariable(const std::string& variableName) {
    _config.apiKeyEnvironmentVariable = variableName;
}

std::string AIAssistantDefaultImpl::getApiKeyEnvironmentVariable() const {
    return _config.apiKeyEnvironmentVariable;
}

bool AIAssistantDefaultImpl::loadApiKeyFromEnvironment() {
    if (_config.apiKeyEnvironmentVariable.empty()) {
        _lastDiagnostics = "loadApiKeyFromEnvironment: no environment variable name configured.";
        return false;
    }
    const char* value = std::getenv(_config.apiKeyEnvironmentVariable.c_str());
    if (value == nullptr || value[0] == '\0') {
        _lastDiagnostics = "loadApiKeyFromEnvironment: variable '"
                         + _config.apiKeyEnvironmentVariable
                         + "' is not set or is empty.";
        return false;
    }
    std::fill(_apiKey.begin(), _apiKey.end(), '\0');
    _apiKey = std::string(value);
    _lastDiagnostics = "loadApiKeyFromEnvironment: key loaded from '"
                     + _config.apiKeyEnvironmentVariable + "'.";
    _updateState();
    return true;
}

// ---------------------------------------------------------------------------
// Integration with GenESyS — SimulatorFacade
// ---------------------------------------------------------------------------

void AIAssistantDefaultImpl::setSimulatorFacade(SimulatorFacade* simulatorFacade) {
    _simulatorFacade = simulatorFacade;
}

SimulatorFacade* AIAssistantDefaultImpl::getSimulatorFacade() const {
    return _simulatorFacade;
}

bool AIAssistantDefaultImpl::hasSimulatorFacade() const {
    return _simulatorFacade != nullptr;
}

// ---------------------------------------------------------------------------
// Provider client
// ---------------------------------------------------------------------------

void AIAssistantDefaultImpl::setProviderClient(AIProviderClient_if* client) {
    // Release any previously owned client.
    if (_ownsProviderClient) {
        delete _providerClient;
        _ownsProviderClient = false;
    }
    _providerClient = client;
    // Injected client is not owned by this object.
}

AIProviderClient_if* AIAssistantDefaultImpl::getProviderClient() const {
    return _providerClient;
}

bool AIAssistantDefaultImpl::hasProviderClient() const {
    return _providerClient != nullptr;
}

bool AIAssistantDefaultImpl::createAndAttachProviderClient() {
    // Release any previously owned client.
    if (_ownsProviderClient) {
        delete _providerClient;
        _providerClient = nullptr;
        _ownsProviderClient = false;
    }

    AIProviderClient_if* client = nullptr;

    switch (_config.provider) {
    case AIProvider::OpenAI: {
        auto* c = new OpenAIProviderClientImpl();
        if (!_config.organizationId.empty()) c->setOrganizationId(_config.organizationId);
        if (!_config.projectId.empty())      c->setProjectId(_config.projectId);
        client = c;
        break;
    }
    case AIProvider::Anthropic:
        client = new AnthropicProviderClientImpl();
        break;
    case AIProvider::Local: {
        auto* c = new LocalProviderClientImpl();
        if (!_config.baseUrl.empty()) c->setBaseUrl(_config.baseUrl);
        client = c;
        break;
    }
    case AIProvider::CustomHttp: {
        // CustomHttp uses LocalProviderClientImpl (OpenAI-compatible) with a custom URL.
        auto* c = new LocalProviderClientImpl();
        if (!_config.baseUrl.empty()) c->setBaseUrl(_config.baseUrl);
        client = c;
        break;
    }
    case AIProvider::Undefined:
    default:
        _lastError = "createAndAttachProviderClient: provider is Undefined; "
                     "call setProvider() before creating a client.";
        return false;
    }

    // Transfer the API key.
    if (!_apiKey.empty()) {
        client->setApiKey(_apiKey);
    }
    client->setTimeoutSeconds(_config.timeoutSeconds);

    _providerClient     = client;
    _ownsProviderClient = true;
    return true;
}

// ---------------------------------------------------------------------------
// Knowledge base
// ---------------------------------------------------------------------------

void AIAssistantDefaultImpl::clearKnowledgeBase() {
    _knowledgeItems.clear();
}

void AIAssistantDefaultImpl::addKnowledgeItem(const GenesysKnowledgeItem& item) {
    _knowledgeItems.push_back(item);
}

std::vector<GenesysKnowledgeItem> AIAssistantDefaultImpl::getKnowledgeItems() const {
    return _knowledgeItems;
}

GenesysKnowledgeBaseSummary AIAssistantDefaultImpl::getKnowledgeBaseSummary() const {
    GenesysKnowledgeBaseSummary summary;
    for (const GenesysKnowledgeItem& item : _knowledgeItems) {
        if (item.category == "Component") {
            ++summary.numberOfComponents;
        } else if (item.category == "DataDefinition") {
            ++summary.numberOfDataDefinitions;
        } else if (item.category == "Plugin") {
            ++summary.numberOfPlugins;
        }
    }
    summary.summaryMarkdown = exportKnowledgeBaseAsMarkdown();
    return summary;
}

std::string AIAssistantDefaultImpl::exportKnowledgeBaseAsMarkdown() const {
    if (_knowledgeItems.empty()) {
        return "# GenESyS Knowledge Base\n\n*No items registered.*\n";
    }

    std::ostringstream md;
    md << "# GenESyS Knowledge Base\n\n";
    md << "| Name | Category | Type | Description |\n";
    md << "|------|----------|------|-------------|\n";
    for (const GenesysKnowledgeItem& item : _knowledgeItems) {
        md << "| " << item.name
           << " | " << item.category
           << " | " << item.typeName
           << " | " << item.description
           << " |\n";
    }
    md << "\n## Usage and Constraints\n\n";
    for (const GenesysKnowledgeItem& item : _knowledgeItems) {
        if (!item.usage.empty() || !item.constraints.empty()) {
            md << "### " << item.name << "\n";
            if (!item.usage.empty()) {
                md << "**Usage:** " << item.usage << "\n\n";
            }
            if (!item.constraints.empty()) {
                md << "**Constraints:** " << item.constraints << "\n\n";
            }
        }
    }
    return md.str();
}

bool AIAssistantDefaultImpl::refreshKnowledgeBaseFromSimulator(std::string* message) {
    // -----------------------------------------------------------------
    // Stage 2 — Plugin introspection via SimulatorFacade
    // -----------------------------------------------------------------
    clearKnowledgeBase();

    // 1. Static catalog — well-known GenESyS building blocks.
    //    These are always populated so the assistant has baseline knowledge
    //    even when no SimulatorFacade is attached or plugins are not loaded.
    static const struct {
        const char* name;
        const char* category;
        const char* typeName;
        const char* description;
        const char* usage;
        const char* constraints;
    } STATIC_CATALOG[] = {
        {
            "Create",
            "Component",
            "Create",
            "Generates entities (arrivals) according to an inter-arrival time distribution. "
            "Represents the entry point of entities into the model.",
            "Create c(name='arrivals', timeBetweenCreations='Expo(1/lambda)', firstCreation='0', "
            "maxCreations='MAX_INT');",
            "Source component: no predecessors required. "
            "Connect output to the next component in the flow."
        },
        {
            "Dispose",
            "Component",
            "Dispose",
            "Permanently removes entities from the model. "
            "Represents a sink or termination point of the flow.",
            "Dispose d(name='exit');",
            "Sink component: no successors required. "
            "Typically connected from the last step in an entity path."
        },
        {
            "Queue",
            "DataDefinition",
            "Queue",
            "First-in first-out waiting line that holds entities until a resource becomes available.",
            "Queue q(name='waitingLine');",
            "Used together with Seize and Release. "
            "Capacity defaults to infinite."
        },
        {
            "Seize",
            "Component",
            "Seize",
            "Requests one or more units of a Resource. "
            "Entities that cannot be satisfied immediately wait in the associated Queue.",
            "Seize sz(name='grabServer', resource='server', qty='1', queue='waitingLine');",
            "Must be paired with a Release. "
            "Resource and Queue must be declared as DataDefinitions."
        },
        {
            "Release",
            "Component",
            "Release",
            "Frees one or more units of a Resource previously seized by an entity.",
            "Release rl(name='freeServer', resource='server', qty='1');",
            "Must follow a matching Seize in the entity flow."
        },
        {
            "Process",
            "Component",
            "Process",
            "Combined Seize-Delay-Release block for common single-server patterns.",
            "Process p(name='service', resource='server', qty='1', delay='Norm(mu,sigma)', "
            "queue='waitingLine');",
            "Convenience macro: equivalent to Seize + Delay + Release in sequence."
        },
        {
            "Delay",
            "Component",
            "Delay",
            "Holds an entity for a specified duration without consuming a resource.",
            "Delay d(name='processing', delayTime='Unif(a,b)');",
            "No resource interaction. Use Process instead if resource seizure is needed."
        },
        {
            "Decide",
            "Component",
            "Decide",
            "Routes entities to one of two or more paths based on a condition or probability.",
            "Decide dc(name='branch', type='2-way-by-condition', condition='attribute1>5');",
            "Has multiple output ports (true/false for 2-way, or percentage splits). "
            "Connect each output to the appropriate continuation."
        },
        {
            "Assign",
            "Component",
            "Assign",
            "Sets entity attributes, variables or other model values.",
            "Assign a(name='setAttr', assignments={'attr1=Expo(1)','var1=var1+1'});",
            "No routing logic; passes entity to next component after assignments."
        },
        {
            "Record",
            "Component",
            "Record",
            "Collects a statistic (observation) at the point an entity passes through.",
            "Record r(name='collectTime', expression='tnow-attr_arrivalTime', "
            "statistics='time_in_system');",
            "Does not affect entity flow. The statistics name must match a "
            "declared Statistics data definition."
        },
        {
            "Resource",
            "DataDefinition",
            "Resource",
            "Countable, reusable unit consumed by Seize and released by Release.",
            "Resource res(name='server', capacity='1');",
            "Declare before any Seize/Release/Process that references it."
        },
        {
            "Entity",
            "DataDefinition",
            "Entity",
            "Template for objects that flow through the model, carrying attributes.",
            "Entity e(name='customer');",
            "Attributes are defined on the entity type and accessed in expressions."
        },
        {
            "Statistics",
            "DataDefinition",
            "Statistics",
            "Accumulator for collecting output performance metrics.",
            "Statistics s(name='time_in_system', type='time-series');",
            "Referenced by Record components and included in simulation reports."
        },
        {
            "Variable",
            "DataDefinition",
            "Variable",
            "Global mutable numerical value accessible from any component expression.",
            "Variable v(name='counter', initialValue='0');",
            "Not entity-local; shared across all entities. "
            "Use entity attributes for per-entity state."
        }
    };

    for (const auto& s : STATIC_CATALOG) {
        GenesysKnowledgeItem item;
        item.name        = s.name;
        item.category    = s.category;
        item.typeName    = s.typeName;
        item.description = s.description;
        item.usage       = s.usage;
        item.constraints = s.constraints;
        _knowledgeItems.push_back(item);
    }

    if (_simulatorFacade == nullptr) {
        if (message != nullptr) {
            *message = "Static catalog loaded ("
                     + std::to_string(_knowledgeItems.size())
                     + " items). No SimulatorFacade attached — dynamic plugin "
                       "introspection skipped.";
        }
        return true; // Static catalog is still useful.
    }

    // 2. Dynamic introspection — enumerate loaded plugins.
    unsigned int dynamicCount = 0;
    Plugin* plugin = _simulatorFacade->firstPlugin();
    while (plugin != nullptr) {
        PluginInformation* info = plugin->getPluginInfo();
        if (info != nullptr) {
            GenesysKnowledgeItem item;
            item.typeName    = info->getPluginTypename();
            item.name        = item.typeName;
            item.description = info->getDescriptionHelp();

            if (info->isComponent()) {
                item.category = "Component";
                // Build constraint description from input/output topology.
                std::ostringstream constraints;
                constraints << "Inputs: "   << info->getMinimumInputs()
                            << ".."        << info->getMaximumInputs()
                            << "; Outputs: " << info->getMinimumOutputs()
                            << ".."        << info->getMaximumOutputs();
                if (info->isSource())       constraints << "; source (no predecessor required)";
                if (info->isSink())         constraints << "; sink (no successor required)";
                if (info->isSendTransfer()) constraints << "; may send transfers";
                if (info->isReceiveTransfer()) constraints << "; may receive transfers";
                item.constraints = constraints.str();
            } else {
                item.category = "DataDefinition";
            }

            // Language template as usage example (may be empty for some plugins).
            item.usage = info->getLanguageTemplate();

            // Avoid duplicating static catalog entries.
            bool alreadyPresent = false;
            for (const GenesysKnowledgeItem& existing : _knowledgeItems) {
                if (existing.typeName == item.typeName) {
                    alreadyPresent = true;
                    break;
                }
            }
            if (!alreadyPresent) {
                _knowledgeItems.push_back(item);
                ++dynamicCount;
            }
        }
        plugin = _simulatorFacade->nextPlugin();
    }

    if (message != nullptr) {
        std::ostringstream msg;
        msg << "Knowledge base refreshed: "
            << _knowledgeItems.size() << " total items ("
            << (_knowledgeItems.size() - dynamicCount) << " static, "
            << dynamicCount << " from loaded plugins).";
        *message = msg.str();
    }
    return true;
}

// ---------------------------------------------------------------------------
// Workflow
// ---------------------------------------------------------------------------

bool AIAssistantDefaultImpl::checkReady(std::string* message) const {
    if (_config.provider == AIProvider::Undefined) {
        if (message != nullptr) {
            *message = "AI provider not configured. "
                       "Call setProvider() or setConfiguration() with a valid AIProvider.";
        }
        return false;
    }

    if (_config.modelName.empty()) {
        if (message != nullptr) {
            *message = "Model name not configured. "
                       "Call setModelName() with the provider's model identifier "
                       "(e.g. \"gpt-4o\", \"claude-opus-4-7\").";
        }
        return false;
    }

    if (_providerRequiresApiKey() && !hasApiKey()) {
        if (_config.apiKeyEnvironmentVariable.empty()) {
            if (message != nullptr) {
                *message = "API key not set and no environment variable configured. "
                           "Call setApiKey() or setApiKeyEnvironmentVariable() "
                           "followed by loadApiKeyFromEnvironment().";
            }
        } else {
            if (message != nullptr) {
                *message = "API key not loaded. "
                           "Call loadApiKeyFromEnvironment() to read from '"
                           + _config.apiKeyEnvironmentVariable + "'.";
            }
        }
        return false;
    }

    if (_providerRequiresBaseUrl() && _config.baseUrl.empty()) {
        if (message != nullptr) {
            *message = "Base URL required for the CustomHttp provider. "
                       "Call setBaseUrl() with the chat-completions endpoint.";
        }
        return false;
    }

    if (message != nullptr) {
        *message = "Ready for prompt analysis. "
                   "Note: model construction and simulation require SimulatorFacade "
                   "and the corresponding allowModelMutation / allowSimulationExecution flags.";
    }
    return true;
}

// ---------------------------------------------------------------------------
// Workflow — Stage 4: analyzePrompt and createModelPlan
// ---------------------------------------------------------------------------

AIAssistantResponse AIAssistantDefaultImpl::analyzePrompt(
        const AIAssistantRequest& request) {
    const auto _t0 = std::chrono::steady_clock::now();
    const std::string _pp = promptPreview(request);
    auto _audit = [&](const AIAssistantResponse& r) {
        _appendAudit("analyzePrompt", r,
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - _t0).count(), _pp);
    };

    // Permission check
    if (!_config.allowNetworkAccess) {
        AIAssistantResponse response;
        response.success = false;
        response.state   = ExecutionState::Failed;
        response.message = "Network access is not permitted. "
                           "Set AIAssistantConfiguration::allowNetworkAccess = true to enable it.";
        _state     = ExecutionState::Failed;
        _lastError = response.message;
        _audit(response);
        return response;
    }

    // Readiness check
    std::string readyMsg;
    if (!checkReady(&readyMsg)) {
        AIAssistantResponse response;
        response.success = false;
        response.state   = ExecutionState::Failed;
        response.message = "Assistant is not ready: " + readyMsg;
        _state     = ExecutionState::Failed;
        _lastError = response.message;
        _audit(response);
        return response;
    }

    // Provider client check
    if (_providerClient == nullptr) {
        AIAssistantResponse response;
        response.success = false;
        response.state   = ExecutionState::Failed;
        response.message = "No provider client attached. "
                           "Call setProviderClient() or createAndAttachProviderClient() first.";
        _state     = ExecutionState::Failed;
        _lastError = response.message;
        _audit(response);
        return response;
    }

    if (!_providerClient->isAvailable()) {
        AIAssistantResponse response;
        response.success = false;
        response.state   = ExecutionState::Failed;
        response.message = "Provider client is not available "
                           "(check API key and base URL configuration).";
        _state     = ExecutionState::Failed;
        _lastError = response.message;
        _audit(response);
        return response;
    }

    _state = ExecutionState::Running;

    // Build the conversation
    ProviderRequest provReq;
    provReq.model       = _config.modelName;
    provReq.temperature = _config.temperature;
    provReq.maxTokens   = _config.maxOutputTokens;
    provReq.highReasoningMode = (_config.reasoningEffort == ReasoningEffort::High);

    // System message
    std::ostringstream systemMsg;
    systemMsg << "You are a GenESyS discrete-event simulation modelling assistant. "
              << "Your task is to analyse a user's description of a system and "
              << "identify the simulation concepts involved (entities, resources, "
              << "queues, arrival processes, service steps, routing logic, etc.).\n\n";

    if (!_knowledgeItems.empty()) {
        systemMsg << "## Available GenESyS Building Blocks\n\n"
                  << exportKnowledgeBaseAsMarkdown() << "\n\n";
    }

    if (!request.systemContext.empty()) {
        systemMsg << "## Additional Context\n\n" << request.systemContext << "\n\n";
    }

    systemMsg << "Respond with:\n"
              << "1. A concise summary of the simulation scenario.\n"
              << "2. The key simulation elements identified (entities, resources, "
              << "processes, arrival rates, service patterns).\n"
              << "3. Any ambiguities or missing information that would affect model construction.\n";

    ChatMessage sysMessage;
    sysMessage.role    = "system";
    sysMessage.content = systemMsg.str();
    provReq.messages.push_back(sysMessage);

    // User message
    std::ostringstream userMsg;
    if (!request.modelDescription.empty()) {
        userMsg << "## System Description\n\n" << request.modelDescription << "\n\n";
    }
    if (!request.experimentDescription.empty()) {
        userMsg << "## Experiment\n\n" << request.experimentDescription << "\n\n";
    }
    if (!request.userPrompt.empty()) {
        userMsg << request.userPrompt;
    }

    ChatMessage userMessage;
    userMessage.role    = "user";
    userMessage.content = userMsg.str().empty() ? "(no prompt provided)" : userMsg.str();
    provReq.messages.push_back(userMessage);

    // Send request
    const ProviderResponse provResp = _providerClient->send(provReq);

    AIAssistantResponse response;
    response.diagnostics = "Provider: " + _providerClient->getProviderName()
                         + "; model: " + _config.modelName
                         + "; inputTokens: " + std::to_string(provResp.inputTokens)
                         + "; outputTokens: " + std::to_string(provResp.outputTokens)
                         + "; HTTP: " + std::to_string(provResp.httpStatusCode);

    if (!provResp.success) {
        response.success      = false;
        response.state        = ExecutionState::Failed;
        response.message      = "Provider request failed: " + provResp.errorMessage;
        _state                = ExecutionState::Failed;
        _lastError            = response.message;
        _lastDiagnostics      = response.diagnostics;
        _audit(response);
        return response;
    }

    response.success  = true;
    response.state    = ExecutionState::Finished;
    response.message  = provResp.content;
    _state            = ExecutionState::Finished;
    _lastDiagnostics  = response.diagnostics;
    _audit(response);
    return response;
}

AIAssistantResponse AIAssistantDefaultImpl::createModelPlan(
        const AIAssistantRequest& request) {
    const auto _t0 = std::chrono::steady_clock::now();
    const std::string _pp = promptPreview(request);
    auto _audit = [&](const AIAssistantResponse& r) {
        _appendAudit("createModelPlan", r,
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - _t0).count(), _pp);
    };

    // Permission and readiness checks mirror analyzePrompt.
    if (!_config.allowNetworkAccess) {
        AIAssistantResponse response;
        response.success = false;
        response.state   = ExecutionState::Failed;
        response.message = "Network access is not permitted. "
                           "Set AIAssistantConfiguration::allowNetworkAccess = true to enable it.";
        _state     = ExecutionState::Failed;
        _lastError = response.message;
        _audit(response);
        return response;
    }

    std::string readyMsg;
    if (!checkReady(&readyMsg)) {
        AIAssistantResponse response;
        response.success = false;
        response.state   = ExecutionState::Failed;
        response.message = "Assistant is not ready: " + readyMsg;
        _state     = ExecutionState::Failed;
        _lastError = response.message;
        _audit(response);
        return response;
    }

    if (_providerClient == nullptr || !_providerClient->isAvailable()) {
        AIAssistantResponse response;
        response.success = false;
        response.state   = ExecutionState::Failed;
        response.message = "Provider client not available. "
                           "Call setProviderClient() or createAndAttachProviderClient() first.";
        _state     = ExecutionState::Failed;
        _lastError = response.message;
        _audit(response);
        return response;
    }

    _state = ExecutionState::Running;

    ProviderRequest provReq;
    provReq.model       = _config.modelName;
    provReq.temperature = _config.temperature;
    provReq.maxTokens   = _config.maxOutputTokens;
    provReq.highReasoningMode = (_config.reasoningEffort == ReasoningEffort::High);

    // System message with structured plan instructions
    std::ostringstream systemMsg;
    systemMsg << "You are a GenESyS discrete-event simulation modelling assistant. "
              << "Your task is to produce a structured simulation model plan in Markdown.\n\n";

    if (!_knowledgeItems.empty()) {
        systemMsg << "## Available GenESyS Components and Data Definitions\n\n"
                  << exportKnowledgeBaseAsMarkdown() << "\n\n";
    }

    if (!request.systemContext.empty()) {
        systemMsg << "## Additional Context\n\n" << request.systemContext << "\n\n";
    }

    systemMsg << "## Required Output Format\n\n"
              << "Produce a Markdown document with the following sections:\n\n"
              << "### Model Plan\n"
              << "1. **Entities**: list entity types with relevant attributes.\n"
              << "2. **Resources**: list resources with initial capacity.\n"
              << "3. **Queues**: list queues with discipline (FIFO, LIFO, priority).\n"
              << "4. **Arrival process**: Create component parameters "
              << "(inter-arrival distribution, first creation time, max entities).\n"
              << "5. **Processing steps**: ordered list of components with GenESyS type, "
              << "name, parameters, and routing connections.\n"
              << "6. **Statistics to collect**: list of Statistics data definitions.\n"
              << "7. **Simulation parameters**: number of replications, warm-up time, "
              << "replication length, time unit.\n\n"
              << "Use GenESyS component and distribution syntax in all parameter values "
              << "(e.g. Expo(lambda), Norm(mu,sigma), Unif(a,b)).\n";

    ChatMessage sysMessage;
    sysMessage.role    = "system";
    sysMessage.content = systemMsg.str();
    provReq.messages.push_back(sysMessage);

    // User message
    std::ostringstream userMsg;
    if (!request.modelDescription.empty()) {
        userMsg << "## System Description\n\n" << request.modelDescription << "\n\n";
    }
    if (!request.experimentDescription.empty()) {
        userMsg << "## Experiment Goals\n\n" << request.experimentDescription << "\n\n";
    }
    if (!request.userPrompt.empty()) {
        userMsg << request.userPrompt;
    }

    ChatMessage userMessage;
    userMessage.role    = "user";
    userMessage.content = userMsg.str().empty() ? "(no description provided)" : userMsg.str();
    provReq.messages.push_back(userMessage);

    const ProviderResponse provResp = _providerClient->send(provReq);

    AIAssistantResponse response;
    response.diagnostics = "Provider: " + _providerClient->getProviderName()
                         + "; model: " + _config.modelName
                         + "; inputTokens: " + std::to_string(provResp.inputTokens)
                         + "; outputTokens: " + std::to_string(provResp.outputTokens)
                         + "; HTTP: " + std::to_string(provResp.httpStatusCode);

    if (!provResp.success) {
        response.success      = false;
        response.state        = ExecutionState::Failed;
        response.message      = "Provider request failed: " + provResp.errorMessage;
        _state                = ExecutionState::Failed;
        _lastError            = response.message;
        _lastDiagnostics      = response.diagnostics;
        _audit(response);
        return response;
    }

    response.success       = true;
    response.state         = ExecutionState::Finished;
    response.generatedPlan = provResp.content;
    response.message       = "Model plan generated successfully.";
    _state                 = ExecutionState::Finished;
    _lastDiagnostics       = response.diagnostics;
    _audit(response);
    return response;
}

// ---------------------------------------------------------------------------
// Stage 5 helpers
// ---------------------------------------------------------------------------

/**
 * @brief Extracts clean .gen file content from an LLM response.
 *
 * @details
 * Strips leading prose and markdown code fences, returning only the
 * content from the first `# Genesys Simulation Model` line onwards.
 * Any trailing ``` or plain preamble before the model header is dropped.
 */
static std::string extractGenContent(const std::string& llmOutput) {
    const std::string header = "# Genesys Simulation Model";
    const auto pos = llmOutput.find(header);
    if (pos == std::string::npos) {
        // Fallback: return the whole response trimmed.
        return llmOutput;
    }
    std::string content = llmOutput.substr(pos);
    // Strip trailing markdown code fence if present.
    const std::string fence = "```";
    const auto fencePos = content.rfind(fence);
    if (fencePos != std::string::npos) {
        // Only strip if the fence appears after the last actual line content.
        const auto lastNewline = content.rfind('\n', fencePos);
        if (lastNewline != std::string::npos) {
            content = content.substr(0, lastNewline);
        }
    }
    return content;
}

/**
 * @brief Builds the .gen format reference used in the buildModel system prompt.
 */
static std::string genFormatReference() {
    return
        "## GenESyS Model Language (.gen) Format\n\n"
        "A .gen file must have this exact structure:\n\n"
        "```\n"
        "# Genesys Simulation Model\n"
        "# Simulator, Model and Simulation infos\n"
        "0   Simulator  \"GenESyS\" versionNumber=220517\n"
        "0   ModelInfo  \"ModelName\" version=\"1.0\"\n"
        "0   ModelSimulation \"\" traceLevel=9 warmUpTimeTimeUnit=6 warmUpTime=0.0"
        " replicationLength=480.0 numberOfReplications=5 replicationLengthTimeUnit=6\n"
        "\n"
        "# Model Data Definitions\n"
        "1   EntityType \"Customer\"\n"
        "2   Resource   \"Server\" capacity=1\n"
        "3   Queue      \"WaitingLine\"\n"
        "\n"
        "# Model Components\n"
        "4   Create     \"Arrivals\" entityType=\"Customer\" timeBetweenCreations=\"Expo(10)\""
        " timeBetweenCreationsTimeUnit=6 nextId=5\n"
        "5   Seize      \"GrabServer\" queueable=\"WaitingLine\" requestSeizable[0]=\"Server\""
        " nextId=6\n"
        "6   Delay      \"Service\" delayExpression=\"Norm(8,2)\" delayExpressionTimeUnit=6"
        " nextId=7\n"
        "7   Release    \"FreeServer\" requestSeizable[0]=\"Server\" nextId=8\n"
        "8   Dispose    \"Leave\" nexts=0\n"
        "```\n\n"
        "## Rules\n"
        "- IDs: unique positive integers; 0 is reserved for metadata lines.\n"
        "- `nextId=X` points to the successor component's ID.\n"
        "- Sink components use `nexts=0`. Source components do not need `nextId` from a predecessor.\n"
        "- Time unit codes: 5=second, 6=minute, 7=hour, 8=day.\n"
        "- Distribution syntax: `Expo(mean)`, `Norm(mean,stddev)`, `Unif(min,max)`, `Tria(min,mode,max)`.\n"
        "- String values are quoted; numeric values are unquoted.\n"
        "- Do NOT include any text outside the model file content.\n"
        "- Start the output with `# Genesys Simulation Model` on the very first line.\n";
}

// ---------------------------------------------------------------------------
// Stage 5: buildModel
// ---------------------------------------------------------------------------

AIAssistantResponse AIAssistantDefaultImpl::buildModel(const AIAssistantRequest& request) {
    const auto _t0 = std::chrono::steady_clock::now();
    const std::string _pp = promptPreview(request);
    auto _audit = [&](const AIAssistantResponse& r) {
        _appendAudit("buildModel", r,
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - _t0).count(), _pp);
    };

    if (!_config.allowModelMutation) {
        AIAssistantResponse r;
        r.success = false; r.state = ExecutionState::Failed;
        r.message = "Model construction is not permitted. "
                    "Set AIAssistantConfiguration::allowModelMutation = true.";
        _state = ExecutionState::Failed; _lastError = r.message;
        _audit(r); return r;
    }
    if (!_config.allowNetworkAccess) {
        AIAssistantResponse r;
        r.success = false; r.state = ExecutionState::Failed;
        r.message = "Network access is not permitted. "
                    "Set AIAssistantConfiguration::allowNetworkAccess = true.";
        _state = ExecutionState::Failed; _lastError = r.message;
        _audit(r); return r;
    }
    if (_simulatorFacade == nullptr) {
        AIAssistantResponse r;
        r.success = false; r.state = ExecutionState::Failed;
        r.message = "No SimulatorFacade attached. "
                    "Call setSimulatorFacade() before buildModel().";
        _state = ExecutionState::Failed; _lastError = r.message;
        _audit(r); return r;
    }
    if (_providerClient == nullptr || !_providerClient->isAvailable()) {
        AIAssistantResponse r;
        r.success = false; r.state = ExecutionState::Failed;
        r.message = "Provider client not available. "
                    "Call setProviderClient() or createAndAttachProviderClient() first.";
        _state = ExecutionState::Failed; _lastError = r.message;
        _audit(r); return r;
    }

    _state = ExecutionState::Running;

    // ------------------------------------------------------------------
    // Sandbox: clear current model before building so the LLM starts
    // from an empty canvas without inheriting the user's existing model.
    // ------------------------------------------------------------------
    if (_config.sandboxEnabled && !_config.dryRun) {
        _simulatorFacade->modelClear();
    }

    // ------------------------------------------------------------------
    // 1. Ask the LLM to generate a .gen model file.
    // ------------------------------------------------------------------
    ProviderRequest provReq;
    provReq.model       = _config.modelName;
    provReq.temperature = _config.temperature;
    provReq.maxTokens   = _config.maxOutputTokens;
    provReq.highReasoningMode = (_config.reasoningEffort == ReasoningEffort::High);

    std::ostringstream sysMsg;
    sysMsg << "You are a GenESyS discrete-event simulation model file generator. "
           << "Your task is to produce a syntactically correct GenESyS .gen file "
           << "that represents the simulation model described below.\n\n"
           << genFormatReference();
    if (!_knowledgeItems.empty()) {
        sysMsg << "\n## Available Components and Data Definitions\n\n"
               << exportKnowledgeBaseAsMarkdown() << "\n";
    }

    ChatMessage cm;
    cm.role    = "system";
    cm.content = sysMsg.str();
    provReq.messages.push_back(cm);

    std::ostringstream userMsg;
    if (!request.modelPlan.empty()) {
        userMsg << "## Model Plan\n\n" << request.modelPlan << "\n\n";
    }
    if (!request.modelDescription.empty()) {
        userMsg << "## System Description\n\n" << request.modelDescription << "\n\n";
    }
    if (!request.experimentDescription.empty()) {
        userMsg << "## Experiment\n\n" << request.experimentDescription << "\n\n";
    }
    if (!request.userPrompt.empty()) {
        userMsg << request.userPrompt << "\n\n";
    }
    userMsg << "Generate the complete .gen model file now.";

    ChatMessage um;
    um.role    = "user";
    um.content = userMsg.str();
    provReq.messages.push_back(um);

    const ProviderResponse provResp = _providerClient->send(provReq);

    AIAssistantResponse response;
    response.diagnostics = "buildModel; provider=" + _providerClient->getProviderName()
                         + "; model=" + _config.modelName
                         + "; inputTokens=" + std::to_string(provResp.inputTokens)
                         + "; outputTokens=" + std::to_string(provResp.outputTokens)
                         + ((_config.dryRun)    ? "; dryRun=ON"    : "")
                         + ((_config.sandboxEnabled) ? "; sandbox=ON" : "");

    if (!provResp.success) {
        response.success = false;
        response.state   = ExecutionState::Failed;
        response.message = "LLM call failed during buildModel: " + provResp.errorMessage;
        _state = ExecutionState::Failed; _lastError = response.message;
        _lastDiagnostics = response.diagnostics;
        _audit(response); return response;
    }

    const std::string genContent = extractGenContent(provResp.content);
    response.generatedModelLanguage = genContent;

    // ------------------------------------------------------------------
    // Dry-run: return the generated .gen preview without loading it.
    // ------------------------------------------------------------------
    if (_config.dryRun) {
        const std::string preview = genContent.size() > 400
                                    ? genContent.substr(0, 400) + "\n..." : genContent;
        response.success = true;
        response.state   = ExecutionState::Finished;
        response.message = "[DRY RUN] .gen file generated — not loaded into simulator.\n\n"
                           "Preview:\n" + preview;
        _state = ExecutionState::Finished;
        _lastDiagnostics = response.diagnostics;
        _audit(response); return response;
    }

    // ------------------------------------------------------------------
    // 2. Write .gen content to a temp file and load it.
    // ------------------------------------------------------------------
    // mkstemp doesn't support a suffix; create a temp file and rename it
    // to add the .gen extension so the ModelManager recognises it.
    char basePath[] = "/tmp/genesys-ai-model-XXXXXX";
    int fd = ::mkstemp(basePath);
    if (fd == -1) {
        response.success = false;
        response.state   = ExecutionState::Failed;
        response.message = "Failed to create temporary .gen file for model loading.";
        _state = ExecutionState::Failed; _lastError = response.message;
        _audit(response); return response;
    }
    ::fchmod(fd, 0600);

    const ssize_t written = ::write(fd, genContent.c_str(), genContent.size());
    ::close(fd);
    if (written < 0 || static_cast<std::size_t>(written) != genContent.size()) {
        ::unlink(basePath);
        response.success = false;
        response.state   = ExecutionState::Failed;
        response.message = "Failed to write generated .gen content to temp file.";
        _state = ExecutionState::Failed; _lastError = response.message;
        _audit(response); return response;
    }

    std::string genPath = std::string(basePath) + ".gen";
    if (::rename(basePath, genPath.c_str()) != 0) {
        genPath = basePath;
    }

    // ------------------------------------------------------------------
    // 3. Load model via SimulatorFacade.
    // ------------------------------------------------------------------
    Model* loadedModel = _simulatorFacade->loadModel(genPath);
    ::unlink(genPath.c_str());

    if (loadedModel == nullptr) {
        response.success = false;
        response.state   = ExecutionState::Failed;
        response.message = "SimulatorFacade::loadModel() failed. "
                           "The generated .gen file may contain syntax errors. "
                           "Check generatedModelLanguage in the response for details.";
        _state = ExecutionState::Failed; _lastError = response.message;
        _lastDiagnostics = response.diagnostics;
        _audit(response); return response;
    }

    response.success  = true;
    response.state    = ExecutionState::Finished;
    response.message  = "Model loaded successfully from generated .gen file.";
    _state            = ExecutionState::Finished;
    _lastDiagnostics  = response.diagnostics;
    _audit(response); return response;
}

// ---------------------------------------------------------------------------
// Stage 5: configureSimulation
// ---------------------------------------------------------------------------

AIAssistantResponse AIAssistantDefaultImpl::configureSimulation(const AIAssistantRequest& request) {
    const auto _t0 = std::chrono::steady_clock::now();
    const std::string _pp = request.experimentDescription.size() > 120
                            ? request.experimentDescription.substr(0, 120)
                            : request.experimentDescription;
    auto _audit = [&](const AIAssistantResponse& r) {
        _appendAudit("configureSimulation", r,
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - _t0).count(), _pp);
    };

    if (!_config.allowModelMutation) {
        AIAssistantResponse r;
        r.success = false; r.state = ExecutionState::Failed;
        r.message = "Simulation configuration is not permitted. "
                    "Set AIAssistantConfiguration::allowModelMutation = true.";
        _state = ExecutionState::Failed; _lastError = r.message;
        _audit(r); return r;
    }
    if (_simulatorFacade == nullptr) {
        AIAssistantResponse r;
        r.success = false; r.state = ExecutionState::Failed;
        r.message = "No SimulatorFacade attached. "
                    "Call setSimulatorFacade() before configureSimulation().";
        _state = ExecutionState::Failed; _lastError = r.message;
        _audit(r); return r;
    }

    _state = ExecutionState::Running;

    // ------------------------------------------------------------------
    // Extract simulation parameters from the experiment description via LLM,
    // then apply them — or just preview them when dry-run is enabled.
    // ------------------------------------------------------------------
    std::string descSource = request.experimentDescription;
    if (descSource.empty() && !request.modelPlan.empty()) {
        descSource = request.modelPlan;
    }

    auto parseTimeUnit = [](const std::string& unitStr) -> Util::TimeUnit {
        if (unitStr == "second") return Util::TimeUnit::second;
        if (unitStr == "minute") return Util::TimeUnit::minute;
        if (unitStr == "hour")   return Util::TimeUnit::hour;
        if (unitStr == "day")    return Util::TimeUnit::day;
        return Util::TimeUnit::minute;
    };

    // Extracted values — populated when the LLM call succeeds.
    int    extractedReps   = 0;
    double extractedRl     = 0.0;
    std::string extractedRlUnit;
    double extractedWu     = 0.0;
    std::string extractedWuUnit;

    if (!descSource.empty() && _config.allowNetworkAccess
            && _providerClient != nullptr && _providerClient->isAvailable()) {

        ProviderRequest provReq;
        provReq.model       = _config.modelName;
        provReq.temperature = 0.0;  // Deterministic for structured extraction.
        provReq.maxTokens   = 256;
        provReq.highReasoningMode = false;

        ChatMessage sm;
        sm.role    = "system";
        sm.content =
            "Extract simulation parameters from the text below. "
            "Return ONLY a JSON object with these fields (omit fields that are not mentioned):\n"
            "{\n"
            "  \"numberOfReplications\": <integer>,\n"
            "  \"replicationLength\": <number>,\n"
            "  \"replicationLengthTimeUnit\": \"second\" | \"minute\" | \"hour\" | \"day\",\n"
            "  \"warmUpPeriod\": <number>,\n"
            "  \"warmUpPeriodTimeUnit\": \"second\" | \"minute\" | \"hour\" | \"day\"\n"
            "}\n"
            "Return only the JSON object — no explanation, no markdown.";
        provReq.messages.push_back(sm);

        ChatMessage um;
        um.role    = "user";
        um.content = descSource;
        provReq.messages.push_back(um);

        const ProviderResponse provResp = _providerClient->send(provReq);

        if (provResp.success && !provResp.content.empty()) {
            const std::string& json = provResp.content;
            extractedReps   = HttpProviderClientBase::_extractJsonInt(json, "numberOfReplications");
            extractedRl     = HttpProviderClientBase::_extractJsonDouble(json, "replicationLength");
            extractedRlUnit = HttpProviderClientBase::_extractJsonString(json, "replicationLengthTimeUnit");
            extractedWu     = HttpProviderClientBase::_extractJsonDouble(json, "warmUpPeriod");
            extractedWuUnit = HttpProviderClientBase::_extractJsonString(json, "warmUpPeriodTimeUnit");
        }
    }

    // ------------------------------------------------------------------
    // Dry-run: show what would be applied without touching the simulator.
    // ------------------------------------------------------------------
    if (_config.dryRun) {
        std::ostringstream msg;
        msg << "[DRY RUN] configureSimulation would apply:\n";
        if (extractedReps > 0)
            msg << "  numberOfReplications = " << extractedReps << "\n";
        if (extractedRl > 0.0)
            msg << "  replicationLength    = " << extractedRl << " " << extractedRlUnit << "\n";
        if (extractedWu > 0.0)
            msg << "  warmUpPeriod         = " << extractedWu << " " << extractedWuUnit << "\n";
        if (extractedReps == 0 && extractedRl == 0.0 && extractedWu == 0.0)
            msg << "  (no parameters extracted from description)\n";

        AIAssistantResponse response;
        response.success     = true;
        response.state       = ExecutionState::Finished;
        response.message     = msg.str();
        response.diagnostics = "configureSimulation dryRun=ON";
        _state               = ExecutionState::Finished;
        _lastDiagnostics     = response.diagnostics;
        _audit(response); return response;
    }

    // ------------------------------------------------------------------
    // Apply extracted values (skip zero/empty ones — .gen defaults remain).
    // ------------------------------------------------------------------
    if (extractedReps > 0) {
        _simulatorFacade->simSetNumberOfReplications(static_cast<unsigned int>(extractedReps));
    }
    if (extractedRl > 0.0) {
        _simulatorFacade->simSetReplicationLength(extractedRl, parseTimeUnit(extractedRlUnit));
    }
    if (extractedWu > 0.0) {
        _simulatorFacade->simSetWarmUpPeriod(extractedWu, parseTimeUnit(extractedWuUnit));
    }

    // Build diagnostic string with current settings.
    std::ostringstream diag;
    diag << "configureSimulation: replications="
         << _simulatorFacade->simGetNumberOfReplications()
         << "; replicationLength=" << _simulatorFacade->simGetReplicationLength()
         << "; warmUpPeriod=" << _simulatorFacade->simGetWarmUpPeriod();
    _lastDiagnostics = diag.str();

    AIAssistantResponse response;
    response.success     = true;
    response.state       = ExecutionState::Finished;
    response.message     = "Simulation configured: "
                         + std::to_string(_simulatorFacade->simGetNumberOfReplications())
                         + " replication(s), length="
                         + std::to_string(_simulatorFacade->simGetReplicationLength())
                         + ", warm-up="
                         + std::to_string(_simulatorFacade->simGetWarmUpPeriod()) + ".";
    response.diagnostics = _lastDiagnostics;
    _state               = ExecutionState::Finished;
    _audit(response); return response;
}

// ---------------------------------------------------------------------------
// Stage 6: runSimulation
// ---------------------------------------------------------------------------

AIAssistantResponse AIAssistantDefaultImpl::runSimulation(const AIAssistantRequest& request) {
    (void)request;
    const auto _t0 = std::chrono::steady_clock::now();
    auto _audit = [&](const AIAssistantResponse& r) {
        _appendAudit("runSimulation", r,
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - _t0).count(), "");
    };

    if (!_config.allowSimulationExecution) {
        AIAssistantResponse r;
        r.success = false; r.state = ExecutionState::Failed;
        r.message = "Simulation execution is not permitted. "
                    "Set AIAssistantConfiguration::allowSimulationExecution = true.";
        _state = ExecutionState::Failed; _lastError = r.message;
        _audit(r); return r;
    }
    if (_simulatorFacade == nullptr) {
        AIAssistantResponse r;
        r.success = false; r.state = ExecutionState::Failed;
        r.message = "No SimulatorFacade attached. "
                    "Call setSimulatorFacade() before runSimulation().";
        _state = ExecutionState::Failed; _lastError = r.message;
        _audit(r); return r;
    }
    if (_simulatorFacade->simIsRunning()) {
        AIAssistantResponse r;
        r.success = false; r.state = ExecutionState::Failed;
        r.message = "A simulation is already running.";
        _state = ExecutionState::Failed; _lastError = r.message;
        _audit(r); return r;
    }

    _state = ExecutionState::Running;

    const unsigned int reps = _simulatorFacade->simGetNumberOfReplications();

    // ------------------------------------------------------------------
    // Dry-run: report what would run without calling simStart().
    // ------------------------------------------------------------------
    if (_config.dryRun) {
        AIAssistantResponse response;
        response.success = true;
        response.state   = ExecutionState::Finished;
        response.message = "[DRY RUN] Simulation would start "
                         + std::to_string(reps) + " replication(s) — not executed.";
        _state = ExecutionState::Finished;
        _lastDiagnostics = "runSimulation dryRun=ON";
        _audit(response); return response;
    }

    // simStart() is synchronous — it runs all replications and returns.
    _simulatorFacade->simStart();

    AIAssistantResponse response;
    response.success  = true;
    response.state    = ExecutionState::Finished;
    response.message  = "Simulation completed: "
                      + std::to_string(reps) + " replication(s) executed.";
    _state            = ExecutionState::Finished;
    _lastDiagnostics  = response.message;
    _audit(response); return response;
}

// ---------------------------------------------------------------------------
// Stage 6: collectResults
// ---------------------------------------------------------------------------

AIAssistantResponse AIAssistantDefaultImpl::collectResults(const AIAssistantRequest& request) {
    (void)request;
    const auto _t0 = std::chrono::steady_clock::now();
    auto _audit = [&](const AIAssistantResponse& r) {
        _appendAudit("collectResults", r,
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - _t0).count(), "");
    };

    if (_simulatorFacade == nullptr) {
        AIAssistantResponse r;
        r.success = false; r.state = ExecutionState::Failed;
        r.message = "No SimulatorFacade attached. "
                    "Call setSimulatorFacade() before collectResults().";
        _state = ExecutionState::Failed; _lastError = r.message;
        _audit(r); return r;
    }

    List<SimulationResponse*>* responses = _simulatorFacade->modelGetResponses();
    const unsigned int count = (responses != nullptr) ? responses->size() : 0u;

    // ------------------------------------------------------------------
    // Dry-run: just report the count of available responses.
    // ------------------------------------------------------------------
    if (_config.dryRun) {
        AIAssistantResponse response;
        response.success = true;
        response.state   = ExecutionState::Finished;
        response.message = "[DRY RUN] " + std::to_string(count)
                         + " response(s) available in current model — not rendered.";
        _state = ExecutionState::Finished;
        _lastDiagnostics = "collectResults dryRun=ON";
        _audit(response); return response;
    }

    std::ostringstream md;
    md << "## Simulation Results\n\n";

    if (count == 0u) {
        md << "*No simulation responses available.*\n";
    } else {
        md << "| Metric | Value |\n";
        md << "|--------|-------|\n";

        SimulationResponse* resp = responses->front();
        while (resp != nullptr) {
            md << "| " << resp->getName()
               << " | " << resp->getValue() << " |\n";
            resp = responses->next();
        }
    }

    AIAssistantResponse response;
    response.success  = true;
    response.state    = ExecutionState::Finished;
    response.message  = md.str();
    _state            = ExecutionState::Finished;
    _lastDiagnostics  = "collectResults: rendered " + std::to_string(count) + " response(s).";
    _audit(response); return response;
}

// ---------------------------------------------------------------------------
// execute — orchestrates the full pipeline
// ---------------------------------------------------------------------------

AIAssistantResponse AIAssistantDefaultImpl::execute(const AIAssistantRequest& request) {
    // Step 1: Analyse the prompt.
    AIAssistantResponse response = analyzePrompt(request);
    if (!response.success && response.state != ExecutionState::NotImplemented) {
        return response;
    }

    // Step 2: Create a model plan (always attempted; feeds buildModel).
    AIAssistantResponse planResponse = createModelPlan(request);
    if (!planResponse.success && planResponse.state != ExecutionState::NotImplemented) {
        return planResponse;
    }

    // Enrich the request with the generated plan for downstream steps.
    AIAssistantRequest enriched = request;
    if (!planResponse.generatedPlan.empty()) {
        enriched.modelPlan = planResponse.generatedPlan;
    }
    response = planResponse;

    // Step 3: Build the model.
    if (request.buildModel) {
        response = buildModel(enriched);
        if (!response.success && response.state != ExecutionState::NotImplemented) {
            return response;
        }
    }

    // Step 4: Configure the simulation.
    if (request.configureSimulation) {
        response = configureSimulation(enriched);
        if (!response.success && response.state != ExecutionState::NotImplemented) {
            return response;
        }
    }

    // Step 5: Run the simulation.
    if (request.runSimulation) {
        response = runSimulation(enriched);
        if (!response.success && response.state != ExecutionState::NotImplemented) {
            return response;
        }
    }

    // Step 6: Collect results.
    if (request.collectResults) {
        response = collectResults(enriched);
    }

    return response;
}

// ---------------------------------------------------------------------------
// Diagnostics
// ---------------------------------------------------------------------------

ExecutionState AIAssistantDefaultImpl::getState() const {
    return _state;
}

std::string AIAssistantDefaultImpl::getLastError() const {
    return _lastError;
}

std::string AIAssistantDefaultImpl::getLastDiagnostics() const {
    return _lastDiagnostics;
}

void AIAssistantDefaultImpl::clearDiagnostics() {
    _lastError.clear();
    _lastDiagnostics.clear();
}

// ---------------------------------------------------------------------------
// Audit log (Stage 8)
// ---------------------------------------------------------------------------

std::vector<AuditEntry> AIAssistantDefaultImpl::getAuditEntries(
        unsigned int maxEntries) const {
    return _auditLog.loadRecentEntries(maxEntries);
}

std::string AIAssistantDefaultImpl::getAuditLogPath() const {
    return _auditLog.logFilePath();
}

unsigned int AIAssistantDefaultImpl::exportAuditLog(const std::string& csvPath) const {
    return _auditLog.exportCsv(csvPath);
}

void AIAssistantDefaultImpl::_appendAudit(const std::string& operation,
                                          const AIAssistantResponse& resp,
                                          long long durationMs,
                                          const std::string& preview) {
    AuditEntry entry;
    entry.timestamp    = std::chrono::system_clock::now();
    entry.operation    = operation;
    entry.provider     = _config.providerName.empty()
                         ? providerToString(_config.provider)
                         : _config.providerName;
    entry.modelName    = _config.modelName;
    entry.promptPreview = preview.size() > 120 ? preview.substr(0, 120) : preview;
    entry.success      = resp.success;
    entry.dryRun       = _config.dryRun;
    entry.durationMs   = durationMs;
    if (!resp.success) {
        entry.errorSummary = resp.message.size() > 200
                             ? resp.message.substr(0, 200) : resp.message;
    }
    _auditLog.append(entry);
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

bool AIAssistantDefaultImpl::_providerRequiresApiKey() const {
    return _config.provider == AIProvider::OpenAI
        || _config.provider == AIProvider::Anthropic
        || _config.provider == AIProvider::CustomHttp;
}

bool AIAssistantDefaultImpl::_providerRequiresBaseUrl() const {
    return _config.provider == AIProvider::CustomHttp;
}

AIAssistantResponse AIAssistantDefaultImpl::_notImplementedResponse(
        const std::string& operationName,
        unsigned int plannedStage) const {
    AIAssistantResponse response;
    response.success     = false;
    response.state       = ExecutionState::NotImplemented;
    response.message     = operationName + " is not yet implemented.";
    response.diagnostics = operationName
                         + " is planned for Stage "
                         + std::to_string(plannedStage)
                         + " of the AI Assistant roadmap. "
                           "See source/tools/ARCHITECTURE_tools.md for details.";
    return response;
}

void AIAssistantDefaultImpl::_updateState() {
    if (_state == ExecutionState::Running) {
        return;
    }
    std::string readyMessage;
    if (checkReady(&readyMessage)) {
        _state = ExecutionState::Ready;
    } else {
        _state = ExecutionState::NotConfigured;
    }
}
