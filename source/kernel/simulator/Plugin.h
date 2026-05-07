/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Plugin.h
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 12:58
 */

#ifndef PLUGIN_H
#define PLUGIN_H

#include <string>
#include <functional>
#include <list>

#include "../util/Util.h"
#include "Persistence.h"
#include "PluginInformation.h"

//namespace GenesysKernel {

/*!
 * A Plugin represents a dynamically linked component class (ModelComponent) or modeldatum class (ModelDataDefinition); It gives access to a ModelComponent so it can be used by the model. Classes like Create, Delay, and Dispose are examples of PlugIns.  It corresponds directly to the  "Expansible" part (the capitalized 'E') of the GenESyS acronymous
PlugIns are NOT implemented yet
 */
class Plugin {
public:
	/*!
	 * \brief Creates a plugin wrapper for a dynamic library file.
	 * \param filename_so_dll Plugin library filename or path.
	 */
	Plugin(std::string filename_so_dll);
	/*!
	 * \brief Creates a plugin wrapper from a compiled-in metadata callback.
	 * \param getInformation Callback that returns the plugin metadata.
	 */
	Plugin(StaticGetPluginInformation getInformation); // @ToDo: (importante): temporary. Just while compiled together
	/*!
	 * \brief Releases metadata allocated for the wrapped plugin.
	 */
	virtual ~Plugin();
public:
	/*!
	 * \brief Returns a human-readable description of the plugin state.
	 * \return Textual summary used in traces and diagnostics.
	 */
	std::string show();
public:
	/*!
	 * \brief Indicates whether the plugin metadata was successfully initialized.
	 * \return \c true when this wrapper contains a valid plugin.
	 */
	bool isIsValidPlugin() const;
	/*!
	 * \brief Returns the plugin metadata object.
	 * \return Plugin information block, or \c nullptr when invalid.
	 */
	PluginInformation* getPluginInfo() const;
	/*!
	 * \brief Creates a new data definition instance from serialized fields.
	 * \details Creates a new ModelDataDefinition from fields loaded from a
	 * file or another persistence source.
	 * \param model Parent model that will own the new instance.
	 * \param fields Serialized fields to deserialize.
	 * \return New data definition instance, or \c nullptr on failure.
	 */
	ModelDataDefinition* loadNew(Model* model, PersistenceRecord *fields);
	/*!
     * \brief Loads a new instance and inserts it into the model in one step.
     * \param model Parent model that will own the new instance.
     * \param fields Serialized fields to deserialize.
     * \return \c true when loading and insertion succeed.
	*/
	bool loadAndInsertNew(Model* model, PersistenceRecord *fields);
	/*!
	 * \brief Creates a new in-memory instance of the plugin's primary type.
	 * \param model Parent model that will own the instance.
	 * \param name Initial object name.
	 * \return Newly created instance, or \c nullptr on failure.
	 */
	ModelDataDefinition* newInstance(Model* model, std::string name = "");

private:
	ModelComponent* _loadNewComponent(Model* model, PersistenceRecord *fields);
	ModelDataDefinition* _loadNewElement(Model* model, PersistenceRecord *fields);

private: // read only
	// Default invalid state avoids reading indeterminate validity after construction failures.
	bool _isValidPlugin = false;
	// Store plugin metadata pointer with a safe default for invalid plugin construction paths.
	PluginInformation* _pluginInfo = nullptr;
private:
	StaticGetPluginInformation _StatMethodGetInformation;
};
//namespace\\}
#endif /* PLUGIN_H */
