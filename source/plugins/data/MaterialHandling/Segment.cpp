/*
 * File:   Segment.cpp
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 30 de Agosto de 2026
 */

#include "Segment.h"
#include "../../../kernel/simulator/model/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Segment::GetPluginInformation;
}
#endif

ModelDataDefinition* Segment::NewInstance(Model* model, std::string name) {
	return new Segment(model, name);
}

Segment::Segment(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<Segment>(), name) {
}

Segment::~Segment() {
	for (SegmentStep* step : *_steps->list()) {
		delete step;
	}
	delete _steps;
}

std::string Segment::show() {
	return ModelDataDefinition::show() + ", steps=" + std::to_string(_steps->size());
}

List<SegmentStep*>* Segment::getSteps() const {
	return _steps;
}

void Segment::insertStep(SegmentStep* step) {
	_steps->insert(step);
}

double Segment::getDistanceBetween(const std::string& fromStationName, const std::string& toStationName) const {
	bool foundFrom = false;
	double accumulated = 0.0;
	for (SegmentStep* step : *_steps->list()) {
		if (!foundFrom) {
			if (step->stationName == fromStationName) {
				foundFrom = true;
			} else {
				continue;
			}
		}
		if (step->stationName == toStationName) {
			return accumulated;
		}
		accumulated += step->lengthToNext;
	}
	return -1.0;
}

PluginInformation* Segment::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Segment>(), &Segment::LoadInstance, &Segment::NewInstance);
	info->setCategory("MaterialHandling");
	info->setDescriptionHelp("Defines the ordered sequence of stations and inter-station lengths that make up one Conveyor's physical path.");
	return info;
}

ModelDataDefinition* Segment::LoadInstance(Model* model, PersistenceRecord *fields) {
	Segment* newElement = new Segment(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
		newElement->traceError("Failed to load Segment instance: " + std::string(e.what()));
	}
	return newElement;
}

bool Segment::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		unsigned int numSteps = fields->loadField("steps", 0u);
		for (unsigned int i = 0; i < numSteps; i++) {
			std::string prefix = "step" + Util::StrIndex(i) + ".";
			std::string stationName = fields->loadField(prefix + "station", std::string(""));
			double lengthToNext = fields->loadField(prefix + "lengthToNext", 0.0);
			_steps->insert(new SegmentStep(stationName, lengthToNext));
		}
	}
	return res;
}

void Segment::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("steps", _steps->size(), 0u, saveDefaultValues);
	unsigned int i = 0;
	for (SegmentStep* step : *_steps->list()) {
		std::string prefix = "step" + Util::StrIndex(i) + ".";
		fields->saveField(prefix + "station", step->stationName, std::string(""), saveDefaultValues);
		fields->saveField(prefix + "lengthToNext", step->lengthToNext, 0.0, saveDefaultValues);
		i++;
	}
}

bool Segment::_check(std::string& errorMessage) {
	if (_steps->size() < 2) {
		errorMessage += "Segment \"" + getName() + "\" needs at least two stations. ";
		return false;
	}
	return true;
}
