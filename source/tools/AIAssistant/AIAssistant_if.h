/*
 * File:   AIAssistant_if.h
 * Author: Genesys Team
 *
 * Created on 28 de Maio de 2026
 */

#ifndef AIASSISTANT_IF_H
#define AIASSISTANT_IF_H

#include "AIAuditLog.h"

#include <string>
#include <vector>

// Non-owning handles to kernel objects.
// Implementation files that call facade methods must include the full headers.
class SimulatorFacade;
class AIProviderClient_if;

// ---------------------------------------------------------------------------
// Enumerations
// ---------------------------------------------------------------------------

/**
 * @brief Identifies the AI provider backend to be used by the assistant.
 */
enum class AIProvider {
    Undefined,    ///< Not yet configured.
    OpenAI,       ///< OpenAI API (GPT-4o, o3, etc.).
    Anthropic,    ///< Anthropic API (Claude 3/4 family).
    Local,        ///< Local model server; no API key required.
    CustomHttp    ///< Generic HTTP endpoint compatible with OpenAI chat format.
};

/**
 * @brief Lifecycle state of the AI assistant's current operation.
 */
enum class ExecutionState {
    NotConfigured,  ///< Minimum configuration not yet provided.
    Ready,          ///< Configured and able to receive requests.
    Running,        ///< A request is currently being processed.
    Finished,       ///< Last request completed successfully.
    Failed,         ///< Last request ended with an error.
    Cancelled,      ///< Last request was cancelled by the caller.
    NotImplemented  ///< Requested operation is not yet implemented.
};

/**
 * @brief Desired reasoning depth / compute budget hint sent to the provider.
 *
 * @details
 * Providers map this to their own parameter (e.g., reasoning_effort for
 * OpenAI o-series models). When the provider does not support the concept,
 * implementations may ignore this field or translate it to temperature adjustments.
 */
enum class ReasoningEffort {
    Low,    ///< Fast, lighter responses; suitable for simple queries.
    Medium, ///< Balanced cost/quality (default).
    High    ///< Deeper reasoning; higher cost and latency.
};

// ---------------------------------------------------------------------------
// Configuration structures
// ---------------------------------------------------------------------------

/**
 * @brief Full configuration record for the AI assistant.
 *
 * @details
 * Security contract:
 * - @p apiKeyEnvironmentVariable holds only the *name* of an environment
 *   variable, never the secret value itself.
 * - The actual API key must be supplied via AIAssistant_if::setApiKey() or
 *   loaded at runtime via AIAssistant_if::loadApiKeyFromEnvironment().
 * - This struct must not be serialised to disk without first redacting the
 *   providerName field if it could be confused with a secret, and ensuring
 *   no key value has been stored here.
 *
 * Permission flags default to @c false (deny-by-default). Each capability
 * that touches external systems or mutates simulator state must be explicitly
 * enabled before use.
 */
struct AIAssistantConfiguration {
    AIProvider   provider                  = AIProvider::Undefined;
    std::string  providerName;             ///< Human-readable label (e.g. "OpenAI GPT-4o").
    std::string  modelName;                ///< Provider model identifier (e.g. "gpt-4o").
    std::string  baseUrl;                  ///< Required when provider == CustomHttp.
    std::string  apiKeyEnvironmentVariable;///< Name of env-var holding the API key, not the key itself.
    std::string  organizationId;           ///< Optional; passed in request headers when non-empty.
    std::string  projectId;                ///< Optional; passed in request headers when non-empty.
    ReasoningEffort reasoningEffort        = ReasoningEffort::Medium;
    double       temperature               = 0.2;
    unsigned int maxOutputTokens           = 4096;
    unsigned int timeoutSeconds            = 120;
    bool         allowNetworkAccess        = false; ///< Permit outbound HTTP calls to the AI provider.
    bool         allowModelMutation        = false; ///< Permit creation or modification of GenESyS models.
    bool         allowSimulationExecution  = false; ///< Permit starting simulation replications.
    bool         allowFileSystemWrites     = false; ///< Permit writing model/result files to disk.

