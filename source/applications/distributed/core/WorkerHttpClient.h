#pragma once

#include <string>

namespace genesys::distributed {

/**
 * @brief Outcome of an outbound HTTP request to a worker.
 *
 * `ok` reports transport success (a response was received and parsed); `status` carries the
 * HTTP status code. On transport failure (connect/timeout/read error) `ok` is false, `status`
 * is 0 and `error` describes the failure.
 */
struct HttpClientResponse {
    bool ok = false;
    int status = 0;
    std::string body;
    std::string error;
};

/**
 * @brief Minimal blocking HTTP/1.1 client used to drive remote GenESyS workers.
 *
 * Self-contained over POSIX sockets (no external dependency), mirroring the limits of the
 * worker's SimpleHttpServer: bounded response size, per-socket receive/send timeouts and a
 * bounded connect timeout. Requests send `Connection: close`, so the response is read until
 * the peer closes the connection.
 */
class WorkerHttpClient {
public:
    /** @param timeoutSeconds Connect and receive/send timeout applied to every request. */
    explicit WorkerHttpClient(int timeoutSeconds = 5);

    /** @brief Performs a GET request, optionally authenticated with a Bearer token. */
    HttpClientResponse get(const std::string& host,
                           int port,
                           const std::string& path,
                           const std::string& bearerToken = "") const;

    /** @brief Performs a POST request with an optional body and Bearer token. */
    HttpClientResponse post(const std::string& host,
                            int port,
                            const std::string& path,
                            const std::string& body,
                            const std::string& contentType = "application/json",
                            const std::string& bearerToken = "") const;

private:
    HttpClientResponse _request(const std::string& method,
                                const std::string& host,
                                int port,
                                const std::string& path,
                                const std::string& body,
                                const std::string& contentType,
                                const std::string& bearerToken) const;

    int _timeoutSeconds;
};

} // namespace genesys::distributed
