/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   DefaultNode.cpp
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 * 
 * Created on 01 de Julho de 2025, 14:26
 */

#include "plugins/components/ModalModel/DefaultNode.h"
#include "kernel/simulator/Model.h"
#include <algorithm>
//#include "kernel/simulator/Simulator.h"
//#include "kernel/simulator/PluginManager.h"

DefaultNodeTransition::DefaultNodeTransition(DefaultNode* source, DefaultNode* destination, std::string name) {
	_source = source;
	_destination = destination;
	_name = name;
}

void DefaultNodeTransition::setSource(DefaultNode* source) {
	_source = source;
}

DefaultNode* DefaultNodeTransition::getSource() const {
	return _source;
}

void DefaultNodeTransition::setDestination(DefaultNode* destination) {
	_destination = destination;
}

DefaultNode* DefaultNodeTransition::getDestination() const {
	return _destination;
}

void DefaultNodeTransition::setName(std::string name) {
	_name = name;
}

std::string DefaultNodeTransition::getName() const {
	return _name;
}

void DefaultNodeTransition::setGuardExpression(std::string guardExpression) {
	_guardExpression = guardExpression;
}

std::string DefaultNodeTransition::getGuardExpression() const {
	return _guardExpression;
}

void DefaultNodeTransition::setOutputExpression(std::string outputExpression) {
	_outputExpression = outputExpression;
}

std::string DefaultNodeTransition::getOutputExpression() const {
	return _outputExpression;
}

void DefaultNodeTransition::setInputEvent(std::string inputEvent) {
	_inputEvent = inputEvent;
}

std::string DefaultNodeTransition::getInputEvent() const {
	return _inputEvent;
}

void DefaultNodeTransition::setPriority(unsigned int priority) {
	_priority = priority;
}

unsigned int DefaultNodeTransition::getPriority() const {
	return _priority;
}

void DefaultNodeTransition::setProbability(double probability) {
	_probability = probability;
}

double DefaultNodeTransition::getProbability() const {
	return _probability;
}

void DefaultNodeTransition::setTransitionKind(TransitionKind transitionKind) {
	_transitionKind = transitionKind;
}

DefaultNodeTransition::TransitionKind DefaultNodeTransition::getTransitionKind() const {
	return _transitionKind;
}

bool DefaultNodeTransition::canFire(Model* model, Entity* entity) const {
	(void) entity;
	if (_guardExpression == "") {
		return true;
	}
	return model->parseExpression(_guardExpression) != 0.0;
}

void DefaultNodeTransition::execute(Model* model, Entity* entity) const {
	(void) entity;
	if (_outputExpression != "") {
		model->parseExpression(_outputExpression);
	}
}



/// Externalize function GetPluginInformation to be accessible throught dynamic linked library
#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
    return &DefaultNode::GetPluginInformation;
}
#endif


//
// public: /// constructors
//

DefaultNode::DefaultNode(Model* model, std::string componentTypename, std::string name) : ModelComponent(model, componentTypename, name), DEFAULT() {
}

DefaultNode::DefaultNode(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<DefaultNode>(), name), DEFAULT() {
}


//
// public: /// new public user methods for this component
//

void DefaultNode::addTransition(DefaultNodeTransition* transition) {
	if (transition != nullptr) {
		_transitions->insert(transition);
	}
}

void DefaultNode::removeTransition(DefaultNodeTransition* transition) {
	_transitions->remove(transition);
}

List<DefaultNodeTransition*>* DefaultNode::getTransitions() const {
	return _transitions;
}

void DefaultNode::setInitialNode(bool initialNode) {
	_initialNode = initialNode;
}

bool DefaultNode::isInitialNode() const {
	return _initialNode;
}

void DefaultNode::setFinalNode(bool finalNode) {
	_finalNode = finalNode;
}

bool DefaultNode::isFinalNode() const {
	return _finalNode;
}


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
		_initialNode = fields->loadField("initialNode", DEFAULT.initialNode);
		_finalNode = fields->loadField("finalNode", DEFAULT.finalNode);
	}
	return res;
}

void DefaultNode::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("initialNode", _initialNode, DEFAULT.initialNode, saveDefaultValues);
	fields->saveField("finalNode", _finalNode, DEFAULT.finalNode, saveDefaultValues);
}

void DefaultNode::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	traceSimulation(this, "I'm just a dummy model and I'll just send the entity forward");
	this->_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}


//
// protected: /// virtual methods that could be overriden by derived classes, if needed
//

/*
bool DefaultNode::_check(std::string& errorMessage) {
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
	}
	if (_internalDataDefinition != nullptr) {
		_internalDataInsert("JustaDummy", _internalDataDefinition);
	} else {
		_internalDataRemove("JustaDummy");
	}
	if (_attachedDataDefinition == nullptr) {
		PluginManager* pm = _parentModel->getParentSimulator()->getPlugins();
		_attachedDataDefinition = pm->newInstance<DummyElement>(_parentModel);
	}
	if (_attachedDataDefinition != nullptr) {
		_attachedDataInsert("JustaDummy", _attachedDataDefinition);
	} else {
		_attachedDataRemove("JustaDummy");
	}
}
*/

/*
void DefaultNode::_addProperty(SimulationControl* property) {
}
*/
