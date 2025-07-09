/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Delay.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 19:49
 */

#include "Delay.h"
#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/Attribute.h"
#include "../../kernel/simulator/SimulationControlAndResponse.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Delay::GetPluginInformation;
}
#endif

ModelDataDefinition* Delay::NewInstance(Model* model, std::string name) {
	return new Delay(model, name);
}

void Delay::setAllocation(Util::AllocationType allocation) {
	_allocation = allocation;
}

Util::AllocationType Delay::getAllocation() const {
	return _allocation;
}

Delay::Delay(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Delay>(), name) {
	SimulationControlGeneric<std::string>* propExpression = new SimulationControlGeneric<std::string>(
									std::bind(&Delay::delayExpression, this), std::bind(&Delay::setDelayExpression, this, std::placeholders::_1, Util::TimeUnit::unknown),
									Util::TypeOf<Delay>(), getName(), "DelayExpression", "");
	SimulationControlGeneric<double>* propDelay = new SimulationControlGeneric<double>(
									std::bind(&Delay::delay, this), std::bind(&Delay::setDelay, this, std::placeholders::_1),
									Util::TypeOf<Delay>(), getName(), "Delay", "");
    SimulationControlGenericEnum<Util::TimeUnit, Util>* propUnitTime = new SimulationControlGenericEnum<Util::TimeUnit, Util>(
									std::bind(&Delay::delayTimeUnit, this),	std::bind(&Delay::setDelayTimeUnit, this, std::placeholders::_1),
									Util::TypeOf<Delay>(), getName(), "DelayTimeUnit", "");
    SimulationControlGenericEnum<Util::AllocationType, Util>* propAlloc = new SimulationControlGenericEnum<Util::AllocationType, Util>(
                                    std::bind(&Delay::getAllocation, this), std::bind(&Delay::setAllocation,  this, std::placeholders::_1),
                                    Util::TypeOf<Delay>(), getName(), "AllocationType", "");

	_parentModel->getControls()->insert(propExpression);
	_parentModel->getControls()->insert(propDelay);
	_parentModel->getControls()->insert(propUnitTime);
    _parentModel->getControls()->insert(propAlloc);

	// setting properties
	_addProperty(propExpression);
	_addProperty(propDelay);
	_addProperty(propUnitTime);
    _addProperty(propAlloc);
}

void Delay::setDelay(double delay) {
	_delayExpression = std::to_string(delay);
}

double Delay::delay() const {
	return _parentModel->parseExpression(_delayExpression);
}

std::string Delay::show() {
	return ModelComponent::show() +
			",delayExpression=" + this->_delayExpression +
			",timeUnit=" + std::to_string(static_cast<int> (this->_delayTimeUnit));
}

//void Delay::setDelayExpression(std::string _delayExpression) {
//	this->_delayExpression = _delayExpression;
//}

void Delay::setDelayExpression(std::string _delayExpression, Util::TimeUnit _delayTimeUnit) {
	this->_delayExpression = _delayExpression;
	if (_delayTimeUnit != Util::TimeUnit::unknown) {
		this->_delayTimeUnit = _delayTimeUnit;
	}
}

std::string Delay::delayExpression() const {
	return _delayExpression;
}

void Delay::setDelayTimeUnit(Util::TimeUnit _delayTimeUnit) {
	this->_delayTimeUnit = _delayTimeUnit;
}

Util::TimeUnit Delay::delayTimeUnit() const {
	return _delayTimeUnit;
}

