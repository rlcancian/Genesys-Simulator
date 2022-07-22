/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Group.cpp
 * Author: rlcancian
 *
 * Created on 12 de Junho de 2019, 19:00
 */

#include "EntityGroup.h"
#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/Attribute.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &EntityGroup::GetPluginInformation;
}
#endif

ModelDataDefinition* EntityGroup::NewInstance(Model* model, std::string name) {
	return new EntityGroup(model, name);
}

EntityGroup::EntityGroup(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<EntityGroup>(), name) {
	// it is invoked in the constructor since EntityGroups are creted runtime by Components such as Batch
	this->_createInternalAndAttachedData();
}

EntityGroup::~EntityGroup() {
	//_parentModel->elements()->remove(Util::TypeOf<StatisticsCollector>(), _cstatNumberInGroup);
}

std::string EntityGroup::show() {
	return ModelDataDefinition::show(); // +
	// TODO: Sow every group in the map",entities=" + this->_list->show();
}

void EntityGroup::insertElement(unsigned int idKey, Entity* modeldatum) {
	std::map<unsigned int, List<Entity*>*>::iterator it = _groupMap->find(idKey);
	while (it == _groupMap->end()) {
		_groupMap->insert({idKey, new List<Entity*>()});
		it = _groupMap->find(idKey);
	}
	(*it).second->insert(modeldatum);
	_cstatNumberInGroup->getStatistics()->getCollector()->addValue((*it).second->size());
}

void EntityGroup::removeElement(unsigned int idKey, Entity * modeldatum) {
	std::map<unsigned int, List<Entity*>*>::iterator it = _groupMap->find(idKey);
	if (it != _groupMap->end()) {
		(*it).second->remove(modeldatum);
		_cstatNumberInGroup->getStatistics()->getCollector()->addValue((*it).second->size());
	}
}

List<Entity*>* EntityGroup::getGroup(unsigned int idKey) {
	std::map<unsigned int, List<Entity*>*>::iterator it = _groupMap->find(idKey);
	if (it == _groupMap->end()) {
		return new List<Entity*>(); // not found
	} else {
		return (*it).second;
	}
}

bool EntityGroup::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		try {
		} catch (...) {
		}
	}
	return res;
}

void EntityGroup::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
}

PluginInformation * EntityGroup::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<EntityGroup>(), &EntityGroup::LoadInstance, &EntityGroup::NewInstance);
	std::string text = "Represent entities grouped by an 'Entity.Group' attribute.";
	text += "An EntityGroup my contain several groups indexed by that attribute.";
	text += "Grouped entitties may be separated by a 'Separate' like coponent.";
	info->setDescriptionHelp(text);
	return info;
}

ModelDataDefinition * EntityGroup::LoadInstance(Model* model, PersistenceRecord *fields) {
	EntityGroup* newElement = new EntityGroup(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

void EntityGroup::_createInternalAndAttachedData() {
	this->_attachedAttributesInsert({"Entity.Group"});
	if (_reportStatistics) {
		if (_cstatNumberInGroup == nullptr) {
			_cstatNumberInGroup = new StatisticsCollector(_parentModel, "NumberInGroup", this);
			_internalDataInsert("NumberInGroup", _cstatNumberInGroup);
		}
	} else
		if (_cstatNumberInGroup != nullptr) {
		_internalDataClear();
	}
}

bool EntityGroup::_check(std::string * errorMessage) {
	*errorMessage += "";
	return true;
}