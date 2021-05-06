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

#include "ElementManager.h"
#include "Model.h"

//using namespace GenesysKernel;

ElementManager::ElementManager(Model* model) {
	_parentModel = model;
	/* \todo: -- Sort methods for elements should be a decorator */
	_elements = new std::map<std::string, List<ModelElement*>*>(); /// Elements are organized as a map from a string (key), the type of an element, and a list of elements of that type
}

bool ElementManager::insert(ModelElement* anElement) {
	std::string elementTypename = anElement->getClassname();
	return insert(elementTypename, anElement);
}

bool ElementManager::insert(std::string elementTypename, ModelElement* anElement) {
	List<ModelElement*>* listElements = getElementList(elementTypename);
	if (listElements->find(anElement) == listElements->list()->end()) { //not found
		listElements->insert(anElement);
		_hasChanged = true;
		this->_parentModel->getTracer()->trace(Util::TraceLevel::L7_detailed, anElement->getClassname() + " \"" + anElement->getName() + "\"" + " successfully inserted.");
		return true;
	} else {
		this->_parentModel->getTracer()->trace(Util::TraceLevel::L7_detailed, anElement->getClassname() + " \"" + anElement->getName() + "\" already exists."); //could not be inserted.");
	}
	return false;
}

void ElementManager::remove(ModelElement* anElement) {
	std::string elementTypename = anElement->getClassname();
	List<ModelElement*>* listElements = getElementList(elementTypename);
	listElements->remove(anElement);
	_hasChanged = true;
	_parentModel->getTracer()->trace(Util::TraceLevel::L3_results, "Element successfully removed");

}

void ElementManager::remove(std::string elementTypename, ModelElement* anElement) {
	List<ModelElement*>* listElements = getElementList(elementTypename);
	_hasChanged = true;
	listElements->remove(anElement);
}

bool ElementManager::check(std::string elementTypename, std::string elementName, std::string expressionName, bool mandatory, std::string* errorMessage) {
	if (elementName == "" && !mandatory) {
		return true;
	}
	bool result = getElement(elementTypename, elementName) != nullptr;
	if (!result) {
		std::string msg = elementTypename + " \"" + elementName + "\" for '" + expressionName + "' is not in the model.";
		errorMessage->append(msg);
	}
	return result;
}

bool ElementManager::check(std::string elementTypename, ModelElement* anElement, std::string expressionName, std::string* errorMessage) {
	bool result = anElement != nullptr;
	if (!result) {
		std::string msg = elementTypename + " for '" + expressionName + "' is null.";
		errorMessage->append(msg);
	} else {
		result = check(elementTypename, anElement->getName(), expressionName, true, errorMessage);
	}
	return result;
}

void ElementManager::clear() {
	_hasChanged = true;
	this->_elements->clear();
}

unsigned int ElementManager::getNumberOfElements(std::string elementTypename) {
	List<ModelElement*>* listElements = getElementList(elementTypename);
	return listElements->size();
}

unsigned int ElementManager::getNumberOfElements() {
	unsigned int total = 0;
	for (std::map<std::string, List<ModelElement*>*>::iterator it = _elements->begin(); it != _elements->end(); it++) {
		total += (*it).second->size();
	}
	return total;
}

void ElementManager::show() {
	_parentModel->getTracer()->trace(Util::TraceLevel::L7_detailed, "Model Elements:");
	//std::map<std::string, List<ModelElement*>*>* _elements;
	std::string key;
	List<ModelElement*>* list;
	Util::IncIndent();
	{
		for (std::map<std::string, List<ModelElement*>*>::iterator infraIt = _elements->begin(); infraIt != _elements->end(); infraIt++) {
			key = (*infraIt).first;
			list = (*infraIt).second;
			_parentModel->getTracer()->trace(Util::TraceLevel::L7_detailed, key + ": (" + std::to_string(list->size()) + ")");
			Util::IncIndent();
			{
				for (std::list<ModelElement*>::iterator it = list->list()->begin(); it != list->list()->end(); it++) {
					_parentModel->getTracer()->trace(Util::TraceLevel::L7_detailed, (*it)->show());
				}
			}
			Util::DecIndent();
		}
	}
	Util::DecIndent();
}

Model* ElementManager::getParentModel() const {
	return _parentModel;
}

bool ElementManager::hasChanged() const {
	if (_hasChanged)
		return _hasChanged;
	else {
		for (std::pair<std::string, List<ModelElement*>*> pair : *_elements) {
			for (ModelElement* element : *pair.second->list()) {
				if (element->hasChanged()) {
					return true;
				}
			}
		}
	}
	return false;
}

void ElementManager::setHasChanged(bool _hasChanged) {
	this->_hasChanged = _hasChanged;
}

List<ModelElement*>* ElementManager::getElementList(std::string elementTypename) const {
	std::map<std::string, List<ModelElement*>*>::iterator it = this->_elements->find(elementTypename);
	if (it == this->_elements->end()) {
		// list does not exists yet. Create it and set a valid iterator
		List<ModelElement*>* newList = new List<ModelElement*>();
		newList->setSortFunc([](const ModelElement* a, const ModelElement * b) {
			return a->getId() < b->getId();
		});
		_elements->insert(std::pair<std::string, List<ModelElement*>*>(elementTypename, newList));
		it = this->_elements->find(elementTypename);
	}
	List<ModelElement*>* infras = it->second;
	return infras;
}

ModelElement* ElementManager::getElement(std::string elementTypename, Util::identification id) {
	List<ModelElement*>* list = getElementList(elementTypename);
	for (std::list<ModelElement*>::iterator it = list->list()->begin(); it != list->list()->end(); it++) {
		if ((*it)->getId() == id) { // found
			return (*it);
		}
	}
	return nullptr;
}

int ElementManager::getRankOf(std::string elementTypename, std::string name) {
	int rank = 0;
	List<ModelElement*>* list = getElementList(elementTypename);
	for (std::list<ModelElement*>::iterator it = list->list()->begin(); it != list->list()->end(); it++) {
		if ((*it)->getName() == name) { // found
			return rank;
		} else {
			rank++;
		}
	}
	return -1;
}

std::list<std::string>* ElementManager::getElementClassnames() const {
	std::list<std::string>* keys = new std::list<std::string>();
	for (std::map<std::string, List<ModelElement*>*>::iterator it = _elements->begin(); it != _elements->end(); it++) {
		keys->insert(keys->end(), (*it).first);
	}
	return keys;
}

ModelElement* ElementManager::getElement(std::string elementTypename, std::string name) {
	List<ModelElement*>* list = getElementList(elementTypename);
	for (std::list<ModelElement*>::iterator it = list->list()->begin(); it != list->list()->end(); it++) {
		if ((*it)->getName() == name) { // found
			return (*it);
		}
	}
	return nullptr;
}