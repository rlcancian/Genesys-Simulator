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

TEST(RuntimePluginManagerClassTest, InsertReturnsNullptrAndDoesNotChangeSizeWhenLibraryIsMissing) {
    Simulator simulator;

    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);

    const auto before = manager->size();
    Plugin* inserted = manager->insert("definitely_missing_plugin_library.so");

    EXPECT_EQ(inserted, nullptr);
    EXPECT_EQ(manager->size(), before);
}
