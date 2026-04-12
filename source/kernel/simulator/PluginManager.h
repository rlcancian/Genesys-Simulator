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

#include <string>
#include "../util/List.h"
#include "../util/Util.h"
#include "Plugin.h"
#include "PluginConnector_if.h"

class Simulator;
class Model;

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
	bool check(const std::string dynamicLibraryFilename);
	/*! \brief Loads and inserts a plugin from a dynamic library file. */
	Plugin* insert(const std::string dynamicLibraryFilename);
	/*! \brief Removes/disconnects a plugin by its dynamic library filename. */
	bool remove(const std::string dynamicLibraryFilename);
	/*! \brief Removes/disconnects a plugin by pointer. */
	bool remove(Plugin* plugin);
	/*! \brief Finds a connected plugin by plugin type name. */
	Plugin* find(std::string pluginTypeName);
	/*! \brief Auto-loads plugins listed in file (or discovered automatically as fallback). */
    List<Plugin*>* autoInsertPlugins(const std::string pluginsListFilename, const bool lookForPluginsIfFilenameNotFound = true);
    /*! \brief Auto-loads plugins discovered automatically). */
    List<Plugin*>* autoInsertPlugins();
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
		// Keep this header free of Simulator implementation details to avoid include cycles.
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
