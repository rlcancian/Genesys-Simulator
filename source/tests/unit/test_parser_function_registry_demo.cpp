#include <gtest/gtest.h>
#include <string>

#include "fakes/FakePlugin.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ParserDefaultImpl2.h"
#include "kernel/simulator/Simulator.h"
#include "parser/FunctionRegistry.h"

namespace {

double parseWithRegistry(Model* model, FunctionRegistry& registry, const std::string& expression, bool& success, std::string& errorMessage) {
	ParserDefaultImpl2 parser(model, nullptr, false);
	parser.setFunctionRegistry(&registry);
	return parser.parse(expression, success, errorMessage);
}

class ParserFunctionRegistryDemoTest : public ::testing::Test {
protected:
	Simulator simulator;
	Model* model = nullptr;

	void SetUp() override {
		model = simulator.getModelManager()->newModel();
		ASSERT_NE(model, nullptr);
	}
};

}

TEST_F(ParserFunctionRegistryDemoTest, FakeAddIsUnavailableBeforeFakePluginRegistration) {
	FunctionRegistry registry;
	bool success = true;
	std::string errorMessage;

	(void)parseWithRegistry(model, registry, "FakeAdd(2,3)", success, errorMessage);

	EXPECT_FALSE(success);
	EXPECT_FALSE(errorMessage.empty());
	EXPECT_NE(errorMessage.find("FakeAdd"), std::string::npos);
	EXPECT_NE(errorMessage.find("function is not registered"), std::string::npos);
}

TEST_F(ParserFunctionRegistryDemoTest, FakeAddIsResolvedAfterFakePluginRegistration) {
	FunctionRegistry registry;
	const FunctionRegistrationResult registration = FakePlugin::registerFunctions(registry);
	ASSERT_TRUE(registration.success) << registration.errorMessage;

	bool success = false;
	std::string errorMessage;
	const double result = parseWithRegistry(model, registry, "FakeAdd(2,3)", success, errorMessage);

	EXPECT_TRUE(success);
	EXPECT_TRUE(errorMessage.empty());
	EXPECT_DOUBLE_EQ(result, 5.0);
}

TEST_F(ParserFunctionRegistryDemoTest, FakePluginRegisteredFunctionReportsWrongArity) {
	FunctionRegistry registry;
	const FunctionRegistrationResult registration = FakePlugin::registerFunctions(registry);
	ASSERT_TRUE(registration.success) << registration.errorMessage;

	bool success = true;
	std::string errorMessage;

	(void)parseWithRegistry(model, registry, "FakeAdd(1)", success, errorMessage);

	EXPECT_FALSE(success);
	EXPECT_FALSE(errorMessage.empty());
	EXPECT_NE(errorMessage.find("FakeAdd"), std::string::npos);
	EXPECT_NE(errorMessage.find("expected 2"), std::string::npos);
	EXPECT_NE(errorMessage.find("FakePlugin"), std::string::npos);
}

TEST_F(ParserFunctionRegistryDemoTest, MissingFunctionReportsControlledErrorWithRegisteredFunctions) {
	FunctionRegistry registry;
	const FunctionRegistrationResult registration = FakePlugin::registerFunctions(registry);
	ASSERT_TRUE(registration.success) << registration.errorMessage;

	bool success = true;
	std::string errorMessage;

	(void)parseWithRegistry(model, registry, "FuncaoInexistente(2)", success, errorMessage);

	EXPECT_FALSE(success);
	EXPECT_FALSE(errorMessage.empty());
	EXPECT_NE(errorMessage.find("FuncaoInexistente"), std::string::npos);
	EXPECT_NE(errorMessage.find("function is not registered"), std::string::npos);
	EXPECT_NE(errorMessage.find("FakeAdd"), std::string::npos);
}
