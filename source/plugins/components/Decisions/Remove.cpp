/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Remove.cpp
 * Author: rlcancian
 * 
 * Created on 03 de Junho de 2019, 15:20
 */

#include "plugins/components/Decisions/Remove.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/SimulationControlAndResponse.h"
#include "../../data/Grouping/EntityGroup.h"
#include "plugins/data/DiscreteProcessing/Queue.h"
#include <algorithm>
#include <cstdlib>
#include <vector>

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Remove::GetPluginInformation;
}
#endif

ModelDataDefinition* Remove::NewInstance(Model* model, std::string name) {
	return new Remove(model, name);
}

void Remove::setRemoveEndRank(std::string _removeEndRank) {
	this->_removeEndRank = _removeEndRank;
}

std::string Remove::getRemoveEndRank() const {
	return _removeEndRank;
}

void Remove::setRemoveStartRank(std::string _removeFromRank) {
	this->_removeStartRank = _removeFromRank;
}

std::string Remove::getRemoveStartRank() const {
	return _removeStartRank;
}

void Remove::setRemoveFromType(Remove::RemoveFromType _removeFromType) {
	this->_removeFromType = _removeFromType;
}

Remove::RemoveFromType Remove::getRemoveFromType() const {
	return _removeFromType;
}

void Remove::setRemoveFrom(ModelDataDefinition* _removeFrom) {
	this->_removeFrom = _removeFrom;
}

ModelDataDefinition* Remove::getRemoveFrom() const {
	return _removeFrom;
}

std::string Remove::convertEnumToStr(RemoveFromType type) {
	switch (static_cast<int> (type)) {
		case 0: return "QUEUE";
		case 1: return "ENTITYGROUP";
	}
	return "Unknown";
}

Remove::Remove(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Remove>(), name) {
	SimulationControlGeneric<std::string>* propRemoveStart = new SimulationControlGeneric<std::string>(
									std::bind(&Remove::getRemoveStartRank, this), std::bind(&Remove::setRemoveStartRank,  this, std::placeholders::_1),
									Util::TypeOf<Remove>(), getName(), "RemoveStartRank", "");
	SimulationControlGeneric<std::string>* propRemoveEnd = new SimulationControlGeneric<std::string>(
									std::bind(&Remove::getRemoveEndRank, this), std::bind(&Remove::setRemoveEndRank, this, std::placeholders::_1),
									Util::TypeOf<Remove>(), getName(), "RemoveEndRank", "");
    SimulationControlGenericEnum<Remove::RemoveFromType, Remove>* propRemoveType = new SimulationControlGenericEnum<Remove::RemoveFromType, Remove>(
                                    std::bind(&Remove::getRemoveFromType, this), std::bind(&Remove::setRemoveFromType, this, std::placeholders::_1),
                                    Util::TypeOf<Remove>(), getName(), "RemoveFromType", "");
	SimulationControlGenericClass<ModelDataDefinition*, Model*, Queue>* propRemoveFrom = new SimulationControlGenericClass<ModelDataDefinition*, Model*, Queue>(
									_parentModel,
									std::bind(&Remove::getRemoveFrom, this),
									[this](ModelDataDefinition* removeFrom) {
										this->setRemoveFromType(Remove::RemoveFromType::QUEUE);
										this->setRemoveFrom(removeFrom);
									},
									Util::TypeOf<Remove>(), getName(), "RemoveFrom", "");

	
	_parentModel->getControls()->insert(propRemoveFrom);
    _parentModel->getControls()->insert(propRemoveType);
	_parentModel->getControls()->insert(propRemoveStart);
	_parentModel->getControls()->insert(propRemoveEnd);

	_addSimulationControl(propRemoveFrom);
    _addSimulationControl(propRemoveType);
	_addSimulationControl(propRemoveStart);
	_addSimulationControl(propRemoveEnd);
}

std::string Remove::show() {
	return ModelComponent::show() + "";
}

ModelComponent* Remove::LoadInstance(Model* model, PersistenceRecord *fields) {
	Remove* newComponent = new Remove(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

void Remove::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void) inputPortNumber;
	auto parseNumericOrExpression = [this](const std::string& expression)->int {
		char* parseEnd = nullptr;
		double numericValue = std::strtod(expression.c_str(), &parseEnd);
		const bool isNumericConstant = parseEnd != expression.c_str() && *parseEnd == '\0';
		if (isNumericConstant) {
			return static_cast<int>(numericValue);
		}
		return _parentModel->parseExpression(expression);
	};
	if (_removeFromType == RemoveFromType::QUEUE) {
		Queue* queue = dynamic_cast<Queue*> (_removeFrom);
		const int parsedStartRank = parseNumericOrExpression(_removeStartRank);
		const int parsedEndRank = parseNumericOrExpression(_removeEndRank);
		const unsigned int startRank = parsedStartRank <= parsedEndRank ? static_cast<unsigned int>(std::max(0, parsedStartRank)) : static_cast<unsigned int>(std::max(0, parsedEndRank));
		const unsigned int endRank = parsedStartRank <= parsedEndRank ? static_cast<unsigned int>(std::max(0, parsedEndRank)) : static_cast<unsigned int>(std::max(0, parsedStartRank));
		if (startRank == endRank) {
			traceSimulation(this, TraceManager::Level::L7_internal, "Removing entity from queue \"" + queue->getName() + "\" at rank " + std::to_string(startRank) + "  // " + _removeStartRank);
		} else {
			traceSimulation(this, TraceManager::Level::L7_internal, "Removing entities from queue \"" + queue->getName() + "\" from rank " + std::to_string(startRank) + " to rank " + std::to_string(endRank) + "  // " + _removeStartRank + "  // " + _removeEndRank);
		}
		std::vector<Waiting*> waitingToRemove;
		for (unsigned int rank = startRank; rank <= endRank && rank < queue->size(); rank++) {
			Waiting* waiting = queue->getAtRank(rank);
			if (waiting != nullptr) {
				waitingToRemove.push_back(waiting);
				Entity* removedEntity = waiting->getEntity();
				traceSimulation(this, TraceManager::Level::L8_detailed, "Entity \"" + removedEntity->getName() + "\" was removed from queue \"" + queue->getName() + "\" at rank "+std::to_string(rank));
				_parentModel->sendEntityToComponent(removedEntity, this->getConnectionManager()->getConnectionAtPort(1)); // port 1 is the removed entities output
			} else {
				traceSimulation(this, TraceManager::Level::L8_detailed, "Could not remove entity from queue \"" + queue->getName() + "\" at rank " + std::to_string(rank));
			}
		}
		for (Waiting* waiting : waitingToRemove) {
			queue->removeElement(waiting);
		}
	}
	if (_removeFromType == RemoveFromType::ENTITYGROUP) {
		traceSimulation(this, TraceManager::Level::L8_detailed, "RemoveFromType ENTITYGROUP is not supported in this implementation batch.");
	}
	_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}

