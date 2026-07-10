#ifndef AICONVERSATIONSERVICE_H
#define AICONVERSATIONSERVICE_H

#include "AIProviderClient_if.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

enum class AIConversationProvider { OpenAI, Anthropic, Local };

struct AIConversationConfiguration {
    AIConversationProvider provider = AIConversationProvider::Local;
    std::string model = "llama3.2";
    std::string baseUrl;
    std::string apiKeyEnvironmentVariable;
    std::string systemInstructions;
    double temperature = 0.2;
    unsigned int maxOutputTokens = 512;
    unsigned int timeoutSeconds = 120;
    bool highReasoningMode = false;
    bool conversationEnabled = true;
    unsigned int maxHistoryMessages = 20;
};

struct AIConversationResponse {
    bool success = false;
    std::string content;
    std::string errorMessage;
    int httpStatusCode = 0;
    unsigned int inputTokens = 0;
    unsigned int outputTokens = 0;
};

class AIConversationService {
public:
    AIConversationService() = default;
    ~AIConversationService() = default;

    void setConfiguration(const AIConversationConfiguration& configuration);
    const AIConversationConfiguration& getConfiguration() const;
    bool initialize(std::string& errorMessage);
    void setProviderClient(std::unique_ptr<AIProviderClient_if> client);
    AIConversationResponse send(const std::string& conversationKey, const std::string& prompt);
    void clearConversation(const std::string& conversationKey);
    void clearAllConversations();
    bool isInitialized() const;

private:
    std::vector<ChatMessage> _buildMessages(const std::string& key, const std::string& prompt) const;
    void _appendHistory(const std::string& key, const ChatMessage& message);

    AIConversationConfiguration _configuration;
    std::unique_ptr<AIProviderClient_if> _client;
    std::map<std::string, std::vector<ChatMessage>> _histories;
};

#endif
