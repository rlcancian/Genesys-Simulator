#ifndef CELLCYCLECHECKPOINTCOMPONENT_H
#define CELLCYCLECHECKPOINTCOMPONENT_H

#include <string>

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"

/*!
 * \brief Evaluates lifecycle checkpoints and advances whole-cell step time.
 *
 * CellCycleCheckpointComponent centralizes two concerns that otherwise become
 * scattered across individual submodels:
 *  - explicit advancement of WholeCellState current time and step count; and
 *  - annotation of a coarse lifecycle phase based on energy availability,
 *    division readiness, and viability.
 *
 * The component does not execute metabolism or division itself. It only reads
 * WholeCellState and writes back lifecycle metadata such as:
 *  - `LifecyclePhase` = `newborn`, `growth`, `division_ready`, `starved`, `dead`
 *  - `Viable`
 *
 * This keeps WholeCellState as a passive data container and concentrates the
 * event-boundary checkpoint logic in a dedicated ModelComponent.
 */
class CellCycleCheckpointComponent : public ModelComponent {
public:
	CellCycleCheckpointComponent(Model* model, std::string name = "");
	virtual ~CellCycleCheckpointComponent() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setWholeCellState(WholeCellState* state);
	WholeCellState* getWholeCellState() const;
	void setDeltaT(double deltaT);
	double getDeltaT() const;
	void setAdvanceWholeCellClock(bool advance);
	bool getAdvanceWholeCellClock() const;
	void setEnergyMetaboliteKey(std::string key);
	std::string getEnergyMetaboliteKey() const;
	void setCompartmentEnergyRegion(std::string region);
	std::string getCompartmentEnergyRegion() const;
	void setCompartmentEnergyMetaboliteKey(std::string key);
	std::string getCompartmentEnergyMetaboliteKey() const;
	void setStarvationAtpThreshold(double threshold);
	double getStarvationAtpThreshold() const;
	void setCompartmentStarvationThreshold(double threshold);
	double getCompartmentStarvationThreshold() const;
	void setLethalStarvationSteps(unsigned int steps);
	unsigned int getLethalStarvationSteps() const;
	void setDivisionMassThreshold(double threshold);
	double getDivisionMassThreshold() const;
	void setFtsZRingKey(std::string key);
	std::string getFtsZRingKey() const;
	void setFtsZThreshold(double threshold);
	double getFtsZThreshold() const;
	void setCriticalPathwayActivityKey(std::string key);
	std::string getCriticalPathwayActivityKey() const;
	void setCriticalPathwayActivityThreshold(double threshold);
	double getCriticalPathwayActivityThreshold() const;
	unsigned int getStarvationStreak() const;

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
	bool _isDivisionReady() const;
	void _updateLifecyclePhase();
	void _forwardEntity(Entity* entity);

private:
	const struct DEFAULT_VALUES {
		std::string wholeCellStateName = "";
		double deltaT = 60.0;
		bool advanceWholeCellClock = true;
		std::string energyMetaboliteKey = "ATP";
		std::string compartmentEnergyRegion = "";
		std::string compartmentEnergyMetaboliteKey = "";
		double starvationAtpThreshold = 0.25;
		double compartmentStarvationThreshold = 0.0;
		unsigned int lethalStarvationSteps = 0u;
		double divisionMassThreshold = 0.0;
		std::string ftsZRingKey = "FtsZ_ring_completion";
		double ftsZThreshold = 0.0;
		std::string criticalPathwayActivityKey = "";
		double criticalPathwayActivityThreshold = 0.0;
	} DEFAULT;

	WholeCellState* _wholeCellState = nullptr;
	double _deltaT = DEFAULT.deltaT;
	bool _advanceWholeCellClock = DEFAULT.advanceWholeCellClock;
	std::string _energyMetaboliteKey = DEFAULT.energyMetaboliteKey;
	std::string _compartmentEnergyRegion = DEFAULT.compartmentEnergyRegion;
	std::string _compartmentEnergyMetaboliteKey = DEFAULT.compartmentEnergyMetaboliteKey;
	double _starvationAtpThreshold = DEFAULT.starvationAtpThreshold;
	double _compartmentStarvationThreshold = DEFAULT.compartmentStarvationThreshold;
	unsigned int _lethalStarvationSteps = DEFAULT.lethalStarvationSteps;
	double _divisionMassThreshold = DEFAULT.divisionMassThreshold;
	std::string _ftsZRingKey = DEFAULT.ftsZRingKey;
	double _ftsZThreshold = DEFAULT.ftsZThreshold;
	std::string _criticalPathwayActivityKey = DEFAULT.criticalPathwayActivityKey;
	double _criticalPathwayActivityThreshold = DEFAULT.criticalPathwayActivityThreshold;
	unsigned int _starvationStreak = 0u;
};

#endif /* CELLCYCLECHECKPOINTCOMPONENT_H */
