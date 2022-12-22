/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/* 
 * File:   Schedule.cpp
 * Author: rlcancian
 * 
 * Created on 14 de dezembro de 2022, 04:24
 */

#include "Schedule.h"

#include "../../kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Schedule::GetPluginInformation;
}
#endif

//
// constructors
//

ModelDataDefinition* Schedule::NewInstance(Model* model, std::string name) {
	return new Schedule(model, name);
}

Schedule::Schedule(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<Schedule>(), name) {
}
//
//public
//

std::string Schedule::show() {
	return ModelDataDefinition::show();
}

std::string Schedule::getExpression() {
	double tnow = _parentModel->getSimulation()->getSimulatedTime();
	double accumDuration = 0.0;
	do
		for (SchedulableItem* item : *_schedulableItems->list()) {
			if (tnow <= accumDuration + item->getDuration()) {
				return item->getExpression();
			}
			accumDuration += item->getDuration();
		} while (_repeatAfterLast);
	return _schedulableItems->last()->getExpression();
}

List<SchedulableItem*>* Schedule::getSchedulableItems() const {
	return _schedulableItems;
}

void Schedule::setRepeatAfterLast(bool _repeatAfterLast) {
	this->_repeatAfterLast = _repeatAfterLast;
}

bool Schedule::isRepeatAfterLast() const {
	return _repeatAfterLast;
}

//
// public static 
//

ModelDataDefinition* Schedule::LoadInstance(Model* model, PersistenceRecord *fields) {
	Schedule* newElement = new Schedule(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

PluginInformation* Schedule::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Schedule>(), &Schedule::LoadInstance, &Schedule::NewInstance);
	info->setDescriptionHelp("//@TODO");
	//info->setDescriptionHelp("");
	//info->setObservation("");
	//info->setMinimumOutputs();
	//info->setDynamicLibFilenameDependencies();
	//info->setFields();
	// ...
	return info;
}

//
// protected virtual -- must be overriden 
//

bool Schedule::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
            _repeatAfterLast = fields->loadField("RepeatAfterLast", DEFAULT.repeatAfterLast);
	}
	return res;
}

void Schedule::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("RepeatAfterLast", _repeatAfterLast, DEFAULT.repeatAfterLast);
}

//
// protected virtual -- could be overriden 
//

bool Schedule::_check(std::string* errorMessage) {
	bool resultAll = true;
	//resultAll &= _someString != "";
	//resultAll &= _someUint > 0;
	return resultAll;
}

void Schedule::_initBetweenReplications() {
	//_someString = "Test";
	//_someUint = 1;
}

void Schedule::_createInternalAndAttachedData() {
	if (_reportStatistics) {
		//if (_internal == nullptr) {
		//	_internal = new StatisticsCollector(_parentModel, getName() + "." + "NumberInQueue", this); 
		//	_internelElementsInsert("NumberInQueue", _internal);
		//}
	} else { //if (_cstatNumberInQueue != nullptr) {
		this->_internalDataClear();
	}
}

ParserChangesInformation* Schedule::_getParserChangesInformation() {
	ParserChangesInformation* changes = new ParserChangesInformation();
	//@TODO not implemented yet
	//changes->getProductionToAdd()->insert(...);
	//changes->getTokensToAdd()->insert(...);
	return changes;
}

void Schedule::_addProperty(PropertyBase* property) {

}

//
// private
//