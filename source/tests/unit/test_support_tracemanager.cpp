#include <gtest/gtest.h>

#include <stdexcept>
#include <vector>

#include "kernel/simulator/TraceManager.h"

namespace {
TraceManager::Level g_last_trace_error_level = TraceManager::Level::L9_mostDetailed;
std::vector<TraceManager::Level> g_trace_simulation_levels;

void CaptureTraceErrorLevel(TraceErrorEvent e) {
    g_last_trace_error_level = e.getTracelevel();
}

void CaptureTraceSimulationLevel(TraceSimulationEvent e) {
    g_trace_simulation_levels.push_back(e.getTracelevel());
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

TEST(SupportTraceManagerClassTest, SimulationTraceFilteringKeepsInternalDetailHierarchy) {
    TraceManager tm(nullptr);
    tm.addTraceSimulationHandler(&CaptureTraceSimulationLevel);

    g_trace_simulation_levels.clear();
    tm.setTraceLevel(TraceManager::Level::L6_arrival);
    tm.traceSimulation(&tm, "internal", TraceManager::Level::L7_internal);
    tm.traceSimulation(&tm, "detailed", TraceManager::Level::L8_detailed);
    tm.traceSimulation(&tm, "most detailed", TraceManager::Level::L9_mostDetailed);
    EXPECT_TRUE(g_trace_simulation_levels.empty());

    tm.setTraceLevel(TraceManager::Level::L7_internal);
    tm.traceSimulation(&tm, "internal", TraceManager::Level::L7_internal);
    tm.traceSimulation(&tm, "detailed", TraceManager::Level::L8_detailed);
    ASSERT_EQ(g_trace_simulation_levels.size(), 1u);
    EXPECT_EQ(g_trace_simulation_levels.back(), TraceManager::Level::L7_internal);

    tm.setTraceLevel(TraceManager::Level::L8_detailed);
    tm.traceSimulation(&tm, "detailed", TraceManager::Level::L8_detailed);
    ASSERT_EQ(g_trace_simulation_levels.size(), 2u);
    EXPECT_EQ(g_trace_simulation_levels.back(), TraceManager::Level::L8_detailed);

    tm.setTraceLevel(TraceManager::Level::L9_mostDetailed);
    tm.traceSimulation(&tm, "most detailed", TraceManager::Level::L9_mostDetailed);
    ASSERT_EQ(g_trace_simulation_levels.size(), 3u);
    EXPECT_EQ(g_trace_simulation_levels.back(), TraceManager::Level::L9_mostDetailed);
}
