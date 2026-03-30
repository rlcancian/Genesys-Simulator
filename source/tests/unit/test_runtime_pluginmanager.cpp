#include <gtest/gtest.h>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/PluginManager.h"

TEST(RuntimePluginManagerClassTest, SimulatorProvidesPluginManagerWithDefaultPlugins) {
    Simulator simulator;

    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    EXPECT_GE(manager->size(), 4u);
    EXPECT_NE(manager->front(), nullptr);
}
