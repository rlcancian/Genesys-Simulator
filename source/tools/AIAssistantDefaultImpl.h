/*
 * File:   AIAssistantDefaultImpl.h
 * Author: Genesys Team
 *
 * Created on 28 de Maio de 2026
 */

#ifndef AIASSISTANTDEFAULTIMPL_H
#define AIASSISTANTDEFAULTIMPL_H

#include "AIAssistant_if.h"
#include "AIProviderClient_if.h"

#include <memory>
#include <string>
#include <vector>

/**
 * @brief Initial concrete scaffold for the GenESyS AI Assistant.
 *
 * @details
 * This class implements AIAssistant_if and provides:
 * - Full configuration management including secure API key handling.
 * - A knowledge base backed by a simple in-memory vector.
 * - A checkReady() implementation that validates minimum configuration.
 * - Safe stubs for all workflow methods (analyzePrompt, buildModel, etc.)
 *   that return ExecutionState::NotImplemented until the corresponding
 *   stage is implemented.
 *
 * Current status:
 * - Stage 1: configuration, security, knowledge base, diagnostics — complete.
 * - Stage 2 and above: intentionally deferred; see @todo annotations.
 *
 * ## Security model
 * - The API key is held in a private @c std::string member, never written to
 *   disk or included in trace output.
 * - No getter exposes the key value; only getMaskedApiKey() is available.
 * - Copy construction and copy assignment are deleted to prevent accidental
 *   key propagation through object copying.
 *
 * ## Ownership
 * - The SimulatorFacade pointer is non-owning; this class does not delete it.
 */
class AIAssistantDefaultImpl : public AIAssistant_if {
public:
    AIAssistantDefaultImpl();
    virtual ~AIAssistantDefaultImpl();

    // Not copyable: would propagate the in-memory API key and the non-owning
    // SimulatorFacade pointer without a clear ownership contract.
    AIAssistantDefaultImpl(const AIAssistantDefaultImpl&) = delete;
    AIAssistantDefaultImpl& operator=(const AIAssistantDefaultImpl&) = delete;

public:
    // -----------------------------------------------------------------------
    // Configuration
    // -----------------------------------------------------------------------
    virtual void setConfiguration(const AIAssistantConfiguration& configuration) override;
    virtual AIAssistantConfiguration getConfiguration() const override;

    virtual void setProvider(AIProvider provider) override;
    virtual AIProvider getProvider() const override;

    virtual void setProviderName(const std::string& providerName) override;
    virtual std::string getProviderName() const override;

    virtual void setModelName(const std::string& modelName) override;
    virtual std::string getModelName() const override;

    virtual void setBaseUrl(const std::string& baseUrl) override;
    virtual std::string getBaseUrl() const override;

    virtual void setReasoningEffort(ReasoningEffort effort) override;
    virtual ReasoningEffort getReasoningEffort() const override;

    virtual void setTemperature(double temperature) override;
    virtual double getTemperature() const override;

    virtual void setMaxOutputTokens(unsigned int maxOutputTokens) override;
    virtual unsigned int getMaxOutputTokens() const override;

    virtual void setTimeoutSeconds(unsigned int timeoutSeconds) override;
    virtual unsigned int getTimeoutSeconds() const override;

    // -----------------------------------------------------------------------
    // Security and secrets
    // -----------------------------------------------------------------------
    virtual void setApiKey(const std::string& apiKey) override;
    virtual void clearApiKey() override;
    virtual bool hasApiKey() const override;
    virtual std::string getMaskedApiKey() const override;

    virtual void setApiKeyEnvironmentVariable(const std::string& variableName) override;
    virtual std::string getApiKeyEnvironmentVariable() const override;
    virtual bool loadApiKeyFromEnvironment() override;

    // -----------------------------------------------------------------------
    // Integration with GenESyS
    // -----------------------------------------------------------------------
    virtual void setSimulatorFacade(SimulatorFacade* simulatorFacade) override;
    virtual SimulatorFacade* getSimulatorFacade() const override;
    virtual bool hasSimulatorFacade() const override;

    // -----------------------------------------------------------------------
    // Provider client
    // -----------------------------------------------------------------------
    virtual void setProviderClient(AIProviderClient_if* client) override;
    virtual AIProviderClient_if* getProviderClient() const override;
    virtual bool hasProviderClient() const override;

