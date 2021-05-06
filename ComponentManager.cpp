/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ComponentManager.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 28 de Maio de 2019, 10:41
 */

#include "ComponentManager.h"
#include "List.h"
#include "Model.h"

//using namespace GenesysKernel;

ComponentManager::ComponentManager(Model* model) {
	_parentModel = model;
	_components = new List<ModelComponent*>();
	_components->setSortFunc([](const ModelComponent* a, const ModelComponent * b) {
		return a->getId() < b->getId(); /// Components are sorted by ID
	});
}

bool ComponentManager::insert(ModelComponent* comp) {
	if (_components->find(comp) == _components->list()->end()) {
		_components->insert(comp);
		_parentModel->getTracer()->trace(Util::TraceLevel::L3_results, "Component \"" + comp->getName() + "\" successfully inserted");
		_hasChanged = true;
		return true;
	}
	_parentModel->getTracer()->trace(Util::TraceLevel::L3_results, "Component \"" + comp->getName() + "\" could not be inserted");
	return false;
}

ModelComponent* ComponentManager::find(std::string name) {
	for (ModelComponent* component : *_components->list()) {
		if (component->getName() == name) {
			return component;
		}
	}
	return nullptr;
}

void ComponentManager::remove(ModelComponent* comp) {
	_components->remove(comp);
	_parentModel->getTracer()->trace(Util::TraceLevel::L3_results, "Component \"" + comp->getName() + "\" successfully removed");
	_hasChanged = true;
}

void ComponentManager::clear() {
	this->_components->clear();
	_hasChanged = true;
}

//ModelComponent* ComponentManager::getComponent(Util::identification id) {
//}

//ModelComponent* ComponentManager::getComponent(std::string name) {
//}

unsigned int ComponentManager::getNumberOfComponents() {
	return _components->size();
}

std::list<ModelComponent*>::iterator ComponentManager::begin() {
	return _components->list()->begin();
}

std::list<ModelComponent*>::iterator ComponentManager::end() {
	return _components->list()->end();
}

ModelComponent* ComponentManager::front() {
	return _components->front();
}

ModelComponent* ComponentManager::next() {
	return _components->next();
}

bool ComponentManager::hasChanged() const {
	if (_hasChanged)
		return _hasChanged;
	for (ModelComponent* component : *_components->list()) {
		if (component->hasChanged()) {
			//_hasChanged = true;
			return true;
		}
	}
	return false;
}

void ComponentManager::setHasChanged(bool _hasChanged) {
	this->_hasChanged = _hasChanged;
}