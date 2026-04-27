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
#include <memory>
#include <sstream>
#include <utility>
#include "PluginManager.h"
#include "Simulator.h"
#include "../TraitsKernel.h"
#include "Attribute.h"
#include "Counter.h"
#include "EntityType.h"
#include "StatisticsCollector.h"
#include "Model.h"

//using namespace GenesysKernel;

PluginLoadIssue::PluginLoadIssue(std::string filename,
                                 std::string pluginTypename,
                                 Reason reason,
                                 std::string message,
                                 SystemDependencyCheckResult systemDependencyResult) noexcept
	: _filename(std::move(filename)),
	  _pluginTypename(std::move(pluginTypename)),
	  _reason(reason),
	  _message(std::move(message)),
	  _systemDependencyResult(std::move(systemDependencyResult)) {
}

const std::string& PluginLoadIssue::getFilename() const {
	return _filename;
}

const std::string& PluginLoadIssue::getPluginTypename() const {
	return _pluginTypename;
}

PluginLoadIssue::Reason PluginLoadIssue::getReason() const {
	return _reason;
}

const std::string& PluginLoadIssue::getMessage() const {
	return _message;
}

const SystemDependencyCheckResult& PluginLoadIssue::getSystemDependencyResult() const {
	return _systemDependencyResult;
}

bool PluginLoadIssue::hasSystemDependencyResult() const {
	return !_systemDependencyResult.entries().empty();
}

std::string PluginLoadIssue::reasonToString(Reason reason) {
	switch (reason) {
		case Reason::InvalidPlugin:
			return "invalid plugin";
		case Reason::MissingSystemDependency:
			return "missing system dependency";
		case Reason::DynamicDependencyFailure:
			return "dynamic dependency failure";
		case Reason::ConnectionFailure:
			return "connection failure";
		case Reason::InsertionFailure:
			return "insertion failure";
		case Reason::Exception:
			return "exception";
	}
	return "unknown";
}

std::string PluginLoadIssue::diagnosticText() const {
	std::ostringstream text;
	text << "Plugin load issue\n";
	text << "  File: " << (_filename.empty() ? "<not available>" : _filename) << "\n";
	text << "  Plugin type: " << (_pluginTypename.empty() ? "<not available>" : _pluginTypename) << "\n";
	text << "  Reason: " << reasonToString(_reason) << "\n";
	if (!_message.empty()) {
		text << "  Diagnostic: " << _message << "\n";
	}
	if (hasSystemDependencyResult()) {
		text << "\nSystem dependency diagnostics:\n" << _systemDependencyResult.diagnosticText(false);
	}
	return text.str();
}

PluginManager::PluginManager(Simulator* simulator) {
	_simulator = simulator;
	_plugins = new List<Plugin*>();
	_pluginLoadIssues = new List<PluginLoadIssue>();
	_pluginConnector = new TraitsKernel<PluginConnector_if>::Implementation();
	_systemCommandExecutor = new SystemShellCommandExecutor();
	_insertDefaultKernelElements();
}

PluginManager::PluginManager(Simulator* simulator, PluginConnector_if* pluginConnector, SystemCommandExecutor_if* systemCommandExecutor) {
	_simulator = simulator;
	_plugins = new List<Plugin*>();
	_pluginLoadIssues = new List<PluginLoadIssue>();
	_pluginConnector = pluginConnector;
	_systemCommandExecutor = systemCommandExecutor;
	_insertDefaultKernelElements();
}

// Release connector and plugin wrappers owned by the manager during simulator teardown.
PluginManager::~PluginManager() {
	for (Plugin* const plugin : *_plugins->list()) {
		delete plugin;
	}
	delete _plugins;
	delete _pluginLoadIssues;
	delete _pluginConnector;
	delete _systemCommandExecutor;
}

List<Plugin*>* PluginManager::_autoFindPlugins(const PluginInsertionOptions& options) {
	if (_pluginConnector == nullptr) {
		return completePluginsFieldsAndTemplates();
	}
	std::unique_ptr<List<std::string>> filenames(_pluginConnector->find());
	if (filenames == nullptr) {
		return completePluginsFieldsAndTemplates();
	}
	for (const std::string& filename : *filenames->list()) {
		insert(filename, options);
	}
	return completePluginsFieldsAndTemplates();
}

