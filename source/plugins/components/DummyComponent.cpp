/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Dummy.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 22 de Maio de 2019, 18:41
 */

#include "DummyComponent.h"
#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/Simulator.h"
#include "../../kernel/simulator/PluginManager.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &DummyComponent::GetPluginInformation;
}
#endif

ModelDataDefinition* DummyComponent::NewInstance(Model* model, std::string name) {
	return new DummyComponent(model, name);
}

DummyComponent::DummyComponent(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<DummyComponent>(), name) {
}

std::string DummyComponent::show() {
	return ModelComponent::show() + "";
}

// public static 

ModelComponent* DummyComponent::LoadInstance(Model* model, PersistenceRecord *fields) {
	DummyComponent* newComponent = new DummyComponent(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

PluginInformation* DummyComponent::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<DummyComponent>(), &DummyComponent::LoadInstance, &DummyComponent::NewInstance);
	info->setDescriptionHelp("//@TODO");
	return info;
}

// protected virtual -- must be overriden

void DummyComponent::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	_parentModel->getTracer()->trace("I'm just a dummy model and I'll just send the entity forward");
	this->_parentModel->sendEntityToComponent(entity, this->getConnections()->getFrontConnection());
}

bool DummyComponent::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		// @TODO: not implemented yet
	}
	return res;
}

void DummyComponent::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	// @TODO: not implemented yet
}


// protected virtual -- could be overriden 

//ParserChangesInformation* DummyElement::_getParserChangesInformation() {}

bool DummyComponent::_check(std::string* errorMessage) {
	bool resultAll = true;
	resultAll &= _someString != "";
	resultAll &= _someUint > 0;
	return resultAll;
}

void DummyComponent::_initBetweenReplications() {
	_someString = "Test";
	_someUint = 1;
}

void DummyComponent::_createInternalAndAttachedData() {
	if (_internalDataDefinition == nullptr) {
		PluginManager* pm = _parentModel->getParentSimulator()->getPlugins();
		_internalDataDefinition = pm->newInstance<DummyElement>(_parentModel, getName() + "." + "JustaDummy");
		_internalDataInsert("JustaDummy", _internalDataDefinition);
	}
}