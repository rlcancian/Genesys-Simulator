#ifndef CELLDIVISIONEVENT_H
#define CELLDIVISIONEVENT_H

#include <random>
#include <string>

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"

/**
 * Discrete cell division event for whole-cell model simulations.
 *
 * Implements the M. genitalium CellDivision process (Karr et al. 2012):
 *   Trigger: cell mass >= divisionMassThreshold (default 2 × cellInitialDryWeight)
 *            AND (ftsZRingCompletion key in WholeCellState >= ftsZThreshold OR
 *                 ftsZThreshold == 0, i.e., mass-only trigger)
 *
 * On division:
 *   - Each integer molecule count is binomially partitioned between daughters
 *     (each copy independently assigned to daughter 1 with p=0.5)
 *   - This simulated cell becomes daughter 1
 *   - Cell mass and volume are set to half the pre-division values
 *   - Simulation step count is reset to 0 (new generation)
 *
 * Output ports:
 *   Port 0: entity forwarded when no division occurred this step (normal flow)
 *   Port 1: entity forwarded immediately after a division event (optional handler)
 *
 * The division mass threshold is read from WholeCellState metabolite pool entry
 * "param.cellDivisionMassThreshold" when set by loadParameters(). Override with
 * the divisionMassThreshold property for manual configuration.
 *
 * FtsZ ring completion is read from WholeCellState molecule count key
 * ftsZRingKey (default "FtsZ_ring_completion", expected range 0–1000 as integer
 * per-mille value; divide by 1000 for fraction).
 */
class CellDivisionEvent : public ModelComponent {
public:
	CellDivisionEvent(Model* model, std::string name = "");
	virtual ~CellDivisionEvent() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setWholeCellState(WholeCellState* state);
	WholeCellState* getWholeCellState() const;
	void setDivisionMassThreshold(double threshold);
	double getDivisionMassThreshold() const;
	void setFtsZThreshold(double threshold);
	double getFtsZThreshold() const;
	void setFtsZRingKey(std::string key);
	std::string getFtsZRingKey() const;
	void setRandomSeed(unsigned int seed);
	unsigned int getRandomSeed() const;
	bool getLastDivided() const;
	int getDivisionCount() const;

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
	bool _checkDivisionCondition() const;
	void _applyDivision();
	void _forwardEntity(Entity* entity, unsigned int port);

private:
	const struct DEFAULT_VALUES {
		std::string wholeCellStateName  = "";
		double divisionMassThreshold    = 7.86e-15; // 2 × 3.93e-15 g (M. genitalium, parameters.json)
		double ftsZThreshold            = 0.0;      // 0 = mass-only trigger; >0 requires FtsZ ring
		std::string ftsZRingKey         = "FtsZ_ring_completion";
		unsigned int randomSeed         = 44u;
	} DEFAULT;

	WholeCellState* _wholeCellState    = nullptr;
	double _divisionMassThreshold      = DEFAULT.divisionMassThreshold;
	double _ftsZThreshold              = DEFAULT.ftsZThreshold;
	std::string _ftsZRingKey           = DEFAULT.ftsZRingKey;
	unsigned int _randomSeed           = DEFAULT.randomSeed;
	bool _lastDivided                  = false;
	int _divisionCount                 = 0;
	std::mt19937 _rng{DEFAULT.randomSeed};
};

#endif /* CELLDIVISIONEVENT_H */
