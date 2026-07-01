#ifndef FUNCTIONREGISTRY_H
#define FUNCTIONREGISTRY_H

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

struct FunctionDescriptor {
	std::string publicName;
	std::size_t minArity = 0;
	std::size_t maxArity = 0;
	std::string originName;
	std::string description;
	std::string category;

	bool acceptsArity(std::size_t arity) const;
};

struct FunctionRegistrationResult {
	bool success = false;
	std::string errorMessage;

	static FunctionRegistrationResult ok();
	static FunctionRegistrationResult fail(const std::string& message);
};

struct FunctionCallResult {
	bool success = false;
	double value = 0.0;
	std::string errorMessage;

	static FunctionCallResult ok(double value);
	static FunctionCallResult fail(const std::string& message);
};

using FunctionCallback = std::function<double(const std::vector<double>&)>;

class FunctionRegistry {
public:
	FunctionRegistrationResult registerFunction(FunctionDescriptor descriptor, FunctionCallback callback);
	const FunctionDescriptor* lookup(const std::string& name) const;
	bool hasFunction(const std::string& name) const;
	FunctionCallResult callFunction(const std::string& name, const std::vector<double>& arguments) const;
	std::vector<FunctionDescriptor> listFunctions() const;

private:
	struct FunctionEntry {
		FunctionDescriptor descriptor;
		FunctionCallback callback;
	};

	static std::string normalizeName(const std::string& name);
	static std::string buildArityError(const FunctionDescriptor& descriptor, std::size_t actualArity);

	std::vector<FunctionEntry>::const_iterator findEntry(const std::string& name) const;

private:
	std::vector<FunctionEntry> _functions;
};

#endif
