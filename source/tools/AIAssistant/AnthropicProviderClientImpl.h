/*
 * File:   AnthropicProviderClientImpl.h
 * Author: Genesys Team
 *
 * Created on 28 de Maio de 2026
 */

#ifndef ANTHROPICPROVIDERCLIENTIMPL_H
#define ANTHROPICPROVIDERCLIENTIMPL_H

#include "HttpProviderClientBase.h"

/**
 * @brief AI provider client for the Anthropic Messages API.
 *
 * @details
 * Wraps the Anthropic `/v1/messages` endpoint (Claude 3/4 model family).
 * Default base URL: `https://api.anthropic.com/v1/messages`
 *
 * Wire format differences from the OpenAI schema:
 * - Auth header: `x-api-key: <key>` (not `Authorization: Bearer`).
 * - Required extra header: `anthropic-version: 2023-06-01`.
 * - Request body: `max_tokens` (required), `system` is a top-level field
 *   (not a message in the array), `messages` contains only user/assistant turns.
 * - Response: content is in `content[0].text`, tokens in
 *   `usage.input_tokens` / `usage.output_tokens`.
 * - Extended thinking: when `highReasoningMode` is true, a
 *   `thinking.type=enabled` budget block is added to the request and the
 *   text is extracted from the first content block whose `type` is `"text"`.
 *
 * ## API key
 * Sent as `x-api-key: <key>`. Written to the curl config file at mode 0600;
 * never placed on the command line.
 */
class AnthropicProviderClientImpl : public HttpProviderClientBase {
public:
    /**
     * @brief Constructs the client with the standard Anthropic base URL.
     */
    AnthropicProviderClientImpl();
    virtual ~AnthropicProviderClientImpl() = default;

    // -------------------------------------------------------------------------
    // Provider identification
    // -------------------------------------------------------------------------

    std::string getProviderName() const override;

    /** @brief Returns @c true when an API key is set and a base URL is configured. */
    bool isAvailable() const override;

    // -------------------------------------------------------------------------
    // Anthropic-specific configuration
    // -------------------------------------------------------------------------

    /**
     * @brief Sets the Anthropic API version header value.
     *
     * @details
     * Defaults to "2023-06-01" which is the stable version as of mid-2026.
     * Override only when Anthropic releases a newer stable version that
     * changes the wire format.
     */
    void setApiVersion(const std::string& version);
    /** @brief Returns the configured Anthropic API version string. */
    std::string getApiVersion() const;

    /**
     * @brief Sets the extended-thinking budget in tokens (default: 10000).
     *
     * @details
     * Used only when ProviderRequest::highReasoningMode is true.
     * Must be less than ProviderRequest::maxTokens.
     */
    void setThinkingBudgetTokens(unsigned int budget);
    /** @brief Returns the thinking budget in tokens. */
    unsigned int getThinkingBudgetTokens() const;

protected:
    std::string _buildRequestBody(const ProviderRequest& request) const override;
    ProviderResponse _parseResponseBody(const std::string& rawBody,
                                        int httpStatus) const override;
    std::string _authorizationHeader() const override;
    std::vector<std::string> _extraRequestHeaders() const override;

private:
    std::string  _apiVersion           = "2023-06-01";
    unsigned int _thinkingBudgetTokens = 10000;
};

#endif /* ANTHROPICPROVIDERCLIENTIMPL_H */
