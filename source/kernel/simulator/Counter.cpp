/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   CounterDefaultImpl1.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 29 de Maio de 2019, 11:24
 */

#include "Counter.h"
#include "Model.h"

//using namespace GenesysKernel;

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Counter::GetPluginInformation;
}
#endif

ModelDataDefinition* Counter::NewInstance(Model* model, std::string name) {
	return new Counter(model, name);
}

Counter::Counter(Model* model, std::string name, ModelDataDefinition* parent) : ModelDataDefinition(model, Util::TypeOf<Counter>(), name) {
	_parent = parent;
	std::string parentName = "";
	if (_parent != nullptr)
		parentName = _parent->getName();
	// OLD
	//GetterMemberDouble getterMember = DefineGetterMember<Counter>(this, &Counter::getCountValue);
	//SimulationResponse* resp = new SimulationResponse(Util::TypeOf<Counter>(), /*parentName + ":" +*/ getName(), getterMember, parentName);
	//_parentModel->getResponses()->insert(resp);
	// NEW !!
	model->getResponses()->insert(new PropertyT<double>(Util::TypeOf<Counter>(), getName(),
			DefineGetter<Counter,double>(this, &Counter::getCountValue),
			nullptr));
}

std::string Counter::show() {
	return ModelDataDefinition::show() +
			", count=" + std::to_string(this->_count);
}

void Counter::clear() {
	_count = 0.0;
}

void Counter::incCountValue(double value) {
	_count += value;
}

double Counter::getCountValue() const {
	return _count;
}

ModelDataDefinition* Counter::getParent() const {
	return _parent;
}

PluginInformation* Counter::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Counter>(), &Counter::LoadInstance, &Counter::NewInstance);
	info->setGenerateReport(true);
	info->setDescriptionHelp("The Counter modeldatum is used to count events, and its internal count value is added by a configurable amount, usually incremented by one.");
	return info;
}

ModelDataDefinition* Counter::LoadInstance(Model* model, PersistenceRecord *fields) {
	Counter* newElement = new Counter(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

bool Counter::_loadInstance(PersistenceRecord *fields) {
	return ModelDataDefinition::_loadInstance(fields);
}

void Counter::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
}

bool Counter::_check(std::string* errorMessage) {
	*errorMessage += "";
	return true;
}

void Counter::_initBetweenReplications() {
	_count = 0.0;
}
