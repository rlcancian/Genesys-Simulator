/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   File.cpp
 * Author: rlcancian
 * 
 * Created on 20 de Fileembro de 2019, 20:07
 */

#include "File.h"
#include "kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &File::GetPluginInformation;
}
#endif

ModelDataDefinition* File::NewInstance(Model* model, std::string name) {
	return new File(model, name);
}

File::File(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<File>(), name) {
	SimulationControlGeneric<std::string>* propSystemFilename = new SimulationControlGeneric<std::string>(
			std::bind(&File::getSystemFilename, this), std::bind(&File::setSystemFilename, this, std::placeholders::_1),
			Util::TypeOf<File>(), getName(), "SystemFilename", "");
	SimulationControlGeneric<std::string>* propAccessMode = new SimulationControlGeneric<std::string>(
			std::bind(&File::getAccessModeAsString, this), std::bind(&File::setAccessModeAsString, this, std::placeholders::_1),
			Util::TypeOf<File>(), getName(), "AccessMode", "");
	_parentModel->getControls()->insert(propSystemFilename);
	_parentModel->getControls()->insert(propAccessMode);
	_addProperty(propSystemFilename);
	_addProperty(propAccessMode);
}

std::string File::show() {
	return ModelDataDefinition::show() +
			",systemFilename=\"" + _systemFilename + "\"" +
			",filenameOnly=\"" + getFilenameOnly() + "\"" +
			",pathOnly=\"" + getPathOnly() + "\"" +
			",accessMode=\"" + getAccessModeAsString() + "\"";
}

PluginInformation* File::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<File>(), &File::LoadInstance, &File::NewInstance);
	info->setDescriptionHelp("Defines metadata for external files used by model elements that read/write structured data.");
	return info;
}

ModelDataDefinition* File::LoadInstance(Model* model, PersistenceRecord *fields) {
	File* newElement = new File(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

bool File::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		try {
			setSystemFilename(fields->loadField("systemFilename", DEFAULT.systemFilename));
			setAccessModeAsString(fields->loadField("accessMode", _accessModeToString(DEFAULT.accessMode)));
		} catch (...) {
		}
	}
	return res;
}

void File::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("systemFilename", _systemFilename, DEFAULT.systemFilename, saveDefaultValues);
	fields->saveField("accessMode", getAccessModeAsString(), _accessModeToString(DEFAULT.accessMode), saveDefaultValues);
}

bool File::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_systemFilename.empty()) {
		errorMessage += "File \"" + getName() + "\" must define a non-empty systemFilename. ";
		resultAll = false;
	}
	if (_systemFilename.empty() == false) {
		if (_accessModeWasInvalid) {
			errorMessage += "File \"" + getName() + "\" has unsupported accessMode. ";
			resultAll = false;
		}
		if ((_accessMode == AccessMode::Read || _accessMode == AccessMode::ReadWrite) && !Util::FileExists(_systemFilename)) {
			errorMessage += "File \"" + getName() + "\" requires an existing file for accessMode \"" + getAccessModeAsString() + "\". ";
			resultAll = false;
		}
	}
	return resultAll;
}

ParserChangesInformation* File::_getParserChangesInformation() {
	ParserChangesInformation* changes = new ParserChangesInformation();
	/*!
	 * \brief Return parser deltas required by File.
	 *
	 * File currently does not require parser extensions.
	 */
	//changes->getProductionToAdd()->insert(...);
	//changes->getTokensToAdd()->insert(...);
	return changes;
}

void File::setSystemFilename(std::string systemFilename) {
	_systemFilename = _normalizePathSeparators(systemFilename);
}

std::string File::getSystemFilename() const {
	return _systemFilename;
}

std::string File::getFilenameOnly() const {
	const std::string normalized = _normalizePathSeparators(_systemFilename);
	return Util::FilenameFromFullFilename(normalized);
}

std::string File::getPathOnly() const {
	const std::string normalized = _normalizePathSeparators(_systemFilename);
	const size_t sepPos = normalized.find_last_of(Util::DirSeparator());
	if (sepPos == std::string::npos) {
		return "";
	}
	return Util::PathFromFullFilename(normalized);
}

void File::setAccessMode(AccessMode accessMode) {
	_accessMode = accessMode;
	_accessModeWasInvalid = false;
}

File::AccessMode File::getAccessMode() const {
	return _accessMode;
}

void File::setAccessModeAsString(std::string accessMode) {
	AccessMode parsed = DEFAULT.accessMode;
	if (_stringToAccessMode(accessMode, &parsed)) {
		_accessMode = parsed;
		_accessModeWasInvalid = false;
	} else {
		_accessModeWasInvalid = true;
	}
}

std::string File::getAccessModeAsString() const {
	return _accessModeToString(_accessMode);
}

std::string File::_accessModeToString(AccessMode accessMode) {
	switch (accessMode) {
		case AccessMode::Read: return "Read";
		case AccessMode::Write: return "Write";
		case AccessMode::Append: return "Append";
		case AccessMode::ReadWrite: return "ReadWrite";
		default: return "Read";
	}
}

bool File::_stringToAccessMode(const std::string& accessMode, AccessMode* parsedAccessMode) {
	if (accessMode == "Read") {
		*parsedAccessMode = AccessMode::Read;
		return true;
	}
	if (accessMode == "Write") {
		*parsedAccessMode = AccessMode::Write;
		return true;
	}
	if (accessMode == "Append") {
		*parsedAccessMode = AccessMode::Append;
		return true;
	}
	if (accessMode == "ReadWrite") {
		*parsedAccessMode = AccessMode::ReadWrite;
		return true;
	}
	return false;
}

std::string File::_normalizePathSeparators(const std::string& filename) {
	std::string normalized = filename;
	const char separator = Util::DirSeparator();
	for (char& ch : normalized) {
		if (ch == '/' || ch == '\\') {
			ch = separator;
		}
	}
	return normalized;
}
