/*
 * File:   BacteriaColony.h
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#ifndef BACTERIACOLONY_H
#define BACTERIACOLONY_H

#include "kernel/simulator/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "kernel/util/Util.h"
#include "plugins/data/BiochemicalSimulation/BioNetwork.h"
#include "plugins/data/BiologicalModeling/GroProgram.h"
#include "plugins/data/BiologicalModeling/GroProgramRuntime.h"
#include "plugins/data/BiologicalModeling/BacteriaSignalGrid.h"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

/*!
 * \brief First GenESyS component for Gro-inspired bacteria colony simulation.
 *
 * BacteriaColony owns an internal biological simulation clock and a population
 * state that is independent from ModelSimulation global event time in this
 * first slice. The component references a reusable GroProgram and executes the
 * first supported runtime commands through plugin-side parser/runtime helpers.
 */
class BacteriaColony : public ModelComponent {
public:
	struct BacteriumState {
		unsigned int id = 0;
		unsigned int parentId = 0;
		unsigned int generation = 0;
		unsigned int divisionCount = 0;
		double birthTime = 0.0;
		double lastUpdateTime = 0.0;
		double lastDivisionTime = 0.0;
		unsigned int gridX = 0;
		unsigned int gridY = 0;
		bool alive = true;
		std::map<std::string, double> runtimeVariables;
	};

public:
	BacteriaColony(Model* model, std::string name = "");
	virtual ~BacteriaColony() override = default;

public:
	virtual std::string show() override;

public: // static plugin interface
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	/*! \brief Sets the reusable Gro program used by this colony. */
	void setGroProgram(GroProgram* groProgram);
	/*! \brief Returns the reusable Gro program used by this colony. */
	GroProgram* getGroProgram() const;
	/*! \brief Sets the internal colony time increment used by one colony step. */
	void setSimulationStep(double simulationStep);
	/*! \brief Returns the internal colony time increment. */
	double getSimulationStep() const;
	/*! \brief Sets the initial internal colony time for each replication. */
	void setInitialColonyTime(double initialColonyTime);
	/*! \brief Returns the initial internal colony time. */
	double getInitialColonyTime() const;
	/*! \brief Sets the final internal colony time reached before forwarding the entity. */
	void setFinalColonyTime(double finalColonyTime);
	/*! \brief Returns the final internal colony time reached before forwarding the entity. */
	double getFinalColonyTime() const;
	/*! \brief Sets the time unit used by the colony time-related attributes. */
	void setColonyTimeUnit(Util::TimeUnit colonyTimeUnit);
	/*! \brief Returns the time unit used by the colony time-related attributes. */
	Util::TimeUnit getColonyTimeUnit() const;
	/*! \brief Returns the current internal colony time. */
	double getColonyTime() const;
	/*! \brief Sets the initial bacteria population summary for each replication. */
	void setInitialPopulation(unsigned int initialPopulation);
	/*! \brief Returns the initial bacteria population summary. */
	unsigned int getInitialPopulation() const;
	/*! \brief Returns the current bacteria population summary. */
	unsigned int getPopulationSize() const;
	/*! \brief Sets the optional BioNetwork reused as the colony biochemical state. */
	void setBioNetwork(BioNetwork* bioNetwork);
	/*! \brief Returns the optional BioNetwork reused as the colony biochemical state. */
	BioNetwork* getBioNetwork() const;
	/*! \brief Sets the reusable signal-grid definition used by this colony. */
	void setSignalGrid(BacteriaSignalGrid* signalGrid);
	/*! \brief Returns the reusable signal-grid definition used by this colony. */
	BacteriaSignalGrid* getSignalGrid() const;
	/*! \brief Returns the number of internal bacteria currently owned by the colony. */
	std::size_t getInternalBacteriaCount() const;
	/*! \brief Returns one internal bacterium state by zero-based index. */
	const BacteriumState& getBacteriumState(std::size_t index) const;
	/*! \brief Returns the age of one internal bacterium at the current colony time. */
	double getBacteriumAge(std::size_t index) const;
	/*! \brief Returns the last stored value for one bacterium-scoped runtime variable. */
	double getBacteriumRuntimeVariableValue(std::size_t index, const std::string& variableName) const;
	/*! \brief Tells whether one bacterium currently stores one runtime scalar variable. */
	bool hasBacteriumRuntimeVariable(std::size_t index, const std::string& variableName) const;
	/*! \brief Returns the last stored value for one runtime scalar variable. */
	double getRuntimeVariableValue(const std::string& variableName) const;
	/*! \brief Tells whether the colony currently stores one runtime scalar variable. */
	bool hasRuntimeVariable(const std::string& variableName) const;
	/*! \brief Returns the runtime signal value stored at one grid coordinate. */
	double getSignalValueAt(unsigned int x, unsigned int y) const;
	/*! \brief Returns the runtime signal value seen by one bacterium at its current cell. */
	double getBacteriumLocalSignal(std::size_t index) const;
	/*! \brief Sets the discrete spatial grid width reserved for the first colony model. */
	void setGridWidth(unsigned int gridWidth);
	/*! \brief Returns the discrete spatial grid width. */
	unsigned int getGridWidth() const;
	/*! \brief Sets the discrete spatial grid height reserved for the first colony model. */
	void setGridHeight(unsigned int gridHeight);
	/*! \brief Returns the discrete spatial grid height. */
	unsigned int getGridHeight() const;
	/*! \brief Advances the internal colony clock by one configured simulation step. */
	double advanceColonyTime();
	/*! \brief Executes the configured Gro program once against the current colony state. */
	GroProgramRuntime::ExecutionResult executeGroProgram();

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	virtual void _createInternalAndAttachedData() override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;


protected:
	void _doCreateReportStatisticsDataDefinitions();
	void _doCreateEditableDataDefinitions();
	void _doCreateOthersDataDefinitions();


