/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   PluginManager.h
 * Author: rafael.luiz.cancian
 *
 * Created on 30 de Maio de 2019, 17:49
 */

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <functional>
#include <string>
#include <vector>

#include "Model.h"
#include "../util/List.h"
#include "../util/Util.h"
#include "Plugin.h"
#include "PluginConnector_if.h"
#include "SystemDependencyResolver.h"

class Simulator;
class Model;

/*!
 * \brief Optional policy hooks used while inserting a plugin.
 *
 * The kernel reports dependency diagnostics through this structure and leaves
 * user confirmation to the caller. An empty callback means non-interactive
 * insertion: missing dependencies are reported through traces and no install
 * command is executed.
 */
struct PluginInsertionOptions {
	/*! \brief Called before running any install command for missing system dependencies. */
	std::function<bool(const SystemDependencyCheckResult&)> confirmSystemDependencyInstallation;
};

/*!
 * \brief Diagnostic record for a plugin candidate that could not be loaded.
 *
 * The manager keeps these records so interactive front-ends can explain startup
 * autoload failures after the application UI is ready. The kernel stores only
 * technical diagnostics; GUI code decides how to present and resolve them.
 */
class PluginLoadIssue {
public:
	/*! \brief High-level reason why a plugin candidate was not loaded. */
	enum class Reason {
		InvalidPlugin,
		MissingSystemDependency,
		DynamicDependencyFailure,
		ConnectionFailure,
		InsertionFailure,
		Exception
	};

	/*! \brief Creates an empty diagnostic record. */
	PluginLoadIssue() = default;
	/*! \brief Creates a diagnostic record for a failed plugin load attempt. */
	PluginLoadIssue(std::string filename,
	                std::string pluginTypename,
	                Reason reason,
	                std::string message,
	                SystemDependencyCheckResult systemDependencyResult = {}) noexcept;

	/*! \brief Returns the filename passed to the plugin connector. */
	const std::string& getFilename() const;
	/*! \brief Returns the plugin typename when it could be read from PluginInformation. */
	const std::string& getPluginTypename() const;
	/*! \brief Returns the high-level failure reason. */
	Reason getReason() const;
	/*! \brief Returns the human-readable diagnostic message captured by the manager. */
	const std::string& getMessage() const;
	/*! \brief Returns dependency diagnostics captured during system dependency preflight. */
	const SystemDependencyCheckResult& getSystemDependencyResult() const;
	/*! \brief Returns true when this issue includes system dependency diagnostics. */
	bool hasSystemDependencyResult() const;
	/*! \brief Returns a stable textual label for a failure reason. */
	static std::string reasonToString(Reason reason);
	/*! \brief Formats the issue for trace logs or GUI details. */
	std::string diagnosticText() const;

private:
	std::string _filename = "";
	std::string _pluginTypename = "";
	Reason _reason = Reason::Exception;
	std::string _message = "";
	SystemDependencyCheckResult _systemDependencyResult;
};

//namespace GenesysKernel {

/*!
 * \brief Manages discovery, loading, lookup and instantiation of simulation plugins.
 *
 * The plugin manager centralizes dynamic-library checks/connections through a
 * \c PluginConnector_if and keeps the in-memory list of connected plugins used
 * by the simulator to create model data/component instances.
 */
