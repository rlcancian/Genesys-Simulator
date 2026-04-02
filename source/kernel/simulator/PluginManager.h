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

#include "../util/List.h"
//#include "Simulator.h"
#include "Plugin.h"
#include "PluginConnector_if.h"

//namespace GenesysKernel {
class Simulator;

class PluginManager {
public:
	PluginManager(Simulator* simulator);
	virtual ~PluginManager() = default;
public:
	std::string show();
public:
	List<Plugin*>* completePluginsFieldsAndTemplates();
public:
	bool check(const std::string dynamicLibraryFilename);
	Plugin* insert(const std::string dynamicLibraryFilename);
	bool remove(const std::string dynamicLibraryFilename);
	bool remove(Plugin* plugin);
	Plugin* find(std::string pluginTypeName);
	List<Plugin*>* autoInsertPlugins(const std::string pluginsListFilename, const bool lookForPluginsIfFilenameNotFound = true);
public:
	Plugin* front();
	Plugin* next();
	Plugin* last();
	unsigned int size();
	Plugin* getAtRank(unsigned int rank);
public:
	ModelDataDefinition* newInstance(std::string pluginTypename, Model* model, std::string name = "");

	template <typename T>T* newInstance(Model* model, std::string name = "") {
		name = Util::StrReplace(name, " ", "_");
		Plugin* plugin;
		std::string pluginTypename = Util::TypeOf<T>();
		//@TODO: Use Find method??
		for (unsigned short i = 0; i < _plugins->size(); i++) {
			plugin = _plugins->getAtRank(i);
			if (plugin->getPluginInfo()->getPluginTypename() == pluginTypename) {
				T* instance;
				StaticConstructorDataDefinitionInstance constructor = plugin->getPluginInfo()->getDataDefinitionConstructor();
				instance = static_cast<T*> (constructor(model, name));
				return instance;
			}
		}
		// innvalid use of incomplete class
		///_simulator->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "Error: Could not find any plugin with Typename \"" + pluginTypename + "\"");
		return nullptr;
	}
private:
	bool _insert(Plugin* plugin);
	void _insertDefaultKernelElements();
	List<Plugin*>* _autoFindPlugins();
private:
	List<Plugin*>* _plugins = new List<Plugin*>();
	Simulator* _simulator;
	PluginConnector_if* _pluginConnector;
};
//namespace\\}
#endif /* PLUGINMANAGER_H */
