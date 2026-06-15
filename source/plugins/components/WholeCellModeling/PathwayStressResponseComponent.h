#ifndef PATHWAYSTRESSRESPONSECOMPONENT_H
#define PATHWAYSTRESSRESPONSECOMPONENT_H

#include <string>

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"

/*!
 * \brief Applies sustained pathway-collapse responses to a whole-cell state.
 *
 * PathwayStressResponseComponent watches one pathway activity proxy already
 * stored in WholeCellState and converts repeated low-activity windows into
 * higher-level lifecycle responses such as arrest and death.
 *
 * The component is intentionally simple:
 * - it does not solve metabolism;
 * - it does not schedule time;
 * - it does not route entities.
 *
 * Instead, it acts as a dedicated biological stress-response layer that can be
 * placed between checkpointing and fate-routing components in hybrid
 * whole-cell examples.
 */
class PathwayStressResponseComponent : public ModelComponent {
public:
	PathwayStressResponseComponent(Model* model, std::string name = "");
	virtual ~PathwayStressResponseComponent() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setWholeCellState(WholeCellState* state);
	WholeCellState* getWholeCellState() const;
	void setMonitoredPathwayKey(std::string key);
	std::string getMonitoredPathwayKey() const;
	void setStressThreshold(double threshold);
	double getStressThreshold() const;
	void setArrestAfterSteps(unsigned int steps);
	unsigned int getArrestAfterSteps() const;
	void setDeathAfterSteps(unsigned int steps);
	unsigned int getDeathAfterSteps() const;
	void setArrestPhase(std::string phase);
	std::string getArrestPhase() const;
	void setDeadPhase(std::string phase);
	std::string getDeadPhase() const;
	void setRecoveryPhase(std::string phase);
	std::string getRecoveryPhase() const;
	void setResetStreakOnRecovery(bool reset);
	bool getResetStreakOnRecovery() const;
	unsigned int getStressStreak() const;
	double getLastObservedActivity() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	virtual void _createEditableDataDefinitions() override;

private:
	void _forwardEntity(Entity* entity);

private:
	const struct DEFAULT_VALUES {
		std::string wholeCellStateName = "";
		std::string monitoredPathwayKey = "biomass_objective";
		double stressThreshold = 0.0;
		unsigned int arrestAfterSteps = 0u;
		unsigned int deathAfterSteps = 0u;
		std::string arrestPhase = "arrested";
		std::string deadPhase = "dead";
		std::string recoveryPhase = "growth";
		bool resetStreakOnRecovery = true;
		unsigned int stressStreak = 0u;
		double lastObservedActivity = 0.0;
	} DEFAULT;

	WholeCellState* _wholeCellState = nullptr;
	std::string _monitoredPathwayKey = DEFAULT.monitoredPathwayKey;
	double _stressThreshold = DEFAULT.stressThreshold;
	unsigned int _arrestAfterSteps = DEFAULT.arrestAfterSteps;
	unsigned int _deathAfterSteps = DEFAULT.deathAfterSteps;
	std::string _arrestPhase = DEFAULT.arrestPhase;
	std::string _deadPhase = DEFAULT.deadPhase;
	std::string _recoveryPhase = DEFAULT.recoveryPhase;
	bool _resetStreakOnRecovery = DEFAULT.resetStreakOnRecovery;
	unsigned int _stressStreak = DEFAULT.stressStreak;
	double _lastObservedActivity = DEFAULT.lastObservedActivity;
};

#endif /* PATHWAYSTRESSRESPONSECOMPONENT_H */
