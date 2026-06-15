#ifndef METABOLICFLUXBALANCE_H
#define METABOLICFLUXBALANCE_H

#include <map>
#include <string>

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/BiochemicalSimulation/MetabolicNetwork.h"

/**
 * Component that evaluates flux-balance analysis (FBA) over a MetabolicNetwork.
 *
 * ## LP solver dependency (GLPK)
 *
 * MetabolicFluxBalance requires GLPK for metabolic networks with more than a
 * few dozen reactions (e.g., M. genitalium: 641 reactions; yeast-GEM: 4131
 * reactions). Without GLPK, a built-in basis-enumeration fallback is used,
 * which is only practical for small toy networks.
 *
 * ### Installation
 *   Ubuntu/Debian:  sudo apt-get install libglpk-dev
 *   Fedora/RHEL:    sudo dnf install glpk-devel
 *   macOS (Brew):   brew install glpk
 *
 * ### Re-configuration after installation
 *   cmake --preset terminal-app   # or any other preset
 *   cmake --build build/terminal-app --target genesys_plugins_components
 *
 * CMake will print at configure time:
 *   GenESyS: GLPK found — MetabolicFluxBalance ... will use GLPK LP solver
 * or:
 *   GenESyS: GLPK not found — ... (suitable for small models only; install libglpk-dev ...)
 *
 * The preprocessor guard is GENESYS_HAVE_GLPK. The GLPK solver wrapper is in
 * source/tools/Biochemical/GlpkFluxBalanceSolver.h.
 */
class MetabolicFluxBalance : public ModelComponent {
public:
	MetabolicFluxBalance(Model* model, std::string name = "");
	virtual ~MetabolicFluxBalance() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setMetabolicNetwork(MetabolicNetwork* metabolicNetwork);
	MetabolicNetwork* getMetabolicNetwork() const;
	void setObjectiveReactionName(std::string objectiveReactionName);
	std::string getObjectiveReactionName() const;
	void setLastSucceeded(bool lastSucceeded);
	bool getLastSucceeded() const;
	void setLastObjectiveValue(double lastObjectiveValue);
	double getLastObjectiveValue() const;
	void setLastMessage(std::string lastMessage);
	std::string getLastMessage() const;
	const std::map<std::string, double>& getLastFluxes() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	// virtual void _createInternalAndAttachedData() override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;

protected:
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	virtual void _createEditableDataDefinitions() override;
	// virtual void _createAttachedAttributes() override;

private:
	void _forwardEntity(Entity* entity);

private:
	const struct DEFAULT_VALUES {
		std::string metabolicNetworkName = "";
		std::string objectiveReactionName = "";
		bool lastSucceeded = false;
		double lastObjectiveValue = 0.0;
		std::string lastMessage = "";
	} DEFAULT;

	MetabolicNetwork* _metabolicNetwork = nullptr;
	std::string _objectiveReactionName = DEFAULT.objectiveReactionName;
	bool _lastSucceeded = DEFAULT.lastSucceeded;
	double _lastObjectiveValue = DEFAULT.lastObjectiveValue;
	std::string _lastMessage = DEFAULT.lastMessage;
	std::map<std::string, double> _lastFluxes;
};

#endif /* METABOLICFLUXBALANCE_H */
