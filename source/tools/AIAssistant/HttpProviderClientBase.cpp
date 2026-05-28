/*
 * File:   HttpProviderClientBase.cpp
 * Author: Genesys Team
 *
 * Created on 28 de Maio de 2026
 */

#include "HttpProviderClientBase.h"

#include <algorithm>   // std::fill
#include <array>       // std::array for popen buffer
#include <cstdio>      // popen, pclose, fgets, fclose
#include <cstring>     // strlen
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// POSIX temp-file creation and file permissions
#include <unistd.h>    // unlink, close
#include <fcntl.h>     // O_WRONLY, O_CREAT
#include <sys/stat.h>  // fchmod, chmod

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

HttpProviderClientBase::HttpProviderClientBase() = default;

HttpProviderClientBase::~HttpProviderClientBase() {
    clearApiKey();
}

// ---------------------------------------------------------------------------
// AIProviderClient_if — configuration
// ---------------------------------------------------------------------------

bool HttpProviderClientBase::isAvailable() const {
    return !_baseUrl.empty();
}

void HttpProviderClientBase::setBaseUrl(const std::string& url) {
    _baseUrl = url;
}

std::string HttpProviderClientBase::getBaseUrl() const {
    return _baseUrl;
}

void HttpProviderClientBase::setTimeoutSeconds(unsigned int seconds) {
    _timeoutSeconds = seconds;
}

unsigned int HttpProviderClientBase::getTimeoutSeconds() const {
    return _timeoutSeconds;
}

// ---------------------------------------------------------------------------
// Security — API key
// ---------------------------------------------------------------------------

void HttpProviderClientBase::setApiKey(const std::string& apiKey) {
    std::fill(_apiKey.begin(), _apiKey.end(), '\0');
    _apiKey = apiKey;
}

void HttpProviderClientBase::clearApiKey() {
    std::fill(_apiKey.begin(), _apiKey.end(), '\0');
    _apiKey.clear();
}

bool HttpProviderClientBase::hasApiKey() const {
    return !_apiKey.empty();
}

// ---------------------------------------------------------------------------
// Default hook implementations
// ---------------------------------------------------------------------------

std::vector<std::string> HttpProviderClientBase::_extraRequestHeaders() const {
    return {};
}

// ---------------------------------------------------------------------------
// send() — curl transport
// ---------------------------------------------------------------------------

