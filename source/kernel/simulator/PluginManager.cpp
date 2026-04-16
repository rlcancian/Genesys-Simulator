/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   PluginManager.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 30 de Maio de 2019, 17:49
 */

#include <fstream>
#include "PluginManager.h"
#include "Simulator.h"
#include "../TraitsKernel.h"
#include "Attribute.h"
#include "Counter.h"
#include "EntityType.h"
#include "StatisticsCollector.h"
#include "Model.h"

//using namespace GenesysKernel;

PluginManager::PluginManager(Simulator* simulator) {
	_simulator = simulator;
	_pluginConnector = new TraitsKernel<PluginConnector_if>::Implementation();
	_systemCommandExecutor = new SystemShellCommandExecutor();
	_insertDefaultKernelElements();
}

PluginManager::PluginManager(Simulator* simulator, PluginConnector_if* pluginConnector, SystemCommandExecutor_if* systemCommandExecutor) {
	_simulator = simulator;
	_pluginConnector = pluginConnector;
	_systemCommandExecutor = systemCommandExecutor;
	_insertDefaultKernelElements();
}

// Release connector and plugin wrappers owned by the manager during simulator teardown.
PluginManager::~PluginManager() {
	for (Plugin* plugin : *_plugins->list()) {
		delete plugin;
	}
	delete _plugins;
	delete _pluginConnector;
	delete _systemCommandExecutor;
}

List<Plugin*>* PluginManager::_autoFindPlugins() {
	List<std::string>* filenames = _pluginConnector->find();
	for (std::string filename: *filenames->list()) {
		insert(filename);
	}
	return  completePluginsFieldsAndTemplates();
}

List<Plugin*>* PluginManager::autoInsertPlugins() {
    return autoInsertPlugins("", true);
}

List<Plugin*>* PluginManager::autoInsertPlugins(const std::string pluginsListFilename, const bool lookForPluginsIfFilenameNotFound)
{
	List<Plugin*>* loadedPlugins = nullptr;
	if (pluginsListFilename.empty()) {
		 if (lookForPluginsIfFilenameNotFound)
		 	loadedPlugins = _autoFindPlugins();
		return loadedPlugins;
	}
	std::string line;
	std::string fullFilename;
	if (pluginsListFilename[0] == Util::DirSeparator()) // absolute path
		fullFilename = pluginsListFilename;
	else	// relative path
		fullFilename = Util::RunningPath()+Util::DirSeparator()+pluginsListFilename;
	std::ifstream file(fullFilename, std::ifstream::in);
	if (file.is_open()) {
		loadedPlugins = new List<Plugin*>();
		while (std::getline(file, line)) {
			if (line.length()>=1) {
                // @ToDo: (pequena alteração): 2500701 why [0-2] are special chars?
                while (line[0]>126 || line[0]<32) {
                    line.erase(0,1);
                }
				if (line[0] != '#') { // not a comment
					insert(line);
				}
			}
		}
		file.close();
		loadedPlugins = completePluginsFieldsAndTemplates();
	} else
	{
		_simulator->getTraceManager()->traceError("Could not open file \""+pluginsListFilename+"\" (\""+fullFilename+"\")");
		if (lookForPluginsIfFilenameNotFound) {
			loadedPlugins = _autoFindPlugins();
		}
	}
	return loadedPlugins;
}

std::string PluginManager::show() {
	std::string message = "Plugins=[";
	for (Plugin* plugin : *_plugins->list()) {
		message += +"{" + plugin->show() + "}, ";
	}
	message = message.substr(0, message.length() - 2);
	message += "]";
	return message;
}

void PluginManager::_insertDefaultKernelElements() {
	_plugins->insert(new Plugin(&EntityType::GetPluginInformation));
	_plugins->insert(new Plugin(&Attribute::GetPluginInformation));
	_plugins->insert(new Plugin(&Counter::GetPluginInformation));
	_plugins->insert(new Plugin(&StatisticsCollector::GetPluginInformation));
}

List<Plugin*>* PluginManager::completePluginsFieldsAndTemplates() {
	return _simulator->_completePluginsFieldsAndTemplate();
}

