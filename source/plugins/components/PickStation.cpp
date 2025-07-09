/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PickStation.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 11 de Setembro de 2019, 13:08
 */

#include "PickStation.h"
#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/Simulator.h"
#include "../../kernel/simulator/PluginManager.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &PickStation::GetPluginInformation;
}
#endif

//******************************************

ModelDataDefinition* PickStation::NewInstance(Model* model, std::string name) {
	return new PickStation(model, name);
}

void PickStation::setPickConditionExpression(bool _pickConditionExpression) {
	this->_pickConditionExpression = _pickConditionExpression;
}

bool PickStation::isPickConditionExpression() const {
	return _pickConditionExpression;
}

void PickStation::setPickConditionNumberInQueue(bool _pickConditionNumberInQueue) {
	this->_pickConditionNumberInQueue = _pickConditionNumberInQueue;
}

bool PickStation::isPickConditionNumberInQueue() const {
	return _pickConditionNumberInQueue;
}

void PickStation::setPickConditionNumberBusyResource(bool _pickConditionNumberBusyResource) {
	this->_pickConditionNumberBusyResource = _pickConditionNumberBusyResource;
}

bool PickStation::isPickConditionNumberBusyResource() const {
	return _pickConditionNumberBusyResource;
}

/*
void PickStation::setPickCondition(PickStation::PickCondition _pickCondition) {
	this->_pickCondition = _pickCondition;
}

PickStation::PickCondition PickStation::getPickCondition() const {
	return _pickCondition;
}
*/

void PickStation::setTestCondition(PickStation::TestCondition _testCondition) {
	this->_testCondition = _testCondition;
}

PickStation::TestCondition PickStation::getTestCondition() const {
	return _testCondition;
}

void PickStation::setSaveAttribute(std::string _saveAttribute) {
	this->_saveAttribute = _saveAttribute;
}

std::string PickStation::getSaveAttribute() const {
	return _saveAttribute;
}

List<PickableStationItem*>* PickStation::getPickableStationItens() const {
	return _pickableStationItens;
}

void PickStation::addPickableStationItem(PickableStationItem* newItem) {
	_pickableStationItens->insert(newItem);
}

void PickStation::removePickableStationItem(PickableStationItem* item) {
	_pickableStationItens->remove(item);
}

std::string PickStation::convertEnumToStr(TestCondition condition) {
	switch (static_cast<int> (condition)) {
		case 0: return "MINIMUM";
		case 1: return "MAXIMUM";
	}
	return "Unknown";
}

PickStation::PickStation(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<PickStation>(), name) {
    SimulationControlGenericEnum<PickStation::TestCondition, PickStation>* propTestCondition = new SimulationControlGenericEnum<PickStation::TestCondition, PickStation>(
                                    std::bind(&PickStation::getTestCondition, this), std::bind(&PickStation::setTestCondition, this, std::placeholders::_1),
                                    Util::TypeOf<PickStation>(), getName(), "TestCondition", "");
	SimulationControlGeneric<std::string>* propSaveAttribute = new SimulationControlGeneric<std::string>(
									std::bind(&PickStation::getSaveAttribute, this), std::bind(&PickStation::setSaveAttribute, this, std::placeholders::_1),
									Util::TypeOf<PickStation>(), getName(), "SaveAttribute", "");
	SimulationControlGeneric<bool>* propPickConditionExpression = new SimulationControlGeneric<bool>(
									std::bind(&PickStation::isPickConditionExpression, this), std::bind(&PickStation::setPickConditionExpression, this, std::placeholders::_1),
									Util::TypeOf<PickStation>(), getName(), "PickConditionExpression", "");
	SimulationControlGeneric<bool>* propPickConditionNumberInQueue = new SimulationControlGeneric<bool>(
									std::bind(&PickStation::isPickConditionNumberInQueue, this), std::bind(&PickStation::setPickConditionNumberInQueue, this, std::placeholders::_1),
									Util::TypeOf<PickStation>(), getName(), "PickConditionNumberInQueue", "");
	SimulationControlGeneric<bool>* propPickConditionNumberBusyResource = new SimulationControlGeneric<bool>(
									std::bind(&PickStation::isPickConditionNumberBusyResource, this), std::bind(&PickStation::setPickConditionNumberBusyResource, this, std::placeholders::_1),
									Util::TypeOf<PickStation>(), getName(), "PickConditionNumberBusyResource", "");
	SimulationControlGenericListPointer<PickableStationItem*, Model*, PickableStationItem>* propPickableStationItens = new SimulationControlGenericListPointer<PickableStationItem*, Model*, PickableStationItem> (
									_parentModel,
                                    std::bind(&PickStation::getPickableStationItens, this), std::bind(&PickStation::addPickableStationItem, this, std::placeholders::_1), std::bind(&PickStation::removePickableStationItem, this, std::placeholders::_1),
									Util::TypeOf<PickStation>(), getName(), "PickableStationItens", "");																																	

    _parentModel->getControls()->insert(propTestCondition);
	_parentModel->getControls()->insert(propSaveAttribute);
	_parentModel->getControls()->insert(propPickConditionExpression);
	_parentModel->getControls()->insert(propPickConditionNumberInQueue);
	_parentModel->getControls()->insert(propPickConditionNumberBusyResource);
	_parentModel->getControls()->insert(propPickableStationItens);

	// setting properties
    _addProperty(propTestCondition);
	_addProperty(propSaveAttribute);
	_addProperty(propPickConditionExpression);
	_addProperty(propPickConditionNumberInQueue);
	_addProperty(propPickConditionNumberBusyResource);
	_addProperty(propPickableStationItens);
}

