/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ElementManager.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 7 de Novembro de 2018, 12:48
 */

#include "ModelDataManager.h"
#include "Model.h"

//using namespace GenesysKernel;

ModelDataManager::ModelDataManager(Model* model) {
	_parentModel = model;
	// Elements are organized as a map from a string (key), the type of an modeldatum, and a list of elements of that type
	_datadefinitions = new std::map<std::string, List<ModelDataDefinition*>*>();
}

// Releases per-type list containers and the map container itself; Model owns pointed data objects lifecycle.
ModelDataManager::~ModelDataManager() {
	if (_datadefinitions == nullptr) {
		return;
	}
	for (auto& pair : *_datadefinitions) {
		delete pair.second;
	}
	delete _datadefinitions;
	_datadefinitions = nullptr;
}

bool ModelDataManager::insert(ModelDataDefinition* anElement) {
	std::string datadefinitionTypename = anElement->getClassname();
	bool result = insert(datadefinitionTypename, anElement);
	//	ModelDataDefinition::CreateInternalData(anElement);
	//}
	return result;
}

bool ModelDataManager::insert(std::string datadefinitionTypename, ModelDataDefinition* anElement) {
	List<ModelDataDefinition*>* listElements = getDataDefinitionList(datadefinitionTypename);
	std::string text = "";
	bool result = false;
	if (listElements->find(anElement) == listElements->list()->end()) {
		//not found
		listElements->insert(anElement);
		_hasChanged = true;
		text = anElement->getClassname() + " \"" + anElement->getName() + "\"" + " successfully inserted.";
		result = true;
		//	} else {
		//		text = anElement->getClassname() + " \"" + anElement->getName() + "\" already exists.";
	}
	if (result) {
		if (_parentModel->getSimulation()->isRunning()) {
			_parentModel->getTracer()->traceSimulation(this, TraceManager::Level::L8_detailed, text);
		}
		else {
			_parentModel->getTracer()->trace(text);
		}
	}
	return result;
}

void ModelDataManager::remove(ModelDataDefinition* anElement) {
	std::string datadefinitionTypename = anElement->getClassname();
	List<ModelDataDefinition*>* listElements = getDataDefinitionList(datadefinitionTypename);
	listElements->remove(anElement);
	_hasChanged = true;
	////trace("Element successfully removed.");
}

void ModelDataManager::remove(std::string datadefinitionTypename, ModelDataDefinition* anElement) {
	List<ModelDataDefinition*>* listElements = getDataDefinitionList(datadefinitionTypename);
	_hasChanged = true;
	listElements->remove(anElement);
}

bool ModelDataManager::check(std::string datadefinitionTypename, std::string elementName, std::string expressionName,
                             bool mandatory, std::string& errorMessage) {
	if (elementName == "" && !mandatory) {
		return true;
	}
	bool result = getDataDefinition(datadefinitionTypename, elementName) != nullptr;
	if (!result) {
		std::string msg = datadefinitionTypename + " \"" + elementName + "\" for '" + expressionName +
			"' is not in the model.";
		errorMessage.append(msg);
	}
	return result;
}

bool ModelDataManager::check(std::string datadefinitionTypename, ModelDataDefinition* anElement,
                             std::string expressionName, std::string& errorMessage) {
	bool result = anElement != nullptr;
	if (!result) {
		std::string msg = datadefinitionTypename + " for '" + expressionName + "' is null.";
		errorMessage.append(msg);
	}
	else {
		result = check(datadefinitionTypename, anElement->getName(), expressionName, true, errorMessage);
	}
	return result;
}

void ModelDataManager::clear() {
	_hasChanged = true;
	// Deletes list containers before clearing the map to avoid leaking heap-allocated per-type lists.
	for (auto& pair : *this->_datadefinitions) {
		delete pair.second;
	}
	this->_datadefinitions->clear();
}

unsigned int ModelDataManager::getNumberOfDataDefinitions(std::string datadefinitionTypename) {
	List<ModelDataDefinition*>* listElements = getDataDefinitionList(datadefinitionTypename);
	return listElements->size();
}

