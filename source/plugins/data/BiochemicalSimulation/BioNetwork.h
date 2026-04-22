#ifndef BIONETWORK_H
#define BIONETWORK_H

#include <string>
#include <vector>

#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"
#include "tools/BioSimulationAnalysis.h"
#include "tools/BioSimulationResult.h"

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
class BioNetwork : public ModelDataDefinition {
public:
	BioNetwork(Model* model, std::string name = "");
	virtual ~BioNetwork() override = default;

public:
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setStartTime(double startTime);
	double getStartTime() const;
	void setStopTime(double stopTime);
	double getStopTime() const;
	void setStepSize(double stepSize);
	double getStepSize() const;
	void setCurrentTime(double currentTime);
	double getCurrentTime() const;
	void setAutoSchedule(bool autoSchedule);
	bool getAutoSchedule() const;
	void setLastStatus(std::string lastStatus);
	std::string getLastStatus() const;
	void setLastErrorMessage(std::string lastErrorMessage);
	std::string getLastErrorMessage() const;
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

	bool simulate(std::string& errorMessage);
	bool simulate(double startTime, double stopTime, double stepSize, std::string& errorMessage);
	bool advanceOneStep(std::string& errorMessage);

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;

private:
	void handleInternalEvent(void* parameter);
	void scheduleNextInternalEvent();
	bool collectSpecies(std::vector<BioSpecies*>& species, std::string& errorMessage) const;
	bool collectReactions(std::vector<BioReaction*>& reactions, std::string& errorMessage) const;
	bool buildSystem(const std::vector<BioSpecies*>& species, const std::vector<BioReaction*>& reactions, MassActionOdeSystem* system, std::string& errorMessage) const;
	void updatePayload(const std::vector<BioSpecies*>& species);
	void configureSimulationResult(const std::vector<BioSpecies*>& species);
	void appendSimulationResultSample(const std::vector<BioSpecies*>& species);

private:
	const struct DEFAULT_VALUES {
		double startTime = 0.0;
		double stopTime = 1.0;
		double stepSize = 0.1;
		double currentTime = 0.0;
		bool autoSchedule = false;
		std::string lastStatus = "Idle";
		std::string lastErrorMessage = "";
		std::string lastResponsePayload = "";
	} DEFAULT;

	double _startTime = DEFAULT.startTime;
	double _stopTime = DEFAULT.stopTime;
	double _stepSize = DEFAULT.stepSize;
	double _currentTime = DEFAULT.currentTime;
	bool _autoSchedule = DEFAULT.autoSchedule;
	std::string _lastStatus = DEFAULT.lastStatus;
	std::string _lastErrorMessage = DEFAULT.lastErrorMessage;
	std::string _lastResponsePayload = DEFAULT.lastResponsePayload;
	std::vector<std::string> _speciesNames;
	std::vector<std::string> _reactionNames;
	BioSimulationResult _lastSimulationResult;
};

#endif /* BIONETWORK_H */
