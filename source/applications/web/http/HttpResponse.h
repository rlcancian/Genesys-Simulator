#pragma once

#include <string>
#include <unordered_map>

/**
 * @brief Minimal HTTP response payload produced by the web API router.
 *
 * The response mirrors the small transport model used by the embedded server:
 * status, content type, body, and a simple header map.
 */
struct HttpResponse {
    /// HTTP status code returned to the client.
    int status = 200;
    /// MIME type advertised in the response.
    std::string contentType = "application/json";
    /// Raw response body, usually JSON.
    std::string body;
    /// Arbitrary response headers written by the router or handlers.
    std::unordered_map<std::string, std::string> headers;
};
