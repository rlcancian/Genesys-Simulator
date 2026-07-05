#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <vector>

#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "parser/FunctionRegistry.h"

namespace {

struct RealComponentPluginCase {
	std::string filename;
	std::string pluginTypename;
	std::string category;
};

const std::vector<RealComponentPluginCase>& representativeComponentPlugins() {
	static const std::vector<RealComponentPluginCase> plugins = {
		{"delay.so", "Delay", "DiscreteProcessing"},
		{"dispose.so", "Dispose", "Logic"},
		{"separate.so", "Separate", "Grouping"},
		{"record.so", "Record", "InputOutput"},
		{"exit.so", "Exit", "MaterialHandling"}
	};
	return plugins;
}

std::size_t countFunctionsFromOrigin(FunctionRegistry* registry, const std::string& originName) {
	if (registry == nullptr) {
		return 0;
	}
	std::size_t count = 0;
	for (const FunctionDescriptor& descriptor : registry->listFunctions()) {
		if (descriptor.originName == originName) {
			++count;
		}
	}
	return count;
}

double parse(Model* model, const std::string& expression, bool& success, std::string& errorMessage) {
	errorMessage.clear();
	return model->parseExpression(expression, success, errorMessage);
}

class PluginManagerRealComponentsTest : public ::testing::Test {
protected:
	Simulator simulator;
	Model* model = nullptr;
	PluginManager* pluginManager = nullptr;
	FunctionRegistry* registry = nullptr;

	void SetUp() override {
		model = simulator.getModelManager()->newModel();
		ASSERT_NE(model, nullptr);
		pluginManager = simulator.getPluginManager();
		ASSERT_NE(pluginManager, nullptr);
		registry = simulator.getFunctionRegistry();
		ASSERT_NE(registry, nullptr);
	}

	std::vector<Plugin*> insertRepresentativePlugins() {
		std::vector<Plugin*> insertedPlugins;
		for (const RealComponentPluginCase& pluginCase : representativeComponentPlugins()) {
			Plugin* plugin = pluginManager->insert(pluginCase.filename);
			EXPECT_NE(plugin, nullptr) << pluginCase.filename;
			if (plugin != nullptr) {
				insertedPlugins.push_back(plugin);
			}
		}
		return insertedPlugins;
	}

	void expectLegacyParserExpressionsWork() {
		bool success = false;
		std::string errorMessage;

		EXPECT_DOUBLE_EQ(parse(model, "2+3", success, errorMessage), 5.0);
		EXPECT_TRUE(success) << errorMessage;

		EXPECT_DOUBLE_EQ(parse(model, "10/2", success, errorMessage), 5.0);
		EXPECT_TRUE(success) << errorMessage;

		EXPECT_DOUBLE_EQ(parse(model, "(2+3)*4", success, errorMessage), 20.0);
		EXPECT_TRUE(success) << errorMessage;
	}
};

}

TEST_F(PluginManagerRealComponentsTest, LoadsAndFindsRepresentativeRealComponentPlugins) {
	const std::size_t initialRegistrySize = registry->listFunctions().size();

	insertRepresentativePlugins();

	for (const RealComponentPluginCase& pluginCase : representativeComponentPlugins()) {
		Plugin* plugin = pluginManager->find(pluginCase.pluginTypename);
		ASSERT_NE(plugin, nullptr) << pluginCase.pluginTypename;
		ASSERT_NE(plugin->getPluginInfo(), nullptr) << pluginCase.pluginTypename;
		EXPECT_EQ(plugin->getPluginInfo()->getCategory(), pluginCase.category) << pluginCase.pluginTypename;
		EXPECT_FALSE(plugin->getPluginInfo()->hasParserFunctions()) << pluginCase.pluginTypename;
	}
	EXPECT_EQ(registry->listFunctions().size(), initialRegistrySize);
}

TEST_F(PluginManagerRealComponentsTest, LegacyParserStillWorksAfterLoadingRealComponents) {
	expectLegacyParserExpressionsWork();

	insertRepresentativePlugins();

	expectLegacyParserExpressionsWork();
}

