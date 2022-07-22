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

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Delay::GetPluginInformation;
}
#endif

ModelDataDefinition* Delay::NewInstance(Model* model, std::string name) {
	return new Delay(model, name);
}

Delay::Delay(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Delay>(), name) {
	PropertyT<std::string>* prop1 = new PropertyT<std::string>(Util::TypeOf<Delay>(), "Delay Expression",
			DefineGetter<Delay, std::string>(this, &Delay::delayExpression),
			DefineSetter<Delay, std::string>(this, &Delay::setDelayExpression));
	model->getControls()->insert(prop1);
	_addProperty(prop1);
	PropertyT<Util::TimeUnit>* prop2 = new PropertyT<Util::TimeUnit>(Util::TypeOf<Delay>(), "Delay Time Unit",
			DefineGetter<Delay, Util::TimeUnit>(this, &Delay::delayTimeUnit),
			DefineSetter<Delay, Util::TimeUnit>(this, &Delay::setDelayTimeUnit));
	model->getControls()->insert(prop2);
	_addProperty(prop2);
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

void Delay::setDelayExpression(std::string _delayExpression) {
	this->_delayExpression = _delayExpression;
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
		_cstatWaitTime->getStatistics()->getCollector()->addValue(waitTime);
		if (entity->getEntityType()->isReportStatistics())
			entity->getEntityType()->addGetStatisticsCollector(entity->getEntityTypeName() + ".WaitTime")->getStatistics()->getCollector()->addValue(waitTime);
	}
	double totalWaitTime = entity->getAttributeValue("Entity.TotalWaitTime");
	entity->setAttributeValue("Entity.TotalWaitTime", totalWaitTime + waitTime);
	double delayEndTime = _parentModel->getSimulation()->getSimulatedTime() + waitTime;
	Event* newEvent = new Event(delayEndTime, entity, this->getConnections()->getFrontConnection());
	_parentModel->getFutureEvents()->insert(newEvent);
	_parentModel->getTracer()->traceSimulation(this, "End of delay of "/*entity " + std::to_string(entity->entityNumber())*/ + entity->getName() + " scheduled to time " + std::to_string(delayEndTime) + Util::StrTimeUnitShort(stu) + " (wait time " + std::to_string(waitTime) + Util::StrTimeUnitShort(stu) + ") // " + _delayExpression);
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
		this->_delayExpression = fields->loadField("delayExpression", DEFAULT.delayExpression);
		this->_delayTimeUnit = fields->loadField("delayExpressionTimeUnit", DEFAULT.delayTimeUnit);
	}
	return res;
}

void Delay::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("delayExpression", this->_delayExpression, DEFAULT.delayExpression, saveDefaultValues);
	fields->saveField("delayExpressionTimeUnit", _delayTimeUnit, DEFAULT.delayTimeUnit, saveDefaultValues);
}

bool Delay::_check(std::string* errorMessage) {
	_attachedAttributesInsert({"Entity.TotalWaitTime"});
	return _parentModel->checkExpression(_delayExpression, "Delay expression", errorMessage);
}

void Delay::_createInternalAndAttachedData() {
	if (_reportStatistics && _cstatWaitTime == nullptr) {
		_cstatWaitTime = new StatisticsCollector(_parentModel, getName() + "." + "WaitTime", this);
		_internalDataInsert("WaitTime", _cstatWaitTime);
		// include StatisticsCollector needed in EntityType
		ModelDataManager* elements = _parentModel->getDataManager();
		std::list<ModelDataDefinition*>* enttypes = elements->getDataDefinitionList(Util::TypeOf<EntityType>())->list();
		for (ModelDataDefinition* modeldatum : *enttypes) {
			EntityType* enttype = static_cast<EntityType*> (modeldatum);
			if (modeldatum->isReportStatistics())
				enttype->addGetStatisticsCollector(enttype->getName() + ".WaitTime"); // force create this CStat before simulation starts
		}
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
