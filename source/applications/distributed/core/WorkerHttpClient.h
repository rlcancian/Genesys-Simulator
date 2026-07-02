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
 *
 * Connect and receive timeouts are separate on purpose: connecting (or detecting a dead worker)
 * should be fast, while a request that drives a whole simulation may legitimately take minutes.
 * Use a short connect timeout and a receive timeout sized to the expected job duration.
 */
class WorkerHttpClient {
public:
    /**
     * @param connectTimeoutSeconds Timeout for establishing the TCP connection.
     * @param recvTimeoutSeconds     Receive/send timeout applied while reading the response
     *                               (must cover the full server-side processing time).
     */
    explicit WorkerHttpClient(int connectTimeoutSeconds = 5, int recvTimeoutSeconds = 5);

    // Virtual so tests can inject a fake client; the real implementation uses POSIX sockets.
    virtual ~WorkerHttpClient() = default;

    /** @brief Performs a GET request, optionally authenticated with a Bearer token. */
    virtual HttpClientResponse get(const std::string& host,
                                   int port,
                                   const std::string& path,
                                   const std::string& bearerToken = "") const;

    /** @brief Performs a POST request with an optional body and Bearer token. */
    virtual HttpClientResponse post(const std::string& host,
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

    int _connectTimeoutSeconds;
    int _recvTimeoutSeconds;
};

} // namespace genesys::distributed
