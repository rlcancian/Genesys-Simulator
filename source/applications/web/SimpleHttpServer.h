#pragma once

#include <functional>
#include <string>

struct HttpRequest {
    std::string method;
    std::string path;
    std::string body;
};

struct HttpResponse {
    int status = 200;
    std::string contentType = "application/json";
    std::string body;
};

class SimpleHttpServer {
public:
    using RequestHandler = std::function<HttpResponse(const HttpRequest&)>;

    explicit SimpleHttpServer(unsigned short port);

    bool serve(const RequestHandler& handler, unsigned long maxRequests = 0);

private:
    unsigned short _port;
};
