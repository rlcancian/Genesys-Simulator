#include "WebWorkerRuntime.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <utility>

namespace {
constexpr std::size_t kMaxHistoryEntries = 12;

class RequestCounterGuard {
public:
    explicit RequestCounterGuard(std::atomic<unsigned long>& activeRequests)
        : _activeRequests(activeRequests) {
        _activeRequests.fetch_add(1);
    }

    ~RequestCounterGuard() {
        _activeRequests.fetch_sub(1);
    }

private:
    std::atomic<unsigned long>& _activeRequests;
};

std::string formatTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm localTime{};
#if defined(_WIN32)
    localtime_s(&localTime, &nowTime);
#else
    localtime_r(&nowTime, &localTime);
#endif
    std::ostringstream stream;
    stream << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return stream.str();
}

std::string makeHistoryLine(const std::string& message) {
    return "[" + formatTimestamp() + "] " + message;
}
}  // namespace

WebWorkerRuntime::WebWorkerRuntime(unsigned short defaultPort, unsigned long maxRequests)
    : _configuredPort(defaultPort), _configuredMaxRequests(maxRequests) {
    _tokenService = std::make_unique<TokenService>();
    _workerJobManager = std::make_unique<WorkerJobManager>();
    _sessionManager = std::make_unique<SessionManager>(*_tokenService, []() {
        return _createSimulator();
    });
    _simulatorSessionService = std::make_unique<SimulatorSessionService>(*_sessionManager, *_workerJobManager);
    _router = std::make_unique<ApiRouter>(*_simulatorSessionService);
    _statusMessage = makeHistoryLine("Web worker runtime ready");
    _recordEventLocked(_statusMessage);
}

WebWorkerRuntime::~WebWorkerRuntime() {
    stop();
}

bool WebWorkerRuntime::start() {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_threadRunning.load() || _starting.load() || _stopping.load()) {
        return false;
    }

    if (_thread.joinable()) {
        _thread.join();
    }

    _stopRequested.store(false);
    _servedRequests.store(0);
    _activeRequests.store(0);
    _lastError.clear();
    _statusMessage = makeHistoryLine("Starting web worker on port " + std::to_string(_configuredPort));
    _recordEventLocked(_statusMessage);
    _server = std::make_unique<SimpleHttpServer>(_configuredPort);
    _starting.store(true);
    _thread = std::thread([this]() { _run(); });
    return true;
}

void WebWorkerRuntime::stop() {
    bool shouldJoin = false;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_threadRunning.load() && !_starting.load()) {
            shouldJoin = _thread.joinable();
            _server.reset();
            _stopping.store(false);
        } else {
            _stopping.store(true);
            _stopRequested.store(true);
            _statusMessage = makeHistoryLine("Stop requested by GUI");
            _recordEventLocked(_statusMessage);
            if (_server != nullptr) {
                _server->requestStop();
            }
        }
    }

    if (shouldJoin || _thread.joinable()) {
        _thread.join();
    }

    std::lock_guard<std::mutex> lock(_mutex);
    _server.reset();
    _threadRunning.store(false);
    _starting.store(false);
    _stopping.store(false);
}

void WebWorkerRuntime::wait() {
    if (_thread.joinable() && std::this_thread::get_id() != _thread.get_id()) {
        _thread.join();
        std::lock_guard<std::mutex> lock(_mutex);
        _server.reset();
    }
}

bool WebWorkerRuntime::restart() {
    stop();
    return start();
}

bool WebWorkerRuntime::isRunning() const {
    return _threadRunning.load();
}

WebWorkerRuntime::Snapshot WebWorkerRuntime::snapshot() const {
    std::lock_guard<std::mutex> lock(_mutex);
    Snapshot snapshot{};
    snapshot.isRunning = _threadRunning.load();
    snapshot.isStarting = _starting.load();
    snapshot.isStopping = _stopping.load();
    snapshot.hasFailed = !_lastError.empty() && !_threadRunning.load() && !_starting.load() && !_stopping.load();
    snapshot.port = _configuredPort;
    snapshot.maxRequests = _configuredMaxRequests;
    snapshot.servedRequests = _servedRequests.load();
    snapshot.activeRequests = _activeRequests.load();
    snapshot.statusMessage = _statusMessage.empty() ? _statusMessageLocked() : _statusMessage;
    snapshot.lastError = _lastError;
    snapshot.recentEvents.assign(_recentEvents.begin(), _recentEvents.end());
    snapshot.recentErrors.assign(_recentErrors.begin(), _recentErrors.end());
    return snapshot;
}

