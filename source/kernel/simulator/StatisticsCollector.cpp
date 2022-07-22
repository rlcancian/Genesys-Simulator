/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   StatisticsCollector.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 30 de Agosto de 2018, 17:24
 */

#include "StatisticsCollector.h"
#include "../TraitsKernel.h"

//using namespace GenesysKernel;

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &StatisticsCollector::GetPluginInformation;
}
#endif

ModelDataDefinition* StatisticsCollector::NewInstance(Model* model, std::string name) {
	return new StatisticsCollector(model, name);
}


typedef TraitsKernel<Model>::StatisticsCollector_StatisticsImplementation StatisticsClass;

StatisticsCollector::StatisticsCollector(Model* model, std::string name, ModelDataDefinition* parent, bool insertIntoModel) : ModelDataDefinition(model, Util::TypeOf<StatisticsCollector>(), name, insertIntoModel) {
	_parent = parent;
	_initStaticsAndCollector();
	_parentModel->getResponses()->insert(new PropertyT<double>(Util::TypeOf<StatisticsClass>(), getName() + ".average",
			DefineGetter<StatisticsClass, double>(static_cast<StatisticsClass*>(this->_statistics),  &StatisticsClass::average), nullptr, getName()));
	_parentModel->getResponses()->insert(new PropertyT<double>(Util::TypeOf<StatisticsClass>(), getName() + ".halfwidth",
			DefineGetter<StatisticsClass,double>(static_cast<StatisticsClass*>(this->_statistics),  &StatisticsClass::halfWidthConfidenceInterval), nullptr, getName()));
}

void StatisticsCollector::_initStaticsAndCollector() {
	Collector_if* collector = new TraitsKernel<Model>::StatisticsCollector_CollectorImplementation();
	_statistics = new StatisticsClass(collector);
}

std::string StatisticsCollector::show() {
	std::string parentStr = "";
	if (_parent != nullptr) {
		try {
			parentStr = _parent->getName();
		} catch (...) { // if parent changed or deleted, can cause seg fault
			parentStr = "<<INCONSISTENT>>"; /* @TODO: ++*/
		}
	}
	return ModelDataDefinition::show() +
			",parent=\"" + parentStr + "\"" +
			",numElements=" + std::to_string(_statistics->numElements());
}

ModelDataDefinition* StatisticsCollector::getParent() const {
	return _parent;
}

Statistics_if* StatisticsCollector::getStatistics() const {
	return _statistics;
}

PluginInformation* StatisticsCollector::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<StatisticsCollector>(), &StatisticsCollector::LoadInstance, &StatisticsCollector::NewInstance);
	info->setGenerateReport(true);
	info->setDescriptionHelp("The StatisticsCollector is the ModelDataDefinition responsible for collecting data from the model (using the Collector) and simultaneouly keeping statistics updated (using the Statistics)");

	return info;
}

ModelDataDefinition* StatisticsCollector::LoadInstance(Model* model, PersistenceRecord *fields) {
	StatisticsCollector* newElement = new StatisticsCollector(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

bool StatisticsCollector::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
	}
	return res;
}

void StatisticsCollector::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	std::string parentId = "", parentTypename = "";
	if (this->_parent != nullptr) {
		parentId = std::to_string(_parent->getId());
		parentTypename = _parent->getClassname();
	}
	fields->saveField("parentTypename", parentTypename);
	fields->saveField("parentId", parentId);
}

bool StatisticsCollector::_check(std::string* errorMessage) {
	// @TODO: To implement!
	*errorMessage += "";
	return true;
}
