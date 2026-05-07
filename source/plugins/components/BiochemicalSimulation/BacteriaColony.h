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
#include "plugins/data/BiochemicalSimulation/GroProgram.h"
#include "plugins/data/BiochemicalSimulation/GroProgramRuntime.h"
#include "plugins/data/BiochemicalSimulation/BacteriaSignalGrid.h"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

/*!
 * \brief First GenESyS component for Gro-inspired bacteria colony simulation.
 *
 * BacteriaColony owns bacteria and signal-field runtime state while using the
 * enclosing ModelSimulation time as the single colony clock. The component
 * references a reusable GroProgram and executes the first supported runtime
 * commands through plugin-side parser/runtime helpers.
 */
class BacteriaColony : public ModelComponent {
public:
	struct BarrierSegment {
		double x1 = 0.0;
		double y1 = 0.0;
		double x2 = 0.0;
		double y2 = 0.0;
	};

	struct BacteriumState {
		unsigned int id = 0;
		unsigned int parentId = 0;
		unsigned int generation = 0;
		std::string programName = "";
		std::vector<double> programArguments;
		unsigned int divisionCount = 0;
		double birthTime = 0.0;
		double lastUpdateTime = 0.0;
		double lastDivisionTime = 0.0;
		double positionX = 0.0;
		double positionY = 0.0;
		double directionRadians = 0.0;
		double volume = 1.0;
		double size = 1.0;
		double gfp = 0.0;
		double rfp = 0.0;
		double yfp = 0.0;
		double cfp = 0.0;
		double speed = 0.1;
		unsigned int tickCount = 0;
		unsigned int gridX = 0;
		unsigned int gridY = 0;
		bool hasExplicitGridPosition = false;
		bool justDivided = false;
		bool daughter = false;
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
	/*! \brief Sets the number of colony steps processed before forwarding one entity. */
	void setNumSteps(unsigned int numSteps);
	/*! \brief Returns the number of colony steps processed before forwarding one entity. */
	unsigned int getNumSteps() const;
	/*! \brief Sets the time unit used by the colony time-related attributes. */
	void setColonyTimeUnit(Util::TimeUnit colonyTimeUnit);
	/*! \brief Returns the time unit used by the colony time-related attributes. */
	Util::TimeUnit getColonyTimeUnit() const;
	/*! \brief Returns the current colony time from the enclosing ModelSimulation. */
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
	/*! \brief Returns the continuous X coordinate used by the viewer. */
	double getBacteriumPositionX(std::size_t index) const;
	/*! \brief Returns the continuous Y coordinate used by the viewer. */
	double getBacteriumPositionY(std::size_t index) const;
	/*! \brief Returns the bacterium direction in radians. */
	double getBacteriumDirectionRadians(std::size_t index) const;
	/*! \brief Returns the current bacterium volume. */
	double getBacteriumVolume(std::size_t index) const;
	/*! \brief Returns the current bacterium size used for rendering. */
	double getBacteriumSize(std::size_t index) const;
	/*! \brief Returns the current GFP amount used for rendering. */
	double getBacteriumGfp(std::size_t index) const;
	/*! \brief Returns the current RFP amount used for rendering. */
	double getBacteriumRfp(std::size_t index) const;
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
	/*! \brief Enables or disables the colony chemostat mode used by Gro microfluidics. */
	void setChemostatMode(bool enabled);
	/*! \brief Returns whether the colony chemostat mode is enabled. */
	bool getChemostatMode() const;
	/*! \brief Adds one barrier segment to the colony environment. */
	void addBarrier(double x1, double y1, double x2, double y2);
	/*! \brief Returns the current barrier segments used by the viewer. */
	const std::vector<BarrierSegment>& getBarriers() const;
	/*! \brief Returns the latest per-cell values produced by map_to_cells, if any. */
	const std::vector<double>& getMappedCellValues() const;
	/*! \brief Returns the expression used by the latest map_to_cells call, if any. */
	const std::string& getMappedCellExpression() const;
	/*! \brief Sets the discrete spatial grid width reserved for the first colony model. */
	void setGridWidth(unsigned int gridWidth);
	/*! \brief Returns the discrete spatial grid width. */
	unsigned int getGridWidth() const;
	/*! \brief Sets the discrete spatial grid height reserved for the first colony model. */
	void setGridHeight(unsigned int gridHeight);
	/*! \brief Returns the discrete spatial grid height. */
	unsigned int getGridHeight() const;
	/*! \brief Executes the configured Gro program once against the current colony state. */
	GroProgramRuntime::ExecutionResult executeGroProgram();

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	// virtual void _createInternalAndAttachedData() override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;


protected:
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	// virtual void _createEditableDataDefinitions() override;
	virtual void _createAttachedAttributes() override;

private:
	const struct DEFAULT_VALUES {
		const std::string groProgramName = "";
		const std::string bioNetworkName = "";
		const std::string signalGridName = "";
		const double simulationStep = 0.01;
		const unsigned int numSteps = 100;
		const Util::TimeUnit colonyTimeUnit = Util::TimeUnit::second;
		const unsigned int initialPopulation = 1;
		const unsigned int gridWidth = 1;
		const unsigned int gridHeight = 1;
	} DEFAULT;

