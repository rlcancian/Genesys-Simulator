#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "parser/Genesys++-driver.h"
#include "parser/FunctionRegistry.h"

namespace {

FunctionDescriptor descriptor(const std::string& name, std::size_t minArity, std::size_t maxArity) {
	return FunctionDescriptor{
		name,
		minArity,
		maxArity,
		"DCS test",
		"Test function",
		"test"
	};
}

FunctionRegistry makeRegistryWithFakeFunctions() {
	FunctionRegistry registry;
	EXPECT_TRUE(registry.registerFunction(descriptor("FakeAdd", 2, 2), [](const std::vector<double>& args) {
		return args[0] + args[1];
	}).success);
	EXPECT_TRUE(registry.registerFunction(descriptor("FakeSquare", 1, 1), [](const std::vector<double>& args) {
		return args[0] * args[0];
	}).success);
	EXPECT_TRUE(registry.registerFunction(descriptor("FakeConst", 0, 0), [](const std::vector<double>&) {
		return 42.0;
	}).success);
	return registry;
}

}

TEST(FunctionRegistryTest, RegistersAndListsFunctions) {
	FunctionRegistry registry = makeRegistryWithFakeFunctions();

	const std::vector<FunctionDescriptor> functions = registry.listFunctions();
	ASSERT_EQ(functions.size(), 3u);
	EXPECT_EQ(functions[0].publicName, "FakeAdd");
	EXPECT_EQ(functions[0].originName, "DCS test");
	EXPECT_EQ(functions[0].category, "test");
	EXPECT_EQ(functions[1].publicName, "FakeSquare");
	EXPECT_EQ(functions[2].publicName, "FakeConst");
}

TEST(FunctionRegistryTest, LooksUpFunctionByNameCaseInsensitively) {
	FunctionRegistry registry = makeRegistryWithFakeFunctions();

	const FunctionDescriptor* descriptor = registry.lookup("fakeadd");
	ASSERT_NE(descriptor, nullptr);
	EXPECT_EQ(descriptor->publicName, "FakeAdd");
	EXPECT_TRUE(registry.hasFunction("FAKESQUARE"));
	EXPECT_TRUE(registry.hasFunction("FakeConst"));
}

TEST(FunctionRegistryTest, ExecutesRegisteredFunctions) {
	FunctionRegistry registry = makeRegistryWithFakeFunctions();

	FunctionCallResult add = registry.callFunction("FakeAdd", {2.0, 3.0});
	ASSERT_TRUE(add.success) << add.errorMessage;
	EXPECT_DOUBLE_EQ(add.value, 5.0);

	FunctionCallResult square = registry.callFunction("FakeSquare", {4.0});
	ASSERT_TRUE(square.success) << square.errorMessage;
	EXPECT_DOUBLE_EQ(square.value, 16.0);

	FunctionCallResult constant = registry.callFunction("FakeConst", {});
	ASSERT_TRUE(constant.success) << constant.errorMessage;
	EXPECT_DOUBLE_EQ(constant.value, 42.0);
}

TEST(FunctionRegistryTest, ReportsMissingFunction) {
	FunctionRegistry registry = makeRegistryWithFakeFunctions();

	FunctionCallResult result = registry.callFunction("MissingFunction", {});
	EXPECT_FALSE(result.success);
	EXPECT_EQ(result.value, 0.0);
	EXPECT_NE(result.errorMessage.find("MissingFunction"), std::string::npos);
	EXPECT_FALSE(registry.hasFunction("MissingFunction"));
	EXPECT_EQ(registry.lookup("MissingFunction"), nullptr);
}

TEST(FunctionRegistryTest, RejectsNameConflicts) {
	FunctionRegistry registry;
	ASSERT_TRUE(registry.registerFunction(descriptor("FakeAdd", 2, 2), [](const std::vector<double>& args) {
		return args[0] + args[1];
	}).success);

	FunctionRegistrationResult duplicate = registry.registerFunction(descriptor("fakeadd", 2, 2), [](const std::vector<double>& args) {
		return args[0] - args[1];
	});

	EXPECT_FALSE(duplicate.success);
	EXPECT_NE(duplicate.errorMessage.find("fakeadd"), std::string::npos);
	EXPECT_EQ(registry.listFunctions().size(), 1u);
}

TEST(FunctionRegistryTest, RejectsInvalidRegistrationInputs) {
	FunctionRegistry registry;

	FunctionRegistrationResult emptyName = registry.registerFunction(descriptor("", 0, 0), [](const std::vector<double>&) {
		return 0.0;
	});
	EXPECT_FALSE(emptyName.success);

	FunctionRegistrationResult invalidArity = registry.registerFunction(descriptor("InvalidArity", 2, 1), [](const std::vector<double>&) {
		return 0.0;
	});
	EXPECT_FALSE(invalidArity.success);

	FunctionRegistrationResult emptyCallback = registry.registerFunction(descriptor("NoCallback", 0, 0), {});
	EXPECT_FALSE(emptyCallback.success);
}

