/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   DefaultNode.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 01 de Julho de 2025, 14:26
 */

#include "DefaultNode.h"
#include "../../../kernel/simulator/Model.h"
//#include "../../kernel/simulator/Simulator.h"
//#include "../../kernel/simulator/PluginManager.h"



/// Externalize function GetPluginInformation to be accessible throught dynamic linked library
#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
    return &DefaultNode::GetPluginInformation;
}
#endif


//
// public: /// constructors
//

DefaultNode::DefaultNode(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<DefaultNode>(), name) {
}


//
// public: /// new public user methods for this component
//

// ...


//
// public: /// virtual methods
//

std::string DefaultNode::show() {
	return ModelComponent::show() + "";
}


//
// public: /// static methods that must have implementations (Load and New just the same. GetInformation must provide specific infos for the new component
//

PluginInformation* DefaultNode::GetPluginInformation() {
    PluginInformation* info = new PluginInformation(Util::TypeOf<DefaultNode>(), &DefaultNode::LoadInstance, &DefaultNode::NewInstance);
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

ModelComponent* DefaultNode::LoadInstance(Model* model, PersistenceRecord *fields) {
    DefaultNode* newComponent = new DefaultNode(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

ModelDataDefinition* DefaultNode::NewInstance(Model* model, std::string name) {
    return new DefaultNode(model, name);
}

//
// protected: /// virtual method that must be overriden
//

bool DefaultNode::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		// @TODO: not implemented yet
	}
	return res;
}

void DefaultNode::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	// @TODO: not implemented yet
}

void DefaultNode::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	traceSimulation(this, "I'm just a dummy model and I'll just send the entity forward");
	this->_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}


//
// protected: /// virtual methods that could be overriden by derived classes, if needed
//

/*
bool DefaultNode::_check(std::string* errorMessage) {
	bool resultAll = true;
	resultAll &= _someString != "";
	resultAll &= _someUint > 0;
	return resultAll;
}
*/

/*
ParserChangesInformation* DefaultNode::_getParserChangesInformation() {
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
void DefaultNode::_initBetweenReplications() {
	_someString = "Test";
	_someUint = 1;
}
*/

/*
void DefaultNode::_createInternalAndAttachedData() {
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
void DefaultNode::_addProperty(PropertyBase* property) {
}
*/
