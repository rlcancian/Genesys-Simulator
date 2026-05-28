/*
 * File:   OpenAIProviderClientImpl.cpp
 * Author: Genesys Team
 *
 * Created on 28 de Maio de 2026
 */

#include "OpenAIProviderClientImpl.h"

#include <sstream>

// ---------------------------------------------------------------------------
// Default OpenAI base URL
// ---------------------------------------------------------------------------

static const char* const OPENAI_DEFAULT_URL =
    "https://api.openai.com/v1/chat/completions";

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

OpenAIProviderClientImpl::OpenAIProviderClientImpl() {
    _baseUrl = OPENAI_DEFAULT_URL;
}

// ---------------------------------------------------------------------------
// Provider identification
// ---------------------------------------------------------------------------

std::string OpenAIProviderClientImpl::getProviderName() const {
    return "OpenAI";
}

bool OpenAIProviderClientImpl::isAvailable() const {
    return hasApiKey() && !_baseUrl.empty();
}

// ---------------------------------------------------------------------------
// Optional headers
// ---------------------------------------------------------------------------

void OpenAIProviderClientImpl::setOrganizationId(const std::string& orgId) {
    _organizationId = orgId;
}

std::string OpenAIProviderClientImpl::getOrganizationId() const {
    return _organizationId;
}

void OpenAIProviderClientImpl::setProjectId(const std::string& projectId) {
    _projectId = projectId;
}

std::string OpenAIProviderClientImpl::getProjectId() const {
    return _projectId;
}

// ---------------------------------------------------------------------------
// Protected hooks
// ---------------------------------------------------------------------------

std::string OpenAIProviderClientImpl::_authorizationHeader() const {
    return "Authorization: Bearer " + _apiKey;
}

std::vector<std::string> OpenAIProviderClientImpl::_extraRequestHeaders() const {
    std::vector<std::string> headers;
    if (!_organizationId.empty()) {
        headers.push_back("OpenAI-Organization: " + _organizationId);
    }
    if (!_projectId.empty()) {
        headers.push_back("OpenAI-Project: " + _projectId);
    }
    return headers;
}

std::string OpenAIProviderClientImpl::_buildRequestBody(
        const ProviderRequest& request) const {
    std::ostringstream body;
    body << "{";
    body << "\"model\":\"" << _escapeJsonString(request.model) << "\"";
    body << ",\"messages\":" << _buildMessagesArray(request.messages);
    body << ",\"max_tokens\":" << request.maxTokens;

    if (request.highReasoningMode) {
        // o-series reasoning models: omit temperature, set reasoning_effort.
        body << ",\"reasoning_effort\":\"high\"";
    } else {
        body << ",\"temperature\":" << request.temperature;
    }

    body << "}";
    return body.str();
}

ProviderResponse OpenAIProviderClientImpl::_parseResponseBody(
        const std::string& rawBody,
        int httpStatus) const {
    ProviderResponse response;
    response.httpStatusCode = httpStatus;

    if (rawBody.empty()) {
        response.errorMessage = "Empty response from OpenAI API.";
        return response;
    }

    // Check for error object: {"error": {"message": "..."}}
    const auto errPos = rawBody.find("\"error\"");
    if (errPos != std::string::npos) {
        // Try to extract error.message from the nested object
        const auto errObjStart = rawBody.find('{', errPos + 7);
        std::string errBlock = (errObjStart != std::string::npos)
            ? rawBody.substr(errObjStart)
            : rawBody;
        response.errorMessage = _extractJsonString(errBlock, "message");
        if (response.errorMessage.empty()) {
            response.errorMessage = "OpenAI API returned an error (HTTP " +
                                     std::to_string(httpStatus) + ").";
        }
        return response;
    }

    // Extract choices[0].message.content
    const std::string choicesElem = _extractFirstArrayElement(rawBody, "choices");
    if (!choicesElem.empty()) {
        const std::string messageObj = _extractFirstArrayElement(choicesElem, "message");
        if (!messageObj.empty()) {
            response.content = _extractJsonString(messageObj, "content");
        }
        // Fallback: try direct content field in element (some streaming formats)
        if (response.content.empty()) {
            response.content = _extractJsonString(choicesElem, "content");
        }
    }

    if (response.content.empty()) {
        response.errorMessage = "Could not extract response content from OpenAI reply.";
        return response;
    }

    // Extract token usage from top-level "usage" object
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