ProviderResponse HttpProviderClientBase::send(const ProviderRequest& request) {
    ProviderResponse response;

    if (_baseUrl.empty()) {
        response.errorMessage = "Provider base URL is not configured.";
        return response;
    }

    // ------------------------------------------------------------------
    // 1. Serialise the request body to a temporary file.
    // ------------------------------------------------------------------
    char bodyPath[] = "/tmp/genesys-ai-body-XXXXXX";
    int bodyFd = ::mkstemp(bodyPath);
    if (bodyFd == -1) {
        response.errorMessage = "Failed to create temporary request body file.";
        return response;
    }
    // Restrict permissions immediately so no other user can read the body.
    ::fchmod(bodyFd, 0600);

    const std::string bodyJson = _buildRequestBody(request);
    {
        const ssize_t written = ::write(bodyFd, bodyJson.c_str(), bodyJson.size());
        ::close(bodyFd);
        if (written < 0 || static_cast<size_t>(written) != bodyJson.size()) {
            ::unlink(bodyPath);
            response.errorMessage = "Failed to write request body to temporary file.";
            return response;
        }
    }

    // ------------------------------------------------------------------
    // 2. Build a curl config file containing the URL, headers (including
    //    the API key), body reference, and timeout.
    //    The API key is written here — NOT on the curl command line.
    // ------------------------------------------------------------------
    char cfgPath[] = "/tmp/genesys-ai-cfg-XXXXXX";
    int cfgFd = ::mkstemp(cfgPath);
    if (cfgFd == -1) {
        ::unlink(bodyPath);
        response.errorMessage = "Failed to create temporary curl config file.";
        return response;
    }
    ::fchmod(cfgFd, 0600);

    {
        std::ostringstream cfg;
        cfg << "url = \"" << _baseUrl << "\"\n";
        cfg << "request = \"POST\"\n";
        cfg << "header = \"Content-Type: application/json\"\n";

        // Authorization header (contains the API key)
        const std::string authHdr = _authorizationHeader();
        if (!authHdr.empty()) {
            cfg << "header = \"" << authHdr << "\"\n";
        }

        // Extra provider-specific headers
        for (const std::string& hdr : _extraRequestHeaders()) {
            cfg << "header = \"" << hdr << "\"\n";
        }

        // Body is referenced by file to avoid any escaping issues.
        cfg << "data = @" << bodyPath << "\n";

        // Timeout: curl's --max-time in seconds
        cfg << "max-time = " << _timeoutSeconds << "\n";

        // Ask curl to write the HTTP status code at the very end of stdout.
        // We use --write-out to append it after the body.
        cfg << "write-out = \"\\nHTTP_STATUS:%{http_code}\"\n";

        // Suppress progress bar
        cfg << "silent\n";

        const std::string cfgText = cfg.str();
        const ssize_t written = ::write(cfgFd, cfgText.c_str(), cfgText.size());
        ::close(cfgFd);
        if (written < 0 || static_cast<size_t>(written) != cfgText.size()) {
            ::unlink(bodyPath);
            ::unlink(cfgPath);
            response.errorMessage = "Failed to write curl config file.";
            return response;
        }
    }

    // ------------------------------------------------------------------
    // 3. Execute curl, capture stdout.
    //    The API key is in the config file, never on argv.
    // ------------------------------------------------------------------
    const std::string curlCmd = std::string("curl --config ") + cfgPath + " 2>&1";

    std::string rawOutput;
    {
        FILE* pipe = ::popen(curlCmd.c_str(), "r");
        if (pipe == nullptr) {
            ::unlink(bodyPath);
            ::unlink(cfgPath);
            response.errorMessage = "popen(curl) failed. Ensure curl is installed and on PATH.";
            return response;
        }

        std::array<char, 4096> buf{};
        while (::fgets(buf.data(), static_cast<int>(buf.size()), pipe) != nullptr) {
            rawOutput += buf.data();
        }
        const int exitCode = ::pclose(pipe);
        (void)exitCode; // We extract HTTP status from write-out instead.
    }

    // ------------------------------------------------------------------
    // 4. Unlink temp files as early as possible.
    // ------------------------------------------------------------------
    ::unlink(bodyPath);
    ::unlink(cfgPath);

    // ------------------------------------------------------------------
    // 5. Extract the HTTP status code appended by --write-out.
    // ------------------------------------------------------------------
    int httpStatus = 0;
    {
        const std::string marker = "\nHTTP_STATUS:";
        const auto pos = rawOutput.rfind(marker);
        if (pos != std::string::npos) {
            const std::string statusStr = rawOutput.substr(pos + marker.size());
            try { httpStatus = std::stoi(statusStr); } catch (...) {}
            rawOutput = rawOutput.substr(0, pos);
        }
    }

    // ------------------------------------------------------------------
    // 6. Delegate JSON parsing to the concrete implementation.
    // ------------------------------------------------------------------
    return _parseResponseBody(rawOutput, httpStatus);
}

// ---------------------------------------------------------------------------
// JSON utilities
// ---------------------------------------------------------------------------

std::string HttpProviderClientBase::_escapeJsonString(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (const unsigned char c : s) {
        switch (c) {
        case '"':  out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\b': out += "\\b";  break;
        case '\f': out += "\\f";  break;
        case '\n': out += "\\n";  break;
        case '\r': out += "\\r";  break;
        case '\t': out += "\\t";  break;
        default:
            if (c < 0x20) {
                // Control character — emit \uXXXX escape
                char hex[8];
                std::snprintf(hex, sizeof(hex), "\\u%04x", static_cast<unsigned>(c));
                out += hex;
            } else {
                out += static_cast<char>(c);
            }
            break;
        }
    }
    return out;
}

std::string HttpProviderClientBase::_extractJsonString(const std::string& json,
                                                        const std::string& key) {
    // Look for "key": "value"  (whitespace tolerant)
    const std::string needle = "\"" + key + "\"";
    auto pos = json.find(needle);
    if (pos == std::string::npos) return "";

    pos += needle.size();
    // Skip whitespace and colon
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' ||
                                  json[pos] == '\n' || json[pos] == '\r')) {
        ++pos;
    }
    if (pos >= json.size() || json[pos] != ':') return "";
    ++pos; // skip ':'
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' ||
                                  json[pos] == '\n' || json[pos] == '\r')) {
        ++pos;
    }
    if (pos >= json.size() || json[pos] != '"') return "";
    ++pos; // skip opening quote

    // Read characters, handling backslash escapes.
    std::string value;
    while (pos < json.size()) {
        const char c = json[pos];
        if (c == '"') break; // closing quote
        if (c == '\\' && pos + 1 < json.size()) {
            ++pos;
            switch (json[pos]) {
            case '"':  value += '"';  break;
            case '\\': value += '\\'; break;
            case '/':  value += '/';  break;
            case 'b':  value += '\b'; break;
            case 'f':  value += '\f'; break;
            case 'n':  value += '\n'; break;
            case 'r':  value += '\r'; break;
            case 't':  value += '\t'; break;
            default:   value += json[pos]; break;
            }
        } else {
            value += c;
        }
        ++pos;
    }
    return value;
}

