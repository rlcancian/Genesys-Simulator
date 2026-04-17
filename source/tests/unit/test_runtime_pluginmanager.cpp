#include <gtest/gtest.h>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/SystemDependencyResolver.h"
#include "plugins/PluginConnectorDummyImpl1.h"

#include <algorithm>
#include <map>
#include <memory>
#include <vector>

namespace {

PluginInformation* BuildPluginWithMissingSystemDependency() {
    auto* info = new PluginInformation(
        "PluginWithMissingSystemDependency",
        static_cast<StaticLoaderDataDefinitionInstance>(nullptr),
        static_cast<StaticConstructorDataDefinitionInstance>(nullptr));
    info->setCategory("Test");
    info->insertSystemDependency(SystemDependency(
        SystemDependency::OS::Any,
        "MissingDependency",
        "install-missing-dependency",
        "check-missing-dependency"));
    return info;
}

SystemCommandResult RuntimeCommandResultWithExitCode(int exitCode) {
    SystemCommandResult result;
    result.started = true;
    result.exitCode = exitCode;
    return result;
}

class RuntimeFakeCommandExecutor : public SystemCommandExecutor_if {
public:
    std::map<std::string, std::vector<SystemCommandResult>> results;
    std::vector<std::string> commands;

    SystemCommandResult run(const std::string& command) override {
        commands.push_back(command);
        auto it = results.find(command);
        if (it == results.end() || it->second.empty()) {
            return {};
        }
        SystemCommandResult result = it->second.front();
        it->second.erase(it->second.begin());
        return result;
    }
};

class RuntimeFakePluginConnector : public PluginConnector_if {
public:
    Plugin* check(const std::string) override {
        return new Plugin(&BuildPluginWithMissingSystemDependency);
    }

    Plugin* connect(const std::string) override {
        return new Plugin(&BuildPluginWithMissingSystemDependency);
    }

    List<std::string>* find() override {
        return new List<std::string>();
    }

    bool disconnect(const std::string) override {
        return true;
    }

    bool disconnect(Plugin* plugin) override {
        delete plugin;
        disconnectedPlugins++;
        return true;
    }

    unsigned int disconnectedPlugins = 0;
};

}

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

TEST(RuntimePluginManagerClassTest, InsertRefusesPluginWhenSystemDependencyIsMissingWithoutConfirmation) {
    Simulator simulator;
    auto* executor = new RuntimeFakeCommandExecutor();
    auto* connector = new RuntimeFakePluginConnector();
    executor->results["check-missing-dependency"] = {RuntimeCommandResultWithExitCode(1)};
    PluginManager manager(&simulator, connector, executor);

    const auto before = manager.size();
    Plugin* inserted = manager.insert("fake_plugin_library.so");

    EXPECT_EQ(inserted, nullptr);
    EXPECT_EQ(manager.size(), before);
    ASSERT_EQ(executor->commands.size(), 1u);
    EXPECT_EQ(executor->commands.front(), "check-missing-dependency");
    EXPECT_EQ(connector->disconnectedPlugins, 1u);
}

TEST(RuntimePluginManagerClassTest, InsertInstallsAndRevalidatesSystemDependencyWhenUserConfirms) {
    Simulator simulator;
    auto* executor = new RuntimeFakeCommandExecutor();
    executor->results["check-missing-dependency"] = {
        RuntimeCommandResultWithExitCode(1),
        RuntimeCommandResultWithExitCode(0)
    };
    executor->results["install-missing-dependency"] = {RuntimeCommandResultWithExitCode(0)};
    PluginManager manager(&simulator, new RuntimeFakePluginConnector(), executor);
    PluginInsertionOptions options;
    bool confirmationAsked = false;
    options.confirmSystemDependencyInstallation = [&confirmationAsked](const SystemDependencyCheckResult& result) {
        confirmationAsked = true;
        EXPECT_FALSE(result.canInsertPlugin());
        return true;
    };

    const auto before = manager.size();
    Plugin* inserted = manager.insert("fake_plugin_library.so", options);

    ASSERT_NE(inserted, nullptr);
    EXPECT_TRUE(confirmationAsked);
    EXPECT_EQ(manager.size(), before + 1);
    ASSERT_EQ(executor->commands.size(), 3u);
    EXPECT_EQ(executor->commands[0], "check-missing-dependency");
    EXPECT_EQ(executor->commands[1], "install-missing-dependency");
    EXPECT_EQ(executor->commands[2], "check-missing-dependency");
}

TEST(RuntimePluginManagerClassTest, DummyConnectorRegistersConcreteModelPlugins) {
    PluginConnectorDummyImpl1 connector;
    std::unique_ptr<List<std::string>> filenames(connector.find());
    ASSERT_NE(filenames, nullptr);

    const std::vector<std::string> expectedPluginFiles = {
        "biosimulatorrunner.so",
        "cellularautomata.so",
        "defaultnode.so",
        "dummyelement.so",
        "old_odeelement.so",
        "petriplace.so",
        "rsimulator.so",
        "rsimulatorrunner.so",
        "submodel.so"
    };

    for (const std::string& filename : expectedPluginFiles) {
        EXPECT_NE(std::find(filenames->list()->begin(), filenames->list()->end(), filename), filenames->list()->end()) << filename;

        std::unique_ptr<Plugin> plugin(connector.connect(filename));
        ASSERT_NE(plugin, nullptr) << filename;
        ASSERT_NE(plugin->getPluginInfo(), nullptr) << filename;
        EXPECT_TRUE(plugin->isIsValidPlugin()) << filename;
        EXPECT_FALSE(plugin->getPluginInfo()->getPluginTypename().empty()) << filename;
    }
}
