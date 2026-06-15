/*
 * File:   AnthropicProviderClientImpl.cpp
 * Author: Genesys Team
 *
 * Created on 28 de Maio de 2026
 */

#include "AnthropicProviderClientImpl.h"

#include <sstream>

// ---------------------------------------------------------------------------
// Default Anthropic base URL
// ---------------------------------------------------------------------------

static const char* const ANTHROPIC_DEFAULT_URL =
    "https://api.anthropic.com/v1/messages";

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

AnthropicProviderClientImpl::AnthropicProviderClientImpl() {
    _baseUrl = ANTHROPIC_DEFAULT_URL;
}

// ---------------------------------------------------------------------------
// Provider identification
// ---------------------------------------------------------------------------

std::string AnthropicProviderClientImpl::getProviderName() const {
    return "Anthropic";
}

bool AnthropicProviderClientImpl::isAvailable() const {
    return hasApiKey() && !_baseUrl.empty();
}

// ---------------------------------------------------------------------------
// Anthropic-specific configuration
// ---------------------------------------------------------------------------

void AnthropicProviderClientImpl::setApiVersion(const std::string& version) {
    _apiVersion = version;
}

std::string AnthropicProviderClientImpl::getApiVersion() const {
    return _apiVersion;
}

void AnthropicProviderClientImpl::setThinkingBudgetTokens(unsigned int budget) {
    _thinkingBudgetTokens = budget;
}

unsigned int AnthropicProviderClientImpl::getThinkingBudgetTokens() const {
    return _thinkingBudgetTokens;
}

// ---------------------------------------------------------------------------
// Protected hooks
// ---------------------------------------------------------------------------

std::string AnthropicProviderClientImpl::_authorizationHeader() const {
    return "x-api-key: " + _apiKey;
}

std::vector<std::string> AnthropicProviderClientImpl::_extraRequestHeaders() const {
    return { "anthropic-version: " + _apiVersion };
}

std::string AnthropicProviderClientImpl::_buildRequestBody(
        const ProviderRequest& request) const {
    // Anthropic separates the system message from the conversation turns.
    // Extract the first system message (if any) into the top-level "system" field.
    std::string systemText;
    std::vector<ChatMessage> conversationMessages;
    for (const ChatMessage& msg : request.messages) {
        if (msg.role == "system" && systemText.empty()) {
            systemText = msg.content;
        } else {
            conversationMessages.push_back(msg);
        }
    }

    std::ostringstream body;
    body << "{";
    body << "\"model\":\"" << _escapeJsonString(request.model) << "\"";
    body << ",\"max_tokens\":" << request.maxTokens;
    body << ",\"messages\":" << _buildMessagesArray(conversationMessages);

    if (!systemText.empty()) {
        body << ",\"system\":\"" << _escapeJsonString(systemText) << "\"";
    }

    if (request.highReasoningMode) {
        // Extended thinking: add thinking block and omit temperature.
        body << ",\"thinking\":{\"type\":\"enabled\",\"budget_tokens\":"
             << _thinkingBudgetTokens << "}";
    } else {
        body << ",\"temperature\":" << request.temperature;
    }

    body << "}";
    return body.str();
}

ProviderResponse AnthropicProviderClientImpl::_parseResponseBody(
        const std::string& rawBody,
        int httpStatus) const {
    ProviderResponse response;
    response.httpStatusCode = httpStatus;

    if (rawBody.empty()) {
        response.errorMessage = "Empty response from Anthropic API.";
        return response;
    }

    // Check for top-level error: {"type":"error","error":{...}}
    if (rawBody.find("\"type\":\"error\"") != std::string::npos ||
        rawBody.find("\"type\": \"error\"") != std::string::npos) {
        const auto errPos = rawBody.find("\"error\"");
        if (errPos != std::string::npos) {
            const auto errObjStart = rawBody.find('{', errPos + 7);
            std::string errBlock = (errObjStart != std::string::npos)
                ? rawBody.substr(errObjStart)
                : rawBody;
            response.errorMessage = _extractJsonString(errBlock, "message");
        }
        if (response.errorMessage.empty()) {
            response.errorMessage = "Anthropic API returned an error (HTTP " +
                                     std::to_string(httpStatus) + ").";
        }
        return response;
    }

    // Anthropic response: content is an array of blocks.
    // We want the first block with "type":"text" and extract its "text" field.
    // The structure is: "content": [{"type": "text", "text": "..."}, ...]
    // For extended thinking, there may be a "thinking" block first; skip it.

    std::string textContent;
    {
        // Locate the "content" array and walk its elements.
        const std::string needle = "\"content\"";
        auto pos = rawBody.find(needle);
        if (pos != std::string::npos) {
            pos += needle.size();
            while (pos < rawBody.size() && rawBody[pos] != '[') ++pos;
            if (pos < rawBody.size()) {
                ++pos; // skip '['
                // Iterate array elements
                while (pos < rawBody.size() && rawBody[pos] != ']') {
                    // Skip whitespace / commas
                    while (pos < rawBody.size() &&
                           (rawBody[pos] == ' ' || rawBody[pos] == ',' ||
                            rawBody[pos] == '\n' || rawBody[pos] == '\r' ||
                            rawBody[pos] == '\t')) {
                        ++pos;
                    }
                    if (pos >= rawBody.size() || rawBody[pos] != '{') break;

                    // Find closing brace for this element.
                    const std::size_t elemStart = pos;
                    int depth = 0;
                    std::size_t elemEnd = pos;
                    while (elemEnd < rawBody.size()) {
                        if (rawBody[elemEnd] == '{') ++depth;
                        else if (rawBody[elemEnd] == '}') {
                            --depth;
                            if (depth == 0) { ++elemEnd; break; }
                        } else if (rawBody[elemEnd] == '"') {
                            ++elemEnd;
                            while (elemEnd < rawBody.size()) {
                                if (rawBody[elemEnd] == '\\') ++elemEnd;
                                else if (rawBody[elemEnd] == '"') break;
                                ++elemEnd;
                            }
                        }
                        ++elemEnd;
                    }

                    const std::string elem = rawBody.substr(elemStart, elemEnd - elemStart);
                    pos = elemEnd;

                    const std::string blockType = _extractJsonString(elem, "type");
                    if (blockType == "text") {
                        textContent = _extractJsonString(elem, "text");
                        break; // Use the first text block.
                    }
                }
            }
        }
    }

    if (textContent.empty()) {
        response.errorMessage = "Could not extract text content from Anthropic response.";
        return response;
    }

    response.content = textContent;

    // Token usage: "usage": {"input_tokens": N, "output_tokens": N}
    const auto usagePos = rawBody.find("\"usage\"");
    if (usagePos != std::string::npos) {
        const auto usageObjStart = rawBody.find('{', usagePos + 7);
        if (usageObjStart != std::string::npos) {
            const std::string usageBlock = rawBody.substr(usageObjStart);
            response.inputTokens  = static_cast<unsigned int>(
                _extractJsonInt(usageBlock, "input_tokens"));
            response.outputTokens = static_cast<unsigned int>(
                _extractJsonInt(usageBlock, "output_tokens"));
        }
    }

    response.success = true;
    return response;
}
