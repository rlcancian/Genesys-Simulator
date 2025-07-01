/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Genesys.h
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 12:48
 */

#ifndef GENESYS_H
#define GENESYS_H

#include <string>
#include <iostream>
#include "Model.h"
#include "Plugin.h"
#include "../util/List.h"
#include "LicenceManager.h"
#include "PluginManager.h"
#include "ModelManager.h"
//#include "ToolManager.h"
#include "ParserManager.h"
#include "ExperimentManager.h"

//namespace GenesysKernel {

/* https://www.doxygen.nl/manual/commands.html
 * Bried description \class \implements
 *
 * Detailed description
 * \param \return
 */

/*!
 * The main class of the Genesys KERNEL simulation. It gives access to simulation models and tools.
 * Simulation is the top level class and is supossed to be available to application as a dynamic linked library.
 */
class Simulator {
	typedef void (*eventHandler)();
public:
	Simulator();
	virtual ~Simulator() = default;
public: // only get
	/*!
	 * \brief getVersion
	 * \return
	 */
	std::string getVersion() const;
	/*!
	 * \brief getVersionNumber
	 * \return
	 */
	unsigned int getVersionNumber() const;
	/*!
	 * \brief getName
	 * \return
	 */
	std::string getName() const;
	/*!
	 * \brief getLicence
	 * \return
	 */
    LicenceManager* getLicenceManager() const;
	/*!
	 * \brief getPlugins
	 * \return
	 */
    PluginManager* getPluginManager() const;
	/*!
     * \brief getModelManager
	 * \return
	 */
    ModelManager* getModelManager() const;
	/*!
     * \brief getTraceManager
	 * \return
	 */
    TraceManager* getTraceManager() const;
	/*!
     * \brief getParserManager
	 * \return
	 */
    ParserManager* getParserManager() const;
	/*!
     * \brief getExperimentManager
	 * \return
	 */
    ExperimentManager* getExperimentManager() const;

private:
	bool _completePluginsFieldsAndTemplate();
	friend class PluginManager; //@TODO: should be only member function PluginManager::completePluginsFieldsAndTemplate()

private: // attributes 1:1 objects
	LicenceManager* _licenceManager;
	PluginManager* _pluginManager;
	ModelManager* _modelManager;
	TraceManager* _traceManager;
	ParserManager* _parserManager;
	ExperimentManager* _experimentManager;

private: // attributes 1:1 native
	const std::string _name = "GenESyS - GENeric and Expansible SYstem Simulator";
	const std::string _versionName = "turnaroud";
	const unsigned int _versionNumber = 230914;
};
//namespace\\}

// passing a "C" class factory function which instantiates the class. 
typedef /*GenesysKernel::*/Simulator* GenesysSimulator;
#endif /* GENESYS_H */

