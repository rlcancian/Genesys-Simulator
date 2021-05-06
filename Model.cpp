/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SimulationModel.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 21 de Junho de 2018, 15:01
 */

#include <typeinfo>
#include <iostream>
#include <algorithm>
#include <string>

#include "Model.h"
#include "SourceModelComponent.h"
#include "Simulator.h"
#include "StatisticsCollector.h"
#include "Traits.h"
//#include "Access.h"

//using namespace GenesysKernel;

bool EventCompare(const Event* a, const Event * b) {
	return a->getTime() < b->getTime();
}

Model::Model(Simulator* simulator) {
	_parentSimulator = simulator; // a simulator is the "parent" of a model
	// 1:1 associations (no Traits)
	_traceManager = simulator->getTracer(); // every model starts with the same tracer, unless a specific one is set
	_modelInfo = new ModelInfo();
	_eventManager = new OnEventManager(); // should be on .h (all that does not depends on THIS)
	_elementManager = new ElementManager(this);
	_componentManager = new ComponentManager(this);
	_simulation = new ModelSimulation(this);
	// 1:1 associations (Traits)
	//Sampler_if* sampler = new Traits<Sampler_if>::Implementation();
	_parser = new Traits<Parser_if>::Implementation(this, new Traits<Sampler_if>::Implementation());
	_modelChecker = new Traits<ModelChecker_if>::Implementation(this);
	_modelPersistence = new Traits<ModelPersistence_if>::Implementation(this);
	// 1:n associations
	_futureEvents = new List<Event*>(); /// The future events list must be chronologicaly sorted
	//_events->setSortFunc(&EventCompare); // It works too
	_futureEvents->setSortFunc([](const Event* a, const Event * b) {
		return a->getTime() < b->getTime(); /// Events are sorted chronologically
	});
	// for process analyser
	_responses = new List<SimulationResponse*>();
	_controls = new List<SimulationControl*>();
	// insert controls
	_controls->insert(new SimulationControl("ModelSimulation", "NumberOfReplications",
			DefineGetterMember<ModelSimulation>(this->_simulation, &ModelSimulation::getNumberOfReplications),
			DefineSetterMember<ModelSimulation>(this->_simulation, &ModelSimulation::setNumberOfReplications))
			);
	_controls->insert(new SimulationControl("ModelSimulation", "ReplicationLength",
			DefineGetterMember<ModelSimulation>(this->_simulation, &ModelSimulation::getReplicationLength),
			DefineSetterMember<ModelSimulation>(this->_simulation, &ModelSimulation::setReplicationLength))
			);
	_controls->insert(new SimulationControl("ModelSimulation", "WarmupPeriod",
			DefineGetterMember<ModelSimulation>(this->_simulation, &ModelSimulation::getWarmUpPeriod),
			DefineSetterMember<ModelSimulation>(this->_simulation, &ModelSimulation::setWarmUpPeriod))
			);
}

void Model::sendEntityToComponent(Entity* entity, Connection* connection, double timeDelay) {
	this->sendEntityToComponent(entity, connection->first, timeDelay, connection->second);
}

void Model::sendEntityToComponent(Entity* entity, ModelComponent* component, double timeDelay, unsigned int componentInputNumber) {
	this->getOnEvents()->NotifyEntityMoveHandlers(new SimulationEvent(_simulation->getCurrentReplicationNumber(), new Event(_simulation->getSimulatedTime(), entity, component, componentInputNumber))); //\todo: Event should include information about "from component" and timeDelay, but it doesn't
	//// if (timeDelay > 0) {
		// schedule to send it
		Event* newEvent = new Event(this->getSimulation()->getSimulatedTime() + timeDelay, entity, component, componentInputNumber);
		this->getFutureEvents()->insert(newEvent);
	//// } else {
	//// 	// send it now
	//// 	/*  \todo: -: supposed not to be a queue associated to a component */
	//// 	Util::DecIndent();
	//// 	ModelComponent::Execute(entity, component, componentInputNumber);
	//// 	Util::IncIndent();
	//// }
}

bool Model::save(std::string filename) {
	bool res = this->_modelPersistence->save(filename);
	if (res) {
		this->_traceManager->trace(Util::TraceLevel::L3_results, "Model successfully saved");
	} else {
		this->_traceManager->trace(Util::TraceLevel::L3_results, "Model could not be saved");

	}
	return res;
}

bool Model::load(std::string filename) {
	this->clear();
	bool res = this->_modelPersistence->load(filename);
	if (res)
		this->_traceManager->trace(Util::TraceLevel::L3_results, "Model successfully loaded");
	else
		this->_traceManager->trace(Util::TraceLevel::L3_results, "Model could not be loaded");
	return res;
}

