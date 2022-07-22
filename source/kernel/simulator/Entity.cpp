/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Entity.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 21 de Junho de 2018, 16:30
 */

#include <typeinfo>
#include "Entity.h"
#include "Attribute.h"
#include "Model.h"
#include <cassert>

//using namespace GenesysKernel;

Entity::Entity(Model* model, std::string name, bool insertIntoModel) : ModelDataDefinition(model, Util::TypeOf<Entity>(), name, insertIntoModel) {
	_entityNumber = Util::GetLastIdOfType(Util::TypeOf<Entity>());
	unsigned int numAttributes = _parentModel->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<Attribute>());
	for (unsigned i = 0; i < numAttributes; i++) {
		std::map<std::string, double>* map = new std::map<std::string, double>();
		_attributeValues->insert(map);
	}
}

void Entity::setEntityTypeName(std::string entityTypeName) {
	EntityType* entitytype = dynamic_cast<EntityType*> (_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<EntityType>(), entityTypeName));
	if (entitytype != nullptr) {
		this->_entityType = entitytype;
	}
}

std::string Entity::getEntityTypeName() const {
	return this->_entityType->getName();
}

void Entity::setEntityType(EntityType* entityType) {

	_entityType = entityType;
}

EntityType* Entity::getEntityType() const {
	return _entityType;
}

std::string Entity::show() {
	std::string message = ModelDataDefinition::show();
	if (this->_entityType != nullptr) {
		message += ",entityType=\"" + this->_entityType->getName() + "\"";
	}
	message += ",attributes=[";
	_attributeValues->front();
	for (unsigned int i = 0; i < _attributeValues->size(); i++) {
		std::map<std::string, double>* map = _attributeValues->current();
		std::string attributeName = _parentModel->getDataManager()->getDataDefinitionList(Util::TypeOf<Attribute>())->getAtRank(i)->getName();
		message += attributeName + "=";
		if (map->size() == 0) { // scalar
			message += "NaN;"; //std::to_string(map->begin()->second) + ";";
		} else if (map->size() == 1) { // scalar
			message += strTruncIfInt(std::to_string(map->begin()->second)) + ", ";
		} else {
			// array or matrix
			message += "[";
			for (std::pair<std::string, double> valIt : *map) {
				message += valIt.first + "=>" + strTruncIfInt(std::to_string(valIt.second)) + ", ";
			}
			message = message.substr(0, message.length() - 2);
			message += "];";
		}
		_attributeValues->next();
	}
	message = message.substr(0, message.length() - 1);
	message += "]";

	return message;
}

double Entity::getAttributeValue(std::string attributeName) {

	return getAttributeValue("", attributeName);
}

double Entity::getAttributeValue(std::string index, std::string attributeName) {
	int rank = _parentModel->getDataManager()->getRankOf(Util::TypeOf<Attribute>(), attributeName);
	if (rank >= 0) {
		std::map<std::string, double>* map = this->_attributeValues->getAtRank(rank);
		std::map<std::string, double>::iterator mapIt = map->find(index);
		if (mapIt != map->end()) {//found
			return (*mapIt).second;
		} else { // not found
			return 0.0;
		}
	}
	_parentModel->getTracer()->traceError(TraceManager::Level::L3_errorRecover, "Attribute \"" + attributeName + "\" not found");
	return 0.0;
}

double Entity::getAttributeValue(Util::identification attributeID) {

	return getAttributeValue("", attributeID);
}

double Entity::getAttributeValue(std::string index, Util::identification attributeID) {
	//assert(this->_parentModel != nullptr);
	ModelDataDefinition* modeldatum = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Attribute>(), attributeID);
	if (modeldatum != nullptr) {

		return getAttributeValue(index, modeldatum->getName());
	}
	return 0.0; // attribute not found
}

void Entity::setAttributeValue(std::string attributeName, double value) {
	setAttributeValue("", attributeName, value);
}

void Entity::setAttributeValue(std::string index, std::string attributeName, double value) {
	int rank = _parentModel->getDataManager()->getRankOf(Util::TypeOf<Attribute>(), attributeName);
	if (rank >= 0) {
		std::map<std::string, double>* map = _attributeValues->getAtRank(rank);
		std::map<std::string, double>::iterator mapIt = map->find(index);
		if (mapIt != map->end()) {//found
			(*mapIt).second = value;
		} else { // not found
			map->insert({index, value}); // (map->end(), std::pair<std::string, double>(index, value));
		}
	} else
		_parentModel->getTracer()->traceError(TraceManager::Level::L3_errorRecover, "Attribute \"" + attributeName + "\" not found");

}

void Entity::setAttributeValue(Util::identification attributeID, double value) {

	setAttributeValue("", attributeID, value);
}

void Entity::setAttributeValue(std::string index, Util::identification attributeID, double value) {

	std::string attrname = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Attribute>(), attributeID)->getName();
	setAttributeValue(index, attrname, value);
}

Util::identification Entity::entityNumber() const {

	return _entityNumber;
}

bool Entity::_loadInstance(PersistenceRecord *fields) {
	// never loads an entity 
	return true;
}

void Entity::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
}

bool Entity::_check(std::string* errorMessage) {
	*errorMessage += "";
	return true;
}
