/*
 * File:   AIProviderClient_if.h
 * Author: Genesys Team
 *
 * Created on 28 de Maio de 2026
 */

#ifndef AIPROVIDERCLIENT_IF_H
#define AIPROVIDERCLIENT_IF_H

#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// LLM chat structures
// ---------------------------------------------------------------------------

/**
 * @brief One message in a multi-turn conversation with an LLM.
 *
 * @details
 * Role conventions follow the OpenAI and Anthropic chat-completion APIs:
 * - "system"    — instructions/persona injected before the conversation.
 * - "user"      — message from the human turn.
 * - "assistant" — previous model replies (for multi-turn context).
 */
struct ChatMessage {
    std::string role;     ///< "system", "user", or "assistant".
    std::string content;  ///< Text content of the message.
};

/**
 * @brief Input to a single LLM chat-completion call.
 */
struct ProviderRequest {
    std::string              model;                        ///< Provider model identifier.
    std::vector<ChatMessage> messages;                     ///< Conversation turns.
    double                   temperature       = 0.2;      ///< Sampling temperature [0,2].
    unsigned int             maxTokens         = 4096;     ///< Maximum output tokens.
    bool                     highReasoningMode = false;    ///< Request extended reasoning (o-series / Claude thinking).
};

/**
 * @brief Output from a single LLM chat-completion call.
 */
struct ProviderResponse {
    bool         success        = false;  ///< @c true when the provider returned a valid completion.
    std::string  content;                 ///< Model reply text. Empty on failure.
    std::string  errorMessage;            ///< Provider or transport error description.
    int          httpStatusCode = 0;      ///< Raw HTTP status (0 when transport not used).
    unsigned int inputTokens    = 0;      ///< Prompt tokens consumed (0 when unreported).
    unsigned int outputTokens   = 0;      ///< Completion tokens produced (0 when unreported).
};

// ---------------------------------------------------------------------------
// Interface
// ---------------------------------------------------------------------------

/**
 * @brief Abstract interface for AI provider HTTP clients.
 *
 * @details
 * A provider client wraps the HTTP call to one AI backend (OpenAI, Anthropic,
 * a local Ollama server, etc.) and converts GenESyS-internal request/response
 * objects to and from the provider's wire format.
 *
 * ## Security contract
 * - The API key is stored in-memory only; no getter exposes it.
 * - clearApiKey() overwrites the buffer before releasing it.
 * - Implementations must not write the key to logs or trace output.
 *
 * ## Ownership
 * Provider client objects do not own the assistants that use them.
 * Callers that inject clients into AIAssistant_if are responsible for
 * keeping the client alive for the duration of any ongoing request.
 */
class AIProviderClient_if {
public:
    virtual ~AIProviderClient_if() = default;

    // -----------------------------------------------------------------------
    // Provider identification
    // -----------------------------------------------------------------------

    /** @brief Returns a human-readable provider name (e.g. "OpenAI", "Anthropic"). */
    virtual std::string getProviderName() const = 0;

    /**
     * @brief Returns @c true when this client can attempt to send requests.
     *
     * @details
     * A client is considered available when it holds an API key (if required
     * by the provider) and a reachable base URL.
     */
    virtual bool isAvailable() const = 0;

    // -----------------------------------------------------------------------
    // Configuration
    // -----------------------------------------------------------------------

    /** @brief Sets the base URL for the provider API endpoint. */
    virtual void setBaseUrl(const std::string& url) = 0;
    /** @brief Returns the currently configured base URL. */
    virtual std::string getBaseUrl() const = 0;

    /** @brief Sets the HTTP request timeout in seconds. */
    virtual void setTimeoutSeconds(unsigned int seconds) = 0;
    /** @brief Returns the configured HTTP request timeout. */
    virtual unsigned int getTimeoutSeconds() const = 0;

    // -----------------------------------------------------------------------
    // Secrets — same security contract as AIAssistant_if
    // -----------------------------------------------------------------------

    /**
     * @brief Stores the API key in memory.
     *
     * @details
     * Never write the key to disk or logs. Prefer loading from an environment
     * variable via the owning AIAssistant_if rather than embedding keys in source.
     */
    virtual void setApiKey(const std::string& apiKey) = 0;

    /**
     * @brief Overwrites (best-effort) and clears the in-memory API key.
     */
    virtual void clearApiKey() = 0;

    /** @brief Returns @c true when a non-empty API key is held. */
    virtual bool hasApiKey() const = 0;

    // -----------------------------------------------------------------------
    // Request
    // -----------------------------------------------------------------------

    /**
     * @brief Sends a chat-completion request and returns the response.
     *
     * @details
     * This call is synchronous and may block for up to timeoutSeconds.
     * On transport failure @p ProviderResponse::success is @c false and
     * @p ProviderResponse::errorMessage is populated.
     */
    virtual ProviderResponse send(const ProviderRequest& request) = 0;
};

#endif /* AIPROVIDERCLIENT_IF_H */
