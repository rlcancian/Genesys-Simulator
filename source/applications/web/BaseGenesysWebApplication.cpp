#include "BaseGenesysWebApplication.h"

#include "SimpleHttpServer.h"

#include <iostream>
#include <string>

namespace {
HttpResponse badRequest(const std::string& message) {
    return HttpResponse{400, "application/json", "{\"ok\":false,\"error\":\"" + message + "\"}"};
}
}  // namespace

int BaseGenesysWebApplication::main(int argc, char** argv) {
    const unsigned short port = _readPort(argc, argv);
    const unsigned long maxRequests = _readMaxRequests(argc, argv);

    Simulator simulator;
    SimpleHttpServer server(port);

    std::cout << "[genesys-web] listening on port " << port << std::endl;

    const bool ok = server.serve([&simulator](const HttpRequest& request) -> HttpResponse {
        if (request.method.empty() || request.path.empty()) {
            return badRequest("invalid request line");
        }

        if (request.method != "GET") {
            return HttpResponse{405, "application/json", "{\"ok\":false,\"error\":\"only GET is supported in v0\"}"};
        }

        if (request.path == "/health") {
            return HttpResponse{200, "application/json", "{\"ok\":true,\"status\":\"up\"}"};
        }

        if (request.path == "/api/v1/simulator/info") {
            const std::string name = _escapeJson(simulator.getName());
            const std::string versionName = _escapeJson(simulator.getVersion());
            const std::string payload =
                "{\"ok\":true,\"data\":{"
                "\"name\":\"" + name + "\"," 
                "\"versionName\":\"" + versionName + "\"," 
                "\"versionNumber\":" + std::to_string(simulator.getVersionNumber()) + "}}";
            return HttpResponse{200, "application/json", payload};
        }

        return HttpResponse{404, "application/json", "{\"ok\":false,\"error\":\"route not found\"}"};
    }, maxRequests);

    if (!ok) {
        std::cerr << "[genesys-web] failed to start/bind HTTP server" << std::endl;
        return 1;
    }
    return 0;
}

unsigned short BaseGenesysWebApplication::_readPort(int argc, char** argv) {
    for (int i = 1; i < argc - 1; ++i) {
        if (std::string(argv[i]) == "--port") {
            return static_cast<unsigned short>(std::stoi(argv[i + 1]));
        }
    }
    return 8080;
}

unsigned long BaseGenesysWebApplication::_readMaxRequests(int argc, char** argv) {
    for (int i = 1; i < argc - 1; ++i) {
        if (std::string(argv[i]) == "--max-requests") {
            return static_cast<unsigned long>(std::stoul(argv[i + 1]));
        }
    }
    return 0;
}

std::string BaseGenesysWebApplication::_escapeJson(const std::string& value) {
    std::string out;
    out.reserve(value.size());
    for (char c : value) {
        if (c == '\\') {
            out += "\\\\";
        } else if (c == '"') {
            out += "\\\"";
        } else {
            out += c;
        }
    }
    return out;
}
