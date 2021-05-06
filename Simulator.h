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
#include "List.h"
#include "LicenceManager.h"
#include "PluginManager.h"
#include "ModelManager.h"
//#include "ToolManager.h"
#include "ParserManager.h"

//namespace GenesysKernel {
	/*!
	 * The main class of the ReGenesys KERNEL simulation. It gives access to simulation models and tools.
	 * Simulation is the top level class and is supossed to be available to application as a dynamic linked library.
	 */
	class Simulator {
		typedef void (*eventHandler)();
	public:
		Simulator();
		virtual ~Simulator() = default;
	public: // only get
		std::string getVersion() const;
		unsigned int getVersionNumber() const;
		std::string getName() const;
		LicenceManager* getLicenceManager() const;
		PluginManager* getPlugins() const;
		ModelManager* getModels() const;
		TraceManager* getTracer() const;
		ParserManager* getParser() const;
	private:
	private: // attributes 1:1 objects
		LicenceManager* _licenceManager;
		PluginManager* _pluginManager;
		ModelManager* _modelManager;
		TraceManager* _traceManager;
		ParserManager* _parserManager;
	private: // attributes 1:1 native
		const std::string _name = "GenESyS - GENeric and Expansible SYstem Simulator";
		const std::string _versionName = "russel";
		const unsigned int _versionNumber = 210420;
	};
//namespace\\}

// passing a "C" class factory function which instantiates the class. 
typedef /*GenesysKernel::*/Simulator* GenesysSimulator;
#endif /* GENESYS_H */

