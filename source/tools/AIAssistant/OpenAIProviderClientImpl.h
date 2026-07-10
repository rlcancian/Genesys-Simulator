/*
 * File:   OpenAIProviderClientImpl.h
 * Author: Genesys Team
 *
 * Created on 28 de Maio de 2026
 */

#ifndef OPENAIPROVIDERIMPL_H
#define OPENAIPROVIDERIMPL_H

#include "HttpProviderClientBase.h"

/**
 * @brief AI provider client for the OpenAI Chat Completions API.
 *
 * @details
 * Wraps the OpenAI `/v1/chat/completions` endpoint.
 * Default base URL: `https://api.openai.com/v1/chat/completions`
 *
 * Supported features:
 * - Standard chat completions (gpt-4o, gpt-4-turbo, gpt-3.5-turbo, etc.).
 * - Reasoning models via highReasoningMode (o3, o4-mini): temperature is
 *   omitted and reasoning_effort is set to "high" in the request body.
 * - Organization and project headers when set via setOrganizationId() /
 *   setProjectId().
 *
 * ## API key
 * Passed as `Authorization: Bearer <key>`. Stored in the parent's _apiKey
 * member; never written to the command line or logs.
 */
class OpenAIProviderClientImpl : public HttpProviderClientBase {
public:
    /**
     * @brief Constructs the client with the standard OpenAI base URL.
     *
     * To use with Azure OpenAI or a compatible proxy, call setBaseUrl()
     * after construction.
     */
    OpenAIProviderClientImpl();
    virtual ~OpenAIProviderClientImpl() = default;

    // -------------------------------------------------------------------------
    // Provider identification
    // -------------------------------------------------------------------------

    std::string getProviderName() const override;

    /**
     * @brief Returns @c true when an API key is set and a base URL is configured.
     */
    bool isAvailable() const override;

    // -------------------------------------------------------------------------
    // Optional per-request headers
    // -------------------------------------------------------------------------

    /** @brief Sets the OpenAI organization identifier (sent as OpenAI-Organization header). */
    void setOrganizationId(const std::string& orgId);
    /** @brief Returns the configured organization identifier. */
    std::string getOrganizationId() const;

    /** @brief Sets the OpenAI project identifier (sent as OpenAI-Project header). */
    void setProjectId(const std::string& projectId);
    /** @brief Returns the configured project identifier. */
    std::string getProjectId() const;

protected:
    std::string _buildRequestBody(const ProviderRequest& request) const override;
    ProviderResponse _parseResponseBody(const std::string& rawBody,
                                        int httpStatus) const override;
    std::string _authorizationHeader() const override;
    std::vector<std::string> _extraRequestHeaders() const override;

private:
    std::string _organizationId;
    std::string _projectId;
};

#endif /* OPENAIPROVIDERIMPL_H */