void Delay::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	double waitTime = _parentModel->parseExpression(_delayExpression);
	Util::TimeUnit stu = _parentModel->getSimulation()->getReplicationBaseTimeUnit(); //getReplicationLengthTimeUnit();
	waitTime *= Util::TimeUnitConvert(_delayTimeUnit, stu);
	if (_reportStatistics) {
		std::string allocationCategory = Util::StrAllocation(_allocation);
		//try { //@TODO: What the hell????!!!
			_cstatWaitTime->getStatistics()->getCollector()->addValue(waitTime);
		//} catch (const std::exception& e) {
		//	traceError(e.what());
		//}
		if (entity->getEntityType()->isReportStatistics())
			entity->getEntityType()->addGetStatisticsCollector(entity->getEntityTypeName() + "." + allocationCategory+ "Time")->getStatistics()->getCollector()->addValue(waitTime);
		double totalWaitTime = entity->getAttributeValue("Entity.Total" + allocationCategory + "Time");
		std::string attribIndex="";
		entity->setAttributeValue("Entity.Total" + allocationCategory + "Time", totalWaitTime + waitTime, attribIndex, true);
	}
	double delayEndTime = _parentModel->getSimulation()->getSimulatedTime() + waitTime;
	Event* newEvent = new Event(delayEndTime, entity, this->getConnectionManager()->getFrontConnection());
	_parentModel->getFutureEvents()->insert(newEvent);
	traceSimulation(this, "End of delay of "/*entity " + std::to_string(entity->entityNumber())*/ + entity->getName() + " scheduled to time " + std::to_string(delayEndTime) + Util::StrTimeUnitShort(stu) + " (wait time " + std::to_string(waitTime) + Util::StrTimeUnitShort(stu) + ") // " + _delayExpression+ " "+Util::StrTimeUnitShort(_delayTimeUnit));
}

ModelComponent* Delay::LoadInstance(Model* model, PersistenceRecord *fields) {
	Delay* newComponent = new Delay(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

bool Delay::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_delayExpression = fields->loadField("delayExpression", DEFAULT.delayExpression);
		_delayTimeUnit = fields->loadField("delayExpressionTimeUnit", DEFAULT.delayTimeUnit);
		_allocation = static_cast<Util::AllocationType> (fields->loadField("allocation", static_cast<int> (DEFAULT.allocation)));
	}
	return res;
}

void Delay::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("delayExpression", this->_delayExpression, DEFAULT.delayExpression, saveDefaultValues);
	fields->saveField("delayExpressionTimeUnit", _delayTimeUnit, DEFAULT.delayTimeUnit, saveDefaultValues);
	fields->saveField("allocation", static_cast<int> (_allocation), static_cast<int> (DEFAULT.allocation), saveDefaultValues);
}

bool Delay::_check(std::string* errorMessage) {
	return _parentModel->checkExpression(_delayExpression, "Delay expression", errorMessage);
}

void Delay::_createInternalAndAttachedData() {
	if (_reportStatistics && _cstatWaitTime == nullptr) {
		_attachedAttributesInsert({"Entity.Total" + Util::StrAllocation(_allocation)+"Time"});
		_cstatWaitTime = new StatisticsCollector(_parentModel, getName() + "." + "DelayTime", this);
		_internalDataInsert("DelayTime", _cstatWaitTime);
		// include StatisticsCollector needed in EntityType
		//ModelDataManager* elements = _parentModel->getDataManager();
		//std::list<ModelDataDefinition*>* enttypes = elements->getDataDefinitionList(Util::TypeOf<EntityType>())->list();
		//for (ModelDataDefinition* modeldatum : *enttypes) {
		//	EntityType* enttype = static_cast<EntityType*> (modeldatum);
		//	if (modeldatum->isReportStatistics())
		//		enttype->addGetStatisticsCollector(enttype->getName() + ".DelayTime");
		//}
	} else {
		_internalDataClear();
		// @TODO remove StatisticsCollector needed in EntityType
	}
}

PluginInformation* Delay::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Delay>(), &Delay::LoadInstance, &Delay::NewInstance);
	std::string text = "The Delay module delays an entity by a specified amount of time.";
	text += " When an entity arrives at a Delay module, the time delay expression is evaluated and the entity remains in the module for the resulting time.";
	text += " The time is then allocated to the entity’s value-added, non-value added, transfer, wait, or other time.";
	text += " Associated costs are calculated and allocated as well.";
	text += " TYPICAL USES: (1) Processing a check at a bank; (2) Performing a setup on a machine; (3) Transferring a document to another department";
	info->setDescriptionHelp(text);
	return info;
}