    /**
     * @brief When @c true, clears the current model before every buildModel() call.
     *
     * @details
     * Sandbox mode provides a clean-room environment: any existing model
     * elements are discarded so the LLM always starts from an empty canvas.
     * This prevents the AI from accidentally inheriting components from a model
     * the user was editing. The cleared state is permanent; no undo is provided.
     */
    bool         sandboxEnabled            = false;

    /**
     * @brief When @c true, workflow methods simulate their actions without
     *        mutating the simulator state or consuming network quota beyond
     *        what is needed for planning.
     *
     * @details
     * - buildModel()          — generates the .gen text via LLM but does NOT load it.
     * - configureSimulation() — extracts parameters via LLM but does NOT apply them.
     * - runSimulation()       — reports what would run without calling simStart().
     * - collectResults()      — reports the count of available responses without rendering.
     * - analyzePrompt() / createModelPlan() — always execute regardless of this flag.
     *
     * Use dry-run to verify the LLM output before committing changes.
     */
    bool         dryRun                    = false;
};

// ---------------------------------------------------------------------------
// Request / response structures
// ---------------------------------------------------------------------------

/**
 * @brief Input to a single AI assistant request.
 *
 * @details
 * Not all fields need to be populated for every request. The assistant
 * interprets the populated subset and ignores empty strings. Boolean flags
 * signal intent; the assistant may reject them if the corresponding
 * AIAssistantConfiguration::allow* flag is not set.
 */
struct AIAssistantRequest {
    std::string  userPrompt;            ///< Free-text prompt from the user.
    std::string  systemContext;         ///< Optional extra context to prepend to the system message.
    std::string  modelDescription;      ///< Description of the system to model (may be empty).
    std::string  experimentDescription; ///< Description of the simulation experiment.
    /**
     * @brief Structured model plan (Markdown) to guide model construction.
     *
     * @details
     * When non-empty, buildModel() uses this plan directly without calling the
     * LLM a second time to produce a new plan. Populated automatically by
     * execute() from the output of createModelPlan(). Callers may also supply
     * a previously generated or manually written plan.
     */
    std::string  modelPlan;
    bool         buildModel            = false; ///< Request model construction via SimulatorFacade.
    bool         configureSimulation   = false; ///< Request ModelSimulation configuration.
    bool         runSimulation         = false; ///< Request simulation execution.
    bool         collectResults        = false; ///< Request result collection and reporting.
};

/**
 * @brief Output produced by a single AI assistant operation.
 */
struct AIAssistantResponse {
    bool          success               = false;
    ExecutionState state                = ExecutionState::NotImplemented;
    std::string   message;              ///< Human-readable outcome summary or error description.
    std::string   generatedPlan;        ///< Intermediate structured plan (Markdown or JSON fragment).
    std::string   generatedModelLanguage; ///< GenESyS model language text, when generated.
    std::string   diagnostics;          ///< Technical detail for debugging; not shown to end users.
};

// ---------------------------------------------------------------------------
// Knowledge base structures
// ---------------------------------------------------------------------------

/**
 * @brief One entry in the GenESyS knowledge base used to inform the AI assistant.
 *
 * @details
 * Knowledge items describe available simulation building blocks so that the
 * assistant can map natural-language descriptions to concrete GenESyS types.
 * Items are populated from:
 * - Static documentation embedded at build time.
 * - Dynamic introspection of connected plugins via SimulatorFacade
 *   (Stage 2, see AIAssistant_if::refreshKnowledgeBaseFromSimulator).
 */
struct GenesysKnowledgeItem {
    std::string name;        ///< Short identifier (e.g. "Queue", "Create", "Entity").
    std::string category;    ///< Broad category: "Component", "DataDefinition", "Plugin", etc.
    std::string typeName;    ///< Exact C++ typename used by the kernel (e.g. "Queue").
    std::string description; ///< One-paragraph description of purpose and behaviour.
    std::string usage;       ///< Typical usage pattern or construction example.
    std::string constraints; ///< Known constraints, limitations, or required connection rules.
};