void WebWorkerRuntime::setConfiguredPort(unsigned short port) {
    std::lock_guard<std::mutex> lock(_mutex);
    _configuredPort = port;
}

unsigned short WebWorkerRuntime::configuredPort() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _configuredPort;
}

void WebWorkerRuntime::setConfiguredMaxRequests(unsigned long maxRequests) {
    std::lock_guard<std::mutex> lock(_mutex);
    _configuredMaxRequests = maxRequests;
}

unsigned long WebWorkerRuntime::configuredMaxRequests() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _configuredMaxRequests;
}

void WebWorkerRuntime::_run() {
    const unsigned long maxRequests = [&]() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _configuredMaxRequests;
    }();

    _threadRunning.store(true);
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _statusMessage = makeHistoryLine("Web worker is listening on port " + std::to_string(_configuredPort));
        _recordEventLocked(_statusMessage);
    }
    bool ok = false;
    try {
        ok = _server->serve([this](const HttpRequest& request) -> HttpResponse {
            RequestCounterGuard activeGuard(_activeRequests);
            HttpResponse response = _router->handle(request);
            _servedRequests.fetch_add(1);
            return response;
        }, maxRequests);
    } catch (const std::exception& e) {
        _fail(e.what());
        return;
    } catch (...) {
        _fail("Unexpected web worker failure");
        return;
    }

    if (_stopRequested.load()) {
        std::lock_guard<std::mutex> lock(_mutex);
        _statusMessage = makeHistoryLine("Web worker stopped after a shutdown request");
        _recordEventLocked(_statusMessage);
        _threadRunning.store(false);
        _starting.store(false);
        _stopping.store(false);
        return;
    }

    if (!ok) {
        _fail("Failed to bind or start the HTTP server");
        std::lock_guard<std::mutex> lock(_mutex);
        _threadRunning.store(false);
        _starting.store(false);
        _stopping.store(false);
        return;
    }

    std::lock_guard<std::mutex> lock(_mutex);
    _statusMessage = makeHistoryLine("Web worker stopped normally");
    _recordEventLocked(_statusMessage);
    _threadRunning.store(false);
    _starting.store(false);
    _stopping.store(false);
    _lastError.clear();
}

void WebWorkerRuntime::_fail(const std::string& errorMessage) {
    std::lock_guard<std::mutex> lock(_mutex);
    _lastError = errorMessage;
    _statusMessage = makeHistoryLine(errorMessage);
    _recordErrorLocked(_statusMessage);
    _threadRunning.store(false);
    _starting.store(false);
}

void WebWorkerRuntime::_recordEventLocked(const std::string& message) {
    _recentEvents.push_back(message);
    while (_recentEvents.size() > kMaxHistoryEntries) {
        _recentEvents.pop_front();
    }
}

void WebWorkerRuntime::_recordErrorLocked(const std::string& message) {
    _recentErrors.push_back(message);
    while (_recentErrors.size() > kMaxHistoryEntries) {
        _recentErrors.pop_front();
    }
}

std::string WebWorkerRuntime::_statusMessageLocked() const {
    if (_stopping.load()) {
        return makeHistoryLine("Stopping web worker");
    }
    if (_starting.load()) {
        return makeHistoryLine("Starting web worker");
    }
    if (_threadRunning.load()) {
        return makeHistoryLine("Web worker is running");
    }
    if (!_lastError.empty()) {
        return makeHistoryLine(_lastError);
    }
    return makeHistoryLine("Web worker is stopped");
}

std::unique_ptr<Simulator> WebWorkerRuntime::_createSimulator() {
    auto simulator = std::make_unique<Simulator>();
    // Register the built-in component plugins so that language-imported models can resolve
    // their component types by name. The dummy plugin connector maps names to compiled-in
    // classes, so this works in the static build without loading any .so file.
    simulator->getPluginManager()->autoInsertPlugins();
    return simulator;
}
