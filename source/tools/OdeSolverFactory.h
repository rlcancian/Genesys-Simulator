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

// Builds ODE solvers on demand, so the rest of the code doesn't need to know
// which method it's using.
//
// Before this, BioNetwork created a RungeKutta4OdeSolver directly, so changing
// the method meant editing every place that used it. Now callers just ask for a
// solver by name ("RungeKutta4" or "DormandPrince54") and get back an
// OdeSolver_if to use, without caring about the concrete type.
//
// The solvers live in a small name->creator table. To add a new one you can put
// it in the table below, or call registerCreator() at startup without touching
// this file. There are two ways to name a solver: the OdeSolverType enum (handy
// in code) and a string key (used when saving the model and in the GUI);
// toKey()/fromKey() convert between them.
enum class OdeSolverType {
	RungeKutta4,     // the original fixed-step RK4
	DormandPrince54  // the new adaptive Dormand-Prince 5(4)
};

class OdeSolverFactory {
public:
	// A function that makes a new solver.
	using Creator = std::function<std::unique_ptr<OdeSolver_if>()>;

	// Name of the RK4 solver.
	static const std::string& keyRungeKutta4() {
		static const std::string k = "RungeKutta4";
		return k;
	}

	// Name of the Dormand-Prince 5(4) solver.
	static const std::string& keyDormandPrince54() {
		static const std::string k = "DormandPrince54";
		return k;
	}

	// What we use when no solver was chosen.
	static const std::string& defaultKey() {
		return keyRungeKutta4();
	}

	// Makes a solver from the enum. Always returns a valid pointer.
	static std::unique_ptr<OdeSolver_if> create(OdeSolverType type) {
		switch (type) {
			case OdeSolverType::DormandPrince54:
				return std::make_unique<DormandPrince54OdeSolver>();
			case OdeSolverType::RungeKutta4:
			default:
				return std::make_unique<RungeKutta4OdeSolver>();
		}
	}

	// Makes a solver from its name (case-sensitive). If the name isn't known we
	// fall back to RK4 so an old or broken model still runs.
	static std::unique_ptr<OdeSolver_if> create(const std::string& key) {
		auto& reg = registry();
		auto it = reg.find(key);
		if (it != reg.end()) {
			return it->second();
		}
		return std::make_unique<RungeKutta4OdeSolver>();
	}

	// True if this name is one of our solvers.
	static bool isRegistered(const std::string& key) {
		return registry().find(key) != registry().end();
	}

	// All solver names (std::map keeps them sorted).
	static std::vector<std::string> availableKeys() {
		std::vector<std::string> keys;
		keys.reserve(registry().size());
		for (const auto& entry : registry()) {
			keys.push_back(entry.first);
		}
		return keys;
	}

	// Adds a new solver under this name. Returns false if the name is taken.
	static bool registerCreator(const std::string& key, Creator creator) {
		return registry().emplace(key, std::move(creator)).second;
	}

	// enum -> name.
	static std::string toKey(OdeSolverType type) {
		switch (type) {
			case OdeSolverType::DormandPrince54:
				return keyDormandPrince54();
			case OdeSolverType::RungeKutta4:
			default:
				return keyRungeKutta4();
		}
	}

	// name -> enum. Returns false (and leaves out untouched) if unknown.
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
	// The name->creator table, filled once with the built-in solvers the first
	// time it's used (static local, so there's only ever one copy).
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