/**
 * @brief Aggregate summary of the current knowledge base state.
 */
struct GenesysKnowledgeBaseSummary {
    unsigned int numberOfPlugins          = 0;
    unsigned int numberOfComponents       = 0;
    unsigned int numberOfDataDefinitions  = 0;
    std::string  summaryMarkdown;         ///< Full knowledge base rendered as a Markdown table.
};

// ---------------------------------------------------------------------------
// Interface
// ---------------------------------------------------------------------------

/**
 * @brief Abstract interface for the GenESyS AI Assistant tool.
 *
 * @details
 * The AI Assistant translates natural-language prompts into GenESyS simulation
 * models, configures experiments and optionally executes them. This interface
 * defines the full contract; concrete implementations supply the provider
 * integration logic.
 *
 * ## Security model
 * - API keys are accepted only through setApiKey() or loadApiKeyFromEnvironment().
 * - No method returns the raw key value (getApiKey() does not exist by design).
 * - getMaskedApiKey() exposes only the last four characters.
 * - All capability-affecting operations are guarded by explicit permission flags
 *   in AIAssistantConfiguration. The deny-by-default posture means nothing is
 *   sent to external services, no model is mutated, and no simulation is run
 *   unless the caller has explicitly enabled each permission.
 *
 * ## Planned evolution
 * See source/tools/ARCHITECTURE_tools.md for the multi-stage roadmap.
 *
 * ## Thread safety
 * Implementations are not required to be thread-safe in this version.
 */
class AIAssistant_if {
public:
    virtual ~AIAssistant_if() = default;

    // -----------------------------------------------------------------------
    // Configuration
    // -----------------------------------------------------------------------

    /**
     * @brief Replaces the entire configuration record.
     *
     * @details
     * Does not transfer the API key, which must be set separately via
     * setApiKey() or loadApiKeyFromEnvironment().
     */
    virtual void setConfiguration(const AIAssistantConfiguration& configuration) = 0;

    /**
     * @brief Returns a copy of the current configuration record.
     *
     * @details
     * The returned struct never contains the API key value.
     */
    virtual AIAssistantConfiguration getConfiguration() const = 0;

    /** @brief Sets the AI backend provider. */
    virtual void setProvider(AIProvider provider) = 0;
    /** @brief Returns the currently configured AI provider. */
    virtual AIProvider getProvider() const = 0;

    /** @brief Sets a human-readable provider label. */
    virtual void setProviderName(const std::string& providerName) = 0;
    /** @brief Returns the human-readable provider label. */
    virtual std::string getProviderName() const = 0;

    /** @brief Sets the provider model identifier (e.g. "gpt-4o", "claude-opus-4-7"). */
    virtual void setModelName(const std::string& modelName) = 0;
    /** @brief Returns the provider model identifier. */
    virtual std::string getModelName() const = 0;

    /**
     * @brief Sets the base URL; required when provider == CustomHttp.
     *
     * @details
     * The URL should point to a chat-completions-compatible endpoint.
     * Example: "http://localhost:11434/v1" for a local Ollama server.
     */
    virtual void setBaseUrl(const std::string& baseUrl) = 0;
    /** @brief Returns the currently configured base URL. */
    virtual std::string getBaseUrl() const = 0;

    /** @brief Sets the desired reasoning effort hint. */
    virtual void setReasoningEffort(ReasoningEffort effort) = 0;
    /** @brief Returns the currently configured reasoning effort. */
    virtual ReasoningEffort getReasoningEffort() const = 0;

    /**
     * @brief Sets the sampling temperature (0.0 = deterministic, 1.0 = creative).
     *
     * @details
     * Not all providers support this parameter; implementations may ignore it
     * when the provider uses a fixed sampling scheme.
     */
    virtual void setTemperature(double temperature) = 0;
    /** @brief Returns the configured sampling temperature. */
    virtual double getTemperature() const = 0;

