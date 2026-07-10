#include "AIConversationService.h"

#include "AnthropicProviderClientImpl.h"
#include "LocalProviderClientImpl.h"
#include "OpenAIProviderClientImpl.h"

#include <cstdlib>

void AIConversationService::setConfiguration(const AIConversationConfiguration& configuration) {
    _configuration = configuration;
    _client.reset();
}

const AIConversationConfiguration& AIConversationService::getConfiguration() const {
    return _configuration;
}

bool AIConversationService::initialize(std::string& errorMessage) {
    switch (_configuration.provider) {
    case AIConversationProvider::OpenAI:
        _client = std::make_unique<OpenAIProviderClientImpl>();
        break;
    case AIConversationProvider::Anthropic:
        _client = std::make_unique<AnthropicProviderClientImpl>();
        break;
    case AIConversationProvider::Local:
        _client = std::make_unique<LocalProviderClientImpl>();
        break;
    }
    if (!_configuration.baseUrl.empty()) {
        _client->setBaseUrl(_configuration.baseUrl);
    }
    _client->setTimeoutSeconds(_configuration.timeoutSeconds);
    if (!_configuration.apiKeyEnvironmentVariable.empty()) {
        const char* value = std::getenv(_configuration.apiKeyEnvironmentVariable.c_str());
        if (value != nullptr && *value != '\0') {
            _client->setApiKey(value);
        }
    }
    if (!_client->isAvailable()) {
        errorMessage = "AI provider is unavailable. Check the endpoint and API-key environment variable.";
        _client.reset();
        return false;
    }
    errorMessage.clear();
    return true;
}

AIConversationResponse AIConversationService::send(const std::string& conversationKey,
                                                    const std::string& prompt) {
    AIConversationResponse result;
    if (_client == nullptr) {
        if (!initialize(result.errorMessage)) return result;
    }

    ProviderRequest request;
    request.model = _configuration.model;
    request.messages = _buildMessages(conversationKey, prompt);
    request.temperature = _configuration.temperature;
    request.maxTokens = _configuration.maxOutputTokens;
    request.highReasoningMode = _configuration.highReasoningMode;

    ProviderResponse providerResponse = _client->send(request);
    result.success = providerResponse.success;
    result.content = providerResponse.content;
    result.errorMessage = providerResponse.errorMessage;
    result.httpStatusCode = providerResponse.httpStatusCode;
    result.inputTokens = providerResponse.inputTokens;
    result.outputTokens = providerResponse.outputTokens;
    if (result.success && _configuration.conversationEnabled) {
        _appendHistory(conversationKey, {"user", prompt});
        _appendHistory(conversationKey, {"assistant", result.content});
    }
    return result;
}

void AIConversationService::setProviderClient(std::unique_ptr<AIProviderClient_if> client) {
    _client = std::move(client);
}

void AIConversationService::clearConversation(const std::string& conversationKey) {
    _histories.erase(conversationKey);
}

void AIConversationService::clearAllConversations() {
    _histories.clear();
}

bool AIConversationService::isInitialized() const { return _client != nullptr; }

std::vector<ChatMessage> AIConversationService::_buildMessages(const std::string& key,
                                                               const std::string& prompt) const {
    std::vector<ChatMessage> messages;
    if (!_configuration.systemInstructions.empty()) {
        messages.push_back({"system", _configuration.systemInstructions});
    }
    if (_configuration.conversationEnabled) {
        auto found = _histories.find(key);
        if (found != _histories.end()) messages.insert(messages.end(), found->second.begin(), found->second.end());
    }
    messages.push_back({"user", prompt});
    return messages;
}

void AIConversationService::_appendHistory(const std::string& key, const ChatMessage& message) {
    auto& history = _histories[key];
    history.push_back(message);
    const std::size_t limit = _configuration.maxHistoryMessages;
    if (limit > 0 && history.size() > limit) {
        history.erase(history.begin(), history.begin() + static_cast<std::ptrdiff_t>(history.size() - limit));
    }
}
