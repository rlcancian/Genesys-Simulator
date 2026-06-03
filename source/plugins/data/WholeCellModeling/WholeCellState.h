#ifndef WHOLECELLSTATE_H
#define WHOLECELLSTATE_H

#include <map>
#include <string>
#include <vector>

#include "../../../kernel/simulator/model/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"

/**
 * Shared state container for whole-cell model simulations.
 *
 * WholeCellState is the GenESyS analog to the Vivarium Store: a single
 * ModelDataDefinition instance shared by all WCM component plugins in a
 * simulation step. It holds:
 *   - discrete molecule counts (mRNA, proteins) updated by SSA
 *   - continuous metabolite pool (ATP, amino acids) updated by FBA
 *   - resource budget allocated by fair-allocation before each step
 *   - cell geometry (volume, mass) for propensity scaling
 *
 * JSON loading (Q5a) reads fixedConstants.json and parameters.json from the
 * CovertLab/WholeCell M. genitalium repository (MIT license).
 */
class WholeCellState : public ModelDataDefinition {
public:
	WholeCellState(Model* model, std::string name = "");
	virtual ~WholeCellState() override = default;

public:
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	// Molecule count access (integer, for SSA)
	void setMoleculeCount(const std::string& speciesName, int count);
	int getMoleculeCount(const std::string& speciesName) const;
	bool hasMoleculeCount(const std::string& speciesName) const;
	const std::map<std::string, int>& getMoleculeCounts() const;

	// Metabolite pool access (continuous, for FBA output)
	void setMetaboliteAmount(const std::string& metaboliteName, double amount);
	double getMetaboliteAmount(const std::string& metaboliteName) const;
	const std::map<std::string, double>& getMetabolitePool() const;

	// Resource budget access (integer, set by fair allocation before SSA)
	void setResourceBudget(const std::string& speciesName, int budget);
	int getResourceBudget(const std::string& speciesName) const;
	void clearResourceBudget();

	// Cell geometry
	void setCellVolume(double volume);
	double getCellVolume() const;
	void setCellMass(double mass);
	double getCellMass() const;

	// Simulation progress
	void setCurrentTime(double time);
	double getCurrentTime() const;
	void setStepCount(int stepCount);
	int getStepCount() const;
	void incrementStep();

	// JSON loading from CovertLab/WholeCell data files (MIT)
	bool loadFixedConstants(const std::string& jsonPath);
	bool loadParameters(const std::string& jsonPath);

	// JSON file paths (persisted so the model file remembers them)
	void setFixedConstantsPath(std::string path);
	std::string getFixedConstantsPath() const;
	void setParametersPath(std::string path);
	std::string getParametersPath() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;

private:
	const struct DEFAULT_VALUES {
		double cellVolume = 1.0e-15;  // 1 fL, typical M. genitalium volume
		double cellMass = 1.0e-13;   // ~100 fg, typical M. genitalium mass
		double currentTime = 0.0;
		int stepCount = 0;
		std::string fixedConstantsPath = "";
		std::string parametersPath = "";
	} DEFAULT;

	std::map<std::string, int>    _moleculeCounts;
	std::map<std::string, int>    _initialMoleculeCounts;
	std::map<std::string, double> _metabolitePool;
	std::map<std::string, double> _initialMetabolitePool;
	std::map<std::string, int>    _resourceBudget;

	double _cellVolume = DEFAULT.cellVolume;
	double _cellMass   = DEFAULT.cellMass;
	double _currentTime = DEFAULT.currentTime;
	int    _stepCount   = DEFAULT.stepCount;
	std::string _fixedConstantsPath = DEFAULT.fixedConstantsPath;
	std::string _parametersPath     = DEFAULT.parametersPath;
};

#endif /* WHOLECELLSTATE_H */
