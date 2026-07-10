#ifndef AISUPPORT_H
#define AISUPPORT_H

#include "kernel/simulator/model/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"
#include "tools/AIAssistant/AIConversationService.h"

class AISupport : public ModelDataDefinition {
public:
    AISupport(Model* model, std::string name = "");
    virtual ~AISupport() override = default;

    static PluginInformation* GetPluginInformation();
    static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
    static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

    AIConversationResponse converse(const std::string& conversationKey, const std::string& prompt);
    void clearConversation(const std::string& conversationKey);
    void clearAllConversations();

    void setProvider(std::string value);
    std::string getProvider() const;
    void setModel(std::string value);
    std::string getModel() const;
    void setBaseUrl(std::string value);
    std::string getBaseUrl() const;
    void setApiKeyEnvironmentVariable(std::string value);
    std::string getApiKeyEnvironmentVariable() const;
    void setSystemInstructions(std::string value);
    std::string getSystemInstructions() const;
    void setTemperature(double value);
    double getTemperature() const;
    void setMaxOutputTokens(unsigned int value);
    unsigned int getMaxOutputTokens() const;
    void setTimeoutSeconds(unsigned int value);
    unsigned int getTimeoutSeconds() const;
    void setHighReasoningMode(bool value);
    bool isHighReasoningMode() const;
    void setConversationEnabled(bool value);
    bool isConversationEnabled() const;
    void setMaxHistoryMessages(unsigned int value);
    unsigned int getMaxHistoryMessages() const;
    void setClearHistoryBetweenReplications(bool value);
    bool isClearHistoryBetweenReplications() const;
    unsigned long getCallCount() const;
    unsigned long getFailureCount() const;

    virtual std::string show() override;

protected:
    virtual bool _loadInstance(PersistenceRecord* fields) override;
    virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
    virtual bool _check(std::string& errorMessage) override;
    virtual void _initBetweenReplications() override;

private:
    void _configurationChanged();
    AIConversationConfiguration _makeConfiguration() const;

    const struct DEFAULT_VALUES {
        std::string provider = "Local";
        std::string model = "llama3.2";
        std::string baseUrl = "";
        std::string apiKeyEnvironmentVariable = "";
        std::string systemInstructions = "You are a decision service inside a GenESyS simulation. Return only valid JSON.";
        double temperature = 0.2;
        unsigned int maxOutputTokens = 512;
        unsigned int timeoutSeconds = 120;
        bool highReasoningMode = false;
        bool conversationEnabled = true;
        unsigned int maxHistoryMessages = 20;
        bool clearHistoryBetweenReplications = true;
    } DEFAULT;

    std::string _provider = DEFAULT.provider;
    std::string _model = DEFAULT.model;
    std::string _baseUrl = DEFAULT.baseUrl;
    std::string _apiKeyEnvironmentVariable = DEFAULT.apiKeyEnvironmentVariable;
    std::string _systemInstructions = DEFAULT.systemInstructions;
    double _temperature = DEFAULT.temperature;
    unsigned int _maxOutputTokens = DEFAULT.maxOutputTokens;
    unsigned int _timeoutSeconds = DEFAULT.timeoutSeconds;
    bool _highReasoningMode = DEFAULT.highReasoningMode;
    bool _conversationEnabled = DEFAULT.conversationEnabled;
    unsigned int _maxHistoryMessages = DEFAULT.maxHistoryMessages;
    bool _clearHistoryBetweenReplications = DEFAULT.clearHistoryBetweenReplications;
    unsigned long _callCount = 0;
    unsigned long _failureCount = 0;
    AIConversationService _service;
};

#endif
