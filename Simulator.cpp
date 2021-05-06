/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Genesys.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 21 de Junho de 2018, 12:48
 */

#include "Simulator.h"
#include "LicenceManager.h"
//#include "ToolManager.h"

extern "C" GenesysSimulator CreateSimulator2() {
	return new /*GenesysKernel::*/Simulator();
}

extern "C" /*GenesysKernel::*/Simulator* CreateSimulator() {
	return new /*GenesysKernel::*/Simulator();
}

extern "C" void DestroySimulator2(GenesysSimulator p) {
	delete p; // Can use a base class or derived class pointer here
}

extern "C" void DestroySimulator(/*GenesysKernel::*/Simulator* p) {
	delete p; // Can use a base class or derived class pointer here
}


//using namespace GenesysKernel;

Simulator::Simulator() {
	// This is the ONLY method in the entire software where std::cout is allowed.
	std::cout << "STARTING " << _name << ", version " << getVersion() << std::endl;
	_licenceManager = new LicenceManager(this);
	_pluginManager = new PluginManager(this);
	_modelManager = new ModelManager(this);
	//_toolManager = new ToolManager(this);
	_traceManager = new TraceManager(this);
	std::cout << '|' << '\t' << _licenceManager->showLicence() << std::endl;
	//std::cout << '|' << '\t' << _licenceManager->showActivationCode() << std::endl;
	//std::cout << '|' << '\t' << _licenceManager->showLimits() << std::endl;
}

PluginManager* Simulator::getPlugins() const {
	return _pluginManager;
}

ModelManager* Simulator::getModels() const {
	return _modelManager;
}

TraceManager* Simulator::getTracer() const {
	return _traceManager;
}

ParserManager* Simulator::getParser() const {
	return _parserManager;
}

std::string Simulator::getVersion() const {
	return std::to_string(_versionNumber) + " (" + _versionName + ")";
}

unsigned int Simulator::getVersionNumber() const {
	return _versionNumber;
}

std::string Simulator::getName() const {
	return _name;
}

LicenceManager* Simulator::getLicenceManager() const {
	return _licenceManager;
}

