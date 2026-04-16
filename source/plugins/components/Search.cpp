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
#include "../../kernel/simulator/Event.h"
#include "../../plugins/data/EntityGroup.h"
#include "../../plugins/data/Queue.h"
#include "../../kernel/simulator/SimulationControlAndResponse.h"
#include <algorithm>
#include <cstdlib>

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

std::string Search::convertEnumToStr(SearchInType type) {
	switch (static_cast<int> (type)) {
		case 0: return "QUEUE";
		case 1: return "ENTITYGROUP";
	}
	return "Unknown";
}

Search::Search(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Search>(), name) {
	SimulationControlGeneric<std::string>* propStart = new SimulationControlGeneric<std::string>(
									std::bind(&Search::getStartRank, this), std::bind(&Search::setStartRank, this, std::placeholders::_1),
									Util::TypeOf<Search>(), getName(), "StartRank", "");
	SimulationControlGeneric<std::string>* propEnd = new SimulationControlGeneric<std::string>(
									std::bind(&Search::getEndRank, this), std::bind(&Search::setEndRank, this, std::placeholders::_1),
									Util::TypeOf<Search>(), getName(), "EndRank", "");
	SimulationControlGeneric<std::string>* propCondition = new SimulationControlGeneric<std::string>(
									std::bind(&Search::getSearchCondition, this), std::bind(&Search::setSearchCondition, this, std::placeholders::_1),
									Util::TypeOf<Search>(), getName(), "SearchCondition", "");
	SimulationControlGeneric<std::string>* propSaveAttribute = new SimulationControlGeneric<std::string>(
									std::bind(&Search::getSaveFounRankAttribute, this), std::bind(&Search::setSaveFounRankAttribute, this, std::placeholders::_1),
									Util::TypeOf<Search>(), getName(), "SaveFounRankAttribute", "");
	SimulationControlGeneric<std::string>* propSearchInName = new SimulationControlGeneric<std::string>(
									std::bind(&Search::getSearchInName, this), std::bind(&Search::setSearchInName, this, std::placeholders::_1),
									Util::TypeOf<Search>(), getName(), "SearchInName", "");
	SimulationControlGenericClass<ModelDataDefinition*, Model*, Queue>* propSearchIn = new SimulationControlGenericClass<ModelDataDefinition*, Model*, Queue>(
									_parentModel,
									std::bind(&Search::getSearchIn, this),
									[this](ModelDataDefinition* searchIn) {
										this->setSearchInType(Search::SearchInType::QUEUE);
										this->setSearchIn(searchIn);
									},
									Util::TypeOf<Search>(), getName(), "SearchIn", "");
    SimulationControlGenericEnum<Search::SearchInType, Search>* propSearchInType = new SimulationControlGenericEnum<Search::SearchInType, Search>(
                                    std::bind(&Search::getSearchInType, this), std::bind(&Search::setSearchInType, this, std::placeholders::_1),
                                    Util::TypeOf<Search>(), getName(), "SearchInType", "");


	_parentModel->getControls()->insert(propStart);								
	_parentModel->getControls()->insert(propEnd);
	_parentModel->getControls()->insert(propCondition);
	_parentModel->getControls()->insert(propSaveAttribute);
	_parentModel->getControls()->insert(propSearchInName);
	_parentModel->getControls()->insert(propSearchIn);
    _parentModel->getControls()->insert(propSearchInType);

	// setting properties
	_addProperty(propStart);
	_addProperty(propEnd);
	_addProperty(propCondition);
	_addProperty(propSaveAttribute);
	_addProperty(propSearchInName);
	_addProperty(propSearchIn);
    _addProperty(propSearchInType);
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
	int startRank = parseNumericOrExpression(_startRank);
	int endRank = parseNumericOrExpression(_endRank);
	traceSimulation(this, TraceManager::Level::L7_internal, "Searching for \"" + _searchCondition + "\" in \"" + _searchIn->getName() + "\" from rank " + std::to_string(startRank) + " to " + std::to_string(endRank));
	Entity* searchedEnt;
	bool found = false;
	int foundRank = 0;
	double value;
	if (_searchInType == SearchInType::QUEUE) {
		Queue* queue = dynamic_cast<Queue*> (_searchIn);
		const int lowerRank = std::min(startRank, endRank);
		const int upperRank = std::max(startRank, endRank);
		for (int rank = lowerRank; rank <= upperRank && rank < static_cast<int>(queue->size()) && !found; rank++) {
			if (rank < 0) {
				continue;
			}
			Waiting* waiting = queue->getAtRank(static_cast<unsigned int> (rank));
			if (waiting == nullptr) {
				continue;
			}
			searchedEnt = waiting->getEntity();
			Event* currentEvent = _parentModel->getSimulation()->getCurrentEvent();
			if (currentEvent != nullptr) {
				currentEvent->setEntity(searchedEnt); // condition MUST be tested on the entity being searched, so set it as the current entity
			}
			char* parseEndCondition = nullptr;
			value = std::strtod(_searchCondition.c_str(), &parseEndCondition);
			const bool isNumericConstant = parseEndCondition != _searchCondition.c_str() && *parseEndCondition == '\0';
			if (!isNumericConstant) {
				value = _parentModel->parseExpression(_searchCondition);
			}
			traceSimulation(this, TraceManager::Level::L9_mostDetailed, "Searching on entity \"" + searchedEnt->getName() + "\": " + std::to_string(value));
			found = value != 0;
			if (found) {
				foundRank = rank;
			}
		}
		Event* currentEvent = _parentModel->getSimulation()->getCurrentEvent();
		if (currentEvent != nullptr) {
			currentEvent->setEntity(entity); // set back original entity as the current one
		}
	} else if (_searchInType == SearchInType::ENTITYGROUP) {
		traceSimulation(this, TraceManager::Level::L8_detailed, "SearchInType ENTITYGROUP is not supported in this implementation batch.");
	}
	if (found) {
		traceSimulation(this, TraceManager::Level::L8_detailed, "Found entity \"" + searchedEnt->getName() + "\" at rank " + std::to_string(foundRank) + ". Saved on \"" + _saveFounRankAttribute + "\" attribute.");
		entity->setAttributeValue(_saveFounRankAttribute, foundRank);
		_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getConnectionAtPort(1));
	} else {
		entity->setAttributeValue(_saveFounRankAttribute, 0.0);
		_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getConnectionAtPort(0));
	}
}

