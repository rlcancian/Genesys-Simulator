#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "parser/SemanticResolver.h"

namespace {

FunctionDescriptor descriptor(const std::string& name, std::size_t minArity, std::size_t maxArity) {
	return FunctionDescriptor{
		name,
		minArity,
		maxArity,
		"DCS test",
		"Semantic resolver test function",
		"test"
	};
}

FunctionRegistry makeResolverTestRegistry() {
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

TEST(SemanticResolverTest, ResolvesRegisteredFunctions) {
	FunctionRegistry registry = makeResolverTestRegistry();
	SemanticResolver resolver(&registry);

	const SemanticResolverResult add = resolver.resolveFunction("FakeAdd", {2.0, 3.0});
	ASSERT_TRUE(add.success) << add.errorMessage;
	EXPECT_DOUBLE_EQ(add.value, 5.0);

	const SemanticResolverResult square = resolver.resolveFunction({"FakeSquare", {4.0}});
	ASSERT_TRUE(square.success) << square.errorMessage;
	EXPECT_DOUBLE_EQ(square.value, 16.0);

	const SemanticResolverResult constant = resolver.resolveFunction("FakeConst", {});
	ASSERT_TRUE(constant.success) << constant.errorMessage;
	EXPECT_DOUBLE_EQ(constant.value, 42.0);
}

TEST(SemanticResolverTest, ReportsMissingRegistry) {
	SemanticResolver resolver;

	const SemanticResolverResult result = resolver.resolveFunction("FakeAdd", {2.0, 3.0});
	EXPECT_FALSE(result.success);
	EXPECT_EQ(result.value, 0.0);
	EXPECT_NE(result.errorMessage.find("FakeAdd"), std::string::npos);
	EXPECT_NE(result.errorMessage.find("no FunctionRegistry"), std::string::npos);
}

TEST(SemanticResolverTest, ReportsMissingFunctionWithRegisteredFunctions) {
	FunctionRegistry registry = makeResolverTestRegistry();
	SemanticResolver resolver(&registry);

	const SemanticResolverResult result = resolver.resolveFunction("FuncaoInexistente", {1.0});
	EXPECT_FALSE(result.success);
	EXPECT_EQ(result.value, 0.0);
	EXPECT_NE(result.errorMessage.find("FuncaoInexistente"), std::string::npos);
	EXPECT_NE(result.errorMessage.find("1 arguments"), std::string::npos);
	EXPECT_NE(result.errorMessage.find("FakeAdd"), std::string::npos);
	EXPECT_NE(result.errorMessage.find("FakeSquare"), std::string::npos);
	EXPECT_NE(result.errorMessage.find("FakeConst"), std::string::npos);
}

TEST(SemanticResolverTest, ReportsWrongArityWithOrigin) {
	FunctionRegistry registry = makeResolverTestRegistry();
	SemanticResolver resolver(&registry);

	const SemanticResolverResult result = resolver.resolveFunction("FakeAdd", {1.0});
	EXPECT_FALSE(result.success);
	EXPECT_EQ(result.value, 0.0);
	EXPECT_NE(result.errorMessage.find("FakeAdd"), std::string::npos);
	EXPECT_NE(result.errorMessage.find("DCS test"), std::string::npos);
	EXPECT_NE(result.errorMessage.find("1 arguments"), std::string::npos);
	EXPECT_NE(result.errorMessage.find("expected 2"), std::string::npos);
}

TEST(SemanticResolverTest, CapturesCallbackErrors) {
	FunctionRegistry registry;
	ASSERT_TRUE(registry.registerFunction(descriptor("Throws", 0, 0), [](const std::vector<double>&) -> double {
		throw std::runtime_error("boom");
	}).success);
	SemanticResolver resolver(&registry);

	const SemanticResolverResult result = resolver.resolveFunction("Throws", {});
	EXPECT_FALSE(result.success);
	EXPECT_EQ(result.value, 0.0);
	EXPECT_NE(result.errorMessage.find("Throws"), std::string::npos);
	EXPECT_NE(result.errorMessage.find("DCS test"), std::string::npos);
	EXPECT_NE(result.errorMessage.find("boom"), std::string::npos);
}

TEST(SemanticResolverTest, ReportsNonFiniteCallbackReturn) {
	FunctionRegistry registry;
	ASSERT_TRUE(registry.registerFunction(descriptor("Infinity", 0, 0), [](const std::vector<double>&) {
		return std::numeric_limits<double>::infinity();
	}).success);
	SemanticResolver resolver(&registry);

	const SemanticResolverResult result = resolver.resolveFunction("Infinity", {});
	EXPECT_FALSE(result.success);
	EXPECT_EQ(result.value, 0.0);
	EXPECT_NE(result.errorMessage.find("Infinity"), std::string::npos);
	EXPECT_NE(result.errorMessage.find("non-finite"), std::string::npos);
}

TEST(SemanticResolverTest, CanReplaceRegistryReference) {
	FunctionRegistry firstRegistry = makeResolverTestRegistry();
	FunctionRegistry secondRegistry;
	ASSERT_TRUE(secondRegistry.registerFunction(descriptor("OnlySecond", 0, 0), [](const std::vector<double>&) {
		return 7.0;
	}).success);

	SemanticResolver resolver(&firstRegistry);
	ASSERT_TRUE(resolver.hasFunctionRegistry());
	ASSERT_EQ(resolver.getFunctionRegistry(), &firstRegistry);

	resolver.setFunctionRegistry(&secondRegistry);
	ASSERT_EQ(resolver.getFunctionRegistry(), &secondRegistry);

	const SemanticResolverResult result = resolver.resolveFunction("OnlySecond", {});
	ASSERT_TRUE(result.success) << result.errorMessage;
	EXPECT_DOUBLE_EQ(result.value, 7.0);
}
