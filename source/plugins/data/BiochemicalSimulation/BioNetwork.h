#ifndef BIONETWORK_H
#define BIONETWORK_H

#include <string>
#include <vector>

#include "plugins/data/Continuous/ContinuousModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"
#include "tools/Biochemical/BioSimulationAnalysis.h"
#include "tools/Biochemical/BioSimulationResult.h"

class BioReaction;
class BioSpecies;
class MassActionOdeSystem;

/**
 * Native deterministic biochemical network runner.
 *
 * BioNetwork advances BioSpecies with fixed-step RK4 over a mass-action ODE
 * system. Networks may define explicit BioSpecies and BioReaction membership by
 * name; when a membership list is empty, the runner falls back to discovering
 * every registered definition of that type in the owning model for compatibility
 * with existing models.
 */
class BioNetwork : public ContinuousModelDataDefinition {
public:
	BioNetwork(Model* model, std::string name = "");
	virtual ~BioNetwork() override = default;

public:
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setLastResponsePayload(std::string lastResponsePayload);
	std::string getLastResponsePayload() const;
	void addSpecies(std::string speciesName);
	void addReaction(std::string reactionName);
	void clearSpecies();
	void clearReactions();
	const std::vector<std::string>& getSpeciesNames() const;
	const std::vector<std::string>& getReactionNames() const;
	const BioSimulationResult& getLastSimulationResult() const;
	bool getSpeciesTimeCourseDataset(const std::string& speciesName, SimulationResultsDataset* dataset,
	                                 std::string* errorMessage = nullptr) const;
	bool buildOdeSystemForAnalysis(MassActionOdeSystem* system, std::string& errorMessage) const;
	bool getStoichiometryMatrix(BioStoichiometryMatrix* matrix, std::string* errorMessage = nullptr) const;
	bool getReactionRateTimeCourse(BioReactionRateTimeCourse* timeCourse, std::string* errorMessage = nullptr) const;
	bool checkLastSampleSteadyState(double tolerance, BioSteadyStateCheck* check, std::string* errorMessage = nullptr) const;
	bool scanLocalParameterSensitivity(double relativeStep, double absoluteStep, BioSensitivityScan* scan,
	                                   std::string* errorMessage = nullptr) const;


	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	void initializeContinuousState() override;
	void onContinuousStepCompleted() override;
	bool evolveState(double currentTime, double dt, std::string& errorMessage) override;


protected:
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	/*!
	 * \brief Refreshes editable associations to referenced biochemical definitions.
	 *
	 * BioNetwork stores model membership by name for persistence. During model
	 * checking these associations keep referenced species, reactions, and
	 * parameters from being pruned as orphan ModelDataDefinitions.
	 */
	virtual void _createEditableDataDefinitions() override;
	// virtual void _createAttachedAttributes() override;

private:
	bool collectSpecies(std::vector<BioSpecies*>& species, std::string& errorMessage) const;
	bool collectReactions(std::vector<BioReaction*>& reactions, std::string& errorMessage) const;
	bool buildSystem(const std::vector<BioSpecies*>& species, const std::vector<BioReaction*>& reactions, MassActionOdeSystem* system, std::string& errorMessage) const;
	void updatePayload(const std::vector<BioSpecies*>& species);
	void configureSimulationResult(const std::vector<BioSpecies*>& species);
	void appendSimulationResultSample(const std::vector<BioSpecies*>& species);

private:
	const struct DEFAULT_VALUES {
		std::string lastResponsePayload = "";
	} DEFAULT;

	std::string _lastResponsePayload = DEFAULT.lastResponsePayload;
	std::vector<std::string> _speciesNames;
	std::vector<std::string> _reactionNames;
	BioSimulationResult _lastSimulationResult;
};

#endif /* BIONETWORK_H */
