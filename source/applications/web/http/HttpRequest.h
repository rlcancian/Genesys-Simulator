#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_map>

/**
 * @brief Lightweight representation of one parsed HTTP request.
 *
 * The web application uses a small transport surface, so the request keeps only
 * the pieces needed by the router: method, path, raw body and normalized
 * header lookup.
 */
struct HttpRequest {
    /// HTTP method token as received from the request line.
    std::string method;
    /// Request target path used by the API router.
    std::string path;
    /// Raw body payload passed to JSON parsers and handlers.
    std::string body;
    /// Lowercase header map used for case-insensitive lookups.
    std::unordered_map<std::string, std::string> headers;

    /**
     * @brief Returns a header value using a case-insensitive lookup.
     * @param name Header name to search.
     * @return Header value when present, or an empty string otherwise.
     */
    [[nodiscard]] std::string header(const std::string& name) const {
        std::string normalized = name;
        std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        auto it = headers.find(normalized);
        if (it == headers.end()) {
            return {};
        }
        return it->second;
    }
};
