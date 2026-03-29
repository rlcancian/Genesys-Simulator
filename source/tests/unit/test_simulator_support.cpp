#include <gtest/gtest.h>

#include "kernel/simulator/TraceManager.h"
#include "kernel/simulator/ParserManager.h"
#include "kernel/simulator/LicenceManager.h"

TEST(SimulatorSupportTest, TraceManagerInitializesErrorMessagesList) {
    TraceManager tm(nullptr);
    ASSERT_NE(tm.errorMessages(), nullptr);
}

TEST(SimulatorSupportTest, ParserManagerCanBeConstructed) {
    ParserManager pm;
    SUCCEED();
}

TEST(SimulatorSupportTest, LicenceManagerCanBeConstructedWithoutSimulatorUse) {
    LicenceManager lm(nullptr);
    EXPECT_FALSE(lm.showLicence().empty());
}
