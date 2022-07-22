/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
/*
 * File:   ModelComponent.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 15:56
 */

#include "ModelComponent.h"
#include "Model.h"
#include "../TraitsKernel.h"

//using namespace GenesysKernel;

ModelComponent::ModelComponent(Model* model, std::string componentTypename, std::string name) : ModelDataDefinition(model, componentTypename, name, false) {
	model->getComponents()->insert(this);
	//PropertySetterString* prop1 = new PropertySetterString(Util::TypeOf<ModelComponent>(), "description",
	//		DefineGetterString<ModelComponent>(this, &ModelComponent::getDescription),
	//		DefineSetterString<ModelComponent>(this, &ModelComponent::setDescription));
	PropertyT<std::string>* prop1_ = new PropertyT<std::string>(Util::TypeOf<ModelComponent>(), "Description",
			DefineGetter<ModelComponent, std::string>(this, &ModelComponent::getDescription),
			DefineSetter<ModelComponent, std::string>(this, &ModelComponent::setDescription));
	_addProperty(prop1_);

}

ModelComponent::~ModelComponent() {
	_parentModel->getComponents()->remove(this);
}

void ModelComponent::DispatchEvent(Event* event) {
	Entity* entity = event->getEntity();
	ModelComponent* component = event->getComponent();
	unsigned int inputPortNumber = event->getComponentinputPortNumber();
	std::string msg = /*"Entity " +std::to_string(entity->entityNumber())*/ entity->getName() + " has arrived at component \"" + component->getName() + "\"";
	if (component->getDescription() != "")
		msg += ": " + component->getDescription();
	if (inputPortNumber > 0)
		msg += " by input " + std::to_string(inputPortNumber);
	component->_parentModel->getTracer()->traceSimulation(component, TraceManager::Level::L7_internal, msg); //:L6_arrival
	Util::IncIndent();
	try {
		component->_onDispatchEvent(entity, inputPortNumber);
	} catch (const std::exception& e) {
		component->_parentModel->getTracer()->traceError(e, "Error executing component " + component->show());
	}
	Util::DecIndent();
}

void ModelComponent::CreateInternalData(ModelComponent* component) {
	//component->_model->getTraceManager()->trace(TraceManager::Level::blockArrival, "Writing component \"" + component->_name + "\""); //std::to_string(component->_id));
	try {
		component->_createInternalAndAttachedData();
	} catch (const std::exception& e) {
		component->_parentModel->getTracer()->traceError(e, "Error creating elements of component " + component->show());
	};
}

void ModelComponent::SaveInstance(PersistenceRecord *fields, ModelComponent* component) {
	component->_parentModel->getTracer()->trace(TraceManager::Level::L9_mostDetailed, "Writing component \"" + component->getName() + "\"");
	try {
		component->_saveInstance(fields, component->_getSaveDefaultsOption());
	} catch (const std::exception& e) {
		component->_parentModel->getTracer()->traceError(e, "Error executing component " + component->show());
	}
}

void ModelComponent::setDescription(std::string description) {
	if (_description != description) {
		this->_description = description;
		_hasChanged = true;
	}
}

std::string ModelComponent::getDescription() const {
	return _description;
}

bool ModelComponent::Check(ModelComponent* component) {
	component->_parentModel->getTracer()->trace(TraceManager::Level::L8_detailed, "Checking " + component->_typename + ": \"" + component->getName() + "\""); //std::to_string(component->_id));
	bool res = false;
	std::string* errorMessage = new std::string();
	Util::IncIndent();
	{
		try {
			res = component->_check(errorMessage);
			if (!res) {
				component->_parentModel->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "Error: Checking has failed with message '" + *errorMessage + "'");
			}
		} catch (const std::exception& e) {
			component->_parentModel->getTracer()->traceError(e, "Error verifying component " + component->show());
		}
	}
	Util::DecIndent();
	return res;
}

ConnectionManager* ModelComponent::getConnections() const {
	return _connections; // @TODO How to know if it changes?
}

bool ModelComponent::hasBreakpointAt() {
	return _parentModel->getSimulation()->getBreakpointsOnComponent()->find(this) != _parentModel->getSimulation()->getBreakpointsOnComponent()->list()->end();
}

std::string ModelComponent::show() {
	return ModelDataDefinition::show(); // "{id=" + std::to_string(this->_id) + ",name=\""+this->_name + "\"}"; // , nextComponents[]=(" + _nextComponents->show() + ")}";
}

bool ModelComponent::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_description = fields->loadField("caption", DEFAULT.description);
		// Now it should load nextComponents. The problem is that the nextComponent may not be loaded yet.
	}
	return res;
}

void ModelComponent::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("caption", _description, DEFAULT.description, saveDefaultValues);
	if (true) {//(_connections->size() != 1) { // save nextSize only if it is != 1
		fields->saveField("nexts", _connections->size(), DEFAULT.nextSize, saveDefaultValues);
	}
	unsigned short i;
	for (std::pair<unsigned int, Connection*> connectionPair : *_connections->connections()) {
		i = connectionPair.first; // output port
		if (_connections->connections()->size() == 1 && i == 0) {
			fields->saveField("nextId", connectionPair.second->component->_id, 0, saveDefaultValues);
		} else {
			fields->saveField("nextId" + strIndex(i), connectionPair.second->component->_id, 0, saveDefaultValues);
		}
		if (connectionPair.second->port != 0) {//((*it)->second != 0) { // save nextinputPortNumber only if it is != 0
			fields->saveField("nextinputPortNumber" + strIndex(i), connectionPair.second->port, DEFAULT.nextinputPortNumber, saveDefaultValues);
		}
	}
}
