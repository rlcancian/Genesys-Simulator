/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Separate.cpp
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:14
 */

#include "plugins/components/Grouping/Separate.h"

#include "kernel/simulator/Model.h"
#include "kernel/simulator/Attribute.h"
#include "../../data/Grouping/EntityGroup.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Separate::GetPluginInformation;
}
#endif

ModelDataDefinition* Separate::NewInstance(Model* model, std::string name) {
	return new Separate(model, name);
}

Separate::Separate(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Separate>(), name) {
}

std::string Separate::show() {
	return ModelComponent::show() + "";
}

ModelComponent* Separate::LoadInstance(Model* model, PersistenceRecord *fields) {
	Separate* newComponent = new Separate(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

void Separate::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	// This path handles split-existing-batch semantics using the representative "Entity.Group" marker.
	unsigned int entityGroupId = entity->getAttributeValue("Entity.Group"); //This attribute refers to the Batch internal modeldatum EntityGroup (which may contain several groups --map--
	if (entityGroupId == 0) {
		traceSimulation(this, TraceManager::Level::L7_internal, "Entity is not grouped. Nothing to do");
		this->_parentModel->sendEntityToComponent(entity, getConnectionManager()->getFrontConnection());
	} else {
		EntityGroup* entityGroup = dynamic_cast<EntityGroup*> (_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<EntityGroup>(), entityGroupId));
		if (entityGroup == nullptr) {
			traceError("Error: Could not find EntityGroup Id=" + std::to_string(entityGroupId), TraceManager::Level::L3_errorRecover);
			this->_parentModel->sendEntityToComponent(entity, getConnectionManager()->getFrontConnection());
		} else {
			unsigned int idGroupKey = entity->getId();
			List<Entity*>* members = entityGroup->getGroup(idGroupKey);
			if (members->size() == 0) {
				traceSimulation(this, TraceManager::Level::L7_internal, "Group key has no members. Representative entity will continue unchanged");
				this->_parentModel->sendEntityToComponent(entity, getConnectionManager()->getFrontConnection());
				return;
			}
			while (members->size() > 0) {
				Entity* e = members->front();
				entityGroup->removeElement(idGroupKey, e);
				// Reset group marker so separated members do not keep stale grouping state.
				e->setAttributeValue("Entity.Group", 0.0);
				traceSimulation(this, TraceManager::Level::L7_internal, "Entity " + e ->getName() + " was separated out of the group " + std::to_string(entityGroupId) + " key=" + std::to_string(idGroupKey));
				_parentModel->sendEntityToComponent(e, _connections->getFrontConnection());
			}
			// Remove temporary representative after all members are released.
			_parentModel->removeEntity(entity);
		}
	}
}

bool Separate::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	return res;
}

void Separate::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
}

bool Separate::_check(std::string& errorMessage) {
	// Require the special marker attribute used by split-existing-batch flow.
	bool resultAll = true;
	_attachedAttributesInsert({"Entity.Group"});
	ModelDataManager* elements = _parentModel->getDataManager();
	resultAll &= elements->check(Util::TypeOf<Attribute>(), "Entity.Group", "Entity.Group", true, errorMessage);
	return resultAll;
}

PluginInformation* Separate::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Separate>(), &Separate::LoadInstance, &Separate::NewInstance);
	info->setCategory("Grouping");
	// ...
	return info;
}