List<Plugin*>* PluginManager::autoInsertPlugins() {
	PluginInsertionOptions options;
    return autoInsertPlugins("", true, options);
}

List<Plugin*>* PluginManager::autoInsertPlugins(const PluginInsertionOptions& options) {
    return autoInsertPlugins("", true, options);
}

List<Plugin*>* PluginManager::autoInsertPlugins(const std::string& pluginsListFilename, const bool lookForPluginsIfFilenameNotFound)
{
	PluginInsertionOptions options;
	return autoInsertPlugins(pluginsListFilename, lookForPluginsIfFilenameNotFound, options);
}

List<Plugin*>* PluginManager::autoInsertPlugins(const std::string& pluginsListFilename,
                                                const bool lookForPluginsIfFilenameNotFound,
                                                const PluginInsertionOptions& options)
{
	List<Plugin*>* loadedPlugins = new List<Plugin*>();
	if (pluginsListFilename.empty()) {
		if (lookForPluginsIfFilenameNotFound) {
			delete loadedPlugins;
			loadedPlugins = _autoFindPlugins(options);
		}
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
		while (std::getline(file, line)) {
			if (line.length()>=1) {
                // @ToDo: (pequena alteração): 2500701 why [0-2] are special chars?
                while (line[0]>126 || line[0]<32) {
                    line.erase(0,1);
                }
				if (line[0] != '#') { // not a comment
					insert(line, options);
				}
			}
		}
		file.close();
		delete loadedPlugins;
		loadedPlugins = completePluginsFieldsAndTemplates();
	} else
	{
		_simulator->getTraceManager()->traceError("Could not open file \""+pluginsListFilename+"\" (\""+fullFilename+"\")");
		if (lookForPluginsIfFilenameNotFound) {
			delete loadedPlugins;
			loadedPlugins = _autoFindPlugins(options);
		}
	}
	return loadedPlugins;
}

