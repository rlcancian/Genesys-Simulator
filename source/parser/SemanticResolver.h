#ifndef SEMANTICRESOLVER_H
#define SEMANTICRESOLVER_H

#include <string>
#include <vector>

#include "FunctionRegistry.h"

struct FunctionCallRequest {
	std::string functionName;
	std::vector<double> arguments;
};

struct SemanticResolverResult {
	bool success = false;
	double value = 0.0;
	std::string errorMessage;

	static SemanticResolverResult ok(double value);
	static SemanticResolverResult fail(const std::string& message);
};

class SemanticResolver {
public:
	explicit SemanticResolver(const FunctionRegistry* functionRegistry = nullptr);

	void setFunctionRegistry(const FunctionRegistry* functionRegistry);
	const FunctionRegistry* getFunctionRegistry() const;
	bool hasFunctionRegistry() const;

	SemanticResolverResult resolveFunction(const FunctionCallRequest& request) const;
	SemanticResolverResult resolveFunction(const std::string& functionName, const std::vector<double>& arguments) const;

private:
	static std::string describeExpectedArity(const FunctionDescriptor& descriptor);
	static std::string describeOrigin(const FunctionDescriptor& descriptor);
	static std::string listRegisteredFunctions(const FunctionRegistry& registry);
	static bool isFinite(double value);

private:
	const FunctionRegistry* _functionRegistry = nullptr;
};

#endif
