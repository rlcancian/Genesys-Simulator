/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   SourceModelCOmponent.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 19:50
 */

#include "SourceModelComponent.h"
#include "Model.h"
#include "PropertyGenesys.h"

//using namespace GenesysKernel;

SourceModelComponent::SourceModelComponent(Model* model, std::string componentTypename, std::string name) : ModelComponent(model, componentTypename, name) {
	std::string className = this->getClassname();

	SimulationControlGeneric<double>* propFirstCreation = new SimulationControlGeneric<double>(
				std::bind(&SourceModelComponent::getFirstCreation, this),
				std::bind(&SourceModelComponent::setFirstCreation, this, std::placeholders::_1),
				className, name, "FirstCreation", "The instant when the first entity arrives");
	SimulationControlGeneric<unsigned int>* propEntitiesPerCreation = new SimulationControlGeneric<unsigned int>(
				std::bind(&SourceModelComponent::getEntitiesPerCreation, this),
				std::bind(&SourceModelComponent::setEntitiesPerCreation, this, std::placeholders::_1),
				className, name, "EntitiesPerCreation", "The amount of entities to be created on each arrival");
	// SimulationControlString* propMaxCreation = new SimulationControlString(
	// 			std::bind(&SourceModelComponent::getMaxCreations, this),
	// 			std::bind(&SourceModelComponent::setMaxCreations, this, std::placeholders::_1),
	// 			className, name, "MaxCreations", "");
	SimulationControlGeneric<std::string>* propTimeBetweenCreations = new SimulationControlGeneric<std::string>(
				std::bind(&SourceModelComponent::getTimeBetweenCreationsExpression, this),
				std::bind(&SourceModelComponent::setTimeBetweenCreationsExpression, this, std::placeholders::_1, Util::TimeUnit::unknown),
				className, name, "TimeBetweenArrivals", "Expression that defines the interval between two consecutive arrivals");
    SimulationControlGenericEnum<Util::TimeUnit, Util>* propTimeUnit = new SimulationControlGenericEnum<Util::TimeUnit, Util>(
				std::bind(&SourceModelComponent::getTimeUnit, this),
				std::bind(&SourceModelComponent::setTimeUnit, this, std::placeholders::_1),
				className, name, "TimeUnit", "The time unit of time between arrivals");
	SimulationControlGenericClass<EntityType*, Model*, EntityType>* propEntityType = new SimulationControlGenericClass<EntityType*, Model*, EntityType>(
				_parentModel,
				std::bind(&SourceModelComponent::getEntityType, this),
				std::bind(&SourceModelComponent::setEntityType, this, std::placeholders::_1),
				className, name, "EntityType", "The entity type created by this source component");


	_parentModel->getControls()->insert(propFirstCreation);
	_parentModel->getControls()->insert(propEntitiesPerCreation);
	// _parentModel->getControls()->insert(propMaxCreation);
	_parentModel->getControls()->insert(propTimeBetweenCreations);
	_parentModel->getControls()->insert(propTimeUnit);
	_parentModel->getControls()->insert(propEntityType);

	// setting properties
	_addProperty(propFirstCreation);
	_addProperty(propEntitiesPerCreation);
	// _addProperty(propMaxCreation);
	_addProperty(propTimeBetweenCreations);
	_addProperty(propTimeUnit);
	_addProperty(propEntityType);
}

std::string SourceModelComponent::show() {
	std::string text = ModelComponent::show();
	if (_entityType != nullptr)
		text += ",entityType=\"" + _entityType->getName() + "\"";
	else
		text += ",entityType=NULL";
	text += ",firstCreation=" + std::to_string(_firstCreation);
	return text;
}

bool SourceModelComponent::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		this->_entitiesPerCreation = fields->loadField("entitiesPerCreation", DEFAULT.entitiesPerCreation);
		this->_firstCreation = fields->loadField("firstCreation", DEFAULT.firstCreation);
		this->_timeBetweenCreationsExpression = fields->loadField("timeBetweenCreations", DEFAULT.timeBetweenCreationsExpression);
		this->_timeBetweenCreationsTimeUnit = fields->loadField("timeBetweenCreationsTimeUnit", DEFAULT.timeBetweenCreationsTimeUnit);
		this->_maxCreationsExpression = fields->loadField("maxCreations", DEFAULT.maxCreationsExpression);
		std::string entityTypename = fields->loadField("entityType", DEFAULT.entityTypename);
		ModelDataDefinition* enttype = (_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<EntityType>(), entityTypename));
		if (enttype != nullptr) {
			this->_entityType = dynamic_cast<EntityType*> (enttype);
		} else {
			this->_entityType = nullptr;
		}
	}
	return res;
}

void SourceModelComponent::_initBetweenReplications() {
	for (unsigned int i=0; i<this->_entitiesPerCreation; i++) {
		Entity* newEntity = _parentModel->createEntity(_entityType->getName() + "_%", false);
		newEntity->setEntityType(_entityType);
		Event* newEvent = new Event(_firstCreation, newEntity, this);
		_parentModel->getFutureEvents()->insert(newEvent);
	}
	_entitiesCreatedSoFar = _entitiesPerCreation;
}