TEST_F(PluginManagerRealComponentsTest, RealComponentsWithoutParserFunctionsDoNotPolluteRegistry) {
	const std::size_t initialRegistrySize = registry->listFunctions().size();

	insertRepresentativePlugins();

	EXPECT_EQ(registry->listFunctions().size(), initialRegistrySize);
	for (const RealComponentPluginCase& pluginCase : representativeComponentPlugins()) {
		EXPECT_EQ(countFunctionsFromOrigin(registry, pluginCase.pluginTypename), 0u) << pluginCase.pluginTypename;
	}
}

TEST_F(PluginManagerRealComponentsTest, UnloadingRealComponentsLeavesRegistryAndParserConsistent) {
	const std::size_t initialRegistrySize = registry->listFunctions().size();
	std::vector<Plugin*> insertedPlugins = insertRepresentativePlugins();
	ASSERT_EQ(insertedPlugins.size(), representativeComponentPlugins().size());

	for (auto it = insertedPlugins.rbegin(); it != insertedPlugins.rend(); ++it) {
		Plugin* plugin = *it;
		ASSERT_NE(plugin, nullptr);
		ASSERT_NE(plugin->getPluginInfo(), nullptr);
		const std::string pluginTypename = plugin->getPluginInfo()->getPluginTypename();

		ASSERT_TRUE(pluginManager->remove(plugin)) << pluginTypename;
		EXPECT_EQ(pluginManager->find(pluginTypename), nullptr) << pluginTypename;
		EXPECT_EQ(countFunctionsFromOrigin(registry, pluginTypename), 0u) << pluginTypename;
		expectLegacyParserExpressionsWork();
	}

	EXPECT_EQ(registry->listFunctions().size(), initialRegistrySize);
}

TEST_F(PluginManagerRealComponentsTest, RemoveByDynamicLibraryFilenameUnloadsRealComponentAndKeepsRegistryConsistent) {
	const std::size_t initialRegistrySize = registry->listFunctions().size();
	std::vector<Plugin*> insertedPlugins = insertRepresentativePlugins();
	ASSERT_EQ(insertedPlugins.size(), representativeComponentPlugins().size());

	const RealComponentPluginCase& removedCase = representativeComponentPlugins().front();
	ASSERT_NE(pluginManager->find(removedCase.pluginTypename), nullptr);

	ASSERT_TRUE(pluginManager->remove(removedCase.filename)) << removedCase.filename;
	EXPECT_EQ(pluginManager->find(removedCase.pluginTypename), nullptr) << removedCase.pluginTypename;
	EXPECT_EQ(countFunctionsFromOrigin(registry, removedCase.pluginTypename), 0u) << removedCase.pluginTypename;
	EXPECT_EQ(registry->listFunctions().size(), initialRegistrySize);
	expectLegacyParserExpressionsWork();

	for (std::size_t index = 1; index < representativeComponentPlugins().size(); ++index) {
		const RealComponentPluginCase& pluginCase = representativeComponentPlugins()[index];
		Plugin* plugin = pluginManager->find(pluginCase.pluginTypename);
		ASSERT_NE(plugin, nullptr) << pluginCase.pluginTypename;
		EXPECT_EQ(countFunctionsFromOrigin(registry, pluginCase.pluginTypename), 0u) << pluginCase.pluginTypename;
	}

	for (std::size_t index = 1; index < insertedPlugins.size(); ++index) {
		Plugin* plugin = insertedPlugins[index];
		ASSERT_NE(plugin, nullptr);
		ASSERT_NE(plugin->getPluginInfo(), nullptr);
		const std::string pluginTypename = plugin->getPluginInfo()->getPluginTypename();
		ASSERT_TRUE(pluginManager->remove(plugin)) << pluginTypename;
	}

	EXPECT_EQ(registry->listFunctions().size(), initialRegistrySize);
	expectLegacyParserExpressionsWork();
}