TEST(FunctionRegistryTest, ReportsWrongArityWithoutCallingCallback) {
	FunctionRegistry registry;
	bool called = false;
	ASSERT_TRUE(registry.registerFunction(descriptor("FakeSquare", 1, 1), [&](const std::vector<double>& args) {
		called = true;
		return args[0] * args[0];
	}).success);

	FunctionCallResult noArguments = registry.callFunction("FakeSquare", {});
	EXPECT_FALSE(noArguments.success);
	EXPECT_FALSE(called);
	EXPECT_NE(noArguments.errorMessage.find("expects 1 arguments"), std::string::npos);

	FunctionCallResult tooManyArguments = registry.callFunction("FakeSquare", {2.0, 3.0});
	EXPECT_FALSE(tooManyArguments.success);
	EXPECT_FALSE(called);
	EXPECT_NE(tooManyArguments.errorMessage.find("got 2"), std::string::npos);
}

TEST(FunctionRegistryTest, CapturesCallbackExceptions) {
	FunctionRegistry registry;
	ASSERT_TRUE(registry.registerFunction(descriptor("Throws", 0, 0), [](const std::vector<double>&) -> double {
		throw std::runtime_error("boom");
	}).success);

	FunctionCallResult result = registry.callFunction("Throws", {});
	EXPECT_FALSE(result.success);
	EXPECT_EQ(result.value, 0.0);
	EXPECT_NE(result.errorMessage.find("Throws"), std::string::npos);
	EXPECT_NE(result.errorMessage.find("boom"), std::string::npos);
}

TEST(FunctionRegistryTest, SupportsArityRanges) {
	FunctionRegistry registry;
	ASSERT_TRUE(registry.registerFunction(descriptor("Range", 1, 3), [](const std::vector<double>& args) {
		double sum = 0.0;
		for (double value : args) {
			sum += value;
		}
		return sum;
	}).success);

	EXPECT_TRUE(registry.callFunction("Range", {1.0}).success);
	EXPECT_TRUE(registry.callFunction("Range", {1.0, 2.0, 3.0}).success);

	FunctionCallResult tooMany = registry.callFunction("Range", {1.0, 2.0, 3.0, 4.0});
	EXPECT_FALSE(tooMany.success);
	EXPECT_NE(tooMany.errorMessage.find("between 1 and 3"), std::string::npos);
}

TEST(ParserDriverFunctionRegistryTest, DriverStartsWithoutFunctionRegistry) {
	genesyspp_driver driver;

	EXPECT_FALSE(driver.hasFunctionRegistry());
	EXPECT_EQ(driver.getFunctionRegistry(), nullptr);
}

TEST(ParserDriverFunctionRegistryTest, DriverAcceptsAndExposesExternalFunctionRegistry) {
	FunctionRegistry registry = makeRegistryWithFakeFunctions();
	genesyspp_driver driver;

	driver.setFunctionRegistry(&registry);

	ASSERT_TRUE(driver.hasFunctionRegistry());
	ASSERT_EQ(driver.getFunctionRegistry(), &registry);
	EXPECT_TRUE(driver.getFunctionRegistry()->hasFunction("FakeAdd"));

	const FunctionCallResult result = driver.getFunctionRegistry()->callFunction("FakeAdd", {10.0, 5.0});
	ASSERT_TRUE(result.success) << result.errorMessage;
	EXPECT_DOUBLE_EQ(result.value, 15.0);
}

TEST(ParserDriverFunctionRegistryTest, DriverCanClearFunctionRegistryReference) {
	FunctionRegistry registry = makeRegistryWithFakeFunctions();
	genesyspp_driver driver;

	driver.setFunctionRegistry(&registry);
	ASSERT_TRUE(driver.hasFunctionRegistry());

	driver.setFunctionRegistry(nullptr);
	EXPECT_FALSE(driver.hasFunctionRegistry());
	EXPECT_EQ(driver.getFunctionRegistry(), nullptr);
}

TEST(ParserDriverFunctionRegistryTest, DriverCopiesNonOwningFunctionRegistryReference) {
	FunctionRegistry registry = makeRegistryWithFakeFunctions();
	genesyspp_driver driver;
	driver.setFunctionRegistry(&registry);

	genesyspp_driver copied(driver);

	ASSERT_TRUE(copied.hasFunctionRegistry());
	ASSERT_EQ(copied.getFunctionRegistry(), &registry);
	EXPECT_TRUE(copied.getFunctionRegistry()->hasFunction("FakeSquare"));
}

TEST(ParserDriverFunctionRegistryTest, DriverMovesNonOwningFunctionRegistryReference) {
	FunctionRegistry registry = makeRegistryWithFakeFunctions();
	genesyspp_driver driver;
	driver.setFunctionRegistry(&registry);

	genesyspp_driver moved(std::move(driver));

	ASSERT_TRUE(moved.hasFunctionRegistry());
	ASSERT_EQ(moved.getFunctionRegistry(), &registry);
	EXPECT_FALSE(driver.hasFunctionRegistry());
}
