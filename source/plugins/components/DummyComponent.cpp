/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Dummy.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 22 de Maio de 2019, 18:41
 */

#include "DummyComponent.h"
#include "../../kernel/simulator/Model.h"
//#include "../../kernel/simulator/Simulator.h"
//#include "../../kernel/simulator/PluginManager.h"



/// Externalize function GetPluginInformation to be accessible throught dynamic linked library
#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &DummyComponent::GetPluginInformation;
}
#endif


//
// public: /// constructors
//

DummyComponent::DummyComponent(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<DummyComponent>(), name) {
}


//
// public: /// new public user methods for this component
//

// ...


//
// public: /// virtual methods
//

std::string DummyComponent::show() {
	return ModelComponent::show() + "";
}


//
// public: /// static methods that must have implementations (Load and New just the same. GetInformation must provide specific infos for the new component
//

PluginInformation* DummyComponent::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<DummyComponent>(), &DummyComponent::LoadInstance, &DummyComponent::NewInstance);
	//info->setCategory("Discrete Processing");
	//info->setMinimumInputs(1);
	//info->setMinimumOutputs(1);
	//info->setMaximumInputs(1);
	//info->setMaximumOutputs(1);
	//info->setSource(false);
	//info->setSink(false);
	//info->setSendTransfer(false);
	//info->setReceiveTransfer(false);
	//info->insertDynamicLibFileDependence("...");
	//info->setDescriptionHelp("//@TODO");
	//info->setAuthor("...");
	//info->setDate("...");
	//info->setObservation("...");
	return info;
}

ModelComponent* DummyComponent::LoadInstance(Model* model, PersistenceRecord *fields) {
	DummyComponent* newComponent = new DummyComponent(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

ModelDataDefinition* DummyComponent::NewInstance(Model* model, std::string name) {
	return new DummyComponent(model, name);
}

//
// protected: /// virtual method that must be overriden
//

bool DummyComponent::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		// @TODO: not implemented yet
	}
	return res;
}

void DummyComponent::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	// @TODO: not implemented yet
}

void DummyComponent::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	traceSimulation(this, "I'm just a dummy model and I'll just send the entity forward");
	this->_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}


//
// protected: /// virtual methods that could be overriden by derived classes, if needed
//

/*
bool DummyComponent::_check(std::string* errorMessage) {
	bool resultAll = true;
	resultAll &= _someString != "";
	resultAll &= _someUint > 0;
	return resultAll;
}
*/

/*
ParserChangesInformation* DummyComponent::_getParserChangesInformation() {
	ParserChangesInformation* changes = new ParserChangesInformation();
	//@TODO not implemented yet
	changes->getassignments().append("");
	changes->getexpressionProductions().append("");
	changes->getexpressions().append("");
	changes->getfunctionProdutions().append("");
	changes->getassignments().append("");
	changes->getincludes().append("");
	changes->gettokens().append("");
	changes->gettypeObjs().append("");
	return changes;
}
*/

/*
void DummyComponent::_initBetweenReplications() {
	_someString = "Test";
	_someUint = 1;
}
*/

/*
void DummyComponent::_createInternalAndAttachedData() {
	if (_internalDataDefinition == nullptr) {
		PluginManager* pm = _parentModel->getParentSimulator()->getPlugins();
		_internalDataDefinition = pm->newInstance<DummyElement>(_parentModel, getName() + "." + "JustaDummy");
		_internalDataInsert("JustaDummy", _internalDataDefinition);
	}
	if (_attachedDataDefinition == nullptr) {
		PluginManager* pm = _parentModel->getParentSimulator()->getPlugins();
		_attachedDataDefinition = pm->newInstance<DummyElement>(_parentModel);
		_attachedDataInsert("JustaDummy", _attachedDataDefinition);
	}
}
*/

/*
void DummyComponent::_addProperty(PropertyBase* property) {
}
*/
