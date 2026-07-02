#include "BaseGenesysWebApplication.h"

#include "api/ApiRouter.h"
#include "auth/TokenService.h"
#include "http/SimpleHttpServer.h"
#include "service/SimulatorSessionService.h"
#include "session/SessionManager.h"
#include "worker/WorkerJobManager.h"

#include <charconv>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

/**
 * @brief Boots the web stack and keeps the request loop alive until shutdown.
 *
 * This entry point wires token management, session creation, worker-job storage,
 * API routing and the embedded HTTP server into one runtime pipeline.
 */
int BaseGenesysWebApplication::main(int argc, char** argv) {
    const unsigned short port = _readPort(argc, argv);
    const unsigned long maxRequests = _readMaxRequests(argc, argv);

    TokenService tokenService;
    SessionManager sessionManager(tokenService, [] {
        auto simulator = std::make_unique<Simulator>();
        // Register the built-in component plugins so that language-imported models can resolve
        // their component types by name. The dummy plugin connector maps names to compiled-in
        // classes, so this works in the static build without loading any .so file.
        simulator->getPluginManager()->autoInsertPlugins();
        return simulator;
    });
    WorkerJobManager workerJobManager;
    SimulatorSessionService simulatorSessionService(sessionManager, workerJobManager);
    ApiRouter router(simulatorSessionService);
    SimpleHttpServer server(port);

    std::cout << "[genesys-web] listening on port " << port << std::endl;

    const bool ok = server.serve([&router](const HttpRequest& request) -> HttpResponse {
        return router.handle(request);
    }, maxRequests);

    if (!ok) {
        std::cerr << "[genesys-web] failed to start/bind HTTP server" << std::endl;
        return 1;
    }
    return 0;
}

/**
 * @brief Parses the listening port from the command line or falls back to 8080.
 */
unsigned short BaseGenesysWebApplication::_readPort(int argc, char** argv) {
    for (int i = 1; i < argc - 1; ++i) {
        if (std::string_view(argv[i]) == "--port") {
            unsigned short port = 8080;
            const std::string_view value(argv[i + 1]);
            const auto result = std::from_chars(value.data(), value.data() + value.size(), port);
            if (result.ec == std::errc()) {
                return port;
            }
        }
    }
    return 8080;
}

/**
 * @brief Parses the optional request limit used to stop the server loop.
 */
unsigned long BaseGenesysWebApplication::_readMaxRequests(int argc, char** argv) {
    for (int i = 1; i < argc - 1; ++i) {
        if (std::string_view(argv[i]) == "--max-requests") {
            unsigned long maxRequests = 0;
            const std::string_view value(argv[i + 1]);
            const auto result = std::from_chars(value.data(), value.data() + value.size(), maxRequests);
            if (result.ec == std::errc()) {
                return maxRequests;
            }
        }
    }
    return 0;
}
