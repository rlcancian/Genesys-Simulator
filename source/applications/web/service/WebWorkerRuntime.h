#pragma once

#include "api/ApiRouter.h"
#include "auth/TokenService.h"
#include "http/SimpleHttpServer.h"
#include "kernel/simulator/Simulator.h"
#include "service/SimulatorSessionService.h"
#include "session/SessionManager.h"
#include "worker/WorkerJobManager.h"

#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

/**
 * @brief Owns the embedded web worker runtime used by the GUI and the standalone web app.
 *
 * The runtime wires the HTTP server, routing stack, and simulator-backed session
 * services into a reusable lifecycle object. GUI code can start, stop, and inspect the
 * worker without embedding HTTP details in the main window.
 */
class WebWorkerRuntime {
public:
    /**
     * @brief Captures a thread-safe snapshot of the worker state.
     */
    struct Snapshot {
        bool isRunning = false;
        bool isStarting = false;
        bool isStopping = false;
        bool hasFailed = false;
        unsigned short port = 8080;
        unsigned long maxRequests = 0;
        unsigned long servedRequests = 0;
        unsigned long activeRequests = 0;
        std::string statusMessage;
        std::string lastError;
        std::vector<std::string> recentEvents;
        std::vector<std::string> recentErrors;
    };

    /**
     * @brief Builds the reusable worker runtime with a default port and request cap.
     */
    explicit WebWorkerRuntime(unsigned short defaultPort = 8080, unsigned long maxRequests = 0);
    /**
     * @brief Stops the worker and releases background resources.
     */
    ~WebWorkerRuntime();

    /**
     * @brief Starts the worker if it is not already running.
     * @return True when a new background worker thread was launched.
     */
    bool start();
    /**
     * @brief Requests a graceful shutdown and waits for the worker thread to finish.
     */
    void stop();
    /**
     * @brief Waits for the worker thread to complete if it is running.
     */
    void wait();
    /**
     * @brief Restarts the worker using the currently configured settings.
     * @return True when the worker was scheduled to start again.
     */
    bool restart();

    /**
     * @brief Returns whether the worker thread is currently serving requests.
     */
    bool isRunning() const;
    /**
     * @brief Returns a snapshot of the current worker lifecycle and counters.
     */
    Snapshot snapshot() const;

    /**
     * @brief Updates the configured listening port used by the next start.
     */
    void setConfiguredPort(unsigned short port);
    /**
     * @brief Returns the listening port configured for the next start.
     */
    unsigned short configuredPort() const;

    /**
     * @brief Updates the configured request cap used by the next start.
     */
    void setConfiguredMaxRequests(unsigned long maxRequests);
    /**
     * @brief Returns the configured request cap used by the next start.
     */
    unsigned long configuredMaxRequests() const;

private:
    /**
     * @brief Executes the blocking HTTP serving loop on the background thread.
     */
    void _run();
    /**
     * @brief Sets the latest error text and lifecycle state in one place.
     */
    void _fail(const std::string& errorMessage);
    /**
     * @brief Records a lifecycle event in the bounded in-memory log.
     */
    void _recordEventLocked(const std::string& message);
    /**
     * @brief Records an error event in the bounded in-memory error history.
     */
    void _recordErrorLocked(const std::string& message);
    /**
     * @brief Computes a human-readable status message from the current lifecycle state.
     */
    std::string _statusMessageLocked() const;
    /**
     * @brief Returns a fresh simulator instance for the web session manager.
     */
    static std::unique_ptr<Simulator> _createSimulator();

    mutable std::mutex _mutex;
    std::atomic<bool> _stopRequested{false};
    std::atomic<bool> _threadRunning{false};
    std::atomic<bool> _starting{false};
    std::atomic<bool> _stopping{false};
    std::atomic<unsigned long> _servedRequests{0};
    std::atomic<unsigned long> _activeRequests{0};
    unsigned short _configuredPort = 8080;
    unsigned long _configuredMaxRequests = 0;
    std::string _lastError;
    std::string _statusMessage;
    std::deque<std::string> _recentEvents;
    std::deque<std::string> _recentErrors;
    std::thread _thread;

    std::unique_ptr<TokenService> _tokenService;
    std::unique_ptr<SessionManager> _sessionManager;
    std::unique_ptr<WorkerJobManager> _workerJobManager;
    std::unique_ptr<SimulatorSessionService> _simulatorSessionService;
    std::unique_ptr<ApiRouter> _router;
    std::unique_ptr<SimpleHttpServer> _server;
};