	GroProgram* _groProgram = nullptr;
	BioNetwork* _bioNetwork = nullptr;
	BacteriaSignalGrid* _signalGrid = nullptr;
	double _simulationStep = DEFAULT.simulationStep;
	unsigned int _currentStep = 0;
	unsigned int _numSteps = DEFAULT.numSteps;
	Util::TimeUnit _colonyTimeUnit = DEFAULT.colonyTimeUnit;
	unsigned int _initialPopulation = DEFAULT.initialPopulation;
	unsigned int _populationSize = DEFAULT.initialPopulation;
	unsigned int _gridWidth = DEFAULT.gridWidth;
	unsigned int _gridHeight = DEFAULT.gridHeight;
	unsigned int _nextBacteriumId = 1;
	unsigned int _colonyTickCount = 0;
	bool _chemostatMode = false;
	std::vector<BacteriumState> _bacteria;
	std::map<std::string, double> _runtimeVariables;
	std::vector<BarrierSegment> _barriers;
	std::vector<double> _mappedCellValues;
	std::string _mappedCellExpression;
	std::vector<double> _signalField;
	struct GroSeedDefinition {
		unsigned int gridX = 0;
		unsigned int gridY = 0;
		std::string programName = "";
		std::vector<double> programArguments;
	};
	std::vector<GroSeedDefinition> _groSeedDefinitions;
	std::string _groSeedSignature = "";
	bool _groSeedsApplied = false;

private:
	// SignalGrid remains the authoritative source for spatial configuration
	// when attached; colony timing is always read from ModelSimulation.
	bool _usesSignalGridDimensions() const;
	void _synchronizeGridDimensionsFromSignalGrid();
	bool _resetRuntimeSignalField(std::string& errorMessage);
	bool _collectBioNetworkSpecies(std::vector<BioSpecies*>& species, std::string& errorMessage) const;
	bool _collectBioNetworkSpeciesByIdentifier(std::map<std::string, BioSpecies*>& speciesByIdentifier,
	                                           std::string& errorMessage) const;
	void _appendBioNetworkContextVariables(GroProgramRuntimeState& runtimeState, std::string& errorMessage) const;
	bool _applyBioNetworkAssignments(const std::map<std::string, double>& assignedVariables, std::string& errorMessage);
	void _removeBioNetworkAssignmentVariables(std::map<std::string, double>& variables) const;
	bool _refreshRuntimeConfigurationFromGroProgram(std::string& errorMessage);
	void _ensureGroSeededBacteriaInitialized();
	void _rebuildInternalBacteriaFromGroSeeds();
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
	bool _applyColonyMutations(const std::vector<GroProgramRuntime::ColonyMutation>& mutations,
	                           GroProgramRuntime::ExecutionResult& result,
	                           bool allowStructureMutations,
	                           std::string& errorMessage);
	bool _executeSeededNamedGroPrograms(const GroProgramIr& ir, GroProgramRuntime::ExecutionResult& result);
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
	void _appendBacterium(unsigned int parentId = 0, unsigned int generation = 0,
	                     const std::string& programName = "");
	void _appendBacterium(const GroSeedDefinition& seedDefinition);
	void _removeBacteria(unsigned int amount);
	bool _removeBacteriumById(unsigned int bacteriumId);
	void _refreshBacteriaUpdateTime();
	void _applyBacteriumGrowth(BacteriumState& bacterium) const;
	void _rebuildBacteriaGridPositions();
	void _assignBacteriumGridPosition(BacteriumState& bacterium, std::size_t index) const;
	void _initializeBacteriumPhenotype(BacteriumState& bacterium, std::size_t index) const;
	void _syncBacteriumSpatialState(BacteriumState& bacterium, const GroProgramRuntimeState& runtimeState) const;
	void _updateBacteriumSpatialMotion(BacteriumState& bacterium) const;
	void _setBacteriumPosition(BacteriumState& bacterium, double x, double y) const;
	double _clampPositionX(double value) const;
	double _clampPositionY(double value) const;
	void _bindProgramArguments(GroProgramRuntimeState& runtimeState,
	                           const std::vector<std::string>& parameterNames,
	                           const std::vector<double>& arguments) const;
};

#endif /* BACTERIACOLONY_H */
