/*
 * File:   LocalProviderClientImpl.cpp
 * Author: Genesys Team
 *
 * Created on 28 de Maio de 2026
 */

#include "LocalProviderClientImpl.h"

#include <sstream>

// ---------------------------------------------------------------------------
// Default local server URL (Ollama)
// ---------------------------------------------------------------------------

static const char* const LOCAL_DEFAULT_URL =
    "http://localhost:11434/v1/chat/completions";

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

LocalProviderClientImpl::LocalProviderClientImpl() {
    _baseUrl = LOCAL_DEFAULT_URL;
}

// ---------------------------------------------------------------------------
// Provider identification
// ---------------------------------------------------------------------------

std::string LocalProviderClientImpl::getProviderName() const {
    return "Local";
}

bool LocalProviderClientImpl::isAvailable() const {
    // No API key required; just need a URL.
    return !_baseUrl.empty();
}

// ---------------------------------------------------------------------------
// Protected hooks
// ---------------------------------------------------------------------------

std::string LocalProviderClientImpl::_authorizationHeader() const {
    // Return auth header only when a key has been set.
    if (_apiKey.empty()) return "";
    return "Authorization: Bearer " + _apiKey;
}

std::string LocalProviderClientImpl::_buildRequestBody(
        const ProviderRequest& request) const {
    std::ostringstream body;
    body << "{";
    body << "\"model\":\"" << _escapeJsonString(request.model) << "\"";
    body << ",\"messages\":" << _buildMessagesArray(request.messages);
    body << ",\"max_tokens\":" << request.maxTokens;
    body << ",\"temperature\":" << request.temperature;
    body << ",\"stream\":false";
    body << "}";
    return body.str();
}

ProviderResponse LocalProviderClientImpl::_parseResponseBody(
        const std::string& rawBody,
        int httpStatus) const {
    // Local servers use the OpenAI response schema.
    ProviderResponse response;
    response.httpStatusCode = httpStatus;

    if (rawBody.empty()) {
        response.errorMessage = "Empty response from local model server.";
        return response;
    }

    // Error check
    if (rawBody.find("\"error\"") != std::string::npos) {
        const auto errPos = rawBody.find("\"error\"");
        const auto errObjStart = rawBody.find('{', errPos + 7);
        std::string errBlock = (errObjStart != std::string::npos)
            ? rawBody.substr(errObjStart)
            : rawBody;
        response.errorMessage = _extractJsonString(errBlock, "message");
        if (response.errorMessage.empty()) {
            response.errorMessage = "Local model server returned an error (HTTP " +
                                     std::to_string(httpStatus) + ").";
        }
        return response;
    }

    // Extract choices[0].message.content (OpenAI format)
    const std::string choicesElem = _extractFirstArrayElement(rawBody, "choices");
    if (!choicesElem.empty()) {
        const std::string messageObj = _extractFirstArrayElement(choicesElem, "message");
        if (!messageObj.empty()) {
            response.content = _extractJsonString(messageObj, "content");
        }
        if (response.content.empty()) {
            response.content = _extractJsonString(choicesElem, "content");
        }
    }

    if (response.content.empty()) {
        response.errorMessage = "Could not extract response content from local server reply.";
        return response;
    }

    // Token usage (optional — not all local servers report it)
    const auto usagePos = rawBody.find("\"usage\"");
    if (usagePos != std::string::npos) {
        const auto usageObjStart = rawBody.find('{', usagePos + 7);
        if (usageObjStart != std::string::npos) {
            const std::string usageBlock = rawBody.substr(usageObjStart);
            response.inputTokens  = static_cast<unsigned int>(
                _extractJsonInt(usageBlock, "prompt_tokens"));
            response.outputTokens = static_cast<unsigned int>(
                _extractJsonInt(usageBlock, "completion_tokens"));
        }
    }

    response.success = true;
    return response;
}
