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
#include "OnEventManager.h"
#include "StatisticsCollector.h"
#include "../TraitsKernel.h"
//#include "Access.h"

//using namespace GenesysKernel;

bool EventCompare(const Event* a, const Event * b) {
	return a->getTime() < b->getTime();
}

Model::Model(Simulator* simulator, unsigned int level) {
	_parentSimulator = simulator; // a simulator is the "parent" of a model
	_level = level;
	// 1:1 associations (no Traits)
	_traceManager = simulator->getTracer(); // every model starts with the same tracer, unless a specific one is set
	_modelInfo = new ModelInfo();
	_eventManager = new OnEventManager(); // should be on .h (all that does not depends on THIS)
	_modeldataManager = new ModelDataManager(this);
	_componentManager = new ComponentManager(this);
	_simulation = new ModelSimulation(this);
	// 1:1 associations (Traits)
	//Sampler_if* sampler = new Traits<Sampler_if>::Implementation();
	_parser = new TraitsKernel<Parser_if>::Implementation(this, new TraitsKernel<Sampler_if>::Implementation());
	_modelChecker = new TraitsKernel<ModelChecker_if>::Implementation(this);
	_modelPersistence = new TraitsKernel<ModelPersistence_if>::Implementation(this);
	_automaticallyCreatesModelDataDefinitions = TraitsKernel<Model>::automaticallyCreatesModelDatas;
	// 1:n associations
	_futureEvents = new List<Event*>(); /// The future events list must be chronologicaly sorted
	//_events->setSortFunc(&EventCompare); // It works too
	_futureEvents->setSortFunc([](const Event* a, const Event * b) {
		return a->getTime() < b->getTime(); /// Events are sorted chronologically
	});
	// for process analyser
	//_responses = new List<SimulationResponse*>();
	//_controls = new List<SimulationControl*>();
	// insert controls
	//_controls->insert(new SimulationControl("ModelSimulation", "NumberOfReplications",
	//        DefineGetterMember<ModelSimulation>(this->_simulation, &ModelSimulation::getNumberOfReplications),
	//        DefineSetterMember<ModelSimulation>(this->_simulation, &ModelSimulation::setNumberOfReplications)) );
	//_controls->insert(new SimulationControl("ModelSimulation", "ReplicationLength",
	//        DefineGetterMember<ModelSimulation>(this->_simulation, &ModelSimulation::getReplicationLength),
	//        DefineSetterMember<ModelSimulation>(this->_simulation, &ModelSimulation::setReplicationLength)) );
	//_controls->insert(new SimulationControl("ModelSimulation", "WarmupPeriod",
	//        DefineGetterMember<ModelSimulation>(this->_simulation, &ModelSimulation::getWarmUpPeriod),
	//        DefineSetterMember<ModelSimulation>(this->_simulation, &ModelSimulation::setWarmUpPeriod)) );
	// for NEW process analyser
	_responsesNew = new List<PropertyBase*>();
	_controlsNew = new List<PropertyBase*>();
	// insert NEW controls
	_controlsNew->insert(new PropertyT<unsigned int>("ModelSimulation", "NumberOfReplications",
			DefineGetter<ModelSimulation,unsigned int>(this->_simulation, &ModelSimulation::getNumberOfReplications),
			DefineSetter<ModelSimulation,unsigned int>(this->_simulation, &ModelSimulation::setNumberOfReplications)) );
	_controlsNew->insert(new PropertyT<double>("ModelSimulation", "ReplicationLength",
			DefineGetter<ModelSimulation,double>(this->_simulation, &ModelSimulation::getReplicationLength),
			DefineSetter<ModelSimulation,double>(this->_simulation, &ModelSimulation::setReplicationLength)) );
	_controlsNew->insert(new PropertyT<double>("ModelSimulation", "WarmupPeriod",
			DefineGetter<ModelSimulation,double>(this->_simulation, &ModelSimulation::getWarmUpPeriod),
			DefineSetter<ModelSimulation,double>(this->_simulation, &ModelSimulation::setWarmUpPeriod)) );
}

