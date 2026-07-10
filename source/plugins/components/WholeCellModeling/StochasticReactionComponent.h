#ifndef STOCHASTICREACTIONCOMPONENT_H
#define STOCHASTICREACTIONCOMPONENT_H

#include <map>
#include <random>
#include <string>
#include <vector>

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/WholeCellModeling/StochasticReactionRule.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"

/**
 * Gillespie Direct Method SSA runner for whole-cell stochastic simulations.
 *
 * On each entity arrival the component runs the Direct Method (Gillespie 1977)
 * for a configurable time window [0, timeWindow] using all StochasticReactionRule
 * definitions registered in the model. Species counts are read from and written
 * back to the referenced WholeCellState (if set) or directly to MolecularSpecies
 * objects in the model.
 *
 * Contract (Q7b):
 *   Input:  current molecule counts from WholeCellState (or MolecularSpecies)
 *           + resource budget from WholeCellState
 *   Output: updated counts written back to WholeCellState; entity forwarded
 *
 * Resource cap: reactions whose firing would deplete a species below its
 * resource budget floor are skipped, matching the fair-allocation contract
 * of the WholeCell M. genitalium model (Karr et al. 2012).
 */
class StochasticReactionComponent : public ModelComponent {
public:
	StochasticReactionComponent(Model* model, std::string name = "");
	virtual ~StochasticReactionComponent() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setWholeCellState(WholeCellState* state);
	WholeCellState* getWholeCellState() const;
	void setTimeWindow(double timeWindow);
	double getTimeWindow() const;
	void setAdvanceWholeCellClock(bool advance);
	bool getAdvanceWholeCellClock() const;
	void setRandomSeed(unsigned int seed);
	unsigned int getRandomSeed() const;
	int getLastReactionCount() const;
	double getLastSimulatedTime() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;

protected:
	virtual void _createEditableDataDefinitions() override;

private:
	struct GillespieState {
		std::map<std::string, int> counts;
		double simulatedTime = 0.0;
		int reactionCount = 0;
	};

	GillespieState _runGillespie(
		const std::map<std::string, int>& initialCounts,
		const std::vector<StochasticReactionRule*>& rules,
		const std::map<std::string, int>& resourceBudget,
		double timeWindow);

	void _collectCountsFromState(std::map<std::string, int>& counts) const;
	void _applyCountsToState(const std::map<std::string, int>& counts);
	std::vector<StochasticReactionRule*> _collectRules() const;

	void _forwardEntity(Entity* entity);

private:
		const struct DEFAULT_VALUES {
			std::string wholeCellStateName = "";
			double timeWindow = 1.0;
			bool advanceWholeCellClock = true;
			unsigned int randomSeed = 42u;
		} DEFAULT;

	WholeCellState* _wholeCellState = nullptr;
	double _timeWindow = DEFAULT.timeWindow;
	bool _advanceWholeCellClock = DEFAULT.advanceWholeCellClock;
	unsigned int _randomSeed = DEFAULT.randomSeed;
	int _lastReactionCount = 0;
	double _lastSimulatedTime = 0.0;

	std::mt19937 _rng{DEFAULT.randomSeed};
};

#endif /* STOCHASTICREACTIONCOMPONENT_H */