std::string PickStation::show() {
	return ModelComponent::show() + "";
}

// public static 

ModelComponent* PickStation::LoadInstance(Model* model, PersistenceRecord *fields) {
	PickStation* newComponent = new PickStation(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

PluginInformation* PickStation::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<PickStation>(), &PickStation::LoadInstance, &PickStation::NewInstance);
	info->setCategory("Decisions");
	info->insertDynamicLibFileDependence("station.so");
	info->insertDynamicLibFileDependence("resource.so");
	info->insertDynamicLibFileDependence("queue.so");
	std::string text = "The PickStation module allows an entity to select a station from the multiple stations specified.";
	text += " This module picks among the group of stations based on the selection logic defined with the module.";
	text += " The entity may then route, transport, convey, or connect to the station specified.";
	text += " If the method chosen is connect, the selected station is assigned to an entity attribute.";
	text += " The station selection process is based on the minimum or maximum value of a variety of system variables and expressions.";
	text += " TYPICAL USES: (1) A part sent to a processing station based on machine’s availability at each station;";
	text += " (2) A loan application sent to a set of loan officers based on the number sent to each officer;";
	text += " (3) A customer selecting among cashier lines based on the least number waiting in each line";
	info->setDescriptionHelp(text);
	return info;
}

// protected virtual -- must be overriden

void PickStation::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	double value, valueResource=0, valueQueue=0, valueExpression=0, bestValue;
	Station* bestStation = nullptr;
	if (_testCondition == TestCondition::MAXIMUM) {
		bestValue = std::numeric_limits<double>::min();
	} else {
		bestValue = std::numeric_limits<double>::max();
	}
	for (PickableStationItem* item : *_pickableStationItens->list()) {
		if (_pickConditionExpression) {
			valueExpression = _parentModel->parseExpression(item->getExpression());
		}
		if (_pickConditionNumberInQueue) {
			if (item->getQueue() != nullptr) {
				valueQueue = item->getQueue()->size();
			} else {
				valueQueue = 0;
			}
		}
		if (_pickConditionNumberBusyResource) {
			if (item->getResource() != nullptr) {
				valueResource = item->getResource()->getNumberBusy();
			} else {
				valueResource = 0;
			}
		}
		value = valueResource + valueQueue + valueExpression;
		if ((_testCondition == TestCondition::MAXIMUM && value > bestValue) || (_testCondition == TestCondition::MINIMUM && value < bestValue)) {
			bestValue = value;
			bestStation = item->getStation();
		}
	}
	entity->setAttributeValue(_saveAttribute, bestStation->getId());
	this->_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}

bool PickStation::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		// @TODO: not implemented yet
	}
	return res;
}

void PickStation::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	// @TODO: not implemented yet
}


// protected virtual -- could be overriden 

//ParserChangesInformation* DummyElement::_getParserChangesInformation() {}

bool PickStation::_check(std::string* errorMessage) {
	bool resultAll = true;
	//resultAll &= _someString != "";
	//resultAll &= _someUint > 0;
	return resultAll;
}

ParserChangesInformation* PickStation::_getParserChangesInformation() {
	ParserChangesInformation* changes = new ParserChangesInformation();
	//@TODO not implemented yet
	//changes->getProductionToAdd()->insert(...);
	//changes->getTokensToAdd()->insert(...);
	return changes;
}

void PickStation::_initBetweenReplications() {
	//_someString = "Test";
	//_someUint = 1;
}

void PickStation::_createInternalAndAttachedData() {
	unsigned int i = 0;
	_attachedDataClear();
	for (PickableStationItem* item : *_pickableStationItens->list()) {
		_attachedDataInsert("Station" + std::to_string(i), item->getStation());
		if (item->getResource() != nullptr) {
			_attachedDataInsert("Resource" + std::to_string(i), item->getResource());
		}
		if (item->getQueue() != nullptr) {
			_attachedDataInsert("Queue" + std::to_string(i), item->getQueue());
		}
		i++;
	}
	//if (_internalDataDefinition == nullptr) {
	//	PluginManager* pm = _parentModel->getParentSimulator()->getPlugins();
	//	_internalDataDefinition = pm->newInstance<DummyElement>(_parentModel, getName() + "." + "JustaDummy");
	//	_internalDataInsert("JustaDummy", _internalDataDefinition);
	//}
}

void PickStation::_addProperty(PropertyBase* property) {
	_properties->insert(property);
}



