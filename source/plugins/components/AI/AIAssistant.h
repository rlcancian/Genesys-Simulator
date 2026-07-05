#ifndef AIASSISTANTCOMPONENT_H
#define AIASSISTANTCOMPONENT_H

#include "kernel/simulator/model/ModelComponent.h"
#include "plugins/data/AI/AISupport.h"

#include <map>

class AIAssistant : public ModelComponent {
public:
    AIAssistant(Model* model, std::string name = "");
    virtual ~AIAssistant() override = default;

    static PluginInformation* GetPluginInformation();
    static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
    static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

    void setAISupport(AISupport* support);
    AISupport* getAISupport() const;
    void setAISupportName(std::string value);
    std::string getAISupportName() const;
    void setPromptTemplate(std::string value);
    std::string getPromptTemplate() const;
    void setConversationScope(std::string value);
    std::string getConversationScope() const;
    void setConversationKeyExpression(std::string value);
    std::string getConversationKeyExpression() const;
    void setActionMappings(std::string value);
    std::string getActionMappings() const;
    void setDelayExpression(std::string value);
    std::string getDelayExpression() const;
    void setDefaultOutputPort(unsigned int value);
    unsigned int getDefaultOutputPort() const;
    void setErrorOutputPort(unsigned int value);
    unsigned int getErrorOutputPort() const;
    void setAllowDirectOutputPort(bool value);
    bool isAllowDirectOutputPort() const;
    void setMinimumConfidence(double value);
    double getMinimumConfidence() const;
    void setSaveConfidenceAttribute(std::string value);
    std::string getSaveConfidenceAttribute() const;
    void setSaveSuccessAttribute(std::string value);
    std::string getSaveSuccessAttribute() const;
    std::string getLastPrompt() const;
    std::string getLastResponse() const;
    std::string getLastError() const;
    bool renderPrompt(std::string& result, std::string& error);

    virtual std::string show() override;

protected:
    virtual bool _loadInstance(PersistenceRecord* fields) override;
    virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
    virtual bool _check(std::string& errorMessage) override;
    virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;

private:
    struct ParsedResponse {
        bool valid = false;
        std::string action;
        int outputPort = -1;
        double confidence = 1.0;
        std::string explanation;
        std::map<std::string, double> values;
        std::string error;
    };

    AISupport* _resolveSupport();
    bool _renderPrompt(std::string& result, std::string& error);
    std::string _conversationKey(Entity* entity, std::string& error);
    ParsedResponse _parseResponse(const std::string& content) const;
    std::map<std::string, unsigned int> _parseActionMappings() const;
    unsigned int _selectPort(const ParsedResponse& response, bool& success) const;
    void _forward(Entity* entity, unsigned int port, double delay);
    void _fail(Entity* entity, const std::string& message);

    const struct DEFAULT_VALUES {
        std::string aiSupportName = "";
        std::string promptTemplate = "Decide the route for this entity. Return only JSON with action, confidence, explanation, and numeric values fields.";
        std::string conversationScope = "PerEntity";
        std::string conversationKeyExpression = "Entity.Id";
        std::string actionMappings = "route:0";
        std::string delayExpression = "0";
        unsigned int defaultOutputPort = 0;
        unsigned int errorOutputPort = 0;
        bool allowDirectOutputPort = false;
        double minimumConfidence = 0.0;
        std::string saveConfidenceAttribute = "";
        std::string saveSuccessAttribute = "";
    } DEFAULT;

    AISupport* _aiSupport = nullptr;
    std::string _aiSupportName = DEFAULT.aiSupportName;
    std::string _promptTemplate = DEFAULT.promptTemplate;
    std::string _conversationScope = DEFAULT.conversationScope;
    std::string _conversationKeyExpression = DEFAULT.conversationKeyExpression;
    std::string _actionMappings = DEFAULT.actionMappings;
    std::string _delayExpression = DEFAULT.delayExpression;
    unsigned int _defaultOutputPort = DEFAULT.defaultOutputPort;
    unsigned int _errorOutputPort = DEFAULT.errorOutputPort;
    bool _allowDirectOutputPort = DEFAULT.allowDirectOutputPort;
    double _minimumConfidence = DEFAULT.minimumConfidence;
    std::string _saveConfidenceAttribute = DEFAULT.saveConfidenceAttribute;
    std::string _saveSuccessAttribute = DEFAULT.saveSuccessAttribute;
    std::string _lastPrompt;
    std::string _lastResponse;
    std::string _lastError;
};

#endif
