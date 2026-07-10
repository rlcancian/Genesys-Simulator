#include "GuiScopeTrace.h"

#include <QDebug>
#include <QThread>

namespace {
thread_local int g_guiTraceDepth = 0;
}

GuiScopeTrace::GuiScopeTrace(const char* methodName, const void* thisPtr)
    : _methodName(methodName),
      _thisPtr(thisPtr),
      _depth(g_guiTraceDepth),
      _start(std::chrono::steady_clock::now()) {
    // Logs structured ENTER trace with thread id, object pointer, and nesting depth.
    qInfo().nospace() << "GUI TRACE ENTER method=" << _methodName
                      << " this=" << _thisPtr
                      << " tid=" << reinterpret_cast<quintptr>(QThread::currentThreadId())
                      << " depth=" << _depth;
    ++g_guiTraceDepth;
}

GuiScopeTrace::~GuiScopeTrace() {
    --g_guiTraceDepth;
    const auto elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - _start).count();
    // Logs structured EXIT trace with elapsed time for scoped execution diagnostics.
    qInfo().nospace() << "GUI TRACE EXIT method=" << _methodName
                      << " this=" << _thisPtr
                      << " tid=" << reinterpret_cast<quintptr>(QThread::currentThreadId())
                      << " depth=" << _depth
                      << " elapsed_us=" << elapsedUs;
}