    /**
     * @brief Creates an owned provider client from the current configuration and
     *        attaches it, replacing any previously held client pointer.
     *
     * @return @c true when a suitable client was created and attached.
     *
     * @details
     * The assistant takes ownership of the created client and deletes it in the
     * destructor or on the next call to createAndAttachProviderClient().
     * Clients injected via setProviderClient() are never deleted by the assistant.
     *
     * Provider selection is based on AIAssistantConfiguration::provider:
     * - AIProvider::OpenAI     → OpenAIProviderClientImpl
     * - AIProvider::Anthropic  → AnthropicProviderClientImpl
     * - AIProvider::Local      → LocalProviderClientImpl
     * - AIProvider::CustomHttp → LocalProviderClientImpl with custom base URL
     * - AIProvider::Undefined  → returns false, no client created
     */
    bool createAndAttachProviderClient();

    // -----------------------------------------------------------------------
    // Knowledge base
    // -----------------------------------------------------------------------
    virtual void clearKnowledgeBase() override;
    virtual void addKnowledgeItem(const GenesysKnowledgeItem& item) override;
    virtual std::vector<GenesysKnowledgeItem> getKnowledgeItems() const override;
    virtual GenesysKnowledgeBaseSummary getKnowledgeBaseSummary() const override;
    virtual std::string exportKnowledgeBaseAsMarkdown() const override;
    virtual bool refreshKnowledgeBaseFromSimulator(std::string* message = nullptr) override;

    // -----------------------------------------------------------------------
    // Workflow
    // -----------------------------------------------------------------------
    virtual bool checkReady(std::string* message = nullptr) const override;

    virtual AIAssistantResponse analyzePrompt(const AIAssistantRequest& request) override;
    virtual AIAssistantResponse createModelPlan(const AIAssistantRequest& request) override;
    virtual AIAssistantResponse buildModel(const AIAssistantRequest& request) override;
    virtual AIAssistantResponse configureSimulation(const AIAssistantRequest& request) override;
    virtual AIAssistantResponse runSimulation(const AIAssistantRequest& request) override;
    virtual AIAssistantResponse collectResults(const AIAssistantRequest& request) override;
    virtual AIAssistantResponse execute(const AIAssistantRequest& request) override;

    // -----------------------------------------------------------------------
    // Diagnostics
    // -----------------------------------------------------------------------
    virtual ExecutionState getState() const override;
    virtual std::string getLastError() const override;
    virtual std::string getLastDiagnostics() const override;
    virtual void clearDiagnostics() override;

private:
    // -----------------------------------------------------------------------
    // Private helpers
    // -----------------------------------------------------------------------

    /**
     * @brief Returns @c true when the current provider requires an API key.
     */
    bool _providerRequiresApiKey() const;

    /**
     * @brief Returns @c true when the current provider requires a base URL.
     */
    bool _providerRequiresBaseUrl() const;

    /**
     * @brief Creates a stub response with NotImplemented state.
     *
     * @param operationName Identifies the operation for the diagnostic message.
     * @param plannedStage  Stage number in which this operation is planned.
     */
    AIAssistantResponse _notImplementedResponse(const std::string& operationName,
                                                 unsigned int plannedStage) const;

    /**
     * @brief Recomputes _state based on current configuration.
     *
     * Called after any configuration change that could affect readiness.
     */
    void _updateState();

private:
    // -----------------------------------------------------------------------
    // Private data members
    // -----------------------------------------------------------------------

    AIAssistantConfiguration _config;

    /**
     * @brief In-memory API key.
     *
     * Security contract:
     * - Never written to disk.
     * - Never included in trace/log output.
     * - Not accessible via a public getter.
     * - Overwritten (best-effort) before clearing in clearApiKey().
     */
    std::string _apiKey;

    /** @brief Non-owning pointer to the simulator facade; may be @c nullptr. */
    SimulatorFacade* _simulatorFacade = nullptr;

    /**
     * @brief Provider client used for HTTP requests; may be @c nullptr.
     *
     * Ownership semantics:
     * - When set via setProviderClient(): non-owning; never deleted here.
     * - When created via createAndAttachProviderClient(): owned; deleted in
     *   destructor and on subsequent calls to createAndAttachProviderClient().
     */
    AIProviderClient_if* _providerClient    = nullptr;
    bool                 _ownsProviderClient = false;

    /** @brief In-memory knowledge base items. */
    std::vector<GenesysKnowledgeItem> _knowledgeItems;

    /** @brief Current lifecycle state. */
    ExecutionState _state = ExecutionState::NotConfigured;

    /** @brief Error message from the most recent failed operation. */
    std::string _lastError;

    /** @brief Technical diagnostics from the most recent operation. */
    std::string _lastDiagnostics;
};

#endif /* AIASSISTANTDEFAULTIMPL_H */