	void _createReportStatisticsDataDefinitions() override;
	void _createEditableDataDefinitions() override;
	void _createOthersDataDefinitions() override;

private:
	const struct DEFAULT_VALUES {
		const std::string groProgramName = "";
		const std::string bioNetworkName = "";
		const std::string signalGridName = "";
		const double simulationStep = 0.01;
		const double initialColonyTime = 0.0;
		const double finalColonyTime = 1.0;
		const Util::TimeUnit colonyTimeUnit = Util::TimeUnit::second;
		const unsigned int initialPopulation = 1;
		const unsigned int gridWidth = 1;
		const unsigned int gridHeight = 1;
	} DEFAULT;

	GroProgram* _groProgram = nullptr;
	BioNetwork* _bioNetwork = nullptr;
	BacteriaSignalGrid* _signalGrid = nullptr;
	double _simulationStep = DEFAULT.simulationStep;
	double _initialColonyTime = DEFAULT.initialColonyTime;
	double _finalColonyTime = DEFAULT.finalColonyTime;
	Util::TimeUnit _colonyTimeUnit = DEFAULT.colonyTimeUnit;
	double _colonyTime = DEFAULT.initialColonyTime;
	unsigned int _initialPopulation = DEFAULT.initialPopulation;
	unsigned int _populationSize = DEFAULT.initialPopulation;
	unsigned int _gridWidth = DEFAULT.gridWidth;
	unsigned int _gridHeight = DEFAULT.gridHeight;
	unsigned int _nextBacteriumId = 1;
	std::vector<BacteriumState> _bacteria;
	std::map<std::string, double> _runtimeVariables;
	std::vector<double> _signalField;

private:
	bool _resetRuntimeSignalField(std::string& errorMessage);
	bool _collectBioNetworkSpecies(std::vector<BioSpecies*>& species, std::string& errorMessage) const;
	bool _collectBioNetworkSpeciesByIdentifier(std::map<std::string, BioSpecies*>& speciesByIdentifier,
	                                           std::string& errorMessage) const;
	void _appendBioNetworkContextVariables(GroProgramRuntimeState& runtimeState, std::string& errorMessage) const;
	bool _applyBioNetworkAssignments(const std::map<std::string, double>& assignedVariables, std::string& errorMessage);
	void _removeBioNetworkAssignmentVariables(std::map<std::string, double>& variables) const;
	bool _advanceBioNetworkStep(double stepSize, std::string& errorMessage);
	std::size_t _signalIndex(unsigned int x, unsigned int y) const;
	double _signalValueAt(unsigned int x, unsigned int y) const;
	void _setSignalValueAt(unsigned int x, unsigned int y, double value);
	void _addSignalAt(unsigned int x, unsigned int y, double value);
	double _computeNeighborSignalSum(unsigned int x, unsigned int y) const;
	unsigned int _computeLocalBacteriaCount(unsigned int x, unsigned int y) const;
	void _applySignalFieldStep();
	void _applyBacteriumSignalMutations(const BacteriumState& bacterium,
	                                    const std::vector<GroProgramRuntime::SignalMutation>& mutations);
	void _rebuildInternalBacteria(unsigned int populationSize);
	void _resizeInternalBacteria(unsigned int populationSize);
	void _applyRuntimePopulationMutations(const std::vector<GroProgramRuntime::PopulationMutation>& mutations,
	                                      unsigned int finalPopulationSize);
	bool _executeBacteriumScopedGroProgram(const GroProgramIr& ir, GroProgramRuntime::ExecutionResult& result);
	bool _containsBacteriumScopedOnlyUnsupportedCommand(const std::vector<GroProgramIr::Command>& commands,
	                                                    std::string& unsupportedCommand) const;
	std::size_t _findBacteriumIndexById(unsigned int bacteriumId) const;
	GroProgramRuntimeState _createBacteriumRuntimeState(const BacteriumState& bacterium,
	                                                    std::size_t bacteriumIndex) const;
	void _applyBacteriumScopedPopulationMutations(unsigned int bacteriumId,
	                                              unsigned int parentGeneration,
	                                              const std::vector<GroProgramRuntime::PopulationMutation>& mutations,
	                                              GroProgramRuntime::ExecutionResult& result);
	void _appendBacterium(unsigned int parentId = 0, unsigned int generation = 0);
	void _removeBacteria(unsigned int amount);
	bool _removeBacteriumById(unsigned int bacteriumId);
	void _refreshBacteriaUpdateTime();
	void _rebuildBacteriaGridPositions();
	void _assignBacteriumGridPosition(BacteriumState& bacterium, std::size_t index) const;
};

#endif /* BACTERIACOLONY_H */
