/*
 * File:   OctaveRunner.h
 * Author: rlcancian
 *
 * Octave integration data definition.
 */

#ifndef OCTAVERUNNER_H
#define OCTAVERUNNER_H

#include <string>

#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"

/*!
 * \brief Model data definition that executes Octave code through Rscript.
 *
 * The runner stores a compact Octave execution configuration, writes a temporary
 * script, executes it with Rscript, and keeps status, exit code, stdout, stderr
 * and response payload inside the model data definition for later inspection.
 */
class OctaveRunner : public ModelDataDefinition {
public:
	/*! \brief Creates an Octave Runner bound to a model. */
	OctaveRunner(Model* model, std::string name = "");
	/*! \brief Destroys the Octave Runner. */
	virtual ~OctaveRunner() = default;

public:
	/*! \brief Loads an Octave Runner from a persistence record. */
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	/*! \brief Returns plugin metadata and system dependency declarations. */
	static PluginInformation* GetPluginInformation();
	/*! \brief Creates a new Octave Runner instance for plugin construction. */
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	/*! \brief Sets the Rscript executable name or path. */
	void setOctaveExecutable(std::string octaveExecutable);
	/*! \brief Returns the Rscript executable name or path. */
	std::string getOctaveExecutable() const;
	/*! \brief Sets the working directory used for generated scripts and command execution. */
	void setWorkingDirectory(std::string workingDirectory);
	/*! \brief Returns the working directory used for generated scripts and command execution. */
	std::string getWorkingDirectory() const;
	/*! \brief Sets optional Octave code prepended before the command block. */
	void setPreludeScript(std::string preludeScript);
	/*! \brief Returns optional Octave code prepended before the command block. */
	std::string getPreludeScript() const;
	/*! \brief Sets the Octave command/script body to execute. */
	void setCommand(std::string command);
	/*! \brief Returns the Octave command/script body to execute. */
	std::string getCommand() const;
	/*! \brief Sets the last execution status. */
	void setLastStatus(std::string lastStatus);
	/*! \brief Returns the last execution status. */
	std::string getLastStatus() const;
	/*! \brief Sets the last process exit code. */
	void setLastExitCode(int lastExitCode);
	/*! \brief Returns the last process exit code. */
	int getLastExitCode() const;
	/*! \brief Sets stdout captured from the last Octave execution. */
	void setLastStdout(std::string lastStdout);
	/*! \brief Returns stdout captured from the last Octave execution. */
	std::string getLastStdout() const;
	/*! \brief Sets stderr captured from the last Octave execution. */
	void setLastStderr(std::string lastStderr);
	/*! \brief Returns stderr captured from the last Octave execution. */
	std::string getLastStderr() const;
	/*! \brief Sets the response payload exposed to other tools. */
	void setLastResponsePayload(std::string lastResponsePayload);
	/*! \brief Returns the response payload exposed to other tools. */
	std::string getLastResponsePayload() const;
	/*! \brief Sets the generated Octave script filename from the last execution. */
	void setLastScriptFilename(std::string lastScriptFilename);
	/*! \brief Returns the generated Octave script filename from the last execution. */
	std::string getLastScriptFilename() const;
	/*! \brief Sets the stdout file path from the last execution. */
	void setLastResponseFilename(std::string lastResponseFilename);
	/*! \brief Returns the stdout file path from the last execution. */
	std::string getLastResponseFilename() const;

	/*! \brief Executes the configured Octave code and updates last-execution fields atomically. */
	bool executeCommand(std::string& errorMessage);

	/*! \brief Returns a compact human-readable description of the runner. */
	virtual std::string show() override;

protected:
	/*! \brief Loads persisted Octave Runner configuration and last-execution state. */
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	/*! \brief Saves Octave Runner configuration and last-execution state. */
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	/*! \brief Validates the runner configuration without executing Octave. */
	virtual bool _check(std::string& errorMessage) override;
	/*! \brief Creates internal/attached data definitions; currently no-op. */
	// virtual void _createInternalAndAttachedData() override;


protected:
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	// virtual void _createEditableDataDefinitions() override;
	// virtual void _createAttachedAttributes() override;

private:
	const struct DEFAULT_VALUES {
		std::string octaveExecutable = "Ocavescript";
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

	std::string _octaveExecutable = DEFAULT.octaveExecutable;
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

#endif /* OCTAVERUNNER_H */
