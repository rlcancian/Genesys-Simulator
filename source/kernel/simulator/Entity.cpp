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
		SparseValueStore* store = new SparseValueStore();
		Attribute* attribute = dynamic_cast<Attribute*>(_parentModel->getDataManager()->getDataDefinitionList(Util::TypeOf<Attribute>())->getAtRank(i));
		if (attribute != nullptr) {
			// Each entity receives its own mutable copy of the attribute initial sparse values.
			*store = *attribute->getInitialValueStore();
		}
		_attributeValues->insert(store);
	}
}

Entity::~Entity() {
	// Release each per-attribute sparse value store owned by this entity instance.
	for (SparseValueStore* attributeStore : *_attributeValues->list()) {
		delete attributeStore;
	}
	// Release the container that tracks all attribute stores created for this entity.
	delete _attributeValues;
	_attributeValues = nullptr;
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
		SparseValueStore* store = _attributeValues->current();
		std::map<std::string, double>* values = store != nullptr ? store->values() : nullptr;
		std::string attributeName = _parentModel->getDataManager()->getDataDefinitionList(Util::TypeOf<Attribute>())->getAtRank(i)->getName();
		message += attributeName + "=";
		if (values == nullptr || values->size() == 0) { // scalar
			message += "NaN;"; //std::to_string(map->begin()->second) + ";";
		} else if (values->size() == 1 && values->begin()->first.empty()) { // scalar
			message += Util::StrTruncIfInt(std::to_string(values->begin()->second)) + ", ";
		} else {
			// array or matrix
			message += "[";
			for (std::pair<std::string, double> valIt : *values) {
				message += valIt.first + "=>" + Util::StrTruncIfInt(std::to_string(valIt.second)) + ", ";
			}
			message = message.substr(0, message.length() - 2);
			message += "];";
		}
		_attributeValues->next();
	}
	if (_attributeValues->size() > 0) {
		message = message.substr(0, message.length() - 1);
	}
	message += "]";

	return message;
}

double Entity::getAttributeValue(std::string attributeName, std::string index) {
	int rank = _parentModel->getDataManager()->getRankOf(Util::TypeOf<Attribute>(), attributeName);
	if (rank >= 0) {
		return _ensureAttributeStore(rank)->value(index);
	}
	traceError("Attribute \"" + attributeName + "\" not found", TraceManager::Level::L3_errorRecover);
	return 0.0;
}

double Entity::getAttributeValue(Util::identification attributeID, std::string index) {
	//assert(this->_parentModel != nullptr);
	ModelDataDefinition* modeldatum = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Attribute>(), attributeID);
	if (modeldatum != nullptr) {
		return getAttributeValue(modeldatum->getName(), index);
	}
	return 0.0; // attribute not found
}

void Entity::setAttributeValue(std::string attributeName, double value, std::string index, bool createIfNotFound) {
	int rank = _parentModel->getDataManager()->getRankOf(Util::TypeOf<Attribute>(), attributeName);
	if (rank < 0) {
		if (createIfNotFound) {
			new Attribute(_parentModel, attributeName);
			rank = _parentModel->getDataManager()->getRankOf(Util::TypeOf<Attribute>(), attributeName);
		} else
			traceError("Attribute \"" + attributeName + "\" not found", TraceManager::Level::L3_errorRecover);
	}
	if (rank >= 0) {
		_ensureAttributeStore(rank)->setValue(value, index);
		// @ToDo: (importante): Check if it is a special attribute, eg Entity.Type
	}
}

void Entity::setAttributeValue(Util::identification attributeID, double value, std::string index) {
	ModelDataDefinition* attribute = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Attribute>(), attributeID);
	if (attribute != nullptr) {
		setAttributeValue(attribute->getName(), value, index);
	}
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

bool Entity::_check(std::string& errorMessage) {
	errorMessage += "";
	return true;
}

SparseValueStore* Entity::_ensureAttributeStore(unsigned int rank) {
	while (_attributeValues->size() <= rank) {
		_attributeValues->insert(new SparseValueStore());
	}
	SparseValueStore* store = _attributeValues->getAtRank(rank);
	if (store == nullptr) {
		store = new SparseValueStore();
		_attributeValues->setAtRank(rank, store);
	}
	return store;
}