//bool PluginManager::check(Plugin* plugin){
//    return true;
//}

bool PluginManager::_preflightAndMaybeInstallSystemDependencies(PluginInformation* plugInfo, const PluginInsertionOptions& options) {
	if (plugInfo == nullptr || !plugInfo->hasSystemDependencies()) {
		return true;
	}

	SystemDependencyCheckResult preflight = SystemDependencyResolver::evaluate(
		plugInfo->getSystemDependencies(),
		*_systemCommandExecutor);
	if (preflight.canInsertPlugin()) {
		return true;
	}

	// In headless or startup/autoload flows there is no callback, so install commands are never run silently.
	_simulator->getTraceManager()->traceError(
		"Plugin system dependencies are not satisfied: " + preflight.summary(),
		TraceManager::Level::L3_errorRecover);
	if (!options.confirmSystemDependencyInstallation) {
		_simulator->getTraceManager()->traceError(
			"No interactive confirmation callback is available; plugin will not be inserted.",
			TraceManager::Level::L3_errorRecover);
		return false;
	}

	// The GUI callback is the only place allowed to ask the user before package installation.
	if (!options.confirmSystemDependencyInstallation(preflight)) {
		_simulator->getTraceManager()->traceError(
			"User did not authorize system dependency installation; plugin will not be inserted.",
			TraceManager::Level::L3_errorRecover);
		return false;
	}

	SystemDependencyInstallResult installResult = SystemDependencyResolver::installMissingDependencies(
		preflight,
		*_systemCommandExecutor);
	if (!installResult.succeeded()) {
		_simulator->getTraceManager()->traceError(
			"System dependency installation failed: " + installResult.summary(),
			TraceManager::Level::L3_errorRecover);
		return false;
	}

	// Revalidation after installation prevents inserting a plugin after a partial or ineffective install.
	SystemDependencyCheckResult validation = SystemDependencyResolver::evaluate(
		plugInfo->getSystemDependencies(),
		*_systemCommandExecutor);
	if (!validation.canInsertPlugin()) {
		_simulator->getTraceManager()->traceError(
			"System dependencies are still not satisfied after installation: " + validation.summary(),
			TraceManager::Level::L3_errorRecover);
		return false;
	}
	return true;
}

bool PluginManager::_insert(Plugin * plugin, const PluginInsertionOptions& options) {
	if (plugin == nullptr) {
		return false;
	}
	PluginInformation *plugInfo = plugin->getPluginInfo();
	if (plugin->isIsValidPlugin() && plugInfo != nullptr) {
		std::string msg = "Inserting ";
		if (plugInfo->isComponent())
			msg += "component";
		else
			msg += "modeldatum";
		msg += " plugin \"" + plugin->getPluginInfo()->getPluginTypename() + "\"";
		_simulator->getTraceManager()->trace(msg);
		if (this->find(plugInfo->getPluginTypename()) != nullptr) { // plugin alread exists
			Util::IncIndent();
			_simulator->getTraceManager()->trace("The plugin already exists and was not inserted again");
			Util::DecIndent();
			return true; // It already exists. It was NOT inserted again, BUT it has been inserted BEFORE, therefore returns TRUE
		}
		// insert all dependencies before to insert this plugin
		bool allDependenciesInserted = true;
		if (plugInfo->getDynamicLibFilenameDependencies()->size() > 0) {
			Util::IncIndent();
			{
				_simulator->getTraceManager()->trace("Inserting dependencies...");
				Util::IncIndent();
				{
					for (std::string str : *plugInfo->getDynamicLibFilenameDependencies()) {
						allDependenciesInserted &= (this->insert(str, options) != nullptr);
					}
				}
				Util::DecIndent();
			}
			Util::DecIndent();
		}
		if (!allDependenciesInserted) {
			_simulator->getTraceManager()->traceError("Plugin dependencies could not be inserted; therefore, the plugin will not be inserted", TraceManager::Level::L3_errorRecover);
			return false;
		}
		if (!_preflightAndMaybeInstallSystemDependencies(plugInfo, options)) {
			return false;
		}
		_plugins->insert(plugin);
		Util::IncIndent();
		this->_simulator->getTraceManager()->trace(TraceManager::Level::L2_results, "Plugin successfully inserted");
		Util::DecIndent();
		return true;
	} else {
		Util::IncIndent();
		this->_simulator->getTraceManager()->trace(TraceManager::Level::L2_results, "Plugin could not be inserted");
		delete plugin; //->~Plugin(); // destroy the invalid plugin
		Util::DecIndent();
		return false;
	}
}

