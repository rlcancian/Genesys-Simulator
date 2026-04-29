/*
 * File:   RSimulatorRunner.h
 * Author: GenESyS
 *
 * R integration data definition.
 */

#ifndef RSIMULATORRUNNER_H
#define RSIMULATORRUNNER_H

#include <string>

#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"

/*!
 * \brief Model data definition that executes R code through Rscript.
 *
 * The runner stores a compact R execution configuration, writes a temporary
 * script, executes it with Rscript, and keeps status, exit code, stdout, stderr
 * and response payload inside the model data definition for later inspection.
 */
class RSimulatorRunner : public ModelDataDefinition {
public:
	/*! \brief Creates an R runner bound to a model. */
	RSimulatorRunner(Model* model, std::string name = "");
	/*! \brief Destroys the R runner. */
	virtual ~RSimulatorRunner() = default;

public:
	/*! \brief Loads an R runner from a persistence record. */
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	/*! \brief Returns plugin metadata and system dependency declarations. */
	static PluginInformation* GetPluginInformation();
	/*! \brief Creates a new R runner instance for plugin construction. */
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	/*! \brief Sets the Rscript executable name or path. */
	void setRExecutable(std::string rExecutable);
	/*! \brief Returns the Rscript executable name or path. */
	std::string getRExecutable() const;
	/*! \brief Sets the working directory used for generated scripts and command execution. */
	void setWorkingDirectory(std::string workingDirectory);
	/*! \brief Returns the working directory used for generated scripts and command execution. */
	std::string getWorkingDirectory() const;
	/*! \brief Sets optional R code prepended before the command block. */
	void setPreludeScript(std::string preludeScript);
	/*! \brief Returns optional R code prepended before the command block. */
	std::string getPreludeScript() const;
	/*! \brief Sets the R command/script body to execute. */
	void setCommand(std::string command);
	/*! \brief Returns the R command/script body to execute. */
	std::string getCommand() const;
	/*! \brief Sets the last execution status. */
	void setLastStatus(std::string lastStatus);
	/*! \brief Returns the last execution status. */
	std::string getLastStatus() const;
	/*! \brief Sets the last process exit code. */
	void setLastExitCode(int lastExitCode);
	/*! \brief Returns the last process exit code. */
	int getLastExitCode() const;
	/*! \brief Sets stdout captured from the last R execution. */
	void setLastStdout(std::string lastStdout);
	/*! \brief Returns stdout captured from the last R execution. */
	std::string getLastStdout() const;
	/*! \brief Sets stderr captured from the last R execution. */
	void setLastStderr(std::string lastStderr);
	/*! \brief Returns stderr captured from the last R execution. */
	std::string getLastStderr() const;
	/*! \brief Sets the response payload exposed to other tools. */
	void setLastResponsePayload(std::string lastResponsePayload);
	/*! \brief Returns the response payload exposed to other tools. */
	std::string getLastResponsePayload() const;
	/*! \brief Sets the generated R script filename from the last execution. */
	void setLastScriptFilename(std::string lastScriptFilename);
	/*! \brief Returns the generated R script filename from the last execution. */
	std::string getLastScriptFilename() const;
	/*! \brief Sets the stdout file path from the last execution. */
	void setLastResponseFilename(std::string lastResponseFilename);
	/*! \brief Returns the stdout file path from the last execution. */
	std::string getLastResponseFilename() const;

	/*! \brief Executes the configured R code and updates last-execution fields atomically. */
	bool executeCommand(std::string& errorMessage);

	/*! \brief Returns a compact human-readable description of the runner. */
	virtual std::string show() override;

protected:
	/*! \brief Loads persisted R runner configuration and last-execution state. */
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	/*! \brief Saves R runner configuration and last-execution state. */
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	/*! \brief Validates the runner configuration without executing R. */
	virtual bool _check(std::string& errorMessage) override;
	/*! \brief Creates internal/attached data definitions; currently no-op. */
	virtual void _createInternalAndAttachedData() override;


protected:
	void _doCreateReportStatisticsDataDefinitions();
	void _doCreateEditableDataDefinitions();
	void _doCreateOthersDataDefinitions();


	void _createReportStatisticsDataDefinitions() override;
	void _createEditableDataDefinitions() override;
	void _createOthersDataDefinitions() override;

private:
	const struct DEFAULT_VALUES {
		std::string rExecutable = "Rscript";
		std::string workingDirectory = "";
		std::string preludeScript = "";
		std::string command = "";
		std::string lastStatus = "Idle";
		int lastExitCode = -1;
		std::string lastStdout = "";
		std::string lastStderr = "";
		std::string lastResponsePayload = "";
		std::string lastScriptFilename = "";
		std::string lastResponseFilename = "";
	} DEFAULT;

	std::string _rExecutable = DEFAULT.rExecutable;
	std::string _workingDirectory = DEFAULT.workingDirectory;
	std::string _preludeScript = DEFAULT.preludeScript;
	std::string _command = DEFAULT.command;
	std::string _lastStatus = DEFAULT.lastStatus;
	int _lastExitCode = DEFAULT.lastExitCode;
	std::string _lastStdout = DEFAULT.lastStdout;
	std::string _lastStderr = DEFAULT.lastStderr;
	std::string _lastResponsePayload = DEFAULT.lastResponsePayload;
	std::string _lastScriptFilename = DEFAULT.lastScriptFilename;
	std::string _lastResponseFilename = DEFAULT.lastResponseFilename;
};

#endif /* RSIMULATORRUNNER_H */
