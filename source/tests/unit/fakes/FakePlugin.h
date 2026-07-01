#ifndef TESTS_UNIT_FAKES_FAKEPLUGIN_H
#define TESTS_UNIT_FAKES_FAKEPLUGIN_H

#include <vector>

#include "parser/FunctionRegistry.h"

class FakePlugin {
public:
	static FunctionRegistrationResult registerFunctions(FunctionRegistry& registry) {
		FunctionRegistrationResult addResult = registry.registerFunction(
			{"FakeAdd", 2, 2, "FakePlugin", "Adds two numeric arguments", "demo"},
			[](const std::vector<double>& arguments) {
				return arguments[0] + arguments[1];
			});
		if (!addResult.success) {
			return addResult;
		}

		return registry.registerFunction(
			{"FakeSquare", 1, 1, "FakePlugin", "Squares one numeric argument", "demo"},
			[](const std::vector<double>& arguments) {
				return arguments[0] * arguments[0];
			});
	}
};

#endif
