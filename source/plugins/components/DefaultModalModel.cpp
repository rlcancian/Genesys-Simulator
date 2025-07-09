/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   DefaultModalModel.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 01 de Julho de 2025, 14:26
 */

#include "DefaultModalModel.h"
#include "../../kernel/simulator/Model.h"
//#include "../../kernel/simulator/Simulator.h"
//#include "../../kernel/simulator/PluginManager.h"



/// Externalize function GetPluginInformation to be accessible throught dynamic linked library
#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
    return &DefaultModalModel::GetPluginInformation;
}
#endif


//
// public: /// constructors
//

DefaultModalModel::DefaultModalModel(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<DefaultModalModel>(), name) {
}


//
// public: /// new public user methods for this component
//

void DefaultModalModel::addNode(DefaultNode* node){
    _nodes->insert(node);
}
void DefaultModalModel::removeNode(DefaultNode* node){
    _nodes->remove(node);
}
void DefaultModalModel::addTransition(DefaultNodeTransition* transition){
    _transitions->insert(transition);
}
void DefaultModalModel::removeTransition(DefaultNodeTransition* transition){
    _transitions->remove(transition);
}
// ...


//
// public: /// virtual methods
//

std::string DefaultModalModel::show() {
	return ModelComponent::show() + "";
}


//
// public: /// static methods that must have implementations (Load and New just the same. GetInformation must provide specific infos for the new component
//

PluginInformation* DefaultModalModel::GetPluginInformation() {
    PluginInformation* info = new PluginInformation(Util::TypeOf<DefaultModalModel>(), &DefaultModalModel::LoadInstance, &DefaultModalModel::NewInstance);
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

ModelComponent* DefaultModalModel::LoadInstance(Model* model, PersistenceRecord *fields) {
    DefaultModalModel* newComponent = new DefaultModalModel(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

ModelDataDefinition* DefaultModalModel::NewInstance(Model* model, std::string name) {
    return new DefaultModalModel(model, name);
}

//
// protected: /// virtual method that must be overriden
//

bool DefaultModalModel::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		// @TODO: not implemented yet
	}
	return res;
}

void DefaultModalModel::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	// @TODO: not implemented yet
}

void DefaultModalModel::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	traceSimulation(this, "I'm just a dummy model and I'll just send the entity forward");
	this->_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}


//
// protected: /// virtual methods that could be overriden by derived classes, if needed
//

/*
bool DefaultModalModel::_check(std::string* errorMessage) {
	bool resultAll = true;
	resultAll &= _someString != "";
	resultAll &= _someUint > 0;
	return resultAll;
}
*/

/*
ParserChangesInformation* DefaultModalModel::_getParserChangesInformation() {
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
void DefaultModalModel::_initBetweenReplications() {
	_someString = "Test";
	_someUint = 1;
}
*/

/*
void DefaultModalModel::_createInternalAndAttachedData() {
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
void DefaultModalModel::_addProperty(PropertyBase* property) {
}
*/