void Model::sendEntityToComponent(Entity* entity, Connection* connection, double timeDelay) {
	this->sendEntityToComponent(entity, connection->component, timeDelay, connection->port);
}

void Model::sendEntityToComponent(Entity* entity, ModelComponent* component, double timeDelay, unsigned int componentinputPortNumber) {
	SimulationEvent* se = _simulation->_createSimulationEvent();
	se->setDestinationComponent(component);
	se->setEntityMoveTimeDelay(timeDelay);
	this->getOnEvents()->NotifyEntityMoveHandlers(se); // it's my friend
	Event* newEvent = new Event(this->getSimulation()->getSimulatedTime() + timeDelay, entity, component, componentinputPortNumber);
	this->getFutureEvents()->insert(newEvent);
}

bool Model::save(std::string filename) {
	bool res = this->_modelPersistence->save(filename);
	if (res) {
		this->_traceManager->trace(Util::TraceLevel::L2_results, "Model successfully saved");
		//@TODO Create a onModelSave event handler
	} else {
		this->_traceManager->trace(Util::TraceLevel::L2_results, "Model could not be saved");

	}
	return res;
}

bool Model::load(std::string filename) {
	this->clear();
	bool res = this->_modelPersistence->load(filename);
	if (res)
		this->_traceManager->trace(Util::TraceLevel::L2_results, "Model successfully loaded");
		//@TODO Create a onModelLoad event handler
	else
		this->_traceManager->trace(Util::TraceLevel::L2_results, "Model could not be loaded");
	return res;
}

double Model::parseExpression(const std::string expression) {
	try {
		return _parser->parse(expression);
	} catch (...) {
		//@TODO Create a onParserError event handler
		return 0.0; // @TODO: HOW SAY THERE WAS AN ERROR?
	}
}

bool Model::checkExpression(const std::string expression, const std::string expressionName, std::string* errorMessage) {
	bool result;
	getTracer()->trace("Checking expression \"" + expression + "\"", Util::TraceLevel::L8_detailed);
	try {
		parseExpression(expression, &result, errorMessage);
	} catch (...) {
		result = false;
	}
	if (!result) {
		std::string msg = "Expression \"" + expression + "\" for '" + expressionName + "' is incorrect. ";
		this->_traceManager->trace(msg, Util::TraceLevel::L3_errorRecover);
		errorMessage->append(msg);
	}
	return result;
}

double Model::parseExpression(const std::string expression, bool* success, std::string* errorMessage) {
	double value = _parser->parse(expression, success, errorMessage);
	return value;
}

void Model::show() {
	getTracer()->trace(Util::TraceLevel::L2_results, "Simulation Model:");
	Util::IncIndent();
	{
		getTracer()->trace(Util::TraceLevel::L2_results, "Information:");
		Util::IncIndent();
		getTracer()->trace(Util::TraceLevel::L2_results, this->getInfos()->show());
		Util::DecIndent();
		_showConnections();
		_showComponents();
		_showElements();
		_showSimulationControls();
		_showSimulationResponses();
	}
	Util::DecIndent();
	getTracer()->trace(Util::TraceLevel::L2_results, "End of Simulation Model");
}

bool Model::insert(ModelDataDefinition* elemOrComp) {
	elemOrComp->setModelLevel(_level);
	ModelComponent* comp = dynamic_cast<ModelComponent*> (elemOrComp);
	if (comp == nullptr) // it's a ModelDataDefinition
		return this->getDataManager()->insert(elemOrComp);
	else // it's a ModelComponent
		return this->getComponents()->insert(comp);
}

void Model::remove(ModelDataDefinition* elemOrComp) {
	ModelComponent* comp = dynamic_cast<ModelComponent*> (elemOrComp);
	if (comp == nullptr) // it's a ModelDataDefinition
		this->getDataManager()->remove(elemOrComp);
	else // it's a ModelComponent
		this->getComponents()->remove(comp);
}

void Model::setSamplerSeed(int seed) {
	TraitsKernel<Sampler_if>::Parameters* param;
	param = static_cast<TraitsKernel<Sampler_if>::Parameters*>(_parser->sampler()->getRNGparameters());
	param->seed = seed;
}

