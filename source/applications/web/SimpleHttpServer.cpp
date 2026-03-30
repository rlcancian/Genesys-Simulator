#include "SimpleHttpServer.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <sstream>

namespace {
std::string statusText(int status) {
    switch (status) {
        case 200:
            return "OK";
        case 400:
            return "Bad Request";
        case 404:
            return "Not Found";
        case 405:
            return "Method Not Allowed";
        case 500:
            return "Internal Server Error";
        default:
            return "OK";
    }
}

HttpRequest parseRequest(const std::string& requestText) {
    HttpRequest request;
    std::istringstream stream(requestText);
    stream >> request.method >> request.path;

    const std::string separator = "\r\n\r\n";
    const auto bodyPos = requestText.find(separator);
    if (bodyPos != std::string::npos) {
        request.body = requestText.substr(bodyPos + separator.size());
    }

    return request;
}

std::string buildResponse(const HttpResponse& response) {
    const std::string payload = response.body;
    std::ostringstream builder;
    builder << "HTTP/1.1 " << response.status << " " << statusText(response.status) << "\r\n";
    builder << "Content-Type: " << response.contentType << "\r\n";
    builder << "Content-Length: " << payload.size() << "\r\n";
    builder << "Connection: close\r\n\r\n";
    builder << payload;
    return builder.str();
}
}  // namespace

SimpleHttpServer::SimpleHttpServer(unsigned short port) : _port(port) {}

bool SimpleHttpServer::serve(const RequestHandler& handler, unsigned long maxRequests) {
    const int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        return false;
    }

    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(_port);

    if (bind(serverFd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        close(serverFd);
        return false;
    }

    if (listen(serverFd, 16) < 0) {
        close(serverFd);
        return false;
    }

    unsigned long served = 0;
    while (maxRequests == 0 || served < maxRequests) {
        sockaddr_in clientAddress{};
        socklen_t clientLen = sizeof(clientAddress);
        const int clientFd = accept(serverFd, reinterpret_cast<sockaddr*>(&clientAddress), &clientLen);
        if (clientFd < 0) {
            if (errno == EINTR) {
                continue;
            }
            close(serverFd);
            return false;
        }

        char buffer[8192];
        const ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0) {
            close(clientFd);
            continue;
        }
        buffer[bytesRead] = '\0';

        const HttpRequest request = parseRequest(std::string(buffer));
        HttpResponse response = handler(request);
        const std::string wire = buildResponse(response);
        (void)send(clientFd, wire.c_str(), wire.size(), 0);
        close(clientFd);
        ++served;
    }

    close(serverFd);
    return true;
}
