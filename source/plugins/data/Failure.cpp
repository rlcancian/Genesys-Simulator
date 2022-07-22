/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Failure.cpp
 * Author: rlcancian
 *
 * Created on 20 de Failureembro de 2019, 20:07
 */

#include "Failure.h"
#include "../../kernel/simulator/Model.h"
#include "../data/Resource.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Failure::GetPluginInformation;
}
#endif

ModelDataDefinition* Failure::NewInstance(Model* model, std::string name) {
	return new Failure(model, name);
}

Failure::Failure(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<Failure>(), name) {
}

std::string Failure::show() {
	return ModelDataDefinition::show() +
			"";
}

void Failure::checkIsGoingToFailByCount(Resource* resource) {
	std::map<Resource*, unsigned int>::iterator it;
	it = _releaseCounts->find(resource);
	if (it == _releaseCounts->end()) {
		_releaseCounts->insert({resource, 0});
		it = _releaseCounts->find(resource);
	}
	it->second++;
	unsigned int countReleased = it->second;
	double countLimit = _parentModel->parseExpression(_countExpression);
	if (countReleased >= countLimit) { // is going to fail
		resource->_fail();
		it->second = 0;
		_scheduleActivation(it->first);
	}
}

void Failure::_scheduleActivation(Resource* resource) {
	double time = _parentModel->parseExpression(_downTimeExpression);
	double timeScale = Util::TimeUnitConvert(_downTimeTimeUnit, _parentModel->getSimulation()->getReplicationBaseTimeUnit());
	time *=timeScale;
	time += _parentModel->getSimulation()->getSimulatedTime();
	InternalEvent* intEvent = new InternalEvent(time, "Resource Activated");//, res);
	intEvent->setEventHandler<Failure>(this, &Failure::_onFailureActiveEventHandler, resource);
	_parentModel->getFutureEvents()->insert(intEvent);
	_parentModel->getTracer()->traceSimulation(this,"Activation of Resource \""+resource->getName()+"\" schedule to "+std::to_string(time), TraceManager::Level::L8_detailed);
}

void Failure::setFailureType(FailureType _failureType) {
	this->_failureType = _failureType;
}

Failure::FailureType Failure::getFailureType() const {
	return _failureType;
}

void Failure::setCountExpression(std::string countExpression) {
	_countExpression = countExpression;
}

std::string Failure::getCountExpression() const {
	_countExpression;
}

void Failure::setDownTimeTimeUnit(Util::TimeUnit downTimeTimeUnit) {
	_downTimeTimeUnit = downTimeTimeUnit;
}

Util::TimeUnit Failure::getDownTimeTimeUnit() const {
	_downTimeTimeUnit;
}

void Failure::setDownTimeExpression(std::string downTimeExpression) {
	_downTimeExpression = downTimeExpression;
}

std::string Failure::getDownTimeExpression() const {
	return _downTimeExpression;
}

void Failure::setUpTimeTimeUnit(Util::TimeUnit upTimeTimeUnit) {
	_upTimeTimeUnit = upTimeTimeUnit;
}

Util::TimeUnit Failure::getUpTimeTimeUnit() const {
	return _upTimeTimeUnit;
}

void Failure::setUpTimeExpression(std::string upTimeExpression) {
	_upTimeExpression = upTimeExpression;
}

std::string Failure::getUpTimeExpression() const {
	return _upTimeExpression;
}

void Failure::setFailureRule(FailureRule _failureRule) {
	this->_failureRule = _failureRule;
}

Failure::FailureRule Failure::getFailureRule() const {
	return _failureRule;
}

PluginInformation* Failure::GetPluginInformation() {
	//@TODO not implemented yet
	PluginInformation* info = new PluginInformation(Util::TypeOf<Failure>(), &Failure::LoadInstance, &Failure::NewInstance);
	//info->insertDynamicLibFileDependence("resource.so"); -- Circular dependence!! Do not add it
	return info;
}

ModelDataDefinition* Failure::LoadInstance(Model* model, PersistenceRecord *fields) {
	Failure* newElement = new Failure(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

bool Failure::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		try {
			//@TODO not implemented yet
		} catch (...) {
		}
	}
	return res;
}

void Failure::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	//@TODO not implemented yet
}

bool Failure::_check(std::string* errorMessage) {
	bool resultAll = true;
	//@TODO not implemented yet
	// resultAll |= ...
	*errorMessage += "";
	return resultAll;
}

void Failure::_initBetweenReplications() {
	_releaseCounts->clear(); //TODO: really needed?
	if (_failureType==FailureType::TIME) {
		double time, timeScale;
		for (Resource* res: *_falingResources->list()) {
			time = _parentModel->parseExpression(_upTimeExpression);
			timeScale = Util::TimeUnitConvert(_upTimeTimeUnit, _parentModel->getSimulation()->getReplicationBaseTimeUnit());
			time *=timeScale;
			InternalEvent* intEvent = new InternalEvent(time, "Resource Failed");//, res);
			intEvent->setEventHandler<Failure>(this, &Failure::_onFailureFailEventHandler, res);
			_parentModel->getFutureEvents()->insert(intEvent);
		}
	}
}

List<Resource*>*Failure::falingResources() const{
	return _falingResources;
}

// private (internal!!) simulation event handlers

void Failure::_onFailureActiveEventHandler(void* resourcePtr){
	Resource* resource = static_cast<Resource*>(resourcePtr);
	resource->_active();
	if (_failureType==FailureType::TIME) {
		// schedule next resource fail
		double time = _parentModel->parseExpression(_upTimeExpression);
		double timeScale = Util::TimeUnitConvert(_upTimeTimeUnit, _parentModel->getSimulation()->getReplicationBaseTimeUnit());
		time *=timeScale;
		time += _parentModel->getSimulation()->getSimulatedTime();
		InternalEvent* intEvent = new InternalEvent(time, "Resource Failed");//, res);
		intEvent->setEventHandler<Failure>(this, &Failure::_onFailureFailEventHandler, resource);
		_parentModel->getFutureEvents()->insert(intEvent);
	}
}

void Failure::_onFailureFailEventHandler(void* resourcePtr){
	Resource* resource = static_cast<Resource*>(resourcePtr);
	resource->_fail();
	// schedule next resource activation
	_scheduleActivation(resource);
}


