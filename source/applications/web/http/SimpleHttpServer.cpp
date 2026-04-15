#include "SimpleHttpServer.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cctype>
#include <cstring>
#include <netinet/in.h>
#include <optional>
#include <sstream>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

namespace {
constexpr std::size_t kMaxRequestBytes = 1024 * 1024;  // 1 MiB transport guardrail.
constexpr int kSocketTimeoutSeconds = 5;
constexpr std::size_t kReadChunkSize = 4096;

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
        case 408:
            return "Request Timeout";
        case 413:
            return "Payload Too Large";
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

std::optional<std::size_t> parseContentLengthFromHeaders(const std::string& requestText) {
    const std::string separator = "\r\n\r\n";
    const auto headersEnd = requestText.find(separator);
    if (headersEnd == std::string::npos) {
        return std::nullopt;
    }

    std::istringstream headerStream(requestText.substr(0, headersEnd));
    std::string line;
    std::getline(headerStream, line);  // Skip request line.

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
        if (name != "content-length") {
            continue;
        }

        std::string value = line.substr(colonPos + 1);
        while (!value.empty() && value.front() == ' ') {
            value.erase(value.begin());
        }

        try {
            return static_cast<std::size_t>(std::stoul(value));
        } catch (...) {
            return std::nullopt;
        }
    }

    return 0;
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

enum class ReadRequestResult {
    Complete,
    Timeout,
    TooLarge,
    Invalid,
    SocketError,
    Closed
};

ReadRequestResult readHttpRequest(int clientFd, std::string& requestText) {
    requestText.clear();
    requestText.reserve(kReadChunkSize);

    std::optional<std::size_t> contentLength;
    std::size_t expectedTotalSize = 0;

    while (true) {
        char chunk[kReadChunkSize];
        const ssize_t bytesRead = recv(clientFd, chunk, sizeof(chunk), 0);
        if (bytesRead == 0) {
            return requestText.empty() ? ReadRequestResult::Closed : ReadRequestResult::Invalid;
        }
        if (bytesRead < 0) {
            // Timeout from SO_RCVTIMEO.
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return ReadRequestResult::Timeout;
            }
            if (errno == EINTR) {
                continue;
            }
            return ReadRequestResult::SocketError;
        }

        requestText.append(chunk, static_cast<std::size_t>(bytesRead));
        if (requestText.size() > kMaxRequestBytes) {
            return ReadRequestResult::TooLarge;
        }

        if (!contentLength.has_value()) {
            const std::string separator = "\r\n\r\n";
            const auto headersEnd = requestText.find(separator);
            if (headersEnd != std::string::npos) {
                contentLength = parseContentLengthFromHeaders(requestText);
                if (!contentLength.has_value()) {
                    return ReadRequestResult::Invalid;
                }

                expectedTotalSize = headersEnd + separator.size() + *contentLength;
                if (expectedTotalSize > kMaxRequestBytes) {
                    return ReadRequestResult::TooLarge;
                }
            }
        }

        if (contentLength.has_value() && requestText.size() >= expectedTotalSize) {
            requestText.resize(expectedTotalSize);
            return ReadRequestResult::Complete;
        }
    }
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

HttpResponse requestTimeoutResponse() {
    return HttpResponse{408,
                        "application/json",
                        "{\"ok\":false,\"error\":{\"code\":\"REQUEST_TIMEOUT\",\"message\":\"Timed out while reading HTTP request\"}}"};
}

HttpResponse payloadTooLargeResponse() {
    return HttpResponse{413,
                        "application/json",
                        "{\"ok\":false,\"error\":{\"code\":\"PAYLOAD_TOO_LARGE\",\"message\":\"HTTP request exceeds transport limit\"}}"};
}

HttpResponse internalErrorResponse() {
    return HttpResponse{500, "application/json", "{\"ok\":false,\"error\":{\"code\":\"INTERNAL_SERVER_ERROR\",\"message\":\"Unhandled server error\"}}"};
}

void configureSocketTimeouts(int fd) {
    timeval timeout{};
    timeout.tv_sec = kSocketTimeoutSeconds;
    timeout.tv_usec = 0;
    (void)setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    (void)setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
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

        configureSocketTimeouts(clientFd);

        std::string requestText;
        const ReadRequestResult readResult = readHttpRequest(clientFd, requestText);

        HttpResponse response;
        if (readResult == ReadRequestResult::Complete) {
            if (auto request = parseRequest(requestText); request.has_value()) {
                try {
                    response = handler(*request);
                } catch (...) {
                    response = internalErrorResponse();
                }
            } else {
                response = badRequestResponse();
            }
        } else if (readResult == ReadRequestResult::Timeout) {
            response = requestTimeoutResponse();
        } else if (readResult == ReadRequestResult::TooLarge) {
            response = payloadTooLargeResponse();
        } else if (readResult == ReadRequestResult::Invalid) {
            response = badRequestResponse();
        } else if (readResult == ReadRequestResult::SocketError) {
            response = internalErrorResponse();
        } else {
            close(clientFd);
            continue;
        }

        const std::string wire = buildResponse(response);
        (void)send(clientFd, wire.c_str(), wire.size(), 0);
        close(clientFd);
        ++served;
    }

    close(serverFd);
    return true;
}
