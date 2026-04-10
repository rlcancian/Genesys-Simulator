#ifndef GUISCOPETRACE_H
#define GUISCOPETRACE_H

#include <chrono>

class GuiScopeTrace {
public:
    // Starts a scoped GUI trace entry for a critical method invocation.
    GuiScopeTrace(const char* methodName, const void* thisPtr);
    // Ends a scoped GUI trace and logs elapsed execution time.
    ~GuiScopeTrace();

private:
    const char* _methodName;
    const void* _thisPtr;
    int _depth;
    std::chrono::steady_clock::time_point _start;
};

#endif // GUISCOPETRACE_H