std::string PluginManager::show() {
	std::string message = "Plugins=[";
	for (Plugin* const plugin : *_plugins->list()) {
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

bool PluginManager::_preflightAndMaybeInstallSystemDependencies(PluginInformation* plugInfo,
                                                                const PluginInsertionOptions& options,
                                                                SystemDependencyCheckResult* blockingResult,
                                                                std::string* failureMessage) {
	if (plugInfo == nullptr || !plugInfo->hasSystemDependencies()) {
		return true;
	}

	_simulator->getTraceManager()->trace(
		"Checking system dependencies for plugin \"" + plugInfo->getPluginTypename() + "\"");
	SystemDependencyCheckResult preflight = SystemDependencyResolver::evaluate(
		plugInfo->getSystemDependencies(),
		*_systemCommandExecutor);
	if (preflight.canInsertPlugin()) {
		_simulator->getTraceManager()->trace(
			"System dependencies satisfied: " + preflight.summary());
		return true;
	}

	// In headless or startup/autoload flows there is no callback, so install commands are never run silently.
	_simulator->getTraceManager()->traceError(
		"Plugin system dependencies are not satisfied:\n" + preflight.diagnosticText(false),
		TraceManager::Level::L3_errorRecover);
	if (blockingResult != nullptr) {
		*blockingResult = preflight;
	}
	if (!options.confirmSystemDependencyInstallation) {
		if (failureMessage != nullptr) {
			*failureMessage = "No interactive confirmation callback is available; plugin will not be inserted.";
		}
		_simulator->getTraceManager()->traceError(
			"No interactive confirmation callback is available; plugin will not be inserted. "
			"To satisfy the dependencies manually, run the install command(s) listed above and try again.",
			TraceManager::Level::L3_errorRecover);
		return false;
	}

	// The GUI callback is the only place allowed to ask the user before package installation.
	if (!options.confirmSystemDependencyInstallation(preflight)) {
		if (failureMessage != nullptr) {
			*failureMessage = "User did not authorize system dependency installation; plugin will not be inserted.";
		}
		_simulator->getTraceManager()->traceError(
			"User did not authorize system dependency installation; plugin will not be inserted.",
			TraceManager::Level::L3_errorRecover);
		return false;
	}

	SystemDependencyInstallResult installResult = SystemDependencyResolver::installMissingDependencies(
		preflight,
		*_systemCommandExecutor);
	if (!installResult.succeeded()) {
		if (failureMessage != nullptr) {
			*failureMessage = "System dependency installation failed:\n" + installResult.diagnosticText();
		}
		_simulator->getTraceManager()->traceError(
			"System dependency installation failed:\n" + installResult.diagnosticText(),
			TraceManager::Level::L3_errorRecover);
		return false;
	}
	_simulator->getTraceManager()->trace(
		"System dependency installation command(s) finished:\n" + installResult.diagnosticText());

	// Revalidation after installation prevents inserting a plugin after a partial or ineffective install.
	SystemDependencyCheckResult validation = SystemDependencyResolver::evaluate(
		plugInfo->getSystemDependencies(),
		*_systemCommandExecutor);
	if (!validation.canInsertPlugin()) {
		if (blockingResult != nullptr) {
			*blockingResult = validation;
		}
		if (failureMessage != nullptr) {
			*failureMessage = "System dependencies are still not satisfied after installation.";
		}
		_simulator->getTraceManager()->traceError(
			"System dependencies are still not satisfied after installation:\n" + validation.diagnosticText(false),
			TraceManager::Level::L3_errorRecover);
		return false;
	}
	return true;
}

bool PluginManager::_insert(Plugin* plugin, const PluginInsertionOptions& options, const std::string& dynamicLibraryFilename) {
	if (plugin == nullptr) {
		return false;
	}
	PluginInformation* plugInfo = plugin->getPluginInfo();
	if (plugInfo == nullptr) {
		return false;
	}
    const std::string& pluginname = plugInfo->getPluginTypename();
    if (plugin->isIsValidPlugin() && plugInfo != nullptr) {
		std::string msg = "Inserting ";
		if (plugInfo->isComponent())
			msg += "component";
		else
			msg += "modeldatum";
		msg += " plugin \"" + pluginname + "\"";
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
					for (const std::string& str : *plugInfo->getDynamicLibFilenameDependencies()) {
						allDependenciesInserted &= (this->insert(str, options) != nullptr);
					}
				}
				Util::DecIndent();
			}
			Util::DecIndent();
		}
		if (!allDependenciesInserted) {
			_simulator->getTraceManager()->traceError("Plugin dependencies could not be inserted; therefore, the plugin will not be inserted", TraceManager::Level::L3_errorRecover);
			_recordLoadIssue(PluginLoadIssue(
				dynamicLibraryFilename,
				plugInfo->getPluginTypename(),
				PluginLoadIssue::Reason::DynamicDependencyFailure,
				"One or more dynamic library dependencies could not be inserted."));
			return false;
		}
		SystemDependencyCheckResult blockingResult;
		std::string failureMessage;
		if (!_preflightAndMaybeInstallSystemDependencies(plugInfo, options, &blockingResult, &failureMessage)) {
			_recordLoadIssue(PluginLoadIssue(
				dynamicLibraryFilename,
				plugInfo->getPluginTypename(),
				PluginLoadIssue::Reason::MissingSystemDependency,
				failureMessage,
				blockingResult));
			return false;
		}
		_plugins->insert(plugin);
		_removeLoadIssue(dynamicLibraryFilename, plugInfo->getPluginTypename());
		Util::IncIndent();
		this->_simulator->getTraceManager()->trace(TraceManager::Level::L2_results, "Plugin \""+pluginname+"\" successfully inserted");
		Util::DecIndent();
		return true;
	} else {
		Util::IncIndent();
		this->_simulator->getTraceManager()->trace(TraceManager::Level::L2_results, "Plugin \""+pluginname+"\" could not be inserted");
		_recordLoadIssue(PluginLoadIssue(
			dynamicLibraryFilename,
			"",
			PluginLoadIssue::Reason::InvalidPlugin,
			"Connected plugin object is invalid or has no PluginInformation."));
		delete plugin; //->~Plugin(); // destroy the invalid plugin
		Util::DecIndent();
		return false;
	}
}

