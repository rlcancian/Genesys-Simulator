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

std::string Failure::convertEnumToStr(FailureType type) {
	switch (static_cast<int> (type)) {
		case 0: return "COUNT";
		case 1: return "TIME";
	}
	return "Unknown";
}

std::string Failure::convertEnumToStr(FailureRule rule) {
	switch (static_cast<int> (rule)) {
		case 0: return "IGNORE";
		case 1: return "PREEMPT";
		case 2: return "WAIT";
	}
	return "Unknown";
}

Failure::Failure(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<Failure>(), name) {
	SimulationControlGenericEnum<Failure::FailureType, Failure>* propFailureType = new SimulationControlGenericEnum<Failure::FailureType, Failure>(
                                    std::bind(&Failure::getFailureType, this), std::bind(&Failure::setFailureType, this, std::placeholders::_1),
                                    Util::TypeOf<Failure>(), getName(), "FailureType", "");
	SimulationControlGeneric<std::string>* propCountExpression = new SimulationControlGeneric<std::string>(
									std::bind(&Failure::getCountExpression, this), std::bind(&Failure::setCountExpression, this, std::placeholders::_1),
									Util::TypeOf<Failure>(), getName(), "CountExpression", "");															
	SimulationControlGenericEnum<Util::TimeUnit, Util>* propDownUnitTime = new SimulationControlGenericEnum<Util::TimeUnit, Util>(
									std::bind(&Failure::getDownTimeTimeUnit, this),	std::bind(&Failure::setDownTimeTimeUnit, this, std::placeholders::_1),
									Util::TypeOf<Failure>(), getName(), "DownTimeTimeUnit", "");
	SimulationControlGeneric<std::string>* propDownTimeExpression = new SimulationControlGeneric<std::string>(
									std::bind(&Failure::getDownTimeExpression, this), std::bind(&Failure::setDownTimeExpression, this, std::placeholders::_1),
									Util::TypeOf<Failure>(), getName(), "DownTimeExpression", "");
	SimulationControlGenericEnum<Util::TimeUnit, Util>* propUpUnitTime = new SimulationControlGenericEnum<Util::TimeUnit, Util>(
									std::bind(&Failure::getUpTimeTimeUnit, this),	std::bind(&Failure::setUpTimeTimeUnit, this, std::placeholders::_1),
									Util::TypeOf<Failure>(), getName(), "UpTimeTimeUnit", "");
	SimulationControlGeneric<std::string>* propUpTimeExpression = new SimulationControlGeneric<std::string>(
									std::bind(&Failure::getUpTimeExpression, this), std::bind(&Failure::setUpTimeExpression, this, std::placeholders::_1),
									Util::TypeOf<Failure>(), getName(), "UpTimeExpression", "");
	SimulationControlGenericEnum<Failure::FailureRule, Failure>* propFailureRule = new SimulationControlGenericEnum<Failure::FailureRule, Failure>(
                                    std::bind(&Failure::getFailureRule, this), std::bind(&Failure::setFailureRule, this, std::placeholders::_1),
                                    Util::TypeOf<Failure>(), getName(), "FailureType", "");
	SimulationControlGenericListPointer<Resource*, Model*, Resource>* propFalingResources = new SimulationControlGenericListPointer<Resource*, Model*, Resource>(
									_parentModel,
									std::bind(&Failure::falingResources, this), std::bind(&Failure::addResource, this, std::placeholders::_1), std::bind(&Failure::removeResource, this, std::placeholders::_1),
									Util::TypeOf<Failure>(), getName(), "FalingResources", "");
	
	_parentModel->getControls()->insert(propFailureType);
	_parentModel->getControls()->insert(propCountExpression);
	_parentModel->getControls()->insert(propDownUnitTime);
	_parentModel->getControls()->insert(propDownTimeExpression);
	_parentModel->getControls()->insert(propUpUnitTime);
	_parentModel->getControls()->insert(propUpTimeExpression);
	_parentModel->getControls()->insert(propFailureRule);
	_parentModel->getControls()->insert(propFalingResources);

	// setting properties
	_addProperty(propFailureType);
	_addProperty(propCountExpression);
	_addProperty(propDownUnitTime);
	_addProperty(propDownTimeExpression);
	_addProperty(propUpUnitTime);
	_addProperty(propUpTimeExpression);
	_addProperty(propFailureRule);
	_addProperty(propFalingResources);
}

Failure::~Failure() {
	if (_falingResources != nullptr) {
		while (!_falingResources->empty()) {
			Resource* resource = _falingResources->front();
			if (resource != nullptr) {
				resource->removeFailure(this);
			} else {
				_falingResources->pop_front();
			}
		}
		delete _falingResources;
		_falingResources = nullptr;
	}
	delete _releaseCounts;
	_releaseCounts = nullptr;
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
	traceSimulation(this,"Activation of Resource \""+resource->getName()+"\" schedule to "+std::to_string(time), TraceManager::Level::L8_detailed);
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
	// TODO(codex|kernel-review|2026-03-30): Confirm if expression needs normalization/validation before returning.
	return _countExpression;
}

void Failure::setDownTimeTimeUnit(Util::TimeUnit downTimeTimeUnit) {
	_downTimeTimeUnit = downTimeTimeUnit;
}