double HttpProviderClientBase::_extractJsonDouble(const std::string& json,
                                                   const std::string& key) {
    const std::string needle = "\"" + key + "\"";
    auto pos = json.find(needle);
    if (pos == std::string::npos) return 0.0;

    pos += needle.size();
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' ||
                                  json[pos] == '\n' || json[pos] == '\r')) {
        ++pos;
    }
    if (pos >= json.size() || json[pos] != ':') return 0.0;
    ++pos;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' ||
                                  json[pos] == '\n' || json[pos] == '\r')) {
        ++pos;
    }
    if (pos >= json.size()) return 0.0;

    // Collect a floating-point literal: optional minus, digits, optional dot+digits,
    // optional exponent.
    std::string numStr;
    if (json[pos] == '-') { numStr += '-'; ++pos; }
    while (pos < json.size() && (
               (json[pos] >= '0' && json[pos] <= '9') ||
               json[pos] == '.' ||
               json[pos] == 'e' ||
               json[pos] == 'E' ||
               json[pos] == '+' ||
               (json[pos] == '-' && !numStr.empty()))) {
        numStr += json[pos++];
    }
    if (numStr.empty() || numStr == "-") return 0.0;
    try { return std::stod(numStr); } catch (...) { return 0.0; }
}

int HttpProviderClientBase::_extractJsonInt(const std::string& json,
                                             const std::string& key) {
    const std::string needle = "\"" + key + "\"";
    auto pos = json.find(needle);
    if (pos == std::string::npos) return 0;

    pos += needle.size();
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' ||
                                  json[pos] == '\n' || json[pos] == '\r')) {
        ++pos;
    }
    if (pos >= json.size() || json[pos] != ':') return 0;
    ++pos;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' ||
                                  json[pos] == '\n' || json[pos] == '\r')) {
        ++pos;
    }
    if (pos >= json.size()) return 0;

    // Collect digits (and optional leading minus)
    std::string digits;
    if (json[pos] == '-') { digits += '-'; ++pos; }
    while (pos < json.size() && json[pos] >= '0' && json[pos] <= '9') {
        digits += json[pos++];
    }
    if (digits.empty() || digits == "-") return 0;
    try { return std::stoi(digits); } catch (...) { return 0; }
}

std::string HttpProviderClientBase::_buildMessagesArray(
        const std::vector<ChatMessage>& messages) {
    std::ostringstream arr;
    arr << "[";
    for (std::size_t i = 0; i < messages.size(); ++i) {
        if (i > 0) arr << ",";
        arr << "{\"role\":\""    << _escapeJsonString(messages[i].role)    << "\","
            << "\"content\":\"" << _escapeJsonString(messages[i].content) << "\"}";
    }
    arr << "]";
    return arr.str();
}

std::string HttpProviderClientBase::_extractFirstArrayElement(
        const std::string& json,
        const std::string& arrayKey) {
    const std::string needle = "\"" + arrayKey + "\"";
    auto pos = json.find(needle);
    if (pos == std::string::npos) return "";

    pos += needle.size();
    // Skip to ':'
    while (pos < json.size() && json[pos] != ':') ++pos;
    if (pos >= json.size()) return "";
    ++pos;
    // Skip whitespace
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\n' ||
                                  json[pos] == '\r' || json[pos] == '\t')) ++pos;
    if (pos >= json.size() || json[pos] != '[') return "";
    ++pos; // skip '['
    // Skip whitespace
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\n' ||
                                  json[pos] == '\r' || json[pos] == '\t')) ++pos;
    if (pos >= json.size() || json[pos] != '{') return "";

    // Find matching closing brace for the first element.
    int depth = 0;
    const std::size_t start = pos;
    while (pos < json.size()) {
        if (json[pos] == '{') ++depth;
        else if (json[pos] == '}') {
            --depth;
            if (depth == 0) { ++pos; break; }
        } else if (json[pos] == '"') {
            // Skip string literal
            ++pos;
            while (pos < json.size()) {
                if (json[pos] == '\\') { ++pos; } // skip escaped char
                else if (json[pos] == '"') { break; }
                ++pos;
            }
        }
        ++pos;
    }
    return json.substr(start, pos - start);
}
