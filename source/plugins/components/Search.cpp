/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Search.cpp
 * Author: rlcancian
 * 
 * Created on 03 de Junho de 2019, 15:20
 */

#include "Search.h"
#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/Simulator.h"
#include "../../kernel/simulator/Attribute.h"
#include "../../plugins/data/EntityGroup.h"
#include "../../plugins/data/Queue.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Search::GetPluginInformation;
}
#endif

ModelDataDefinition* Search::NewInstance(Model* model, std::string name) {
	return new Search(model, name);
}

void Search::setSearchIn(ModelDataDefinition* _searchIn) {
	this->_searchIn = _searchIn;
}

ModelDataDefinition* Search::getSearchIn() const {
	return _searchIn;
}

void Search::setSearchInType(Search::SearchInType _searchInType) {
	this->_searchInType = _searchInType;
}

Search::SearchInType Search::getSearchInType() const {
	return _searchInType;
}

void Search::setSaveFounRankAttribute(std::string _saveFounRankAttribute) {
	this->_saveFounRankAttribute = _saveFounRankAttribute;
}

std::string Search::getSaveFounRankAttribute() const {
	return _saveFounRankAttribute;
}

void Search::setSearchCondition(std::string _searchCondition) {
	this->_searchCondition = _searchCondition;
}

std::string Search::getSearchCondition() const {
	return _searchCondition;
}

void Search::setEndRank(std::string _endRank) {
	this->_endRank = _endRank;
}

std::string Search::getEndRank() const {
	return _endRank;
}

void Search::setStartRank(std::string _startRank) {
	this->_startRank = _startRank;
}

std::string Search::getStartRank() const {
	return _startRank;
}

void Search::setSearchInName(std::string searchInName) {
	ModelDataDefinition* datadef = nullptr;
	if (_searchInType == SearchInType::QUEUE) {
		datadef = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), searchInName);
	} else if (_searchInType == SearchInType::ENTITYGROUP) {
		datadef = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<EntityGroup>(), searchInName);
	}
	if (datadef != nullptr) {
		_searchIn = datadef;
	}
}

std::string Search::getSearchInName() const {
	if (_searchIn != nullptr)
		return _searchIn->getName();
	else
		return "";
}

Search::Search(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Search>(), name) {
}

std::string Search::show() {
	return ModelComponent::show() + "";
}

ModelComponent* Search::LoadInstance(Model* model, PersistenceRecord *fields) {
	Search* newComponent = new Search(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

void Search::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	int startRank = _parentModel->parseExpression(_startRank);
	int endRank = _parentModel->parseExpression(_endRank);
	_parentModel->getTracer()->traceSimulation(this, TraceManager::Level::L7_internal, "Searching for \"" + _searchCondition + "\" in \"" + _searchIn->getName() + "\" from rank " + std::to_string(startRank) + " to " + std::to_string(endRank));
	Entity* searchedEnt;
	bool found = false;
	int i = startRank;
	double value;
	if (_searchInType == SearchInType::QUEUE) {
		Queue* queue = dynamic_cast<Queue*> (_searchIn);
		while (i < queue->size() && i < endRank && !found) {
			searchedEnt = queue->getAtRank(i)->getEntity();
			_parentModel->getSimulation()->getCurrentEvent()->setEntity(searchedEnt); // condition MUST be tested on the entity being searched, so set it as the current entity
			value = _parentModel->parseExpression(_searchCondition);
			_parentModel->getTracer()->traceSimulation(this, TraceManager::Level::L9_mostDetailed, "Searching on entity \"" + searchedEnt->getName() + "\": " + std::to_string(value));
			found = value != 0;
			i++;
		}
		_parentModel->getSimulation()->getCurrentEvent()->setEntity(entity); // set back original entity as the current one
		if (found) {
			i--;
			_parentModel->getTracer()->traceSimulation(this, TraceManager::Level::L8_detailed, "Found entity \"" + searchedEnt->getName() + "\" at rank " + std::to_string(i) + ". Saved on \"" + _saveFounRankAttribute + "\" attribute.");
			entity->setAttributeValue(_saveFounRankAttribute, i);
		}
	} else if (_searchInType == SearchInType::ENTITYGROUP) {
	}
	if (found) {
		_parentModel->sendEntityToComponent(entity, this->getConnections()->getConnectionAtPort(1));
	} else {
		_parentModel->sendEntityToComponent(entity, this->getConnections()->getConnectionAtPort(0));
	}
}

bool Search::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
            _searchInType = static_cast<Search::SearchInType> (fields->loadField("SearchInType", static_cast<int> (DEFAULT.searchInType)));
            _startRank = fields->loadField("StartRank", DEFAULT.startRank);
            _endRank = fields->loadField("EndRank", DEFAULT.endRank);
            _searchCondition = fields->loadField("SearchCondition", DEFAULT.searchCondition);
            _saveFounRankAttribute = fields->loadField("SaveFounRankAttribute", DEFAULT.saveFounRankAttribute);  //se consertar o typo, precisa consertar aqui também
	}
	return res;
}