void SourceModelComponent::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("entitiesPerCreation", _entitiesPerCreation, DEFAULT.entitiesPerCreation, saveDefaultValues);
	fields->saveField("firstCreation", _firstCreation, DEFAULT.firstCreation, saveDefaultValues);
	fields->saveField("timeBetweenCreations", _timeBetweenCreationsExpression, DEFAULT.timeBetweenCreationsExpression, saveDefaultValues);
	fields->saveField("timeBetweenCreationsTimeUnit", _timeBetweenCreationsTimeUnit, DEFAULT.timeBetweenCreationsTimeUnit, saveDefaultValues);
	fields->saveField("maxCreations", _maxCreationsExpression, DEFAULT.maxCreationsExpression, saveDefaultValues);
	if (_entityType != nullptr) {
		fields->saveField("entityType", _entityType->getName());
	} else {
		fields->saveField("entityType", DEFAULT.entityTypename);
	}
}

bool SourceModelComponent::_check(std::string& errorMessage) {
	bool resultAll = true;
    resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<EntityType>(), _entityType, "entitytype", errorMessage);
    resultAll &= _parentModel->checkExpression(this->_timeBetweenCreationsExpression, "time between creations", errorMessage);
	return resultAll;
}

void SourceModelComponent::_createInternalAndAttachedData() {
	_attachedAttributesInsert({"Entity.ArrivalTime", "Entity.Type"});
	if (this->_entityType == nullptr) {
		std::string defaulName = DEFAULT.entityTypename;
		for (ModelDataDefinition* elem : *_parentModel->getDataManager()->getDataDefinitionList(Util::TypeOf<EntityType>())->list()) {
			if (elem->getName() == defaulName) { // there is an entitytype wich the same default name.
				_entityType = static_cast<EntityType*>(elem);
				break;
			}
		}
		if (_entityType == nullptr) {
			_entityType = new EntityType(_parentModel, DEFAULT.entityTypename);
		}
	}
	if (_entityType != nullptr) {
		_attachedDataInsert("EntityType", _entityType);
	}
	else {
		_attachedDataRemove("EntityType");
	}
}

void SourceModelComponent::setFirstCreation(double _firstCreation) {
	this->_firstCreation = _firstCreation;
}

double SourceModelComponent::getFirstCreation() const {
	return _firstCreation;
}

void SourceModelComponent::setEntityType(EntityType* entityType) {
	_entityType = entityType;
}

void SourceModelComponent::setEntityTypeName(std::string entityTypeName) {
	ModelDataDefinition* data = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<EntityType>(), entityTypeName);
	if (data != nullptr) {
		_entityType = dynamic_cast<EntityType*> (data);
	} else {
		_entityType = new EntityType(_parentModel, entityTypeName);
	}
}

EntityType* SourceModelComponent::getEntityType() const {
	return _entityType;
}

void SourceModelComponent::setTimeUnit(Util::TimeUnit _timeUnit) {
	this->_timeBetweenCreationsTimeUnit = _timeUnit;
}

Util::TimeUnit SourceModelComponent::getTimeUnit() const {
	return this->_timeBetweenCreationsTimeUnit;
}

//void SourceModelComponent::setTimeBetweenCreationsExpression(std::string _timeBetweenCreations) {
//	this->_timeBetweenCreationsExpression = _timeBetweenCreations;
//}

void SourceModelComponent::setTimeBetweenCreationsExpression(std::string _timeBetweenCreations, Util::TimeUnit _timeUnit) {
	this->_timeBetweenCreationsExpression = _timeBetweenCreations;
	if (_timeUnit != Util::TimeUnit::unknown) {
		this->_timeBetweenCreationsTimeUnit = _timeUnit;
	}
}

std::string SourceModelComponent::getTimeBetweenCreationsExpression() const {
	return _timeBetweenCreationsExpression;
}

void SourceModelComponent::setMaxCreations(std::string _maxCreationsExpression) {
	this->_maxCreationsExpression = _maxCreationsExpression;
}

void SourceModelComponent::setMaxCreations(unsigned long _maxCreations) {
	this->_maxCreationsExpression = std::to_string(_maxCreations);
}

std::string SourceModelComponent::getMaxCreations() const {
	return _maxCreationsExpression;
}

unsigned int SourceModelComponent::getEntitiesCreated() const {
	return _entitiesCreatedSoFar;
}

void SourceModelComponent::setEntitiesCreated(unsigned int _entitiesCreated) {
	this->_entitiesCreatedSoFar = _entitiesCreated;
}

void SourceModelComponent::setEntitiesPerCreation(unsigned int _entitiesPerCreation) {
	this->_entitiesPerCreation = _entitiesPerCreation;
	//this->_entitiesCreatedSoFar = _entitiesPerCreation; // that's because "entitiesPerCreation" entities are included in the future events list BEFORE replication starts (to initialize events list)
}

unsigned int SourceModelComponent::getEntitiesPerCreation() const {
	return _entitiesPerCreation;
}
