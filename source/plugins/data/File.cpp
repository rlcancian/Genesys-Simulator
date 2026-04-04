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

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &File::GetPluginInformation;
}
#endif

ModelDataDefinition* File::NewInstance(Model* model, std::string name) {
	return new File(model, name);
}

File::File(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<File>(), name) {
	//_elems = elems;
}

std::string File::show() {
	return ModelDataDefinition::show() +
			"";
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
			/*!
			 * \brief Load file metadata fields.
			 *
			 * File currently stores only the base ModelDataDefinition data.
			 * Keep the template commands below as guidance when file-specific
			 * attributes are introduced.
			 */
			// this->_accessType = fields->loadField("accessType", DEFAULT.accessType);
			// this->_systemFilename = fields->loadField("systemFilename", DEFAULT.systemFilename);
			// this->_recordsetName = fields->loadField("recordsetName", DEFAULT.recordsetName);
		} catch (...) {
		}
	}
	return res;
}

void File::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	/*!
	 * \brief Save file metadata fields.
	 */
	// fields->saveField("accessType", _accessType, DEFAULT.accessType, saveDefaultValues);
	// fields->saveField("systemFilename", _systemFilename, DEFAULT.systemFilename, saveDefaultValues);
	// fields->saveField("recordsetName", _recordsetName, DEFAULT.recordsetName, saveDefaultValues);
}

bool File::_check(std::string& errorMessage) {
	/*!
	 * \brief Validate file metadata consistency.
	 */
	bool resultAll = true;
	// resultAll &= (_systemFilename != "");
	// resultAll &= Util::FileExists(_systemFilename);
	(void) errorMessage;
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