Util::TimeUnit Failure::getDownTimeTimeUnit() const {
	// TODO(codex|kernel-review|2026-03-30): Revisit semantic contract for down-time unit defaults/validation.
	return _downTimeTimeUnit;
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
	PluginInformation* info = new PluginInformation(Util::TypeOf<Failure>(), &Failure::LoadInstance, &Failure::NewInstance);
	//info->insertDynamicLibFileDependence("resource.so"); -- Circular dependence!! Do not add it
	info->setDescriptionHelp("Defines failure behavior (time-based or count-based) that can be attached to one or more Resources.");
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
			/*!
			 * \brief Load Failure persistence fields.
			 *
			 * This method mirrors \ref _saveInstance field names to preserve a
			 * deterministic serialization contract.
			 */
			_failureType = static_cast<FailureType>(fields->loadField("failureType", static_cast<int>(DEFAULT.failureType)));
			_failureRule = static_cast<FailureRule>(fields->loadField("failureRule", static_cast<int>(DEFAULT.failureRule)));
			_countExpression = fields->loadField("countExpression", DEFAULT.countExpression);
			_upTimeExpression = fields->loadField("upTimeExpression", DEFAULT.upTimeExpression);
			_upTimeTimeUnit = static_cast<Util::TimeUnit>(fields->loadField("upTimeTimeUnit", static_cast<int>(DEFAULT.upTimeTimeUnit)));
			_downTimeExpression = fields->loadField("downTimeExpression", DEFAULT.downTimeExpression);
			_downTimeTimeUnit = static_cast<Util::TimeUnit>(fields->loadField("downTimeTimeUnit", static_cast<int>(DEFAULT.downTimeTimeUnit)));
			while (!_falingResources->empty()) {
				Resource* resource = _falingResources->front();
				if (resource != nullptr) {
					resource->removeFailure(this);
				} else {
					_falingResources->pop_front();
				}
			}
			unsigned int failingResourcesSize = fields->loadField("failingResources", 0u);
			for (unsigned int i = 0; i < failingResourcesSize; i++) {
				std::string resourceName = fields->loadField("failingResource" + Util::StrIndex(i), std::string(""));
				Resource* resource = static_cast<Resource*> (_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), resourceName));
				if (resource != nullptr) {
					addResource(resource);
				}
			}
		} catch (...) {
		}
	}
	return res;
}

void Failure::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	/*!
	 * \brief Persist Failure configuration fields.
	 */
	fields->saveField("failureType", static_cast<int>(_failureType), static_cast<int>(DEFAULT.failureType), saveDefaultValues);
	fields->saveField("failureRule", static_cast<int>(_failureRule), static_cast<int>(DEFAULT.failureRule), saveDefaultValues);
	fields->saveField("countExpression", _countExpression, DEFAULT.countExpression, saveDefaultValues);
	fields->saveField("upTimeExpression", _upTimeExpression, DEFAULT.upTimeExpression, saveDefaultValues);
	fields->saveField("upTimeTimeUnit", static_cast<int>(_upTimeTimeUnit), static_cast<int>(DEFAULT.upTimeTimeUnit), saveDefaultValues);
	fields->saveField("downTimeExpression", _downTimeExpression, DEFAULT.downTimeExpression, saveDefaultValues);
	fields->saveField("downTimeTimeUnit", static_cast<int>(_downTimeTimeUnit), static_cast<int>(DEFAULT.downTimeTimeUnit), saveDefaultValues);
	fields->saveField("failingResources", _falingResources->size(), 0u, saveDefaultValues);
	unsigned int i = 0;
	for (Resource* resource : *_falingResources->list()) {
		fields->saveField("failingResource" + Util::StrIndex(i), resource != nullptr ? resource->getName() : std::string(""), std::string(""), saveDefaultValues);
		i++;
	}
}

bool Failure::_check(std::string& errorMessage) {
	/*!
	 * \brief Validate failure expressions according to selected failure mode.
	 */
	bool resultAll = true;
	if (_failureType == FailureType::COUNT) {
		resultAll &= _parentModel->checkExpression(_countExpression, getName() + ".CountExpression", errorMessage);
	}
	resultAll &= _parentModel->checkExpression(_downTimeExpression, getName() + ".DownTimeExpression", errorMessage);
	if (_failureType == FailureType::TIME) {
		resultAll &= _parentModel->checkExpression(_upTimeExpression, getName() + ".UpTimeExpression", errorMessage);
	}
	// Optional semantic checks:
	// resultAll &= (_falingResources->size() > 0);
	return resultAll;
}

void Failure::_initBetweenReplications() {
	_releaseCounts->clear(); //@TODO: really needed?
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

void Failure::addResource(Resource* newResource){
	if (newResource == nullptr) {
		return;
	}
	if (_falingResources->find(newResource) == _falingResources->list()->end()) {
		newResource->insertFailure(this);
	}
}

void Failure::removeResource(Resource* resource){
	if (resource == nullptr) {
		return;
	}
	if (_falingResources->find(resource) != _falingResources->list()->end()) {
		resource->removeFailure(this);
	}
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
