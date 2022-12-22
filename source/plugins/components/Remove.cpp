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

#include "Remove.h"
#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/Simulator.h"
#include "../../plugins/data/EntityGroup.h"
#include "../../plugins/data/Queue.h"

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

Remove::Remove(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Remove>(), name) {
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
	if (_removeFromType == RemoveFromType::QUEUE) {
		Queue* queue = dynamic_cast<Queue*> (_removeFrom);
		unsigned int startRank = _parentModel->parseExpression(_removeStartRank);
		unsigned int endRank = _parentModel->parseExpression(_removeEndRank);
		if (startRank == endRank) {
			_parentModel->getTracer()->traceSimulation(this, TraceManager::Level::L7_internal, "Removing entity from queue \"" + queue->getName() + "\" at rank " + std::to_string(startRank) + "  // " + _removeStartRank);
		} else {
			_parentModel->getTracer()->traceSimulation(this, TraceManager::Level::L7_internal, "Removing entities from queue \"" + queue->getName() + "\" from rank " + std::to_string(startRank) + " to rank " + std::to_string(endRank) + "  // " + _removeStartRank + "  // " + _removeEndRank);
		}
		Waiting* waiting;
		for (unsigned int rank = startRank; rank < endRank; rank++) {
			waiting = queue->getAtRank(rank);
			if (waiting != nullptr) {
				//queue->removeElement(waiting); // will remove later, on other loop
				Entity* removedEntity = waiting->getEntity();
				_parentModel->getTracer()->traceSimulation(this, TraceManager::Level::L8_detailed, "Entity \"" + removedEntity->getName() + "\" was removed from queue \"" + queue->getName() + "\" at rank "+std::to_string(rank));
				_parentModel->sendEntityToComponent(removedEntity, this->getConnections()->getConnectionAtPort(1)); // port 1 is the removed entities output
			} else {
				_parentModel->getTracer()->traceSimulation(this, TraceManager::Level::L8_detailed, "Could not remove entity from queue \"" + queue->getName() + "\" at rank " + std::to_string(rank));
			}
		}
		for (unsigned int rank = startRank; rank < endRank; rank++) {
			waiting = queue->getAtRank(startRank); //always startRank, since when one is removed, the next one take its place
			if (waiting != nullptr) {
				queue->removeElement(waiting);
			}			
		}
	}
	if (_removeFromType == RemoveFromType::ENTITYGROUP) {
		//@TODO
	}
	_parentModel->sendEntityToComponent(entity, this->getConnections()->getFrontConnection());
}

bool Remove::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_removeFromType = static_cast<Remove::RemoveFromType> (fields->loadField("RemoveFromType", static_cast<int> (DEFAULT.removeFromType)));
		_removeStartRank = fields->loadField("RemoveStartRank", DEFAULT.removeStartRank);
		_removeEndRank = fields->loadField("RemoveEndRank", DEFAULT.removeEndRank);
	}
	return res;
}

void Remove::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("RemoveFromType", static_cast<int> (_removeFromType), static_cast<int> (DEFAULT.removeFromType), saveDefaultValues);        
	fields->saveField("RemoveStartRank", _removeStartRank, DEFAULT.removeStartRank, saveDefaultValues);
	fields->saveField("RemoveEndRank", _removeEndRank, DEFAULT.removeEndRank, saveDefaultValues);
}

bool Remove::_check(std::string* errorMessage) {
	bool resultAll = true;
	resultAll = _removeFrom != nullptr;
	if (!resultAll) {
		*errorMessage += "RemoveFrom was not defined.";
	}
	if (resultAll) {
		resultAll &= (_removeFrom->getClassname() == Util::TypeOf<Queue>() && _removeFromType == RemoveFromType::QUEUE) ||
				(_removeFrom->getClassname() == Util::TypeOf<EntityGroup>() && _removeFromType == RemoveFromType::ENTITYGROUP);
		if (!resultAll) {
			*errorMessage += "RemoveFromType differs from what RemoveFrom actually is.";
		}
	}
	return resultAll;
}

void Remove::_createInternalAndAttachedData() {
	PluginManager* plugins = _parentModel->getParentSimulator()->getPlugins();
	if (_removeFromType == Remove::RemoveFromType::QUEUE) {
		if (_removeFrom == nullptr) {
			_removeFrom = plugins->newInstance<Queue>(_parentModel, getName() + ".Queue");
		}
		_attachedDataInsert("Queue", _removeFrom);
	}
	if (_removeFromType == Remove::RemoveFromType::ENTITYGROUP) {
		//@TODO
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