    /** @brief Sets the maximum number of output tokens to request. */
    virtual void setMaxOutputTokens(unsigned int maxOutputTokens) = 0;
    /** @brief Returns the maximum output tokens limit. */
    virtual unsigned int getMaxOutputTokens() const = 0;

    /** @brief Sets the HTTP request timeout in seconds. */
    virtual void setTimeoutSeconds(unsigned int timeoutSeconds) = 0;
    /** @brief Returns the HTTP request timeout in seconds. */
    virtual unsigned int getTimeoutSeconds() const = 0;

    // -----------------------------------------------------------------------
    // Security and secrets
    // -----------------------------------------------------------------------

    /**
     * @brief Stores the API key in memory for the duration of the session.
     *
     * @details
     * The key is never written to disk or included in any log output.
     * It is intentionally not retrievable via a getter.
     *
     * Security guidance for production deployments:
     * - Prefer loadApiKeyFromEnvironment() over embedding keys in source code.
     * - For long-running services, consider OS keyring / secret-store integration
     *   (planned for Stage 8; not implemented in this version).
     */
    virtual void setApiKey(const std::string& apiKey) = 0;

    /**
     * @brief Erases the in-memory API key.
     *
     * @details
     * Implementations should overwrite the key buffer before clearing to reduce
     * the window of exposure. Note that standard C++ provides no guarantee that
     * the compiler will not optimise away the overwrite; platform-specific secure
     * zeroing (explicit_bzero, SecureZeroMemory) should be used in
     * security-critical contexts.
     */
    virtual void clearApiKey() = 0;

    /** @brief Returns @c true when a non-empty API key is held in memory. */
    virtual bool hasApiKey() const = 0;

    /**
     * @brief Returns a redacted representation of the current API key.
     *
     * @details
     * Only the last four characters are exposed, preceded by "****".
     * Returns an empty string when no key is set.
     * Example: "****abcd".
     */
    virtual std::string getMaskedApiKey() const = 0;

    /**
     * @brief Stores the name of the environment variable that holds the API key.
     *
     * @details
     * This is the variable *name*, not the value. Call loadApiKeyFromEnvironment()
     * to actually read the variable and load it into memory.
     */
    virtual void setApiKeyEnvironmentVariable(const std::string& variableName) = 0;
    /** @brief Returns the configured environment-variable name. */
    virtual std::string getApiKeyEnvironmentVariable() const = 0;

    /**
     * @brief Reads the API key from the configured environment variable.
     *
     * @return @c true when the variable exists and holds a non-empty value.
     *
     * @details
     * Does nothing and returns @c false when apiKeyEnvironmentVariable is empty
     * or the variable is not defined in the process environment.
     * The loaded value is kept only in memory; it is never written to disk.
     */
    virtual bool loadApiKeyFromEnvironment() = 0;

    // -----------------------------------------------------------------------
    // Integration with GenESyS
    // -----------------------------------------------------------------------

    /**
     * @brief Attaches a non-owning SimulatorFacade pointer.
     *
     * @details
     * The assistant does not take ownership of the facade. The caller is
     * responsible for keeping the facade alive for the duration of any
     * operation that uses it.
     */
    virtual void setSimulatorFacade(SimulatorFacade* simulatorFacade) = 0;
    /** @brief Returns the currently attached SimulatorFacade pointer, or @c nullptr. */
    virtual SimulatorFacade* getSimulatorFacade() const = 0;
    /** @brief Returns @c true when a non-null SimulatorFacade is attached. */
    virtual bool hasSimulatorFacade() const = 0;

    // -----------------------------------------------------------------------
    // Provider client
    // -----------------------------------------------------------------------

