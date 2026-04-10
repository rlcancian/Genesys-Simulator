#include "GuiCrashDiagnostics.h"

#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <cstdlib>

namespace {

// Writes a NUL-terminated C string to stderr in a signal handler context.
void writeStderr(const char* text) {
    if (text == nullptr) {
        return;
    }
    const char* p = text;
    while (*p != '\0') {
        ++p;
    }
    ::write(STDERR_FILENO, text, static_cast<size_t>(p - text));
}

// Writes an integer value to stderr without using heap allocations.
void writeInt(int value) {
    char buffer[32];
    int idx = 0;
    if (value == 0) {
        buffer[idx++] = '0';
    } else {
        if (value < 0) {
            buffer[idx++] = '-';
            value = -value;
        }
        char digits[16];
        int didx = 0;
        while (value > 0 && didx < 16) {
            digits[didx++] = static_cast<char>('0' + (value % 10));
            value /= 10;
        }
        while (didx > 0) {
            buffer[idx++] = digits[--didx];
        }
    }
    ::write(STDERR_FILENO, buffer, static_cast<size_t>(idx));
}

// Handles fatal GUI signals and prints a backtrace for crash diagnosis.
void fatalSignalHandler(int signalNumber) {
    void* trace[128];
    const int traceSize = ::backtrace(trace, 128);

    writeStderr("\n=== GUI CRASH ===\n");
    writeStderr("Signal: ");
    writeInt(signalNumber);
    writeStderr("\nBacktrace:\n");
    ::backtrace_symbols_fd(trace, traceSize, STDERR_FILENO);

    std::_Exit(128 + signalNumber);
}

// Registers one POSIX signal to the temporary GUI diagnostic handler.
void registerSignal(int signalNumber) {
    struct sigaction action;
    action.sa_handler = fatalSignalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESETHAND;
    sigaction(signalNumber, &action, nullptr);
}

} // namespace

namespace GuiCrashDiagnostics {

void installFatalSignalHandlers() {
    // Installs temporary handlers for the most common fatal crash signals.
    registerSignal(SIGSEGV);
    registerSignal(SIGABRT);
    registerSignal(SIGBUS);
    registerSignal(SIGILL);
    registerSignal(SIGFPE);
}

} // namespace GuiCrashDiagnostics
