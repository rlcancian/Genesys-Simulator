#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"

#include <functional>

/**
 * @brief Minimal blocking HTTP/1.1 server for single-request-per-connection handling.
 *
 * This server intentionally keeps the transport model simple (no keep-alive, no chunked
 * transfer decoding) and delegates application routing to a request handler callback.
 */
class SimpleHttpServer {
public:
    using RequestHandler = std::function<HttpResponse(const HttpRequest&)>;

    /**
     * @brief Construct a server bound to a TCP port.
     * @param port TCP port that the server will bind/listen on.
     */
    explicit SimpleHttpServer(unsigned short port);

    /**
     * @brief Serve HTTP requests until the optional request limit is reached.
     * @param handler Callback used to transform a parsed request into a response.
     * @param maxRequests Maximum number of client requests to process (0 means unlimited).
     * @return true when the serving loop ends normally; false on socket setup/runtime failure.
     */
    bool serve(const RequestHandler& handler, unsigned long maxRequests = 0);

private:
    unsigned short _port;
};
