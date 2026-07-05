#include <gtest/gtest.h>
#include <map>
#include <string>
#include <vector>

#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"

namespace {

PluginInformation* BuildParserFunctionPlugin() {
	auto* info = new PluginInformation(
		"ParserFunctionPlugin",
		static_cast<StaticLoaderDataDefinitionInstance>(nullptr),
		static_cast<StaticConstructorDataDefinitionInstance>(nullptr));
	info->setCategory("Test");
	info->insertParserFunction(
		{"PluginAdd", 2, 2, "", "Adds two numeric arguments", "test"},
		[](const std::vector<double>& arguments) {
			return arguments[0] + arguments[1];
		});
	return info;
}

PluginInformation* BuildConflictingParserFunctionPlugin() {
	auto* info = new PluginInformation(
		"ConflictingParserFunctionPlugin",
		static_cast<StaticLoaderDataDefinitionInstance>(nullptr),
		static_cast<StaticConstructorDataDefinitionInstance>(nullptr));
	info->setCategory("Test");
	info->insertParserFunction(
		{"pluginadd", 2, 2, "", "Conflicts with ParserFunctionPlugin", "test"},
		[](const std::vector<double>& arguments) {
			return arguments[0] - arguments[1];
		});
	return info;
}

class ParserFunctionPluginConnector : public PluginConnector_if {
public:
	Plugin* check(const std::string dynamicLibraryFilename) override {
		return create(dynamicLibraryFilename);
	}

	Plugin* connect(const std::string dynamicLibraryFilename) override {
		++connectCalls[dynamicLibraryFilename];
		return create(dynamicLibraryFilename);
	}

	List<std::string>* find() override {
		auto* filenames = new List<std::string>();
		filenames->insert("parser_function_plugin.so");
		filenames->insert("conflicting_parser_function_plugin.so");
		return filenames;
	}

	bool disconnect(const std::string dynamicLibraryFilename) override {
		++disconnectByFilenameCalls[dynamicLibraryFilename];
		return true;
	}

	bool disconnect(Plugin* plugin) override {
		if (plugin != nullptr && plugin->getPluginInfo() != nullptr) {
			++disconnectByPluginCalls[plugin->getPluginInfo()->getPluginTypename()];
		}
		return true;
	}

	std::map<std::string, unsigned int> connectCalls;
	std::map<std::string, unsigned int> disconnectByFilenameCalls;
	std::map<std::string, unsigned int> disconnectByPluginCalls;

private:
	Plugin* create(const std::string& dynamicLibraryFilename) {
		if (dynamicLibraryFilename == "parser_function_plugin.so") {
			return new Plugin(&BuildParserFunctionPlugin);
		}
		if (dynamicLibraryFilename == "conflicting_parser_function_plugin.so") {
			return new Plugin(&BuildConflictingParserFunctionPlugin);
		}
		return nullptr;
	}
};

class ParserFunctionNoopCommandExecutor : public SystemCommandExecutor_if {
public:
	SystemCommandResult run(const std::string&) override {
		return {};
	}
};

double parse(Model* model, const std::string& expression, bool& success, std::string& errorMessage) {
	errorMessage.clear();
	return model->parseExpression(expression, success, errorMessage);
}

class ParserFunctionRegistryDemoTest : public ::testing::Test {
protected:
	Simulator simulator;
	Model* model = nullptr;
	ParserFunctionPluginConnector* connector = nullptr;
	PluginManager* pluginManager = nullptr;

	void SetUp() override {
		model = simulator.getModelManager()->newModel();
		ASSERT_NE(model, nullptr);
		connector = new ParserFunctionPluginConnector();
		pluginManager = new PluginManager(&simulator, connector, new ParserFunctionNoopCommandExecutor());
	}

	void TearDown() override {
		delete pluginManager;
		pluginManager = nullptr;
		connector = nullptr;
	}
};

}

TEST_F(ParserFunctionRegistryDemoTest, PluginFunctionIsUnavailableBeforePluginManagerInsertion) {
	bool success = true;
	std::string errorMessage;

	(void)parse(model, "PluginAdd(2,3)", success, errorMessage);

	EXPECT_FALSE(success);
	EXPECT_FALSE(errorMessage.empty());
	EXPECT_NE(errorMessage.find("PluginAdd"), std::string::npos);
	EXPECT_NE(errorMessage.find("function is not registered"), std::string::npos);
}

TEST_F(ParserFunctionRegistryDemoTest, PluginManagerInsertionRegistersPluginFunctionForModelParser) {
	Plugin* inserted = pluginManager->insert("parser_function_plugin.so");
	ASSERT_NE(inserted, nullptr);

	bool success = false;
	std::string errorMessage;
	const double result = parse(model, "PluginAdd(2,3)", success, errorMessage);

	EXPECT_TRUE(success);
	EXPECT_TRUE(errorMessage.empty());
	EXPECT_DOUBLE_EQ(result, 5.0);

	const FunctionDescriptor* descriptor = simulator.getFunctionRegistry()->lookup("PluginAdd");
	ASSERT_NE(descriptor, nullptr);
	EXPECT_EQ(descriptor->originName, "ParserFunctionPlugin");
}

