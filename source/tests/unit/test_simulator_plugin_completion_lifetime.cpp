#include <gtest/gtest.h>

#include "kernel/simulator/Plugin.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/ModelManager.h"
#include "kernel/util/List.h"

#include <memory>

namespace {

bool hasCompletedMetadata(PluginManager* pluginManager) {
    if (pluginManager == nullptr) {
        return false;
    }
    for (unsigned int index = 0; index < pluginManager->size(); ++index) {
        Plugin* plugin = pluginManager->getAtRank(index);
        if (plugin == nullptr || plugin->getPluginInfo() == nullptr) {
            continue;
        }
        PluginInformation* information = plugin->getPluginInfo();
        if (!information->getFields()->empty() || !information->getLanguageTemplate().empty()) {
            return true;
        }
    }
    return false;
}

} // namespace

TEST(SimulatorPluginCompletionLifetimeTest, CompletionDoesNotRegisterOrRetainTemporaryModels) {
    Simulator simulator;
    ASSERT_NE(simulator.getPluginManager(), nullptr);
    ASSERT_NE(simulator.getModelManager(), nullptr);

    const unsigned int initialModelCount = simulator.getModelManager()->size();

    {
        std::unique_ptr<List<Plugin*>> completed(
            simulator.getPluginManager()->completePluginsFieldsAndTemplates());
        ASSERT_NE(completed, nullptr);
        EXPECT_EQ(simulator.getModelManager()->size(), initialModelCount);
        EXPECT_TRUE(hasCompletedMetadata(simulator.getPluginManager()));
    }

    {
        std::unique_ptr<List<Plugin*>> completedAgain(
            simulator.getPluginManager()->completePluginsFieldsAndTemplates());
        ASSERT_NE(completedAgain, nullptr);
        EXPECT_EQ(simulator.getModelManager()->size(), initialModelCount);
    }
}
