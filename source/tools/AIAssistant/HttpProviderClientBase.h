/*
 * File:   HttpProviderClientBase.h
 * Author: Genesys Team
 *
 * Created on 28 de Maio de 2026
 */

#ifndef HTTPPROVIDERCLIENTBASE_H
#define HTTPPROVIDERCLIENTBASE_H

#include "AIProviderClient_if.h"

#include <string>
#include <vector>

/**
 * @brief Abstract base class for HTTP-based AI provider clients.
 *
 * @details
 * Implements the common parts of AIProviderClient_if that are shared across
 * all cloud and local provider backends:
 * - In-memory API key management with the same security contract as AIAssistant_if.
 * - Base URL and timeout configuration.
 * - A synchronous send() that builds a curl command, writes the API key and
 *   request body to temporary files, invokes curl via popen(), and returns the
 *   raw response body to the subclass for parsing.
 *
 * ## Transport design
 * The HTTP request is executed by shelling out to the system `curl` binary.
 * This avoids any C++ HTTP library dependency while remaining portable.
 * Security considerations:
 * - The API key is written to a temporary curl config file created with
 *   mkstemp() and restricted to mode 0600. The key is never passed on the
 *   command line, keeping it out of /proc/<pid>/cmdline and shell history.
 * - Both the body temp file and the config temp file are unlinked immediately
 *   after the curl invocation begins (unlink-before-pclose pattern).
 *
 * ## Subclass responsibilities
 * Concrete implementations must override:
 * - _buildRequestBody() — serialise ProviderRequest to the provider's JSON format.
 * - _parseResponseBody() — deserialise the raw HTTP body to ProviderResponse.
 * - _authorizationHeader() — return the full auth header line (e.g. "Authorization: Bearer <key>").
 * - _extraRequestHeaders() — return additional provider-specific headers (default: none).
 *
 * ## curl dependency
 * curl must be installed and on PATH at runtime. If curl is unavailable, send()
 * returns a ProviderResponse with success=false and a descriptive error message.
 */
class HttpProviderClientBase : public AIProviderClient_if {
public:
    HttpProviderClientBase();
    virtual ~HttpProviderClientBase();

    // Not copyable: API key in member string, non-owning semantics.
    HttpProviderClientBase(const HttpProviderClientBase&) = delete;
    HttpProviderClientBase& operator=(const HttpProviderClientBase&) = delete;

    // -------------------------------------------------------------------------
    // AIProviderClient_if — common implementations
    // -------------------------------------------------------------------------

    bool isAvailable() const override;

    void setBaseUrl(const std::string& url) override;
    std::string getBaseUrl() const override;

    void setTimeoutSeconds(unsigned int seconds) override;
    unsigned int getTimeoutSeconds() const override;

    void setApiKey(const std::string& apiKey) override;
    void clearApiKey() override;
    bool hasApiKey() const override;

    /**
     * @brief Executes the HTTP request via curl and returns the provider response.
     *
     * @details
     * Steps:
     * -# Calls _buildRequestBody() to serialise the request.
     * -# Writes the body JSON to a temp file.
     * -# Writes a curl config file with URL, headers (including auth), data
     *    reference, and timeout.
     * -# Invokes curl via popen(), capturing stdout.
     * -# Unlinks both temp files.
     * -# Calls _parseResponseBody() on the captured output.
     */
    ProviderResponse send(const ProviderRequest& request) override;

protected:
    // -------------------------------------------------------------------------
    // Subclass hooks (pure virtual)
    // -------------------------------------------------------------------------

    /** @brief Serialises @p request to the provider's JSON wire format. */
    virtual std::string _buildRequestBody(const ProviderRequest& request) const = 0;

    /**
     * @brief Deserialises @p rawBody (the full curl stdout) into a ProviderResponse.
     *
     * @param rawBody   Complete HTTP response body as received from curl.
     * @param httpStatus HTTP status code captured from curl exit / response line,
     *                   or 0 when unavailable.
     */
    virtual ProviderResponse _parseResponseBody(const std::string& rawBody,
                                                int httpStatus) const = 0;

    /**
     * @brief Returns the Authorization header line without the "header = " prefix.
     *
     * @details
     * Example return values:
     * - "Authorization: Bearer sk-..."  (OpenAI, LocalProvider)
     * - "x-api-key: sk-ant-..."         (Anthropic)
     *
     * Called only from within send(); the value is written to a 0600 temp file
     * and never logged.
     */
    virtual std::string _authorizationHeader() const = 0;

    /**
     * @brief Returns additional HTTP headers to send with every request.
     *
     * @details
     * Default implementation returns an empty vector.
     * Anthropic overrides to add "anthropic-version: 2023-06-01".
     *
     * Each element must be a complete header line, e.g.
     * "anthropic-version: 2023-06-01".
     */
    virtual std::vector<std::string> _extraRequestHeaders() const;

public:
    // -------------------------------------------------------------------------
    // JSON utilities — public static; usable by any caller that needs
    // lightweight JSON field extraction without a full parser dependency.
    // -------------------------------------------------------------------------

    /** @brief Escapes a plain string for safe embedding inside a JSON string value. */
    static std::string _escapeJsonString(const std::string& s);

    /**
     * @brief Extracts the value of a top-level JSON string field by key.
     * Returns an empty string when not found or value is not a quoted string.
     */
    static std::string _extractJsonString(const std::string& json,
                                          const std::string& key);

    /**
     * @brief Extracts the value of a top-level JSON integer field by key.
     * Returns 0 when not found.
     */
    static int _extractJsonInt(const std::string& json, const std::string& key);

    /**
     * @brief Extracts the value of a top-level JSON number field (int or float) by key.
     * Returns 0.0 when not found.
     */
    static double _extractJsonDouble(const std::string& json, const std::string& key);

    /** @brief Serialises a vector of ChatMessage objects to a JSON array string. */
    static std::string _buildMessagesArray(const std::vector<ChatMessage>& messages);

    /**
     * @brief Extracts the raw substring of the first element of a named JSON array.
     * Returns an empty string when not found.
     */
    static std::string _extractFirstArrayElement(const std::string& json,
                                                  const std::string& arrayKey);

protected:
    std::string  _baseUrl;
    unsigned int _timeoutSeconds = 30;

    /**
     * @brief In-memory API key.
     *
     * Security contract (matches AIAssistant_if):
     * - Never written to disk.
     * - Never included in log or trace output.
     * - Not accessible via a public getter.
     * - Overwritten (best-effort) before clearing in clearApiKey().
     */
    std::string _apiKey;
};

#endif /* HTTPPROVIDERCLIENTBASE_H */
