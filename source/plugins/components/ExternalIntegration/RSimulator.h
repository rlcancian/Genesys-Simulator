/*
 * File:   RSimulator.h
 * Author: GenESyS
 *
 * Component that executes R commands when entities arrive.
 */

#ifndef RSIMULATOR_H
#define RSIMULATOR_H

#include <list>
#include <string>

#include "kernel/simulator/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/ExternalIntegration/RSimulatorRunner.h"

/*!
 * \brief Component that sends configured R commands to an internal RSimulatorRunner.
 *
 * Each arriving entity triggers the configured command list. Commands are
 * executed sequentially by the internal RSimulatorRunner, and the first version
 * exposes results through simulation trace messages before forwarding the entity.
 */
class RSimulator : public ModelComponent {
public:
	/*! \brief Creates an RSimulator component attached to a model. */
	RSimulator(Model* model, std::string name = "");
	/*! \brief Destroys the RSimulator component. */
	virtual ~RSimulator() = default;

public:
	/*! \brief Sets the Rscript executable name or path used by the internal runner. */
	void setRExecutable(std::string rExecutable);
	/*! \brief Returns the Rscript executable name or path used by the internal runner. */
	std::string getRExecutable() const;
	/*! \brief Sets the working directory used by generated R scripts. */
	void setWorkingDirectory(std::string workingDirectory);
	/*! \brief Returns the working directory used by generated R scripts. */
	std::string getWorkingDirectory() const;
	/*! \brief Sets optional R code prepended to every command. */
	void setPreludeScript(std::string preludeScript);
	/*! \brief Returns optional R code prepended to every command. */
	std::string getPreludeScript() const;
	/*! \brief Appends one R command to the execution list. */
	void insertCommand(std::string command);
	/*! \brief Replaces the command list with the provided commands. */
	void setCommands(std::list<std::string> commands);
	/*! \brief Returns the command list used when entities arrive. */
	List<std::string>* getCommands() const;
	/*! \brief Replaces the command list using one command per non-empty line. */
	void setCommandList(std::string commandList);
	/*! \brief Returns commands serialized as one command per line for GUI editing. */
	std::string getCommandList() const;
	/*! \brief Returns the internal runner, creating it first if necessary. */
	RSimulatorRunner* getRunner();
	/*! \brief Returns a compact human-readable description of the component. */
	virtual std::string show() override;

public:
	/*! \brief Returns plugin metadata for the RSimulator component. */
	static PluginInformation* GetPluginInformation();
	/*! \brief Loads an RSimulator component from persistence. */
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	/*! \brief Creates a new RSimulator component instance. */
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	/*! \brief Executes R commands for the arriving entity and forwards it. */
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	/*! \brief Loads persisted RSimulator configuration. */
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	/*! \brief Saves RSimulator configuration. */
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	/*! \brief Validates the executable, working directory and command list. */
	virtual bool _check(std::string& errorMessage) override;
	/*! \brief Creates or updates the internal RSimulatorRunner. */
	virtual void _createInternalAndAttachedData() override;

private:
	/*! \brief Creates the internal runner if it does not exist yet. */
	void _ensureRunner();
	/*! \brief Copies component configuration into the internal runner before execution. */
	void _syncRunnerConfiguration();
	/*! \brief Returns true if the command contains non-whitespace characters. */
	bool _isNonEmptyCommand(const std::string& command) const;

private:
	const struct DEFAULT_VALUES {
		std::string rExecutable = "Rscript";
		std::string workingDirectory = "";
		std::string preludeScript = "";
	} DEFAULT;

	std::string _rExecutable = DEFAULT.rExecutable;
	std::string _workingDirectory = DEFAULT.workingDirectory;
	std::string _preludeScript = DEFAULT.preludeScript;
	List<std::string>* _commands = new List<std::string>();
	RSimulatorRunner* _runner = nullptr;
};

#endif /* RSIMULATOR_H */
