/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Clone.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 30 de Novembro de 2021, 18:50
 */


#include "Clone.h"
#include "../../kernel/simulator/Attribute.h"
#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/Simulator.h"
#include "../../kernel/simulator/PluginManager.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Clone::GetPluginInformation;
}
#endif

ModelDataDefinition* Clone::NewInstance(Model* model, std::string name) {
	return new Clone(model, name);
}

Clone::Clone(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Clone>(), name) {
}

std::string Clone::show() {
	return ModelComponent::show() + "";
}

// public

void Clone::setNumClonesExpression(std::string numClones) {
	_numClonesExpression = numClones;
}

std::string Clone::getNumClonesExpression() const {
	return _numClonesExpression;
}

// public static 

ModelComponent* Clone::LoadInstance(Model* model, PersistenceRecord *fields) {
	Clone* newComponent = new Clone(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

PluginInformation* Clone::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Clone>(), &Clone::LoadInstance, &Clone::NewInstance);
	info->setDescriptionHelp("//@TODO");
	info->setMinimumOutputs(2); // port 0: original ; port 1: clones
	info->setMaximumOutputs(2);
	return info;
}

// protected virtual -- must be overriden

void Clone::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	unsigned int numClones = _parentModel->parseExpression(_numClonesExpression);
	_parentModel->getTracer()->traceSimulation(this, TraceManager::Level::L7_internal, "Clonig " + std::to_string(numClones) + " entities.  // " + _numClonesExpression);
	std::string attribName;
	double value;
	for (unsigned int i = 0; i < numClones; i++) {
		Entity* newEntity = _parentModel->createEntity(entity->getEntityType()->getName() + "_%", true);
		newEntity->setEntityType(entity->getEntityType());
		for (ModelDataDefinition* attrib : *_parentModel->getDataManager()->getDataDefinitionList(Util::TypeOf<Attribute>())->list()) {
			attribName = attrib->getName();
			value = entity->getAttributeValue(attribName);
			newEntity->setAttributeValue(attribName, value);
		}
		_parentModel->getTracer()->traceSimulation(this, TraceManager::Level::L8_detailed, "Entity \"" + entity->getName() + "\" was cloned to " + newEntity->getName());
		_parentModel->sendEntityToComponent(newEntity, this->getConnections()->getConnectionAtPort(1)); // port 1 is the clone output port
	}
	if (_reportStatistics) {
		_counter->incCountValue(numClones);
	}
	this->_parentModel->sendEntityToComponent(entity, this->getConnections()->getFrontConnection()); // port 0 is the original output
}

bool Clone::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
            _numClonesExpression = fields->loadField("NumClonesExpression", DEFAULT.numClonesExpression);
	}
	return res;
}

void Clone::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("NumClonesExpression", _numClonesExpression, DEFAULT.numClonesExpression, saveDefaultValues);
}


// protected virtual -- could be overriden 

//ParserChangesInformation* DummyElement::_getParserChangesInformation() {}

bool Clone::_check(std::string* errorMessage) {
	return _parentModel->checkExpression(_numClonesExpression, "Num clones", errorMessage);
}

void Clone::_initBetweenReplications() {
}

void Clone::_createInternalAndAttachedData() {
	if (_reportStatistics) {
		if (_counter == nullptr) {
			PluginManager* pm = _parentModel->getParentSimulator()->getPlugins();
			_counter = pm->newInstance<Counter>(_parentModel, getName() + "." + "CountClones");
			_internalDataInsert("CountClones", _counter);
		}
	} else {
		if (_counter != nullptr) {
			_internalDataRemove("CountClones");
		}
	}
}