bool PluginManager::check(std::string dynamicLibraryFilename) {
	Plugin* plugin;
	try {
		plugin = _pluginConnector->check(dynamicLibraryFilename);
	} catch (...) {
		return false;
	}
	const bool checked = plugin != nullptr && plugin->isIsValidPlugin();
	delete plugin;
	return checked;
}

SystemDependencyCheckResult PluginManager::checkSystemDependencies(std::string dynamicLibraryFilename) {
	SystemDependencyCheckResult result;
	Plugin* plugin = nullptr;
	try {
		plugin = _pluginConnector->check(dynamicLibraryFilename);
	} catch (...) {
		return result;
	}
	if (plugin != nullptr && plugin->isIsValidPlugin() && plugin->getPluginInfo() != nullptr) {
		result = SystemDependencyResolver::evaluate(plugin->getPluginInfo()->getSystemDependencies(), *_systemCommandExecutor);
	}
	delete plugin;
	return result;
}

Plugin * PluginManager::insert(std::string dynamicLibraryFilename) {
	PluginInsertionOptions options;
	return insert(dynamicLibraryFilename, options);
}

Plugin * PluginManager::insert(std::string dynamicLibraryFilename, const PluginInsertionOptions& options) {
	Plugin* plugin = nullptr;
	try {
		plugin = _pluginConnector->connect(dynamicLibraryFilename);
		if (plugin != nullptr) {
			const bool validBeforeInsert = plugin->isIsValidPlugin();
			if (!_insert(plugin, options)) {
				if (validBeforeInsert) {
					// A connected but rejected plugin must be released by the connector.
					_pluginConnector->disconnect(plugin);
				}
				plugin = nullptr;
			}
		} else {
			_simulator->getTraceManager()->traceError("Plugin from file \"" + dynamicLibraryFilename + "\" could not be loaded.", TraceManager::Level::L3_errorRecover);
		}
	} catch (...) {
		return nullptr;
	}
	return plugin;
}

bool PluginManager::remove(std::string dynamicLibraryFilename) {

	Plugin* pi = this->find(dynamicLibraryFilename);
	return remove(pi);
}

bool PluginManager::remove(Plugin * plugin) {
	if (plugin != nullptr) {
		try {
			if (!_pluginConnector->disconnect(plugin)) {
				return false;
			}
		} catch (...) {
			return false;
		}
		_plugins->remove(plugin);
		delete plugin;
		_simulator->getTraceManager()->trace(TraceManager::Level::L2_results, "Plugin successfully removed");
		return true;
	}
	_simulator->getTraceManager()->trace(TraceManager::Level::L2_results, "Plugin could not be removed");
	return false;
}

Plugin * PluginManager::find(std::string pluginTypeName) {
	for (Plugin* plugin : *this->_plugins->list()) {
		if (plugin->getPluginInfo()->getPluginTypename() == pluginTypeName) {
			return plugin;
		}
	}
	return nullptr;
}

Plugin * PluginManager::front() {
	return this->_plugins->front();
}

Plugin * PluginManager::next() {
	return _plugins->next();
}

Plugin * PluginManager::last() {
	return this->_plugins->last();
}

unsigned int PluginManager::size() {
	return _plugins->size();
}

Plugin * PluginManager::getAtRank(unsigned int rank) {
	return _plugins->getAtRank(rank);
}

ModelDataDefinition* PluginManager::newInstance(std::string pluginTypename, Model* model, std::string name) {
	Plugin* plugin = find(pluginTypename);
	if (plugin != nullptr) {
		return plugin->newInstance(model, name);
	}
	return nullptr;
}
