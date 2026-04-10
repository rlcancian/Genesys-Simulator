#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"

#include <functional>

class SimpleHttpServer {
public:
    using RequestHandler = std::function<HttpResponse(const HttpRequest&)>;

    explicit SimpleHttpServer(unsigned short port);

    bool serve(const RequestHandler& handler, unsigned long maxRequests = 0);

private:
    unsigned short _port;
};
