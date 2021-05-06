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
#include "SimulationResponse.h"
#include "Model.h"

//using namespace GenesysKernel;

Counter::Counter(Model* model, std::string name, ModelElement* parent) : ModelElement(model, Util::TypeOf<Counter>(), name) {
	_parent = parent;
	GetterMember getterMember = DefineGetterMember<Counter>(this, &Counter::getCountValue);
	//std::string parentName = "";
	//if (_parent != nullptr)
	//	parentName = _parent->name();
	SimulationResponse* resp = new SimulationResponse(Util::TypeOf<Counter>(), /*parentName + ":" +*/ getName(), getterMember);
	_parentModel->getResponses()->insert(resp);
}



std::string Counter::show() {
	return ModelElement::show() +
			", count=" + std::to_string(this->_count);
}

void
Counter::clear() {
	_count = 0;
}

void Counter::incCountValue(int value) {
	_count += value;
}

unsigned long Counter::getCountValue() const {
	return _count;
}

ModelElement* Counter::getParent() const {
	return _parent;
}

PluginInformation* Counter::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Counter>(), &Counter::LoadInstance);
	info->setGenerateReport(true);
	return info;
}

ModelElement* Counter::LoadInstance(Model* model, std::map<std::string, std::string>* fields) {
	Counter* newElement = new Counter(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

bool Counter::_loadInstance(std::map<std::string, std::string>* fields) {
	return ModelElement::_loadInstance(fields);
}

std::map<std::string, std::string>* Counter::_saveInstance() {
	return ModelElement::_saveInstance();
}
//std::list<std::map<std::string,std::string>*>* _saveInstance(std::string type){}

bool Counter::_check(std::string* errorMessage) {
	return true;
}

