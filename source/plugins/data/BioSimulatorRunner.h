/*
 * File:   BioSimulatorRunner.h
 * Author: GenESyS
 *
 * Structural biochemical simulator runner data definition.
 */

#ifndef BIOSIMULATORRUNNER_H
#define BIOSIMULATORRUNNER_H

#include <string>

#include "../../kernel/simulator/ModelDataDefinition.h"
#include "../../kernel/simulator/PluginInformation.h"

class BioSimulatorRunner : public ModelDataDefinition {
public:
	BioSimulatorRunner(Model* model, std::string name = "");
	virtual ~BioSimulatorRunner() = default;

public:
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setBackend(std::string backend);
	std::string getBackend() const;
	void setModelSourceType(std::string modelSourceType);
	std::string getModelSourceType() const;
	void setModelSource(std::string modelSource);
	std::string getModelSource() const;
	void setCommand(std::string command);
	std::string getCommand() const;
	void setLastStatus(std::string lastStatus);
	std::string getLastStatus() const;
	void setLastErrorMessage(std::string lastErrorMessage);
	std::string getLastErrorMessage() const;
	void setLastResponsePayload(std::string lastResponsePayload);
	std::string getLastResponsePayload() const;
	void setLastResponseFilename(std::string lastResponseFilename);
	std::string getLastResponseFilename() const;
	void setWorkingDirectory(std::string workingDirectory);
	std::string getWorkingDirectory() const;
	void setWorkingInputFilename(std::string workingInputFilename);
	std::string getWorkingInputFilename() const;
	void setWorkingOutputFilename(std::string workingOutputFilename);
	std::string getWorkingOutputFilename() const;
	void setEndpointOrLibrary(std::string endpointOrLibrary);
	std::string getEndpointOrLibrary() const;
	void setTimeoutSeconds(unsigned int timeoutSeconds);
	unsigned int getTimeoutSeconds() const;
	void setAutoValidateModel(bool autoValidateModel);
	bool getAutoValidateModel() const;

	bool executeCommand(std::string& errorMessage);

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _createInternalAndAttachedData() override;

private:
	const struct DEFAULT_VALUES {
		std::string backend = "RoadRunnerEmbedded";
		std::string modelSourceType = "SBMLString";
		std::string modelSource = "";
		std::string command = "";
		std::string lastStatus = "Idle";
		std::string lastErrorMessage = "";
		std::string lastResponsePayload = "";
		std::string lastResponseFilename = "";
		std::string workingDirectory = "";
		std::string workingInputFilename = "biosim_input.xml";
		std::string workingOutputFilename = "biosim_output.json";
		std::string endpointOrLibrary = "";
		unsigned int timeoutSeconds = 30;
		bool autoValidateModel = true;
	} DEFAULT;

	std::string _backend = DEFAULT.backend;
	std::string _modelSourceType = DEFAULT.modelSourceType;
	std::string _modelSource = DEFAULT.modelSource;
	std::string _command = DEFAULT.command;
	std::string _lastStatus = DEFAULT.lastStatus;
	std::string _lastErrorMessage = DEFAULT.lastErrorMessage;
	std::string _lastResponsePayload = DEFAULT.lastResponsePayload;
	std::string _lastResponseFilename = DEFAULT.lastResponseFilename;
	std::string _workingDirectory = DEFAULT.workingDirectory;
	std::string _workingInputFilename = DEFAULT.workingInputFilename;
	std::string _workingOutputFilename = DEFAULT.workingOutputFilename;
	std::string _endpointOrLibrary = DEFAULT.endpointOrLibrary;
	unsigned int _timeoutSeconds = DEFAULT.timeoutSeconds;
	bool _autoValidateModel = DEFAULT.autoValidateModel;
};

#endif /* BIOSIMULATORRUNNER_H */
