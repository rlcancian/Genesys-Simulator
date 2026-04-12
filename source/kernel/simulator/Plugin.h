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
	Plugin(std::string filename_so_dll);
	Plugin(StaticGetPluginInformation getInformation); // @TODO: temporary. Just while compiled together
	// Destroy plugin-owned metadata allocated by the plugin-information callback.
	virtual ~Plugin();
public:
	std::string show();
public:
	/*!
	 * \brief isIsValidPlugin
	 * \return
	 */
	bool isIsValidPlugin() const;
	/*!
	 * \brief getPluginInfo
	 * \return
	 */
	PluginInformation* getPluginInfo() const;
	/*!
	 * \brief loadNew
	 * \param model
	 * \param fields
	 * \return
	 */
	ModelDataDefinition* loadNew(Model* model, PersistenceRecord *fields); //!< creates a new ModelDataDefinition from fields loaded from a file
	/*!
     * \brief loadAndInsertNew
     * \param model
     * \param fields
     * \return
	*/
	bool loadAndInsertNew(Model* model, PersistenceRecord *fields);
	/*!
	 * \brief newInstance
	 * \param model
	 * \param name
	 * \return
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
