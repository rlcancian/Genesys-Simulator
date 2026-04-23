/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/* 
 * File:   Schedule.cpp
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 * 
 * Created on 14 de dezembro de 2022, 04:24
 */

#include "plugins/data/DiscreteProcessing/Schedule.h"

#include "kernel/simulator/Model.h"
#include <cmath>
#include <sstream>

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

Schedule::~Schedule() {
	for (SchedulableItem* item : *_schedulableItems->list()) {
		delete item;
	}
	delete _schedulableItems;
}
//
//public
//

std::string Schedule::show() {
	std::stringstream ss;
	ss << ModelDataDefinition::show()
	   << ",items=" << _schedulableItems->size()
	   << ",repeatAfterLast=" << (_repeatAfterLast ? "true" : "false");
	return ss.str();
}

std::string Schedule::getExpression() {
	if (_schedulableItems->size() == 0) {
		return "";
	}

	double tnow = _parentModel->getSimulation()->getSimulatedTime();
	double cycleDuration = 0.0;
	for (SchedulableItem* item : *_schedulableItems->list()) {
		cycleDuration += item->getDuration();
	}

	double targetTime = tnow;
	if (_repeatAfterLast) {
		if (cycleDuration <= 0.0) {
			return _schedulableItems->last()->getExpression();
		}
		targetTime = std::fmod(tnow, cycleDuration);
		if (targetTime < 0.0) {
			targetTime += cycleDuration;
		}
	}

	double accumDuration = 0.0;
	for (SchedulableItem* item : *_schedulableItems->list()) {
		accumDuration += item->getDuration();
		if (targetTime <= accumDuration) {
			return item->getExpression();
		}
	}
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
	info->setDescriptionHelp("Defines a repeating or finite list of expression-duration items that can be queried by simulated time.");
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
		try {
			/*!
			 * \brief Load schedulable items and repeat behavior.
			 */
			_repeatAfterLast = fields->loadField("repeatAfterLast", DEFAULT.repeatAfterLast);
			unsigned int items = fields->loadField("items", 0u);
			for (SchedulableItem* item : *_schedulableItems->list()) {
				delete item;
			}
			_schedulableItems->clear();
			for (unsigned int i = 0; i < items; i++) {
				std::string suffix = Util::StrIndex(i);
				std::string expression = fields->loadField("itemExpression" + suffix, "");
				double duration = fields->loadField("itemDuration" + suffix, 0.0);
				SchedulableItem::Rule rule = static_cast<SchedulableItem::Rule>(fields->loadField("itemRule" + suffix, static_cast<int>(SchedulableItem::Rule::IGNORE)));
				_schedulableItems->insert(new SchedulableItem(expression, duration, rule));
			}
		} catch (...) {
		}
	}
	return res;
}

void Schedule::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	/*!
	 * \brief Persist schedule items and repeat behavior.
	 */
	fields->saveField("repeatAfterLast", _repeatAfterLast, DEFAULT.repeatAfterLast, saveDefaultValues);
	fields->saveField("items", _schedulableItems->size(), 0u, saveDefaultValues);
	unsigned int i = 0;
	for (SchedulableItem* item : *_schedulableItems->list()) {
		std::string suffix = Util::StrIndex(i);
		fields->saveField("itemExpression" + suffix, item->getExpression(), "", saveDefaultValues);
		fields->saveField("itemDuration" + suffix, item->getDuration(), 0.0, saveDefaultValues);
		fields->saveField("itemRule" + suffix, static_cast<int>(item->getRule()), static_cast<int>(SchedulableItem::Rule::IGNORE), saveDefaultValues);
		i++;
	}
}

//
// protected virtual -- could be overriden 
//

bool Schedule::_check(std::string& errorMessage) {
	/*!
	 * \brief Validate that schedule has items and non-negative durations.
	 */
	bool resultAll = true;
	bool hasPositiveDuration = false;
	if (_schedulableItems->size() == 0) {
		errorMessage += "Schedule has no schedulable items. ";
		resultAll = false;
	}
	for (SchedulableItem* item : *_schedulableItems->list()) {
		if (item->getDuration() < 0.0) {
			errorMessage += "Schedule item duration must be >= 0. ";
			resultAll = false;
		}
		if (item->getDuration() > 0.0) {
			hasPositiveDuration = true;
		}
		resultAll &= _parentModel->checkExpression(item->getExpression(), getName() + ".ItemExpression", errorMessage);
	}
	if (_repeatAfterLast && !hasPositiveDuration) {
		errorMessage += "Schedule repeating cycle must contain at least one item with duration > 0. ";
		resultAll = false;
	}
	return resultAll;
}

void Schedule::_initBetweenReplications() {
	//_someString = "Test";
	//_someUint = 1;
}

void Schedule::_createInternalAndAttachedData() {
	// Schedule currently has no internal or attached data to instantiate.
}

ParserChangesInformation* Schedule::_getParserChangesInformation() {
	ParserChangesInformation* changes = new ParserChangesInformation();
	/*!
	 * \brief Return parser changes required by Schedule.
	 *
	 * No parser customization is currently necessary.
	 */
	//changes->getProductionToAdd()->insert(...);
	//changes->getTokensToAdd()->insert(...);
	return changes;
}

void Schedule::_addSimulationControl(SimulationControl* property) {
	/*!
	 * \brief Keep local control registration aligned with the base model-data contract.
	 */
	ModelDataDefinition::_addSimulationControl(property);
}

//
// private
//