double Model::parseExpression(const std::string expression) {
	try {
		return _parser->parse(expression);
	} catch (...) {
		return 0.0; // \todo: HOW SAY THERE WAS AN ERROR?
	}
}

bool Model::checkExpression(const std::string expression, const std::string expressionName, std::string* errorMessage) {
	bool result;
	getTracer()->trace("Checking expression \"" + expression + "\"");
	try {
		parseExpression(expression, &result, errorMessage);
	} catch (...) {
		result = false;
	}
	if (!result) {
		std::string msg = "Expression \"" + expression + "\" for '" + expressionName + "' is incorrect. ";
		errorMessage->append(msg);
	}
	return result;
}

double Model::parseExpression(const std::string expression, bool* success, std::string* errorMessage) {
	double value = _parser->parse(expression, success, errorMessage);
	return value;
}

void Model::show() {
	getTracer()->trace(Util::TraceLevel::L3_results, "Simulation Model:");
	Util::IncIndent();
	{
		getTracer()->trace(Util::TraceLevel::L3_results, "Information:");
		Util::IncIndent();
		getTracer()->trace(Util::TraceLevel::L3_results, this->getInfos()->show());
		Util::DecIndent();
		_showConnections();
		_showComponents();
		_showElements();
		_showSimulationControls();
		_showSimulationResponses();
	}
	Util::DecIndent();
	getTracer()->trace(Util::TraceLevel::L3_results, "End of Simulation Model");
}

bool Model::insert(ModelElement* elemOrComp) {
	ModelComponent* comp = dynamic_cast<ModelComponent*> (elemOrComp);
	if (comp == nullptr) // it's a ModelElement
		return this->getElements()->insert(elemOrComp);
	else // it's a ModelComponent
		return this->getComponents()->insert(comp);
}

void Model::remove(ModelElement* elemOrComp) {
	ModelComponent* comp = dynamic_cast<ModelComponent*> (elemOrComp);
	if (comp == nullptr) // it's a ModelElement
		this->getElements()->remove(elemOrComp);
	else // it's a ModelComponent
		this->getComponents()->remove(comp);
}

void Model::_showElements() const {
	getTracer()->trace(Util::TraceLevel::L3_results, "Elements:");
	Util::IncIndent();
	{
		std::string elementType;
		ModelElement* element;
		std::list<std::string>* elementTypes = getElements()->getElementClassnames();
		for (std::list<std::string>::iterator typeIt = elementTypes->begin(); typeIt != elementTypes->end(); typeIt++) {
			elementType = (*typeIt);
			List<ModelElement*>* em = getElements()->getElementList(elementType);
			getTracer()->trace(Util::TraceLevel::L3_results, elementType + ":");
			Util::IncIndent();
			{
				for (std::list<ModelElement*>::iterator it = em->list()->begin(); it != em->list()->end(); it++) {
					element = (*it);
					getTracer()->trace(Util::TraceLevel::L3_results, element->show());
				}
			}
			Util::DecIndent();
		}
	}
	Util::DecIndent();
}

void Model::_showConnections() const {
	// \todo
}

void Model::_showComponents() const {
	getTracer()->trace(Util::TraceLevel::L3_results, "Components:");
	Util::IncIndent();
	for (std::list<ModelComponent*>::iterator it = getComponents()->begin(); it != getComponents()->end(); it++) {
		getTracer()->trace(Util::TraceLevel::L3_results, (*it)->show()); ////
	}
	Util::DecIndent();
}

void Model::_showSimulationControls() const {
	getTracer()->trace(Util::TraceLevel::L3_results, "Simulation Controls:");
	Util::IncIndent();
	for (std::list<SimulationControl*>::iterator it = _controls->list()->begin(); it != _controls->list()->end(); it++) {
		getTracer()->trace(Util::TraceLevel::L3_results, (*it)->show()); ////
	}
	Util::DecIndent();
}

void Model::_showSimulationResponses() const {
	getTracer()->trace(Util::TraceLevel::L3_results, "Simulation Responses:");
	Util::IncIndent();
	for (std::list<SimulationResponse*>::iterator it = _responses->list()->begin(); it != _responses->list()->end(); it++) {
		getTracer()->trace(Util::TraceLevel::L3_results, (*it)->show()); ////
	}
	Util::DecIndent();
}

void Model::clear() {
	this->_componentManager->clear();
	this->_elementManager->clear();
	this->_futureEvents->clear();
	//this->_simulation->clear();  // \todo clear method
	//this->_modelInfo->clear(); // \todo clear method
	//Util::ResetAllIds(); // \todo: To implement
}