class PluginManager {
public:
	/*! \brief Creates a plugin manager bound to a simulator instance. */
	PluginManager(Simulator* simulator);
	/*!
	 * \brief Creates a plugin manager with injectable connector and command executor.
	 *
	 * Ownership of both injected pointers is transferred to the manager. This
	 * constructor exists primarily for unit tests and narrow integration seams.
	 */
	PluginManager(Simulator* simulator, PluginConnector_if* pluginConnector, SystemCommandExecutor_if* systemCommandExecutor);
	/*! \brief Destroys owned plugin wrappers and connector resources. */
	virtual ~PluginManager();

public:
	/*! \brief Returns a human-readable summary of currently connected plugins. */
	std::string show();

public:
	/*! \brief Completes plugin metadata (fields/templates) after loading. */
	List<Plugin*>* completePluginsFieldsAndTemplates();

public:
	/*! \brief Validates whether a dynamic library provides a compatible plugin. */
	bool check(const std::string& dynamicLibraryFilename);
	/*! \brief Evaluates system dependencies declared by a dynamic-library plugin without inserting it. */
	SystemDependencyCheckResult checkSystemDependencies(const std::string& dynamicLibraryFilename);
	/*! \brief Discovers plugin filenames through the configured connector without inserting them. */
	List<std::string>* discoverPluginFilenames() const;
	/*! \brief Returns plugin candidates that failed to load during check/connect/preflight. */
	List<PluginLoadIssue>* getPluginLoadIssues() const;
	/*! \brief Clears all stored plugin load diagnostics. */
	void clearPluginLoadIssues();
	/*! \brief Removes stored diagnostics for one plugin candidate filename. */
	void clearPluginLoadIssue(const std::string& dynamicLibraryFilename);
	/*! \brief Loads and inserts a plugin from a dynamic library file. */
	Plugin* insert(const std::string& dynamicLibraryFilename);
	/*! \brief Loads and inserts a plugin, optionally asking the caller to authorize dependency installation. */
	Plugin* insert(const std::string& dynamicLibraryFilename, const PluginInsertionOptions& options);
	/*! \brief Removes/disconnects a plugin by its dynamic library filename. */
	bool remove(const std::string& dynamicLibraryFilename);
	/*! \brief Removes/disconnects a plugin by pointer. */
	bool remove(Plugin* plugin);
	/*! \brief Finds a connected plugin by plugin type name. */
	Plugin* find(const std::string& pluginTypeName);
	/*! \brief Returns connected plugin type names that can create ModelDataDefinition instances.
	 *
	 * Polymorphic data structures such as Set use this snapshot to expose concrete,
	 * creatable element types to editors without hard-coding plugin names in the GUI.
	 */
	std::vector<std::string> getDataDefinitionPluginTypenames() const;
	/*! \brief Returns the source include path for a connected plugin type. */
	std::string sourceIncludePathFor(const std::string& pluginTypeName);
	/*! \brief Auto-loads plugins listed in file (or discovered automatically as fallback). */
	List<Plugin*>* autoInsertPlugins(const std::string& pluginsListFilename,
	                                 const bool lookForPluginsIfFilenameNotFound = true);
	/*! \brief Auto-loads plugins listed in file using insertion policy hooks. */
	List<Plugin*>* autoInsertPlugins(const std::string& pluginsListFilename,
	                                 const bool lookForPluginsIfFilenameNotFound,
	                                 const PluginInsertionOptions& options);
	/*! \brief Auto-loads plugins discovered automatically). */
	List<Plugin*>* autoInsertPlugins();
	/*! \brief Auto-loads plugins discovered automatically using insertion policy hooks. */
	List<Plugin*>* autoInsertPlugins(const PluginInsertionOptions& options);

public:
	/*! \brief Returns the first plugin in the internal plugin list. */
	Plugin* front();
	/*! \brief Returns the next plugin using the internal iterator cursor. */
	Plugin* next();
	/*! \brief Returns the last plugin in the internal plugin list. */
	Plugin* last();
	/*! \brief Returns the number of connected plugins. */
	unsigned int size();
	/*! \brief Returns the plugin at a specific list rank. */
	Plugin* getAtRank(unsigned int rank);

public:
	/*! \brief Creates a new model data/component instance using a plugin typename. */
	ModelDataDefinition* newInstance(const std::string& pluginTypename, Model* model, std::string name = "");

	template <typename T>
	T* newInstance(Model* model, std::string name = "") {
		name = Util::StrReplace(name, " ", "_");
		Plugin* plugin;
		std::string pluginTypename = Util::TypeOf<T>();
		// @ToDo: (pequena alteração): Use Find method??
		for (unsigned short i = 0; i < _plugins->size(); i++) {
			plugin = _plugins->getAtRank(i);
			if (plugin->getPluginInfo()->getPluginTypename() == pluginTypename) {
				T* instance;
				StaticConstructorDataDefinitionInstance constructor = plugin->getPluginInfo()->
				                                                              getDataDefinitionConstructor();
				instance = static_cast<T*>(constructor(model, name));
				if (model != nullptr) {
					ModelDataDefinition::CreateInternalData(instance);
				}
				return instance;
			}
		}
		// Keep this header free of Simulator implementation details to avoid include cycles.
		return nullptr;
	}

private:
	/*! \brief Inserts a connected plugin after recursive dynamic dependency and system dependency checks. */
	bool _insert(Plugin* plugin, const PluginInsertionOptions& options, const std::string& dynamicLibraryFilename);
	/*! \brief Runs system dependency preflight and optional confirmed installation for one plugin. */
	bool _preflightAndMaybeInstallSystemDependencies(PluginInformation* plugInfo,
	                                                 const PluginInsertionOptions& options,
	                                                 SystemDependencyCheckResult* blockingResult = nullptr,
	                                                 std::string* failureMessage = nullptr);
	/*! \brief Stores or replaces the diagnostic for a failed plugin candidate. */
	void _recordLoadIssue(const PluginLoadIssue& issue);
	/*! \brief Removes a diagnostic by filename or plugin typename after successful insertion. */
	void _removeLoadIssue(const std::string& dynamicLibraryFilename, const std::string& pluginTypename);
	/*! \brief Registers kernel built-in plugins that do not come from dynamic libraries. */
	void _insertDefaultKernelElements();
	/*! \brief Finds and attempts to load plugins from connector discovery. */
	List<Plugin*>* _autoFindPlugins(const PluginInsertionOptions& options);

private:
	List<Plugin*>* _plugins;
	List<PluginLoadIssue>* _pluginLoadIssues;
	Simulator* _simulator;
	PluginConnector_if* _pluginConnector;
	SystemCommandExecutor_if* _systemCommandExecutor;
};

//namespace\\}
#endif /* PLUGINMANAGER_H */