TEST_F(ParserFunctionRegistryDemoTest, PluginManagerRemovalUnregistersPluginFunction) {
	Plugin* inserted = pluginManager->insert("parser_function_plugin.so");
	ASSERT_NE(inserted, nullptr);

	bool success = false;
	std::string errorMessage;
	EXPECT_DOUBLE_EQ(parse(model, "PluginAdd(2,3)", success, errorMessage), 5.0);
	ASSERT_TRUE(success) << errorMessage;

	ASSERT_TRUE(pluginManager->remove(inserted));

	success = true;
	errorMessage.clear();
	(void)parse(model, "PluginAdd(2,3)", success, errorMessage);

	EXPECT_FALSE(success);
	EXPECT_FALSE(errorMessage.empty());
	EXPECT_NE(errorMessage.find("PluginAdd"), std::string::npos);
	EXPECT_NE(errorMessage.find("function is not registered"), std::string::npos);
	EXPECT_EQ(simulator.getFunctionRegistry()->lookup("PluginAdd"), nullptr);
}

TEST_F(ParserFunctionRegistryDemoTest, PluginManagerRemovalByDynamicLibraryFilenameUnregistersPluginFunction) {
	Plugin* inserted = pluginManager->insert("parser_function_plugin.so");
	ASSERT_NE(inserted, nullptr);
	ASSERT_NE(pluginManager->find("ParserFunctionPlugin"), nullptr);

	bool success = false;
	std::string errorMessage;
	EXPECT_DOUBLE_EQ(parse(model, "PluginAdd(2,3)", success, errorMessage), 5.0);
	ASSERT_TRUE(success) << errorMessage;
	ASSERT_NE(simulator.getFunctionRegistry()->lookup("PluginAdd"), nullptr);

	ASSERT_TRUE(pluginManager->remove("parser_function_plugin.so"));

	EXPECT_EQ(pluginManager->find("ParserFunctionPlugin"), nullptr);
	EXPECT_EQ(simulator.getFunctionRegistry()->lookup("PluginAdd"), nullptr);

	success = true;
	errorMessage.clear();
	(void)parse(model, "PluginAdd(2,3)", success, errorMessage);

	EXPECT_FALSE(success);
	EXPECT_FALSE(errorMessage.empty());
	EXPECT_NE(errorMessage.find("PluginAdd"), std::string::npos);
	EXPECT_NE(errorMessage.find("function is not registered"), std::string::npos);
}

TEST_F(ParserFunctionRegistryDemoTest, PluginManagerRejectsConflictingParserFunctionWithoutOverwrite) {
	Plugin* inserted = pluginManager->insert("parser_function_plugin.so");
	ASSERT_NE(inserted, nullptr);

	Plugin* conflicting = pluginManager->insert("conflicting_parser_function_plugin.so");
	EXPECT_EQ(conflicting, nullptr);

	bool success = false;
	std::string errorMessage;
	const double result = parse(model, "PluginAdd(8,3)", success, errorMessage);

	EXPECT_TRUE(success) << errorMessage;
	EXPECT_DOUBLE_EQ(result, 11.0);
	EXPECT_EQ(pluginManager->find("ConflictingParserFunctionPlugin"), nullptr);
	ASSERT_EQ(pluginManager->getPluginLoadIssues()->size(), 1u);
	const PluginLoadIssue issue = pluginManager->getPluginLoadIssues()->front();
	EXPECT_EQ(issue.getReason(), PluginLoadIssue::Reason::InsertionFailure);
	EXPECT_NE(issue.getMessage().find("already registered"), std::string::npos);
}

TEST_F(ParserFunctionRegistryDemoTest, PluginRegisteredFunctionReportsWrongArity) {
	Plugin* inserted = pluginManager->insert("parser_function_plugin.so");
	ASSERT_NE(inserted, nullptr);

	bool success = true;
	std::string errorMessage;

	(void)parse(model, "PluginAdd(1)", success, errorMessage);

	EXPECT_FALSE(success);
	EXPECT_FALSE(errorMessage.empty());
	EXPECT_NE(errorMessage.find("PluginAdd"), std::string::npos);
	EXPECT_NE(errorMessage.find("expected 2"), std::string::npos);
	EXPECT_NE(errorMessage.find("ParserFunctionPlugin"), std::string::npos);
}

TEST_F(ParserFunctionRegistryDemoTest, MissingFunctionReportsControlledErrorWithRegisteredFunctions) {
	Plugin* inserted = pluginManager->insert("parser_function_plugin.so");
	ASSERT_NE(inserted, nullptr);

	bool success = true;
	std::string errorMessage;

	(void)parse(model, "FuncaoInexistente(2)", success, errorMessage);

	EXPECT_FALSE(success);
	EXPECT_FALSE(errorMessage.empty());
	EXPECT_NE(errorMessage.find("FuncaoInexistente"), std::string::npos);
	EXPECT_NE(errorMessage.find("function is not registered"), std::string::npos);
	EXPECT_NE(errorMessage.find("PluginAdd"), std::string::npos);
}