    /**
     * @brief Injects an external provider client (non-owning).
     *
     * @details
     * The assistant does not take ownership; the caller is responsible for
     * the lifetime of the injected client. The previously held client pointer
     * (if any) is released without deletion.
     *
     * To let the assistant create and own its own client based on the current
     * AIAssistantConfiguration, use createAndAttachProviderClient() in
     * AIAssistantDefaultImpl instead.
     */
    virtual void setProviderClient(AIProviderClient_if* client) = 0;

    /**
     * @brief Returns the currently attached provider client, or @c nullptr.
     */
    virtual AIProviderClient_if* getProviderClient() const = 0;

    /** @brief Returns @c true when a non-null provider client is attached. */
    virtual bool hasProviderClient() const = 0;

    // -----------------------------------------------------------------------
    // Knowledge base
    // -----------------------------------------------------------------------

    /** @brief Removes all items from the knowledge base. */
    virtual void clearKnowledgeBase() = 0;

    /** @brief Appends one item to the knowledge base. */
    virtual void addKnowledgeItem(const GenesysKnowledgeItem& item) = 0;

    /** @brief Returns a copy of all registered knowledge items. */
    virtual std::vector<GenesysKnowledgeItem> getKnowledgeItems() const = 0;

    /** @brief Returns an aggregate summary of the knowledge base. */
    virtual GenesysKnowledgeBaseSummary getKnowledgeBaseSummary() const = 0;

    /**
     * @brief Renders the knowledge base as a Markdown document.
     *
     * @details
     * Intended to be injected into the system prompt so the AI model knows
     * which GenESyS building blocks are available.
     */
    virtual std::string exportKnowledgeBaseAsMarkdown() const = 0;

    /**
     * @brief Populates the knowledge base by introspecting the attached SimulatorFacade.
     *
     * @param message Optional; receives a human-readable status or error message.
     * @return @c true when introspection succeeded and at least one item was added.
     *
     * @details
     * This method is a planned feature (Stage 2). In the current implementation
     * it is a safe stub that returns @c false with a descriptive message.
     *
     * Planned behaviour (Stage 2):
     * - Query SimulatorFacade::getDataDefinitionPluginTypenames() for registered types.
     * - Enumerate ModelComponent and ModelDataDefinition subtypes.
     * - Build GenesysKnowledgeItem entries with typenames, descriptions, and usage examples.
     *
     * @todo (Stage 2): Implement full plugin introspection via SimulatorFacade.
     */
    virtual bool refreshKnowledgeBaseFromSimulator(std::string* message = nullptr) = 0;

    // -----------------------------------------------------------------------
    // Workflow
    // -----------------------------------------------------------------------

    /**
     * @brief Checks whether the assistant has enough configuration to operate.
     *
     * @param message Optional; receives a description of the first unsatisfied
     *        condition, or a confirmation message when ready.
     * @return @c true when all minimum requirements are met for prompt analysis.
     *
     * @details
     * Checks performed (in order):
     * -# Provider is not @c AIProvider::Undefined.
     * -# Model name is not empty.
     * -# For OpenAI/Anthropic/CustomHttp: an API key is held in memory or an
     *    environment-variable name is configured.
     * -# For CustomHttp: baseUrl is not empty.
     *
     * Note: SimulatorFacade and permission flags are checked separately when
     * model-mutation or simulation-execution operations are requested.
     */
    virtual bool checkReady(std::string* message = nullptr) const = 0;

    /**
     * @brief Parses and summarises a user prompt without calling any external service.
     *
     * @details
     * Stage 3 will implement actual LLM calls. Current stub returns NotImplemented.
     */
    virtual AIAssistantResponse analyzePrompt(const AIAssistantRequest& request) = 0;

    /**
     * @brief Produces a structured plan mapping the prompt to GenESyS components.
     *
     * @details
     * Stage 4 will implement prompt-to-plan translation. Current stub returns NotImplemented.
     */
    virtual AIAssistantResponse createModelPlan(const AIAssistantRequest& request) = 0;

