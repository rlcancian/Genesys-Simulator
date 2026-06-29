#include "FunctionRegistry.h"

#include <algorithm>
#include <cctype>
#include <exception>
#include <utility>

bool FunctionDescriptor::acceptsArity(std::size_t arity) const {
	return arity >= minArity && arity <= maxArity;
}

FunctionRegistrationResult FunctionRegistrationResult::ok() {
	return {true, ""};
}

FunctionRegistrationResult FunctionRegistrationResult::fail(const std::string& message) {
	return {false, message};
}

FunctionCallResult FunctionCallResult::ok(double value) {
	return {true, value, ""};
}

FunctionCallResult FunctionCallResult::fail(const std::string& message) {
	return {false, 0.0, message};
}

FunctionRegistrationResult FunctionRegistry::registerFunction(FunctionDescriptor descriptor, FunctionCallback callback) {
	if (descriptor.publicName.empty()) {
		return FunctionRegistrationResult::fail("Function name cannot be empty.");
	}
	if (descriptor.minArity > descriptor.maxArity) {
		return FunctionRegistrationResult::fail("Function minimum arity cannot be greater than maximum arity.");
	}
	if (!callback) {
		return FunctionRegistrationResult::fail("Function callback cannot be empty.");
	}
	if (hasFunction(descriptor.publicName)) {
		return FunctionRegistrationResult::fail("Function \"" + descriptor.publicName + "\" is already registered.");
	}

	_functions.push_back({descriptor, std::move(callback)});
	return FunctionRegistrationResult::ok();
}

const FunctionDescriptor* FunctionRegistry::lookup(const std::string& name) const {
	const auto it = findEntry(name);
	if (it == _functions.end()) {
		return nullptr;
	}
	return &it->descriptor;
}

bool FunctionRegistry::hasFunction(const std::string& name) const {
	return lookup(name) != nullptr;
}

FunctionCallResult FunctionRegistry::callFunction(const std::string& name, const std::vector<double>& arguments) const {
	const auto it = findEntry(name);
	if (it == _functions.end()) {
		return FunctionCallResult::fail("Function \"" + name + "\" is not registered.");
	}

	const FunctionDescriptor& descriptor = it->descriptor;
	if (!descriptor.acceptsArity(arguments.size())) {
		return FunctionCallResult::fail(buildArityError(descriptor, arguments.size()));
	}

	try {
		return FunctionCallResult::ok(it->callback(arguments));
	} catch (const std::exception& e) {
		return FunctionCallResult::fail("Function \"" + descriptor.publicName + "\" failed: " + std::string(e.what()));
	} catch (const std::string& e) {
		return FunctionCallResult::fail("Function \"" + descriptor.publicName + "\" failed: " + e);
	} catch (...) {
		return FunctionCallResult::fail("Function \"" + descriptor.publicName + "\" failed: unknown error.");
	}
}

std::vector<FunctionDescriptor> FunctionRegistry::listFunctions() const {
	std::vector<FunctionDescriptor> result;
	result.reserve(_functions.size());
	for (const FunctionEntry& entry : _functions) {
		result.push_back(entry.descriptor);
	}
	return result;
}

std::string FunctionRegistry::normalizeName(const std::string& name) {
	std::string normalized = name;
	std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char ch) {
		return static_cast<char>(std::tolower(ch));
	});
	return normalized;
}

std::string FunctionRegistry::buildArityError(const FunctionDescriptor& descriptor, std::size_t actualArity) {
	if (descriptor.minArity == descriptor.maxArity) {
		return "Function \"" + descriptor.publicName + "\" expects " + std::to_string(descriptor.minArity)
			+ " arguments, got " + std::to_string(actualArity) + ".";
	}

	return "Function \"" + descriptor.publicName + "\" expects between " + std::to_string(descriptor.minArity)
		+ " and " + std::to_string(descriptor.maxArity) + " arguments, got " + std::to_string(actualArity) + ".";
}

std::vector<FunctionRegistry::FunctionEntry>::const_iterator FunctionRegistry::findEntry(const std::string& name) const {
	const std::string normalizedName = normalizeName(name);
	return std::find_if(_functions.begin(), _functions.end(), [&](const FunctionEntry& entry) {
		return normalizeName(entry.descriptor.publicName) == normalizedName;
	});
}
