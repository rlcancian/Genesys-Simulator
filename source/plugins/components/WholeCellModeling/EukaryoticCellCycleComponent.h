#ifndef EUKARYOTICCELLCYCLECOMPONENT_H
#define EUKARYOTICCELLCYCLECOMPONENT_H

#include <string>

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/WholeCellModeling/BioCompartment.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"

/*!
 * \brief Coordinates a didactic eukaryotic cell cycle over a shared WholeCellState.
 *
 * EukaryoticCellCycleComponent provides an explicit, event-driven lifecycle
 * layer for budding-yeast-like whole-cell models. It interprets shared state
 * variables and compartment-specific energy proxies, then annotates
 * WholeCellState with a coarse-grained eukaryotic cycle:
 * - `newborn`
 * - `g1_budding`
 * - `s_phase`
 * - `g2_phase`
 * - `m_phase`
 * - `division_ready`
 * - `arrested`
 * - `dead`
 *
 * The component does not execute metabolism, transport, expression, or
 * division itself. Instead, it consumes their outputs from WholeCellState and
 * advances four progress variables stored as pathway activities:
 * - bud growth progress
 * - DNA replication progress
 * - spindle assembly progress
 * - mitotic exit progress
 *
 * This design keeps structural biology in BioCompartment, mutable system state
 * in WholeCellState, and dynamic checkpoint logic in a dedicated
 * ModelComponent.
 */
class EukaryoticCellCycleComponent : public ModelComponent {
public:
	EukaryoticCellCycleComponent(Model* model, std::string name = "");
	virtual ~EukaryoticCellCycleComponent() override = default;

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

	void setCytosolCompartmentName(std::string name);
	std::string getCytosolCompartmentName() const;
	void setNucleusCompartmentName(std::string name);
	std::string getNucleusCompartmentName() const;
	void setMitochondrionCompartmentName(std::string name);
	std::string getMitochondrionCompartmentName() const;
	void setBudCompartmentName(std::string name);
	std::string getBudCompartmentName() const;

	void setEnergyMetaboliteKey(std::string key);
	std::string getEnergyMetaboliteKey() const;
	void setCytosolicEnergyMetaboliteKey(std::string key);
	std::string getCytosolicEnergyMetaboliteKey() const;
	void setMitochondrialEnergyMetaboliteKey(std::string key);
	std::string getMitochondrialEnergyMetaboliteKey() const;
	void setBudEnergyMetaboliteKey(std::string key);
	std::string getBudEnergyMetaboliteKey() const;

	void setGrowthFluxKey(std::string key);
	std::string getGrowthFluxKey() const;
	void setRespirationFluxKey(std::string key);
	std::string getRespirationFluxKey() const;

	void setBudProgressKey(std::string key);
	std::string getBudProgressKey() const;
	void setDnaReplicationProgressKey(std::string key);
	std::string getDnaReplicationProgressKey() const;
	void setSpindleAssemblyProgressKey(std::string key);
	std::string getSpindleAssemblyProgressKey() const;
	void setMitoticExitProgressKey(std::string key);
	std::string getMitoticExitProgressKey() const;

	void setGlobalAtpThreshold(double threshold);
	double getGlobalAtpThreshold() const;
	void setCytosolicAtpThreshold(double threshold);
	double getCytosolicAtpThreshold() const;
	void setMitochondrialAtpThreshold(double threshold);
	double getMitochondrialAtpThreshold() const;
	void setBudAtpThreshold(double threshold);
	double getBudAtpThreshold() const;
	void setGrowthFluxThreshold(double threshold);
	double getGrowthFluxThreshold() const;
	void setRespirationFluxThreshold(double threshold);
	double getRespirationFluxThreshold() const;

	void setBudProgressRate(double rate);
	double getBudProgressRate() const;
	void setDnaReplicationRate(double rate);
	double getDnaReplicationRate() const;
	void setSpindleAssemblyRate(double rate);
	double getSpindleAssemblyRate() const;
	void setMitoticExitRate(double rate);
	double getMitoticExitRate() const;

