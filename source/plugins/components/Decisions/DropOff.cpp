/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DropOff.cpp
 * Author: rlcancian
 * 
 * Created on 03 de Junho de 2019, 15:15
 */

#include "plugins/components/Decisions/DropOff.h"
#include "../../../kernel/simulator/model/Model.h"
#include "../../../kernel/simulator/essentialPlugins/Attribute.h"
#include "../../data/Grouping/EntityGroup.h"
#include <algorithm>
#include <cstdlib>
#include <vector>

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &DropOff::GetPluginInformation;
}
#endif

ModelDataDefinition* DropOff::NewInstance(Model* model, std::string name) {
	return new DropOff(model, name);
}

DropOff::DropOff(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<DropOff>(), name) {
	SimulationControlGeneric<std::string>* propQuantity = new SimulationControlGeneric<std::string>(
			std::bind(&DropOff::getQuantityExpression, this), std::bind(&DropOff::setQuantityExpression, this, std::placeholders::_1),
			Util::TypeOf<DropOff>(), getName(), "QuantityExpression", "");
	SimulationControlGeneric<std::string>* propStartingRank = new SimulationControlGeneric<std::string>(
			std::bind(&DropOff::getStartingRankExpression, this), std::bind(&DropOff::setStartingRankExpression, this, std::placeholders::_1),
			Util::TypeOf<DropOff>(), getName(), "StartingRankExpression", "");
	_parentModel->getControls()->insert(propQuantity);
	_parentModel->getControls()->insert(propStartingRank);
	_addSimulationControl(propQuantity);
	_addSimulationControl(propStartingRank);
}

std::string DropOff::show() {
	return ModelComponent::show() +
			",quantityExpression=" + _quantityExpression +
			",startingRankExpression=" + _startingRankExpression;
}

void DropOff::setQuantityExpression(std::string quantityExpression) {
	_quantityExpression = quantityExpression;
}

std::string DropOff::getQuantityExpression() const {
	return _quantityExpression;
}

void DropOff::setStartingRankExpression(std::string startingRankExpression) {
	_startingRankExpression = startingRankExpression;
}

std::string DropOff::getStartingRankExpression() const {
	return _startingRankExpression;
}

ModelComponent* DropOff::LoadInstance(Model* model, PersistenceRecord *fields) {
	DropOff* newComponent = new DropOff(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load DropOff instance: " + std::string(e.what()));
	}
	return newComponent;
}

void DropOff::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void) inputPortNumber;
	const unsigned int entityGroupId = static_cast<unsigned int>(entity->getAttributeValue("Entity.Group"));
	if (entityGroupId == 0u) {
		_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
		return;
	}
	EntityGroup* entityGroup = dynamic_cast<EntityGroup*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<EntityGroup>(), entityGroupId));
	if (entityGroup == nullptr) {
		traceError("DropOff could not find EntityGroup Id=" + std::to_string(entityGroupId) + ".", TraceManager::Level::L3_errorRecover);
		_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
		return;
	}
	List<Entity*>* members = entityGroup->getGroup(entity->getId());
	if (members == nullptr || members->size() == 0u) {
		_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
		return;
	}
	const int parsedQuantity = _parentModel->parseExpression(_quantityExpression);
	const int parsedStartingRank = _parentModel->parseExpression(_startingRankExpression);
	const unsigned int quantity = static_cast<unsigned int>(std::max(0, parsedQuantity));
	const unsigned int startRank = parsedStartingRank <= 1 ? 0u : static_cast<unsigned int>(parsedStartingRank - 1);
	if (quantity == 0u) {
		traceError("DropOff quantity evaluated to zero. Representative entity will continue unchanged.", TraceManager::Level::L3_errorRecover);
		_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
		return;
	}
	std::vector<Entity*> removedMembers;
	for (unsigned int offset = 0; offset < quantity; ++offset) {
		Entity* member = members->getAtRank(startRank);
		if (member == nullptr) {
			break;
		}
		removedMembers.push_back(member);
		entityGroup->removeElement(entity->getId(), member);
		member->setAttributeValue("Entity.Group", 0.0);
		_parentModel->sendEntityToComponent(member, this->getConnectionManager()->getConnectionAtPort(1));
	}
	_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}

bool DropOff::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_quantityExpression = fields->loadField("quantityExpression", DEFAULT.quantityExpression);
		_startingRankExpression = fields->loadField("startingRankExpression", DEFAULT.startingRankExpression);
	}
	return res;
}

//void DropOff::_initBetweenReplications() {}

void DropOff::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("quantityExpression", _quantityExpression, DEFAULT.quantityExpression, saveDefaultValues);
	fields->saveField("startingRankExpression", _startingRankExpression, DEFAULT.startingRankExpression, saveDefaultValues);
}

bool DropOff::_check(std::string& errorMessage) {
	bool resultAll = true;
	_attachedAttributesInsert({"Entity.Group"});
	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Attribute>(), "Entity.Group", "Entity.Group", true, errorMessage);
	resultAll &= _parentModel->checkExpression(_quantityExpression, "QuantityExpression", errorMessage);
	resultAll &= _parentModel->checkExpression(_startingRankExpression, "StartingRankExpression", errorMessage);
	if (this->getConnectionManager()->size() < 2) {
		errorMessage += "DropOff requires two output connections.";
		resultAll = false;
	}
	return resultAll;
}

PluginInformation* DropOff::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<DropOff>(), &DropOff::LoadInstance, &DropOff::NewInstance);
	info->setCategory("Decisions");
	info->setMinimumOutputs(2);
	info->setMaximumOutputs(2);
	info->insertDynamicLibFileDependence("entitygroup.so");
	info->insertDynamicLibFileDependence("attribute.so");
	info->setDescriptionHelp("Drops a parsed number of grouped members starting at a parsed 1-based rank to output port 1 while the representative entity continues on port 0.");
	return info;
}


// void DropOff::_createInternalStatisticReporters() { }

// void DropOff::_createEditableDataDefinitions() { }

// void DropOff::_createAttachedAttributes() { }