unsigned int ModelDataManager::getNumberOfDataDefinitions() {
	unsigned int total = 0;
	for (std::map<std::string, List<ModelDataDefinition*>*>::iterator it = _datadefinitions->begin(); it !=
	     _datadefinitions->end(); it++) {
		total += (*it).second->size();
	}
	return total;
}

void ModelDataManager::show() {
	_parentModel->getTracer()->trace("Model Data Definitions:");
	//std::map<std::string, List<ModelDataDefinition*>*>* _datadefinitions;
	std::string key;
	List<ModelDataDefinition*>* list;
	Util::IncIndent();
	{
		for (const auto& [typenameKey, definitions] : *_datadefinitions) {
			key = typenameKey;
			list = definitions;
			_parentModel->getTracer()->trace(key + ": (" + std::to_string(list->size()) + ")");
			Util::IncIndent();
			{
				for (ModelDataDefinition* modeldatum : *list->list()) {
					_parentModel->getTracer()->trace(modeldatum->show());
				}
			}
			Util::DecIndent();
		}
	}
	Util::DecIndent();
}

Model* ModelDataManager::getParentModel() const {
	return _parentModel;
}

bool ModelDataManager::hasChanged() const {
	if (_hasChanged)
		return _hasChanged;
	else {
		for (std::pair<std::string, List<ModelDataDefinition*>*> pair : *_datadefinitions) {
			for (ModelDataDefinition* modeldatum : *pair.second->list()) {
				if (modeldatum->hasChanged()) {
					return true;
				}
			}
		}
	}
	return false;
}

void ModelDataManager::setHasChanged(bool _hasChanged) {
	this->_hasChanged = _hasChanged;
}

List<ModelDataDefinition*>* ModelDataManager::getDataDefinitionList(std::string datadefinitionTypename) const {
	bool found = (_datadefinitions->find(datadefinitionTypename) != _datadefinitions->end());
	auto it = _datadefinitions->find(datadefinitionTypename);
	if (not found) {
		// list does not exists yet. Create it and set a valid iterator
		List<ModelDataDefinition*>* newList = new List<ModelDataDefinition*>();
		newList->setSortFunc([](const ModelDataDefinition* a, const ModelDataDefinition* b) {
			return a->getId() < b->getId();
		});
		_datadefinitions->insert(std::pair<std::string, List<ModelDataDefinition*>*>(datadefinitionTypename, newList));
		it = this->_datadefinitions->find(datadefinitionTypename);
	}
	List<ModelDataDefinition*>* infras = it->second;
	return infras;
}

ModelDataDefinition* ModelDataManager::getDataDefinition(std::string datadefinitionTypename, Util::identification id) {
	List<ModelDataDefinition*>* list = getDataDefinitionList(datadefinitionTypename);
	for (ModelDataDefinition* modeldatum : *list->list()) {
		if (modeldatum->getId() == id) {
			// found
			return modeldatum;
		}
	}
	return nullptr;
}

int ModelDataManager::getRankOf(std::string datadefinitionTypename, std::string name) {
	int rank = 0;
	List<ModelDataDefinition*>* list = getDataDefinitionList(datadefinitionTypename);
	for (ModelDataDefinition* modeldatum : *list->list()) {
		if (modeldatum->getName() == name) {
			// found
			return rank;
		}
		else {
			rank++;
		}
	}
	return -1;
}

std::list<std::string> ModelDataManager::getDataDefinitionClassnames() const {
	// Build and return a value snapshot with current class names, avoiding manual ownership by callers.
	std::list<std::string> keys;
	for (const auto& pair : *_datadefinitions) {
		keys.push_back(pair.first);
	}
	return keys;
}

ModelDataDefinition* ModelDataManager::getDataDefinition(std::string datadefinitionTypename, std::string name) {
	List<ModelDataDefinition*>* list = getDataDefinitionList(datadefinitionTypename);
	for (ModelDataDefinition* dd : *list->list()) {
		if (dd->getName() == name) {
			return dd;
		}
	}
	return nullptr;
}