bool Remove::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_removeFromType = static_cast<RemoveFromType>(fields->loadField("removeFromType", static_cast<int>(DEFAULT.removeFromType)));
		_removeStartRank = fields->loadField("removeStartRank", DEFAULT.removeStartRank);
		_removeEndRank = fields->loadField("removeEndRank", DEFAULT.removeEndRank);
		std::string removeFromName = fields->loadField("removeFrom", "");
		if (removeFromName != "") {
			if (_removeFromType == RemoveFromType::QUEUE) {
				_removeFrom = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), removeFromName);
			} else if (_removeFromType == RemoveFromType::ENTITYGROUP) {
				_removeFrom = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<EntityGroup>(), removeFromName);
			}
		}
	}
	return res;
}

void Remove::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("removeFromType", static_cast<int>(_removeFromType), static_cast<int>(DEFAULT.removeFromType), saveDefaultValues);
	fields->saveField("removeStartRank", _removeStartRank, DEFAULT.removeStartRank, saveDefaultValues);
	fields->saveField("removeEndRank", _removeEndRank, DEFAULT.removeEndRank, saveDefaultValues);
	if (_removeFrom != nullptr) {
		fields->saveField("removeFrom", _removeFrom->getName(), "", saveDefaultValues);
	}
}

bool Remove::_check(std::string& errorMessage) {
	bool resultAll = true;
	bool sucess = false;
	std::string msg = "";
	resultAll = _removeFrom != nullptr;
	if (!resultAll) {
		errorMessage += "RemoveFrom was not defined.";
	}
	if (_removeStartRank == "") {
		resultAll = false;
		errorMessage += "RemoveStartRank was not defined.";
	} else {
		_parentModel->parseExpression(_removeStartRank, sucess, msg);
		resultAll &= sucess;
		if (!sucess) {
			errorMessage += msg;
		}
	}
	if (_removeEndRank == "") {
		resultAll = false;
		errorMessage += "RemoveEndRank was not defined.";
	} else {
		_parentModel->parseExpression(_removeEndRank, sucess, msg);
		resultAll &= sucess;
		if (!sucess) {
			errorMessage += msg;
		}
	}
	if (resultAll) {
		if (_removeFromType == RemoveFromType::ENTITYGROUP) {
			resultAll = false;
			errorMessage += "RemoveFromType ENTITYGROUP is not supported in this implementation batch.";
			return resultAll;
		}
		resultAll &= (_removeFrom->getClassname() == Util::TypeOf<Queue>() && _removeFromType == RemoveFromType::QUEUE) ||
				(_removeFrom->getClassname() == Util::TypeOf<EntityGroup>() && _removeFromType == RemoveFromType::ENTITYGROUP);
		if (!resultAll) {
			errorMessage += "RemoveFromType differs from what RemoveFrom actually is.";
		}
	}
	return resultAll;
}

void Remove::_createInternalAndAttachedData() {
	PluginManager* plugins = _parentModel->getParentSimulator()->getPluginManager();
	if (_removeFromType == Remove::RemoveFromType::QUEUE) {
		if (_removeFrom == nullptr) {
			_removeFrom = plugins->newInstance<Queue>(_parentModel, getName() + ".Queue");
			if (_removeFrom == nullptr) {
				_removeFrom = new Queue(_parentModel, getName() + ".Queue");
			}
		}
		if (_removeFrom != nullptr) {
			_attachedDataInsert("Queue", _removeFrom);
		} else {
			_attachedDataRemove("Queue");
		}
	}
	if (_removeFromType == Remove::RemoveFromType::ENTITYGROUP) {
		// Not supported in this implementation batch. Explicitly validated in _check().
	}
}

PluginInformation* Remove::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Remove>(), &Remove::LoadInstance, &Remove::NewInstance);
	info->setCategory("Decisions");
	info->insertDynamicLibFileDependence("queue.so");
	info->insertDynamicLibFileDependence("entitygroup.so");
	info->setMinimumOutputs(2);
	info->setMaximumOutputs(2);
	// ...
	return info;
}
