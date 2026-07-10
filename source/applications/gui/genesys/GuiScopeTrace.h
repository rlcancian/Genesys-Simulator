#ifndef GUISCOPETRACE_H
#define GUISCOPETRACE_H

#include <chrono>

/**
 * @brief Emits a scoped trace entry for a GUI method and measures its runtime.
 *
 * Instances are usually created on the stack at the beginning of a method so
 * the constructor logs entry and the destructor logs the elapsed time when the
 * scope exits.
 */
class GuiScopeTrace {
public:
    /**
     * @brief Starts a scoped GUI trace entry for a critical method invocation.
     * @param methodName Name of the traced method.
     * @param thisPtr Pointer to the current object, when available.
     */
    GuiScopeTrace(const char* methodName, const void* thisPtr);
    /**
     * @brief Ends the scoped trace and logs the elapsed execution time.
     */
    ~GuiScopeTrace();

private:
    /// Method name associated with the trace entry.
    const char* _methodName;
    /// Optional object pointer used to disambiguate the traced instance.
    const void* _thisPtr;
    /// Nesting depth used by the tracing output formatter.
    int _depth;
    /// Timestamp captured at scope entry.
    std::chrono::steady_clock::time_point _start;
};

#endif // GUISCOPETRACE_H
