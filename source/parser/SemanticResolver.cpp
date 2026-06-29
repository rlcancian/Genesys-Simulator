#include "SemanticResolver.h"

#include <cmath>
#include <sstream>

SemanticResolverResult SemanticResolverResult::ok(double value) {
	return {true, value, ""};
}

SemanticResolverResult SemanticResolverResult::fail(const std::string& message) {
	return {false, 0.0, message};
}

SemanticResolver::SemanticResolver(const FunctionRegistry* functionRegistry)
	: _functionRegistry(functionRegistry) {}

void SemanticResolver::setFunctionRegistry(const FunctionRegistry* functionRegistry) {
	_functionRegistry = functionRegistry;
}

const FunctionRegistry* SemanticResolver::getFunctionRegistry() const {
	return _functionRegistry;
}

bool SemanticResolver::hasFunctionRegistry() const {
	return _functionRegistry != nullptr;
}

SemanticResolverResult SemanticResolver::resolveFunction(const FunctionCallRequest& request) const {
	return resolveFunction(request.functionName, request.arguments);
}

SemanticResolverResult SemanticResolver::resolveFunction(const std::string& functionName, const std::vector<double>& arguments) const {
	if (_functionRegistry == nullptr) {
		return SemanticResolverResult::fail("Cannot resolve function \"" + functionName + "\": no FunctionRegistry is configured.");
	}

	const FunctionDescriptor* descriptor = _functionRegistry->lookup(functionName);
	if (descriptor == nullptr) {
		return SemanticResolverResult::fail("Cannot resolve function \"" + functionName + "\" with "
			+ std::to_string(arguments.size()) + " arguments: function is not registered. Registered functions: "
			+ listRegisteredFunctions(*_functionRegistry) + ".");
	}

	if (!descriptor->acceptsArity(arguments.size())) {
		return SemanticResolverResult::fail("Cannot resolve function \"" + descriptor->publicName + "\" from "
			+ describeOrigin(*descriptor) + " with " + std::to_string(arguments.size())
			+ " arguments: expected " + describeExpectedArity(*descriptor) + ".");
	}

	const FunctionCallResult callResult = _functionRegistry->callFunction(functionName, arguments);
	if (!callResult.success) {
		return SemanticResolverResult::fail("Cannot resolve function \"" + descriptor->publicName + "\" from "
			+ describeOrigin(*descriptor) + ": " + callResult.errorMessage);
	}

	if (!isFinite(callResult.value)) {
		return SemanticResolverResult::fail("Cannot resolve function \"" + descriptor->publicName + "\" from "
			+ describeOrigin(*descriptor) + ": callback returned a non-finite value.");
	}

	return SemanticResolverResult::ok(callResult.value);
}

std::string SemanticResolver::describeExpectedArity(const FunctionDescriptor& descriptor) {
	if (descriptor.minArity == descriptor.maxArity) {
		return std::to_string(descriptor.minArity);
	}
	return "between " + std::to_string(descriptor.minArity) + " and " + std::to_string(descriptor.maxArity);
}

std::string SemanticResolver::describeOrigin(const FunctionDescriptor& descriptor) {
	if (descriptor.originName.empty()) {
		return "unknown origin";
	}
	return "\"" + descriptor.originName + "\"";
}

std::string SemanticResolver::listRegisteredFunctions(const FunctionRegistry& registry) {
	const std::vector<FunctionDescriptor> functions = registry.listFunctions();
	if (functions.empty()) {
		return "<none>";
	}

	std::ostringstream stream;
	for (std::size_t index = 0; index < functions.size(); ++index) {
		if (index > 0) {
			stream << ", ";
		}
		stream << functions[index].publicName;
	}
	return stream.str();
}

bool SemanticResolver::isFinite(double value) {
	return std::isfinite(value);
}