bool Search::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_searchInType = static_cast<SearchInType>(fields->loadField("searchInType", static_cast<int>(DEFAULT.searchInType)));
		_startRank = fields->loadField("startRank", DEFAULT.startRank);
		_endRank = fields->loadField("endRank", DEFAULT.endRank);
		_searchCondition = fields->loadField("searchCondition", DEFAULT.searchCondition);
		_saveFounRankAttribute = fields->loadField("saveFounRankAttribute", DEFAULT.saveFounRankAttribute);
		const std::string searchInName = fields->loadField("searchIn", "");
		if (searchInName != "") {
			if (_searchInType == SearchInType::QUEUE) {
				_searchIn = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), searchInName);
			} else if (_searchInType == SearchInType::ENTITYGROUP) {
				_searchIn = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<EntityGroup>(), searchInName);
			}
		}
	}
	return res;
}

void Search::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("searchInType", static_cast<int>(_searchInType), static_cast<int>(DEFAULT.searchInType), saveDefaultValues);
	fields->saveField("startRank", _startRank, DEFAULT.startRank, saveDefaultValues);
	fields->saveField("endRank", _endRank, DEFAULT.endRank, saveDefaultValues);
	fields->saveField("searchCondition", _searchCondition, DEFAULT.searchCondition, saveDefaultValues);
	fields->saveField("saveFounRankAttribute", _saveFounRankAttribute, DEFAULT.saveFounRankAttribute, saveDefaultValues);
	if (_searchIn != nullptr) {
		fields->saveField("searchIn", _searchIn->getName(), "", saveDefaultValues);
	}
}

bool Search::_check(std::string& errorMessage) {
	bool resultAll = true;
	bool sucess;
	std::string msg = "";
	resultAll = _searchIn != nullptr;
	if (!resultAll) {
		errorMessage += "SearchIn was not defined.";
	}
	if (_startRank == "") {
		resultAll = false;
		errorMessage += "StartRank was not defined.";
	} else {
        _parentModel->parseExpression(_startRank, sucess, msg);
		resultAll &= sucess;
		if (!sucess) {
			errorMessage += msg;
		}
	}
	if (_endRank == "") {
		resultAll = false;
		errorMessage += "EndRank was not defined.";
	} else {
        _parentModel->parseExpression(_endRank, sucess, msg);
		resultAll &= sucess;
		if (!sucess) {
			errorMessage += msg;
		}
	}
	if (_searchCondition == "") {
		resultAll = false;
		errorMessage += "SearchCondition was not defined.";
	} else {
		_parentModel->parseExpression(_searchCondition, sucess, msg);
		resultAll &= sucess;
		if (!sucess) {
			errorMessage += msg;
		}
	}
	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Attribute>(), _saveFounRankAttribute, "Save Found Rank Attribute", true, errorMessage);
	if (resultAll) {
		if (_searchInType == SearchInType::ENTITYGROUP) {
			resultAll = false;
			errorMessage += "SearchInType ENTITYGROUP is not supported in this implementation batch.";
			return resultAll;
		}
		resultAll &= (_searchIn->getClassname() == Util::TypeOf<Queue>() && _searchInType == SearchInType::QUEUE) ||
				(_searchIn->getClassname() == Util::TypeOf<EntityGroup>() && _searchInType == SearchInType::ENTITYGROUP);
		if (!resultAll) {
			errorMessage += "SearchInType differs from what SearchIn actually is.";
		}
	}
	return resultAll;
}

void Search::_createInternalAndAttachedData() {
	if (_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Attribute>(), _saveFounRankAttribute) == nullptr) {
		this->_attachedAttributesInsert({_saveFounRankAttribute});
	}
	PluginManager* plugins = _parentModel->getParentSimulator()->getPluginManager();
	if (_searchInType == Search::SearchInType::QUEUE) {
		if (_searchIn == nullptr) {
			_searchIn = plugins->newInstance<Queue>(_parentModel, getName() + ".Queue");
			if (_searchIn == nullptr) {
				_searchIn = new Queue(_parentModel, getName() + ".Queue");
			}
		}
		if (_searchIn != nullptr) {
			_attachedDataInsert("Queue", _searchIn); // @TODO: Check internal and attached and shared queues
		} else {
			_attachedDataRemove("Queue");
		}
	}
	if (_searchInType == Search::SearchInType::ENTITYGROUP) {
		// Not supported in this implementation batch. Explicitly validated in _check().
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
