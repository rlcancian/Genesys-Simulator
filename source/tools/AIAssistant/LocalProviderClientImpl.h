/*
 * File:   LocalProviderClientImpl.h
 * Author: Genesys Team
 *
 * Created on 28 de Maio de 2026
 */

#ifndef LOCALPROVIDERCLIENTIMPL_H
#define LOCALPROVIDERCLIENTIMPL_H

#include "HttpProviderClientBase.h"

/**
 * @brief AI provider client for local model servers (Ollama, LM Studio, etc.).
 *
 * @details
 * Targets any OpenAI-compatible chat completions endpoint reachable locally.
 * Default base URL: `http://localhost:11434/v1/chat/completions` (Ollama default).
 *
 * Differences from OpenAIProviderClientImpl:
 * - No API key is required; isAvailable() returns true when the base URL is set.
 * - When a non-empty API key IS set (some local servers require a token), it is
 *   sent as `Authorization: Bearer <key>`. When no key is set, the Authorization
 *   header is omitted.
 * - Extended reasoning (highReasoningMode) adds `"stream": false` only;
 *   no reasoning_effort parameter (not supported by most local servers).
 * - The JSON request/response format otherwise mirrors OpenAI chat completions.
 *
 * ## Tested server compatibility
 * - Ollama (≥0.1.29): set the base URL to `http://localhost:11434/v1/chat/completions`.
 * - LM Studio local server: set base URL to `http://localhost:1234/v1/chat/completions`.
 * - llama.cpp server: set base URL to `http://localhost:8080/v1/chat/completions`.
 */
class LocalProviderClientImpl : public HttpProviderClientBase {
public:
    /**
     * @brief Constructs the client targeting the default Ollama endpoint.
     */
    LocalProviderClientImpl();
    virtual ~LocalProviderClientImpl() = default;

    // -------------------------------------------------------------------------
    // Provider identification
    // -------------------------------------------------------------------------

    std::string getProviderName() const override;

    /**
     * @brief Returns @c true when a base URL is configured.
     *
     * @details
     * Unlike cloud providers, a local server does not require an API key.
     */
    bool isAvailable() const override;

protected:
    std::string _buildRequestBody(const ProviderRequest& request) const override;
    ProviderResponse _parseResponseBody(const std::string& rawBody,
                                        int httpStatus) const override;

    /**
     * @brief Returns the Authorization header when an API key is set, empty otherwise.
     */
    std::string _authorizationHeader() const override;
};

#endif /* LOCALPROVIDERCLIENTIMPL_H */
