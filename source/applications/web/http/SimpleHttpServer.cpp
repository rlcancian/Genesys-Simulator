#include "SimpleHttpServer.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cctype>
#include <cstring>
#include <netinet/in.h>
#include <optional>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>

namespace {
std::string statusText(int status) {
    switch (status) {
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 400:
            return "Bad Request";
        case 401:
            return "Unauthorized";
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

std::string toLower(std::string value) {
    for (char& c : value) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return value;
}

std::optional<HttpRequest> parseRequest(const std::string& requestText) {
    const std::string separator = "\r\n\r\n";
    const auto headersEnd = requestText.find(separator);
    if (headersEnd == std::string::npos) {
        return std::nullopt;
    }

    HttpRequest request;
    std::istringstream headerStream(requestText.substr(0, headersEnd));

    std::string requestLine;
    if (!std::getline(headerStream, requestLine)) {
        return std::nullopt;
    }

    if (!requestLine.empty() && requestLine.back() == '\r') {
        requestLine.pop_back();
    }

    std::istringstream requestLineStream(requestLine);
    std::string httpVersion;
    if (!(requestLineStream >> request.method >> request.path >> httpVersion)) {
        return std::nullopt;
    }

    std::string line;
    while (std::getline(headerStream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }

        const auto colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            continue;
        }

        std::string name = toLower(line.substr(0, colonPos));
        std::string value = line.substr(colonPos + 1);
        while (!value.empty() && value.front() == ' ') {
            value.erase(value.begin());
        }

        request.headers[name] = value;
    }

    const auto bodyStart = headersEnd + separator.size();
    std::size_t contentLength = requestText.size() - bodyStart;
    if (const auto it = request.headers.find("content-length"); it != request.headers.end()) {
        try {
            contentLength = static_cast<std::size_t>(std::stoul(it->second));
        } catch (...) {
            return std::nullopt;
        }
    }

    if (bodyStart + contentLength > requestText.size()) {
        return std::nullopt;
    }

    request.body = requestText.substr(bodyStart, contentLength);
    return request;
}

std::string buildResponse(const HttpResponse& response) {
    const std::string payload = response.body;
    std::ostringstream builder;
    builder << "HTTP/1.1 " << response.status << " " << statusText(response.status) << "\r\n";
    builder << "Content-Type: " << response.contentType << "\r\n";
    builder << "Content-Length: " << payload.size() << "\r\n";
    for (const auto& [name, value] : response.headers) {
        builder << name << ": " << value << "\r\n";
    }
    builder << "Connection: close\r\n\r\n";
    builder << payload;
    return builder.str();
}

HttpResponse badRequestResponse() {
    return HttpResponse{400, "application/json", "{\"ok\":false,\"error\":{\"code\":\"BAD_REQUEST\",\"message\":\"Invalid HTTP request\"}}"};
}

HttpResponse internalErrorResponse() {
    return HttpResponse{500, "application/json", "{\"ok\":false,\"error\":{\"code\":\"INTERNAL_SERVER_ERROR\",\"message\":\"Unhandled server error\"}}"};
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

        char buffer[16384];
        const ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0) {
            close(clientFd);
            continue;
        }
        buffer[bytesRead] = '\0';

        HttpResponse response;
        if (auto request = parseRequest(std::string(buffer, static_cast<std::size_t>(bytesRead))); request.has_value()) {
            try {
                response = handler(*request);
            } catch (...) {
                response = internalErrorResponse();
            }
        } else {
            response = badRequestResponse();
        }

        const std::string wire = buildResponse(response);
        (void)send(clientFd, wire.c_str(), wire.size(), 0);
        close(clientFd);
        ++served;
    }

    close(serverFd);
    return true;
}
