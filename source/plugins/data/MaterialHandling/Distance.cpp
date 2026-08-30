/*
 * File:   Distance.cpp
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 30 de Agosto de 2026
 */

#include "Distance.h"
#include "../../../kernel/simulator/model/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Distance::GetPluginInformation;
}
#endif

ModelDataDefinition* Distance::NewInstance(Model* model, std::string name) {
	return new Distance(model, name);
}

Distance::Distance(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<Distance>(), name) {
}

Distance::~Distance() {
	for (DistanceEntry* entry : *_entries->list()) {
		delete entry;
	}
	delete _entries;
}

std::string Distance::show() {
	return ModelDataDefinition::show() + ", entries=" + std::to_string(_entries->size());
}

List<DistanceEntry*>* Distance::getEntries() const {
	return _entries;
}

void Distance::insertEntry(DistanceEntry* entry) {
	_entries->insert(entry);
}

double Distance::getDistanceBetween(const std::string& fromStationName, const std::string& toStationName) const {
	for (DistanceEntry* entry : *_entries->list()) {
		if (entry->fromStationName == fromStationName && entry->toStationName == toStationName) {
			return entry->length;
		}
		if (entry->bidirectional && entry->fromStationName == toStationName && entry->toStationName == fromStationName) {
			return entry->length;
		}
	}
	return -1.0;
}

PluginInformation* Distance::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Distance>(), &Distance::LoadInstance, &Distance::NewInstance);
	info->setCategory("MaterialHandling");
	info->setDescriptionHelp("Defines a set of direct station-to-station distances used by free-path Transporter units to compute travel time.");
	return info;
}

ModelDataDefinition* Distance::LoadInstance(Model* model, PersistenceRecord *fields) {
	Distance* newElement = new Distance(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
		newElement->traceError("Failed to load Distance instance: " + std::string(e.what()));
	}
	return newElement;
}

bool Distance::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		unsigned int numEntries = fields->loadField("entries", 0u);
		for (unsigned int i = 0; i < numEntries; i++) {
			std::string prefix = "entry" + Util::StrIndex(i) + ".";
			std::string from = fields->loadField(prefix + "from", std::string(""));
			std::string to = fields->loadField(prefix + "to", std::string(""));
			double length = fields->loadField(prefix + "length", 0.0);
			bool bidir = fields->loadField(prefix + "bidirectional", true);
			_entries->insert(new DistanceEntry(from, to, length, bidir));
		}
	}
	return res;
}

void Distance::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("entries", _entries->size(), 0u, saveDefaultValues);
	unsigned int i = 0;
	for (DistanceEntry* entry : *_entries->list()) {
		std::string prefix = "entry" + Util::StrIndex(i) + ".";
		fields->saveField(prefix + "from", entry->fromStationName, std::string(""), saveDefaultValues);
		fields->saveField(prefix + "to", entry->toStationName, std::string(""), saveDefaultValues);
		fields->saveField(prefix + "length", entry->length, 0.0, saveDefaultValues);
		fields->saveField(prefix + "bidirectional", entry->bidirectional, true, saveDefaultValues);
		i++;
	}
}

bool Distance::_check(std::string& errorMessage) {
	if (_entries->size() == 0) {
		errorMessage += "Distance \"" + getName() + "\" has no entries. ";
		return false;
	}
	bool resultAll = true;
	unsigned int index = 0;
	for (DistanceEntry* entry : *_entries->list()) {
		const std::string prefix = "Distance \"" + getName() + "\" entry " + Util::StrIndex(index);
		if (entry == nullptr) {
			errorMessage += prefix + " is null. ";
			resultAll = false;
			index++;
			continue;
		}
		if (entry->fromStationName.empty()) {
			errorMessage += prefix + " must define a beginning station. ";
			resultAll = false;
		}
		if (entry->toStationName.empty()) {
			errorMessage += prefix + " must define an ending station. ";
			resultAll = false;
		}
		if (entry->length < 0.0) {
			errorMessage += prefix + " cannot have a negative distance. ";
			resultAll = false;
		}
		index++;
	}
	return resultAll;
}