void Search::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("SearchInType", static_cast<int> (_searchInType), static_cast<int> (DEFAULT.searchInType), saveDefaultValues);
	fields->saveField("StartRank", _startRank, DEFAULT.startRank, saveDefaultValues);
	fields->saveField("EndRank", _endRank, DEFAULT.endRank, saveDefaultValues);
	fields->saveField("SearchCondition", _searchCondition, DEFAULT.searchCondition, saveDefaultValues);
	fields->saveField("SaveFounRankAttribute", _saveFounRankAttribute, DEFAULT.saveFounRankAttribute, saveDefaultValues);  //se consertar o typo, precisa consertar aqui também
        
}

bool Search::_check(std::string* errorMessage) {
	bool resultAll = true;
	bool sucess;
	std::string msg = "";
	resultAll = _searchIn != nullptr;
	if (!resultAll) {
		*errorMessage += "SearchIn was not defined.";
	}
	if (_startRank == "") {
		resultAll = false;
		*errorMessage += "StartRank was not defined.";
	} else {
		_parentModel->parseExpression(_startRank, &sucess, &msg);
		resultAll &= sucess;
		if (!sucess) {
			*errorMessage += msg;
		}
	}
	if (_endRank == "") {
		resultAll = false;
		*errorMessage += "EndRank was not defined.";
	} else {
		_parentModel->parseExpression(_endRank, &sucess, &msg);
		resultAll &= sucess;
		if (!sucess) {
			*errorMessage += msg;
		}
	}
	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Attribute>(), _saveFounRankAttribute, "Save Found Rank Attribute", true, errorMessage);
	if (resultAll) {
		resultAll &= (_searchIn->getClassname() == Util::TypeOf<Queue>() && _searchInType == SearchInType::QUEUE) ||
				(_searchIn->getClassname() == Util::TypeOf<EntityGroup>() && _searchInType == SearchInType::ENTITYGROUP);
		if (!resultAll) {
			*errorMessage += "SearchInType differs from what SearchIn actually is.";
		}
	}
	return resultAll;
}

void Search::_createInternalAndAttachedData() {
	if (_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Attribute>(), _saveFounRankAttribute) == nullptr) {
		this->_attachedAttributesInsert({_saveFounRankAttribute});
	}
	PluginManager* plugins = _parentModel->getParentSimulator()->getPlugins();
	if (_searchInType == Search::SearchInType::QUEUE) {
		if (_searchIn == nullptr) {
			_searchIn = plugins->newInstance<Queue>(_parentModel, getName() + ".Queue");
		}
		_attachedDataInsert("Queue", _searchIn); // @TODO: Check internal and attached and shared queues
	}
	if (_searchInType == Search::SearchInType::ENTITYGROUP) {
		//@TODO
	}
}

PluginInformation* Search::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Search>(), &Search::LoadInstance, &Search::NewInstance);
	info->setCategory("Decisions");
	info->insertDynamicLibFileDependence("queue.so");
	info->insertDynamicLibFileDependence("entitygroup.so");
	info->setMinimumOutputs(2);
	info->setMaximumOutputs(2);

	// ...  @TODO
	return info;
}