void Model::_showElements() const {
	getTracer()->trace(Util::TraceLevel::L2_results, "Elements:");
	Util::IncIndent();
	{
		std::string elementType;
		ModelDataDefinition* modeldatum;
		std::list<std::string>* elementTypes = getDataManager()->getDataDefinitionClassnames();
		for (std::list<std::string>::iterator typeIt = elementTypes->begin(); typeIt != elementTypes->end(); typeIt++) {
			elementType = (*typeIt);
			List<ModelDataDefinition*>* em = getDataManager()->getDataDefinitionList(elementType);
			getTracer()->trace(Util::TraceLevel::L2_results, elementType + ":");
			Util::IncIndent();
			{
				for (std::list<ModelDataDefinition*>::iterator it = em->list()->begin(); it != em->list()->end(); it++) {
					modeldatum = (*it);
					getTracer()->trace(Util::TraceLevel::L2_results, modeldatum->show());
				}
			}
			Util::DecIndent();
		}
	}
	Util::DecIndent();
}

void Model::_showConnections() const {
	// @TODO
}

void Model::_showComponents() const {
	getTracer()->trace(Util::TraceLevel::L2_results, "Components:");
	Util::IncIndent();
	for (std::list<ModelComponent*>::iterator it = getComponents()->begin(); it != getComponents()->end(); it++) {
		getTracer()->trace(Util::TraceLevel::L2_results, (*it)->show()); ////
	}
	Util::DecIndent();
}

void Model::_showSimulationControls() const {
	getTracer()->trace(Util::TraceLevel::L2_results, "Simulation Controls:");
	Util::IncIndent();
	for (PropertyBase* control : *_controlsNew->list()) {
		getTracer()->trace(Util::TraceLevel::L2_results, control->show()); ////
	}
	Util::DecIndent();
}

void Model::_showSimulationResponses() const {
	getTracer()->trace(Util::TraceLevel::L2_results, "Simulation Responses:");
	Util::IncIndent();
	for (PropertyBase* response: *_responsesNew->list()) {
		getTracer()->trace(Util::TraceLevel::L2_results, response->show()); ////
	}
	Util::DecIndent();
}

void Model::clear() {
	this->_componentManager->clear();
	this->_modeldataManager->clear();
	this->_futureEvents->clear();
	Util::ResetAllIds();
	//this->_simulation->clear();  // @TODO clear method
	//this->_modelInfo->clear(); // @TODO clear method
	//Util::ResetAllIds(); // @TODO: To implement
}

void Model::_createModelInternalElements() {
	getTracer()->trace(Util::TraceLevel::L7_internal, "Creating internal elements");
	Util::IncIndent();

	for (ModelComponent* component : *_componentManager) {
		getTracer()->trace(Util::TraceLevel::L8_detailed, "Internals for " + component->getClassname() + " \"" + component->getName() + "\"");
		Util::IncIndent();
		ModelComponent::CreateInternalData(component);
		Util::DecIndent();
	}

	std::list<ModelDataDefinition*>* modelElements;
	unsigned int originalSize = getDataManager()->getDataDefinitionClassnames()->size(), pos = 1;
	//for (std::list<std::string>::iterator itty = elements()->elementClassnames()->begin(); itty != elements()->elementClassnames()->end(); itty++) {
	std::list<std::string>::iterator itty = getDataManager()->getDataDefinitionClassnames()->begin();
	while (itty != getDataManager()->getDataDefinitionClassnames()->end() && pos <= originalSize) {
		//try {
		modelElements = getDataManager()->getDataDefinitionList((*itty))->list();
		//} catch (const std::exception& e) {
		// @TODO Is there a better solution to iterate over a changing sorted list??
		// ops. Sorted list has changed and iteration fails. Starts iterating again
		//	itty = elements()->elementClassnames()->begin();
		//	modelElements = elements()->elementList((*itty))->list();
		//	tracer()->trace(Util::TraceLevel::L7_internal, "Creating internal elements");
		//}
		for (std::list<ModelDataDefinition*>::iterator itel = modelElements->begin(); itel != modelElements->end(); itel++) {
			getTracer()->trace(Util::TraceLevel::L8_detailed, "Internals for " + (*itel)->getClassname() + " \"" + (*itel)->getName() + "\""); // (" + std::to_string(pos) + "/" + std::to_string(originalSize) + ")");
			Util::IncIndent();
			ModelDataDefinition::CreateInternalData((*itel));
			Util::DecIndent();
		}
		if (originalSize == getDataManager()->getDataDefinitionClassnames()->size()) {
			itty++;
			pos++;
		} else {
			originalSize = getDataManager()->getDataDefinitionClassnames()->size();
			itty = getDataManager()->getDataDefinitionClassnames()->begin();
			pos = 1;
			getTracer()->trace(Util::TraceLevel::L7_internal, "Restarting to create internal elements (due to previous creations)");
		}
	}
	Util::DecIndent();
}

