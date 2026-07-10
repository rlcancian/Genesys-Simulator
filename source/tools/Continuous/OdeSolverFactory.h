#ifndef ODESOLVERFACTORY_H
#define ODESOLVERFACTORY_H

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "DormandPrince54OdeSolver.h"
#include "OdeSolver_if.h"
#include "RungeKutta4OdeSolver.h"

enum class OdeSolverType {
	RungeKutta4,
	DormandPrince54
};

class OdeSolverFactory {
public:
	using Creator = std::function<std::unique_ptr<OdeSolver_if>()>;

	static const std::string& keyRungeKutta4() {
		static const std::string k = "RungeKutta4";
		return k;
	}

	static const std::string& keyDormandPrince54() {
		static const std::string k = "DormandPrince54";
		return k;
	}

	static const std::string& defaultKey() {
		return keyRungeKutta4();
	}

	static std::unique_ptr<OdeSolver_if> create(OdeSolverType type) {
		switch (type) {
			case OdeSolverType::DormandPrince54:
				return std::make_unique<DormandPrince54OdeSolver>();
			case OdeSolverType::RungeKutta4:
			default:
				return std::make_unique<RungeKutta4OdeSolver>();
		}
	}

	static std::unique_ptr<OdeSolver_if> create(const std::string& key) {
		auto& reg = registry();
		auto it = reg.find(key);
		if (it != reg.end()) {
			return it->second();
		}
		return std::make_unique<RungeKutta4OdeSolver>();
	}

	static bool isRegistered(const std::string& key) {
		return registry().find(key) != registry().end();
	}

	static std::vector<std::string> availableKeys() {
		std::vector<std::string> keys;
		keys.reserve(registry().size());
		for (const auto& entry : registry()) {
			keys.push_back(entry.first);
		}
		return keys;
	}

	static bool registerCreator(const std::string& key, Creator creator) {
		return registry().emplace(key, std::move(creator)).second;
	}

	static std::string toKey(OdeSolverType type) {
		switch (type) {
			case OdeSolverType::DormandPrince54:
				return keyDormandPrince54();
			case OdeSolverType::RungeKutta4:
			default:
				return keyRungeKutta4();
		}
	}

	static bool fromKey(const std::string& key, OdeSolverType& out) {
		if (key == keyDormandPrince54()) {
			out = OdeSolverType::DormandPrince54;
			return true;
		}
		if (key == keyRungeKutta4()) {
			out = OdeSolverType::RungeKutta4;
			return true;
		}
		return false;
	}

private:
	static std::map<std::string, Creator>& registry() {
		static std::map<std::string, Creator> reg = [] {
			std::map<std::string, Creator> initial;
			initial.emplace(keyRungeKutta4(),
			                [] { return std::make_unique<RungeKutta4OdeSolver>(); });
			initial.emplace(keyDormandPrince54(),
			                [] { return std::make_unique<DormandPrince54OdeSolver>(); });
			return initial;
		}();
		return reg;
	}
};

#endif /* ODESOLVERFACTORY_H */
