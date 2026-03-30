#include <gtest/gtest.h>

#include "kernel/simulator/TraceManager.h"

namespace {
TraceManager::Level g_last_trace_error_level = TraceManager::Level::L9_mostDetailed;

void CaptureTraceErrorLevel(TraceErrorEvent e) {
    g_last_trace_error_level = e.getTracelevel();
}
}

TEST(SupportTraceManagerClassTest, InitializesErrorMessagesList) {
    TraceManager tm(nullptr);
    ASSERT_NE(tm.errorMessages(), nullptr);
}

TEST(SupportTraceManagerClassTest, TraceErrorPreservesExplicitLevel) {
    TraceManager tm(nullptr);
    tm.addTraceErrorHandler(&CaptureTraceErrorLevel);

    g_last_trace_error_level = TraceManager::Level::L9_mostDetailed;
    tm.traceError("fatal", TraceManager::Level::L1_errorFatal);

    EXPECT_EQ(g_last_trace_error_level, TraceManager::Level::L1_errorFatal);
}