	void setBudProgressThreshold(double threshold);
	double getBudProgressThreshold() const;
	void setDnaReplicationThreshold(double threshold);
	double getDnaReplicationThreshold() const;
	void setSpindleAssemblyThreshold(double threshold);
	double getSpindleAssemblyThreshold() const;
	void setMitoticExitThreshold(double threshold);
	double getMitoticExitThreshold() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	virtual void _createEditableDataDefinitions() override;

private:
	double _normalizedSignal(double value, double threshold) const;
	double _averageSignal(double a, double b) const;
	double _advanceProgress(const std::string& key, double increment) const;
	void _updateLifecyclePhase();
	void _forwardEntity(Entity* entity);
	bool _validateReferencedCompartment(const std::string& name, std::string* errorMessage) const;
	BioCompartment* _findCompartment(const std::string& name) const;

private:
	const struct DEFAULT_VALUES {
		std::string wholeCellStateName = "";
		double deltaT = 60.0;
		bool advanceWholeCellClock = true;
		std::string cytosolCompartmentName = "cytosol";
		std::string nucleusCompartmentName = "nucleus";
		std::string mitochondrionCompartmentName = "mitochondria";
		std::string budCompartmentName = "bud";
		std::string energyMetaboliteKey = "ATP";
		std::string cytosolicEnergyMetaboliteKey = "ATP_c";
		std::string mitochondrialEnergyMetaboliteKey = "ATP_m";
		std::string budEnergyMetaboliteKey = "ATP_bud";
		std::string growthFluxKey = "biomass_flux";
		std::string respirationFluxKey = "respiration_flux";
		std::string budProgressKey = "bud_growth_progress";
		std::string dnaReplicationProgressKey = "dna_replication_progress";
		std::string spindleAssemblyProgressKey = "spindle_assembly_progress";
		std::string mitoticExitProgressKey = "mitotic_exit_progress";
		double globalAtpThreshold = 0.50;
		double cytosolicAtpThreshold = 0.40;
		double mitochondrialAtpThreshold = 0.35;
		double budAtpThreshold = 0.20;
		double growthFluxThreshold = 4.0;
		double respirationFluxThreshold = 1.0;
		double budProgressRate = 0.30;
		double dnaReplicationRate = 0.24;
		double spindleAssemblyRate = 0.22;
		double mitoticExitRate = 0.20;
		double budProgressThreshold = 1.0;
		double dnaReplicationThreshold = 1.0;
		double spindleAssemblyThreshold = 1.0;
		double mitoticExitThreshold = 1.0;
	} DEFAULT;

	WholeCellState* _wholeCellState = nullptr;
	double _deltaT = DEFAULT.deltaT;
	bool _advanceWholeCellClock = DEFAULT.advanceWholeCellClock;
	std::string _cytosolCompartmentName = DEFAULT.cytosolCompartmentName;
	std::string _nucleusCompartmentName = DEFAULT.nucleusCompartmentName;
	std::string _mitochondrionCompartmentName = DEFAULT.mitochondrionCompartmentName;
	std::string _budCompartmentName = DEFAULT.budCompartmentName;
	std::string _energyMetaboliteKey = DEFAULT.energyMetaboliteKey;
	std::string _cytosolicEnergyMetaboliteKey = DEFAULT.cytosolicEnergyMetaboliteKey;
	std::string _mitochondrialEnergyMetaboliteKey = DEFAULT.mitochondrialEnergyMetaboliteKey;
	std::string _budEnergyMetaboliteKey = DEFAULT.budEnergyMetaboliteKey;
	std::string _growthFluxKey = DEFAULT.growthFluxKey;
	std::string _respirationFluxKey = DEFAULT.respirationFluxKey;
	std::string _budProgressKey = DEFAULT.budProgressKey;
	std::string _dnaReplicationProgressKey = DEFAULT.dnaReplicationProgressKey;
	std::string _spindleAssemblyProgressKey = DEFAULT.spindleAssemblyProgressKey;
	std::string _mitoticExitProgressKey = DEFAULT.mitoticExitProgressKey;
	double _globalAtpThreshold = DEFAULT.globalAtpThreshold;
	double _cytosolicAtpThreshold = DEFAULT.cytosolicAtpThreshold;
	double _mitochondrialAtpThreshold = DEFAULT.mitochondrialAtpThreshold;
	double _budAtpThreshold = DEFAULT.budAtpThreshold;
	double _growthFluxThreshold = DEFAULT.growthFluxThreshold;
	double _respirationFluxThreshold = DEFAULT.respirationFluxThreshold;
	double _budProgressRate = DEFAULT.budProgressRate;
	double _dnaReplicationRate = DEFAULT.dnaReplicationRate;
	double _spindleAssemblyRate = DEFAULT.spindleAssemblyRate;
	double _mitoticExitRate = DEFAULT.mitoticExitRate;
	double _budProgressThreshold = DEFAULT.budProgressThreshold;
	double _dnaReplicationThreshold = DEFAULT.dnaReplicationThreshold;
	double _spindleAssemblyThreshold = DEFAULT.spindleAssemblyThreshold;
	double _mitoticExitThreshold = DEFAULT.mitoticExitThreshold;
};

#endif /* EUKARYOTICCELLCYCLECOMPONENT_H */
