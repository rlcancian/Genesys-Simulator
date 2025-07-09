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

#include <memory>

#include "LicenceManager.h"

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

/*!
 * Simulator constructor basically constructs inner classes (managers) and shows basic information on the console.
 * This is the only method on the entire simulator thats directlly plots something to the output
 */
Simulator::Simulator() {
	// This is the ONLY method in the entire software where std::cout is allowed.
	std::cout << "STARTING " << _name << ", version " << getVersion() << "..." << std::endl;
	_licenceManager = new LicenceManager(this);
	_pluginManager = new PluginManager(this);
	_modelManager = new ModelManager(this);
	_traceManager = new TraceManager(this);
	_experimentManager = new ExperimentManager(this);
	std::cout << '|' << '\t' << _licenceManager->showLicence() << std::endl;
    std::cout << '|' << '\t' << _licenceManager->showActivationCode() << std::endl;
    std::cout << '|' << '\t' << _licenceManager->showLimits() << std::endl;
}

PluginManager* Simulator::getPluginManager() const {
	return _pluginManager;
}

ModelManager* Simulator::getModelManager() const {
	return _modelManager;
}

TraceManager* Simulator::getTraceManager() const {
	return _traceManager;
}

ParserManager* Simulator::getParserManager() const {
	return _parserManager;
}

ExperimentManager* Simulator::getExperimentManager() const {
	return _experimentManager;
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

bool Simulator::_completePluginsFieldsAndTemplate() {
	TraceManager::Level savedTraceLevel = _traceManager->getTraceLevel();
	// this crap stuff should not been shown
	_traceManager->trace("Completing plugins and templates", TraceManager::Level::L8_detailed);
	_traceManager->setTraceLevel(TraceManager::Level::L0_noTraces); // this crap stuff should not been shown
	Model* tempModel = new Model(this);
	tempModel->getPersistence()->setOption(ModelPersistence_if::Options::SAVEDEFAULTS, true);
	auto fields = std::make_unique<PersistenceRecord>(*tempModel->getPersistence());
	Plugin* plugin;
	PluginInformation* info;
	//std::map<std::string, std::string>* fields = new std::map<std::string, std::string>();
	ModelDataDefinition* datum;
	ModelComponent* comp;
	std::string text;
	bool result = true;
	for (unsigned int i = 0; i < _pluginManager->size(); i++) {
		try {
			plugin = _pluginManager->getAtRank(i);
			info = plugin->getPluginInfo();
			if (info->getFields()->size() == 0) {
				fields->clear();
				try {
					if (info->isComponent()) {
						comp = dynamic_cast<ModelComponent*> (plugin->loadNew(tempModel, fields.get()));
						comp->setName("name");
						while (comp->getConnectionManager()->size() < info->getMinimumOutputs()) {
							comp->getConnectionManager()->insert(comp);
						}
						fields->clear();
						comp->SaveInstance(fields.get(), comp);
					} else {
						datum = plugin->loadNew(tempModel, fields.get());
						datum->setName("name");
						fields->clear();
						datum->SaveInstance(fields.get(), datum);
					}
				} catch (...) {
					//@TODO
					//std::cout << "ERROR completing plugin " << info->getPluginTypename() << std::endl;
				}
				for (auto& field : *fields.get()) {
					info->getFields()->insert({field.first, ""});
				}
				if (info->getLanguageTemplate() == "") {
					std::string templateLanguage = tempModel->getPersistence()->getFormatedField(fields.get());
					info->setLanguageTemplate(templateLanguage);
				}
			}
		} catch (...) {
			result = false;
		}
	}
	Util::ResetAllIds();
	_traceManager->setTraceLevel(savedTraceLevel);
	return result;
}
