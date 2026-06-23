#include "WorkerHttpClient.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

namespace genesys::distributed {

namespace {
constexpr std::size_t kMaxResponseBytes = 1024 * 1024;  // 1 MiB guardrail, mirrors the server.
constexpr std::size_t kReadChunkSize = 4096;

// RAII guard so the socket is always closed, including on early returns.
class SocketGuard {
public:
    explicit SocketGuard(int fd) : _fd(fd) {}
    ~SocketGuard() {
        if (_fd >= 0) {
            ::close(_fd);
        }
    }
    SocketGuard(const SocketGuard&) = delete;
    SocketGuard& operator=(const SocketGuard&) = delete;
    int get() const { return _fd; }

private:
    int _fd;
};

HttpClientResponse failure(const std::string& message) {
    HttpClientResponse response;
    response.ok = false;
    response.status = 0;
    response.error = message;
    return response;
}

// Connects with a bounded timeout using a temporary non-blocking socket.
bool connectWithTimeout(int fd, const addrinfo* address, int timeoutSeconds, std::string& error) {
    const int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags < 0 || ::fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        error = "Unable to configure socket";
        return false;
    }

    const int result = ::connect(fd, address->ai_addr, address->ai_addrlen);
    if (result < 0 && errno != EINPROGRESS) {
        error = std::string("Connect failed: ") + std::strerror(errno);
        return false;
    }

    if (result < 0) {
        fd_set writeSet;
        FD_ZERO(&writeSet);
        FD_SET(fd, &writeSet);
        timeval timeout{timeoutSeconds, 0};
        const int selected = ::select(fd + 1, nullptr, &writeSet, nullptr, &timeout);
        if (selected <= 0) {
            error = selected == 0 ? "Connect timed out" : "Connect select failed";
            return false;
        }
        int socketError = 0;
        socklen_t length = sizeof(socketError);
        if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, &socketError, &length) < 0 || socketError != 0) {
            error = "Connect failed";
            return false;
        }
    }

    if (::fcntl(fd, F_SETFL, flags) < 0) {
        error = "Unable to restore socket mode";
        return false;
    }
    return true;
}

bool sendAll(int fd, const std::string& data, std::string& error) {
    std::size_t totalSent = 0;
    while (totalSent < data.size()) {
        const ssize_t sent = ::send(fd, data.data() + totalSent, data.size() - totalSent, 0);
        if (sent <= 0) {
            error = std::string("Send failed: ") + std::strerror(errno);
            return false;
        }
        totalSent += static_cast<std::size_t>(sent);
    }
    return true;
}

// Parses the HTTP status code from a status line like "HTTP/1.1 200 OK".
int parseStatusCode(const std::string& responseText) {
    const std::size_t firstSpace = responseText.find(' ');
    if (firstSpace == std::string::npos) {
        return 0;
    }
    return std::atoi(responseText.c_str() + firstSpace + 1);
}

} // namespace

WorkerHttpClient::WorkerHttpClient(int timeoutSeconds) : _timeoutSeconds(timeoutSeconds) {}

HttpClientResponse WorkerHttpClient::get(const std::string& host,
                                         int port,
                                         const std::string& path,
                                         const std::string& bearerToken) const {
    return _request("GET", host, port, path, "", "application/json", bearerToken);
}

HttpClientResponse WorkerHttpClient::post(const std::string& host,
                                          int port,
                                          const std::string& path,
                                          const std::string& body,
                                          const std::string& contentType,
                                          const std::string& bearerToken) const {
    return _request("POST", host, port, path, body, contentType, bearerToken);
}

HttpClientResponse WorkerHttpClient::_request(const std::string& method,
                                              const std::string& host,
                                              int port,
                                              const std::string& path,
                                              const std::string& body,
                                              const std::string& contentType,
                                              const std::string& bearerToken) const {
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* resolved = nullptr;
    const std::string portText = std::to_string(port);
    if (::getaddrinfo(host.c_str(), portText.c_str(), &hints, &resolved) != 0 || resolved == nullptr) {
        return failure("Unable to resolve host " + host);
    }

    int fd = ::socket(resolved->ai_family, resolved->ai_socktype, resolved->ai_protocol);
    if (fd < 0) {
        ::freeaddrinfo(resolved);
        return failure("Unable to create socket");
    }
    SocketGuard socketGuard(fd);

    timeval timeout{_timeoutSeconds, 0};
    ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    ::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    std::string connectError;
    const bool connected = connectWithTimeout(fd, resolved, _timeoutSeconds, connectError);
    ::freeaddrinfo(resolved);
    if (!connected) {
        return failure(connectError);
    }

    std::string request = method + " " + path + " HTTP/1.1\r\n";
    request += "Host: " + host + ":" + portText + "\r\n";
    if (!bearerToken.empty()) {
        request += "Authorization: Bearer " + bearerToken + "\r\n";
    }
    if (method == "POST" || !body.empty()) {
        request += "Content-Type: " + contentType + "\r\n";
        request += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    }
    request += "Connection: close\r\n\r\n";
    request += body;

    std::string sendError;
    if (!sendAll(fd, request, sendError)) {
        return failure(sendError);
    }

    std::string responseText;
    char buffer[kReadChunkSize];
    while (true) {
        const ssize_t received = ::recv(fd, buffer, sizeof(buffer), 0);
        if (received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return failure("Receive timed out");
            }
            return failure(std::string("Receive failed: ") + std::strerror(errno));
        }
        if (received == 0) {
            break;  // Peer closed the connection (we requested Connection: close).
        }
        responseText.append(buffer, static_cast<std::size_t>(received));
        if (responseText.size() > kMaxResponseBytes) {
            return failure("Response exceeded size limit");
        }
    }

    const std::size_t headerEnd = responseText.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        return failure("Malformed HTTP response");
    }

    HttpClientResponse response;
    response.ok = true;
    response.status = parseStatusCode(responseText);
    response.body = responseText.substr(headerEnd + 4);
    return response;
}

} // namespace genesys::distributed
