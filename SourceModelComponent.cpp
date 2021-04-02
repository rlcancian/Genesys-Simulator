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
#include "Attribute.h"

//using namespace GenesysKernel;

SourceModelComponent::SourceModelComponent(Model* model, std::string componentTypename, std::string name) : ModelComponent(model, componentTypename, name) {
}

std::string SourceModelComponent::show() {
	std::string text = ModelComponent::show() +
			",entityType=\"" + _entityType->getName() + "\"" +
			",firstCreation=" + std::to_string(_firstCreation);
	return text;
}

bool SourceModelComponent::_loadInstance(std::map<std::string, std::string>* fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		this->_entitiesPerCreation = std::stoi((*fields->find("entitiesPerCreation")).second);
		this->_firstCreation = std::stod((*fields->find("firstCreation")).second);
		this->_timeBetweenCreationsExpression = (*fields->find("timeBetweenCreations")).second;
		this->_timeBetweenCreationsTimeUnit = static_cast<Util::TimeUnit> (std::stoi((*fields->find("timeBetweenCreationsTimeUnit")).second));
		this->_maxCreationsExpression = (*fields->find("maxCreations")).second;
		std::string entityTypename = (*fields->find("entityTypename")).second;
		this->_entityType = dynamic_cast<EntityType*> (_parentModel->getElements()->getElement(Util::TypeOf<EntityType>(), entityTypename));
	}
	return res;
}

void SourceModelComponent::_initBetweenReplications() {
	this->_entitiesCreatedSoFar = 0;
}

std::map<std::string, std::string>* SourceModelComponent::_saveInstance() {
	std::map<std::string, std::string>* fields = ModelComponent::_saveInstance();
	fields->emplace("entitiesPerCreation", std::to_string(this->_entitiesPerCreation));
	fields->emplace("firstCreation", std::to_string(this->_firstCreation));
	fields->emplace("timeBetweenCreations", "\"" + this->_timeBetweenCreationsExpression + "\"");
	fields->emplace("timeBetweenCreationsTimeUnit", std::to_string(static_cast<int> (this->_timeBetweenCreationsTimeUnit)));
	fields->emplace("maxCreations", "\"" + this->_maxCreationsExpression + "\"");
	fields->emplace("entityTypename", (this->_entityType->getName())); // save the name
	//fields->emplace("collectStatistics" , std::to_string(this->_collectStatistics));
	return fields;
}

bool SourceModelComponent::_check(std::string* errorMessage) {
	/* include attributes needed */
	ElementManager* elements = _parentModel->getElements();
	std::vector<std::string> neededNames = {"Entity.ArrivalTime"}; 
	std::string neededName;
	for (unsigned int i = 0; i < neededNames.size(); i++) {
		neededName = neededNames[i];
		if (elements->getElement(Util::TypeOf<Attribute>(), neededName) == nullptr) {
			Attribute* attr1 = new Attribute(_parentModel, neededName);
			elements->insert(attr1);
		}
	}
	bool resultAll = true;
	resultAll &= _parentModel->getElements()->check(Util::TypeOf<EntityType>(), _entityType, "entitytype", errorMessage);
	resultAll &= _parentModel->checkExpression(this->_timeBetweenCreationsExpression, "time between creations", errorMessage);
	return resultAll;
}

void SourceModelComponent::setFirstCreation(double _firstCreation) {
	this->_firstCreation = _firstCreation;
}

double SourceModelComponent::getFirstCreation() const {
	return _firstCreation;
}

//void SourceModelComponent::setCollectStatistics(bool _collectStatistics) {
//    this->_collectStatistics = _collectStatistics;
//}
//
//bool SourceModelComponent::isCollectStatistics() const {
//    return _collectStatistics;
//}

void SourceModelComponent::setEntityType(EntityType* entityType) {
	_entityType = entityType;
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

void SourceModelComponent::setTimeBetweenCreationsExpression(std::string _timeBetweenCreations) {
	this->_timeBetweenCreationsExpression = _timeBetweenCreations;
}

std::string SourceModelComponent::getTimeBetweenCreationsExpression() const {
	return _timeBetweenCreationsExpression;
}

void SourceModelComponent::setMaxCreations(std::string _maxCreationsExpression) {
	this->_maxCreationsExpression = _maxCreationsExpression;
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

