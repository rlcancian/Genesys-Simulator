#include <gtest/gtest.h>

#include <stdexcept>

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

TEST(SupportTraceManagerClassTest, TraceErrorStoresMessageInErrorList) {
    TraceManager tm(nullptr);

    tm.traceError("tracked-error", TraceManager::Level::L3_errorRecover);

    ASSERT_NE(tm.errorMessages(), nullptr);
    ASSERT_EQ(tm.errorMessages()->size(), 1u);
    EXPECT_NE(tm.errorMessages()->front().find("tracked-error"), std::string::npos);
}

TEST(SupportTraceManagerClassTest, TraceErrorWithExceptionStoresMessageInErrorList) {
    TraceManager tm(nullptr);

    std::runtime_error ex("boom");
    tm.traceError("exception-error", ex);

    ASSERT_NE(tm.errorMessages(), nullptr);
    ASSERT_EQ(tm.errorMessages()->size(), 1u);
    EXPECT_NE(tm.errorMessages()->front().find("exception-error"), std::string::npos);
}