void Model::_createModelInternalElements() {
	getTracer()->trace(Util::TraceLevel::L6_internal, "Creating internal elements");
	Util::IncIndent();

	for (ModelComponent* component : *_componentManager) {
		getTracer()->trace(Util::TraceLevel::L6_internal, "Internals for " + component->getClassname() + " \"" + component->getName() + "\"");
		Util::IncIndent();
		ModelComponent::CreateInternalElements(component);
		Util::DecIndent();
	}

	std::list<ModelElement*>* modelElements;
	unsigned int originalSize = getElements()->getElementClassnames()->size(), pos = 1;
	//for (std::list<std::string>::iterator itty = elements()->elementClassnames()->begin(); itty != elements()->elementClassnames()->end(); itty++) {
	std::list<std::string>::iterator itty = getElements()->getElementClassnames()->begin();
	while (itty != getElements()->getElementClassnames()->end() && pos <= originalSize) {
		//try {
		modelElements = getElements()->getElementList((*itty))->list();
		//} catch (const std::exception& e) {
		// \todo Is there a better solution to iterate over a changing sorted list??
		// ops. Sorted list has changed and iteration fails. Starts iterating again
		//	itty = elements()->elementClassnames()->begin();
		//	modelElements = elements()->elementList((*itty))->list();
		//	tracer()->trace(Util::TraceLevel::L6_internal, "Creating internal elements");
		//}
		for (std::list<ModelElement*>::iterator itel = modelElements->begin(); itel != modelElements->end(); itel++) {
			getTracer()->trace(Util::TraceLevel::L6_internal, "Internals for " + (*itel)->getClassname() + " \"" + (*itel)->getName() + "\""); // (" + std::to_string(pos) + "/" + std::to_string(originalSize) + ")");
			Util::IncIndent();
			ModelElement::CreateInternalElements((*itel));
			Util::DecIndent();
		}
		if (originalSize == getElements()->getElementClassnames()->size()) {
			itty++;
			pos++;
		} else {
			originalSize = getElements()->getElementClassnames()->size();
			itty = getElements()->getElementClassnames()->begin();
			pos = 1;
			getTracer()->trace(Util::TraceLevel::L6_internal, "Restarting to create internal elements (due to previous creations)");
		}
	}
	Util::DecIndent();
}

bool Model::check() {
	getTracer()->trace(Util::TraceLevel::L6_internal, "Checking model consistency");
	Util::IncIndent();
	// before checking the model, creates all necessary internal ModelElements
	_createModelInternalElements();
	bool res = this->_modelChecker->checkAll();
	Util::DecIndent();
	if (res) {
		getTracer()->trace(Util::TraceLevel::L3_results, "End of Model checking: Success");
	} else {
		//std::exception e = new std::exception();
		//getTrace()->traceError() ;
		getTracer()->trace(Util::TraceLevel::L3_results, "End of Model checking: Failed");
	}
	return res;
}

//bool Model::verifySymbol(std::string componentName, std::string expressionName, std::string expression, std::string expressionResult, bool mandatory) {
//    return this->_modelChecker->verifySymbol(componentName, expressionName, expression, expressionResult, mandatory);
//}

void Model::removeEntity(Entity* entity, bool collectStatistics) {
	/*  \todo: -: event onEntityRemove */
	/*  \todo: -: collectStatistics */
	std::string entId = std::to_string(entity->entityNumber());
	this->getElements()->remove(Util::TypeOf<Entity>(), entity);
	getTracer()->trace("Entity " + entId + " was removed from the system");
}

List<Event*>* Model::getFutureEvents() const {
	return _futureEvents;
}

void Model::setTracer(TraceManager * _traceManager) {
	this->_traceManager = _traceManager;
}

TraceManager * Model::getTracer() const {
	return _traceManager;
}

bool Model::hasChanged() const {
	bool changed = _hasChanged;
	changed &= this->_componentManager->hasChanged();
	changed &= this->_elementManager->hasChanged();
	changed &= this->_modelInfo->hasChanged();
	changed &= this->_modelPersistence->hasChanged();
	return changed;
}

ComponentManager * Model::getComponents() const {
	return _componentManager;
}

List<SimulationControl*>* Model::getControls() const {
	return _controls;
}

List<SimulationResponse*>* Model::getResponses() const {
	return _responses;
}

OnEventManager * Model::getOnEvents() const {
	return _eventManager;
}

ElementManager * Model::getElements() const {
	return _elementManager;
}

ModelInfo * Model::getInfos() const {
	return _modelInfo;
}

Simulator * Model::getParentSimulator() const {
	return _parentSimulator;
}

ModelSimulation * Model::getSimulation() const {
	return _simulation;
}

Util::identification Model::getId() const {
	return _id;
}