bool PluginManager::check(const std::string& dynamicLibraryFilename) {
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

SystemDependencyCheckResult PluginManager::checkSystemDependencies(const std::string& dynamicLibraryFilename) {
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

List<std::string>* PluginManager::discoverPluginFilenames() const {
	if (_pluginConnector == nullptr) {
		return new List<std::string>();
	}
	return _pluginConnector->find();
}

List<PluginLoadIssue>* PluginManager::getPluginLoadIssues() const {
	return _pluginLoadIssues;
}

void PluginManager::clearPluginLoadIssues() {
	_pluginLoadIssues->clear();
}

void PluginManager::clearPluginLoadIssue(const std::string& dynamicLibraryFilename) {
	_removeLoadIssue(dynamicLibraryFilename, "");
}

void PluginManager::_recordLoadIssue(const PluginLoadIssue& issue) {
	_removeLoadIssue(issue.getFilename(), issue.getPluginTypename());
	_pluginLoadIssues->insert(issue);
}

void PluginManager::_removeLoadIssue(const std::string& dynamicLibraryFilename, const std::string& pluginTypename) {
	// Collect indices to remove to avoid iterator invalidation during erase
	std::vector<size_t> indicesToRemove;
	size_t index = 0;
	for (const auto& issue : *_pluginLoadIssues->list()) {
		const bool sameFilename = !dynamicLibraryFilename.empty() && issue.getFilename() == dynamicLibraryFilename;
		const bool sameTypename = !pluginTypename.empty() && issue.getPluginTypename() == pluginTypename;
		if (sameFilename || sameTypename) {
			indicesToRemove.push_back(index);
		}
		++index;
	}
	// Remove in reverse order to maintain valid indices
	for (auto it = indicesToRemove.rbegin(); it != indicesToRemove.rend(); ++it) {
		auto listIt = _pluginLoadIssues->list()->begin();
		std::advance(listIt, *it);
		_pluginLoadIssues->list()->erase(listIt);
	}
}

Plugin* PluginManager::insert(const std::string& dynamicLibraryFilename) {
	PluginInsertionOptions options;
	return insert(dynamicLibraryFilename, options);
}

Plugin* PluginManager::insert(const std::string& dynamicLibraryFilename, const PluginInsertionOptions& options) {
	Plugin* plugin = nullptr;
	try {
		std::unique_ptr<Plugin> checkedPlugin(_pluginConnector->check(dynamicLibraryFilename));
		if (checkedPlugin == nullptr || !checkedPlugin->isIsValidPlugin() || checkedPlugin->getPluginInfo() == nullptr) {
			_simulator->getTraceManager()->traceError(
				"Plugin from file \"" + dynamicLibraryFilename + "\" could not be checked as a valid plugin.",
				TraceManager::Level::L3_errorRecover);
			_recordLoadIssue(PluginLoadIssue(
				dynamicLibraryFilename,
				"",
				PluginLoadIssue::Reason::InvalidPlugin,
				"Plugin could not be checked as a valid plugin."));
			return nullptr;
		}
		PluginInformation* checkedInfo = checkedPlugin->getPluginInfo();
		Plugin* alreadyInserted = find(checkedInfo->getPluginTypename());
		if (alreadyInserted != nullptr) {
			_simulator->getTraceManager()->trace(
				"Plugin \"" + checkedInfo->getPluginTypename() + "\" already exists and was not connected again");
			_removeLoadIssue(dynamicLibraryFilename, checkedInfo->getPluginTypename());
			return alreadyInserted;
		}
		// Preflight before connect prevents loading/connecting plugins whose system prerequisites are absent.
		SystemDependencyCheckResult blockingResult;
		std::string failureMessage;
		if (!_preflightAndMaybeInstallSystemDependencies(checkedInfo, options, &blockingResult, &failureMessage)) {
			_recordLoadIssue(PluginLoadIssue(
				dynamicLibraryFilename,
				checkedInfo->getPluginTypename(),
				PluginLoadIssue::Reason::MissingSystemDependency,
				failureMessage,
				blockingResult));
			return nullptr;
		}

		plugin = _pluginConnector->connect(dynamicLibraryFilename);
		if (plugin != nullptr) {
			const bool validBeforeInsert = plugin->isIsValidPlugin();
			if (!_insert(plugin, options, dynamicLibraryFilename)) {
				if (validBeforeInsert) {
					// A connected but rejected plugin must be released by the connector.
					_pluginConnector->disconnect(plugin);
				}
				plugin = nullptr;
			}
		} else {
			_simulator->getTraceManager()->traceError("Plugin from file \"" + dynamicLibraryFilename + "\" could not be loaded.", TraceManager::Level::L3_errorRecover);
			_recordLoadIssue(PluginLoadIssue(
				dynamicLibraryFilename,
				checkedInfo->getPluginTypename(),
				PluginLoadIssue::Reason::ConnectionFailure,
				"Plugin connector returned no plugin instance."));
		}
	} catch (...) {
		_recordLoadIssue(PluginLoadIssue(
			dynamicLibraryFilename,
			"",
			PluginLoadIssue::Reason::Exception,
			"An exception was thrown while checking or connecting the plugin."));
		return nullptr;
	}
	return plugin;
}

bool PluginManager::remove(const std::string& dynamicLibraryFilename) {
	Plugin* pi = this->find(dynamicLibraryFilename);
	return remove(pi);
}

bool PluginManager::remove(Plugin* plugin) {
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

Plugin* PluginManager::find(const std::string& pluginTypeName) {
	for (Plugin* const plugin : *this->_plugins->list()) {
		if (plugin->getPluginInfo()->getPluginTypename() == pluginTypeName) {
			return plugin;
		}
	}
	return nullptr;
}

std::vector<std::string> PluginManager::getDataDefinitionPluginTypenames() const {
	std::vector<std::string> typeNames;
	if (_plugins == nullptr || _plugins->list() == nullptr) {
		return typeNames;
	}
	typeNames.reserve(_plugins->size()); // Reserve space to avoid reallocations
	for (Plugin* const plugin : *_plugins->list()) {
		if (plugin == nullptr || plugin->getPluginInfo() == nullptr) {
			continue;
		}
		PluginInformation* info = plugin->getPluginInfo();
		if (!info->isComponent() && info->getDataDefinitionConstructor() != nullptr) {
			typeNames.push_back(info->getPluginTypename());
		}
	}
	return typeNames;
}

std::string PluginManager::sourceIncludePathFor(const std::string& pluginTypeName) {
	Plugin* plugin = find(pluginTypeName);
	if (plugin == nullptr || plugin->getPluginInfo() == nullptr) {
		return "";
	}

	static const std::vector<std::string> kernelPlugins = {"Attribute", "Counter", "EntityType", "StatisticsCollector"};
	if (std::find(kernelPlugins.begin(), kernelPlugins.end(), pluginTypeName) != kernelPlugins.end()) {
		return "kernel/simulator/" + pluginTypeName + ".h";
	}

	PluginInformation* info = plugin->getPluginInfo();
	const std::string& pluginRoot = info->isComponent() ? "plugins/components" : "plugins/data";
	return pluginRoot + "/" + PluginInformation::categoryFolderName(info->getCategory()) + "/" + pluginTypeName + ".h";
}

Plugin* PluginManager::front() {
	return this->_plugins->front();
}

Plugin* PluginManager::next() {
	return _plugins->next();
}

Plugin* PluginManager::last() {
	return this->_plugins->last();
}

unsigned int PluginManager::size() {
	return _plugins->size();
}

Plugin* PluginManager::getAtRank(unsigned int rank) {
	return _plugins->getAtRank(rank);
}

ModelDataDefinition* PluginManager::newInstance(const std::string& pluginTypename, Model* model, std::string name) {
	Plugin* plugin = find(pluginTypename);
	if (plugin != nullptr) {
		return plugin->newInstance(model, name);
	}
	return nullptr;
}