List<PropertyBase *> *Model::getControls() const
{
	return _controlsNew;
}

List<PropertyBase *> *Model::getResponses() const
{
	return _responsesNew;
}

bool Model::check() {
	getTracer()->trace(Util::TraceLevel::L7_internal, "Checking model consistency");
	Util::IncIndent();
	// before checking the model, creates all necessary internal ModelDatas
	_createModelInternalElements();
	bool res = this->_modelChecker->checkAll();
	Util::DecIndent();
	if (res) {
		getTracer()->trace(Util::TraceLevel::L2_results, "End of Model checking: Success");
	} else {
		//std::exception e = new std::exception();
		//getTrace()->traceError() ;
		getTracer()->trace(Util::TraceLevel::L2_results, "End of Model checking: Failed");
	}
	return res;
}

//bool Model::verifySymbol(std::string componentName, std::string expressionName, std::string expression, std::string expressionResult, bool mandatory) {
//    return this->_modelChecker->verifySymbol(componentName, expressionName, expression, expressionResult, mandatory);
//}

Entity* Model::createEntity(std::string name, bool insertIntoModel) {
	// Entity is my FRIEND, therefore Model can access it
	Entity* newEntity = new Entity(this, name, true);
	SimulationEvent *se = _simulation->_createSimulationEvent(); // it's my friend
	se->setEntityCreated(newEntity);
	//getTracer()->traceSimulation(this, /*"Entity " + entId +*/entity->getName() + " was created");
	getOnEvents()->NotifyEntityCreateHandlers(se);
	return newEntity;
}

void Model::removeEntity(Entity* entity) {//, bool collectStatistics) {
	this->_eventManager->NotifyEntityRemoveHandlers(_simulation->_createSimulationEvent()); // it's my friend
	std::string entId = std::to_string(entity->entityNumber());
	this->getDataManager()->remove(Util::TypeOf<Entity>(), entity);
	getTracer()->traceSimulation(this, /*"Entity " + entId +*/entity->getName() + " was removed from the system");
	entity->~Entity();
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

ModelPersistence_if* Model::getPersistence() const {
	return _modelPersistence;
}

void Model::setAutomaticallyCreatesModelDataDefinitions(bool _automaticallyCreatesModelDataDefinitions) {
	this->_automaticallyCreatesModelDataDefinitions = _automaticallyCreatesModelDataDefinitions;
}

bool Model::isAutomaticallyCreatesModelDataDefinitions() const {
	return _automaticallyCreatesModelDataDefinitions;
}

unsigned int Model::getLevel() const {
	return _level;
}

bool Model::hasChanged() const {
	bool changed = _hasChanged;
	changed &= this->_componentManager->hasChanged();
	changed &= this->_modeldataManager->hasChanged();
	changed &= this->_modelInfo->hasChanged();
	changed &= this->_modelPersistence->hasChanged();
	return changed;
}

ComponentManager * Model::getComponents() const {
	return _componentManager;
}

//List<SimulationControl*>* Model::getControls() const {
//    return _controls;
//}

//List<SimulationResponse*>* Model::getResponses() const {
//    return _responses;
//}

OnEventManager * Model::getOnEvents() const {
	return _eventManager;
}

ModelDataManager * Model::getDataManager() const {
	return _modeldataManager;
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

