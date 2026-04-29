/*
 * File:   BioRunnerCommand.h
 * Author: GenESyS
 *
 * Component that executes BioSimulatorRunner commands when entities arrive.
 */

#ifndef BIORUNNERCOMMAND_H
#define BIORUNNERCOMMAND_H

#include <string>

#include "kernel/simulator/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/BiochemicalSimulation/BioSimulatorRunner.h"

/*!
 * \brief Component wrapper over BioSimulatorRunner command execution.
 *
 * BioRunnerCommand dispatches one configured command into a referenced
 * BioSimulatorRunner whenever an entity arrives and stores the last runner
 * status/payload metadata for downstream model logic.
 */
class BioRunnerCommand : public ModelComponent {
public:
	BioRunnerCommand(Model* model, std::string name = "");
	virtual ~BioRunnerCommand() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setRunner(BioSimulatorRunner* runner);
	BioSimulatorRunner* getRunner() const;
	void setCommand(std::string command);
	std::string getCommand() const;
	void setLastSucceeded(bool lastSucceeded);
	bool getLastSucceeded() const;
	void setLastStatus(std::string lastStatus);
	std::string getLastStatus() const;
	void setLastMessage(std::string lastMessage);
	std::string getLastMessage() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
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
	void _forwardEntity(Entity* entity);

private:
	const struct DEFAULT_VALUES {
		std::string runnerName = "";
		std::string command = "validateModel()";
		bool lastSucceeded = false;
		std::string lastStatus = "";
		std::string lastMessage = "";
	} DEFAULT;

	BioSimulatorRunner* _runner = nullptr;
	std::string _command = DEFAULT.command;
	bool _lastSucceeded = DEFAULT.lastSucceeded;
	std::string _lastStatus = DEFAULT.lastStatus;
	std::string _lastMessage = DEFAULT.lastMessage;
};

#endif /* BIORUNNERCOMMAND_H */