    /**
     * @brief Constructs a GenESyS model via SimulatorFacade using an LLM-generated .gen file.
     *
     * @details
     * Requires @c allowModelMutation == @c true, @c allowNetworkAccess == @c true,
     * a valid SimulatorFacade, and an attached provider client.
     *
     * Behaviour (Stage 5):
     * -# If @p request.modelPlan is non-empty, it is used as the model specification.
     *    Otherwise, the @p request description fields are used directly.
     * -# An LLM call is made requesting a GenESyS model language (.gen) file.
     * -# The generated .gen content is written to a temp file and loaded via
     *    SimulatorFacade::loadModel(). The temp file is deleted immediately after loading.
     * -# @p AIAssistantResponse::generatedModelLanguage holds the generated .gen text.
     */
    virtual AIAssistantResponse buildModel(const AIAssistantRequest& request) = 0;

    /**
     * @brief Overrides ModelSimulation parameters on the current model.
     *
     * @details
     * Requires @c allowModelMutation == @c true and a valid SimulatorFacade.
     *
     * Behaviour (Stage 5):
     * -# Makes a small LLM call to extract structured simulation parameters from
     *    @p request.experimentDescription (replications, replication length,
     *    warm-up period, time units).
     * -# Applies the extracted values via SimulatorFacade::simSet*() methods.
     * -# Parameters already set by a loaded .gen file are overridden only when
     *    the LLM extraction yields non-zero values.
     */
    virtual AIAssistantResponse configureSimulation(const AIAssistantRequest& request) = 0;

    /**
     * @brief Executes the configured simulation synchronously via SimulatorFacade.
     *
     * @details
     * Requires @c allowSimulationExecution == @c true and a valid SimulatorFacade.
     * Calls SimulatorFacade::simStart() (blocking) and returns after all replications
     * complete. @p AIAssistantResponse::message reports the number of replications run.
     */
    virtual AIAssistantResponse runSimulation(const AIAssistantRequest& request) = 0;

    /**
     * @brief Collects and formats simulation results from the current model.
     *
     * @details
     * Iterates SimulatorFacade::modelGetResponses() and renders them as a Markdown
     * table in @p AIAssistantResponse::message.
     */
    virtual AIAssistantResponse collectResults(const AIAssistantRequest& request) = 0;

    /**
     * @brief High-level dispatcher that orchestrates the full pipeline based on request flags.
     *
     * @details
     * Calls analyzePrompt, createModelPlan, buildModel, configureSimulation,
     * runSimulation and collectResults in sequence according to the enabled
     * flags in @p request. Stops at the first failure.
     */
    virtual AIAssistantResponse execute(const AIAssistantRequest& request) = 0;

    // -----------------------------------------------------------------------
    // Diagnostics
    // -----------------------------------------------------------------------

    /** @brief Returns the current lifecycle state. */
    virtual ExecutionState getState() const = 0;

    /** @brief Returns the error message from the most recent failed operation. */
    virtual std::string getLastError() const = 0;

    /**
     * @brief Returns technical diagnostic output from the most recent operation.
     *
     * @details
     * Never contains API key values. Safe to log for debugging.
     */
    virtual std::string getLastDiagnostics() const = 0;

    /** @brief Clears both the last error and diagnostics strings. */
    virtual void clearDiagnostics() = 0;

    // -----------------------------------------------------------------------
    // Audit log
    // -----------------------------------------------------------------------

    /**
     * @brief Returns up to @p maxEntries most-recent audit entries (oldest first).
     *
     * @details
     * Reads from the persistent audit log file. Returns an empty vector when
     * the file does not yet exist or cannot be read.
     */
    virtual std::vector<AuditEntry> getAuditEntries(unsigned int maxEntries = 200) const = 0;

    /** @brief Returns the absolute path of the audit log file. */
    virtual std::string getAuditLogPath() const = 0;

    /**
     * @brief Exports all audit entries to a CSV file.
     * @param csvPath Destination file path.
     * @return Number of data rows written (excluding the header).
     */
    virtual unsigned int exportAuditLog(const std::string& csvPath) const = 0;
};

#endif /* AIASSISTANT_IF_H */
